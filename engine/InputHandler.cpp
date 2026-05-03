#include "InputHandler.h"

InputHandler& InputHandler::getInstance()
{
	static InputHandler instance;
	return instance;
}

bool InputHandler::isKeyPressed(char key) const
{
	auto it = currentKeys.find(key);
	if (it != currentKeys.end())
		return it->second;
	else
		return false;
}

bool InputHandler::isKeyJustPressed(char key) const
{
	bool current = isKeyPressed(key);

	auto it = previousKeys.find(key);
	bool previous;
	if (it != previousKeys.end())
		previous = it->second;
	else
		previous = false;

	return current && !previous;
}

void InputHandler::setKey(char key, bool isPressed)
{
	currentKeys[key] = isPressed;
}

char InputHandler::getCharKeyFromSFMLKey(sf::Keyboard::Key key)
{
	switch (key)
	{
		case sf::Keyboard::W: 
			return 'W';
		case sf::Keyboard::A:
			return 'A';
		case sf::Keyboard::S:
			return 'S';
		case sf::Keyboard::D:
			return 'D';
		case sf::Keyboard::P:
			return 'P';
		case sf::Keyboard::R:
			return 'R';
		default:
			return '\0';
	}
}

void InputHandler::processEvent(const sf::Event& event)
{
	if (event.type == sf::Event::KeyPressed)
	{
		char c = getCharKeyFromSFMLKey(event.key.code);
		if (c != '\0')
			setKey(c, true);
	}
	else if (event.type == sf::Event::KeyReleased)
	{
		char c = getCharKeyFromSFMLKey(event.key.code);
		if (c != '\0')
			setKey(c, false);
	}
}

void InputHandler::endFrame()
{
	previousKeys = currentKeys;
}