### Usage in arduino IDE
Open File > Preferences
Add `http://arduino.esp8266.com/stable/package_esp8266com_index.json, https://dl.espressif.com/dl/package_esp32_index.json` Into `Additional Board Manager URLs`.

Open Tools > Board > Board Manager...
Search for `esp32` and install the `esp32` package by `Espressif Systems`.

Open Tools > Manage libraries
Search for `littlefs` and change `Topic` to `data Storage`

## Usage in Platform.IO
I couldn't run project after adding LITTLEFS from platform.io library manager, so I cloned LITTLEFS project from git and put it inside PROJECT_DIR/.pio/libdeps/lolin32/LittleFS_esp32. Clone it from here: https://github.com/lorol/LITTLEFS