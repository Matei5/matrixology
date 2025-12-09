// endless runner game for 16x2 lcd
// + 8x8 lcd drawing canvas
// features:
// - main menu with "Start", "Scores", and "About" options. + "Draw Matrix"
// - side scrolling platformer mechanics.
// - variable jump height with landing lag.
// - buzzer for sound effects.
// - eeprom used to save top 3 high scores.
// - custom graphics: player, fire, and cups.

#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <LedControl.h>

#define NOTE_C4  262
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_C6  1047

// -- hardware pins --
const int rsPin = 9;
const int enPin = 8;
const int d4Pin = 7;
const int d5Pin = 6;
const int d6Pin = 5;
const int d7Pin = 4;
const int joystickXAxisPin = A0;
const int joystickYAxisPin = A1;
const int joystickButtonPin = 3;
const int menuReturnButtonPin = 2;
const int buzzerPin = 13;
const int randomPin = A5; 
const byte matrixDinPin = 12;
const byte matrixClkPin = 11;
const byte matrixCsPin = 10;
const int echoPin = A2;
const int trigPin = A3;

const byte matrixDevices = 1;
const byte ledMatrixSize = 8;
const byte matrixDefaultIntensity = 5;

// game constants
const int baudRate = 9600;
const int lcdCols = 16;
const int lcdRows = 2;
const int scoreCol = 13;
const int playerColumn = 1;

const int gameSpeedStart = 350;
const int gameSpeedMax = 200;
const int speedStep = 20;
const int scoreLevel = 50; 

const int jumpDistance = 5; 
const int cooldownTime = 4; 

const int debounceTime = 200;
const int stateDelay = 500;
const int scorePageTime = 2000;
const int aboutPageTime = 2000;
const int resetHoldTime = 2000;
const int formatHoldTime = 1000;
const int msgResetDuration = 1000;
const int menuScrollDelay = 250;
const int drawMoveDelay = 180;
const int drawBlinkInterval = 250;
const int drawClearHoldTime = 1200;
const int drawExitHoldTime = 2500;
const int fastBlinkInterval = 150;
const int slowBlinkInterval = 450;
const int ultrasonicDisplayUpdateInterval = 500;
const int ultrasonicBaselineSampleTime = 3000;
const int ultrasonicReadyMessageDuration = 1000;
const int roguelikeMovementDelay = 200;
const int roguelikeTimerUpdateInterval = 1000;
const unsigned long echoTimeout = 20000;
const float microsecondsPerCm = 58.0;

const int probCoinAir = 10;
const int probCoinGround = 20;
const int probSpike = 40;
const int probStalactite = 60;
const int probHazardCup = 30;
const int randomResolution = 100;

const int joystickThresholdLow = 200;
const int joystickThresholdHigh = 800;

const int roguelikeWorldSize = 24;
const int roguelikeViewportSize = 8;
const int roguelikeInitialHealth = 10;
const int roguelikeInitialAmmo = 30;
const byte roguelikeTileEmpty = 0;
const byte roguelikeTileWall = 1;
const byte roguelikeTilePlayer = 2;

// Menu Options
const int menuOptionStart = 0;
const int menuOptionDraw = 1;
const int menuOptionRoguelike = 2;
const int menuOptionUltrasonic = 3;
const int menuOptionScores = 4;
const int menuOptionAbout = 5;
const int menuMaxIndex = 5;

// Page Counts & Indices
const int totalScorePages = 2;
const int totalAboutPages = 2;
const int aboutPageIndexRobotics = 0;

//  audio settings
const int toneMenuMoveDur = 20;
const int toneMenuSelectDur = 100;
const int toneJumpDur = 50;
const int toneResetDur = 300;
const int toneGameOverDur = 500;
const int toneCoinDur = 50;

// memory settings
const int eepromStartAddress = 10;
const int eepromResetCode = 148; 
const int eepromResetCodeAddress = 0;
const int highScoreCount = 3;
const int nameLen = 4;
const int entrySize = 6; 
const int asciiMin = 32;
const int asciiMax = 126;

// Custom characters 
const byte charPlayer = 0;
const byte charFire = 1;
const byte charCup = 2;
const byte charStalactite = 3;
const byte charHeart = 4;
const byte charBullet = 5;
const byte charEmpty = 32;

// bitmaps
byte bmpPlayer[8] = {
  B01110,
  B01110,
  B00100,
  B01110,
  B10101,
  B00100,
  B01010,
  B10001
};

byte bmpFire[8] = {
  B00100,
  B00100,
  B01010,
  B01010,
  B10101,
  B10101,
  B11111,
  B01110
};

byte bmpCup[8] = {
  B00000,
  B11111,
  B10001,
  B11111,
  B00100,
  B00100,
  B01110,
  B00000
};

byte bmpStalactite[8] = {
  B11111,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000,
  B00000
};

byte bmpHeart[8] = {
  B00000,
  B01010,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};

byte bmpBullet[8] = {
  B00000,
  B00100,
  B01110,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};

// -- global variables --
const int stateMenu = 0;
const int statePlaying = 1;
const int stateGameOver = 2;
const int stateScores = 3;
const int stateFormatting = 4;
const int stateAbout = 5; 
const int stateResetMsg = 6; 
const int stateDrawing = 7;
const int stateUltrasonic = 8;
const int stateRoguelike = 9;
int gameState = stateMenu;
const unsigned int initialScore = 0;
const byte playerStartRow = 1;
const bool initialJumpState = false;
const byte initialHazardCooldown = 0;
const byte initialLandingLag = 0;

char gameMap[lcdRows][lcdCols];
int playerRow = 1;
int gameScore = 0;
unsigned long lastMoveTime = 0;
int currentSpeed = 0;

bool isJumping = false;
byte jumpSteps = 0;   
byte landingLag= 0;  
const byte floorRow = 1;
const byte landingPenalty = 1;
const byte jumpReset = 0;

bool gameChanged = true; 
byte hazardCooldown = 0;

int menuSelection = 0;
unsigned long lastMenuMove = 0;

bool lastJoystickButtonState = HIGH;
unsigned long lastJoystickDebounceTime = 0;
bool lastMenuReturnButtonState = HIGH;
unsigned long lastMenuReturnButtonDebounceTime = 0;
unsigned long toneEnd = 0;
unsigned long stateStart = 0;
unsigned long buttonPressStartTime = 0;

bool waitForRelease = false; 
const unsigned int cupScoreBonus = 10;

LiquidCrystal lcd(rsPin, enPin, d4Pin, d5Pin, d6Pin, d7Pin);
LedControl matrixController = LedControl(matrixDinPin, matrixClkPin, matrixCsPin, matrixDevices);

const byte pixelStateOff = 0;
const byte pixelStateOn = 1;
const byte pixelStateBlinkFast = 2;
const byte pixelStateBlinkSlow = 3;

byte matrixPixelState[ledMatrixSize][ledMatrixSize];
byte cursorRow = 0;
byte cursorColumn = 0;
unsigned long lastDrawMoveTime = 0;
unsigned long lastCursorBlinkTime = 0;
bool cursorBlinkOn = true;
bool matrixNeedsRefresh = false;
unsigned long drawButtonHoldStartTime = 0;
bool drawButtonHeld = false;
bool drawClearTriggered = false;
bool drawExitTriggered = false;
unsigned long lastFastBlinkToggle = 0;
unsigned long lastSlowBlinkToggle = 0;
bool fastBlinkVisible = true;
bool slowBlinkVisible = true;

struct UltrasonicBaseline {
  bool ready = false;
  unsigned long startTime = 0;
  float distanceSum = 0;
  unsigned int samples = 0;
  float averageDistance = 0;
};
UltrasonicBaseline ultrasonicBaseline;
unsigned long lastUltrasonicDisplayUpdate = 0;
unsigned long ultrasonicReadyMessageStartTime = 0;
bool ultrasonicShowingReadyMessage = false;
float currentUltrasonicDistance = 0;

byte roguelikeWorld[roguelikeWorldSize][roguelikeWorldSize];
int roguelikePlayerX = 12;
int roguelikePlayerY = 20;
int roguelikeHealth = roguelikeInitialHealth;
int roguelikeAmmo = roguelikeInitialAmmo;
unsigned long roguelikeGameStartTime = 0;
unsigned long roguelikeLastMoveTime = 0;
unsigned long roguelikeLastTimerUpdate = 0;
int roguelikeElapsedSeconds = 0;

// Function predeclarations 
void playTone(int freq, int dur);
void saveScore(int newScore);
void resetHighScores();
void printScore(int addr, int rank);
void showScores(unsigned long now);
void showAbout(unsigned long now); 
void drawGameOver();
void drawGame();
void drawMenu();
void generateCol(int col);
void checkCollision();
void updateGame(unsigned long currentTime, bool isJumpInputActive);
void startGame(unsigned long now);
void changeState(int newState, unsigned long currentTime);
void initMatrix();
void clearMatrixBuffer();
void renderMatrixFrame();
void enterDrawingMode(unsigned long currentTime);
void updateDrawingMode(unsigned long currentTime, int horizontalReading, int verticalReading, int buttonValue, bool buttonPressedEvent);
float readUltrasonicDistance();
void collectUltrasonicBaseline();
void enterUltrasonicMode(unsigned long currentTime);
void updateUltrasonicMode(unsigned long currentTime);
void initRoguelikeWorld();
void enterRoguelikeMode(unsigned long currentTime);
void updateRoguelikeMode(unsigned long currentTime, int horizontalReading, int verticalReading);
void renderRoguelikeViewport();
void updateRoguelikeLCD(unsigned long currentTime);

void setup() {
  Serial.begin(baudRate);
  pinMode(joystickButtonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(menuReturnButtonPin, INPUT_PULLUP);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  lcd.begin(lcdCols, lcdRows);
  lcd.createChar(charPlayer, bmpPlayer);
  lcd.createChar(charFire, bmpFire);
  lcd.createChar(charCup, bmpCup);
  lcd.createChar(charStalactite, bmpStalactite);
  lcd.createChar(charHeart, bmpHeart);
  lcd.createChar(charBullet, bmpBullet);
  initMatrix();
  
  randomSeed(analogRead(randomPin));

  for(int r = 0; r < lcdRows; r++) {
    for(int c = 0; c < lcdCols; c++) gameMap[r][c] = charEmpty;
  }

  if (EEPROM.read(eepromResetCodeAddress) != eepromResetCode) {
    lcd.clear();
    lcd.print("Formatting...");
    EEPROM.put(eepromResetCodeAddress, eepromResetCode);
    
    for(int i = 0; i < highScoreCount; i++) {
      int addr = eepromStartAddress + (i * entrySize);
      char name[nameLen] = "CPU";
      int score = 0;
      EEPROM.put(addr, name);
      EEPROM.put(addr + nameLen, score);
    }
    
    gameState = stateFormatting;
    stateStart = millis();
  } else {
    gameState = stateMenu;
  }}

void loop() {
  unsigned long currentTime = millis();
  
  int joystickVerticalReading = analogRead(joystickYAxisPin);
  int joystickHorizontalReading = analogRead(joystickXAxisPin);
  int joystickButtonValue = digitalRead(joystickButtonPin);
  bool joystickButtonPressed = false;
  bool isJumpInputActive = (joystickVerticalReading < joystickThresholdLow) || (joystickButtonValue == LOW);
  
  if (joystickButtonValue == LOW && lastJoystickButtonState == HIGH && (currentTime - lastJoystickDebounceTime > debounceTime)) {
    joystickButtonPressed = true;
    lastJoystickDebounceTime = currentTime;
  }
  lastJoystickButtonState = joystickButtonValue;

  int menuReturnButtonValue = digitalRead(menuReturnButtonPin);
  bool menuReturnButtonPressed = false;
  if (menuReturnButtonValue == LOW && lastMenuReturnButtonState == HIGH && (currentTime - lastMenuReturnButtonDebounceTime > debounceTime)) {
    menuReturnButtonPressed = true;
    lastMenuReturnButtonDebounceTime = currentTime;
  }
  lastMenuReturnButtonState = menuReturnButtonValue;

  if (menuReturnButtonPressed && gameState != stateMenu) {
    playTone(NOTE_C5, toneMenuSelectDur);
    changeState(stateMenu, currentTime);
    return;
  }

  if (currentTime > toneEnd) noTone(buzzerPin);

  switch (gameState) {
    case stateMenu:
      drawMenu();
      
      if (currentTime - lastMenuMove > menuScrollDelay) { 
        if (joystickVerticalReading < joystickThresholdLow) { // UP
           menuSelection--;
           if (menuSelection < 0) menuSelection = menuMaxIndex; 
           gameChanged = true;
           playTone(NOTE_A5, toneMenuMoveDur);
           lastMenuMove = currentTime;
        }
        else if (joystickVerticalReading > joystickThresholdHigh) { // DOWN
           menuSelection++;
           if (menuSelection > menuMaxIndex) menuSelection = 0; 
           gameChanged = true;
           playTone(NOTE_A5, toneMenuMoveDur);
           lastMenuMove = currentTime;
        }
      }

      if (joystickButtonPressed) {
        playTone(NOTE_C6, toneMenuSelectDur);
        if (menuSelection == menuOptionStart) startGame(currentTime);
        else if (menuSelection == menuOptionDraw) changeState(stateDrawing, currentTime);
        else if (menuSelection == menuOptionRoguelike) changeState(stateRoguelike, currentTime);
        else if (menuSelection == menuOptionUltrasonic) changeState(stateUltrasonic, currentTime);
        else if (menuSelection == menuOptionScores) changeState(stateScores, currentTime);
        else if (menuSelection == menuOptionAbout) changeState(stateAbout, currentTime);
      }
      break;

    case statePlaying:
      updateGame(currentTime, isJumpInputActive);
      
      if (gameState == statePlaying && gameChanged) {
          drawGame();
          gameChanged = false;
      }
      
      if ((joystickButtonPressed || (joystickVerticalReading < joystickThresholdLow && !isJumping)) && playerRow == 1 && landingLag == 0) {
          playerRow = 0;
          isJumping = true;
          jumpSteps = jumpDistance;
          playTone(NOTE_E5, toneJumpDur);
          gameChanged = true;
      }
      break;

    case stateGameOver:
      if (gameChanged) {
        drawGameOver();
        gameChanged = false;
      }
      if (joystickButtonPressed && (currentTime - stateStart > stateDelay)) {
        changeState(stateMenu, currentTime);
      }
      break;

    case stateScores:
      showScores(currentTime);
      
      if (waitForRelease) {
        if (joystickButtonValue == HIGH) waitForRelease = false;
        break; 
      }
      
      if (joystickButtonValue == LOW) {
        if (buttonPressStartTime == 0) buttonPressStartTime = currentTime;
        if (currentTime - buttonPressStartTime > resetHoldTime) {
          playTone(NOTE_B4, toneResetDur);
          resetHighScores();
          changeState(stateResetMsg, currentTime); 
        }
      } else {
        if (buttonPressStartTime != 0) {
           if (currentTime - buttonPressStartTime < resetHoldTime) {
             changeState(stateMenu, currentTime);
           }
           buttonPressStartTime = 0;
        }
      }
      break;

    case stateResetMsg:
      if (gameChanged) {
         lcd.setCursor(0, 0);
         lcd.print("Scores Reset!");
         gameChanged = false;
      }
    if (currentTime - stateStart > msgResetDuration) {
      changeState(stateScores, currentTime);
      }
      break;

    case stateAbout: 
      showAbout(currentTime);
      
      if (waitForRelease) {
        if (joystickButtonValue == HIGH) waitForRelease = false;
        break; 
      }

      if (joystickButtonPressed) {
         changeState(stateMenu, currentTime);
      }
      break;

    case stateDrawing:
      if (waitForRelease) {
        if (joystickButtonValue == HIGH) waitForRelease = false;
        break;
      }
      updateDrawingMode(currentTime, joystickHorizontalReading, joystickVerticalReading, joystickButtonValue, joystickButtonPressed);
      break;

    case stateUltrasonic:
      if (waitForRelease) {
        if (joystickButtonValue == HIGH) waitForRelease = false;
        break;
      }
      updateUltrasonicMode(currentTime);
      break;

    case stateRoguelike:
      if (waitForRelease) {
        if (joystickButtonValue == HIGH) waitForRelease = false;
        break;
      }
      updateRoguelikeMode(currentTime, joystickHorizontalReading, joystickVerticalReading);
      break;

    // it's purpose is for the initial formating, when the resetCode is not the same
    case stateFormatting:
      if (currentTime - stateStart > formatHoldTime) {
        changeState(stateMenu, currentTime);
      }
      break;
  }}

void changeState(int newState, unsigned long currentTime) {
  int previousState = gameState;
  gameState = newState;
  stateStart = currentTime;
  lcd.clear();
  gameChanged = true;
  buttonPressStartTime = 0;
  
  if (previousState == stateDrawing && newState != stateDrawing) {
  matrixController.clearDisplay(0);
  }

  if (newState == stateScores || newState == stateMenu || newState == stateAbout || newState == stateDrawing || newState == stateUltrasonic || newState == stateRoguelike) {
    waitForRelease = true;
  } else {
    waitForRelease = false;
  }

  if (newState == stateDrawing) {
    enterDrawingMode(currentTime);
  }
  
  if (newState == stateUltrasonic) {
    enterUltrasonicMode(currentTime);
  }
  
  if (newState == stateRoguelike) {
    enterRoguelikeMode(currentTime);
  }}

void startGame(unsigned long now) {
  changeState(statePlaying, now);
  gameScore = initialScore;
  playerRow = playerStartRow;
  currentSpeed = gameSpeedStart;
  isJumping = false;
  hazardCooldown = initialHazardCooldown;
  landingLag = initialLandingLag;
  
  // Clears the display buffer
  for(int r = 0; r < lcdRows; r++) 
    for(int c = 0; c < lcdCols; c++) 
      gameMap[r][c] = charEmpty;}

void updateGame(unsigned long currentTime, bool isJumpInputActive) {
  if (currentTime - lastMoveTime > currentSpeed) {
    lastMoveTime = currentTime;
    gameScore++;
    gameChanged = true;
    
    if (landingLag > 0) landingLag--;
    
    if (isJumping) {
        // cut jump short on release
  if (!isJumpInputActive) jumpSteps = jumpReset;
        else if (jumpSteps > 0) jumpSteps--;

        // max jump length check
        if (jumpSteps == jumpReset) {
            playerRow = floorRow; 
            isJumping = false;
            landingLag = landingPenalty; 
        }
    } 
    
    if (gameScore % scoreLevel == 0 && currentSpeed > gameSpeedMax) {
      currentSpeed -= speedStep;
    }
    
    for(int r = 0; r < lcdRows; r++) {
      for(int c = 0; c < lcdCols - 1; c++) {
        gameMap[r][c] = gameMap[r][c+1];
      }
    }
    
    generateCol(lcdCols - 1);
    checkCollision();
  }}

void checkCollision() {
    char obj = gameMap[playerRow][playerColumn];
    
    if (obj == charFire || obj == charStalactite) {
      playTone(NOTE_C4, toneGameOverDur);
      saveScore(gameScore);
      changeState(stateGameOver, millis());
    } else if (obj == charCup) {
      gameScore += cupScoreBonus;
      gameMap[playerRow][playerColumn] = charEmpty;
      playTone(NOTE_G5, toneCoinDur);
    }}

void generateCol(int col) {
    gameMap[0][col] = charEmpty;
    gameMap[1][col] = charEmpty;

    if (hazardCooldown > 0) {
        hazardCooldown--;
        if (random(0, randomResolution) < probHazardCup) gameMap[1][col] = charCup;
        return;
    }

    int r = random(0, randomResolution);
    if (r < probCoinAir) gameMap[0][col] = charCup;
    else if (r < probCoinGround) gameMap[1][col] = charCup;
    else if (r < probSpike) {
        gameMap[1][col] = charFire;
        hazardCooldown = cooldownTime;
    }
    else if (r < probStalactite) {
        gameMap[0][col] = charStalactite;
        hazardCooldown = cooldownTime;
    }}

void drawMenu() {
  if (!gameChanged) return;
  lcd.setCursor(0, 0); lcd.print("ENDLESS RUNNER");
  lcd.setCursor(0, 1);
  
  if (menuSelection == menuOptionStart) lcd.print("> START         ");
  else if (menuSelection == menuOptionDraw) lcd.print("> DRAW MATRIX   ");
  else if (menuSelection == menuOptionRoguelike) lcd.print("> ROGUELIKE     ");
  else if (menuSelection == menuOptionUltrasonic) lcd.print("> ULTRASONIC    ");
  else if (menuSelection == menuOptionScores) lcd.print("> SCORES        ");
  else if (menuSelection == menuOptionAbout) lcd.print("> ABOUT CREATOR ");
  
  gameChanged = false;}

void initMatrix() {
  matrixController.shutdown(0, false);
  matrixController.setIntensity(0, matrixDefaultIntensity);
  matrixController.clearDisplay(0);
  clearMatrixBuffer();
}

void clearMatrixBuffer() {
  for (byte row = 0; row < ledMatrixSize; row++) {
    for (byte col = 0; col < ledMatrixSize; col++) {
      matrixPixelState[row][col] = pixelStateOff;
    }
  }
  matrixController.clearDisplay(0);
  matrixNeedsRefresh = true;
}

void renderMatrixFrame() {
  for (byte row = 0; row < ledMatrixSize; row++) {
    for (byte col = 0; col < ledMatrixSize; col++) {
      bool pixelOn = false;
      byte state = matrixPixelState[row][col];
      if (state == pixelStateOn) pixelOn = true;
      else if (state == pixelStateBlinkFast) pixelOn = fastBlinkVisible;
      else if (state == pixelStateBlinkSlow) pixelOn = slowBlinkVisible;

      if (row == cursorRow && col == cursorColumn && cursorBlinkOn) {
        pixelOn = true;
      }
      matrixController.setLed(0, row, col, pixelOn);
    }
  }
}

void enterDrawingMode(unsigned long currentTime) {
  clearMatrixBuffer();
  cursorRow = 0;
  cursorColumn = 0;
  lastDrawMoveTime = currentTime;
  lastCursorBlinkTime = currentTime;
  lastFastBlinkToggle = currentTime;
  lastSlowBlinkToggle = currentTime;
  cursorBlinkOn = true;
  fastBlinkVisible = true;
  slowBlinkVisible = true;
  matrixNeedsRefresh = true;
  drawButtonHoldStartTime = 0;
  drawButtonHeld = false;
  drawClearTriggered = false;
  drawExitTriggered = false;
}

void updateDrawingMode(unsigned long currentTime, int horizontalReading, int verticalReading, int buttonValue, bool buttonPressedEvent) {
  if (gameChanged) {
    lcd.setCursor(0, 0); lcd.print("Draw: tap paint");
    lcd.setCursor(0, 1); lcd.print("1s clr 2.5s exit");
    gameChanged = false;
  }

  int rowStep = 0;
  int columnStep = 0;
  if (horizontalReading < joystickThresholdLow) rowStep = -1;
  else if (horizontalReading > joystickThresholdHigh) rowStep = 1;
  if (verticalReading < joystickThresholdLow) columnStep = -1;
  else if (verticalReading > joystickThresholdHigh) columnStep = 1;

  if ((rowStep != 0 || columnStep != 0) && (currentTime - lastDrawMoveTime > drawMoveDelay)) {
    cursorRow = constrain(cursorRow + rowStep, 0, ledMatrixSize - 1);
   cursorColumn = constrain(cursorColumn + columnStep, 0, ledMatrixSize - 1);
    lastDrawMoveTime = currentTime;
    matrixNeedsRefresh = true;
  }

  if (buttonPressedEvent && !drawExitTriggered) {
    byte currentState = matrixPixelState[cursorRow][cursorColumn];
    currentState = (currentState + 1) % 4;
    matrixPixelState[cursorRow][cursorColumn] = currentState;
    matrixNeedsRefresh = true;
    playTone(NOTE_G4, toneMenuMoveDur);
  }

  if (buttonValue == LOW) {
    if (!drawButtonHeld) {
      drawButtonHeld = true;
  drawButtonHoldStartTime = currentTime;
      drawClearTriggered = false;
      drawExitTriggered = false;
    } else {
  unsigned long heldDuration = currentTime - drawButtonHoldStartTime;
      if (!drawClearTriggered && heldDuration >= drawClearHoldTime) {
        clearMatrixBuffer();
        drawClearTriggered = true;
        playTone(NOTE_A4, toneMenuMoveDur);
      }
      if (!drawExitTriggered && heldDuration >= drawExitHoldTime) {
        drawExitTriggered = true;
        playTone(NOTE_C6, toneMenuSelectDur);
        changeState(stateMenu, currentTime);
        return;
      }
    }
  } else {
    drawButtonHeld = false;
  }

  if (currentTime - lastCursorBlinkTime > drawBlinkInterval) {
    lastCursorBlinkTime = currentTime;
    cursorBlinkOn = !cursorBlinkOn;
    matrixNeedsRefresh = true;
  }

  if (currentTime - lastFastBlinkToggle > fastBlinkInterval) {
    lastFastBlinkToggle = currentTime;
    fastBlinkVisible = !fastBlinkVisible;
    matrixNeedsRefresh = true;
  }

  if (currentTime - lastSlowBlinkToggle > slowBlinkInterval) {
    lastSlowBlinkToggle = currentTime;
    slowBlinkVisible = !slowBlinkVisible;
    matrixNeedsRefresh = true;
  }

  if (matrixNeedsRefresh) {
    renderMatrixFrame();
    matrixNeedsRefresh = false;
  }
}

void drawGame() {
  for(int r = 0; r < lcdRows; r++) {
    lcd.setCursor(0, r);
    for(int c = 0; c < lcdCols; c++) {
      if (c == playerColumn && r == playerRow) {
        lcd.write((byte)charPlayer);
      } else {
        lcd.write(gameMap[r][c]);
      }
    }
  }
  lcd.setCursor(scoreCol, 0); lcd.print(gameScore);}

void drawGameOver() {
  lcd.setCursor(0, 0); lcd.print("GAME OVER!      ");
  lcd.setCursor(0, 1); lcd.print("Score: "); lcd.print(gameScore);}

void showScores(unsigned long now) {
  int page = (now / scorePageTime) % totalScorePages;
  static int lastPage = -1;
  
  if (page != lastPage) {
      lcd.clear();
      lastPage = page;
      if (page == 0) {
          lcd.setCursor(0,0); 
          lcd.print("1."); 
          printScore(eepromStartAddress, 1);
          lcd.setCursor(0,1); 
          lcd.print("2."); 
          printScore(eepromStartAddress + entrySize, 2);
      } else {
          lcd.setCursor(0,0); 
          lcd.print("3."); 
          printScore(eepromStartAddress + (entrySize * 2), 3);
      }
  }}

void showAbout(unsigned long now) {
  int page = (now / aboutPageTime) % totalAboutPages; 
  static int lastPage = -1;

  if (page != lastPage) {
    lcd.clear();
    lastPage = page;
    
    lcd.setCursor(0, 0); 
    lcd.print("Sescu Matei");

    lcd.setCursor(0, 1);
    if (page == aboutPageIndexRobotics) {
      lcd.print("robotics"); 
    } else {
      lcd.print("enthusiast");
    }
  }}

void printScore(int addr, int rank) {
  char name[nameLen];
  int score;
  EEPROM.get(addr, name);
  EEPROM.get(addr + nameLen, score);
  
  if (name[0] < asciiMin || name[0] > asciiMax) lcd.print("???");
  else lcd.print(name);
  lcd.print(" "); lcd.print(score);}

void playTone(int frequency, int duration) {
  tone(buzzerPin, frequency);
  toneEnd = millis() + duration;}

void resetHighScores() {
  lcd.clear();
  lcd.print("Formatting...");
  EEPROM.put(eepromResetCodeAddress, eepromResetCode);
  for(int i = 0; i < highScoreCount; i++) {
    int addr = eepromStartAddress + (i * entrySize);
    char name[nameLen] = "CPU";
    int score = 0;
    EEPROM.put(addr, name);
    EEPROM.put(addr + nameLen, score);
  }}

void saveScore(int newScore) {
  int scores[highScoreCount];
  
  for(int i = 0; i < highScoreCount; i++) {
    int val = 0;
    EEPROM.get(eepromStartAddress + (i * entrySize) + nameLen, val);
    if(val < 0) val = 0;
    scores[i] = val;
  }

  if (newScore <= scores[highScoreCount - 1]) return;

  scores[highScoreCount - 1] = newScore;
  for(int i = 0; i < highScoreCount - 1; i++) {
    for(int j = 0; j < highScoreCount - 1 - i; j++) {
      if(scores[j] < scores[j + 1]) {
        int temp = scores[j];
        scores[j] = scores[j + 1];
        scores[j + 1] = temp;
      }
    }
  }

  for(int i = 0; i < highScoreCount; i++) {
    int addr = eepromStartAddress + (i * entrySize);
    char name[nameLen] = "YOU";
    EEPROM.put(addr, name);
    EEPROM.put(addr + nameLen, scores[i]);
  }}

float readUltrasonicDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, echoTimeout);
  if (duration == 0) return -1;

  return duration / microsecondsPerCm;
}

void collectUltrasonicBaseline() {
  unsigned long now = millis();
  if (now - ultrasonicBaseline.startTime <= ultrasonicBaselineSampleTime) {
    float distance = readUltrasonicDistance();
    if (distance > 0 && distance < 400) {
      ultrasonicBaseline.distanceSum += distance;
      ultrasonicBaseline.samples++;
    }
  } else if (!ultrasonicBaseline.ready && ultrasonicBaseline.samples > 0) {
    ultrasonicBaseline.averageDistance = ultrasonicBaseline.distanceSum / ultrasonicBaseline.samples;
    ultrasonicBaseline.ready = true;
    ultrasonicShowingReadyMessage = true;
    ultrasonicReadyMessageStartTime = now;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Baseline: ");
    lcd.print(ultrasonicBaseline.averageDistance, 1);
    lcd.print(" cm");
    lcd.setCursor(0, 1);
    lcd.print("System ready!");
  }
}

void enterUltrasonicMode(unsigned long currentTime) {
  ultrasonicBaseline.ready = false;
  ultrasonicBaseline.startTime = currentTime;
  ultrasonicBaseline.distanceSum = 0;
  ultrasonicBaseline.samples = 0;
  ultrasonicBaseline.averageDistance = 0;
  lastUltrasonicDisplayUpdate = currentTime;
  currentUltrasonicDistance = 0;
  gameChanged = true;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrating...");
}

void updateUltrasonicMode(unsigned long currentTime) {
  if (!ultrasonicBaseline.ready) {
    collectUltrasonicBaseline();
    return;
  }
  
  if (ultrasonicShowingReadyMessage) {
    if (currentTime - ultrasonicReadyMessageStartTime >= ultrasonicReadyMessageDuration) {
      ultrasonicShowingReadyMessage = false;
      gameChanged = true;
    }
    return;
  }

  if (currentTime - lastUltrasonicDisplayUpdate >= ultrasonicDisplayUpdateInterval) {
    lastUltrasonicDisplayUpdate = currentTime;
    currentUltrasonicDistance = readUltrasonicDistance();
    
    if (gameChanged) {
      lcd.clear();
      gameChanged = false;
    }
    
    lcd.setCursor(0, 0);
    lcd.print("Distance: ");
    if (currentUltrasonicDistance > 0 && currentUltrasonicDistance < 400) {
      lcd.print(currentUltrasonicDistance, 1);
      lcd.print(" cm  ");
    } else {
      lcd.print("Out range");
    }
    
    lcd.setCursor(0, 1);
    lcd.print("Base: ");
    lcd.print(ultrasonicBaseline.averageDistance, 1);
    lcd.print(" cm  ");
  }
}

void initRoguelikeWorld() {
  // Clear world
  for (int y = 0; y < roguelikeWorldSize; y++) {
    for (int x = 0; x < roguelikeWorldSize; x++) {
      roguelikeWorld[y][x] = roguelikeTileEmpty;
    }
  }
  
  // Add border walls
  for (int i = 0; i < roguelikeWorldSize; i++) {
    roguelikeWorld[0][i] = roguelikeTileWall;
    roguelikeWorld[roguelikeWorldSize - 1][i] = roguelikeTileWall;
    roguelikeWorld[i][0] = roguelikeTileWall;
    roguelikeWorld[i][roguelikeWorldSize - 1] = roguelikeTileWall;
  }
  
  // Add some interior walls for interest
  for (int i = 5; i < 10; i++) {
    roguelikeWorld[5][i] = roguelikeTileWall;
    roguelikeWorld[18][i] = roguelikeTileWall;
  }
  for (int i = 15; i < 20; i++) {
    roguelikeWorld[i][12] = roguelikeTileWall;
  }
}

void enterRoguelikeMode(unsigned long currentTime) {
  initRoguelikeWorld();
  roguelikePlayerX = 12;
  roguelikePlayerY = 20;
  roguelikeHealth = roguelikeInitialHealth;
  roguelikeAmmo = roguelikeInitialAmmo;
  roguelikeGameStartTime = currentTime;
  roguelikeLastMoveTime = currentTime;
  roguelikeLastTimerUpdate = currentTime;
  roguelikeElapsedSeconds = 0;
  gameChanged = true;
}

void renderRoguelikeViewport() {
  // Calculate camera position (center on player)
  int cameraX = roguelikePlayerX - (roguelikeViewportSize / 2);
  int cameraY = roguelikePlayerY - (roguelikeViewportSize / 2);
  
  // Clamp camera to world bounds
  if (cameraX < 0) cameraX = 0;
  if (cameraY < 0) cameraY = 0;
  if (cameraX > roguelikeWorldSize - roguelikeViewportSize) {
    cameraX = roguelikeWorldSize - roguelikeViewportSize;
  }
  if (cameraY > roguelikeWorldSize - roguelikeViewportSize) {
    cameraY = roguelikeWorldSize - roguelikeViewportSize;
  }
  
  // Render 8x8 viewport to matrix
  for (int viewY = 0; viewY < roguelikeViewportSize; viewY++) {
    for (int viewX = 0; viewX < roguelikeViewportSize; viewX++) {
      int worldX = cameraX + viewX;
      int worldY = cameraY + viewY;
      
      bool ledOn = false;
      
      // Check if player is at this position
      if (worldX == roguelikePlayerX && worldY == roguelikePlayerY) {
        ledOn = true;
      }
      // Check world tile
      else if (roguelikeWorld[worldY][worldX] == roguelikeTileWall) {
        ledOn = true;
      }
      
      matrixController.setLed(0, viewY, viewX, ledOn);
    }
  }
}

void updateRoguelikeLCD(unsigned long currentTime) {
  // Update timer
  if (currentTime - roguelikeLastTimerUpdate >= roguelikeTimerUpdateInterval) {
    roguelikeLastTimerUpdate = currentTime;
    roguelikeElapsedSeconds = (currentTime - roguelikeGameStartTime) / 1000;
  }
  
  // Row 0: Heart icon + health, spaces, timer
  lcd.setCursor(0, 0);
  lcd.write((byte)charHeart);
  if (roguelikeHealth < 10) lcd.print(" ");
  lcd.print(roguelikeHealth);
  
  lcd.print("      ");
  
  int minutes = roguelikeElapsedSeconds / 60;
  int seconds = roguelikeElapsedSeconds % 60;
  if (minutes < 10) lcd.print("0");
  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);
  
  // Row 1: Bullet icon + ammo
  lcd.setCursor(0, 1);
  lcd.write((byte)charBullet);
  if (roguelikeAmmo < 10) lcd.print(" ");
  lcd.print(roguelikeAmmo);
  lcd.print("            ");
}

void updateRoguelikeMode(unsigned long currentTime, int horizontalReading, int verticalReading) {
  // Handle movement
  if (currentTime - roguelikeLastMoveTime >= roguelikeMovementDelay) {
    int newX = roguelikePlayerX;
    int newY = roguelikePlayerY;
    bool moved = false;
    
    if (horizontalReading < joystickThresholdLow) {
      newX--;
      moved = true;
    } else if (horizontalReading > joystickThresholdHigh) {
      newX++;
      moved = true;
    }
    
    if (verticalReading < joystickThresholdLow) {
      newY--;
      moved = true;
    } else if (verticalReading > joystickThresholdHigh) {
      newY++;
      moved = true;
    }
    
    // Check boundaries and collisions
    if (moved) {
      if (newX >= 0 && newX < roguelikeWorldSize && 
          newY >= 0 && newY < roguelikeWorldSize &&
          roguelikeWorld[newY][newX] != roguelikeTileWall) {
        roguelikePlayerX = newX;
        roguelikePlayerY = newY;
        roguelikeLastMoveTime = currentTime;
        renderRoguelikeViewport();
      }
    }
  }
  
  // Update LCD display
  updateRoguelikeLCD(currentTime);
}