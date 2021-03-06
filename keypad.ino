// import libraries
#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Adafruit_Fingerprint.h>
#include <EEPROM.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

// variable for student id
uint8_t id;

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(19, 18); // setup pins for RX, TX comm with fingerprint sensor
#else
#define mySerial Serial1
#endif

// set array for keypad keys
char hexaKeys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

// set pins for keypad

byte row_pins[4] = {29, 27, 25, 23};
byte col_pins[4] = {28, 26, 24, 22};

const int buzzer = 8;
int eeprom_address = 0;

// setup fingerprint object
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
// setup keypad object
Keypad custom_keypad = Keypad(makeKeymap(hexaKeys), row_pins, col_pins, 4, 4);

void beep()
{
  tone(buzzer, 1000); // Send 1KHz sound signal...
  delay(100);
  noTone(buzzer);
  delay(100);
  tone(buzzer, 1000);
  delay(100);
  noTone(buzzer);
}

uint8_t getStudentID()
{
  // creating a buffer to store keypad input
  char buffer[123] = {};
  int current_buffer = 0;  // variable to store current buffer index
  uint16_t student_id = 0; // using a 16-bit integer

  // print welcome message on lcd
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter student id");

  while (student_id < 127) // using 8 bit signed integer addresses
  {
    while (!Serial.available())
    {
      // get input from keypad
      char custom_key = custom_keypad.getKey();
      if (custom_key)
      {
        tone(buzzer, 750, 100);
        // delete key
        if (custom_key == '#')
        {
          if (current_buffer != 0)
          {
            --current_buffer;
            lcd.setCursor(current_buffer, 1);
            lcd.print(" ");
            byte lastChar = strlen(buffer) - 1;
            buffer[lastChar] = '\0';
          }
        }
        else if (custom_key == 'A' || custom_key == 'B' || custom_key == 'C' || custom_key == 'D')
        {
          // do nothing if these keys are pressed
          continue;
        }
        // Enter key
        else if (custom_key == '*')
        {
          Serial.println(student_id);
          if (student_id > 0 && student_id < 128)
          {
            if (EEPROM.read(atoi(String(buffer).c_str())) == atoi(String(buffer).c_str()))
            {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("ID already taken");
              delay(2000);
              return false;
            }
            return atoi(String(buffer).c_str());
          }
          else
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Invalid ID");
            lcd.setCursor(0, 1);
            lcd.print("From 1-127");
            beep();
            delay(1000);
            lcd.clear();
            return false;
          }
        }
        // if student enters any other key
        else
        {
          // add 8bit conversion of char to buffer
          buffer[current_buffer] = uint8_t(custom_key);
          lcd.setCursor(current_buffer, 1);
          lcd.print(custom_key);
          // add to current buffer index
          ++current_buffer;
          student_id = atoi(buffer); // convert char array to 8 bit int
          Serial.println(String(buffer));
        }
      }
    }
  }
}

uint8_t getFingerprintID()
{
  uint8_t p = finger.getImage();
  while (p != FINGERPRINT_OK)
  {
    char custom_key = custom_keypad.getKey();

    if (!custom_key)
    {
      p = finger.getImage();
      switch (p)
      {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Image taken");
        delay(1000);
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println("No finger detected");
        return p;
      case FINGERPRINT_IMAGEFAIL:
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("System error:");
        lcd.setCursor(0, 1);
        lcd.print("0x1000"); // communication error
        delay(1000);
        return p;
      default:
        Serial.println("Unknown error");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("rror code 0x1");
        delay(1000);
        return p;
      }
    }
    else
    {
      if (custom_key == 'A')
      {
        mainMenu();
      }
    }
  }
  // OK success!

  p = finger.image2Tz();
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Found a print match!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_NOTFOUND)
  {
    Serial.println("Did not find a match");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No match");
    beep();
    delay(1000);
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Attendance mark");
  lcd.setCursor(0, 1);
  lcd.print("ID No: ");
  lcd.setCursor(7, 1);
  lcd.print(finger.fingerID);
  beep();
  delay(1000);
  lcd.clear();

  return finger.fingerID;
}

uint8_t getFingerprintEnroll()
{

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  while (p != FINGERPRINT_OK)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Place finger");
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Image taken");
      beep();
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("System Error:");
      lcd.setCursor(0, 1);
      lcd.print("0x1000"); // comm error
      delay(1000);
      break;
    case FINGERPRINT_IMAGEFAIL:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("System Error:");
      lcd.setCursor(0, 1);
      lcd.print("0x2000"); // imaging error
      delay(1000);
      Serial.println("Imaging error");
      break;
    default:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("System Error:");
      lcd.setCursor(0, 1);
      lcd.print("0x0000"); // unknown error
      delay(1000);
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  Serial.println("Remove finger");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Remove finger");
  beep();
  delay(1000);

  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place finger");
  lcd.setCursor(0, 1);
  lcd.print("again");
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Image taken");
      tone(buzzer, 1000); // Send 1KHz sound signal...
      delay(100);
      tone(buzzer, 1000);
      delay(100);
      noTone(buzzer);
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Prints matched!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Match");
    delay(1000);
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH)
  {
    Serial.println("Fingerprints did not match");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No match");
    beep();
    delay(1000);
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Stored!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Stored");
    writeToMem(id);
    Serial.println(EEPROM.read(eeprom_address - 1));
    delay(2000);
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Error:");
    lcd.setCursor(0, 1);
    lcd.print("BADMEM"); // comm error
    delay(1000);
    Serial.println("Could not store in that location");
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Error:");
    lcd.setCursor(0, 1);
    lcd.print("ERRFLASH"); // comm error
    delay(1000);
    Serial.println("Error writing to flash");
    return p;
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Error:");
    lcd.setCursor(0, 1);
    lcd.print("0x0000"); // comm error
    delay(1000);
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

void writeToMem(uint8_t data)
{
  // function to write an 8 bit integer to EEPROM memory
  EEPROM.write(data, data);    // write set data and data address the same
  if (data >= EEPROM.length()) // if data address is greater than EEPROM capacity
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mem full!");
    beep();
    delay(2000);
  }
  delay(100);
}

void setup()
{
  lcd.begin(16, 2);
  Serial.begin(9600);

  // setup data rate for sensor
  finger.begin(57600);

  while (true)
  {
    if (finger.verifyPassword())
    {
      Serial.println("Found fingerprint sensor!");
      break;
    }
    else
    {
      Serial.println("Did not find fingerprint sensor :(");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("System error:");
      lcd.setCursor(0, 1);
      lcd.print("No fingerprint");
      while (1)
      {
        beep();
        // delay(100);
      }
    }
  }
}

void mainMenu()
{
  int choice = 0;
  lcd.clear();
  while (choice == 0)
  {
    // lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Select an option");
    lcd.setCursor(0, 1);
    lcd.print("1 2 3");

    char custom_key = custom_keypad.getKey();

    if (custom_key)
    {
      if (custom_key == '1')
      {
        int p = -1;
        while (p != FINGERPRINT_OK)
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Place finger");
          p = finger.getImage();

          getFingerprintID();
        }
      }
      else if (custom_key == '2')
      {
        id = getStudentID();
        if (id == 0 || id == false)
        { // ID #0 not allowed, try again!
          continue;
        }
        Serial.print("Enrolling ID #");
        Serial.println(id);

        while (!getFingerprintEnroll()) // keep loop running
          ;
      }
    }
  }
}

void loop()
{
  mainMenu();
}
