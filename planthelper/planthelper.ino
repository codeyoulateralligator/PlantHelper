#include <Wire.h>
#include <DS1307.h>
#include <LiquidCrystal_I2C.h>

int sensorPin = A0; 
int sensorValue = 0L;  
int limit = 300; 
int startup_clockeriino = 7;

DS1307 rtc;

int fan = 7;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); //

void fan_on() {
  digitalWrite(fan, LOW);
}

void fan_off() {
  digitalWrite(fan, HIGH);
}

void setup() {

  /*init Serial port*/
  Serial.begin(9600);
  while(!Serial); /*wait for serial port to connect - needed for Leonardo only*/

  /*init fan*/
  pinMode(fan, OUTPUT);

  /*init RTC*/
  Serial.println("Init RTC...");

  /*only set the date+time one time*/
  //rtc.set(0, 21, 14, 15, 5, 2020); /*08:00:00 24.12.2014 //sec, min, hour, day, month, year*/

  /*stop/pause RTC*/
  // rtc.stop();

  /*start RTC*/
  rtc.start();

  lcd.begin(20,4);
  lcd.backlight();//Power on the back light
 
}

void loop() {
  uint8_t sec, min, hour, day, month;
  uint16_t year;
 
  /*get time from RTC*/
  rtc.get(&sec, &min, &hour, &day, &month, &year);

  String fan_str = "OFF";
  // Run from x o'clock for one hour
  if (hour == startup_clockeriino) {
    fan_on();
    fan_str = "ON ";
  }
  else {
    fan_off();
  }

  sensorValue = analogRead(sensorPin); 

  lcd.setCursor(0,0); 
  if (day < 10)
    lcd.print("0");
  lcd.print(day);
   
  lcd.print(".");
  if (month < 10)
    lcd.print("0"); 
  lcd.print(month);
  lcd.print(".");
  lcd.print(year);
  lcd.print("  ");

  if (hour < 10)
    lcd.print("0"); 
  lcd.print(hour); 
  lcd.print(":"); 
  if (min < 10)
    lcd.print("0"); 
  lcd.print(min);
  lcd.print(":");
  if (sec < 10)
    lcd.print("0"); 
  lcd.print(sec);

  lcd.setCursor(0,2); 
  lcd.print("      Fan: ");
  lcd.print(fan_str);

  lcd.setCursor(0,3); 
  lcd.print(" Humidity: ");  
  lcd.print(100L - ((sensorValue*100L)/1024));
  lcd.print("%    ");

  delay(1000);

}
