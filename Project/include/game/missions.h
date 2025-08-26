#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>

enum class MissionType {
    REACH_SPEED,           // Reach X Km/h
    CONSECUTIVE_RINGS,     // Take X rings consecutively  
    REACH_SCORE,          // Reach X score
    DESTROY_MOVING_ASTEROIDS, // Destroy X moving asteroids
    FLY_WITHOUT_DAMAGE,   // Fly X minutes without taking damage
    COLLECT_RINGS,        // Collect X rings total
    DESTROY_ASTEROIDS,    // Destroy X asteroids total
    SURVIVE_TIME          // Survive for X minutes
};

enum class MissionStatus {
    NOT_STARTED,
    IN_PROGRESS,
    COMPLETED
};

struct Mission {
    int id;
    MissionType type;
    std::string title;
    std::string description;
    int targetValue;      // Target to achieve (speed, rings, score, etc.)
    int currentProgress;  // Current progress towards target
    MissionStatus status;
    std::string reward;   // Description of reward (future use)
    
    // Progress tracking helpers
    float getProgressPercent() const {
        if (targetValue == 0) return 0.0f;
        return std::min(100.0f, (static_cast<float>(currentProgress) / targetValue) * 100.0f);
    }
    
    bool isCompleted() const {
        return currentProgress >= targetValue;
    }
};

class DailyMissions {
public:
    DailyMissions();
    ~DailyMissions();
    
    // Mission management
    std::vector<Mission> getTodaysMissions();
    void updateMissionProgress(MissionType type, int value);
    void checkMissionCompletion();
    
    // Daily reset and generation
    void generateDailyMissions();
    bool shouldGenerateNewMissions();
    
    // Progress tracking from game events
    void recordSpeedReached(int speed);
    void recordConsecutiveRings(int consecutive);
    void recordScoreReached(int score);
    void recordMovingAsteroidDestroyed();
    void recordAsteroidDestroyed();
    void recordRingCollected();
    void recordDamage(); // Resets damage-free time
    void updatePlayTime(double deltaTime);
    
    // Session tracking for damage-free flying
    void startSession();
    void resetDamageFreeTime();
    
    // File persistence
    void saveToFile(const std::string& filename = "missions.txt");
    void loadFromFile(const std::string& filename = "missions.txt");
    
    // Getters
    const std::vector<Mission>& getCurrentMissions() const { return currentMissions; }
    int getCompletedMissionsCount() const;
    bool hasMissionsForToday() const;

private:
    std::vector<Mission> currentMissions;
    std::string currentDate;
    std::string dataDirFilePath;
    std::string missionsFilePath;
    
    // Session tracking variables
    double sessionStartTime;
    double lastDamageTime;
    double damageFreeTime;
    bool sessionActive;
    
    // Mission generation
    Mission generateMission(MissionType type, int difficultyLevel);
    std::string getCurrentDateString() const;
    void createMissionDescriptions(Mission& mission);
    int calculateTargetValue(MissionType type, int difficultyLevel);
    
    // Mission templates
    std::vector<std::string> getSpeedMissionTitles() const;
    std::vector<std::string> getRingMissionTitles() const;
    std::vector<std::string> getScoreMissionTitles() const;
    std::vector<std::string> getAsteroidMissionTitles() const;
    std::vector<std::string> getSurvivalMissionTitles() const;
};