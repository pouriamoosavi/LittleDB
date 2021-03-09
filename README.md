# Little Sql like DB for ESP32 boards
A little Sql like database, which runs on esp32. 

## Installation
### In Arduino IDE
- Clone project from this repo.
- Extract it.
- Move files from lib, test and src directories into one arduino project.
- Rename main.cpp into YOUR_PROJECT_NAME.ino (in Arduino IDE, .ino file and project must have the same name).
- In Arduino IDE Open File > Preferences
Add `http://arduino.esp8266.com/stable/package_esp8266com_index.json, https://dl.espressif.com/dl/package_esp32_index.json` Into `Additional Board Manager URLs`.
- In Arduino IDE Open Tools > Board > Board Manager...
Search for `esp32` and install the `esp32` package by `Espressif Systems`.
- In Arduino IDE  Open Tools > Manage libraries
Search for `littlefs` and change `Topic` to `data Storage`

### In Platform.&#46;IO
- Clone project from this repo.
- Extract it.
- Open containing folder with VSCode with Platform.&#46;IO installed.
- Add esp32 library from Platform.&#46;IO library manager. (Wemos lolin32).
- Clone <a href="https://github.com/lorol/LITTLEFS">LITTLEFS project</a> from git and put it inside PROJECT_DIR/.pio/libdeps/lolin32/- LittleFS_esp32.  (I couldn't run project after adding LITTLEFS from Platform.&#46;IO library manager)
- Connect esp32 board to your computer.
- Run project on your board. (or run tests)

## Dependencies
- LittleFS
