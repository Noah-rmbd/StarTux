#include "statistics.h"
#include <algorithm>
#include <sstream>

GameStatistics::GameStatistics() {
    // Set default file path (can be customized)
    statsFilePath = "statistics.txt";
    
    // Load existing statistics on startup
    loadFromFile(statsFilePath);
}

GameStatistics::~GameStatistics() {
    // Statistics are saved explicitly when needed, not automatically in destructor
    // to avoid excessive file I/O during gameplay
}

void GameStatistics::recordAsteroidDestroyed(bool isMoving) {
    currentSession.asteroidsDestroyed++;
    if (isMoving) {
        currentSession.movingAsteroidsDestroyed++;
    }
}

void GameStatistics::recordRingTaken() {
    currentSession.ringsTaken++;
    currentSession.currentConsecutiveRings++;
    
    // Update best consecutive rings for this session
    currentSession.consecutiveRings = std::max(currentSession.consecutiveRings, 
                                              currentSession.currentConsecutiveRings);
}

void GameStatistics::recordRingMissed() {
    // Reset consecutive ring counter (missed a ring)
    currentSession.currentConsecutiveRings = 0;
}

void GameStatistics::recordSpeedReached(int speed) {
    currentSession.maxSpeedReached = std::max(currentSession.maxSpeedReached, speed);
}

void GameStatistics::recordGameEnd(int finalScore, double gameTime) {
    currentSession.finalScore = finalScore;
    currentSession.gameTime = gameTime;
    
    // Update lifetime statistics
    updateLifetimeStats();
    
    // Save to file immediately after game ends
    saveToFile(statsFilePath);
}

void GameStatistics::resetSession() {
    currentSession = SessionStats(); // Reset to default values
}

void GameStatistics::updateLifetimeStats() {
    // Add current session to lifetime stats
    lifetime.totalAsteroidsDestroyed += currentSession.asteroidsDestroyed;
    lifetime.totalMovingAsteroidsDestroyed += currentSession.movingAsteroidsDestroyed;
    lifetime.totalRingsTaken += currentSession.ringsTaken;
    lifetime.maxSpeedEverReached = std::max(lifetime.maxSpeedEverReached, 
                                           currentSession.maxSpeedReached);
    lifetime.bestConsecutiveRings = std::max(lifetime.bestConsecutiveRings, 
                                            currentSession.consecutiveRings);
    lifetime.gamesPlayed++;
    lifetime.highScore = std::max(lifetime.highScore, currentSession.finalScore);
    lifetime.totalPlayTime += currentSession.gameTime;
}

void GameStatistics::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to save statistics to " << filename << std::endl;
        return;
    }
    
    // Save lifetime statistics in simple format
    file << "# StarTux Game Statistics" << std::endl;
    file << "totalAsteroidsDestroyed=" << lifetime.totalAsteroidsDestroyed << std::endl;
    file << "totalMovingAsteroidsDestroyed=" << lifetime.totalMovingAsteroidsDestroyed << std::endl;
    file << "totalRingsTaken=" << lifetime.totalRingsTaken << std::endl;
    file << "maxSpeedEverReached=" << lifetime.maxSpeedEverReached << std::endl;
    file << "bestConsecutiveRings=" << lifetime.bestConsecutiveRings << std::endl;
    file << "gamesPlayed=" << lifetime.gamesPlayed << std::endl;
    file << "highScore=" << lifetime.highScore << std::endl;
    file << "totalPlayTime=" << lifetime.totalPlayTime << std::endl;
    
    file.close();
    
    std::cout << "Statistics saved to " << filename << std::endl;
}

void GameStatistics::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "No existing statistics file found. Starting with fresh statistics." << std::endl;
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;
        
        // Parse key=value pairs
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos) continue;
        
        std::string key = line.substr(0, equalPos);
        std::string valueStr = line.substr(equalPos + 1);
        
        try {
            if (key == "totalAsteroidsDestroyed") {
                lifetime.totalAsteroidsDestroyed = std::stoi(valueStr);
            } else if (key == "totalMovingAsteroidsDestroyed") {
                lifetime.totalMovingAsteroidsDestroyed = std::stoi(valueStr);
            } else if (key == "totalRingsTaken") {
                lifetime.totalRingsTaken = std::stoi(valueStr);
            } else if (key == "maxSpeedEverReached") {
                lifetime.maxSpeedEverReached = std::stoi(valueStr);
            } else if (key == "bestConsecutiveRings") {
                lifetime.bestConsecutiveRings = std::stoi(valueStr);
            } else if (key == "gamesPlayed") {
                lifetime.gamesPlayed = std::stoi(valueStr);
            } else if (key == "highScore") {
                lifetime.highScore = std::stoi(valueStr);
            } else if (key == "totalPlayTime") {
                lifetime.totalPlayTime = std::stod(valueStr);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing statistic: " << key << "=" << valueStr << std::endl;
        }
    }
    
    file.close();
    std::cout << "Statistics loaded from " << filename << std::endl;
}