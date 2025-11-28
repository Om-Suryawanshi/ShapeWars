#include "RewindSystem.h"
#include <iostream>
#include <unordered_set>

RewindSystem::RewindSystem(EntityManager& manager, int maxFrames)
    : entManager(EntityManager::getInstance()), maxFrameCount(maxFrames), m_rewinding(false), rewindIndex(0),
    m_writeIndex(0), m_recordedCount(0) {

    history.resize(maxFrames);
}

void RewindSystem::update() {
    if (m_rewinding) {
        if (m_rewindClock.getElapsedTime().asMilliseconds() >= 10) {
            m_rewindClock.restart();

            if (m_recordedCount > 0) {

                if (m_writeIndex == 0) {
                    m_writeIndex = maxFrameCount - 1;
                }
                else {
                    m_writeIndex--;
                }

                loadSnapShot(history[m_writeIndex]);

                m_recordedCount--;

                rewindIndex--;
            }
            else {
                std::cerr << "[RewindSystem] Rewind reached beginning of time.\n";
                m_rewinding = false;
            }
        }
        return;
    }

    if (!isPaused) {
        const auto& entities = entManager.getAllEnt();
        FrameSnapshot& snapshot = history[m_writeIndex];

        snapshot.clear();

        for (const auto& [id, ent] : entities) {
            if (ent && ent->getisAlive()) {
                snapshot.push_back(SnapshotEntityData{
                    id,
                    ent->getType(),
                    ent->getPos(),
                    ent->getVelocity(),
                    ent->getSize(),
                    ent->getVertices(),
                    ent->getLifetime(),
                    ent->getAge(),
                    ent->getisAlive()
                    });
            }
        }

        if (!snapshot.empty()) {
            m_writeIndex = (m_writeIndex + 1) % maxFrameCount;

            if (m_recordedCount < maxFrameCount) {
                m_recordedCount++;
            }
        }
    }
}
void RewindSystem::triggerRewind() {
    if (m_recordedCount > 0) {
        rewindIndex = static_cast<int>(m_recordedCount) - 1;
        m_rewinding = true;
        m_rewindClock.restart();
    }
}

bool RewindSystem::isRewinding() const {
    return m_rewinding;
}

//void RewindSystem::loadSnapShot(const FrameSnapshot& snapshot) {
//
//    auto& currentEntities = entManager.getAllEnt();
//    std::unordered_set<int> processedIDs;
//
//    for (const auto& snap : snapshot) {
//        processedIDs.insert(snap.id);
//
//        auto it = currentEntities.find(snap.id);
//
//        std::shared_ptr<entity> ent;
//
//        if (it != currentEntities.end()) {
//            ent = it->second;
//        }
//        else {
//            switch (snap.type) {
//            case EntityType::Player:
//                ent = entManager.createEntity<Player>();
//                break;
//            case EntityType::Enemy:
//            case EntityType::MiniEnemy: {
//                float speed = std::sqrt(snap.velocity.x * snap.velocity.x + snap.velocity.y * snap.velocity.y);
//                ent = entManager.createEntity<Enemy>(speed, snap.radius, static_cast<float>(snap.vertices), snap.type);
//                break;
//            }
//            case EntityType::Bullet:
//                ent = entManager.createEntity<Bullet>(snap.pos, snap.velocity);
//                break;
//            }
//            int tempId = -1;
//            for (auto& [id, e] : currentEntities) {
//                if (e == ent) { tempId = id; break; }
//            }
//
//            if (tempId != -1) {
//                entManager.remapEntity(tempId, snap.id);
//            }
//        }
//
//        // Apply State Data
//        if (ent) {
//            ent->setPos(snap.pos);
//            ent->setVelocity(snap.velocity);
//            ent->setLifetime(snap.lifetime);
//            ent->setAge(snap.age);
//        }
//    }
//
//    for (auto it = currentEntities.begin(); it != currentEntities.end(); ) {
//        int currentId = it->first;
//        it++;
//
//        if (processedIDs.find(currentId) == processedIDs.end()) {
//            entManager.markForRemoval(currentId);
//        }
//    }
//
//    entManager.destroyEnt();
//}

void RewindSystem::loadSnapShot(const FrameSnapshot& snapshot) {

    auto& currentEntities = entManager.getAllEnt();
    std::unordered_set<int> processedIDs;

    for (const auto& snap : snapshot) {
        processedIDs.insert(snap.id);

        auto it = currentEntities.find(snap.id);
        std::shared_ptr<entity> ent;

        // 1. Entity Exists: Just Update it
        if (it != currentEntities.end()) {
            ent = it->second;
        }
        // 2. Entity Missing (Was dead): RESURRECT IT
        else {
            switch (snap.type) {
            case EntityType::Player:
                ent = entManager.createEntityWithId<Player>(snap.id);
                break;

            case EntityType::Enemy:
            case EntityType::MiniEnemy: {
                // Reconstruct parameters
                float speed = std::sqrt(snap.velocity.x * snap.velocity.x + snap.velocity.y * snap.velocity.y);

                // Note: We pass 0.0f for angle because it might not be in snapshot, 
                // but velocity usually dictates movement anyway.
                ent = entManager.createEntityWithId<Enemy>(
                    snap.id,            // <--- FORCE THE OLD ID
                    speed,
                    snap.radius,
                    static_cast<float>(snap.vertices),
                    snap.type,
                    0.0f                // Angle (approximate)
                );
                break;
            }
            case EntityType::Bullet:
                ent = entManager.createEntityWithId<Bullet>(
                    snap.id,            // <--- FORCE THE OLD ID
                    snap.pos,
                    snap.velocity
                );
                break;
            }

            // REMOVED: remapEntity logic. 
            // We inserted it correctly the first time!
        }

        // Apply State Data
        if (ent) {
            ent->setPos(snap.pos);
            ent->setVelocity(snap.velocity);
            ent->setLifetime(snap.lifetime);
            ent->setAge(snap.age);

            // Optional: If you store color in snapshot, apply it here
            // ent->setColor(...) 
        }
    }

    // Clean up entities that exist now but didn't exist in the past
    for (auto it = currentEntities.begin(); it != currentEntities.end(); ) {
        int currentId = it->first;
        it++; // Increment before potential deletion

        if (processedIDs.find(currentId) == processedIDs.end()) {
            entManager.markForRemoval(currentId);
        }
    }

    entManager.destroyEnt();
}

void RewindSystem::clearHistory()
{
    m_writeIndex = 0;
    m_recordedCount = 0;
    rewindIndex = 0;
    m_rewinding = false;
    std::cerr << "[RewindSystem] History cleared manually.\n";
}

void RewindSystem::pauseCapture()
{
    isPaused = !isPaused;
}