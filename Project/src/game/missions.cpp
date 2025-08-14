#include "missions.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

DailyMissions::DailyMissions() {
    dataDirFilePath = DATA_DIR;
    missionsFilePath = dataDirFilePath + "missions.txt";
    currentDate = getCurrentDateString();
    sessionActive = false;
    sessionStartTime = 0.0;
    lastDamageTime = 0.0;
    damageFreeTime = 0.0;
    
    // Load existing missions or generate new ones
    loadFromFile(missionsFilePath);
    
    // Generate new missions if needed (new day or no missions)
    if (shouldGenerateNewMissions()) {
        generateDailyMissions();
        saveToFile(missionsFilePath);
    }
}

DailyMissions::~DailyMissions() {
    saveToFile(missionsFilePath);
}

std::string DailyMissions::getCurrentDateString() const {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    
    std::stringstream ss;
    ss << std::setfill('0') 
       << std::setw(4) << (1900 + ltm->tm_year) << "-"
       << std::setw(2) << (1 + ltm->tm_mon) << "-"
       << std::setw(2) << ltm->tm_mday;
    
    return ss.str();
}

bool DailyMissions::shouldGenerateNewMissions() {
    std::string todaysDate = getCurrentDateString();
    return currentDate != todaysDate || currentMissions.empty();
}

void DailyMissions::generateDailyMissions() {
    currentMissions.clear();
    currentDate = getCurrentDateString();
    
    // Generate 3 random missions with increasing difficulty
    std::vector<MissionType> availableTypes = {
        MissionType::REACH_SPEED,
        MissionType::CONSECUTIVE_RINGS,
        MissionType::REACH_SCORE,
        MissionType::DESTROY_MOVING_ASTEROIDS,
        MissionType::FLY_WITHOUT_DAMAGE,
        MissionType::COLLECT_RINGS,
        MissionType::DESTROY_ASTEROIDS,
        MissionType::SURVIVE_TIME
    };
    
    // Use date as seed for consistent daily missions
    std::hash<std::string> hasher;
    size_t seed = hasher(currentDate);
    std::mt19937 rng(seed);
    
    // Shuffle available types and pick 3
    std::shuffle(availableTypes.begin(), availableTypes.end(), rng);
    
    for (int i = 0; i < 3 && i < availableTypes.size(); i++) {
        Mission mission = generateMission(availableTypes[i], i + 1); // Difficulty 1-3
        mission.id = i;
        currentMissions.push_back(mission);
    }
    
    std::cout << "Generated " << currentMissions.size() << " daily missions for " << currentDate << std::endl;
}

Mission DailyMissions::generateMission(MissionType type, int difficultyLevel) {
    Mission mission;
    mission.type = type;
    mission.currentProgress = 0;
    mission.status = MissionStatus::NOT_STARTED;
    mission.targetValue = calculateTargetValue(type, difficultyLevel);
    mission.reward = "XP + 100"; // Future use
    
    createMissionDescriptions(mission);
    
    return mission;
}

int DailyMissions::calculateTargetValue(MissionType type, int difficultyLevel) {
    switch (type) {
        case MissionType::REACH_SPEED:
            return 200 + (difficultyLevel * 100); // 300, 400, 500 Km/h
            
        case MissionType::CONSECUTIVE_RINGS:
            return 2 + difficultyLevel; // 3, 4, 5 consecutive rings
            
        case MissionType::REACH_SCORE:
            return 1000 + (difficultyLevel * 1500); // 2500, 4000, 5500 score
            
        case MissionType::DESTROY_MOVING_ASTEROIDS:
            return difficultyLevel * 2; // 2, 4, 6 moving asteroids
            
        case MissionType::FLY_WITHOUT_DAMAGE:
            return 30 + (difficultyLevel * 30); // 60, 90, 120 seconds
            
        case MissionType::COLLECT_RINGS:
            return 5 + (difficultyLevel * 5); // 10, 15, 20 rings
            
        case MissionType::DESTROY_ASTEROIDS:
            return 10 + (difficultyLevel * 10); // 20, 30, 40 asteroids
            
        case MissionType::SURVIVE_TIME:
            return 60 + (difficultyLevel * 60); // 120, 180, 240 seconds
            
        default:
            return 10;
    }
}

void DailyMissions::createMissionDescriptions(Mission& mission) {
    switch (mission.type) {
        case MissionType::REACH_SPEED:
            mission.title = "Speed Demon";
            mission.description = "Reach " + std::to_string(mission.targetValue) + " Km/h";
            break;
            
        case MissionType::CONSECUTIVE_RINGS:
            mission.title = "Ring Master";
            mission.description = "Collect " + std::to_string(mission.targetValue) + " rings in a row";
            break;
            
        case MissionType::REACH_SCORE:
            mission.title = "High Scorer";
            mission.description = "Reach a score of " + std::to_string(mission.targetValue);
            break;
            
        case MissionType::DESTROY_MOVING_ASTEROIDS:
            mission.title = "Moving Target";
            mission.description = "Destroy " + std::to_string(mission.targetValue) + " moving asteroids";
            break;
            
        case MissionType::FLY_WITHOUT_DAMAGE:
            mission.title = "Untouchable";
            mission.description = "Fly for " + std::to_string(mission.targetValue) + " seconds without damage";
            break;
            
        case MissionType::COLLECT_RINGS:
            mission.title = "Ring Collector";
            mission.description = "Collect " + std::to_string(mission.targetValue) + " rings";
            break;
            
        case MissionType::DESTROY_ASTEROIDS:
            mission.title = "Asteroid Hunter";
            mission.description = "Destroy " + std::to_string(mission.targetValue) + " asteroids";
            break;
            
        case MissionType::SURVIVE_TIME:
            mission.title = "Survivor";
            mission.description = "Survive for " + std::to_string(mission.targetValue / 60) + " minutes";
            break;
    }
}

void DailyMissions::recordSpeedReached(int speed) {
    updateMissionProgress(MissionType::REACH_SPEED, speed);
}

void DailyMissions::recordConsecutiveRings(int consecutive) {
    updateMissionProgress(MissionType::CONSECUTIVE_RINGS, consecutive);
}

void DailyMissions::recordScoreReached(int score) {
    updateMissionProgress(MissionType::REACH_SCORE, score);
}

void DailyMissions::recordMovingAsteroidDestroyed() {
    for (auto& mission : currentMissions) {
        if (mission.type == MissionType::DESTROY_MOVING_ASTEROIDS && !mission.isCompleted()) {
            mission.currentProgress++;
            if (mission.currentProgress == 1) {
                mission.status = MissionStatus::IN_PROGRESS;
            }
        }
    }
}

void DailyMissions::recordAsteroidDestroyed() {
    for (auto& mission : currentMissions) {
        if (mission.type == MissionType::DESTROY_ASTEROIDS && !mission.isCompleted()) {
            mission.currentProgress++;
            if (mission.currentProgress == 1) {
                mission.status = MissionStatus::IN_PROGRESS;
            }
        }
    }
}

void DailyMissions::recordRingCollected() {
    for (auto& mission : currentMissions) {
        if (mission.type == MissionType::COLLECT_RINGS && !mission.isCompleted()) {
            mission.currentProgress++;
            if (mission.currentProgress == 1) {
                mission.status = MissionStatus::IN_PROGRESS;
            }
        }
    }
}

void DailyMissions::recordDamage() {
    resetDamageFreeTime();
}

void DailyMissions::startSession() {
    sessionActive = true;
    sessionStartTime = 0.0; // Will be set by game time
    lastDamageTime = 0.0;
    damageFreeTime = 0.0;
}

void DailyMissions::resetDamageFreeTime() {
    damageFreeTime = 0.0;
    lastDamageTime = sessionStartTime;
}

void DailyMissions::updatePlayTime(double currentTime) {
    if (!sessionActive) {
        std::cout << "Session not active, starting session..." << std::endl;
        startSession();
    }
    
    if (sessionStartTime == 0.0) {
        sessionStartTime = currentTime;
        lastDamageTime = currentTime;
        std::cout << "Session started at time: " << currentTime << std::endl;
    }
    
    // Update damage-free time
    damageFreeTime = currentTime - lastDamageTime;
    
    // Update missions
    int survivalSeconds = static_cast<int>(currentTime - sessionStartTime);
    int damageFreSeconds = static_cast<int>(damageFreeTime);
    
    updateMissionProgress(MissionType::FLY_WITHOUT_DAMAGE, damageFreSeconds);
    updateMissionProgress(MissionType::SURVIVE_TIME, survivalSeconds);
    
    // Debug output for time missions
    static double lastDebugTime = 0.0;
    if (currentTime - lastDebugTime > 5.0) { // Print debug every 5 seconds
        std::cout << "Time missions - Survival: " << survivalSeconds << "s, Damage-free: " << damageFreSeconds << "s" << std::endl;
        lastDebugTime = currentTime;
    }
}

void DailyMissions::updateMissionProgress(MissionType type, int value) {
    bool progressChanged = false;
    
    for (auto& mission : currentMissions) {
        if (mission.type == type && !mission.isCompleted()) {
            int oldProgress = mission.currentProgress;
            
            // For max-value missions (speed, score), keep the highest value
            if (type == MissionType::REACH_SPEED || type == MissionType::REACH_SCORE || 
                type == MissionType::CONSECUTIVE_RINGS) {
                mission.currentProgress = std::max(mission.currentProgress, value);
            }
            // For time-based missions, use current value
            else if (type == MissionType::FLY_WITHOUT_DAMAGE || type == MissionType::SURVIVE_TIME) {
                mission.currentProgress = value;
                // Debug time mission updates
                if (type == MissionType::SURVIVE_TIME) {
                    static int lastReportedValue = -1;
                    if (value != lastReportedValue && value % 10 == 0) { // Report every 10 seconds
                        std::cout << "SURVIVE_TIME mission progress: " << value << "/" << mission.targetValue << std::endl;
                        lastReportedValue = value;
                    }
                }
            }
            
            if (mission.currentProgress > 0 && mission.status == MissionStatus::NOT_STARTED) {
                mission.status = MissionStatus::IN_PROGRESS;
                progressChanged = true;
                std::cout << "Mission " << mission.title << " status changed to IN_PROGRESS" << std::endl;
            }
            
            // Check if progress actually changed
            if (mission.currentProgress != oldProgress) {
                progressChanged = true;
            }
        }
    }
    
    checkMissionCompletion();
    
    // Save progress periodically when changes are made (throttled to avoid spam)
    static double lastSaveTime = 0.0;
    static double currentGameTime = 0.0;
    currentGameTime += 0.016; // Approximate frame time for save throttling
    
    if (progressChanged && (currentGameTime - lastSaveTime > 5.0)) { // Save at most every 5 seconds
        saveToFile(missionsFilePath);
        lastSaveTime = currentGameTime;
        std::cout << "Mission progress saved to file" << std::endl;
    }
}

void DailyMissions::checkMissionCompletion() {
    bool progressMade = false;
    for (auto& mission : currentMissions) {
        if (mission.status != MissionStatus::COMPLETED && mission.isCompleted()) {
            mission.status = MissionStatus::COMPLETED;
            std::cout << "Mission completed: " << mission.title << std::endl;
            progressMade = true;
        }
    }
    
    // Save progress to file if any missions were completed
    if (progressMade) {
        saveToFile(missionsFilePath);
    }
}

std::vector<Mission> DailyMissions::getTodaysMissions() {
    return currentMissions;
}

int DailyMissions::getCompletedMissionsCount() const {
    int count = 0;
    for (const auto& mission : currentMissions) {
        if (mission.status == MissionStatus::COMPLETED) {
            count++;
        }
    }
    return count;
}

bool DailyMissions::hasMissionsForToday() const {
    return !currentMissions.empty() && currentDate == getCurrentDateString();
}

void DailyMissions::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to save missions to " << filename << std::endl;
        return;
    }
    
    file << "# StarTux Daily Missions\n";
    file << "date=" << currentDate << "\n";
    
    for (const auto& mission : currentMissions) {
        file << "mission=" << static_cast<int>(mission.type) << ","
             << mission.targetValue << ","
             << mission.currentProgress << ","
             << static_cast<int>(mission.status) << ","
             << mission.title << ","
             << mission.description << "\n";
    }
    
    file.close();
    std::cout << "Missions saved to " << filename << std::endl;
}

void DailyMissions::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "No existing missions file found. Will generate new missions." << std::endl;
        return;
    }
    
    currentMissions.clear();
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        if (line.substr(0, 5) == "date=") {
            currentDate = line.substr(5);
        }
        else if (line.substr(0, 8) == "mission=") {
            std::stringstream ss(line.substr(8));
            std::string item;
            std::vector<std::string> tokens;
            
            while (std::getline(ss, item, ',')) {
                tokens.push_back(item);
            }
            
            if (tokens.size() >= 6) {
                Mission mission;
                mission.id = currentMissions.size();
                mission.type = static_cast<MissionType>(std::stoi(tokens[0]));
                mission.targetValue = std::stoi(tokens[1]);
                mission.currentProgress = std::stoi(tokens[2]);
                mission.status = static_cast<MissionStatus>(std::stoi(tokens[3]));
                mission.title = tokens[4];
                mission.description = tokens[5];
                mission.reward = "XP + 100";
                
                currentMissions.push_back(mission);
            }
        }
    }
    
    file.close();
    std::cout << "Loaded " << currentMissions.size() << " missions from " << filename << std::endl;
}