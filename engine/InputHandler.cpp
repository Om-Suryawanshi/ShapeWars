#include "InputHandler.h"

InputHandler& InputHandler::getInstance()
{
	static InputHandler instance;
	return instance;
}

// Keyboard
bool InputHandler::isKeyPressed(char key) const
{
	auto it = currentKeys.find(key);
	if (it != currentKeys.end())
		return it->second;
	return false;
}

bool InputHandler::isKeyJustPressed(char key) const
{
	auto it = keysJustPressedTracker.find(key);
	if (it != keysJustPressedTracker.end())
		return it->second;
	return false;
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
		case sf::Keyboard::Escape:
			return 'Esc';
		default:
			return '\0';
	}
}

// Mouse
bool InputHandler::isMouseButtonPressed(MouseButton button) const
{
	auto it = currentMouse.find(button);
	if (it != currentMouse.end())
		return it->second;
	return false;
}

bool InputHandler::isMouseButtonJustPressed(MouseButton button) const
{
	auto it = mouseJustPressedTracker.find(button);
	if (it != mouseJustPressedTracker.end())
		return it->second;
	return false;
}

sf::Vector2f InputHandler::getMousePosition() const
{
	return targetMousePos;
}

void InputHandler::setMousePosition(sf::Vector2f pos)
{
	targetMousePos = pos;
}

void InputHandler::setMouseButton(MouseButton button, bool isPressed)
{
	currentMouse[button] = isPressed;
}

MouseButton InputHandler::getMouseButtonFromSFML(sf::Mouse::Button button)
{
	switch (button)
	{
	case sf::Mouse::Left:
		return MouseButton::Left;
	case sf::Mouse::Right:
		return MouseButton::Right;
	case sf::Mouse::Middle:
		return MouseButton::Middle;
	default:
		return MouseButton::Unknown;
	}
}

void InputHandler::processEvent(const sf::Event& event)
{
	// Keyboard
	if (event.type == sf::Event::KeyPressed)
	{
		char c = getCharKeyFromSFMLKey(event.key.code);
		if (c != '\0')
		{
			setKey(c, true);
			keysJustPressedTracker[c] = true;
		}

	}
	else if (event.type == sf::Event::KeyReleased)
	{
		char c = getCharKeyFromSFMLKey(event.key.code);
		if (c != '\0')
			setKey(c, false);
	}

	// Mouse
	else if (event.type == sf::Event::MouseButtonPressed)
	{
		MouseButton btn = getMouseButtonFromSFML(event.mouseButton.button);
		if (btn != MouseButton::Unknown)
		{
			setMouseButton(btn, true);
			mouseJustPressedTracker[btn] = true;
		}
	}
	else if (event.type == sf::Event::MouseButtonReleased)
	{
		MouseButton btn = getMouseButtonFromSFML(event.mouseButton.button);
		if (btn != MouseButton::Unknown)
			setMouseButton(btn, false);
	}
}

void InputHandler::updateHardwareMouse(const sf::RenderWindow& window)
{
	sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
	targetMousePos = window.mapPixelToCoords(pixelPos);
}

void InputHandler::endFrame()
{
	keysJustPressedTracker.clear();
	mouseJustPressedTracker.clear();
	previousKeys = currentKeys;
	previousMouse = currentMouse;
}