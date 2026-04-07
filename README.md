ESP32 Snake Game
Snake game running on an ESP32 with a 128x64 SSD1306 OLED display and 4 push buttons.

Hardware Required

ESP32 DOIT DevKit V1 (30-pin)
SSD1306 128x64 OLED (I2C)
4x momentary push buttons
Jumper wires


Wiring
OLED Display
The OLED uses I2C. Connect VCC to the ESP32's 3.3V pin and GND to any ground pin. Connect SDA to GPIO 21 and SCL to GPIO 22. These are the ESP32's default I2C pins.
Buttons
Each button has one leg going to its assigned GPIO pin and the other leg going to GND. No external resistors needed — the code uses internal pull-ups.

Up 19
Left 18
Right 23
Down 5


How to Build
Open the project folder in VS Code with PlatformIO installed. PlatformIO will automatically install the required libraries on first build. Hit upload and you're good.
Dependencies (auto-installed via platformio.ini)

Adafruit SSD1306
Adafruit GFX Library


How to Play
Press any button on the start screen to begin. Use the four buttons to steer the snake. Eating food grows the snake and increases your score. The game speeds up as your score climbs. Running into a wall or yourself ends the game. Press any button on the game over screen to restart.

Notes

Grid is 32x16 cells at 4px each, filling the full 128x64 display.
Speed starts at 200ms per tick and caps out at 80ms.
180-degree reversals are blocked — you can't instantly go back the way you came.
