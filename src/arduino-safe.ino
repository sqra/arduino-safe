#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Timer.h>
#include "songs.h"

Timer t;
int attempts = 0;
char validPassword[4] = "0000"; // <- set 4 digit pin here
char userPassword[4] = "";
int position = 0;
const byte ROWS = 4;
const byte COLS = 4;
boolean isNormalMode = true;
boolean isOpen;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
int redpin = A0; // red LED
int greenpin = A1; // green LED
int lockpin = A2; // lock
int backlightpin = A3; // lcd backlight
int speakerpin = 0; // buzzer
int switchpin = 5; // door switch
int lightpin = 4; // interior light
boolean alarmStatus;
int alarmEvent;
boolean firstRun = true;
byte rowPins[ROWS] = {13, 12, 11, 10};
byte colPins[COLS] = {9, 8, 7, 6};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

void displayText(char* text, int row, boolean clearScreen) {
  if (clearScreen) {
    lcd.clear();
  }
  int textPosition = (16 - strlen(text)) / 2;
  lcd.setCursor(textPosition, row);
  lcd.print(text);
}

void unlock()
{
  attempts = 0;
  successSound();
  green();
  red();
  displayText("ACCESS GRANTED", 0, true);
  digitalWrite(lockpin, HIGH);
  delay(2000);
  lock();
}

void lock()
{
  green();
  red();
  digitalWrite(lockpin, LOW);
}
void green()
{
  digitalWrite(greenpin, !digitalRead(greenpin));
}

void red()
{
  digitalWrite(redpin, !digitalRead(redpin));
}

void clickSound() {
  tone(speakerpin, 5000);
  delay(50);
  noTone(speakerpin);
}

void errorSound() {
  tone(speakerpin, 3500);
  delay(50);
  noTone(speakerpin);
  delay(50);
  tone(speakerpin, 3500);
  delay(50);
  noTone(speakerpin);
}

void successSound() {
  tone(speakerpin, 3500);
  delay(100);
  tone(speakerpin, 4000);
  delay(400);
  noTone(speakerpin);
}

void setup() {
  Serial.begin(9600);
  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(lockpin, OUTPUT);
  pinMode(backlightpin, OUTPUT);
  pinMode(speakerpin, OUTPUT);
  pinMode(lightpin, OUTPUT);
  pinMode(switchpin, INPUT);
  red();
  lcd.backlight();
  lcd.init();
  reset();
  lightenScreen();
}

boolean array_cmp(char *a, char *b, char len_a, char len_b) {
  int n;
  if (len_a != len_b) return false;
  for (n = 0; n < len_a; n++) if (a[n] != b[n]) return false;
  return true;
}

void reset()
{
  position = 0;
  memset(userPassword, 0, sizeof(userPassword));
  displayText("ENTER CODE", 0, true);
  displayText("____", 1, false);
  startIdle();
}

void startIdle() {
  int backlightEvent = t.after(20000, darkenScreen);
}

void lightenScreen() {
  isNormalMode = true;
  for (int i = 0; i < 9; i++) {
    int v = (1 << i) - 1;
    analogWrite(backlightpin, v);
    delay(50);
  }
}

void darkenScreen() {
  isNormalMode = false;
  for (int i = 7; i > 0; i--) {
    int v = (1 << i) - 1;
    analogWrite(backlightpin, v - 1);
    delay(50);
  }
}

void checkSwitch() {
  isOpen = digitalRead(switchpin);
  digitalWrite(lightpin, isOpen);
}

void checkAlarm() {
  if (isOpen) {
    if (firstRun) {
      alarmStatus = false;
      firstRun = false;
    }
    if (!alarmStatus) {
      alarmEvent = t.after(30000, alarmSound);
      alarmStatus = true;
    }
  } else {
    if (alarmStatus) {
      t.stop(alarmEvent);
      lightenScreen();
      displayText("SECURED", 0, true);
      displayText("THANK YOU", 1, false);
      alarmStatus = false;
      delay(2000);
      reset();
    }
  }
}

void alarmSound() {
  lightenScreen();
  displayText("WARNING", 0, true);
  displayText("CLOSE THE SAFE", 1, false);
  tone(speakerpin, 3500);
  delay(100);
  noTone(speakerpin);
  delay(50);
  tone(speakerpin, 3500);
  delay(100);
  noTone(speakerpin);
  alarmStatus = false;
}

void checkAttempts(){
  if(attempts >= 3){
    int x = 30;
    String text1 = "WAIT...";
    while (x >= 0) {
      String timeText = text1+x;
      displayText("BLOCKED", 0, true);
      displayText(timeText.c_str (), 1, false);
      delay(1000);
      x--;
    }
    attempts = 0;
    displayText("KEYPAD", 0, true);
    displayText("UNLOCKED", 1, false);
    delay(1000);
  }
  return;
}

void loop() {
  t.update();
  char key = customKeypad.getKey();
  if (key && !isOpen) {
    if (isNormalMode) {
      clickSound();
      if (String(key) == "A") {
        displayText("SUPER MARIO", 0, true);
        displayText("THEME", 1, false);
        playSong(1);
        playSong(1);
        reset();
      } else if (String(key) == "B") {
        lcd.leftToRight();
        displayText("KEYBOARD", 0, true);
        displayText("CAT", 1, false);
        playSong(2);
        reset();
      } else if (String(key) == "C") {
        displayText("PINK PANTHER", 0, true);
        displayText("THEME", 1, false);
        playSong(3);
        reset();
      } else if (String(key) == "D") {
        displayText("13.12.2020", 0, true);
        displayText("by Pawel Skorka", 1, false);
        delay(4000);
        reset();
      } else {
        int afterEvent;
        t.stop(afterEvent);
        afterEvent = t.after(10000, reset);
        userPassword[position] = key;
        lcd.setCursor(position + 6, 1);
        lcd.print('*');
        if (position == 3)
        {
          t.stop(afterEvent);
          delay(1000);
          if (array_cmp(userPassword, validPassword, 4, 4) == true) {
            unlock();
          } else {
            attempts++;
            displayText("ACCESS DENIED", 0, true);
            errorSound();
            checkAttempts();
            delay(2000);
          }
          reset();
          return;
        }
        position ++;
      }

    } else {
      lightenScreen();
      startIdle();
    }
  }
  checkSwitch();
  checkAlarm();
  delay(100);
}
