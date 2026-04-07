#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_W 128
#define SCREEN_H 64
#define OLED_ADDR 0x3C

#define BTN_UP    19
#define BTN_LEFT  18
#define BTN_RIGHT 23
#define BTN_DOWN  5

#define CELL      4          // px per grid cell
#define GRID_W    (SCREEN_W / CELL)   // 32
#define GRID_H    (SCREEN_H / CELL)   // 16
#define MAX_LEN   (GRID_W * GRID_H)

#define SPEED_INIT  200      // ms per tick
#define SPEED_MIN   80
#define SPEED_STEP  5        // speed up every food eaten

Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);

struct Point { int8_t x, y; };

Point snake[MAX_LEN];
int   snakeLen;
Point dir;
Point food;
int   score;
unsigned long lastTick;
unsigned long tickInterval;
bool  gameOver;
bool  started;

// ---- helpers ----

void spawnFood() {
  bool onSnake;
  do {
    onSnake = false;
    food.x = random(0, GRID_W);
    food.y = random(0, GRID_H);
    for (int i = 0; i < snakeLen; i++) {
      if (snake[i].x == food.x && snake[i].y == food.y) {
        onSnake = true;
        break;
      }
    }
  } while (onSnake);
}

void initGame() {
  snakeLen = 3;
  dir = {1, 0};
  int sx = GRID_W / 2;
  int sy = GRID_H / 2;
  for (int i = 0; i < snakeLen; i++) {
    snake[i] = {(int8_t)(sx - i), (int8_t)sy};
  }
  score = 0;
  tickInterval = SPEED_INIT;
  lastTick = millis();
  gameOver = false;
  spawnFood();
}

void drawGame() {
  display.clearDisplay();

  // border
  display.drawRect(0, 0, SCREEN_W, SCREEN_H, WHITE);

  // food (small X)
  int fx = food.x * CELL + 1;
  int fy = food.y * CELL + 1;
  display.drawPixel(fx,     fy,     WHITE);
  display.drawPixel(fx + 2, fy,     WHITE);
  display.drawPixel(fx + 1, fy + 1, WHITE);
  display.drawPixel(fx,     fy + 2, WHITE);
  display.drawPixel(fx + 2, fy + 2, WHITE);

  // snake
  for (int i = 0; i < snakeLen; i++) {
    int px = snake[i].x * CELL;
    int py = snake[i].y * CELL;
    // if (i == 0) {
      display.fillRect(px, py, CELL, CELL, WHITE);
    // } else {
      // display.drawRect(px, py, CELL, CELL, WHITE);
    // }
  }

  // score
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(SCREEN_W - 24, 1);
  display.print(score);

  display.display();
}

void drawStartScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 10);
  display.print("SNAKE");
  display.setTextSize(1);
  display.setCursor(18, 36);
  display.print("Press any btn");
  display.setCursor(22, 48);
  display.print("to start");
  display.display();
}

void drawGameOver() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(8, 8);
  display.print("GAME OVER");
  display.setTextSize(1);
  display.setCursor(28, 32);
  display.print("Score: ");
  display.print(score);
  display.setCursor(12, 48);
  display.print("Press any btn");
  display.display();
}

void tickGame() {
  // move: shift body back
  for (int i = snakeLen - 1; i > 0; i--) {
    snake[i] = snake[i - 1];
  }
  // new head
  snake[0].x += dir.x;
  snake[0].y += dir.y;

  // wall collision
  if (snake[0].x < 0 || snake[0].x >= GRID_W ||
      snake[0].y < 0 || snake[0].y >= GRID_H) {
    gameOver = true;
    return;
  }

  // self collision
  for (int i = 1; i < snakeLen; i++) {
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
      gameOver = true;
      return;
    }
  }

  // eat food
  if (snake[0].x == food.x && snake[0].y == food.y) {
    if (snakeLen < MAX_LEN) snakeLen++;
    score++;
    if (tickInterval > SPEED_MIN) tickInterval -= SPEED_STEP;
    spawnFood();
  }
}

// ---- button debounce ----
// Returns true on falling edge (press)
struct Button {
  uint8_t pin;
  bool    lastState;
  unsigned long lastDebounce;
};

Button buttons[4] = {
  {BTN_UP,    HIGH, 0},
  {BTN_LEFT,  HIGH, 0},
  {BTN_RIGHT, HIGH, 0},
  {BTN_DOWN,  HIGH, 0},
};

bool btnPressed(int idx) {
  bool reading = digitalRead(buttons[idx].pin);
  if (reading != buttons[idx].lastState) {
    buttons[idx].lastDebounce = millis();
  }
  buttons[idx].lastState = reading;
  if ((millis() - buttons[idx].lastDebounce) > 30 && reading == LOW) {
    return true;
  }
  return false;
}

// edge-detect state
bool btnWasLow[4] = {false, false, false, false};

bool btnJustPressed(int idx) {
  bool now = btnPressed(idx);
  if (now && !btnWasLow[idx]) {
    btnWasLow[idx] = true;
    return true;
  }
  if (!now) btnWasLow[idx] = false;
  return false;
}

// ---- setup / loop ----

void setup() {
  Serial.begin(115200);
  pinMode(BTN_UP,    INPUT_PULLUP);
  pinMode(BTN_LEFT,  INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_DOWN,  INPUT_PULLUP);

  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 init failed");
    while (true);
  }

  display.clearDisplay();
  display.display();
  randomSeed(analogRead(0));

  started = false;
  gameOver = false;
  drawStartScreen();
}

void loop() {
  // enum: 0=UP 1=LEFT 2=RIGHT 3=DOWN
  bool up    = btnJustPressed(0);
  bool left  = btnJustPressed(1);
  bool right = btnJustPressed(2);
  bool down  = btnJustPressed(3);
  bool any   = up || left || right || down;

  if (!started) {
    if (any) {
      started = true;
      initGame();
      drawGame();
    }
    return;
  }

  if (gameOver) {
    if (any) {
      initGame();
    } else {
      drawGameOver();
    }
    return;
  }

  // direction input — no 180 reversals
  if (up    && dir.y ==  0) { dir = { 0, -1}; }
  if (down  && dir.y ==  0) { dir = { 0,  1}; }
  if (left  && dir.x ==  0) { dir = {-1,  0}; }
  if (right && dir.x ==  0) { dir = { 1,  0}; }

  unsigned long now = millis();
  if (now - lastTick >= tickInterval) {
    lastTick = now;
    tickGame();
    if (gameOver) {
      drawGameOver();
    } else {
      drawGame();
    }
  }
}