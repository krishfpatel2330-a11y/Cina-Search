#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class TaskCompleteWindow {
public:
    bool run(sf::RenderWindow& window,
             const std::vector<std::string>& movies,
             const std::string& actorName,
             int yearStart, int yearEnd,
             const std::string& winnerTree);

private:
    void draw(sf::RenderWindow& window,
              const std::vector<std::string>& movies,
              const std::string& actorName,
              int yearStart, int yearEnd,
              const std::string& winnerTree,
              float scrollOffset);

    sf::Font m_font;

    static constexpr float WIN_W = 900.f;
    static constexpr float WIN_H = 580.f;
};
