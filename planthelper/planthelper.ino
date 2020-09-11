#include <Wire.h>
#include <DS1307.h>
#include <LiquidCrystal_I2C.h>

// General
#define NOF_POTS 2
#define MAIN_DELAY 1000

// Pump
#define PIN_PUMP 6
#define PUMP_RUN_TIME 5
#define PUMP_DELAY 600 // 600
#define PUMP_START_LEVEL 90
#define PUMP_MIN_HOUR 8
#define PUMP_MAX_HOUR 18

// Humidity
#define HUM_VAL_LOW 260
#define HUM_VAL_HIGH 600
#define PIN_HUM1 A0 
#define PIN_HUM2 A1 
#define HUM_UPDATE_FREQ 5
#define HUM_REG_FREQ 3600 // 3600

// Fan
#define PIN_FAN1 7
#define PIN_FAN2 8
#define FAN_RUN_HOUR_START 8
#define FAN_RUN_HOUR_END 18

// Water level sensor (digital)
#define SEN_WATER1 4
#define SEN_WATER2 5

// LED
//#define RED_PIN 2
//#define GREEN_PIN 3
//#define BLUE_PIN 4

// Valve
#define PIN_VALVE1 2
#define PIN_VALVE2 3

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
   int water_pin;
   int valve_pin;
   int fan_pin;
   
   int hum;
   int pump_timer;
   bool pump_activated;
   int water_level_alarm;
   bool want_to_pump;
} Pot;


Pot Pots[NOF_POTS] = {
  { lcd0, 0, "\"LAT KOLLANE\"", PIN_HUM2, SEN_WATER1, PIN_VALVE2, PIN_FAN2, 0, PUMP_DELAY, 0, 0, 0},
  { lcd1, 1, "\"JOG PUNANE\"", PIN_HUM1, SEN_WATER2, PIN_VALVE1, PIN_FAN1, 0, PUMP_DELAY, 0, 0, 0},
};

void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
{
//  analogWrite(RED_PIN, red_light_value);
//  analogWrite(GREEN_PIN, green_light_value);
//  analogWrite(BLUE_PIN, blue_light_value);
//  Serial.println("TERE");
//  Serial.println(red_light_value);
//  Serial.println(green_light_value);
//  Serial.println(blue_light_value);
}

void fan_on(int pin) {
  digitalWrite(pin, LOW);
}

void fan_off(int pin) {
  digitalWrite(pin, HIGH);
}

void pump_on() {
  digitalWrite(PIN_PUMP, LOW);
}

void pump_off() {
  digitalWrite(PIN_PUMP, HIGH);
}

void open_valve(int pin) {
  digitalWrite(pin, LOW);
}

void close_valve(int pin) {
  digitalWrite(pin, HIGH);
}

void do_water_level(Pot* pot) {

  // TODO
  // pot->water_level_alarm = digitalRead(pot->water_pin);
  pot->water_level_alarm = 0;

  pot->lcd.setCursor(0,3); 
  pot->lcd.print("Wat: "); 

  if (pot->water_level_alarm) {
    pot->lcd.print("ALR");
  }
  else {
    pot->lcd.print("OK!");
  }
  
  Serial.print(pot->name);
  Serial.print(", water level: ");
  Serial.println(pot->water_level_alarm);
}

void do_humidity_reg(Pot* pot) {
  if (!(loop_cnt % HUM_REG_FREQ)) {   
    if (pot->hum < PUMP_START_LEVEL) {
      Serial.println("Whaaaaaaaaaaaat");
      Serial.println(pot->hum);
      pot->pump_activated = 1;
    }    
  }
  if (pot->pump_activated) { 
      pot->pump_timer--;

      if (pot->pump_timer > 0) {
        pot->lcd.setCursor(11,3); 
        pot->lcd.print("Pmp: "); 
        pot->lcd.print(pot->pump_timer);

        if((pot->pump_timer) < 100) {
          pot->lcd.print(" "); 
        }
        else if((pot->pump_timer) < 10) {
          pot->lcd.print("  "); 
        }
        //pot->lcd.print(" sec ");
      }
      else if (pot->pump_timer >= (0 - PUMP_RUN_TIME)) {
        pot->lcd.setCursor(11,3); 
        pot->lcd.print("Pmp: NOW "); 
        Serial.println("!!!Panen puraka!!!");
        pot->want_to_pump = true;
      }
      else if (pot->pump_timer <= (0 - PUMP_RUN_TIME)) {
        pot->pump_timer = PUMP_DELAY;
        pot->pump_activated = 0;
        pot->lcd.setCursor(11,3); 
        pot->lcd.print("Pmp: FUT ");
        pot->want_to_pump = false;
      }
  }
  else {
    pot->lcd.setCursor(11,3); 
    pot->lcd.print("Pmp: FUT ");
  } 
}

void do_humidity(Pot* pot) {

  int hum = analogRead(pot->hum_pin); 

  Serial.print(pot->name);
  Serial.print(": ");
  Serial.println(hum);
  
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
  if ((hour >= FAN_RUN_HOUR_START) && (hour < FAN_RUN_HOUR_END)) {
    fan_on(pot->fan_pin);
    fan_str = "ON ";
  }
  else {
    fan_off(pot->fan_pin);
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

int do_pump(Pot* pot, int hour) {
  if (pot->water_level_alarm) {
    pot->pump_timer = PUMP_DELAY;
    pot->pump_activated = 0;
    pot->lcd.setCursor(11,3); 
    pot->lcd.print("Pmp: ALR ");
    pot->want_to_pump = false;
  }
  else if ((hour > PUMP_MAX_HOUR) || (hour < PUMP_MIN_HOUR)) {
    pot->pump_timer = PUMP_DELAY;
    pot->pump_activated = 0;
    pot->lcd.setCursor(11,3); 
    pot->lcd.print("Pmp: DIS ");
    pot->want_to_pump = false;
  }
}

void do_led(int loop_cnt) {
  
  switch (loop_cnt % 8) {
  case 0:
    RGB_color(255, 0, 0); // Red
    break;
  case 1:
    RGB_color(0, 255, 0); // Green
    break;
  case 2:
    RGB_color(0, 0, 255); // Blue
    break;
  case 3:
    RGB_color(255, 255, 125); // Raspberry
    break;
  case 4:
    RGB_color(0, 255, 255); // Cyan
    break;
  case 5:
    RGB_color(255, 0, 255); // Magenta
    break;
  case 6:
    RGB_color(255, 255, 0); // Yellow
    break;
  case 7:
    RGB_color(255, 255, 255); // White
    break;
    
  default:
    // statements
    break;
  }
}



void setup() {

  /*init Serial port*/
  Serial.begin(115200);
  while(!Serial); /*wait for serial port to connect - needed for Leonardo only*/

  /*init fan*/
  pinMode(PIN_FAN1, OUTPUT);
  pinMode(PIN_FAN2, OUTPUT);
  //fan_off();

  /*init PUMP*/
  pinMode(PIN_PUMP, OUTPUT);
  pump_off();

  pinMode(PIN_VALVE1, OUTPUT);
  pinMode(PIN_VALVE2, OUTPUT);

  /*init RTC*/
  Serial.println("Init RTC...");

  /*only set the date+time one time*/
  //rtc.set(0, 20, 14, 25, 8, 2020); /*08:00:00 24.12.2014 //sec, min, hour, day, month, year*/

  /*stop/pause RTC*/
  // rtc.stop();
  
  /*start RTC*/
  rtc.start();

  int i = 0;
  
  for (i = 0; i < NOF_POTS; i++) {
    Pots[i].lcd.begin(20,4);
    Pots[i].lcd.backlight();//Power on the back light
  }

  pinMode(SEN_WATER1,INPUT_PULLUP);
  pinMode(SEN_WATER2,INPUT_PULLUP);  

//
//  pinMode(RED_PIN, OUTPUT);
//  pinMode(GREEN_PIN, OUTPUT);
//  pinMode(BLUE_PIN, OUTPUT);
  
}

void loop() {
 
  int i = 0;

  Serial.println("---------------------");
  for (i = 0; i < NOF_POTS; i++) {

    Pots[i].want_to_pump = false;
    
    do_name(&Pots[i]);
    
    uint8_t hour = do_time(&Pots[i]);
    do_fan(hour, &Pots[i]);  
    
    if (!(loop_cnt % HUM_UPDATE_FREQ)) {
      do_humidity(&Pots[i]);
    }

    do_water_level(&Pots[i]);
    
    do_pump(&Pots[i], hour);

    if (!Pots[i].water_level_alarm) {
      // Only allow pump to run during daytime 
      if ((hour > PUMP_MIN_HOUR) && (hour < PUMP_MAX_HOUR)) {
          do_humidity_reg(&Pots[i]);
      }
    }
  }

  bool pump = false;
  for (i = 0; i < NOF_POTS; i++) {

//    Serial.print("Loop count: ");
//    Serial.println(loop_cnt);
//    
//    if (loop_cnt % 2) {
//      Pots[i].want_to_pump = true;
//    }
//    else{      
//      Pots[i].want_to_pump = false;
//    }

        
    if (Pots[i].want_to_pump == true) {
      Serial.print (Pots[i].name);
      Serial.println(" OPEN");
      open_valve(Pots[i].valve_pin);
      pump = true;
    }
    else {
      Serial.print (Pots[i].name);
      Serial.println("CLOSE");
      close_valve(Pots[i].valve_pin);
    }
  }
  
  if (pump) {
    pump_on();
  }
  else {
    pump_off();
  }

//fan_on();
  
  //do_led(loop_cnt);
      
  delay(MAIN_DELAY);
  
  loop_cnt++;

}
