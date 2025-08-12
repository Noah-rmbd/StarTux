#ifndef STATISTICS_H
#define STATISTICS_H

#include <fstream>
#include <string>
#include <iostream>

class GameStatistics {
public:
    GameStatistics();
    ~GameStatistics();
    
    // Current game session statistics
    struct SessionStats {
        int asteroidsDestroyed = 0;
        int movingAsteroidsDestroyed = 0;
        int ringsTaken = 0;
        int maxSpeedReached = 0;
        int consecutiveRings = 0;
        int currentConsecutiveRings = 0;  // Tracks current streak
        int finalScore = 0;
        double gameTime = 0.0;
    };
    
    // Lifetime/persistent statistics
    struct LifetimeStats {
        int totalAsteroidsDestroyed = 0;
        int totalMovingAsteroidsDestroyed = 0;
        int totalRingsTaken = 0;
        int maxSpeedEverReached = 0;
        int bestConsecutiveRings = 0;
        int gamesPlayed = 0;
        int highScore = 0;
        double totalPlayTime = 0.0;
    };
    
    SessionStats currentSession;
    LifetimeStats lifetime;
    
    // Session tracking methods
    void recordAsteroidDestroyed(bool isMoving = false);
    void recordRingTaken();
    void recordRingMissed(); // Breaks consecutive ring streak
    void recordSpeedReached(int speed);
    void recordGameEnd(int finalScore, double gameTime);
    
    // Session reset (for new game)
    void resetSession();
    
    // Persistence methods
    void saveToFile(const std::string& filename = "statistics.txt");
    void loadFromFile(const std::string& filename = "statistics.txt");
    
    // Getters for easy access
    int getAsteroidsDestroyed() const { return currentSession.asteroidsDestroyed; }
    int getMovingAsteroidsDestroyed() const { return currentSession.movingAsteroidsDestroyed; }
    int getRingsTaken() const { return currentSession.ringsTaken; }
    int getMaxSpeedReached() const { return currentSession.maxSpeedReached; }
    int getConsecutiveRings() const { return currentSession.consecutiveRings; }
    int getCurrentConsecutiveRings() const { return currentSession.currentConsecutiveRings; }
    
    // Lifetime getters
    int getTotalAsteroidsDestroyed() const { return lifetime.totalAsteroidsDestroyed; }
    int getTotalMovingAsteroidsDestroyed() const { return lifetime.totalMovingAsteroidsDestroyed; }
    int getTotalRingsTaken() const { return lifetime.totalRingsTaken; }
    int getMaxSpeedEverReached() const { return lifetime.maxSpeedEverReached; }
    int getBestConsecutiveRings() const { return lifetime.bestConsecutiveRings; }
    int getGamesPlayed() const { return lifetime.gamesPlayed; }
    int getHighScore() const { return lifetime.highScore; }
    double getTotalPlayTime() const { return lifetime.totalPlayTime; }

private:
    std::string statsFilePath;
    void updateLifetimeStats();
};

#endif