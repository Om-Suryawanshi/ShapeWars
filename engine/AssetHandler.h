#pragma once
#include <SFML/Graphics.hpp>
#include "config/config.hpp"

class AssetHandler
{
private:
	std::map<std::string, sf::Texture> textures;
	std::map<std::string, sf::Font> fonts;
	AssetHandler() {};
	AssetHandler(const AssetHandler&) = delete;
	AssetHandler& operator=(const AssetHandler&) = delete;

public:
	static AssetHandler& getInstance();
	sf::Texture& getTexture(const std::string& name);
	sf::Font& getFont(const std::string& name);
	void loadTexture(const std::string& name, const std::string& path);
	void loadFont(const std::string& name, const std::string& path);
	void unloadTexture(const std::string& name);
	void unloadFont(const std::string& name);
};