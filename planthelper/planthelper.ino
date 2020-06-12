#include <Wire.h>
#include <DS1307.h>
#include <LiquidCrystal_I2C.h>

int HUM1_VAL_LOW = 240;
int HUM1_VAL_HIGH = 600;
int HUM2_VAL_LOW = 240;
int HUM2_VAL_HIGH = 600;
int PIN_HUM1 = A0; 
int PIN_HUM2 = A1; 
int PIN_FAN = 7;
int FAN_RUN_HOUR = 7;

int LOOP_CNT = 0;
int HUM_UPDATE_FREQ = 5;

DS1307 rtc;
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void fan_on() {
  digitalWrite(PIN_FAN, LOW);
}

void fan_off() {
  digitalWrite(PIN_FAN, HIGH);
}

void do_humidity() {

  int hum1 = analogRead(PIN_HUM1); 
  int hum2 = analogRead(PIN_HUM2);

//  Serial.print("HUM1: ");
//  Serial.println(hum1);
//  Serial.print("HUM2: ");
//  Serial.println(hum2);
  
  lcd.setCursor(0,3); 
  lcd.print("Hum: 1: ");  
  
  int val1 = 100L - (((hum1 - HUM1_VAL_LOW) * 100L) / HUM1_VAL_HIGH);
  
  if (val1 > 100) {
    val1 = 100;
  }
  
  lcd.print(val1);
  
  //lcd.print(100L - ((HUM1_VAL*100L)/1024));

  if (val1 < 100)
    lcd.print("%  2: ");
  else
    lcd.print("% 2: ");
  
  
  int val2 = 100L - (((hum2 - HUM2_VAL_LOW) * 100L) / HUM2_VAL_HIGH);
  
  if (val2 > 100) {
    val2 = 100;
  }
  
  lcd.print(val2);

  if (val2 < 100)
    lcd.print("% ");
  else
    lcd.print("%");
}

void do_fan(uint8_t hour) {

  String fan_str = "OFF";
  // Run from x o'clock for one hour
  if (hour == FAN_RUN_HOUR) {
    fan_on();
    fan_str = "ON ";
  }
  else {
    fan_off();
  }
  
  lcd.setCursor(0,2); 
  lcd.print("      Fan: ");
  lcd.print(fan_str);
}

int do_time() {
  
  uint8_t sec, min, hour, day, month;
  uint16_t year;
 
  /*get time from RTC*/
  rtc.get(&sec, &min, &hour, &day, &month, &year);
  
  lcd.setCursor(0,0); 
  
  //lcd.print(100L - (((HUM2_VAL - 250)*100L)/600));
  //lcd.print("     ");

  //lcd.setCursor(0,2); 
  //lcd.print(HUM2_VAL);
  
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

  return hour;
}

void setup() {

  /*init Serial port*/
  Serial.begin(9600);
  while(!Serial); /*wait for serial port to connect - needed for Leonardo only*/

  /*init fan*/
  pinMode(PIN_FAN, OUTPUT);

  /*init RTC*/
  Serial.println("Init RTC...");

  /*only set the date+time one time*/
  //rtc.set(0, 55, 14, 10, 6, 2020); /*08:00:00 24.12.2014 //sec, min, hour, day, month, year*/

  /*stop/pause RTC*/
  // rtc.stop();

  /*start RTC*/
  rtc.start();

  lcd.begin(20,4);
  lcd.backlight();//Power on the back light
 
}

void loop() {
  
  uint8_t hour = do_time();

  do_fan(hour);

  if (!(LOOP_CNT % HUM_UPDATE_FREQ)) {
    do_humidity();
  }
  
  delay(1000);

  LOOP_CNT++;

}
