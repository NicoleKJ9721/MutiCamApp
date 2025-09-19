# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview

MutiCamApp (多摄像头应用程序) is a C++ multi-camera application using Qt6, OpenCV, and HikVision SDK for camera control, image processing, and template matching. It provides real-time capture from multiple cameras with advanced image analysis capabilities.

## Build System

This project uses CMake with Qt6 and requires HALCON environment setup.

### Build Commands
```bash
# Configure with CMake (requires HALCON environment variable)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build build --config Release

# For debug builds
cmake --build build --config Debug
```

### Prerequisites
- HALCONROOT environment variable must be set to your HALCON installation directory
- Qt6 with Widgets, Concurrent, and Svg components
- OpenCV 4.10.0 (included in third_party/)
- HikVision SDK (included in third_party/)

## Core Architecture

### Module Structure

**Camera System** (`src/camera/`):
- `CameraManager`: Central controller managing multiple camera instances
- `CameraThread`: Thread-based camera capture with Qt signals/slots
- `HikvisionCamera`: HikVision SDK camera implementation
- Supports three simultaneous cameras: vertical, left, and front views

**Image Processing** (`src/image_processing/`):
- `EdgeDetector`: Canny edge detection with configurable parameters
- `ShapeDetector`: Line and circle detection using HoughLines and HoughCircles

**UI Framework** (`src/ui/`):
- `MutiCamApp`: Main window with multi-tab layout
- `VideoDisplayWidget`: Real-time camera feed display
- `ZoomPanWidget`: Interactive zoom/pan controls for each view
- `PaintingOverlay`: Drawing tools overlay (lines, circles, ROI selection)
- Dialog-based calibration and template creation systems

**Configuration** (`src/utils/`):
- `SettingsManager`: JSON-based configuration with real-time UI binding
- `LogManager`: Application logging system
- `SerialController`: Hardware button control via serial communication
- `TrajectoryRecorder`: XYZ stage movement tracking

### Threading Model

The application uses Qt's threading with separate camera threads for each view:
- Main UI thread handles all Qt widgets and user interactions
- `CameraThread` instances run capture loops for each camera
- Thread-safe communication via Qt signals/slots
- Frame rate monitoring and statistics per camera

### Memory Management

- Smart pointers (`std::shared_ptr`, `std::unique_ptr`) throughout
- QCache for frame caching with configurable limits
- Automatic OpenCV Mat memory management
- Explicit cleanup in destructors for camera resources

## Development Commands

### Test and Debug
```bash
# Run with debug output (comment WIN32 in CMakeLists.txt)
.\build\Debug\MutiCamApp.exe

# Check dependencies
# Use src/utils/dependencies_test.cpp for SDK validation

# Clean build
cmake --build build --target clean
```

### Camera Configuration
Camera serial numbers are configured in `config/settings.json`:
- `VerCamSN`: Vertical view camera
- `LeftCamSN`: Left view camera  
- `FrontCamSN`: Front view camera

### Template Matching
The application supports both OpenCV and HALCON template matching:
- OpenCV implementation in `refer/` directory
- HALCON integration available when HALCONROOT is set
- Custom high-precision matching implementation (reward target: 2000元)

## Key Patterns

### Settings Management
All UI parameters auto-save via `SettingsManager`:
- Real-time binding between UI controls and JSON storage
- Delayed save mechanism to avoid excessive I/O
- Camera serial number changes trigger system reinitialization

### Multi-View Architecture  
Three synchronized camera views with independent:
- Calibration parameters (pixel scale, units)
- Drawing overlays and measurement tools
- Zoom/pan states and view transformations
- Template matching and ROI selection

### Hardware Integration
- XYZ stage control with position feedback
- Physical button capture via serial communication
- Real-time trajectory recording with statistics

## File Structure Navigation

```
src/
├── core/           # Application entry and main window
├── camera/         # Multi-camera management system
├── ui/             # Qt widgets and dialogs
├── image_processing/ # OpenCV-based analysis
└── utils/          # Configuration, logging, serial I/O

config/             # JSON configuration files
third_party/        # Bundled dependencies (OpenCV, HikVision)
refer/              # Reference implementations (Halcon matching)
```

## Common Development Tasks

### Adding New Camera Features
1. Extend `ICameraController` interface in `camera_controller.h`
2. Implement in `HikvisionCamera` class
3. Update `CameraThread` for new capture modes
4. Add UI controls and connect to `CameraManager` signals

### Extending Image Processing
1. Create new detector class in `image_processing/`
2. Follow `EdgeDetector`/`ShapeDetector` patterns
3. Add parameters to `SettingsManager::Settings`
4. Integrate with `PaintingOverlay` for visualization

### UI Dialog Development
1. Create .ui file with Qt Designer
2. Generate header with `uic` (handled by CMake AUTOUIC)
3. Implement dialog class inheriting from `QDialog`
4. Connect to main window via button mapping system

### Performance Optimization
- Use frame caching system in main application
- Leverage OpenMP for parallel processing when available
- Monitor frame rates via built-in statistics
- Consider GPU acceleration for intensive operations