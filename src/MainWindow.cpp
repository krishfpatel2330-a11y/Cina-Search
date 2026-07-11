#include "../include/MainWindow.h"
#include <SFML/Window/Event.hpp>
#include <filesystem>
#include <algorithm>
#include <cctype>

static const sf::Color BG_COLOR        {20, 16, 14, 255};
static const sf::Color PANEL_COLOR     {34, 28, 24, 255};
static const sf::Color ACCENT          {233, 180, 76, 255};   // gold (marquee accent)
static const sf::Color ACCENT_DIM      {150, 110, 40, 255};
static const sf::Color TEXT_PRIMARY    {245, 240, 230, 255};
static const sf::Color TEXT_SECONDARY  {165, 150, 130, 255};
static const sf::Color FIELD_BG        {42, 34, 28, 255};
static const sf::Color FIELD_ACTIVE    {54, 44, 34, 255};
static const sf::Color ERROR_COLOR     {225, 90, 70, 255};
static const sf::Color BTN_NORMAL      {233, 180, 76, 255};
static const sf::Color BTN_HOVER       {245, 200, 110, 255};

// Tiny helper
static void drawCard(sf::RenderWindow& w, float x, float y, float bw, float bh,
                     sf::Color fill, sf::Color outline = sf::Color::Transparent,
                     float thick = 0.f)
{
    sf::RectangleShape r({bw, bh});
    r.setPosition({x, y});
    r.setFillColor(fill);
    r.setOutlineColor(outline);
    r.setOutlineThickness(thick);
    w.draw(r);
}

// run
bool MainWindow::run(sf::RenderWindow& window, UserQuery& query)
{
    // Load font from assets
    std::filesystem::path fontPath = std::filesystem::path("assets") / "fonts" / "arial.ttf";
    m_font.openFromFile(fontPath.string());

    m_submitted    = false;
    m_actorBuf     = "";
    m_yearStartBuf = "";
    m_yearEndBuf   = "";
    m_errorMsg     = "";
    m_focusedField = 0;

    sf::Clock blinkClock;

    while (window.isOpen() && !m_submitted) {
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) {
                window.close();
                return false;
            }
            handleEvent(*ev, window);
        }

        window.clear(BG_COLOR);
        draw(window);

        bool cursorVisible = (static_cast<int>(blinkClock.getElapsedTime().asSeconds() * 2) % 2 == 0);

        window.display();
    }

    if (!m_submitted) return false;

    // Validate & parse
    try {
        query.actorName  = m_actorBuf;
        query.yearStart  = std::stoi(m_yearStartBuf);
        query.yearEnd    = std::stoi(m_yearEndBuf);
    } catch (...) {
        return false;
    }
    return true;
}

// handleEvent
void MainWindow::handleEvent(const sf::Event& event, sf::RenderWindow& window)
{
    // Mouse click
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left) {
            float mx = static_cast<float>(mb->position.x);
            float my = static_cast<float>(mb->position.y);

            const float cx  = WIN_W / 2.f;
            const float baseY = 220.f;
            const float spacing = 70.f;

            // Actor field  (centered, wide)
            float ax = cx - FIELD_W / 2.f, ay = baseY;
            // Year start
            float sx = cx - FIELD_W / 2.f, sy = baseY + spacing;
            // Year end
            float ex = cx + 10.f,          ey = baseY + spacing;

            auto hit = [&](float fx, float fy) {
                return mx >= fx && mx <= fx + FIELD_W &&
                       my >= fy && my <= fy + FIELD_H;
            };

            if (hit(ax, ay)) m_focusedField = 0;
            else if (mx >= sx && mx <= sx + 130.f && my >= sy && my <= sy + FIELD_H)
                m_focusedField = 1;
            else if (mx >= ex && mx <= ex + 130.f && my >= ey && my <= ey + FIELD_H)
                m_focusedField = 2;

            // Go button
            float btnX = cx - 70.f, btnY = baseY + spacing * 2.f + 20.f;
            if (mx >= btnX && mx <= btnX + 140.f && my >= btnY && my <= btnY + 46.f) {
                // Validate
                if (m_actorBuf.empty()) {
                    m_errorMsg = "Please enter an actor name.";
                } else if (m_yearStartBuf.empty() || m_yearEndBuf.empty()) {
                    m_errorMsg = "Please enter both year fields.";
                } else {
                    try {
                        int ys = std::stoi(m_yearStartBuf);
                        int ye = std::stoi(m_yearEndBuf);
                        if (ys > ye) m_errorMsg = "Year start must be <= year end.";
                        else { m_errorMsg = ""; m_submitted = true; }
                    } catch (...) {
                        m_errorMsg = "Years must be valid integers.";
                    }
                }
            }
        }
    }

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Tab) {
            m_focusedField = (m_focusedField + 1) % 3;
        }
        if (kp->code == sf::Keyboard::Key::Enter) {
            // same as clicking Go
            sf::Event::MouseButtonPressed fake{};
            fake.button = sf::Mouse::Button::Left;
            float cx   = WIN_W / 2.f;
            fake.position = {static_cast<int>(cx), static_cast<int>(220.f + 70.f * 2.f + 23.f)};
            // Re-trigger via direct logic
            if (m_actorBuf.empty()) m_errorMsg = "Please enter an actor name.";
            else if (m_yearStartBuf.empty() || m_yearEndBuf.empty()) m_errorMsg = "Please enter both year fields.";
            else {
                try {
                    int ys = std::stoi(m_yearStartBuf);
                    int ye = std::stoi(m_yearEndBuf);
                    if (ys > ye) m_errorMsg = "Year start must be <= year end.";
                    else { m_errorMsg = ""; m_submitted = true; }
                } catch (...) { m_errorMsg = "Years must be valid integers."; }
            }
        }
        if (kp->code == sf::Keyboard::Key::Backspace) {
            auto& buf = m_focusedField == 0 ? m_actorBuf :
                        m_focusedField == 1 ? m_yearStartBuf : m_yearEndBuf;
            if (!buf.empty()) buf.pop_back();
        }
    }

    // Text entered
    if (const auto* te = event.getIf<sf::Event::TextEntered>()) {
        char32_t c = te->unicode;
        if (c >= 32 && c != 127) { // printable, not DEL
            auto& buf = m_focusedField == 0 ? m_actorBuf :
                        m_focusedField == 1 ? m_yearStartBuf : m_yearEndBuf;
            // Year fields: digits only
            if (m_focusedField != 0 && (c < '0' || c > '9')) return;
            if (buf.size() < 60) buf += static_cast<char>(c);
        }
    }
}

//draw
void MainWindow::draw(sf::RenderWindow& window)
{
    const float cx = WIN_W / 2.f;

    // Background gradient-ish via two overlapping rects
    drawCard(window, 0, 0, WIN_W, WIN_H, BG_COLOR);
    sf::RectangleShape glow({WIN_W, 200.f});
    glow.setPosition({0, 0});
    glow.setFillColor({120, 90, 30, 30});
    window.draw(glow);

    // Center card
    drawCard(window, cx - 310.f, 100.f, 620.f, 390.f, PANEL_COLOR, ACCENT_DIM, 1.f);

    // Title
    sf::Text title(m_font, "Welcome to CinaSearch", 28);
    title.setFillColor(ACCENT);
    title.setStyle(sf::Text::Bold);
    title.setPosition({cx - title.getLocalBounds().size.x / 2.f, 118.f});
    window.draw(title);

    sf::Text sub(m_font, "Enter actor and year range to query the database", 13);
    sub.setFillColor(TEXT_SECONDARY);
    sub.setPosition({cx - sub.getLocalBounds().size.x / 2.f, 156.f});
    window.draw(sub);

    // Fields
    const float baseY   = 220.f;
    const float spacing = 70.f;

    // Helper lambda
    auto drawField = [&](float fx, float fy, float fw, const std::string& label,
                         const std::string& buf, bool focused, const std::string& placeholder)
    {
        sf::Color bg = focused ? FIELD_ACTIVE : FIELD_BG;
        sf::Color border = focused ? ACCENT : sf::Color{70, 60, 50, 255};
        drawCard(window, fx, fy, fw, FIELD_H, bg, border, focused ? 2.f : 1.f);

        sf::Text lbl(m_font, label, 11);
        lbl.setFillColor(focused ? ACCENT : TEXT_SECONDARY);
        lbl.setPosition({fx, fy - 18.f});
        window.draw(lbl);

        if (buf.empty() && !focused) {
            // Draw placeholder
            sf::Text ph(m_font, placeholder, 14);
            ph.setFillColor({120, 105, 88, 255});
            ph.setPosition({fx + 10.f, fy + 13.f});
            window.draw(ph);
        } else {
            std::string display = buf;
            if (focused) display += "|";
            sf::Text txt(m_font, display, 15);
            txt.setFillColor(TEXT_PRIMARY);
            txt.setPosition({fx + 10.f, fy + 13.f});
            window.draw(txt);
        }
    };

    // Actor (full width)
    drawField(cx - FIELD_W / 2.f, baseY, FIELD_W, "ACTOR NAME",
              m_actorBuf, m_focusedField == 0, "Insert actor name");

    // Year start / end side by side
    float halfW = (FIELD_W - 10.f) / 2.f;
    drawField(cx - FIELD_W / 2.f, baseY + spacing, halfW, "YEAR START",
              m_yearStartBuf, m_focusedField == 1, "Year start");
    drawField(cx + 10.f,          baseY + spacing, halfW, "YEAR END",
              m_yearEndBuf,   m_focusedField == 2, "Year end");

    // Start button
    float btnX = cx - 70.f, btnY = baseY + spacing * 2.f + 20.f;
    sf::Vector2i mp = sf::Mouse::getPosition(window);
    bool hovering = mp.x >= btnX && mp.x <= btnX + 140 &&
                    mp.y >= btnY && mp.y <= btnY + 46;

    drawCard(window, btnX, btnY, 140.f, 46.f,
             hovering ? BTN_HOVER : BTN_NORMAL,
             sf::Color::Transparent, 0.f);

    sf::Text btnTxt(m_font, "START", 18);
    btnTxt.setStyle(sf::Text::Bold);
    btnTxt.setFillColor({28, 20, 12, 255});
    btnTxt.setPosition({btnX + 70.f - btnTxt.getLocalBounds().size.x / 2.f,
                        btnY + 13.f});
    window.draw(btnTxt);

    // Error message
    if (!m_errorMsg.empty()) {
        sf::Text err(m_font, m_errorMsg, 13);
        err.setFillColor(ERROR_COLOR);
        err.setPosition({cx - err.getLocalBounds().size.x / 2.f, btnY + 58.f});
        window.draw(err);
    }

    //Footer hint
    sf::Text hint(m_font, "Tab to switch fields   Enter to submit", 12);
    hint.setFillColor({95, 82, 66, 255});
    hint.setPosition({cx - hint.getLocalBounds().size.x / 2.f, WIN_H - 30.f});
    window.draw(hint);
}
