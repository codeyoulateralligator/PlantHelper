#include <Wire.h>
#include <DS1307.h>
#include <LiquidCrystal_I2C.h>

// General
#define NOF_POTS 2
#define MAIN_DELAY 1000

// Pump
#define PIN_PUMP 6
#define PUMP_RUN_TIME 3
#define PUMP_DELAY 600 // 600
#define PUMP_START_LEVEL 95
#define PUMP_MIN_HOUR 8
#define PUMP_MAX_HOUR 20

// Humidity
#define HUM_VAL_LOW 240
#define HUM_VAL_HIGH 600
#define PIN_HUM1 A0 
#define PIN_HUM2 A1 
#define HUM_UPDATE_FREQ 5
#define HUM_REG_FREQ 3600 // 3600

// Fan
#define PIN_FAN 7
#define FAN_RUN_HOUR1 6
#define FAN_RUN_HOUR2 7

// 
int loop_cnt = 0;
DS1307 rtc;

LiquidCrystal_I2C lcd0(0x26, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
LiquidCrystal_I2C lcd1(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

typedef struct 
{
   LiquidCrystal_I2C lcd;
   int id; 
   String name;  
   int hum_pin;
   
   int hum;
   int pump_timer;
   bool pump_activated;
} Pot;


Pot Pots[NOF_POTS] = {
  { lcd0, 0, "LAT KOLLANE", PIN_HUM2, 0, PUMP_DELAY, 0},
  { lcd1, 1, "JOG PUNANE", PIN_HUM1, 0, PUMP_DELAY, 0},
};

void fan_on() {
  digitalWrite(PIN_FAN, LOW);
}

void fan_off() {
  digitalWrite(PIN_FAN, HIGH);
}

void pump_on() {
  digitalWrite(PIN_PUMP, LOW);
}

void pump_off() {
  digitalWrite(PIN_PUMP, HIGH);
}

void do_humidity_reg(Pot* pot) {
  if (!(loop_cnt % HUM_REG_FREQ)) {   
    if (pot->hum < PUMP_START_LEVEL) {

      // TODO
      if (pot->id == 0)
      pot->pump_activated = 1;
    }    
  }
  if (pot->pump_activated) { 
      pot->pump_timer--;

      if (pot->pump_timer > 0) {
        pot->lcd.setCursor(0,3); 
        pot->lcd.print("Pump start: "); 
        pot->lcd.print(pot->pump_timer);
        pot->lcd.print(" sec ");
      }
      else if (pot->pump_timer == 0) {
        pot->lcd.setCursor(0,3); 
        pot->lcd.print("Pump start: NOW!!!  "); 
        pump_on();
      }
      else if (pot->pump_timer == (0 - PUMP_RUN_TIME)) {
        pot->pump_timer = PUMP_DELAY;
        pot->pump_activated = 0;
        pot->lcd.setCursor(0,3); 
        pot->lcd.print("Pump start: UNSET   ");
        pump_off(); 
      }
  }
  else {
    pot->lcd.setCursor(0,3); 
    pot->lcd.print("Pump start: UNSET   ");
  }
 
}

void do_humidity(Pot* pot) {

  int hum = analogRead(pot->hum_pin); 

//  Serial.print("HUM1: ");
//  Serial.println(hum1);
//  Serial.print("HUM2: ");
//  Serial.println(hum2);
  
  pot->lcd.setCursor(11,2); 
  pot->lcd.print("Hum: ");  
  
  int val = 100L - (((hum - HUM_VAL_LOW) * 100L) / HUM_VAL_HIGH);
  
  if (val > 100) {
    val = 100;
  }

  pot->lcd.print(val); 

  if (val < 100) {
    pot->lcd.print("% "); 
  }
  else {
    pot->lcd.print("%"); 
  }
  pot->hum = val;  
}

void do_name(Pot* pot) {
  pot->lcd.setCursor(0,1); 
  pot->lcd.print("Typ: "); 
  pot->lcd.print(pot->name); 
}

void do_fan(uint8_t hour, Pot* pot) {

  String fan_str = "OFF";
  // Run from x o'clock for one hour
  if ((hour == FAN_RUN_HOUR1) || (hour == FAN_RUN_HOUR2)) {
    fan_on();
    fan_str = "ON ";
  }
  else {
    fan_off();
  }
  
  pot->lcd.setCursor(0,2); 
  pot->lcd.print("Fan: ");
  pot->lcd.print(fan_str);
}

int do_time(Pot* pot) {
  
  uint8_t sec, min, hour, day, month;
  uint16_t year;
 
  /*get time from RTC*/
  rtc.get(&sec, &min, &hour, &day, &month, &year);
  
  pot->lcd.setCursor(0,0); 
  
  //lcd.print(100L - (((HUM2_VAL - 250)*100L)/600));
  //lcd.print("     ");

  //lcd.setCursor(0,2); 
  //lcd.print(HUM2_VAL);
  
  if (day < 10)
     pot->lcd.print("0");
   pot->lcd.print(day);
   
   pot->lcd.print(".");
   if (month < 10)
     pot->lcd.print("0"); 
   pot->lcd.print(month);
   pot->lcd.print(".");
   pot->lcd.print(year);
   pot->lcd.print("  ");

  if (hour < 10)
    pot->lcd.print("0"); 
  pot->lcd.print(hour); 
  pot->lcd.print(":"); 
  if (min < 10)
    pot->lcd.print("0"); 
  pot->lcd.print(min);
  pot->lcd.print(":");
  if (sec < 10)
    pot->lcd.print("0"); 
  pot->lcd.print(sec);

  return hour;
}

void setup() {

  /*init Serial port*/
  Serial.begin(9600);
  while(!Serial); /*wait for serial port to connect - needed for Leonardo only*/

  /*init fan*/
  pinMode(PIN_FAN, OUTPUT);
  fan_off();

  /*init PUMP*/
  pinMode(PIN_PUMP, OUTPUT);
  pump_off();

  /*init RTC*/
  Serial.println("Init RTC...");

  /*only set the date+time one time*/
  //rtc.set(0, 15, 13, 25, 6, 2020); /*08:00:00 24.12.2014 //sec, min, hour, day, month, year*/

  /*stop/pause RTC*/
  // rtc.stop();
  
  /*start RTC*/
  rtc.start();

  int i = 0;
  
  for (i = 0; i < NOF_POTS; i++) {
    Pots[i].lcd.begin(20,4);
    Pots[i].lcd.backlight();//Power on the back light
  }
}

void loop() {
 
  int i = 0;
  
  //Serial.println("---------------------");
  for (i = 0; i < NOF_POTS; i++) {

    Serial.println(i);
    do_name(&Pots[i]);
    
    uint8_t hour = do_time(&Pots[i]);
    do_fan(hour, &Pots[i]);  
    
    if (!(loop_cnt % HUM_UPDATE_FREQ)) {
      do_humidity(&Pots[i]);
    }

    // Only allow pump to run during daytime 8.00-20.00
    if ((hour > PUMP_MIN_HOUR) && (hour < PUMP_MAX_HOUR)) {
      do_humidity_reg(&Pots[i]);
    }
  }
    
  delay(MAIN_DELAY);

  loop_cnt++;

}
