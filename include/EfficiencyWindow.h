#pragma once
#include <SFML/Graphics.hpp>

class EfficiencyWindow {
public:
    // bTimeMs  – milliseconds the B  tree took
    // bpTimeMs – milliseconds the B+ tree took
    // Returns true when both bars are full and the window should advance.
    bool run(sf::RenderWindow& window, float bTimeMs, float bpTimeMs);

private:
    void draw(sf::RenderWindow& window, float bFill, float bpFill,
              float bTimeMs, float bpTimeMs);

    sf::Font m_font;

    static constexpr float WIN_W   = 900.f;
    static constexpr float WIN_H   = 580.f;
    static constexpr float BAR_W   = 560.f;
    static constexpr float BAR_H   = 42.f;
};
