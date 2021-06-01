#include <Keypad.h>
#include <LiquidCrystal.h>    //LCD library


const byte ROWS = 4; 
const byte COLS = 4; 
const int rs = 53, en = 51, d4 =49, d5 = 47, d6 = 45, d7 = 43;
char buffer[123] = {};
int currentBuffer = 0;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {11, 10, 9, 8}; 
byte colPins[COLS] = {7, 6, 5, 4}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void charToString(char S[], String &D)
{
 String rc(S);
 D = rc;
 Serial.println(D);
}

void enter_student_id() {
  char customKey = customKeypad.getKey();
  if (customKey){
    if (customKey == '#') {
      Serial.println("Delete");
      if (currentBuffer != 0) {
        lcd.setCursor(currentBuffer - 1, 0);
        lcd.print(" ");
        currentBuffer--;
      }
      charToString(buffer, 'A');
    } else {
       buffer[currentBuffer] = customKey;
      // lcd.clear();
      lcd.setCursor(currentBuffer, 0); 
      lcd.print(customKey);
      ++currentBuffer;
    }   
  }
}

void setup(){
  lcd.begin(16, 2); 
  Serial.begin(9600);
}
  
void loop(){
  enter_student_id();
}
