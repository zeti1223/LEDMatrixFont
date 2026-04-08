#include <Arduino.h>
#include "LEDMatrixFont.h"

// ===== Pin kiosztás =====
#define PIN_DI   7
#define PIN_CLK  8
#define PIN_LAT  9
#define PIN_A    5
#define PIN_B    4
#define PIN_C    3
#define PIN_D    2
#define PIN_G    6

String message = "HELLO WORLD!";
int scrollPos = 0;
unsigned long lastScroll = 0;
const int scrollDelay = 100; // ms

void setup() {
  Serial.begin(9600);

  pinMode(PIN_DI, OUTPUT);
  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_LAT, OUTPUT);
  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_C, OUTPUT);
  pinMode(PIN_D, OUTPUT);
  pinMode(PIN_G, OUTPUT);

  Serial.println("Type a message and press Enter...");
}

void loop() {
  if (Serial.available()) {
    message = Serial.readStringUntil('\n');
    Serial.println(message);
    scrollPos = 0;
  }

  // --- Scroll időzítés ---
  if (millis() - lastScroll > scrollDelay) {
    scrollPos++;
    if (scrollPos > message.length() * 6+16) {
      scrollPos = 0;
    }
    lastScroll = millis();
  }

  // --- Kijelző frissítés ---
  drawMessage(message, scrollPos);
}

// ===== Szöveg kirajzolása =====
void drawMessage(String text, int offset) {
  uint16_t frame[16] = {0}; // minden sor egy 16 bites sor

  // --- betűk kirajzolása bufferbe ---
  for (int i = 0; i < text.length(); i++) {
    char c = text[i];
    if (c < 32 || c > 127) c = 32; // ha nincs benne → space
    int index = c - 32;

    for (int row = 0; row < 7; row++) {
      byte rowBits = pgm_read_byte(&(font5x7[index][row]));
      for (int col = 0; col < 5; col++) {
        int x = i * 6 + col - offset + 16; // +16 hogy jobbról induljon
        int y = row + 4; // középre igazítva
        if (x >= 0 && x < 16 && y < 16) {
          if (rowBits & (1 << (4 - col))) {
            frame[y] |= (1 << (15 - x));
          }
        }
      }
    }
  }

  // --- invert + függőleges tükrözés ---
  uint16_t frameOut[16];
  for (int row = 0; row < 16; row++) {
    frameOut[15 - row] = ~frame[row]; // invert + flip Y
  }

  // --- mátrix frissítés ---
  for (int row = 0; row < 16; row++) {
    shiftOutRow(frameOut[row]);

    digitalWrite(PIN_A, row & 0x01);
    digitalWrite(PIN_B, (row >> 1) & 0x01);
    digitalWrite(PIN_C, (row >> 2) & 0x01);
    digitalWrite(PIN_D, (row >> 3) & 0x01);
    digitalWrite(PIN_G, (row >> 4) & 0x01);

    digitalWrite(PIN_LAT, HIGH);
    digitalWrite(PIN_LAT, LOW);

    delayMicroseconds(200); // multiplex sebesség
  }
}

// ===== Sor kiküldése shiftregisterre =====
void shiftOutRow(uint16_t data) {
  for (int i = 0; i < 16; i++) {
    digitalWrite(PIN_DI, (data >> (15 - i)) & 0x01);
    digitalWrite(PIN_CLK, HIGH);
    digitalWrite(PIN_CLK, LOW);
  }
}
