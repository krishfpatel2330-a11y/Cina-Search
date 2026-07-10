#include "../include/TaskCompleteWindow.h"
#include <SFML/Window/Event.hpp>
#include <filesystem>
#include <algorithm>
#include <string>

static const sf::Color TC_BG       {18,  18,  28,  255};
static const sf::Color TC_PANEL    {28,  28,  45,  255};
static const sf::Color TC_ACCENT   {72,  149, 239, 255};
static const sf::Color TC_GREEN    {80,  220, 130, 255};
static const sf::Color TC_DIM      {40,  90,  160, 255};
static const sf::Color TC_TXT_P    {230, 230, 245, 255};
static const sf::Color TC_TXT_S    {130, 130, 160, 255};
static const sf::Color TC_ITEM_BG  {35,  35,  58,  255};
static const sf::Color TC_ITEM_ALT {32,  32,  52,  255};

static void tcCard(sf::RenderWindow& w, float x, float y, float bw, float bh,
                   sf::Color fill, sf::Color outline = sf::Color::Transparent, float t = 0.f)
{
    sf::RectangleShape r({bw, bh});
    r.setPosition({x, y});
    r.setFillColor(fill);
    r.setOutlineColor(outline);
    r.setOutlineThickness(t);
    w.draw(r);
}

//  run
bool TaskCompleteWindow::run(sf::RenderWindow& window,
                             const std::vector<std::string>& movies,
                             const std::string& actorName,
                             int yearStart, int yearEnd,
                             const std::string& winnerTree)
{
    std::filesystem::path fontPath = std::filesystem::path("assets") / "fonts" / "arial.ttf";
    m_font.openFromFile(fontPath.string());

    float scrollOffset = 0.f;
    const float ITEM_H   = 36.f;
    const float LIST_TOP = 210.f;
    const float LIST_BOT = WIN_H - 100.f;
    const float LIST_VIS = LIST_BOT - LIST_TOP;
    const float contentH = movies.size() * ITEM_H;

    while (window.isOpen()) {
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) { window.close(); return true; }

            if (const auto* ks = ev->getIf<sf::Event::KeyPressed>()) {
                if (ks->code == sf::Keyboard::Key::Escape) return true;
                if (ks->code == sf::Keyboard::Key::Down)
                    scrollOffset = std::min(scrollOffset + ITEM_H,
                                            std::max(0.f, contentH - LIST_VIS));
                if (ks->code == sf::Keyboard::Key::Up)
                    scrollOffset = std::max(0.f, scrollOffset - ITEM_H);
            }

            if (const auto* mw = ev->getIf<sf::Event::MouseWheelScrolled>()) {
                scrollOffset -= mw->delta * ITEM_H;
                scrollOffset = std::clamp(scrollOffset, 0.f,
                                          std::max(0.f, contentH - LIST_VIS));
            }
        }

        window.clear(TC_BG);
        draw(window, movies, actorName, yearStart, yearEnd, winnerTree, scrollOffset);
        window.display();
    }
    return true;
}

//  draw
void TaskCompleteWindow::draw(sf::RenderWindow& window,
                              const std::vector<std::string>& movies,
                              const std::string& actorName,
                              int yearStart, int yearEnd,
                              const std::string& winnerTree,
                              float scrollOffset)
{
    const float cx    = WIN_W / 2.f;
    const float ITEM_H  = 36.f;
    const float LIST_X  = cx - 310.f;
    const float LIST_W  = 620.f;
    const float LIST_TOP = 210.f;
    const float LIST_BOT = WIN_H - 100.f;

    // Background glow
    sf::RectangleShape glow({WIN_W, 180.f});
    glow.setPosition({0, 0});
    glow.setFillColor({30, 80, 60, 20});
    window.draw(glow);

    // Header
    sf::Text check(m_font, "Task Complete", 26);
    check.setFillColor(TC_GREEN);
    check.setStyle(sf::Text::Bold);
    check.setPosition({cx - check.getLocalBounds().size.x / 2.f, 28.f});
    window.draw(check);

    // "Movies from [years] featuring [actor]"
    std::string heading = "Movies from " + std::to_string(yearStart) +
                          "-" + std::to_string(yearEnd) +
                          " featuring " + actorName;
    sf::Text hdg(m_font, heading, 17);
    hdg.setFillColor(TC_TXT_P);
    hdg.setPosition({cx - hdg.getLocalBounds().size.x / 2.f, 76.f});
    window.draw(hdg);

    // Result count
    std::string countStr = std::to_string(movies.size()) + " result" +
                           (movies.size() != 1 ? "s" : "") + " found";
    sf::Text countTxt(m_font, countStr, 13);
    countTxt.setFillColor(TC_TXT_S);
    countTxt.setPosition({cx - countTxt.getLocalBounds().size.x / 2.f, 106.f});
    window.draw(countTxt);

    // List panel
    tcCard(window, LIST_X - 4.f, LIST_TOP - 4.f, LIST_W + 8.f,
           LIST_BOT - LIST_TOP + 8.f, TC_PANEL, TC_DIM, 1.f);

    // draw only visible rows
    int firstVisible = static_cast<int>(scrollOffset / ITEM_H);
    int lastVisible  = static_cast<int>((scrollOffset + (LIST_BOT - LIST_TOP)) / ITEM_H) + 1;

    for (int i = firstVisible; i < lastVisible && i < static_cast<int>(movies.size()); ++i) {
        float itemY = LIST_TOP + (i * ITEM_H) - scrollOffset;
        if (itemY + ITEM_H < LIST_TOP || itemY > LIST_BOT) continue;

        sf::Color bg = (i % 2 == 0) ? TC_ITEM_BG : TC_ITEM_ALT;
        tcCard(window, LIST_X, itemY, LIST_W, ITEM_H - 2.f, bg);

        // Row number
        sf::Text num(m_font, std::to_string(i + 1) + ".", 13);
        num.setFillColor(TC_TXT_S);
        num.setPosition({LIST_X + 10.f, itemY + 10.f});
        window.draw(num);

        // Movie title
        sf::Text mtxt(m_font, movies[i], 14);
        mtxt.setFillColor(TC_TXT_P);
        mtxt.setPosition({LIST_X + 44.f, itemY + 10.f});
        window.draw(mtxt);
    }

    sf::RectangleShape fadeTop({LIST_W, 18.f});
    fadeTop.setPosition({LIST_X, LIST_TOP});
    fadeTop.setFillColor({18, 18, 28, 120});
    window.draw(fadeTop);

    sf::RectangleShape fadeBot({LIST_W, 18.f});
    fadeBot.setPosition({LIST_X, LIST_BOT - 18.f});
    fadeBot.setFillColor({18, 18, 28, 120});
    window.draw(fadeBot);

    // Winner message
    std::string winMsg = "The data structure completed first was the  " + winnerTree;
    sf::Text winTxt(m_font, winMsg, 14);
    winTxt.setFillColor(TC_GREEN);
    winTxt.setPosition({cx - winTxt.getLocalBounds().size.x / 2.f, LIST_BOT + 16.f});
    window.draw(winTxt);

    // Footer
    sf::Text hint(m_font, "Scroll to browse   ESC to exit", 12);
    hint.setFillColor({70, 70, 100, 255});
    hint.setPosition({cx - hint.getLocalBounds().size.x / 2.f, WIN_H - 22.f});
    window.draw(hint);
}