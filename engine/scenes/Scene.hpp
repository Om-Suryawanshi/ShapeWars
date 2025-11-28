#pragma once
#pragma once
#include <SFML/Graphics.hpp>
#include <string>

struct ModernButton {
    sf::RectangleShape shape;
    sf::Text text;
    sf::Color idleColor;
    sf::Color hoverColor;
    sf::Color activeColor;
    bool isHovered = false;

    void init(const sf::Font& font, std::string label, sf::Vector2f size, sf::Vector2f pos) {
        // Style Configuration
        idleColor = sf::Color(30, 30, 40, 200);   // Dark semi-transparent
        hoverColor = sf::Color(50, 50, 70, 255);  // Lighter when hovered
        activeColor = sf::Color(0, 255, 255);     // Cyan outline

        // Shape Setup
        shape.setSize(size);
        shape.setOrigin(size.x / 2.f, size.y / 2.f); // Center origin for easy placement
        shape.setPosition(pos);
        shape.setFillColor(idleColor);
        shape.setOutlineThickness(2);
        shape.setOutlineColor(sf::Color(100, 100, 100));

        // Text Setup
        text.setFont(font);
        text.setString(label);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::White);
        centerText();
    }

    // Fixes the clipping/alignment issue
    void centerText() {
        sf::FloatRect textBounds = text.getLocalBounds();
        // Set origin to the actual visual center of the text
        text.setOrigin(textBounds.left + textBounds.width / 2.0f,
            textBounds.top + textBounds.height / 2.0f);
        // Snap to shape position
        text.setPosition(shape.getPosition());
    }

    bool update(const sf::Vector2f& mousePos) {
        isHovered = shape.getGlobalBounds().contains(mousePos);

        if (isHovered) {
            shape.setFillColor(hoverColor);
            shape.setOutlineColor(activeColor); // Glow effect
            shape.setScale(1.05f, 1.05f);       // Pop effect
        }
        else {
            shape.setFillColor(idleColor);
            shape.setOutlineColor(sf::Color(100, 100, 100));
            shape.setScale(1.0f, 1.0f);
        }
        return isHovered; // Return true if hovered (useful for cursors)
    }

    void render(sf::RenderWindow& window) {
        window.draw(shape);
        window.draw(text);
    }

    bool isClicked(const sf::Event& event, const sf::Vector2f& mousePos) {
        return (isHovered && event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left);
    }

    // Allow updating text dynamically (for status buttons)
    void setString(const std::string& s) {
        text.setString(s);
        centerText();
    }
};

class Scene {
public:
    virtual ~Scene() = default;

    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
};
