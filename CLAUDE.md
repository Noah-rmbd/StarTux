# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

StarTux is a 3D space game built with OpenGL in C++. It's an asteroid-dodging game where the player navigates through space, avoiding asteroids and collecting rings while shooting projectiles. The game features two modes: development mode for debugging and camera control, and play mode for actual gameplay.

## Build System and Commands

This project uses CMake as the build system. All build commands should be run from the `Project/` directory.

### Essential Commands
- **Build the project**: `cmake . && make`
- **Run the game**: `make run` (or `./opengl_program` after building)
- **Clean build**: `make clean`
- **Initialize submodules** (required after cloning): `git submodule update --init --recursive`

### Dependencies
The project requires these external libraries:
- libsndfile (audio): Install with `sudo dnf install libsndfile-devel` on Fedora
- Freetype (text rendering)
- Assimp (3D model loading)
- OpenAL (audio)

External libraries are included as submodules:
- GLEW (OpenGL extensions)
- GLFW (windowing)
- GLM (math library)

## Architecture Overview

### Core Architecture
The project follows a component-based architecture with these main systems:

1. **Viewer** (`src/viewer.cpp`, `include/viewer.h`)
   - Main application entry point and window management
   - Handles GLFW initialization, input events, and main render loop
   - Manages the startup screen, game instance, and audio

2. **Game** (`src/game/game.cpp`, `include/game/game.h`)
   - Core game logic and state management
   - Handles collision detection, object spawning, and game mechanics
   - Manages two modes: dev mode (camera control) and play mode (gameplay)

3. **Scene Graph** (`src/node.cpp`, `include/node.h`)
   - Hierarchical scene organization using transformation nodes
   - `world_node` contains all game objects and moves during gameplay
   - `scene_root` is the top-level container

### Key Game Components

- **Player** (`src/game/player.cpp`): The controllable spaceship
- **Camera** (`src/game/camera.cpp`): View management for both dev and play modes
- **Asteroids** (`src/game/asteroid.cpp`): Obstacles that spawn and move toward the player
- **Projectiles** (`src/game/projectile.cpp`, `src/game/light_projectile.cpp`): Player weapons
- **Rings** (`src/game/ring.cpp`): Collectible objects
- **Explosions** (`src/game/explosion.cpp`): Visual effects for collisions

### Graphics System

- **Shader Management** (`src/graphics/shader.cpp`): OpenGL shader loading and compilation
- **Texture System** (`src/graphics/texture.cpp`): Image loading and texture management
- **Shape System** (`src/shapes/`): 3D geometry rendering
  - Base `Shape` class with specialized implementations
  - `TexturedSphere`, `LightingSphere` for different rendering techniques
  - `ShapeModel` for loading OBJ files

### Interface System

- **HUD** (`src/interfaces/hud.cpp`): In-game overlay with score and status
- **StartupScreen** (`src/interfaces/startup_screen.cpp`): Main menu interface
- **Interface** (`src/interfaces/interface.cpp`): Base interface management

### Audio System

- **AudioManager** (`src/audio_manager.cpp`): OpenAL-based audio playback
- Supports background music and sound effects

## Game Controls

### Development Mode (press 'C' to activate)
- W/A/S/D: Camera movement
- R/F: Move up/down
- Mouse: Camera direction

### Play Mode (press 'X' to activate)
- U/J: Up/Down movement
- H/K: Left/Right movement  
- O/P: Roll left/right
- Mouse: Shooting (left click for regular projectiles, right click for light projectiles)

## Directory Structure

```
Project/
├── src/                    # Source files
│   ├── game/              # Game logic components
│   ├── graphics/          # Rendering system
│   ├── interfaces/        # UI components
│   └── shapes/            # 3D geometry
├── include/               # Header files (mirrors src structure)
├── shaders/              # GLSL shader files
├── textures/             # Image assets
├── ressources/           # 3D models (.obj files)
├── audio/                # Sound files (.wav)
└── external/             # Third-party libraries (submodules)
```

## Development Notes

- The project uses C++11 standard
- Shader paths are defined at compile-time via CMake definitions
- Assets (shaders, textures, models, audio) use absolute paths via compile-time macros
- The main executable is `opengl_program`
- Resources are loaded from compile-time defined directories (SHADER_DIR, TEXTURES_DIR, etc.)
- The game runs at 30 FPS by default

## Asset Pipeline

- **3D Models**: OBJ format stored in `ressources/`
- **Textures**: Various formats (PNG, JPEG) in `textures/`
- **Shaders**: Vertex (.vert) and Fragment (.frag) files in `shaders/`
- **Audio**: WAV files in `audio/`
- **Fonts**: TTF and OTF files in `ressources/`