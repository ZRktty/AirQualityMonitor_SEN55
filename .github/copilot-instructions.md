# GitHub Copilot Instructions for Arduino ESP32 Project

## Coding Standards

### General Principles
- **Target Framework:** Use platformio.ini for project configuration.
- **Code Style:** Follow the Google C++ Style Guide for formatting and conventions.
- **Language:** Use C++ for embedded systems.
- **Readability:** Write clear, self-explanatory code. Use meaningful variable, method, and class names.
- **Consistency:** Follow consistent naming conventions and code formatting throughout the project.
- **Error Handling:** Handle errors gracefully. Log errors via Serial for debugging. Avoid silent failures.
- **SOLID Principles:** Adhere to SOLID design principles for maintainable and extensible code.
- **DRY:** Follow DRY principles to avoid code duplication.
- **Magic Numbers:** Avoid magic numbers; use named constants or enums.
- **File Organization:** One class per file. Organize files into appropriate folders by feature or layer.

### Naming Conventions
- **Constants:** Use `UPPER_SNAKE_CASE` for all constants and defines (e.g., `MAX_RETRY_COUNT`, `LED_PIN`)
- **Functions:** Use `camelCase` for functions and methods (e.g., `readSensor()`, `calculateAverage()`)
- **Classes:** Use `PascalCase` for class names (e.g., `SensorManager`, `DataAveraging`)
- **Member Variables:** Use `camelCase` with descriptive names (e.g., `sensorData`, `retryCount`)

### Performance Guidelines
- **Memory Management:** 
  - Board: ESP32-S3 with 320KB SRAM + 8MB PSRAM
  - Minimize dynamic memory allocation in tight loops
  - Prefer stack allocation for small objects (<1KB)
  - Use PSRAM for large buffers (images, audio, large arrays)
  - Monitor heap fragmentation with `ESP.getFreeHeap()` and `ESP.getHeapSize()`
  - Avoid memory leaks by properly deallocating resources
- **Loop Efficiency:** Avoid blocking operations in `loop()`. Use non-blocking patterns and millis() for timing.
- **Serial Output:** Minimize `Serial.print()` calls in production code. Use conditional compilation for debug output.
- **Sensor Polling:** Implement appropriate delays between sensor readings to avoid overwhelming I2C/SPI buses.
- **String Usage:** Avoid excessive String concatenation. Prefer char arrays or F() macro for constant strings.
- **PSRAM Usage:** Leverage 8MB PSRAM for data buffering, averaging arrays, and WiFi/network buffers when available.

### Documentation Style
- **Doxygen Comments:** Use Doxygen-style comments for all functions and classes:
  ```cpp
  /**
   * @brief Brief description of the function
   * 
   * @param paramName Description of the parameter
   * @return Description of return value
   */
  ```
- **Header Files:** Include comprehensive documentation in header files:
  - Class purpose and usage
  - Public method descriptions with parameters and return values
  - Example usage if applicable
- **Inline Comments:** Add inline comments only where necessary to clarify complex logic or hardware-specific behavior.

