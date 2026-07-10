#include "bridges-cxx-master/src/Bridges.h"
#include "bridges-cxx-master/src/DataSource.h"
#include "bridges-cxx-master/src/data_src/MovieActorWikidata.h"
#include <SFML/Graphics.hpp>
#include "include/MainWindow.h"
#include "include/EfficiencyWindow.h"
#include "include/TaskCompleteWindow.h"
#include "include/BPTree.hpp"
#include "include/BTree.h"
#include <set>
#include <string>
#include <vector>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <fstream>

// BRIDGES credentials
static const int    BRIDGES_ASSIGNMENT = 1;
static const char*  BRIDGES_USER_ID    = "oflahertyw";
static const char*  BRIDGES_API_KEY    = "1551005396974";

// B+ Tree order: max keys per node before splitting
static const int BP_TREE_ORDER = 50;

// B Tree minimum degree: t=25
static const int B_TREE_T = 25;

// Clear BRIDGES cache to ensure fresh data on each run, otherwise you get JSON errors, and we don't like JSON errors
static void clearBridgesCache() {
    const char* cacheDir = getenv("FORCE_BRIDGES_CACHEDIR");
    std::string dir;
    if (cacheDir) {
        dir = cacheDir;
    } else {
        const char* localAppData = getenv("LOCALAPPDATA");
        if (localAppData) {
            dir = std::string(localAppData) + "/.cache/bridges_data/cxx";
        }
    }
    if (dir.empty()) return;

    // Use filesystem to iterate and delete only wikidata cache files
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;
        std::string name = entry.path().filename().string();
        if (name.rfind("wikidata-actormovie-", 0) != 0) continue;
        try {
            std::filesystem::remove(entry.path());
        } catch (...) {
        }
    }
}

int main()
{
    constexpr unsigned WIN_W = 900;
    constexpr unsigned WIN_H = 580;

    sf::RenderWindow window(sf::VideoMode({WIN_W, WIN_H}),
                            "CinaSearch -- B/B+ Tree Benchmark",
                            sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(60);

    bridges::Bridges bridgesObj(BRIDGES_ASSIGNMENT, BRIDGES_USER_ID, BRIDGES_API_KEY);
    bridgesObj.setTitle("CinaSearch");
    bridges::DataSource ds(&bridgesObj);

    while (window.isOpen()) {

        // SCREEN 1: Main Input Window
        UserQuery query;
        {
            MainWindow mw;
            bool ok = mw.run(window, query);
            if (!ok || !window.isOpen()) break;
        }

        clearBridgesCache();

        // DATA FETCH
        std::vector<bridges::dataset::MovieActorWikidata> wikidataRecords;
        try {
            std::cerr << "Fetching data for years " << query.yearStart
                      << " to " << query.yearEnd << "\n";
            wikidataRecords = ds.getWikidataActorMovie(query.yearStart,
                                                       query.yearEnd);
            std::cerr << "Fetched " << wikidataRecords.size() << " records.\n";
        } catch (const std::string& e) {
            std::cerr << "BRIDGES error: " << e << "\n";
            continue;
        } catch (const char* e) {
            std::cerr << "BRIDGES error: " << e << "\n";
            continue;
        } catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << "\n";
            continue;
        }

        // TREE BUILD & TIMING
        // both trees receive the same dataset
        std::set<std::pair<std::string,std::string>> seen;
        for (const auto& record : wikidataRecords) {
            seen.insert(std::make_pair(record.getActorName(), record.getMovieName()));
        }

        // B+ Tree
        BPTree bpTree(BP_TREE_ORDER);
        auto bpStart = std::chrono::high_resolution_clock::now();
        for (const auto& pair : seen) {
            bpTree.insert(pair.first, pair.second);
        }
        auto bpEnd = std::chrono::high_resolution_clock::now();
        float bpMs = static_cast<float>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                bpEnd - bpStart).count());

        // B Tree
        BTree bTree(B_TREE_T);
        auto bStart = std::chrono::high_resolution_clock::now();
        for (const auto& pair : seen) {
            bTree.insert(pair.first, pair.second);
        }
        auto bEnd = std::chrono::high_resolution_clock::now();
        float bMs = static_cast<float>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                bEnd - bStart).count());

        //  retrieve movies for the requested actor from B+ Tree
        std::vector<std::string> movies = bpTree.findMovies(query.actorName);

        // If no movies found, loop back to Screen 1 with an error message
        if (movies.empty()) {
            query.errorOverride = "No movies found for \"" + query.actorName +
                                  "\". Check the spelling and try again.";
            continue;
        }

        // Lower time wins. B+ Tree wins on a tie.
        std::string winner = (bMs < bpMs) ? "B Tree" : "B+ Tree";

        // SCREEN 2: Efficiency Display Window
        {
            EfficiencyWindow ew;
            bool ok = ew.run(window, bMs, bpMs);
            if (!ok || !window.isOpen()) break;
        }

        // SCREEN 3: Task Complete Window
        {
            TaskCompleteWindow tw;
            tw.run(window, movies, query.actorName,
                   query.yearStart, query.yearEnd, winner);
        }
    }

    return 0;
}
