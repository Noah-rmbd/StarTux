#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>
#include <functional>

// Forward declaration
template<typename T>
class SpatialHashGrid;

// Hash function for grid coordinates
struct GridCoordHash {
    std::size_t operator()(const std::pair<int, int>& coord) const {
        return std::hash<int>()(coord.first) ^ (std::hash<int>()(coord.second) << 1);
    }
};

template<typename T>
class SpatialHashGrid {
public:
    using ObjectPtr = T*;
    using GridCoord = std::pair<int, int>;
    using CellObjects = std::unordered_set<ObjectPtr>;
    using Grid = std::unordered_map<GridCoord, CellObjects, GridCoordHash>;

private:
    Grid grid_;
    float cell_size_;
    std::unordered_map<ObjectPtr, std::vector<GridCoord>> object_cells_; // Track which cells each object occupies

public:
    explicit SpatialHashGrid(float cell_size) : cell_size_(cell_size) {}

    // Convert world position to grid coordinates
    GridCoord worldToGrid(const glm::vec3& position) const {
        return {
            static_cast<int>(std::floor(position.x / cell_size_)),
            static_cast<int>(std::floor(position.y / cell_size_))
        };
    }

    // Get all cells that a bounding box occupies
    std::vector<GridCoord> getBoundingCells(const glm::vec3& center, float radius) const {
        std::vector<GridCoord> cells;
        
        glm::vec3 min_pos = center - glm::vec3(radius, radius, 0.0f);
        glm::vec3 max_pos = center + glm::vec3(radius, radius, 0.0f);
        
        GridCoord min_cell = worldToGrid(min_pos);
        GridCoord max_cell = worldToGrid(max_pos);
        
        for (int x = min_cell.first; x <= max_cell.first; ++x) {
            for (int y = min_cell.second; y <= max_cell.second; ++y) {
                cells.emplace_back(x, y);
            }
        }
        
        return cells;
    }

    // Insert object into the grid
    void insert(ObjectPtr object, const glm::vec3& position, float radius = 0.0f) {
        remove(object); // Remove from previous position first
        
        std::vector<GridCoord> cells = getBoundingCells(position, radius);
        object_cells_[object] = cells;
        
        for (const auto& cell : cells) {
            grid_[cell].insert(object);
        }
    }

    // Remove object from the grid
    void remove(ObjectPtr object) {
        auto it = object_cells_.find(object);
        if (it != object_cells_.end()) {
            for (const auto& cell : it->second) {
                auto grid_it = grid_.find(cell);
                if (grid_it != grid_.end()) {
                    grid_it->second.erase(object);
                    if (grid_it->second.empty()) {
                        grid_.erase(grid_it);
                    }
                }
            }
            object_cells_.erase(it);
        }
    }

    // Update object position
    void update(ObjectPtr object, const glm::vec3& position, float radius = 0.0f) {
        insert(object, position, radius);
    }

    // Get nearby objects within a certain area
    std::vector<ObjectPtr> getNearby(const glm::vec3& position, float radius) const {
        std::unordered_set<ObjectPtr> nearby;
        std::vector<GridCoord> cells = getBoundingCells(position, radius);
        
        for (const auto& cell : cells) {
            auto it = grid_.find(cell);
            if (it != grid_.end()) {
                for (ObjectPtr obj : it->second) {
                    nearby.insert(obj);
                }
            }
        }
        
        return std::vector<ObjectPtr>(nearby.begin(), nearby.end());
    }

    // Get all objects in the same cells as the given position
    std::vector<ObjectPtr> getObjectsInCell(const glm::vec3& position) const {
        GridCoord cell = worldToGrid(position);
        auto it = grid_.find(cell);
        
        if (it != grid_.end()) {
            return std::vector<ObjectPtr>(it->second.begin(), it->second.end());
        }
        
        return {};
    }

    // Clear all objects
    void clear() {
        grid_.clear();
        object_cells_.clear();
    }

    // Get statistics
    size_t getCellCount() const { return grid_.size(); }
    size_t getObjectCount() const { return object_cells_.size(); }

    // Debug: Get all occupied cells
    std::vector<GridCoord> getOccupiedCells() const {
        std::vector<GridCoord> cells;
        for (const auto& [cell, objects] : grid_) {
            if (!objects.empty()) {
                cells.push_back(cell);
            }
        }
        return cells;
    }
};