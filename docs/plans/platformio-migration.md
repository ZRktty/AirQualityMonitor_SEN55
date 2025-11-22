# Plan: Migrate to PlatformIO

This plan converts the existing Arduino project structure to the standard PlatformIO layout, enabling better dependency management and tooling while keeping the configuration flexible for manual library additions.

## Steps

- [ ] **Create Configuration**: Create `platformio.ini` with the base configuration for ESP32-S3 (board, framework, monitor speed).
- [ ] **Create Source Directory**: Create a `src` directory in the project root.
- [ ] **Move Source Files**: Move `AirQualityMonitor_SEN55.ino`, `DataAveraging.cpp/h`, `SensorUtils.cpp/h`, and `config.h` into `src/`.
- [ ] **Rename Main File**: Rename `AirQualityMonitor_SEN55.ino` to `main.cpp` and add `#include <Arduino.h>` at the top.
- [ ] **Update Git Ignore**: Update `.gitignore` to exclude PlatformIO build artifacts (`.pio` folder).
- [ ] **Install Libraries**: Search PlatformIO Registry for required libraries (Sensirion, etc.) and add them to `lib_deps` in `platformio.ini`.

## Further Considerations

1. **Library Search**: You can use `pio pkg search "library name"` in the terminal or the PlatformIO Home "Libraries" tab to find the correct names/versions.
2. **Config File**: `config.h` stays in `src` so it's automatically found during compilation.
