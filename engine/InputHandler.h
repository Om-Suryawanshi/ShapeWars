#pragma once
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <unordered_map>

enum class MouseButton {
	Left,
	Right,
	Middle,
	Unknown
};

class InputHandler
{
public:
	static InputHandler& getInstance();

	bool isKeyPressed(char key) const;
	bool isKeyJustPressed(char key) const;

	void setKey(char key, bool isPressed);

	bool isMouseButtonPressed(MouseButton button) const;
	bool isMouseButtonJustPressed(MouseButton button) const;
	sf::Vector2f getMousePosition() const;

	void setMouseButton(MouseButton button, bool isPressed);
	void setMousePosition(sf::Vector2f pos);

	void processEvent(const sf::Event& event);
	void updateHardwareMouse(const sf::RenderWindow& window);
	void endFrame();

private:
	InputHandler() = default;
	// Keyboard Maps
	std::unordered_map<char, bool> currentKeys;
	std::unordered_map<char, bool> previousKeys;
	std::unordered_map<char, bool> keysJustPressedTracker;

	// Mouse Maps
	std::unordered_map<MouseButton, bool> currentMouse;
	std::unordered_map<MouseButton, bool> previousMouse;
	std::unordered_map<MouseButton, bool> mouseJustPressedTracker;
	sf::Vector2f targetMousePos;

	char getCharKeyFromSFMLKey(sf::Keyboard::Key key);
	MouseButton getMouseButtonFromSFML(sf::Mouse::Button button);
};

