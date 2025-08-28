# StarTux 🚀

**StarTux** is an exciting 3D space game built with OpenGL and C++. Navigate your spaceship through a perilous asteroid field, collect rings, and blast your way through obstacles in this fast-paced arcade-style game.

![StarTux Logo](textures/startuxlogo41.png)

## 🎮 Game Overview

In StarTux, you pilot a spaceship through endless space, facing these challenges:
- **Dodge Asteroids**: Avoid collision with randomly spawned asteroids
- **Collect Rings**: Gather rings scattered throughout space for points
- **Combat System**: Use two types of projectiles to destroy obstacles
- **Dynamic Environment**: Experience a living space environment with particle effects
- **Mission System**: Complete daily missions for extra challenges
- **Scoring System**: Compete for high scores and track your progress

### Game Modes

- **Play Mode**: The main gameplay experience with full ship control
- **Developer Mode**: Debug mode with free camera movement for development

## 🕹️ Controls

### Play Mode
- **U/J**: Move Up/Down
- **H/K**: Move Left/Right  
- **O/P**: Roll Left/Right
- **Left Mouse**: Fire light projectiles (rapid fire)
- **Right Mouse**: Fire heavy projectiles (limited ammo)
- **V**: Toggle pause
- **B**: Toggle invincibility (debug)
- **D**: Toggle collision debug spheres

### Developer Mode
- **T**: Enter developer mode
- **G**: Exit developer mode  
- **W/A/S/D**: Camera movement
- **R/F**: Move camera up/down
- **Mouse**: Look around

### General Controls
- **Q/ESC**: Quit game

## 🛠️ Installation & Setup

### Prerequisites

#### All Platforms
- **CMake** (3.10 or higher) - [Download here](https://cmake.org/download/)
- **Git** (for cloning and submodules)
- **C++11 compatible compiler** (GCC, Clang, or MSVC)

#### Platform-Specific Dependencies

##### macOS
```bash
# Install using Homebrew
brew install libsndfile freetype assimp openal-soft
```

##### Ubuntu/Debian
```bash
sudo apt update
sudo apt install libsndfile1-dev libfreetype-dev libassimp-dev libopenal-dev
```

##### Fedora/RHEL
```bash
sudo dnf install libsndfile-devel freetype-devel assimp-devel openal-soft-devel
```

##### Windows
- Install dependencies via vcpkg or manually download libraries
- Ensure OpenAL, Freetype, Assimp, and libsndfile are available

### Building the Project

1. **Clone the repository**
   ```bash
   git clone https://github.com/Noah-rmbd/StarTux.git
   cd StarTux/Project
   ```

2. **Initialize submodules** (REQUIRED)
   ```bash
   git submodule update --init --recursive
   ```

3. **Build the project**
   ```bash
   cmake .
   make
   ```

4. **Run the game**
   ```bash
   ./opengl_program
   # OR
   make run
   ```

### Clean Build
```bash
make clean
cmake .
make
```

## 🏗️ Project Architecture

### Core Systems

- **Viewer**: Main application window and event handling
- **Game**: Core game logic, collision detection, and state management
- **Scene Graph**: Hierarchical 3D scene organization
- **Graphics Pipeline**: Shader management, texture loading, and rendering
- **Audio System**: Background music and sound effects using OpenAL
- **Interface System**: HUD, menus, and UI components

### Key Components

- **Player**: Spaceship with physics, animations, and state management
- **Asteroids**: Dynamic obstacle system with collision detection
- **Projectiles**: Two weapon types (light and heavy) with different behaviors  
- **Rings**: Collectible items with scoring system
- **Explosions**: Visual effects for collisions and destruction
- **Mission System**: Daily challenges and progress tracking

## 📁 Directory Structure

```
Project/
├── src/                    # Source code
│   ├── game/              # Game logic (player, asteroids, projectiles)
│   ├── graphics/          # Rendering system (shaders, textures, lighting)
│   ├── interfaces/        # UI components (HUD, menus)
│   ├── shapes/            # 3D geometry and model loading
│   └── *.cpp              # Core application files
├── include/               # Header files (mirrors src/ structure)
├── shaders/              # GLSL vertex and fragment shaders
├── textures/             # Image assets (PNG, JPEG)
├── ressources/           # 3D models (OBJ files) and fonts
├── audio/                # Sound files (WAV format)
├── data/                 # Game data (missions, statistics)
└── external/             # Third-party libraries (submodules)
    ├── glew-cmake/       # OpenGL Extension Wrangler
    ├── glfw/             # Window management
    └── glm/              # OpenGL Mathematics
```

## 🎯 Gameplay Features

### Scoring System
- **Ring Collection**: 100 points per ring
- **Asteroid Destruction**: 25 points (regular projectiles), 50 points (light projectiles)
- **Survival Time**: Bonus points for staying alive longer
- **Speed Multiplier**: Higher speeds increase score multipliers

### Mission System
- **Daily Missions**: New challenges every day
- **Progress Tracking**: Monitor mission completion
- **Statistics**: Track games played, high scores, and achievements

### Visual Effects
- **Engine Flames**: Dynamic particle system for ship thrust
- **Explosions**: Detailed explosion effects with particle systems
- **Dynamic Lighting**: Real-time lighting effects
- **Space Environment**: Immersive 3D space backdrop

## 🔧 Configuration

### Graphics Settings
- **Shaders**: Located in `shaders/` directory
- **Textures**: Stored in `textures/` directory  
- **Models**: 3D assets in `ressources/` directory

### Audio Settings
- **Background Music**: `audio/start_music.wav` and `audio/game_music.wav`
- **Audio can be toggled**: Audio system supports enable/disable

### Performance
- **Target FPS**: 30 FPS (configurable)
- **Collision Optimization**: Spatial partitioning for performance
- **Resource Management**: Efficient memory usage with smart pointers

## 🐛 Troubleshooting

### Common Issues

#### Build Errors
- **Missing submodules**: Run `git submodule update --init --recursive`
- **CMake errors**: Ensure CMake 3.10+ is installed
- **Library not found**: Install platform-specific dependencies listed above

#### Runtime Issues
- **Audio not working**: Install OpenAL drivers for your platform
- **Graphics issues**: Update graphics drivers, ensure OpenGL 3.3+ support
- **Missing textures**: Verify all asset files are present in respective directories

#### Performance Issues
- **Low FPS**: Reduce window size or disable debug features
- **Memory usage**: Close other applications, check for memory leaks in debug mode

### Debug Mode Features
- **Performance Profiling**: Built-in profiler for optimization
- **Collision Visualization**: Toggle collision spheres with 'D' key
- **Free Camera**: Explore the scene with developer mode
- **Statistics Display**: Real-time performance metrics

## 🎮 Development

### Adding New Features
1. **Assets**: Place new textures in `textures/`, models in `ressources/`
2. **Shaders**: Add custom shaders to `shaders/` directory
3. **Game Objects**: Extend existing classes or create new ones in `src/game/`
4. **Audio**: Add sound effects to `audio/` directory

### Code Organization
- **Header files**: All headers in `include/` mirror `src/` structure
- **Game logic**: Main game loop in `src/game/game.cpp`
- **Rendering**: Graphics code in `src/graphics/` and `src/shapes/`
- **UI**: Interface code in `src/interfaces/`

## 📝 License

This project is developed as part of a computer graphics course. Please respect any licensing terms for third-party libraries included in the `external/` directory.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📞 Support

For issues, questions, or contributions:
- Create an issue in the GitHub repository
- Check the troubleshooting section above
- Review the `CLAUDE.md` file for development guidance

## Credit 
Audio made by : fl.setsuko [Instagram]

---

**Happy Gaming!** 🎮✨

Pilot your way through the stars and become the ultimate space navigator in StarTux!