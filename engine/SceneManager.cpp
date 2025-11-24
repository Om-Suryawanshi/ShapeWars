#include "SceneManager.h"

SceneManager::SceneManager()
{
	window = nullptr;
}

SceneManager& SceneManager::getInstance()
{
	static SceneManager instance;
	return instance;
}

void SceneManager::pushScene(std::unique_ptr<Scene> scene)
{
	scenes.push(std::move(scene));
}

void SceneManager::popScene()
{
	if (!scenes.empty()) scenes.pop();
}

void SceneManager::changeScene(std::unique_ptr<Scene> scene)
{
	popScene();
	scenes.push(std::move(scene));
}

void SceneManager::mainMenu()
{
	changeScene(std::make_unique<MainScene>());
}

void SceneManager::COOPMenu()
{

}
/*
void SceneManager::SinglePlayerMenu()
{
	changeScene(std::make_unique<SinglePlayerScene>());
}
*/
Scene* SceneManager::getCurrentScene()
{
	return scenes.empty() ? nullptr : scenes.top().get();
}

bool SceneManager::isEmpty() const
{
	return scenes.empty();
}

void SceneManager::loadScene(SceneID id)
{
	switch (id)
	{
	case SceneID::MainMenu:
		//m_currentScene = std::make_unique<>();
		break;
	case SceneID::COOP:
		break;
	case SceneID::SinglePLayer:
		break;
	}
}

void SceneManager::setRenderWindow(sf::RenderWindow* win)
{
	window = win;
}

sf::RenderWindow* SceneManager::getRenderWindow() const
{
	return window;
}

void SceneManager::handleEvent(const sf::Event& event)
{
	if (!scenes.empty())
	{
		scenes.top()->handleEvent(event);
	}
}

void SceneManager::update(float deltaTime)
{
	if (!scenes.empty())
	{
		scenes.top()->update(deltaTime);
	}
}

void SceneManager::render(sf::RenderWindow& window)
{
	if (!scenes.empty())
	{
		scenes.top()->render(window);
	}
}