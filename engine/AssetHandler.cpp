#include "AssetHandler.h"

AssetHandler& AssetHandler::getInstance()
{
	static AssetHandler instance;
	return instance;
}

sf::Texture& AssetHandler::getTexture(const std::string& name)
{
	return textures.at(name);
}

sf::Font& AssetHandler::getFont(const std::string& name)
{
	return fonts.at(name);
}

void AssetHandler::loadTexture(const std::string& name, const std::string& path)
{
	if (textures.find(name) != textures.end()) return;
	sf::Texture texture;
	if (!texture.loadFromFile(path))
	{
		throw std::runtime_error("Texture Path Not Found");
	}
	textures[name] = std::move(texture);
	//std::cout << "Loaded Texture: " << name << " from " << path << std::endl;
}

void AssetHandler::loadFont(const std::string& name, const std::string& path)
{
	if (fonts.find(name) != fonts.end()) return;
	sf::Font font;
	if (!font.loadFromFile(path))
	{
		throw std::runtime_error("Font Path not found");
	}
	fonts[name] = std::move(font);
}

void AssetHandler::unloadTexture(const std::string& name)
{
	textures.erase(name);
}

void AssetHandler::unloadFont(const std::string& name)
{
	fonts.erase(name);
}