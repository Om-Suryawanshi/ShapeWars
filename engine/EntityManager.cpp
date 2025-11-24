#include "EntityManager.h"


EntityManager::EntityManager()
	:nextId(0)
{

}

EntityManager& EntityManager::getInstance()
{
	static EntityManager instance;
	return instance;
}

void EntityManager::markForRemoval(int id)
{
	entToRemove.push_back(id);
}

void EntityManager::destroyEnt()
{
	for (int id : entToRemove)
	{
		entities.erase(id);
	}
	entToRemove.clear();
}

void EntityManager::update(float deltaTime)
{
	for (auto& [id, ent] : entities)
	{
		if (ent) ent->update(deltaTime);
		if (!ent->getisAlive())
		{
			markForRemoval(id);
		}
	}

	destroyEnt();
}

void EntityManager::pauseEnt()
{
	for (auto& [id, ent] : entities)
	{
		ent->pause();
	}
}

void EntityManager::draw(sf::RenderWindow& window)
{
	for (auto& [id, ent] : entities)
	{
		if (ent) ent->draw(window);
	}
}

int EntityManager::countByType(EntityType type)
{
	int count = 0;
	for (auto& [id, e] : entities)
	{
		if (e->getisAlive() && e->getType() == type)
			++count;
	}
	return count;
}

std::shared_ptr<entity> EntityManager::getEnt(int id)
{
	auto ent = entities.find(id);
	return ent != entities.end() ? ent->second : nullptr;
}

std::shared_ptr<entity> EntityManager::getPlayer()
{
	for (auto& [id, ent] : entities)
	{
		if (ent->getType() == EntityType::Player)
		{
			return ent;
		}
	}
	return nullptr;
}

bool EntityManager::playerExists()
{
	for (auto& ent : getByType(EntityType::Player))
	{
		if (ent && ent->getisAlive()) return true;
	}
	return false;
}

std::vector<entity*> EntityManager::getByType(EntityType type)
{
	std::vector<entity*> entityList;
	for (auto& [id, ent] : entities)
	{
		if (ent->getType() == type)
		{
			entityList.push_back(ent.get());
		}
	}
	return entityList;
}

const std::unordered_map<int, std::shared_ptr<entity>>& EntityManager::getAllEnt() const
{
	return entities;
}


// Very risky do not use allways causes nullptr deref
void EntityManager::clearAll()
{
	entities.clear();
	entToRemove.clear();
}