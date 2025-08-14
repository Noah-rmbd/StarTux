#ifndef GAME_UTILS_H
#define GAME_UTILS_H

#include <vector>
#include <memory>
#include <algorithm>
#include <functional>

/**
 * Modern utility functions for game object management
 */
namespace GameUtils {

/**
 * Safe removal of dead objects using modern C++ algorithms
 * This replaces manual iterator loops with safe, readable code
 * 
 * Example usage:
 *   RemoveDeadObjects(projectiles, [](const auto& proj) { 
 *       return !proj->IsActive(); 
 *   });
 */
template<typename Container, typename Predicate>
void RemoveDeadObjects(Container& container, Predicate shouldRemove) {
    // Use erase-remove idiom - modern, safe, and efficient
    auto newEnd = std::remove_if(container.begin(), container.end(), shouldRemove);
    container.erase(newEnd, container.end());
    
    // Objects are automatically destroyed when erased from container!
    // No manual deletion needed with smart pointers!
}

/**
 * Process all objects in container with a function
 * Much safer than manual loops
 */
template<typename Container, typename Function>
void ProcessObjects(Container& container, Function process) {
    std::for_each(container.begin(), container.end(), process);
}

/**
 * Find first object matching condition
 * Returns nullptr if not found
 */
template<typename Container, typename Predicate>
auto FindFirst(Container& container, Predicate condition) -> typename Container::value_type::element_type* {
    auto it = std::find_if(container.begin(), container.end(), 
        [&condition](const auto& obj) { return condition(*obj); });
    
    if (it != container.end()) {
        return it->get();  // Return raw pointer for temporary access
    }
    return nullptr;
}

/**
 * Check if any object matches condition
 */
template<typename Container, typename Predicate>
bool AnyObject(const Container& container, Predicate condition) {
    return std::any_of(container.begin(), container.end(),
        [&condition](const auto& obj) { return condition(*obj); });
}

/**
 * Count objects matching condition
 */
template<typename Container, typename Predicate>
size_t CountObjects(const Container& container, Predicate condition) {
    return std::count_if(container.begin(), container.end(),
        [&condition](const auto& obj) { return condition(*obj); });
}

} // namespace GameUtils

#endif // GAME_UTILS_H