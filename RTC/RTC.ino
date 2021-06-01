#include <Wire.h>

#include <RTClib.h>

RTC_DS3231 rtc;
DateTime now;

void setup() {
  Wire.begin();
  if (! rtc.begin())
    Serial.println("Couldn't find RTC");
   
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   
  // rtc.adjust(DateTime(2021, 6, 1, 1, 51, 0));
  Serial.begin(9600);
}
 
void loop() {
  now = rtc.now();
  Serial.print(now.hour());
  Serial.print(':');
  Serial.print(now.minute());
  Serial.print(':');
  Serial.print(now.second());
  Serial.println();
 
  delay(1000);
}
