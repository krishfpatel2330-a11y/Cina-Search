#pragma once
#include <SFML/Graphics.hpp>
#include <string>

struct UserQuery {
    std::string actorName;
    int yearStart = 0;
    int yearEnd   = 0;
    std::string errorOverride;
};

class MainWindow {
public:
    bool run(sf::RenderWindow& window, UserQuery& query);

private:
    void draw(sf::RenderWindow& window);
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);

    int m_focusedField{0};
    std::string m_actorBuf;
    std::string m_yearStartBuf;
    std::string m_yearEndBuf;
    std::string m_errorMsg;
    bool m_submitted{false};
    sf::Font m_font;

    static constexpr float WIN_W   = 900.f;
    static constexpr float WIN_H   = 580.f;
    static constexpr float FIELD_W = 260.f;
    static constexpr float FIELD_H = 44.f;
};
