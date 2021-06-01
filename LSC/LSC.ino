#include <LiquidCrystal.h>    //LCD library
const int rs = 53, en = 51, d4 =49, d5 = 47, d6 = 45, d7 = 43;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
void setup() {
  lcd.begin(16, 2);           // number of col. and rows
  lcd.print("Group 10");// Print text on LCD
}
void loop() {}
