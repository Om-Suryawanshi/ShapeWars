#pragma once
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <unordered_map>

class InputHandler
{
public:
	static InputHandler& getInstance();

	bool isKeyPressed(char key) const;
	bool isKeyJustPressed(char key) const;

	void setKey(char key, bool isPressed); // For the AI systems
	void processEvent(const sf::Event& event);
	void endFrame();
private:
	InputHandler() = default;
	std::unordered_map<char, bool> currentKeys;
	std::unordered_map<char, bool> previousKeys;

	char getCharKeyFromSFMLKey(sf::Keyboard::Key key);
};

