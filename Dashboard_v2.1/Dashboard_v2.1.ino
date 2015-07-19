// Date and time functions using just software, based on millis() & timer

#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "FastLED.h"
#include <SoftwareSerial.h>
#include <SerialCommand.h>
#include "EEPROM.h"
// #include "TimerOne.h"

#define DEBUG 

#ifdef DEBUG
 #define D 
#else
 #define D for(;0;)
#endif

#ifndef DEBUG
 #define ND for(;0;)
#else
 #define ND 
#endif


#define DATA_PIN    A5
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    53
#define OLED_RESET  4
#define BRIGHTNESS  50

RTC_Millis rtc;
CRGB leds[NUM_LEDS], bg, events, today;
Adafruit_SSD1306 display(OLED_RESET);
SerialCommand sCmd;
char clockMsg[100]; 
// boolean ledState = 1, lastsec=1;
int light_low, light_high;
DateTime now;


void setup () {
    delay(5000);
    Serial.begin(38400);
    // Serial.println("SERIAL STARTED");
    
    // following line sets the RTC to the date & time this sketch was compiled
    if (EEPROM.read(99) != 1){               // Check if colours have been set or not
      EEPROM.write(0,0x19);                   // bg Colour - R-G-B - White
      EEPROM.write(1,0x19);
      EEPROM.write(2,0x70);
      EEPROM.write(3,0xFF);                   // events Colour - R-G-B - Red
      EEPROM.write(4,0x45);
      EEPROM.write(5,0x00);
      EEPROM.write(6,0x87);                     // today Colour - R-G-B - Green
      EEPROM.write(7,0xCE);
      EEPROM.write(8,0xFA);
      /* EEPROM.write(9,0);                     // BG Colour - R-G-B - Black
      EEPROM.write(10,0);
      EEPROM.write(11,0);*/
      EEPROM.write(12, 0);                   // Light sensitivity - low
      EEPROM.write(13, 55);                  // Light sensitivity - high 
      // EEPROM.write(14, 15);                  // Minutes for each rainbow   
      EEPROM.write(99,1);
    } 
    else {
      bg.r = EEPROM.read(0);
      bg.g = EEPROM.read(1);
      bg.b = EEPROM.read(2);  
      events.r = EEPROM.read(3);
      events.g = EEPROM.read(4);
      events.b = EEPROM.read(5);
      today.r = EEPROM.read(6);
      today.g = EEPROM.read(7);
      today.b = EEPROM.read(8);
      /*bg.r = EEPROM.read(9);
      bg.g = EEPROM.read(10);
      bg.b = EEPROM.read(11);*/
      light_low = EEPROM.read(12);
      light_high = EEPROM.read(13); 
      // rain = EEPROM.read(14); 
    }
    
    D Serial.println("EEPROM READ");

    // ********** Setup the serial commands
    sCmd.addCommand("TIME", set_time);
    sCmd.addCommand("MISSED", missedCall);
    sCmd.addCommand("MISSEDOFF", missedOff);
    sCmd.addCommand("BG", set_bg);
    sCmd.addCommand("EVENT", set_events);
    sCmd.addCommand("TODAY", set_today);
    
    
    rtc.begin(DateTime(__DATE__, __TIME__));
    Serial.println("RTC SET");
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.setTextSize(3);
    display.setTextColor(WHITE);
    D Serial.println("DISPLAY READY");
    
    FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(light_high); 
    fill_solid(leds, NUM_LEDS, bg);
    calendar();
    FastLED.show();  
    // Timer1.initialize();
    // Timer1.attachInterrupt(state, 500);
    // ledState = 1;
    D Serial.println("SETUP COMPLETE");
}

void loop () {
  EVERY_N_MILLISECONDS(100){
      sCmd.readSerial();
    }
  EVERY_N_SECONDS(30){
      display.clearDisplay();
      now = rtc.now();
      sprintf(clockMsg,"%02d:%02d",now.hour() % 12,now.minute());
      display.setCursor(20,10);
      display.println(clockMsg);
      calendar();
    
      display.display();
      FastLED.show();
  }  
}

void calendar() {
  fill_solid(leds, NUM_LEDS, bg);
  D Serial.println("CALENDAR");
  D Serial.println(now.dayOfWeek());
  byte firstSunday = (now.day() - now.dayOfWeek()) % 7;                     // 0 is Sunday
  byte firstSaturday = firstSunday -1;                    
  for(byte i=0; i<31; i++)
    leds[i]=bg;
  for (byte i = 0; i < 5; i++)
  {
    if(firstSunday + i*7 < 31) {
      leds[firstSunday + i*7-1] = CRGB(0,255,0);
      D { Serial.print("Sunday: "); Serial.println(firstSunday + i*7);}
    }
    if(firstSaturday + i*7 < 31) {
      leds[firstSaturday + i*7-1] = CRGB(0,255,0);
      D { Serial.print("Saturday: "); Serial.println(firstSaturday + i*7);}
    }
  }
  leds[now.day()-1]=today;
  leds[now.month()+31-1] = today; 
  if(now.month() == 2){                              // February
    if(now.year() %4 == 0) {                         // Account for leap year
      leds[29] = leds [30] = CRGB:: Black;
    }
    else {
      leds[28] = leds[29] = leds [30] = CRGB::Black;
    }
  }
  if(now.month() == 4 || now.month() == 6 || now.month() == 9 || now.month() == 11)
    leds[30] = CRGB::Black;
}

/*void state(){
  ledState = 1;
}*/

void missedCall()
{
    leds[47] = events;
    FastLED.show();
}

void missedOff()
{
    leds[47] = bg;
    FastLED.show();
}

void set_today(){
  today.r = atoi(sCmd.next());
  today.g = atoi(sCmd.next());
  today.b = atoi(sCmd.next());
  EEPROM.write(6,today.r);
  EEPROM.write(7,today.g);
  EEPROM.write(8,today.b);
  Serial.println("TODAY COLOUR SET");  
}

void set_events(){
  events.r = atoi(sCmd.next());
  events.g = atoi(sCmd.next());
  events.b = atoi(sCmd.next());
  EEPROM.write(3,events.r);
  EEPROM.write(4,events.g);
  EEPROM.write(5,events.b); 
  Serial.println("EVENT COLOUR SET");  
}

void set_bg(){
  bg.r = atoi(sCmd.next());
  bg.g = atoi(sCmd.next());
  bg.b = atoi(sCmd.next());
  EEPROM.write(0,bg.r);
  EEPROM.write(1,bg.g);
  EEPROM.write(2,bg.b);
  Serial.println("BG COLOUR SET"); 
}

void set_time(){
  DateTime now = rtc.now();
  Serial.println("TIME SET");
  uint8_t hr = (uint8_t)atoi(sCmd.next());
  uint8_t minu = (uint8_t)atoi(sCmd.next());
  uint8_t sec = (uint8_t)atoi(sCmd.next());
  uint8_t yea = (uint8_t)atoi(sCmd.next());
  uint8_t mont = (uint8_t)atoi(sCmd.next());
  uint8_t da = (uint8_t)atoi(sCmd.next());
  Serial.println(hr);
  Serial.println(minu);
  Serial.println(sec);
  rtc.adjust(DateTime(yea,mont,da,hr,minu,sec));
}

