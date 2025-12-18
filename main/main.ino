// matrixology
// roguelike game on 8x8 led matrix (with easy upgraade path to larger matrices)
// joystick to move, press button for menu
// cover photoresistor for burst attack
// high scores saved to eeprom

#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <LedControl.h>

#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_C4 262
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_E5 659
#define NOTE_G5 784
#define NOTE_A5 880
#define NOTE_C6 1047

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
const int photoResistorPin = A2;

// matrix constants
const byte ledMatrixSize = 8;
const byte matrixDefaultIntensity = 5;

// game constants
const int lcdCols = 16;
const int lcdRows = 2;

// timing constants
const int debounceTime = 200;
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
const int lightSensorUpdateInterval = 100;
const int lightSensorDisplayInterval = 200;
const int roguelikeMovementDelay = 200;
const int roguelikeAutoShootDelay = 300;
const int howToPlayScrollDelay = 3000;
const unsigned long abilityCooldown = 20000;
int darkThreshold = 400;
const int oneSecondMs = 1000;
const int numDirections = 8;
const int introHeartCol = 7;
const byte invalidStage = 255;
const int gameOverFinalToneDuration = 300;
const int gameOverShortStepDuration = 100;
const int gameOverNoHighScoreStep = 10;
const int paddingThreshold10 = 10;
const int paddingThreshold100 = 100;

// joystick thresholds
const int joystickThresholdLow = 200;
const int joystickThresholdHigh = 800;

// roguelike constants
const int roguelikeWorldSize = 24;
const int roguelikeViewportSize = 8;
const int roguelikeInitialHealth = 10;
const int roguelikeInitialAmmo = 30;
const byte roguelikeTileEmpty = 0;
const byte roguelikeTileWall = 1;
const byte roguelikeTilePlayer = 2;
const byte roguelikeTileEnemy = 3;

// other constants
const int matrixDeviceCount = 1;
const int matrixDeviceIndex = 0;
const int minDarkThreshold = 100;
const int maxDarkThreshold = 900;
const int defaultDarkThreshold = 400;
const int thresholdStep = 50;
const int initialMaxEnemies = 4;
const int introNoteDuration = 100;
const int introDuration = 5000;
const int heartbeatInterval = 400;
const int heartbeatStages = 3;
const int maxTutorialPage = 3;
const int toneSettingAdjustDur = 30;
const int pixelStateCount = 4;
const int minButtonHoldForShoot = 50;
const int gameOverToneDuration = 200;
const int gameOverStepDuration = 150;
const int gameOverLongToneDuration = 500;
const int gameOverDuration = 3000;
const int gameOverDurationNoHighScore = 2000;
const int pillarCount = 15;
const int wallPadding = 2;
const int initialEnemyCount = 4;
const int playerBlinkInterval = 100;
const int scorePerLevel = 50;
const int healthGainOnLevelUp = 2;
const int levelUpToneDuration = 200;
const int levelUpStepDuration = 50;
const int maxSpawnAttempts = 20;
const int minEnemySpawnDistance = 6;
const int enemySpawnToneDuration = 100;
const int noAmmoToneDuration = 80;
const int shootToneDuration = 40;
const int scorePerKill = 10;
const int killToneDuration = 80;
const int burstToneDuration = 60;
const int damageToneDuration = 250;
const int abilityTriggerToneDuration = 50;
const int abilityReadyToneDuration = 50;
const int moveToneDuration = 20;
const int roguelikeMenuHoldTime = 1000;
const byte pixelStateOff = 0;
const byte pixelStateOn = 1;
const byte pixelStateBlinkFast = 2;
const byte pixelStateBlinkSlow = 3;

// enemy and bullet constants
const byte maxEnemies = 12;
const byte maxBullets = 12;
byte currentMaxEnemies = initialMaxEnemies;
const int enemyMoveDelay = 800;
const int bulletMoveDelay = 150;
const int enemySpawnDelay = 5000;
const int damageInvincibilityTime = 1000;

// menu options
const int menuOptionStart = 0;
const int menuOptionHowToPlay = 1;
const int menuOptionScores = 2;
const int menuOptionSettings = 3;
const int menuOptionAbout = 4;
const int menuOptionLightSensor = 5;
const int menuOptionDraw = 6;
const int menuMaxIndex = 6;

// settings submenu
const int settingThreshold = 0;
const int settingResetScores = 1;
const int settingBack = 2;
const int settingsMaxIndex = 2;

enum GameState : byte {
  stateIntro,
  stateMenu,
  stateHowToPlay,
  stateScores,
  stateResetMsg,
  stateAbout,
  stateSettings,
  stateLightSensor,
  stateDrawing,
  stateRoguelike,
  stateFormatting,
  stateGameOver,
  stateScoreResetMsg
};

// page counts for menus
const int totalScorePages = 2;
const int totalAboutPages = 2;

// sound durations
const int toneMenuMoveDur = 10;
const int toneMenuSelectDur = 50;
const int toneResetDur = 150;

// eeprom
const int eepromStartAddress = 10;
const int eepromThresholdAddress = 30;
const int eepromResetCode = 148;
const int eepromResetCodeAddress = 0;
const int highScoreCount = 4;
const int entrySize = 2;

// lcd characters
const byte charHeartSmall = 4;
const byte charHeart = 5;
const byte charHeartBig = 6;

// heart animation frames
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

byte bmpHeartSmall[8] = {
  B00000,
  B00000,
  B01010,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};

byte bmpHeartBig[8] = {
  B01010,
  B11111,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};

// global variables
GameState gameState = stateIntro;

bool gameChanged = true;

int menuSelection = 0;
int settingsSelection = 0;
unsigned long lastMenuMove = 0;

bool lastJoystickButtonState = HIGH;
unsigned long lastJoystickDebounceTime = 0;
bool lastMenuReturnButtonState = HIGH;
unsigned long lastMenuReturnButtonDebounceTime = 0;
unsigned long toneEnd = 0;
unsigned long stateStart = 0;
unsigned long buttonPressStartTime = 0;
unsigned long roguelikeButtonHoldStart = 0;

bool waitForRelease = false;

// intro variables
byte introSoundStep = 0;
unsigned long introSoundTime = 0;
byte gameOverStep = 0;
unsigned long gameOverStepTime = 0;
bool gameOverIsHighScore = false;
byte levelUpStep = 0;
unsigned long levelUpStepTime = 0;

// drawing mode
LiquidCrystal lcd(rsPin, enPin, d4Pin, d5Pin, d6Pin, d7Pin);
LedControl matrixController = LedControl(matrixDinPin, matrixClkPin, matrixCsPin, matrixDeviceCount);

// drawing mode variables
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

// light sensor
int currentLightLevel = 0;
unsigned long lastLightDisplayUpdate = 0;

// ability
unsigned long lastAbilityUse = 0;
bool abilityReady = true;

byte roguelikeWorld[roguelikeWorldSize][roguelikeWorldSize];
int roguelikePlayerX = 0;
int roguelikePlayerY = 0;
int roguelikeHealth = roguelikeInitialHealth;
int roguelikeAmmo = roguelikeInitialAmmo;
int roguelikeScore = 0;
int roguelikeLevel = 1;
unsigned long roguelikeLastMoveTime = 0;
unsigned long roguelikeLastDamageTime = 0;
unsigned long roguelikeLastEnemySpawn = 0;
unsigned long roguelikeLastShootTime = 0;

// enemies
struct Enemy {
  byte positionX, positionY;
  bool active;
  unsigned long lastMoveTime;
};
Enemy enemies[maxEnemies];

struct Bullet {
  int positionX, positionY;
  int directionX, directionY;
  bool active;
  unsigned long lastMoveTime;
};
Bullet bullets[maxBullets];
int lastShootDirX = -1;
int lastShootDirY = -1;

// function declarations
void playTone(int toneFrequency, int toneDuration);
void saveScore(int newScore);
void resetHighScores();
void printScore(int eepromAddress);
void showScores(unsigned long currentTime);
void showAbout(unsigned long currentTime);
void drawMenu();
void changeState(GameState newState, unsigned long currentTime);
void initMatrix();
void clearMatrixBuffer();
void renderMatrixFrame();
void enterDrawingMode(unsigned long currentTime);
void updateDrawingMode(unsigned long currentTime, int horizontalReading, int verticalReading, int buttonValue, bool buttonPressedEvent);
int readLightLevel();
void enterLightSensorMode(unsigned long currentTime);
void updateLightSensorMode(unsigned long currentTime);
void checkAbilityTrigger(unsigned long currentTime);
void updateAbility(unsigned long currentTime);
void initRoguelikeWorld();
void enterRoguelikeMode(unsigned long currentTime);
void updateRoguelikeMode(unsigned long currentTime, int horizontalReading, int verticalReading);
void renderRoguelikeViewport();
void updateRoguelikeLCD(unsigned long currentTime);
void spawnEnemy();
void updateEnemies(unsigned long currentTime);
void moveEnemyTowardsPlayer(byte enemyIndex);
void shootBullet();
void shootBurst();
void updateBullets(unsigned long currentTime);
void playerTakeDamage(unsigned long currentTime);
void roguelikeGameOver(unsigned long currentTime);

void setup() {
  pinMode(joystickButtonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(menuReturnButtonPin, INPUT_PULLUP);

  lcd.begin(lcdCols, lcdRows);
  lcd.createChar(charHeartSmall, bmpHeartSmall);
  lcd.createChar(charHeart, bmpHeart);
  lcd.createChar(charHeartBig, bmpHeartBig);
  initMatrix();

  randomSeed(analogRead(randomPin));

  if (EEPROM.read(eepromResetCodeAddress) != eepromResetCode) {
    resetHighScores();
    EEPROM.put(eepromThresholdAddress, darkThreshold);
    gameState = stateFormatting;
    stateStart = millis();
  } else {
    EEPROM.get(eepromThresholdAddress, darkThreshold);
    if (darkThreshold < minDarkThreshold || darkThreshold > maxDarkThreshold) darkThreshold = defaultDarkThreshold;
    gameState = stateIntro;
    stateStart = millis();
  }
}

void loop() {
  unsigned long currentTime = millis();

  int joystickVerticalReading = analogRead(joystickYAxisPin);
  int joystickHorizontalReading = analogRead(joystickXAxisPin);
  int joystickButtonValue = digitalRead(joystickButtonPin);
  bool joystickButtonPressed = false;

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
    case stateIntro:
      {
        if (gameChanged) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F(" MATRIXOLOGY "));

          // border on matrix
          for (byte ledIndex = 0; ledIndex < ledMatrixSize; ledIndex++) {
            matrixController.setLed(matrixDeviceIndex, 0, ledIndex, true);
            matrixController.setLed(matrixDeviceIndex, ledMatrixSize - 1, ledIndex, true);
            matrixController.setLed(matrixDeviceIndex, ledIndex, 0, true);
            matrixController.setLed(matrixDeviceIndex, ledIndex, ledMatrixSize - 1, true);
          }

          introSoundStep = 0;
          introSoundTime = currentTime;
          gameChanged = false;
        }

        // intro melody
        if (introSoundStep == 0) {
          playTone(NOTE_C5, introNoteDuration);
          introSoundStep = 1;
          introSoundTime = currentTime;
        } else if (introSoundStep == 1 && currentTime - introSoundTime >= introNoteDuration) {
          playTone(NOTE_E5, introNoteDuration);
          introSoundStep = 2;
        }

        // heartbeat animation (3 frames: small, medium, big)
        static byte lastHeartStage = invalidStage;
        byte heartStage = (currentTime / heartbeatInterval) % heartbeatStages;

        if (lastHeartStage != heartStage) {
          lastHeartStage = heartStage;
          lcd.setCursor(introHeartCol, 1);
          if (heartStage == 0) lcd.write((byte)charHeartSmall);
          else if (heartStage == 1) lcd.write((byte)charHeart);
          else lcd.write((byte)charHeartBig);
        }

        if (joystickButtonPressed || (currentTime - stateStart > introDuration)) {
          changeState(stateMenu, currentTime);
        }
        break;
      }

    case stateHowToPlay:
      {
        static int tutorialPage = 0;
        static unsigned long lastTutorialScroll = 0;

        if (gameChanged) {
          tutorialPage = 0;
          lastTutorialScroll = currentTime;
          gameChanged = false;
        }

        if (currentTime - lastTutorialScroll >= howToPlayScrollDelay) {
          lastTutorialScroll = currentTime;
          tutorialPage++;
          if (tutorialPage > maxTutorialPage) tutorialPage = 0;
        }

        switch (tutorialPage) {
          case 0:
            lcd.setCursor(0, 0);
            lcd.print(F("Joystick: Move  "));
            lcd.setCursor(0, 1);
            lcd.print(F("Press to shoot! "));
            break;
          case 1:
            lcd.setCursor(0, 0);
            lcd.print(F("Cover sensor for"));
            lcd.setCursor(0, 1);
            lcd.print(F("8-WAY BURST!    "));
            break;
          case 2:
            lcd.setCursor(0, 0);
            lcd.print(F("Ability cooldown"));
            lcd.setCursor(0, 1);
            lcd.print(F("is 20 seconds   "));
            break;
          case 3:
            lcd.setCursor(0, 0);
            lcd.print(F("Kill enemies!   "));
            lcd.setCursor(0, 1);
            lcd.print(F("                "));
            break;
        }

        if (joystickButtonPressed) {
          changeState(stateMenu, currentTime);
        }
        break;
      }

    case stateMenu:
      if (waitForRelease) {
        if (joystickButtonValue == HIGH) waitForRelease = false;
        drawMenu();
        break;
      }

      drawMenu();

      if (currentTime - lastMenuMove > menuScrollDelay) {
        if (joystickVerticalReading < joystickThresholdLow) {
          menuSelection--;
          if (menuSelection < 0) menuSelection = menuMaxIndex;
          gameChanged = true;
          playTone(NOTE_A5, toneMenuMoveDur);
          lastMenuMove = currentTime;
        } else if (joystickVerticalReading > joystickThresholdHigh) {
          menuSelection++;
          if (menuSelection > menuMaxIndex) menuSelection = 0;
          gameChanged = true;
          playTone(NOTE_A5, toneMenuMoveDur);
          lastMenuMove = currentTime;
        }
      }

      if (joystickButtonPressed) {
        playTone(NOTE_C6, toneMenuSelectDur);
        switch (menuSelection) {
          case menuOptionStart: changeState(stateRoguelike, currentTime); break;
          case menuOptionHowToPlay: changeState(stateHowToPlay, currentTime); break;
          case menuOptionScores: changeState(stateScores, currentTime); break;
          case menuOptionSettings: changeState(stateSettings, currentTime); break;
          case menuOptionAbout: changeState(stateAbout, currentTime); break;
          case menuOptionLightSensor: changeState(stateLightSensor, currentTime); break;
          case menuOptionDraw: changeState(stateDrawing, currentTime); break;
        }
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
        lcd.print(F("Scores Reset!"));
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

    case stateSettings:
      if (waitForRelease) {
        if (joystickButtonValue == HIGH) waitForRelease = false;
        break;
      }

      if (gameChanged) {
        settingsSelection = 0;
        gameChanged = false;
      }

      lcd.setCursor(0, 0);
      lcd.print(F("   SETTINGS     "));
      lcd.setCursor(0, 1);

      if (settingsSelection == settingThreshold) {
        lcd.print(F("> THRESHOLD:"));
        if (darkThreshold < minDarkThreshold) lcd.print(' ');
        lcd.print(darkThreshold);
      } else if (settingsSelection == settingResetScores) {
        lcd.print(F("> RESET SCORES  "));
      } else if (settingsSelection == settingBack) {
        lcd.print(F("> BACK          "));
      }

      if (currentTime - lastMenuMove > menuScrollDelay) {
        if (joystickVerticalReading < joystickThresholdLow) {
          settingsSelection--;
          if (settingsSelection < 0) settingsSelection = settingsMaxIndex;
          playTone(NOTE_A5, toneMenuMoveDur);
          lastMenuMove = currentTime;
        } else if (joystickVerticalReading > joystickThresholdHigh) {
          settingsSelection++;
          if (settingsSelection > settingsMaxIndex) settingsSelection = 0;
          playTone(NOTE_A5, toneMenuMoveDur);
          lastMenuMove = currentTime;
        }

        if (settingsSelection == settingThreshold) {
          if (joystickHorizontalReading < joystickThresholdLow) {
            darkThreshold += thresholdStep;
            if (darkThreshold > maxDarkThreshold) darkThreshold = maxDarkThreshold;
            EEPROM.put(eepromThresholdAddress, darkThreshold);
            playTone(NOTE_G4, toneSettingAdjustDur);
            lastMenuMove = currentTime;
          } else if (joystickHorizontalReading > joystickThresholdHigh) {
            darkThreshold -= thresholdStep;
            if (darkThreshold < minDarkThreshold) darkThreshold = minDarkThreshold;
            EEPROM.put(eepromThresholdAddress, darkThreshold);
            playTone(NOTE_G4, toneSettingAdjustDur);
            lastMenuMove = currentTime;
          }
        }
      }

      if (joystickButtonPressed) {
        if (settingsSelection == settingResetScores) {
          playTone(NOTE_B4, toneResetDur);
          resetHighScores();
          changeState(stateScoreResetMsg, currentTime);
        } else if (settingsSelection == settingBack) {
          playTone(NOTE_C6, toneMenuSelectDur);
          changeState(stateMenu, currentTime);
        }
      }
      break;

    case stateScoreResetMsg:
      if (gameChanged) {
        lcd.setCursor(0, 1);
        lcd.print(F("Scores Reset!   "));
        gameChanged = false;
      }
      if (currentTime - stateStart >= msgResetDuration) {
        changeState(stateSettings, currentTime);
      }
      break;

    case stateDrawing:
      if (waitForRelease) {
        if (joystickButtonValue == HIGH) waitForRelease = false;
        break;
      }
      updateDrawingMode(currentTime, joystickHorizontalReading, joystickVerticalReading, joystickButtonValue, joystickButtonPressed);
      break;

    case stateLightSensor:
      if (waitForRelease) {
        if (joystickButtonValue == HIGH) waitForRelease = false;
        break;
      }
      updateLightSensorMode(currentTime);
      break;

    case stateRoguelike:
      if (waitForRelease) {
        if (joystickButtonValue == HIGH) waitForRelease = false;
        break;
      }

      checkAbilityTrigger(currentTime);
      updateAbility(currentTime);

      // button: press = shoot, hold = menu
      if (joystickButtonValue == LOW) {
        if (roguelikeButtonHoldStart == 0) {
          roguelikeButtonHoldStart = currentTime;
        } else if (currentTime - roguelikeButtonHoldStart >= roguelikeMenuHoldTime) {
          playTone(NOTE_C5, toneMenuSelectDur);
          changeState(stateMenu, currentTime);
          roguelikeButtonHoldStart = 0;
          break;
        }
      } else {
        if (roguelikeButtonHoldStart != 0) {
          unsigned long holdDuration = currentTime - roguelikeButtonHoldStart;
          if (holdDuration < roguelikeMenuHoldTime && holdDuration > minButtonHoldForShoot) {
            if (currentTime - roguelikeLastShootTime >= roguelikeAutoShootDelay) {
              shootBullet();
              roguelikeLastShootTime = currentTime;
            }
          }
          roguelikeButtonHoldStart = 0;
        }
      }

      updateRoguelikeMode(currentTime, joystickHorizontalReading, joystickVerticalReading);
      break;

    // initial eeprom format
    case stateFormatting:
      if (currentTime - stateStart > formatHoldTime) {
        changeState(stateMenu, currentTime);
      }
      break;

    case stateGameOver:
      if (gameChanged) {
        gameOverStep = 0;
        gameOverStepTime = currentTime;
        gameChanged = false;

        lcd.clear();
        if (gameOverIsHighScore) {
          lcd.setCursor(0, 0);
          lcd.print(F("NEW HIGHSCORE!"));
        } else {
          lcd.setCursor(0, 0);
          lcd.print(F("GAME OVER!"));
        }
        lcd.setCursor(0, 1);
        lcd.print(F("Score: "));
        lcd.print(roguelikeScore);

        playTone(NOTE_G4, gameOverToneDuration);
      }

      // game over melody
      if (gameOverStep == 0 && currentTime - gameOverStepTime >= gameOverStepDuration) {
        playTone(NOTE_E4, gameOverToneDuration);
        gameOverStep = 1;
        gameOverStepTime = currentTime;
      } else if (gameOverStep == 1 && currentTime - gameOverStepTime >= gameOverStepDuration) {
        playTone(NOTE_C4, gameOverLongToneDuration);
        gameOverStep = 2;
        gameOverStepTime = currentTime;
      } else if (gameOverStep == 2 && currentTime - gameOverStepTime >= gameOverLongToneDuration) {
        if (gameOverIsHighScore) {
          playTone(NOTE_C6, gameOverToneDuration);
          gameOverStep = 3;
          gameOverStepTime = currentTime;
        } else {
          gameOverStep = gameOverNoHighScoreStep;
          gameOverStepTime = currentTime;
        }
      } else if (gameOverStep == 3 && currentTime - gameOverStepTime >= gameOverShortStepDuration) {
        playTone(NOTE_E5, gameOverToneDuration);
        gameOverStep = 4;
        gameOverStepTime = currentTime;
      } else if (gameOverStep == 4 && currentTime - gameOverStepTime >= gameOverShortStepDuration) {
        playTone(NOTE_G5, gameOverFinalToneDuration);
        gameOverStep = 5;
        gameOverStepTime = currentTime;
      } else if (gameOverStep == 5 && currentTime - gameOverStepTime >= gameOverDuration) {
        changeState(stateMenu, currentTime);
      } else if (gameOverStep == gameOverNoHighScoreStep && currentTime - gameOverStepTime >= gameOverDurationNoHighScore) {
        changeState(stateMenu, currentTime);
      }
      break;
  }
}

void changeState(GameState newState, unsigned long currentTime) {
  GameState previousState = gameState;
  gameState = newState;
  stateStart = currentTime;
  gameChanged = true;
  buttonPressStartTime = 0;

  if (previousState == stateIntro || previousState == stateDrawing || newState == stateMenu || newState == stateRoguelike) {
    matrixController.shutdown(matrixDeviceIndex, false);
    matrixController.setIntensity(matrixDeviceIndex, matrixDefaultIntensity);
    matrixController.clearDisplay(matrixDeviceIndex);
  }

  lcd.clear();

  if (newState == stateScores || newState == stateMenu || newState == stateAbout || newState == stateDrawing || newState == stateLightSensor || newState == stateRoguelike || newState == stateHowToPlay || newState == stateIntro || newState == stateSettings) {
    waitForRelease = true;
  } else {
    waitForRelease = false;
  }

  if (newState == stateDrawing) {
    enterDrawingMode(currentTime);
  }

  if (newState == stateLightSensor) {
    enterLightSensorMode(currentTime);
  }

  if (newState == stateRoguelike) {
    enterRoguelikeMode(currentTime);
  }
}

void drawMenu() {
  static int lastMenuSelection = -1;

  bool needsRedraw = gameChanged || (lastMenuSelection != menuSelection);

  if (!needsRedraw) return;

  lastMenuSelection = menuSelection;

  matrixController.clearDisplay(matrixDeviceIndex);

  if (menuSelection == menuOptionStart) {
    matrixController.setLed(matrixDeviceIndex, 5, 1, true);
    matrixController.setLed(matrixDeviceIndex, 5, 2, true);
    matrixController.setLed(matrixDeviceIndex, 4, 2, true);
    matrixController.setLed(matrixDeviceIndex, 5, 3, true);
    matrixController.setLed(matrixDeviceIndex, 4, 3, true);
    matrixController.setLed(matrixDeviceIndex, 3, 3, true);
    matrixController.setLed(matrixDeviceIndex, 5, 4, true);
    matrixController.setLed(matrixDeviceIndex, 4, 4, true);
    matrixController.setLed(matrixDeviceIndex, 3, 4, true);
    matrixController.setLed(matrixDeviceIndex, 2, 4, true);
    matrixController.setLed(matrixDeviceIndex, 5, 5, true);
    matrixController.setLed(matrixDeviceIndex, 4, 5, true);
    matrixController.setLed(matrixDeviceIndex, 3, 5, true);
    matrixController.setLed(matrixDeviceIndex, 5, 6, true);
    matrixController.setLed(matrixDeviceIndex, 4, 6, true);
    matrixController.setLed(matrixDeviceIndex, 5, 7, true);
  } else if (menuSelection == menuOptionHowToPlay) {
    matrixController.setLed(matrixDeviceIndex, 4, 1, true);
    matrixController.setLed(matrixDeviceIndex, 3, 1, true);
    matrixController.setLed(matrixDeviceIndex, 2, 1, true);
    matrixController.setLed(matrixDeviceIndex, 5, 2, true);
    matrixController.setLed(matrixDeviceIndex, 2, 2, true);
    matrixController.setLed(matrixDeviceIndex, 2, 3, true);
    matrixController.setLed(matrixDeviceIndex, 3, 4, true);
    matrixController.setLed(matrixDeviceIndex, 4, 5, true);
    matrixController.setLed(matrixDeviceIndex, 4, 7, true);
  } else if (menuSelection == menuOptionScores) {
    matrixController.setLed(matrixDeviceIndex, 5, 1, true);
    matrixController.setLed(matrixDeviceIndex, 4, 1, true);
    matrixController.setLed(matrixDeviceIndex, 3, 1, true);
    matrixController.setLed(matrixDeviceIndex, 2, 1, true);
    matrixController.setLed(matrixDeviceIndex, 1, 1, true);
    matrixController.setLed(matrixDeviceIndex, 5, 2, true);
    matrixController.setLed(matrixDeviceIndex, 1, 2, true);
    matrixController.setLed(matrixDeviceIndex, 4, 3, true);
    matrixController.setLed(matrixDeviceIndex, 2, 3, true);
    matrixController.setLed(matrixDeviceIndex, 3, 4, true);
    matrixController.setLed(matrixDeviceIndex, 3, 5, true);
    matrixController.setLed(matrixDeviceIndex, 4, 6, true);
    matrixController.setLed(matrixDeviceIndex, 3, 6, true);
    matrixController.setLed(matrixDeviceIndex, 2, 6, true);
  } else if (menuSelection == menuOptionAbout) {
    matrixController.setLed(matrixDeviceIndex, 3, 1, true);
    matrixController.setLed(matrixDeviceIndex, 4, 3, true);
    matrixController.setLed(matrixDeviceIndex, 3, 3, true);
    matrixController.setLed(matrixDeviceIndex, 3, 4, true);
    matrixController.setLed(matrixDeviceIndex, 3, 5, true);
    matrixController.setLed(matrixDeviceIndex, 3, 6, true);
    matrixController.setLed(matrixDeviceIndex, 4, 7, true);
    matrixController.setLed(matrixDeviceIndex, 3, 7, true);
    matrixController.setLed(matrixDeviceIndex, 2, 7, true);
  } else if (menuSelection == menuOptionSettings) {
    matrixController.setLed(matrixDeviceIndex, 3, 0, true);
    matrixController.setLed(matrixDeviceIndex, 4, 0, true);
    matrixController.setLed(matrixDeviceIndex, 1, 1, true);
    matrixController.setLed(matrixDeviceIndex, 6, 1, true);
    matrixController.setLed(matrixDeviceIndex, 2, 2, true);
    matrixController.setLed(matrixDeviceIndex, 3, 2, true);
    matrixController.setLed(matrixDeviceIndex, 4, 2, true);
    matrixController.setLed(matrixDeviceIndex, 5, 2, true);
    matrixController.setLed(matrixDeviceIndex, 0, 3, true);
    matrixController.setLed(matrixDeviceIndex, 2, 3, true);
    matrixController.setLed(matrixDeviceIndex, 5, 3, true);
    matrixController.setLed(matrixDeviceIndex, 7, 3, true);
    matrixController.setLed(matrixDeviceIndex, 0, 4, true);
    matrixController.setLed(matrixDeviceIndex, 2, 4, true);
    matrixController.setLed(matrixDeviceIndex, 5, 4, true);
    matrixController.setLed(matrixDeviceIndex, 7, 4, true);
    matrixController.setLed(matrixDeviceIndex, 2, 5, true);
    matrixController.setLed(matrixDeviceIndex, 3, 5, true);
    matrixController.setLed(matrixDeviceIndex, 4, 5, true);
    matrixController.setLed(matrixDeviceIndex, 5, 5, true);
    matrixController.setLed(matrixDeviceIndex, 1, 6, true);
    matrixController.setLed(matrixDeviceIndex, 6, 6, true);
    matrixController.setLed(matrixDeviceIndex, 3, 7, true);
    matrixController.setLed(matrixDeviceIndex, 4, 7, true);
  } else if (menuSelection == menuOptionLightSensor) {
    matrixController.setLed(matrixDeviceIndex, 3, 0, true);
    matrixController.setLed(matrixDeviceIndex, 4, 0, true);
    matrixController.setLed(matrixDeviceIndex, 1, 2, true);
    matrixController.setLed(matrixDeviceIndex, 6, 2, true);
    matrixController.setLed(matrixDeviceIndex, 2, 3, true);
    matrixController.setLed(matrixDeviceIndex, 3, 3, true);
    matrixController.setLed(matrixDeviceIndex, 4, 3, true);
    matrixController.setLed(matrixDeviceIndex, 5, 3, true);
    matrixController.setLed(matrixDeviceIndex, 2, 4, true);
    matrixController.setLed(matrixDeviceIndex, 3, 4, true);
    matrixController.setLed(matrixDeviceIndex, 4, 4, true);
    matrixController.setLed(matrixDeviceIndex, 5, 4, true);
    matrixController.setLed(matrixDeviceIndex, 1, 5, true);
    matrixController.setLed(matrixDeviceIndex, 6, 5, true);
    matrixController.setLed(matrixDeviceIndex, 3, 7, true);
    matrixController.setLed(matrixDeviceIndex, 4, 7, true);
  } else if (menuSelection == menuOptionDraw) {
    matrixController.setLed(matrixDeviceIndex, 1, 1, true);
    matrixController.setLed(matrixDeviceIndex, 0, 1, true);
    matrixController.setLed(matrixDeviceIndex, 2, 2, true);
    matrixController.setLed(matrixDeviceIndex, 1, 2, true);
    matrixController.setLed(matrixDeviceIndex, 3, 3, true);
    matrixController.setLed(matrixDeviceIndex, 2, 3, true);
    matrixController.setLed(matrixDeviceIndex, 4, 4, true);
    matrixController.setLed(matrixDeviceIndex, 3, 4, true);
    matrixController.setLed(matrixDeviceIndex, 5, 5, true);
    matrixController.setLed(matrixDeviceIndex, 4, 5, true);
    matrixController.setLed(matrixDeviceIndex, 6, 6, true);
    matrixController.setLed(matrixDeviceIndex, 5, 6, true);
    matrixController.setLed(matrixDeviceIndex, 7, 7, true);
  }

  lcd.setCursor(0, 0);
  lcd.print(F(" MATRIXOLOGY "));
  lcd.setCursor(0, 1);

  if (menuSelection == menuOptionStart) lcd.print(F("> START GAME    "));
  else if (menuSelection == menuOptionHowToPlay) lcd.print(F("> HOW TO PLAY   "));
  else if (menuSelection == menuOptionScores) lcd.print(F("> HIGH SCORES   "));
  else if (menuSelection == menuOptionSettings) lcd.print(F("> SETTINGS      "));
  else if (menuSelection == menuOptionAbout) lcd.print(F("> ABOUT         "));
  else if (menuSelection == menuOptionLightSensor) lcd.print(F("> LIGHT TEST    "));
  else if (menuSelection == menuOptionDraw) lcd.print(F("> DRAW MODE     "));

  gameChanged = false;
}

void initMatrix() {
  matrixController.shutdown(matrixDeviceIndex, false);
  matrixController.setIntensity(matrixDeviceIndex, matrixDefaultIntensity);
  matrixController.clearDisplay(matrixDeviceIndex);
  clearMatrixBuffer();
}

void clearMatrixBuffer() {
  for (byte row = 0; row < ledMatrixSize; row++) {
    for (byte col = 0; col < ledMatrixSize; col++) {
      matrixPixelState[row][col] = pixelStateOff;
    }
  }
  matrixController.clearDisplay(matrixDeviceIndex);
  matrixNeedsRefresh = true;
}

void renderMatrixFrame() {
  for (byte row = 0; row < ledMatrixSize; row++) {
    for (byte col = 0; col < ledMatrixSize; col++) {
      byte state = matrixPixelState[row][col];
      bool pixelOn = (state == pixelStateOn) || (state == pixelStateBlinkFast && fastBlinkVisible) || (state == pixelStateBlinkSlow && slowBlinkVisible) || (row == cursorRow && col == cursorColumn && cursorBlinkOn);
      matrixController.setLed(matrixDeviceIndex, row, col, pixelOn);
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
    lcd.setCursor(0, 0);
    lcd.print(F("Draw: tap paint"));
    lcd.setCursor(0, 1);
    lcd.print(F("1s clr 2.5s exit"));
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
    matrixPixelState[cursorRow][cursorColumn] = (matrixPixelState[cursorRow][cursorColumn] + 1) % pixelStateCount;
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

void showScores(unsigned long currentTime) {
  int scorePage = (currentTime / scorePageTime) % totalScorePages;
  static int lastDisplayedScorePage = -1;

  if (scorePage != lastDisplayedScorePage) {
    lcd.clear();
    lastDisplayedScorePage = scorePage;
    if (scorePage == 0) {
      lcd.setCursor(0, 0);
      lcd.print(F("1st place: "));
      printScore(eepromStartAddress);
      lcd.setCursor(0, 1);
      lcd.print(F("2nd place: "));
      printScore(eepromStartAddress + entrySize);
    } else {
      lcd.setCursor(0, 0);
      lcd.print(F("3rd place: "));
      printScore(eepromStartAddress + entrySize * 2);
      lcd.setCursor(0, 1);
      lcd.print(F("4th place: "));
      printScore(eepromStartAddress + entrySize * 3);
    }
  }
}

void showAbout(unsigned long currentTime) {
  int aboutPage = (currentTime / aboutPageTime) % totalAboutPages;
  static int lastDisplayedAboutPage = -1;

  if (aboutPage != lastDisplayedAboutPage) {
    lcd.clear();
    lastDisplayedAboutPage = aboutPage;

    if (aboutPage == 0) {
      lcd.setCursor(0, 0);
      lcd.print(F("Matrixology 2025"));
      lcd.setCursor(0, 1);
      lcd.print(F("By Sescu Matei"));
    } else {
      lcd.setCursor(0, 0);
      lcd.print(F("GitHub: Matei5/"));
      lcd.setCursor(0, 1);
      lcd.print(F("matrixology"));
    }
  }
}

void printScore(int eepromAddress) {
  int score;
  EEPROM.get(eepromAddress, score);
  lcd.print(score);
}

void playTone(int toneFrequency, int toneDuration) {
  tone(buzzerPin, toneFrequency);
  toneEnd = millis() + toneDuration;
}

void resetHighScores() {
  lcd.clear();
  lcd.print(F("Formatting..."));
  EEPROM.put(eepromResetCodeAddress, eepromResetCode);
  for (int scoreIndex = 0; scoreIndex < highScoreCount; scoreIndex++) {
    int scoreAddress = eepromStartAddress + (scoreIndex * entrySize);
    int score = 0;
    EEPROM.put(scoreAddress, score);
  }
}

void saveScore(int newScore) {
  int highScores[highScoreCount];

  for (int scoreIndex = 0; scoreIndex < highScoreCount; scoreIndex++) {
    int storedScore = 0;
    EEPROM.get(eepromStartAddress + (scoreIndex * entrySize), storedScore);
    if (storedScore < 0) storedScore = 0;
    highScores[scoreIndex] = storedScore;
  }

  if (newScore <= highScores[highScoreCount - 1]) return;

  highScores[highScoreCount - 1] = newScore;
  for (int outerIndex = 0; outerIndex < highScoreCount - 1; outerIndex++) {
    for (int innerIndex = 0; innerIndex < highScoreCount - 1 - outerIndex; innerIndex++) {
      if (highScores[innerIndex] < highScores[innerIndex + 1]) {
        int tempScore = highScores[innerIndex];
        highScores[innerIndex] = highScores[innerIndex + 1];
        highScores[innerIndex + 1] = tempScore;
      }
    }
  }

  for (int scoreIndex = 0; scoreIndex < highScoreCount; scoreIndex++) {
    int scoreAddress = eepromStartAddress + (scoreIndex * entrySize);
    EEPROM.put(scoreAddress, highScores[scoreIndex]);
  }
}

int readLightLevel() {
  return analogRead(photoResistorPin);
}

void enterLightSensorMode(unsigned long currentTime) {
  lastLightDisplayUpdate = currentTime;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Light Sensor"));
}

void updateLightSensorMode(unsigned long currentTime) {
  if (currentTime - lastLightDisplayUpdate >= lightSensorDisplayInterval) {
    lastLightDisplayUpdate = currentTime;
    currentLightLevel = readLightLevel();

    lcd.setCursor(0, 0);
    lcd.print(F("Light: "));
    lcd.print(currentLightLevel);
    lcd.print(F("    "));

    lcd.setCursor(0, 1);
    if (currentLightLevel < darkThreshold) {
      lcd.print(F("DARK - Ability! "));
    } else {
      lcd.print(F("Bright          "));
    }
  }
}

void checkAbilityTrigger(unsigned long currentTime) {
  currentLightLevel = readLightLevel();

  if (abilityReady && currentLightLevel < darkThreshold) {
    abilityReady = false;
    lastAbilityUse = currentTime;
    playTone(NOTE_C6, abilityTriggerToneDuration);
    shootBurst();
  }
}

void updateAbility(unsigned long currentTime) {
  if (!abilityReady && currentTime - lastAbilityUse >= abilityCooldown) {
    abilityReady = true;
    playTone(NOTE_G5, abilityReadyToneDuration);
  }
}

void initRoguelikeWorld() {
  for (int worldY = 0; worldY < roguelikeWorldSize; worldY++) {
    for (int worldX = 0; worldX < roguelikeWorldSize; worldX++) {
      roguelikeWorld[worldY][worldX] = roguelikeTileEmpty;
    }
  }
  
  // border walls
  for (int borderIndex = 0; borderIndex < roguelikeWorldSize; borderIndex++) {
    roguelikeWorld[0][borderIndex] = roguelikeTileWall;
    roguelikeWorld[roguelikeWorldSize - 1][borderIndex] = roguelikeTileWall;
    roguelikeWorld[borderIndex][0] = roguelikeTileWall;
    roguelikeWorld[borderIndex][roguelikeWorldSize - 1] = roguelikeTileWall;
  }

  // random pillars
  for (int pillarIndex = 0; pillarIndex < pillarCount; pillarIndex++) {
    int pillarX = random(wallPadding, roguelikeWorldSize - wallPadding);
    int pillarY = random(wallPadding, roguelikeWorldSize - wallPadding);
    if (roguelikeWorld[pillarY][pillarX] == roguelikeTileEmpty) {
      roguelikeWorld[pillarY][pillarX] = roguelikeTileWall;
    }
  }
}

void enterRoguelikeMode(unsigned long currentTime) {
  initRoguelikeWorld();

  // random spawn
  do {
    roguelikePlayerX = random(wallPadding, roguelikeWorldSize - wallPadding);
    roguelikePlayerY = random(wallPadding, roguelikeWorldSize - wallPadding);
  } while (roguelikeWorld[roguelikePlayerY][roguelikePlayerX] != roguelikeTileEmpty);

  roguelikeHealth = roguelikeInitialHealth;
  roguelikeAmmo = roguelikeInitialAmmo;
  roguelikeScore = 0;
  roguelikeLevel = 1;
  roguelikeLastMoveTime = currentTime;
  roguelikeLastDamageTime = 0;
  roguelikeLastEnemySpawn = currentTime;
  roguelikeLastShootTime = 0;
  lastShootDirX = 0;
  lastShootDirY = -1;
  gameChanged = true;

  // reset ability
  abilityReady = true;
  lastAbilityUse = 0;
  levelUpStep = 0;

  // clear enemies
  for (byte enemyIndex = 0; enemyIndex < maxEnemies; enemyIndex++) {
    enemies[enemyIndex].active = false;
  }

  // clear bullets
  for (byte bulletIndex = 0; bulletIndex < maxBullets; bulletIndex++) {
    bullets[bulletIndex].active = false;
  }

  // reset enemy limit
  currentMaxEnemies = initialMaxEnemies;

  // spawn enemies
  for (byte spawnIndex = 0; spawnIndex < initialEnemyCount; spawnIndex++) {
    spawnEnemy();
  }
}

void renderRoguelikeViewport() {
  // camera centered on player, clamped to world bounds
  int cameraX = constrain(roguelikePlayerX - roguelikeViewportSize / 2, 0, roguelikeWorldSize - roguelikeViewportSize);
  int cameraY = constrain(roguelikePlayerY - roguelikeViewportSize / 2, 0, roguelikeWorldSize - roguelikeViewportSize);

  // 8x8 viewport to matrix
  for (int viewY = 0; viewY < roguelikeViewportSize; viewY++) {
    for (int viewX = 0; viewX < roguelikeViewportSize; viewX++) {
      int worldX = cameraX + viewX;
      int worldY = cameraY + viewY;

      bool ledOn = false;

      // bullets
      for (byte bulletIndex = 0; bulletIndex < maxBullets; bulletIndex++) {
        if (bullets[bulletIndex].active && bullets[bulletIndex].positionX == worldX && bullets[bulletIndex].positionY == worldY) {
          ledOn = true;
          break;
        }
      }

      // player
      if (!ledOn && worldX == roguelikePlayerX && worldY == roguelikePlayerY) {
        bool enemyOnPlayer = false;
        for (byte enemyIndex = 0; enemyIndex < maxEnemies; enemyIndex++) {
          if (enemies[enemyIndex].active && enemies[enemyIndex].positionX == roguelikePlayerX && enemies[enemyIndex].positionY == roguelikePlayerY) {
            enemyOnPlayer = true;
            break;
          }
        }
        if (enemyOnPlayer) {
          ledOn = (millis() / playerBlinkInterval) % 2 == 0;
        } else {
          ledOn = true;
        }
      }
      // enemies - slow blink
      else if (!ledOn) {
        for (byte enemyIndex = 0; enemyIndex < maxEnemies; enemyIndex++) {
          if (enemies[enemyIndex].active && enemies[enemyIndex].positionX == worldX && enemies[enemyIndex].positionY == worldY) {
            ledOn = (millis() / slowBlinkInterval) % 2 == 0;
            break;
          }
        }
      }
      // tiles (walls)
      if (!ledOn && roguelikeWorld[worldY][worldX] != roguelikeTileEmpty) {
        ledOn = true;
      }

      matrixController.setLed(matrixDeviceIndex, viewY, viewX, ledOn);
    }
  }
}

void updateRoguelikeLCD(unsigned long currentTime) {
  // health with heartbeat animation
  lcd.setCursor(0, 0);
  byte heartFrame = (currentTime / heartbeatInterval) % heartbeatStages;
  if (heartFrame == 0) lcd.write((byte)charHeartSmall);
  else if (heartFrame == 1) lcd.write((byte)charHeart);
  else lcd.write((byte)charHeartBig);
  if (roguelikeHealth < paddingThreshold10) lcd.print(' ');
  lcd.print(roguelikeHealth);
  lcd.print(F(" Score:"));
  if (roguelikeScore < paddingThreshold100) lcd.print(' ');
  if (roguelikeScore < paddingThreshold10) lcd.print(' ');
  lcd.print(roguelikeScore);
  lcd.print(F("   "));

  // level + ability
  lcd.setCursor(0, 1);
  lcd.print(F("Lv"));
  lcd.print(roguelikeLevel);
  lcd.print(' ');

  if (abilityReady) {
    lcd.print(F("Power:Ready"));
  } else {
    unsigned long cooldownLeft = abilityCooldown - (millis() - lastAbilityUse);
    int secondsLeft = (cooldownLeft + (oneSecondMs - 1)) / oneSecondMs;
    lcd.print(F("Power:"));
    if (secondsLeft < paddingThreshold10) lcd.print(' ');
    lcd.print(secondsLeft);
    lcd.print(F("s   "));
  }
}

void updateRoguelikeMode(unsigned long currentTime, int horizontalReading, int verticalReading) {
  // movement
  if (currentTime - roguelikeLastMoveTime >= roguelikeMovementDelay) {
    int moveDx = 0, moveDy = 0;

    if (verticalReading < joystickThresholdLow) moveDx = -1;
    else if (verticalReading > joystickThresholdHigh) moveDx = 1;

    if (horizontalReading < joystickThresholdLow) moveDy = -1;
    else if (horizontalReading > joystickThresholdHigh) moveDy = 1;

    if (moveDx != 0 || moveDy != 0) {
      lastShootDirX = moveDx;
      lastShootDirY = moveDy;

      int newX = roguelikePlayerX + moveDx;
      int newY = roguelikePlayerY + moveDy;

      // try diagonal first
      if (newX >= 0 && newX < roguelikeWorldSize && newY >= 0 && newY < roguelikeWorldSize && roguelikeWorld[newY][newX] != roguelikeTileWall) {
        roguelikePlayerX = newX;
        roguelikePlayerY = newY;
        roguelikeLastMoveTime = currentTime;
        playTone(NOTE_C5, moveToneDuration);
      }
      // try horizontal only
      else if (moveDx != 0 && roguelikePlayerX + moveDx >= 0 && roguelikePlayerX + moveDx < roguelikeWorldSize && roguelikeWorld[roguelikePlayerY][roguelikePlayerX + moveDx] != roguelikeTileWall) {
        roguelikePlayerX += moveDx;
        roguelikeLastMoveTime = currentTime;
        playTone(NOTE_C5, moveToneDuration);
      }
      // try vertical only
      else if (moveDy != 0 && roguelikePlayerY + moveDy >= 0 && roguelikePlayerY + moveDy < roguelikeWorldSize && roguelikeWorld[roguelikePlayerY + moveDy][roguelikePlayerX] != roguelikeTileWall) {
        roguelikePlayerY += moveDy;
        roguelikeLastMoveTime = currentTime;
        playTone(NOTE_C5, moveToneDuration);
      }
    }
  }

  // update enemies
  updateEnemies(currentTime);

  // update bullets
  updateBullets(currentTime);

  // spawn enemies
  if (currentTime - roguelikeLastEnemySpawn > enemySpawnDelay) {
    spawnEnemy();
    roguelikeLastEnemySpawn = currentTime;
  }

  // level up  
  if (roguelikeScore >= roguelikeLevel * scorePerLevel && levelUpStep == 0) {
    roguelikeLevel++;
    levelUpStep = 1;
    levelUpStepTime = currentTime;
    playTone(NOTE_C6, levelUpToneDuration);
    roguelikeHealth = min(roguelikeHealth + healthGainOnLevelUp, roguelikeInitialHealth);
    if (currentMaxEnemies < maxEnemies) currentMaxEnemies++;
  }

  // level up melody
  if (levelUpStep == 1 && currentTime - levelUpStepTime >= levelUpStepDuration) {
    playTone(NOTE_E5, levelUpToneDuration);
    levelUpStep = 0;
  }

  // render
  renderRoguelikeViewport();
  updateRoguelikeLCD(currentTime);
}

void spawnEnemy() {
  // count enemies
  byte activeEnemyCount = 0;
  for (byte enemyIndex = 0; enemyIndex < maxEnemies; enemyIndex++) {
    if (enemies[enemyIndex].active) activeEnemyCount++;
  }
  if (activeEnemyCount >= currentMaxEnemies) {
    return; 
  }
  
  for (byte enemyIndex = 0; enemyIndex < maxEnemies; enemyIndex++) {
    if (!enemies[enemyIndex].active) {
      // spawn away from player
      byte spawnX, spawnY;
      int spawnAttempts = 0;
      do {
        spawnX = random(wallPadding, roguelikeWorldSize - wallPadding);
        spawnY = random(wallPadding, roguelikeWorldSize - wallPadding);
        spawnAttempts++;
      } while (spawnAttempts < maxSpawnAttempts && (roguelikeWorld[spawnY][spawnX] != roguelikeTileEmpty || (abs(spawnX - roguelikePlayerX) < minEnemySpawnDistance && abs(spawnY - roguelikePlayerY) < minEnemySpawnDistance)));

      if (spawnAttempts < maxSpawnAttempts) {
        enemies[enemyIndex].positionX = spawnX;
        enemies[enemyIndex].positionY = spawnY;
        enemies[enemyIndex].active = true;
        enemies[enemyIndex].lastMoveTime = millis();
        roguelikeWorld[spawnY][spawnX] = roguelikeTileEnemy;
        playTone(NOTE_D4, enemySpawnToneDuration);
      }
      return;
    }
  }
}

void updateEnemies(unsigned long currentTime) {
  for (byte enemyIndex = 0; enemyIndex < maxEnemies; enemyIndex++) {
    if (!enemies[enemyIndex].active) continue;

    if (currentTime - enemies[enemyIndex].lastMoveTime > enemyMoveDelay) {
      enemies[enemyIndex].lastMoveTime = currentTime;
      moveEnemyTowardsPlayer(enemyIndex);
    }
  }
}

void moveEnemyTowardsPlayer(byte enemyIndex) {
  Enemy* currentEnemy = &enemies[enemyIndex];
  int distanceX = roguelikePlayerX - currentEnemy->positionX;
  int distanceY = roguelikePlayerY - currentEnemy->positionY;

  roguelikeWorld[currentEnemy->positionY][currentEnemy->positionX] = roguelikeTileEmpty;

  int moveDirectionX = 0, moveDirectionY = 0;

  // move towards player - prefer larger distance axis
  if (abs(distanceX) >= abs(distanceY)) {
    if (distanceX > 0) moveDirectionX = 1;
    else if (distanceX < 0) moveDirectionX = -1;
  } else {
    if (distanceY > 0) moveDirectionY = 1;
    else if (distanceY < 0) moveDirectionY = -1;
  }

  // try to move
  int newEnemyX = currentEnemy->positionX + moveDirectionX;
  int newEnemyY = currentEnemy->positionY + moveDirectionY;
  if (newEnemyX >= 0 && newEnemyX < roguelikeWorldSize && newEnemyY >= 0 && newEnemyY < roguelikeWorldSize && roguelikeWorld[newEnemyY][newEnemyX] == roguelikeTileEmpty) {
    currentEnemy->positionX = newEnemyX;
    currentEnemy->positionY = newEnemyY;
  }

  // player collision
  if (currentEnemy->positionX == roguelikePlayerX && currentEnemy->positionY == roguelikePlayerY) {
    playerTakeDamage(millis());
    return;
  }

  // new position
  roguelikeWorld[currentEnemy->positionY][currentEnemy->positionX] = roguelikeTileEnemy;
}

void shootBullet() {
  if (roguelikeAmmo <= 0) {
    playTone(NOTE_C4, noAmmoToneDuration);
    return;
  }

  if (lastShootDirX == 0 && lastShootDirY == 0) {
    lastShootDirY = -1;
  }

  for (byte bulletIndex = 0; bulletIndex < maxBullets; bulletIndex++) {
    if (!bullets[bulletIndex].active) {
      bullets[bulletIndex].positionX = roguelikePlayerX;
      bullets[bulletIndex].positionY = roguelikePlayerY;
      bullets[bulletIndex].directionX = lastShootDirX;
      bullets[bulletIndex].directionY = lastShootDirY;
      bullets[bulletIndex].active = true;
      bullets[bulletIndex].lastMoveTime = millis();
      roguelikeAmmo--;
      playTone(NOTE_A5, shootToneDuration);

      // enemy on player?
      for (byte enemyIndex = 0; enemyIndex < maxEnemies; enemyIndex++) {
        if (enemies[enemyIndex].active && enemies[enemyIndex].positionX == bullets[bulletIndex].positionX && enemies[enemyIndex].positionY == bullets[bulletIndex].positionY) {
          enemies[enemyIndex].active = false;
          roguelikeWorld[enemies[enemyIndex].positionY][enemies[enemyIndex].positionX] = roguelikeTileEmpty;
          bullets[bulletIndex].active = false;
          roguelikeScore += scorePerKill;
          playTone(NOTE_G5, killToneDuration);
          break;
        }
      }
      return;
    }
  }
}

void shootBurst() {
  const int burstDirections[numDirections][2] = { { 0, -1 }, { 0, 1 }, { -1, 0 }, { 1, 0 }, { -1, -1 }, { -1, 1 }, { 1, -1 }, { 1, 1 } };
  int bulletsFiredCount = 0;

  // kill enemies on player tile
  for (byte enemyIndex = 0; enemyIndex < maxEnemies; enemyIndex++) {
    if (enemies[enemyIndex].active && enemies[enemyIndex].positionX == roguelikePlayerX && enemies[enemyIndex].positionY == roguelikePlayerY) {
      enemies[enemyIndex].active = false;
      roguelikeWorld[enemies[enemyIndex].positionY][enemies[enemyIndex].positionX] = roguelikeTileEmpty;
      roguelikeScore += scorePerKill;
      playTone(NOTE_G5, killToneDuration);
    }
  }

  for (byte directionIndex = 0; directionIndex < numDirections; directionIndex++) {
    for (byte bulletIndex = 0; bulletIndex < maxBullets; bulletIndex++) {
      if (!bullets[bulletIndex].active) {
        bullets[bulletIndex].positionX = roguelikePlayerX;
        bullets[bulletIndex].positionY = roguelikePlayerY;
        bullets[bulletIndex].directionX = burstDirections[directionIndex][0];
        bullets[bulletIndex].directionY = burstDirections[directionIndex][1];
        bullets[bulletIndex].active = true;
        bullets[bulletIndex].lastMoveTime = millis();
        bulletsFiredCount++;
        break;
      }
    }
  }

  if (bulletsFiredCount > 0) {
    playTone(NOTE_C6, burstToneDuration);
  }
}

void updateBullets(unsigned long currentTime) {
  for (byte bulletIndex = 0; bulletIndex < maxBullets; bulletIndex++) {
    if (!bullets[bulletIndex].active) continue;

    // check for enemy collision at current position -- sometimes enemies move on bullet
    for (byte enemyIndex = 0; enemyIndex < maxEnemies; enemyIndex++) {
      if (enemies[enemyIndex].active && enemies[enemyIndex].positionX == bullets[bulletIndex].positionX && enemies[enemyIndex].positionY == bullets[bulletIndex].positionY) {
        enemies[enemyIndex].active = false;
        roguelikeWorld[enemies[enemyIndex].positionY][enemies[enemyIndex].positionX] = roguelikeTileEmpty;
        bullets[bulletIndex].active = false;
        roguelikeScore += scorePerKill;
        playTone(NOTE_G5, killToneDuration);
        break;
      }
    }
    if (!bullets[bulletIndex].active) continue;

    if (currentTime - bullets[bulletIndex].lastMoveTime > bulletMoveDelay) {
      bullets[bulletIndex].lastMoveTime = currentTime;

      // move bullet
      bullets[bulletIndex].positionX += bullets[bulletIndex].directionX;
      bullets[bulletIndex].positionY += bullets[bulletIndex].directionY;

      // out of bounds
      if (bullets[bulletIndex].positionX < 0 || bullets[bulletIndex].positionX >= roguelikeWorldSize || bullets[bulletIndex].positionY < 0 || bullets[bulletIndex].positionY >= roguelikeWorldSize) {
        bullets[bulletIndex].active = false;
        continue;
      }

      // wall collision
      if (roguelikeWorld[bullets[bulletIndex].positionY][bullets[bulletIndex].positionX] == roguelikeTileWall) {
        bullets[bulletIndex].active = false;
        continue;
      }

      // enemy collision at new position
      for (byte enemyIndex = 0; enemyIndex < maxEnemies; enemyIndex++) {
        if (enemies[enemyIndex].active && enemies[enemyIndex].positionX == bullets[bulletIndex].positionX && enemies[enemyIndex].positionY == bullets[bulletIndex].positionY) {
          enemies[enemyIndex].active = false;
          roguelikeWorld[enemies[enemyIndex].positionY][enemies[enemyIndex].positionX] = roguelikeTileEmpty;
          bullets[bulletIndex].active = false;
          roguelikeScore += scorePerKill;
          playTone(NOTE_G5, killToneDuration);
          break;
        }
      }
    }
  }
}

void playerTakeDamage(unsigned long currentTime) {
  if (currentTime - roguelikeLastDamageTime < damageInvincibilityTime) return;

  roguelikeLastDamageTime = currentTime;
  roguelikeHealth--;
  playTone(NOTE_C4, damageToneDuration);

  if (roguelikeHealth <= 0) {
    roguelikeGameOver(currentTime);
  }
}

void roguelikeGameOver(unsigned long currentTime) {
  // check if highscore
  int lowestScore = 0;
  EEPROM.get(eepromStartAddress + ((highScoreCount - 1) * entrySize), lowestScore);
  gameOverIsHighScore = (roguelikeScore > lowestScore);

  if (gameOverIsHighScore) {
    saveScore(roguelikeScore);
  }

  gameOverStep = 0;
  changeState(stateGameOver, currentTime);
}