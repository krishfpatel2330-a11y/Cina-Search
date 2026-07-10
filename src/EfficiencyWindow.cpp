#include "../include/EfficiencyWindow.h"
#include <SFML/Window/Event.hpp>
#include <filesystem>
#include <algorithm>
#include <string>
#include <cmath>

static const sf::Color EW_BG        {18,  18,  28,  255};
static const sf::Color EW_PANEL     {28,  28,  45,  255};
static const sf::Color EW_ACCENT    {72,  149, 239, 255};
static const sf::Color EW_ACCENT2   {80,  220, 160, 255};   // teal for B+ bar
static const sf::Color EW_DIM       {40,  90,  160, 255};
static const sf::Color EW_TXT_P     {230, 230, 245, 255};
static const sf::Color EW_TXT_S     {130, 130, 160, 255};
static const sf::Color EW_TRACK     {40,  40,  65,  255};

static void ewCard(sf::RenderWindow& w, float x, float y, float bw, float bh,
                   sf::Color fill, sf::Color outline = sf::Color::Transparent, float t = 0.f)
{
    sf::RectangleShape r({bw, bh});
    r.setPosition({x, y});
    r.setFillColor(fill);
    r.setOutlineColor(outline);
    r.setOutlineThickness(t);
    w.draw(r);
}

// run
bool EfficiencyWindow::run(sf::RenderWindow& window, float bTimeMs, float bpTimeMs)
{
    std::filesystem::path fontPath = std::filesystem::path("assets") / "fonts" / "arial.ttf";
    m_font.openFromFile(fontPath.string());

    // the longer time = WIN animation duration (in seconds, 1 ms → 1 s, cap 30 s)
    float maxMs  = std::max(bTimeMs, bpTimeMs);
    if (maxMs < 1.f) maxMs = 1.f;  // avoid division by zero

    // Each bar's animation duration (seconds)
    float bDur  = (bTimeMs  / maxMs) * std::max(maxMs / 1000.f, 3.f); // at least 3 s total
    float bpDur = (bpTimeMs / maxMs) * std::max(maxMs / 1000.f, 3.f);

    sf::Clock clock;
    bool done = false;

    while (window.isOpen() && !done) {
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) { window.close(); return false; }
        }

        float t = clock.getElapsedTime().asSeconds();
        float bFill  = std::min(1.f, bDur  > 0.f ? t / bDur  : 1.f);
        float bpFill = std::min(1.f, bpDur > 0.f ? t / bpDur : 1.f);

        if (bFill >= 1.f && bpFill >= 1.f) {
            // Hold for 0.6 s so both full bars are visible, then advance
            done = (t - std::max(bDur, bpDur)) > 0.6f;
        }

        window.clear(EW_BG);
        draw(window, bFill, bpFill, bTimeMs, bpTimeMs);
        window.display();
    }

    return true;
}

// draw
void EfficiencyWindow::draw(sf::RenderWindow& window,
                            float bFill, float bpFill,
                            float bTimeMs, float bpTimeMs)
{
    const float cx = WIN_W / 2.f;

    // Background glow
    sf::RectangleShape glow({WIN_W, 200.f});
    glow.setPosition({0, 0});
    glow.setFillColor({30, 60, 140, 25});
    window.draw(glow);

    // Center panel
    ewCard(window, cx - 340.f, 80.f, 680.f, 420.f, EW_PANEL, EW_DIM, 1.f);

    // Title
    sf::Text title(m_font, "Efficiency Analysis", 26);
    title.setFillColor(EW_ACCENT);
    title.setStyle(sf::Text::Bold);
    title.setPosition({cx - title.getLocalBounds().size.x / 2.f, 96.f});
    window.draw(title);

    sf::Text sub(m_font, "Comparing insertion time for B Tree vs B+ Tree", 13);
    sub.setFillColor(EW_TXT_S);
    sub.setPosition({cx - sub.getLocalBounds().size.x / 2.f, 132.f});
    window.draw(sub);

    // Bar helper
    auto drawBar = [&](float barY, const std::string& label, float fill,
                       float timeMs, sf::Color barColor)
    {
        float trackX = cx - BAR_W / 2.f;

        // Label
        sf::Text lbl(m_font, label, 15);
        lbl.setStyle(sf::Text::Bold);
        lbl.setFillColor(barColor);
        lbl.setPosition({trackX, barY - 30.f});
        window.draw(lbl);

        // Track
        ewCard(window, trackX, barY, BAR_W, BAR_H, EW_TRACK, barColor, 1.f);

        // Fill
        float fillW = BAR_W * fill;
        if (fillW > 2.f) {
            ewCard(window, trackX, barY, fillW, BAR_H, barColor);
            // Animated shimmer stripe
            sf::RectangleShape shimmer({6.f, BAR_H});
            shimmer.setPosition({trackX + fillW - 6.f, barY});
            shimmer.setFillColor({255, 255, 255, 60});
            window.draw(shimmer);
        }

        // Percentage text inside bar (or to the right if small)
        int pct = static_cast<int>(fill * 100.f);
        std::string pctStr = std::to_string(pct) + "%";
        sf::Text pctTxt(m_font, pctStr, 13);
        pctTxt.setFillColor({10, 10, 20, 255});
        float pctX = trackX + fillW - pctTxt.getLocalBounds().size.x - 8.f;
        if (pctX < trackX + 4.f) {
            // put it outside the bar in normal color
            pctTxt.setFillColor(EW_TXT_P);
            pctX = trackX + fillW + 6.f;
        }
        pctTxt.setPosition({pctX, barY + 13.f});
        window.draw(pctTxt);

    };

    drawBar(220.f, "B Tree",  bFill,  bTimeMs,  EW_ACCENT);
    drawBar(360.f, "B+ Tree", bpFill, bpTimeMs, EW_ACCENT2);

    // Footer
    sf::Text hint(m_font, "Please wait — populating data structures...", 13);
    hint.setFillColor({70, 70, 100, 255});
    hint.setPosition({cx - hint.getLocalBounds().size.x / 2.f, WIN_H - 30.f});
    window.draw(hint);
}