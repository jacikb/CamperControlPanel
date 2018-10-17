// PORTA |= (1<<PA3);   //ustaw bit PA3
// PORTA &= ~(1<<PA2);   //zeruj bit PA2
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif
#define REFRESH_SENSORS 5000
#include <EEPROM.h>
#include <SD.h>
#include <SPI.h>
#include<stdlib.h>
#include "RF24.h"  // Download and Install (See above)
#include "printf.h" // Needed for "printDetails" Takes up some memory
#include "rf_data.h"
#include "colors.h"

//RTCLib by Neiro
#include <Wire.h>
#include "RTClib.h"
DS1307 rtc;


#define BOOT_MODEM 30

//#define ENABLE_AUDIO

#ifdef ENABLE_AUDIO
#define mp3 Serial3    // Connect the MP3 Serial Player to the Arduino MEGA Serial3 (14 TX3 -> RX, 15 RX3 -> TX)
static int8_t mp3_send_buf[8] = {0}; // Buffer for Send commands.  // BETTER LOCALLY
static uint8_t mp3_ansbuf[10] = {0}; // Buffer for the answers.    // BETTER LOCALLY
String mp3Answer;           // Answer from the MP3.
boolean autoResume = true;
#endif
/************ Command byte **************************/
#define CMD_NEXT_SONG     0X01  // Play next song.
#define CMD_PREV_SONG     0X02  // Play previous song.
#define CMD_PLAY_W_INDEX  0X03
#define CMD_VOLUME_UP     0X04
#define CMD_VOLUME_DOWN   0X05
#define CMD_SET_VOLUME    0X06

#define CMD_SNG_CYCL_PLAY 0X08  // Single Cycle Play.
#define CMD_SEL_DEV       0X09
#define CMD_SLEEP_MODE    0X0A
#define CMD_WAKE_UP       0X0B
#define CMD_RESET         0X0C
#define CMD_PLAY          0X0D
#define CMD_PAUSE         0X0E
#define CMD_PLAY_FOLDER_FILE 0X0F

#define CMD_STOP_PLAY     0X16
#define CMD_FOLDER_CYCLE  0X17
#define CMD_SHUFFLE_PLAY  0x18 //
#define CMD_SET_SNGL_CYCL 0X19 // Set single cycle.

#define CMD_SET_DAC 0X1A
#define DAC_ON  0X00
#define DAC_OFF 0X01

#define CMD_PLAY_W_VOL    0X22
#define CMD_PLAYING_N     0x4C
#define CMD_QUERY_STATUS      0x42
#define CMD_QUERY_VOLUME      0x43
#define CMD_QUERY_FLDR_TRACKS 0x4e
#define CMD_QUERY_TOT_TRACKS  0x48
#define CMD_QUERY_FLDR_COUNT  0x4f

/************ Opitons **************************/
#define DEV_TF            0X02



// (Create an instance of a radio, specifying the CE and CS pins. )
#if defined(__AVR_ATmega2560__)
#define  CE_PIN  48   // The pins to be used for CE and SN
#define  CSN_PIN 49
unsigned char PCB[] = "MEGA";
#else
#define  CE_PIN  9   // The pins to be used for CE and SN
#define  CSN_PIN 10
unsigned char PCB[] = "UNO";
#endif
String GSMResult[10];
byte GSMResultCount = 0;
RF24 Radio(CE_PIN, CSN_PIN);
/*-----( Declare Variables )-----*/
byte addresses[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node", "7Node"};
s_rf_data  dataSend, dataReceived;

#define CHANNEL 118


/* ENCODER */
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
#define ENCODER_1 43
#define ENCODER_2 41
#define ENCODER_push 39

uint8_t ENCODER;
Encoder myEnc(ENCODER_1, ENCODER_2);
String sms_message;
/* ENCODER */

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
#include "definition.h"
#include "icons-logo.h"
#include "icons-40x32.h"
#include "icons-32x32.h"
#include "icons-24x24.h"
#include "icons-24x32.h"

#define YP A2  // must be an analog pin, use "An" notation!
#define XM A1  // must be an analog pin, use "An" notation!

#define YM 6   // can be a digital pin
#define XP 7   // can be a digital pin


// Original values
//#define TS_MINX 150
//#define TS_MINY 220
//#define TS_MAXX 960
//#define TS_MAXY 800

// Calibrate values dla Rotation 0


TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define BUTTON_C BLUE
#define BUTTON_W 100
#define BUTTON_H 80

#define BUTTON_TW 100
#define BUTTON_TH 40
#define BUTTON_ROUND 8
#define CHK_W 35
#define CHK_H 35
#define CHK_C WHITE
//TSPoint tsp ;


File bmpFile;

#define BUFFPIXEL       20                      // must be a divisor of 240 
#define BUFFPIXEL_X3    60                     // BUFFPIXELx3

char  inputString[21];
//char inputTextValue[11];
// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
//TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
// optional
#define LCD_RESET A4

// Assign human-readable names to some common 16-bit color values:





#define ICON_ACTIVE YELLOW
#define ICON_ERROR RED

#define BUTTON_BG BLUEd


#define BOX_BG BLUEd
#define BOX_INPUT_BG YELLOWl
#define BOX_INPUT_TEXT BLUEd

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

int MODULES[10];
struct gsm_t {
  bool connected;
  uint8_t cmd;
  String result;
};
gsm_t gsm;

struct mouse_t {
  uint16_t x;
  uint16_t y;
  uint16_t c;
};

struct type_module_t {
  uint8_t module;
  uint8_t type;
  uint8_t nr;
};

type_module_t type_module;

#define BUFFPIXEL 20


#define MaxSensors 16
#define MaxSensorsData24H 24
#define MaxSensorsData1H 12


struct sensor_data_t {
  int data24H[MaxSensorsData24H];
  int data1H[MaxSensorsData1H];
};
#define SENSORS_KEY 123

struct sensor_t {
  uint8_t type;
  uint16_t id;

  uint8_t active;
  uint8_t options;
  uint8_t icon;
  int val;

  int min;
  int max;
  int alarm_min;
  int alarm_max;
  char name[11];
};

struct settings_t  {
  byte sensors_key;

  uint16_t alarm;//G*60+m
  byte mp3_volume;
  uint16_t mp3_start;
  uint16_t mp3_click;
  uint16_t mp3_alarm;
  uint16_t mp3_1;
  uint16_t mp3_2;
  uint16_t mp3_3;
  uint16_t mp3_4;


  char gsm_network[11];
  char mobile1[11];
  char mobile2[11];
  char mobile3[11];
  DateTime StartTime;


};


sensor_t SENSORS[MaxSensors];
uint8_t SENSORS_ind;
sensor_data_t SENSORSD[MaxSensors];


settings_t Settings;

#define TouchMAX 40
struct box_t {
  uint16_t x;
  uint16_t y;
  uint8_t w;
  uint8_t h;
  uint16_t c;
  int id;
};

box_t LAST_BOX;


box_t Touch[TouchMAX];
uint8_t TouchCount;


volatile int EKRAN, PAGE;
uint32_t refresh_time;
//mouse_t mouse;


byte first_start;

byte GraphGfxStepX;


uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}
uint16_t read16v2(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[1] = f.read(); // LSB
  ((uint8_t *)&result)[0] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}



void setup(void) {
  PAGE = 0;
  ENCODER = 0;
  pinMode(ENCODER_push, INPUT_PULLUP);
  pinMode(BOOT_MODEM, OUTPUT);
  digitalWrite(BOOT_MODEM, 0);
  randomSeed(analogRead(15));

  Serial.begin(115200);
  Serial1.begin(115200);

  while (!Serial) {
    ;
  }
#ifdef ENABLE_AUDIO
  mp3.begin(9600);
  while (!mp3) {;}
  mp3_sendCommand(CMD_RESET, 0); // mp3_sendCommand(CMD_SET_DAC ,DAC_ON);
#endif

  SensorsInit();
  digitalWrite(BOOT_MODEM, 1);

  Serial.println(F("Jacik Camp v1"));
  RTC_Init();
  RF_Init();
  TFT_Init();
  gsm.connected = false;
  GSM_Connect();


  strcpy(inputString, "");

#ifdef ENABLE_AUDIO
  delay(200);
  mp3_sendCommand(CMD_SEL_DEV, DEV_TF);
#endif



  DrawMAIN(R_FULL);
  refresh_time = millis();
#ifdef ENABLE_AUDIO
  mp3_sendCommand(CMD_SET_VOLUME, 15);
#endif
  delay(20);
  Play(P_WELCOME);


  first_start = 1;

  //GSM_Report(Settings.mobile1); //test gsm send


}


void loop()
{
  
  TouchAction();

  if ( millis() - refresh_time > REFRESH_SENSORS)
  {
    refresh_time = millis();
    if (EKRAN == E_MAIN)  {
      ReadSensorsAll();
      DrawMAIN(R_UPDATE);
    }
    if (first_start)
    {
      //sms_message.reserve(100); sms_message = "AutoCamp online";  SMS(Settings.mobile1);
      //GSM_Report(Settings.mobile1);
      first_start = 0;
    }
    //TestModem();
  }
 
}

void DrawButtonGfx(int x, int y, uint8_t w, uint8_t h, uint16_t color)
{
  LAST_BOX.x = x;
  LAST_BOX.y = y;
  LAST_BOX.w = w;
  LAST_BOX.h = h;
  LAST_BOX.c = YELLOW;

  tft.fillRoundRect(x , y , w, h, 8, color);
  tft.drawRoundRect(x, y, w,  h , 8,  YELLOW);
}

void DrawButtonGfx(int x, int y, uint8_t w, uint8_t h)
{
  DrawButtonGfx(x, y, w, h, BUTTON_C);
}

void DrawButtonMENU(int nr, uint32_t icon_nr, uint16_t color, int value, uint8_t type, int button_id, uint8_t active, uint8_t refresh)
{
  sensor_t s;
  int x, y, r;
  uint16_t c;
  char buff[6];
  uint32_t   adr_icon;
  float v;
  s = SENSORS[nr];
  y = nr  / 4;
  x = nr  % 4;
  v = (float)value / 10;

  tft.setTextSize (2);
  tft.setTextColor(RED);

  x = 18 + x * (BUTTON_W + 15);
  y = 40 + y * (BUTTON_H + 15);

  if (refresh == R_FULL) {
    if (type >= 10)
      DrawButtonGfx(x, y, BUTTON_W, BUTTON_H, BLUEl);
    else
      DrawButtonGfx(x, y, BUTTON_W, BUTTON_H);

    if (icon_nr > 30) adr_icon = icon_nr;
    else adr_icon = ICONS40[icon_nr];


    if (button_id > 0)
    {

      if (active)
        tft.drawBitmap3D(x + 14, y + 7, adr_icon, 40, 32, color);
      else
        tft.drawBitmap3D(x + 14 , y + 7 , adr_icon, 40, 32, GRAY);
    }
    else
    {
      tft.drawBitmap3D(x + 25, y + 7, adr_icon, 40, 32, color);
    }
  }

  if (nr < MaxSensors) if (refresh == R_UPDATE &&  SENSORS[nr].active < 2) refresh = R_NONE;
  
  if (SENSORS[nr].active > 1) SENSORS[nr].active = 1;
  
  if (active) {
    if (refresh == R_UPDATE )
    {
      tft.fillRect( x + 5, y + 50, 71, 16, BUTTON_C);
    }
    if (refresh != R_NONE) {
      r = 0;
      if (type == T_VOLT)
      {
        dtostrf((float)value / 10, 3, 1, buff);
        strcat(buff, "V");
        Print3D(x + 5, y + 50, buff);
        r = map(value / 10, s.min, s.max, 1, 50);
      }
      if (type == T_TEMP)
      {
        dtostrf(v , 3, 1, buff);
        strcat(buff, "C");
        Print3D(x + 15, y + 50, buff);
        r = map(v, s.min, s.max, 1, 50);
      }
      if (type == T_PROC || type == T_PWM)
      {
        r = map(v, s.min, s.max, 1, 50);
        sprintf(buff, "%d %%", (int)value);
        Print3D(x + 20, y + 50, buff);
      }
      if (type == T_OUTS)
      {
        r = -1; // = map(v, s.min, s.max, 1, 50);
        STxtIntLn(F("OnOff Value"), SENSORS[nr].val);
        if (SENSORS[nr].val >= 1) strcpy(buff, "On");
        else strcpy(buff, "Off");
        Print3D(x + 20, y + 50, buff);
      }

      if (button_id > 0 && r >= 0) {
        tft.drawRoundRect(x + BUTTON_W - 24, y + 5, 18, 60, 5, GRAY);
        tft.fillRoundRect(x + BUTTON_W - 24 + 1, y + 5 + 1, 18 - 2, 60 - 2, 5, tft.color565(0, 0, 55));
        r = 50 - r;
        tft.fillRoundRect(1 + x + BUTTON_W - 24, 1 + y + 5 + r, 16, 58 - r, 5, WHITE);
      }
    }
  }

  if (type == T_PAGE)  Print3D(x + 25, y + 50, F("PAGE"));
  if (type == T_ALARM) Print3D(x + 20, y + 50, F("ALARM"));
  if (type == T_SETUP) Print3D(x + 20, y + 50, F("SETUP"));

  if (refresh == R_FULL)  TouchAdd(button_id);
}

uint8_t read8(File f) {
  return  f.read(); // LSB

}

void DrawB16(char *filename, int x, int y) {
  File     bmpFile;
  uint16_t      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth

  int      w, h, row, col;
  uint8_t  r, g, b;
  uint16_t color;

  if ((x >= tft.width()) || (y >= tft.height())) return;


  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print(F("File not found "));
    Serial.println(filename);
    return;
  }
  bmpWidth = read16(bmpFile) ;
  bmpHeight = read16(bmpFile) ;
  bmpDepth = read8(bmpFile) ;

  w = bmpWidth;
  h = bmpHeight;

  if ((x + w - 1) >= tft.width())  w = tft.width()  - x;
  if ((y + h - 1) >= tft.height()) h = tft.height() - y;

  for (row = 0; row < h; row++) { // For each scanline...
    for (col = 0; col < w; col++)
    {
      color = read16v2(bmpFile);
      tft.drawPixel(x + col, y + row, color);
    }

  }
  bmpFile.close();
}
void DrawHeader(uint8_t tryb)
{
  int x, y;
  uint16_t c;
  char tmp[21], buff[100];

  DateTime now = rtc.now();


  x = 0; y = 0;

  if (MODULES[0] > 0) c = GREEN;
  else c = RED;
  
  tft.drawBitmap(x, y, icon_ant1, 24, 32, c);

  x += 24;

  if (MODULES[1] > 0) c = GREEN;
  else c = RED;
  tft.drawBitmap(x, y, icon_ant2, 24, 32, c);

  x += 24;

  if (gsm.connected) c = GREEN;
  else c = RED;
  tft.drawBitmap(x, y, icon_antg, 24, 32, c);

  x += 24;  x += 5;

  tft.setCursor (x, 5);
  tft.setTextSize (3);
  tft.setTextColor(WHITE);

  strncpy(buff, "hh:mm  DD \0", 100);
  strcpy(tmp, now.format(buff));
  strcat(tmp, miesiace_pl[now.month() - 1]);

  tft.print(tmp);
  tft.setTextSize (2);
  tft.setCursor (350, 0);

  long int dh = (long int) ( now.unixtime() - Settings.StartTime.unixtime()) / UNIX_DAY  ;

  if (dh < 1 || dh > 300) dh = 0;

  tft.print(F("("));
  tft.print(dh);
  tft.print(F(" dzien)"));

  tft.setCursor (370, 20);
  tft.setTextSize (1);
  tft.print(F("AutoCamp v0.2"));

}

//Main Menu
void DrawMAIN(uint8_t refresh)
{
  uint32_t time = millis();

  int x, y;
  uint8_t ok;
  int i, id;
  uint16_t c;
  char tmp[21],  buff[100];
  //uint32_t time = millis();
  if ( refresh == R_FULL) {
    EKRAN = E_MAIN;

    TouchClear();
    tft.fillScreen(BLACK);

    DrawHeader(R_DRAW);
  }
  else
  {
    DateTime now = rtc.now();
    x = 0; y = 0;
    if (MODULES[0] > 0) c = GREEN;
    else c = RED;
    tft.drawBitmap(x, y, icon_ant1, 24, 32, c);
    x += 24;
    if (MODULES[1] > 0) c = GREEN;
    else c = RED;
    tft.drawBitmap(x, y, icon_ant2, 24, 32, c);
    x += 24;

    if (gsm.connected) c = GREEN;
    else c = RED;
    tft.drawBitmap(x, y, icon_antg, 24, 32, c);
    x += 24;

    x += 5;

    tft.setCursor (x, 5);
    tft.setTextSize (3);
    tft.setTextColor(WHITE);

    strncpy(buff, "hh:mm  DD \0", 100);
    strcpy(tmp, now.format(buff));
    strcat(tmp, miesiace_pl[now.month() - 1]);
    tft.fillRect(74, 5, 270, 22, BLACK);
    tft.print(tmp);

    //refresh days 
    if ((now.hour() == 0) && (now.second() < 30))
    {
      long int dh = (long int) ( now.unixtime() - Settings.StartTime.unixtime()) / UNIX_DAY  ;
      if (dh < 1 || dh > 300) dh = 0;
      tft.setTextSize (2);
      tft.setCursor (350, 0);
      tft.fillRect(350, 0, 130, 14, BLACK);
      tft.print(F("("));
      tft.print(dh);
      tft.print(F(" dzien)"));
    }
  }

  for (i = 0; i < 8 ; i++)  DrawButtonSensor(i, refresh, PAGE);

  if (refresh == R_FULL) {
    tft.drawBitmap(0, 230, icon_camper_team_car, 128, 64, WHITE);
    tft.setTextSize (2);
    tft.setTextColor (WHITE);
    tft.setCursor(40, 300);
    tft.print(F("Jacik"));

    DrawButtonMENU(9, icon_wiatrak, YELLOW, 0, T_PAGE, K_PAGE, 1, refresh);
    DrawButtonMENU( 10, icon_alarm, YELLOW, 0, T_ALARM, K_ALARMS, 1, refresh);
    DrawButtonMENU( 11, icon_setup, YELLOW, 0, T_SETUP, K_SETUP, 1, refresh);

  }

  Serial.print(millis() - time, DEC);  Serial.println(" ms");

}

uint8_t RunPress()
{
  String s;
  int len, i;
  uint8_t a, b, buff[11];

  len = strlen(inputString);

  if (len == 0) return 0;

  if ((inputString[0] == "M" ) || (inputString[0] == "m"))
  {

    return 1;

  }

  return 0;
}

void DrawButtonText(int nr, int button_id, const __FlashStringHelper *text )
{

  int x, y, r;
  uint16_t c;
  y = nr / 8;
  x = nr % 8;


  tft.setTextSize (2);
  tft.setTextColor(RED);


  x = 18 + x * (BUTTON_TW + 15);
  y = 40 + y * (BUTTON_TH + 15);

  DrawButtonGfx(x, y, BUTTON_TW, BUTTON_TH);

  tft.setTextSize (2);
  tft.setTextColor(BLUEd);
  tft.setCursor(x + 7, y + 12);
  tft.print(text);
  tft.setTextColor(WHITE);
  tft.setCursor(x + 5, y + 12); //TH=40    40-16/2=12
  tft.print(text);
  TouchAdd(button_id);
}

void DrawButtonText(int nr, String text)
{

  int x, y, r;
  uint16_t c;
  y = nr / 4;
  x = nr % 4;


  tft.setTextSize (2);
  tft.setTextColor(RED);

  x = 18 + x * (BUTTON_TW + 15);
  y = 40 + y * (BUTTON_TH + 15);

  DrawButtonGfx(x, y, BUTTON_W, BUTTON_H);
  tft.setTextSize (2);
  tft.setTextColor(WHITE);
  tft.setCursor(x + 5, y + 10);
  tft.print(text);
}

uint8_t GetTouch()
{
  char c;
  uint8_t ok;

  ok = 0;

  while (Serial.available() > 0) {
    c = (char)Serial.read();
    if (c == 10) {
      Serial.println(inputString);
      ok = RunPress();
      strcpy(inputString, "");
    }
    else if (c >= 32)     strcat(inputString, c);
  }

  return ok;
}
//Draw Screen
void DrawEkran(uint8_t ekran)
{
  uint8_t tryb;
  if (ekran != EKRAN) tryb = R_FULL;
  else tryb = R_UPDATE;
  EKRAN = ekran;
  switch (ekran) {
    case E_MAIN:
      DrawMAIN(tryb);
      break;

    case E_GRAPH:
      //      Graph(0, 50, 1);
      break;
  }
}
uint8_t * AdrIcon(uint8_t nr)
{
  if (nr < ICONS40_count)   return ICONS40[nr];
  else return 0;


}

void SensorsLoad()
{
  EEPROM.get(200, SENSORS);
}
void SensorsSave()
{
  EEPROM.put(200, SENSORS);
}
void SettingsLoad()
{
  EEPROM.get(0, Settings);
}
void SettingsSave()
{
  EEPROM.put(0, Settings);
}
void SensorsInit()
{
  uint8_t i, j, delta;

  SettingsLoad();
  strcpy(Settings.mobile1, "737456220");//My mobile number
  strcpy(Settings.mobile2, "");
  strcpy(Settings.mobile3, "");

  memset(SENSORSD, 0, sizeof(SENSORSD));

  SensorsLoad();
  if (Settings.sensors_key != SENSORS_KEY)
  {
    SensorsSetDefault();
    SensorsSave();
  }

  for (i = 0; i < MaxSensors; i++) {
    if (SENSORS[i].min < 500) SENSORS[i].min = 0;
    if (SENSORS[i].max > 1000) SENSORS[i].max = 1000;
    if (SENSORS[i].min > SENSORS[i].max) SENSORS[i].max = SENSORS[i].min + 10;
    if (SENSORS[i].id < 1) SENSORS[i].active = 0;
    SENSORS[i].val = 0;
  }

  Settings.sensors_key = SENSORS_KEY;
  SettingsSave();

}
uint8_t DrawTextBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char  *txt, int button_id)
{
  DrawBox(x, y, w, h, WHITE, BOX_BG);
  tft.setTextSize (2);
  tft.setCursor(x + 10, y + ((h - 12) / 2));
  tft.print(txt);
  TouchAdd(button_id);

}

uint8_t DrawTextButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char  *txt, int button_id)
{
  DrawBox(x, y, w, h, WHITE, BOX_BG);
  tft.setTextSize (2);
  tft.setCursor(x + 10, y + ((h - 12) / 2));
  tft.print(txt);
  TouchAdd(button_id);
}

uint8_t DrawTextBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const __FlashStringHelper  *txt, int button_id)
{
  DrawBox(x, y, w, h, WHITE, BOX_BG);
  tft.setTextSize (2);
  tft.setCursor(x + 10, y + ((h - 12) / 2));
  tft.print(txt);
  TouchAdd(button_id);

}

uint8_t DrawInputBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char  *txt, int button_id)
{
  DrawBox(x, y, w, h, WHITE, BOX_INPUT_BG);
  tft.setTextSize (2);
  tft.setCursor(x + 10, y + ((h - 12) / 2));
  tft.print(txt);
  TouchAdd(button_id);

}


uint8_t DrawBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color1, uint16_t color2)
{
  DrawBox(x, y, w, h, color1, color2, 0, 0);

}

uint8_t DrawBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color1, uint16_t color2, uint8_t* icon, int button_id)
{
  LAST_BOX.x = x;
  LAST_BOX.y = y;
  LAST_BOX.w = w;
  LAST_BOX.h = h;
  LAST_BOX.c = color1;


  tft.drawRoundRect( x + 2, y + 2, w, h,  BUTTON_ROUND, GRAY);
  tft.drawRoundRect( x + 1, y + 1, w, h,  BUTTON_ROUND, GRAY);

  tft.fillRoundRect( x, y, w, h,  BUTTON_ROUND, color2);
  tft.drawRoundRect( x, y, w, h,  BUTTON_ROUND, color1);
  if (icon > 0)
    tft.drawBitmap(x + 10, y + ((h - 32) / 2), icon, 32, 32, WHITE);
  TouchAdd(button_id);
}

uint8_t DrawButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color1, uint16_t color2, uint8_t * icon, int button_id)
{
  LAST_BOX.x = x;
  LAST_BOX.y = y;
  LAST_BOX.w = w;
  LAST_BOX.h = h;
  LAST_BOX.c = color1;


  tft.drawRoundRect( x + 2, y + 2, w, h,  BUTTON_ROUND, GRAY);
  tft.drawRoundRect( x + 1, y + 1, w, h,  BUTTON_ROUND, GRAY);

  tft.fillRoundRect( x, y, w, h,  BUTTON_ROUND, color2);
  tft.drawRoundRect( x, y, w, h,  BUTTON_ROUND, color1);
  if (icon > 0)
    tft.drawBitmap(x + 10, y + ((h - 32) / 2), icon, 32, 32, WHITE);
  TouchAdd(button_id);
}



void TouchClear()
{
  TouchCount = 0;
  memset(Touch, 0, sizeof(Touch));
}


void TouchAdd(int id)
{
  if (id == 0) return;
  if (TouchCount < TouchMAX - 1 && id != 0 )
  {
    if (id != 0)LAST_BOX.id = id;
    Touch[TouchCount ] = LAST_BOX;
    TouchCount ++;
  }
}

int TouchId(uint8_t wait_press)
{
  int  i, id;
  int8_t ind, last_ind;
  int16_t encoder_position  = 0;
  uint8_t OK;
  TSPoint tsp;
  box_t t;
  uint16_t color;
  uint32_t zwloka;
  OK = 0;
  ind = 0;
  last_ind = 1;

  if (ENCODER == 0 && digitalRead(ENCODER_push) == 0)
  {
    ENCODER = 1;
    while (digitalRead(ENCODER_push) == 0) {

      myEnc.write(0 );
      encoder_position =  myEnc.read();

    }
  }

  if (ENCODER)
  {
    zwloka = millis();
    while (!OK)
    {
      int16_t newPos = myEnc.read();

      if (millis() - zwloka > 100) {
        
        if (newPos  > encoder_position + 1)  ind++;
        if (newPos  < encoder_position - 1)  ind--;


        if (ind >= TouchCount) ind = 0;
        if (ind < 0)  ind = TouchCount - 1;
        encoder_position = newPos;

        if (last_ind != ind) {
          t = Touch[last_ind];
          tft.drawRoundRect(t.x, t.y, t.w, t.h, BUTTON_ROUND, t.c);

          t = Touch[ind];
          tft.drawRoundRect(t.x, t.y, t.w, t.h, BUTTON_ROUND, ~t.c);

        }

        last_ind = ind;

        zwloka = millis();
      }
      if (digitalRead(ENCODER_push) == 0) {
        id = Touch[ind].id;
        OK = 1;
        ENCODER = 0;
        Play(P_CLICK);
      }

    }

  }
  else
  {

    while (!OK) {

      id = K_NONE;
      tsp = ReadTouch(wait_press);
      if ( tsp.x > 0 &&  tsp.y > 0)
      {
        for (i = 0; i < TouchCount ; i++)
        {
          t = Touch[i];
          if ((tsp.x >= t.x) && ( tsp.x <= (int)t.x + t.w) && ( tsp.y >= t.y) && ( tsp.y <= (int) t.y + t.h))
          {
            id = t.id;

            tft.drawRoundRect(t.x, t.y, t.w, t.h, BUTTON_ROUND, ~t.c);
            delay(200);
            tft.drawRoundRect(t.x, t.y, t.w, t.h, BUTTON_ROUND, t.c);
            OK = 1;
            Play(P_CLICK);
          }
        }
        if (wait_press == 1) OK = 1;
      }
      if (wait_press == 0) OK = 1;
    }
  }
  
  return id;
}



TSPoint ReadTouch(uint8_t wait)
{
  int x, y;
  uint8_t ok;
#define TS_MINX 240
#define TS_MAXX 945

#define TS_MINY 205
#define TS_MAXY 780

#define MINPRESSURE 5


  TSPoint tsp;

  


  ok = 0;
  tsp.x = 0;
  tsp.y = 0;
  while (!ok)
  {
    digitalWrite(13, HIGH);
    tsp = ts.getPoint();
    digitalWrite(13, LOW);

    // if sharing pins, you'll need to fix the directions of the touchscreen pins
    //    pinMode(XP, OUTPUT);
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    //pinMode(YM, OUTPUT);
    if (tsp.z > MINPRESSURE ) {
 
      tsp.x = map(tsp.x, TS_MINX, TS_MAXX, 0, tft.width());
      tsp.y = map(tsp.y, TS_MINY, TS_MAXY, tft.height(), 0);

      ok = 1;
    }
    if (wait == 0) ok = 1;
  }

  return tsp;
}


int InputNumber(const __FlashStringHelper * txt, int def)
{
  int x, y, i, id, v;

  String val;
  char buff[6];
  uint8_t ok, ind, refresh;


#define BW 80
#define BH 40

  TouchClear();
  tft.fillScreen(BLACK);
  tft.setTextSize (2);
  tft.setTextColor(WHITE);

  tft.setCursor(10, 30);
  tft.print(txt);

  strcpy(buff, "   ");
  DrawTextBox(10, 120 - 55,  BW, BH, F("  -"), K_MINUS);

  for (i = 0; i < 20; i++)
  {
    int x, y, r;
    uint16_t c;

    y = i / 5;
    x = i % 5;

    x = 10 + x * (BW + 15);
    y = 120 + y * (BH + 15);

    if (i <= 9)
    {
      buff[2] = char(48 + i);
      DrawTextBox(x, y, BW, BH, buff, 48 + i);
    }
    if (i == 10)DrawTextBox(x, y, BW, BH, F("CLEAR"), K_CLEAR);
    if (i == 13)      DrawTextBox(x, y, BW, BH, F(" DEL"), K_DEL);
    if (i == 14)      DrawTextBox(x, y, BW, BH, F("ENTER"), K_ENTER);


  }
  ok = 0;

  val = String(def);
  ind = val.length();
  tft.setTextSize (3);
  refresh = 1;
  while (!ok)
  {
    if (refresh) {
      DrawBox(10 + BW + 15, 120 - 55, 2 * (BW + 15) + BW, BH, BLUEd, BOX_INPUT_BG);
      tft.setTextColor(BLACK);
      tft.setCursor(20 + BW + 15, 120 - 53 + 5);
      tft.print(val);
      delay(100);
      WaitTouchFree();
      refresh = 0;
    }

    id = TouchId(1);

    if (id >= 32 ) {
      val += (char) id;
      refresh = 1;
    }
    else
    {
		
      if (id == K_MINUS)
      {
        v = val.toInt();
        val = String(v * -1);
        refresh = 1;
      }
	  
      if (id == K_CLEAR)
      {
        val = "";
        refresh = 1;
      }
	  
      if (id == K_DEL && val.length() > 0)
      {
        val.remove(val.length() - 1);
        refresh = 1;
      }
	  
      if (id == K_ENTER ) ok = 1;
    }
  }

  return val.toInt();
}

int InputText(const __FlashStringHelper * txt, char * def_txt)
{
  int x, y, i, id;
  uint8_t ind;
  char buff[20];
  uint8_t ok, refresh;


#define BW 42
#define BH 40
#define SPACE 5
  TouchClear();
  ind = 0;
  tft.fillScreen(BLACK);
  tft.setTextSize (2);
  tft.setTextColor(WHITE);

  tft.setCursor(15, 25);
  tft.print(txt);
  DrawBox(10 , 50, 4 * (BW + SPACE) + BW, BH, BLUEd, BOX_INPUT_BG);
  tft.setTextSize (3);
  tft.setTextColor(BOX_INPUT_TEXT);
  tft.setCursor(10 + 10, 58);
  tft.print(def_txt);
  tft.setTextSize (2);
  tft.setTextColor(WHITE);

  DrawTextBox(8 + 8 * (BW + SPACE), 50, BW * 2 + SPACE, BH, F("Cancel"), K_CANCEL);


  strcpy(buff, " ");
  for (i = 0; i <= 39; i++)
  {
    int x, y, r;
    uint16_t c;

    y = i / 10;
    x = i % 10;

    x = 8 + x * (BW + SPACE);
    y = 100 + y * (BH + SPACE);

    if (i <= 35)
    {

      if (i <= 9)      id = char(48 + i);
      else id = char(55 + i);
      buff[0] = id;
      DrawTextBox(x, y, BW, BH, buff, id);
    }

    if (i == 36)      DrawTextBox(x, y, BW * 2 + SPACE, BH, F("Delete"), K_DEL);
    if (i == 38)      DrawTextBox(x, y, BW * 2 + SPACE, BH, F("Enter"), K_ENTER);

  }
  ok = 0;

  memset(buff, 0, sizeof(buff));
  strcpy(buff, def_txt);

  ind = strlen(buff);
  refresh = 1;
  tft.setTextSize (3);
  tft.setTextColor(BOX_INPUT_TEXT);
  while (ok == 0)
  {
    id = TouchId(1);
    if (id >= 32 && id < 255 && ind < sizeof(buff) - 1) {
      buff[ind] = (char)  id;
      ind++;

    }
    else
    {
      if (id == K_DEL && ind > 0)
      {
        ind--;
        buff[ind] = 0;

        refresh = 1;
      }
      else if (id == K_ENTER ) ok = K_ENTER;
      else if (id == K_CANCEL ) ok = K_CANCEL;
    }
    if (refresh)
    {
      tft.fillRect(10 + 2 , 50 + 2, 4 * (BW + SPACE) + BW - 5, BH - 4, BOX_INPUT_BG);
      tft.setCursor(10 + 10, 58);
      tft.print(buff);

      WaitTouchFree();
    }
  }
  strcpy(inputString, buff);
  return ok;
}

void WaitTouchFree()
{
  TSPoint tsp;
  delay(100);
  tsp = ReadTouch(0);
  while (tsp.z > 5)
  {
    tsp = ReadTouch(0);
    delay(50);
  }

}
int DialogTN(const __FlashStringHelper * txt)
{
#define dialog_start_y 120

  int id;
  TouchClear();

  DrawBox(80, dialog_start_y, 330, 80, WHITE, BOX_BG );

  tft.setTextSize (2);

  tft.setCursor(110 + 1, dialog_start_y + 6);
  tft.setTextColor(BLACK);
  tft.print(txt);
  tft.setCursor(110, dialog_start_y + 5);
  tft.setTextColor(WHITE);
  tft.print(txt);

  DrawBox(135, dialog_start_y + 30, 90, 40, WHITE, BLUE, icon_yes, K_TAK);
  DrawBox(250, dialog_start_y + 30, 90, 40, WHITE, BLUE, icon_no, K_NIE);
  tft.setCursor(185, dialog_start_y + 43); tft.print(F("TAK"));
  tft.setCursor(300, dialog_start_y + 43); tft.print(F("NIE"));

  id = 0;

  while (id != K_TAK && id != K_NIE)
  {
    id = TouchId(1);
    if (id != K_NONE)    Serial.println(id);  
  }
  return id;
}

//Setup Screen
void EkranSETUP()
{

#define BW 180
#define BH 60
  int i, id;
  EKRAN = E_SETUP;
  TouchClear();
  tft.fillScreen(BLACK);
  //DrawHeader(0);
  DrawTitle(F("SETUP SCREEN"));
  //tft.drawRect(40,100,400,200, YELLOW);
  tft.setTextSize (2);
  tft.setTextColor(WHITE);


  DrawButtonText(1 * 24 + 1, BW, BH, K_SENSORS,    F("   SENSORS") );
  DrawButtonText(1 * 24 + 13,  BW, BH, K_SOUND, F("   SOUND") , BLUEd);

  DrawButtonText(5 * 24 + 1,  BW, BH, K_DATETIME, F(" DATE / TIME"));
  DrawButtonText(5 * 24 + 13,  BW, BH, K_ALARMS, F("    ALARMS"), BLUEd );

  DrawButtonText(9 * 24 + 1,  BW, BH, K_MODEM,   F("   MODEM") );
  DrawButtonText(9 * 24 + 13,  BW, BH, K_CANCEL, F("  MAIN SCREEN") );

}
void DrawButtonText(int nr, int w, int h, int button_id, const __FlashStringHelper * text , uint16_t color)
{
  int x, y, r, ty;
  uint16_t c;
  
  y = nr / 24;
  x = nr % 24;

  tft.setTextSize (2);
  tft.setTextColor(RED);
  
  x = 10 + x * (20);
  y = 45 + y * (20 );

  DrawButtonGfx(x, y, w, h, color);

  tft.setTextSize (2);
  tft.setTextColor(BLUEd);
  ty = y + ((h - 16) / 2);
  tft.setCursor(x + 7, ty + 2);
  tft.print(text);
  tft.setTextColor(WHITE);
  tft.setCursor(x + 5, ty);
  tft.print(text);
  
  TouchAdd(button_id);
}

void DrawButtonText(int nr, int w, int h, int button_id, const __FlashStringHelper * text )
{
  DrawButtonText(nr, w, h, button_id, text, BUTTON_C);
}

void DrawButtonText(int x, int y, int w, int h, int button_id, char *text )
{
  DrawButtonText( x, y, w, h,  button_id, text, BUTTON_C );
}

void DrawButtonText(int x, int y, int w, int h, int button_id, char *text , uint16_t color)
{

  int  r, ty;

  tft.setTextSize (2);
  tft.setTextColor(RED);


  DrawButtonGfx(x, y, w, h, color);

  tft.setTextSize (2);
  tft.setTextColor(BLUEd);
  ty = y + ((h - 16) / 2);
  tft.setCursor(x + 7, ty + 2);
  tft.print(text);
  tft.setTextColor(WHITE);
  tft.setCursor(x + 5, ty);
  tft.print(text);
  TouchAdd(button_id);

}

void DrawButtonText(int x, int y, int w, int h, int button_id, const __FlashStringHelper * text )
{
  DrawButtonText(x, y, w, h, button_id, text, BLUE);
}
void DrawButtonText(int x, int y, int w, int h, int button_id, const __FlashStringHelper * text , uint16_t color)
{

  int  r, ty;

  tft.setTextSize (2);
  tft.setTextColor(RED);


  DrawButtonGfx(x, y, w, h, color);

  tft.setTextSize (2);
  tft.setTextColor(BLUEd);
  ty = y + ((h - 16) / 2);
  tft.setCursor(x + 7, ty + 2);
  tft.print(text);
  tft.setTextColor(WHITE);
  tft.setCursor(x + 5, ty);
  tft.print(text);
  TouchAdd(button_id);

}


void DrawTitle(const __FlashStringHelper * text )
{
  TouchClear();
  tft.fillRect(0, 1, tft.width(), 34,  BLUE);
  tft.drawFastHLine(0, 0,  tft.width(), YELLOW);
  tft.drawFastHLine(0, 35,  tft.width(), YELLOW);

  tft.setTextSize (3);
  tft.setTextColor(BLACK);
  tft.setCursor(10 + 2, 7 + 2);
  tft.print(text);
  tft.setTextColor(WHITE);
  tft.setCursor(10, 7);
  tft.print(text);
}


void Print3D(int x, int y, const __FlashStringHelper * txt)
{
  Print3D(x, y, txt, WHITE);
}


void Print3D(int x, int y, const __FlashStringHelper * txt, uint16_t color)
{
  tft.setCursor(x + 2, y + 2);
  tft.setTextColor(BLACK);
  tft.print(txt);
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.print(txt);


}

void Print3D(int x, int y, char *txt)
{
  tft.setCursor(x + 2, y + 2);
  tft.setTextColor(BLACK);
  tft.print(txt);
  tft.setCursor(x, y);
  tft.setTextColor(WHITE);
  tft.print(txt);
}

void DrawSENSORS()
{
  int i, x, y, id;
  uint8_t ok;
  char buff[6];
  EKRAN = E_SENSORS;
  tft.fillScreen(BLACK);
  DrawTitle(F("SETUP SCREEN - SENSORS"));

  ok = 0;
  while (!ok) {
    TouchClear();
    DrawButtonText(410, 2, 60, 32, K_EXIT, F("EXIT"), RED);
    for (i = 0; i < 6; i++)
    {
      y = i / 8;
      x = i % 8;
      sprintf(buff, "%2d", i + 1);
      DrawButtonText(5 + (x * 60), 50, 50, 40, i + 1, buff) ;

    }
    DrawButtonText(5 + (6 * 60), 50, 100, 40, K_DEFAULT, "DEFAULT" ) ;

    id = TouchId(2);
    if (id == K_DEFAULT)  SensorsSetDefault();
    else if (id == K_EXIT) ok = 1;
    else
    {

      ScanSensor(id);
    }
  }

}

void DrawChkBox(int x, int y, int checked, int button_id)
{
  
  DrawButtonGfx( x,  y,  CHK_W,  CHK_H);

  if (checked)
    tft.drawBitmap(x + 6, y + 6, icon_chk24, 24, 24, WHITE);

  TouchAdd(button_id);
}


uint8_t TouchAction()
{
  int id;
  id = TouchId(0);

  switch (EKRAN)
  {
    case E_MAIN:
      if (id != K_NONE)    Serial.println(id);
      if (id == K_SETUP) {
        
        EkranSETUP();
      }
      if (id == K_PAGE)
      {
        PAGE++;
        if (PAGE > 1) PAGE = 0;
        DrawMAIN(R_FULL);
      }
      if (id > 0 && id <= MaxSensors)
      {
        DrawGraph1H(id - 1);
      }
      break;


    case E_SETUP:

      if (id == K_CANCEL) {
        DrawMAIN(R_FULL);
      }
      if (id == K_SENSORS)
      {
        DrawSENSORS();
        EkranSETUP();
      }
      if (id == K_SOUND) {
        //TestColors();
        //EkranSOUND();
      }
      if (id == K_MODEM) {
        EkranMODEM(R_FULL);
      }
      if (id == K_DATETIME) {
        EkranDATETIME();
        EkranSETUP();
      }
      break;

    case E_SENSORS:
      if (id == K_CANCEL) EkranSETUP();
      break;

    case E_SENSOR:
      if (id == K_CANCEL) DrawMAIN(R_FULL);

      if ( id == K_ICON)
      {
        SENSORS[SENSORS_ind].icon = SelectIcons(SENSORS[SENSORS_ind].icon);
        DrawSENSOR(R_FULL);
      }
      if (id == K_ACTIVE) {
        if ( SENSORS[SENSORS_ind].active) SENSORS[SENSORS_ind].active = 0;
        else SENSORS[SENSORS_ind].active = 1;
        DrawSENSOR(R_UPDATE);
        WaitTouchFree();
      }
      if (id == K_ID) {
        SENSORS[SENSORS_ind].id = InputNumber(F("Enter ID:"), SENSORS[SENSORS_ind].id);

        SetDefaultMaxMin(SENSORS_ind);
        DrawSENSOR(R_FULL);
        WaitTouchFree();
      }
      if (id == K_NAME) {

        InputText(F("Enter Name:"), SENSORS[SENSORS_ind].name);
        strcpy(SENSORS[SENSORS_ind].name, inputString);
        DrawSENSOR(R_FULL);
        WaitTouchFree();
      }

      if (id == K_ALARM) {

        if ( SENSORS[SENSORS_ind].options & _BV(O_ALARM)) SENSORS[SENSORS_ind].options &= ~_BV(O_ALARM);
        else SENSORS[SENSORS_ind].options |= _BV(O_ALARM);
        DrawSENSOR(R_UPDATE);
        WaitTouchFree();
      }
      if (id == K_RAPORT) {
        if ( SENSORS[SENSORS_ind].options & _BV(O_RAPORT)) SENSORS[SENSORS_ind].options &= ~_BV(O_RAPORT);
        else SENSORS[SENSORS_ind].options |= _BV(O_RAPORT);
        DrawSENSOR(R_UPDATE);
        WaitTouchFree();
      }

      if (id == K_MIN) {
        SENSORS[SENSORS_ind].min = InputNumber(F("Enter Min:"), SENSORS[SENSORS_ind].min);
        DrawSENSOR(R_FULL);
        WaitTouchFree();
      }
      if (id == K_MAX) {
        SENSORS[SENSORS_ind].max = InputNumber(F("Enter Max:"), SENSORS[SENSORS_ind].max);
        DrawSENSOR(R_FULL);
        WaitTouchFree();

      }
      if (id == K_SAVE)
      {
        SensorsSave();
        DrawGraph1H(SENSORS_ind);

      }
      break;

    case E_GRAPH:
      if (id == K_MENU) DrawMAIN(R_FULL);
      if (id == K_SETUP) DrawSENSOR(R_FULL);
      if (id == K_24H) DrawGraph24H(SENSORS_ind);
      if (id == K_1H) DrawGraph1H(SENSORS_ind);
      if (id == K_RANDOM) {
        SensorRandomData(SENSORS_ind);
        if (GraphGfxStepX >= 18) DrawGraph24H(SENSORS_ind);
        else DrawGraph1H(SENSORS_ind);
      }
      if (id == K_ON) {
        RF_SetSensor(SENSORS[SENSORS_ind].id, 1);
      }
      if (id == K_OFF) {
        RF_SetSensor(SENSORS[SENSORS_ind].id, 0);
      }
      break;

  }
}

void DrawSENSOR(uint8_t refresh)
{
  int x, xx, y;
  char   buff[11];
#define BH 32
#define YP 8
  EKRAN = E_SENSOR;
  if (refresh == R_FULL)
  {
    tft.fillScreen(BLACK);
    DrawTitle(F("SENSOR SETTINGS"));
  }
  else TouchClear();

  tft.setTextSize (2);
  x = 30;
  xx = 120;
  y = 60;


  tft.setCursor(x, y + YP);
  tft.print("Active:");
  
  DrawChkBox(xx, y , SENSORS[SENSORS_ind].active, K_ACTIVE);

  tft.setCursor(x + 195, y + YP);
  tft.print("Icon:");
  
  DrawButtonText(x + 250, y - 5 , 60, 45, K_ICON, "" );
  
  tft.drawBitmap(x + 259, y + 3, AdrIcon(SENSORS[SENSORS_ind].icon), 40, 32, BLACK);
  tft.drawBitmap(x + 257, y + 1, AdrIcon(SENSORS[SENSORS_ind].icon), 40, 32, WHITE);



  y += 50;
  tft.setCursor(x, y + YP);
  tft.print(F("ID:"));
  sprintf(buff, "%3d", SENSORS[SENSORS_ind].id);
  
  DrawButtonText(xx, y ,  60, BH, K_ID, buff) ;

  tft.setCursor(x + 195, y + YP);
  tft.print(F("Name:"));

  DrawButtonText(x + 250, y ,  100, BH, K_NAME, SENSORS[SENSORS_ind].name) ;

  y += 50;
  tft.setCursor(x, y + YP);
  tft.print(F("Type:"));
  tft.setCursor(xx, y + YP);
  tft.print(GetTypeName(SENSORS[SENSORS_ind].id));

  y += 50;
  tft.setCursor(x, y + YP);
  tft.print(F("Alarm:"));
  DrawChkBox(xx, y , SENSORS[SENSORS_ind].options & _BV(O_ALARM), K_ALARM);

  tft.setCursor(x + 200, y + YP);
  tft.print("Min:");
  sprintf(buff, "%2d", SENSORS[SENSORS_ind].min);
  DrawButtonText(x + 250, y , 50, BH, K_MIN, buff ) ;

  tft.setCursor(x + 320, y + YP);
  tft.print("Max:");
  sprintf(buff, "%2d", SENSORS[SENSORS_ind].max);
  DrawButtonText(x + 370, y , 50, BH, K_MAX, buff) ;

  y += 50;
  tft.setCursor(x, y + YP);
  tft.print(F("Raport:"));
  DrawChkBox(xx, y , SENSORS[SENSORS_ind].options & _BV(O_RAPORT), K_RAPORT);



  y += 50;
  DrawButtonText(x + 370, 2, 80, BH, K_SAVE, F(" SAVE"), RED);

}


void DrawGraph1H(uint8_t ind)
{
#define graph_x 40
#define graph_y 300

#define graph_w 440
#define graph_h 200

  int x, i, y, ly, g_min, g_max;

  EKRAN = E_GRAPH;

  SENSORS_ind = ind;
  tft.fillScreen(BLACK);
  TouchClear();
  DrawTitle(F("SENSOR GRAPH 1H"));
  g_min = SENSORS[SENSORS_ind ].min;
  g_max = SENSORS[SENSORS_ind ].max;

  GraphGfx(g_min, g_max, MaxSensorsData1H);

  DrawButtonText(300, 2, 80, 32, K_MENU, F(" MENU"), RED);
  DrawButtonText(390, 2, 80, 32, K_SETUP, F("SETUP"), RED);
  DrawButtonText(300, 40, 80, 32, K_24H, F(" 24H"), RED);
  DrawButtonText(390, 40, 80, 32, K_RANDOM, F("RANDOM"), RED);

  if (SENSORS[SENSORS_ind ].type == T_OUTS)
  {
    DrawButtonText(100, 40, 60, 32, K_ON,  F(" ON"), BLUE);
    DrawButtonText(170, 40, 60, 32, K_OFF, F("OFF"), BLUE);
  }


  tft.setCursor(10, 42);
  tft.print(SENSORS[SENSORS_ind].name);

  ly = GraphGfxPoint(SENSORSD[SENSORS_ind ].data1H[0], g_min, g_max);

  for (i = 1; i < MaxSensorsData1H; i++)
  {
    y = GraphGfxPoint(SENSORSD[SENSORS_ind ].data1H[i], g_min, g_max);
    x = graph_x + (i  * GraphGfxStepX);

    tft.drawLine(x - GraphGfxStepX + 2, ly, x, y, RED);
    tft.drawLine(x - GraphGfxStepX + 2, ly - 1, x, y - 1, RED);
    tft.fillCircle(x, y , 2, YELLOW);
    ly = y;
  }


}
void DrawGraph24H(uint8_t ind)
{
#define graph_x 40
#define graph_y 300

#define graph_w 440
#define graph_h 200

  int x, i, y, ly, g_min, g_max;

  EKRAN = E_GRAPH;

  SENSORS_ind = ind;
  tft.fillScreen(BLACK);
  TouchClear();
  DrawTitle(F("SENSOR GRAPH 24H"));
  g_min = SENSORS[SENSORS_ind ].min;
  g_max = SENSORS[SENSORS_ind ].max;

  GraphGfx(g_min, g_max, MaxSensorsData24H);

  DrawButtonText(300, 2, 80, 32, K_MENU, F(" MENU"), RED);
  DrawButtonText(390, 2, 80, 32, K_SETUP, F("SETUP"), RED);
  DrawButtonText(300, 40, 80, 32, K_24H, F(" 1H"), RED);
  DrawButtonText(390, 40, 80, 32, K_RANDOM, F("RANDOM"), RED);

  if (SENSORS[SENSORS_ind ].type == T_OUTS)
  {
    DrawButtonText(100, 40, 60, 32, K_ON,  F(" ON"), BLUE);
    DrawButtonText(170, 40, 60, 32, K_OFF, F("OFF"), BLUE);
  }

  tft.setCursor(10, 42);
  tft.print(SENSORS[SENSORS_ind].name);

  ly = GraphGfxPoint(SENSORSD[SENSORS_ind ].data24H[0], g_min, g_max);

  for (i = 1; i < MaxSensorsData24H; i++)
  {
    y = GraphGfxPoint(SENSORSD[SENSORS_ind ].data24H[i], g_min, g_max);
    x = graph_x + (i  * GraphGfxStepX);

    tft.drawLine(x - GraphGfxStepX + 2, ly, x, y, RED);
    tft.drawLine(x - GraphGfxStepX + 2, ly - 1, x, y - 1, RED);
    tft.fillCircle(x, y , 2, YELLOW);
    ly = y;
  }


}

/*Testing LCD OK ? */
void TestColors()
{
#define BW 58
#define BH 58
  int x, y, r, g, b;
  uint16_t id;
  uint16_t c, cc;
  tft.fillScreen(BLACK);
  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  TouchClear();
  r = 0; g = 0; b = 0;
  for (y = 0; y < 4; y++)
    for (x = 0; x < 8; x++)
    {
      r = 0;
      g = 0;
      b = ((y * 8) + x) * 4;

      c = tft.color565(r, g, b);
      DrawBox(x * (BW + 2), y * (BH + 2), BW, BH, WHITE, c, 0, c);
    }

  while (1)
  {
    id = (uint16_t) TouchId(1);
    if (id != 0)
    {
      tft.fillRect(0, 290, 200, 30, GRAY);
      tft.setCursor(0, 292);
      tft.print(id, HEX);
      delay(200);
    }
  }
}


/*     MAIN LOOP     */

void Logo()
{
  int id;
  tft.fillScreen(BLUE);
  tft.setTextSize(4);
  TouchClear();

  Print3D(150, 20, F("LCD 3.95"), WHITE);


  Print3D(100, 60, F("ARDUINO MEGA"), WHITE);

  Print3D(150, 120, F("AutoCamp"), RED);

  Print3D(120, 180, F("Build Menu"), YELLOW);

  DrawButtonText(200, 230, 70, 35, K_MENU, F("START"), RED);

  id = 0;
  while (id != K_MENU) id = TouchId(1);

}


int GraphGfx(int g_min, int g_max, byte max_time)
{
  int x, y, p, id;
  float f, step ;
  uint8_t ok;
  GraphGfxStepX = (int)(graph_w / max_time);

  if (g_min > g_max) g_max = g_min;

  tft.setTextSize (2);
  tft.setTextColor(WHITE);

  tft.drawFastVLine(graph_x, graph_y - graph_h, graph_h, YELLOW);
  tft.drawFastHLine(graph_x, graph_y, graph_w, YELLOW);

  step = (g_max - g_min); // przez 10 juz jest
  if (step == 0) step = 1;

  for (f = g_min * 10; f <= g_max * 10; f += step)
  {
    
    p = GraphGfxPoint ((int)f, g_min, g_max);
    tft.drawFastHLine(graph_x - 2, p, 5, WHITE);
    if (f >= 100)    tft.setCursor(1, p - 5);
    else tft.setCursor(10, p - 5);

    tft.print((int)f / 10);

  }

  tft.setTextSize (1);
  int h = 0;

  for (x = graph_x; x <= graph_x + graph_w; x += GraphGfxStepX)
  {

    tft.drawFastVLine(x, graph_y - 2, 5, YELLOW);

    if (h > 0)
    {
      if (h > 9)    tft.setCursor(x - 5, graph_y + 3);
      else tft.setCursor(x - 2, graph_y + 3);

      tft.print(h);
    }
    if (max_time == MaxSensorsData24H)    h++;
    else h += 5;
  }

}

int GraphGfxPoint(int p, int g_min, int g_max )
{
  int y;

  if (g_min > g_max) g_max = g_min;
  y = map(p,  g_min * 10, g_max * 10, 0, graph_h);
  if (y > graph_h) y = graph_h;
  if (y < 0) y = 0;
  return graph_y - y;
}


uint32_t SelectIcons(uint32_t nr) {
  int i, x, y;
  uint16_t color;
#define BW 72
#define BH 54
#define SX 30
#define SY 50


  tft.fillScreen(BLACK);
  DrawTitle(F("SELECT ICONS"));


  tft.setTextSize (2);


  tft.setCursor(x, y + YP);
  tft.print("Active:");

  for (i = 0; i < ICONS40_count; i++)
  {
    y = i / 6;
    x = i % 6;

    DrawButtonText(SX + x * BW, SY + y * BH  , 60, 44, i + 1 , "" );

    if (nr == i) color = RED;
    else color = WHITE;
    tft.drawBitmap3D(SX + x * BW + 8, SY + y * BH + 6, ICONS40[i], 40, 32, color);

  }

  return TouchId(2) - 1;
}
// For testing graphic / demo
void SensorRandomData(int ind) {
  int i, d, p, g_min, g_max;
  g_min = SENSORS[ind].min * 10;
  g_max = SENSORS[ind].max * 10;
  p = g_min + ((g_max - g_min ) / 2);
  for (i = 0; i < MaxSensorsData24H; i++) {
    randomSeed(analogRead(10 + i));
    p = p + random(-20, 20);
    if (p < g_min) p = g_min;
    if (p > g_max) p = g_max;
    //p=g_min;
    SENSORSD[ind].data24H[i] = p;
  }
  for (i = 0; i < MaxSensorsData1H; i++) {
    randomSeed(analogRead(10 + i));
    p = p + random(-20, 20);
    if (p < g_min) p = g_min;
    if (p > g_max) p = g_max;
    //p=g_min;
    SENSORSD[ind].data1H[i] = p;
  }
}

type_module_t DecodeTypeModule(int type) {
  int t;
  type_module.module = type / 100;
  t = type % 100;
  type_module.type = t / 10;
  type_module.nr = t % 10;

  return type_module;
}

char * GetTypeName(uint8_t type) {
  char ret[10];
  if (type > 99) {
    DecodeTypeModule(type);
    type = type_module.type;
  }
  strcpy(ret, type_name[type]) ;
  return (char *) ret;
}

void SetDefaultMaxMin(uint8_t ind) {
  int v_min, v_max;
  v_min = 0;
  v_max = 100;
  DecodeTypeModule(SENSORS[ind].id);
  SENSORS[ind].type = type_module.type;
  switch (type_module.type) {
    case T_VOLT:
      v_min = 0; v_max = 16;
      break;
    case T_TEMP:
      v_min = -20; v_max = 50;
      break;
    case T_PROC:
      v_min = 0; v_max = 100;
      break;
    case T_ANALOG:
      v_min = 0; v_max = 1023;
      break;
  }
  SENSORS[ind].min = v_min;
  SENSORS[ind].max = v_max;
  SensorsSave();
}

//scanning defined on lcd module by id
void ScanSensor(uint8_t id_module) {
  int id, x, y, t, i, ind;
  id_module = id_module * 100;
  tft.setTextSize (1);
  tft.fillRoundRect(10, 100, 460, 210, 5, BLACK);
  tft.drawRoundRect(10, 100, 460, 210, 5, YELLOW);
  for (t = 0; t < 5; t++) {

    tft.setCursor(20 , 105 + t * 30 );
    tft.print(GetTypeName(t));

    for (i = 0; i < 5; i++)
    {
      ind = t * 5 + i;
      x = ind % 5;
      y = ind / 5;
      tft.setCursor(20 + x * 100, 105 + 10 + y * 30 );
      id = id_module + (t * 10) + i;
      tft.print(id);
      tft.print("-");
      RF_GetSensor(id );

      if (dataReceived.id == id) tft.print(dataReceived.val);
      else     {
        tft.print("off");
        STxtIntLn(F("Wrong replay id "), dataReceived.id);
      }

    }
  }
}

void TFT_Init() {
  tft.reset();

  tft.begin(0x9341); // SDFP5408

  tft.setRotation(3); // Need for the Mega, please changed for your choice or rotation initial
}
void RF_Init() {
  Radio.begin();  // Start up the physical nRF24L01 Radio
  //Radio.setChannel(108);  // Above most Wifi Channels
  Radio.setChannel(CHANNEL);  // Above most Wifi Channels
  Radio.setDataRate(RF24_250KBPS); // Fast enough.. Better range
  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  //Radio.setPALevel(RF24_PA_MIN);
  //Radio.setPALevel(RF24_PA_LOW);
  Radio.setPALevel(RF24_PA_MAX);  // Uncomment for more power

  Radio.enableDynamicPayloads();
  Radio.setRetries(10, 15);
  Radio.setCRCLength(RF24_CRC_16);

  //Radio.openReadingPipe(1, addresses[0]); // Use the first entry in array 'addresses' (Only 1 right now)

  Radio.openWritingPipe(addresses[0]);
  Radio.openReadingPipe(1, addresses[1]);
}

//Read RF24 Data
uint8_t  RF_Read()
{
  uint8_t ok;
  int timeout;
  timeout = 0;
  ok = 0;
  Radio.startListening();
  while (!Radio.available() && timeout < 51) {
    delay(1);
    timeout++;
    ;
  }
  if (timeout > 48) return 0;
  dataReceived.id = -1;
  dataReceived.val = 0;
  delay(1);

  while (Radio.available())  // While there is data ready
  {
    Radio.read( &dataReceived, sizeof(dataReceived) ); // Get the data payload (You must have defined that already!)
    ok = 1;
  }
  Radio.stopListening();
  if (dataReceived.id == 0) ok = 0;
  return ok;
}



//Tools for serial debug
void SBool(int war, const __FlashStringHelper * tak, const __FlashStringHelper * nie)
{
  if (war) Serial.print(tak);
  else Serial.print(nie);
}

void STxtInt( const __FlashStringHelper * txt, int i) {
  Serial.print(txt);
  Serial.print(i);
}

void STxtIntLn( const __FlashStringHelper * txt, int i) {
  Serial.print(txt);
  Serial.println(i);
}


int FindSensorId(int id)
{
  int i, ret;
  ret = -1;
  for (i = 0; i < MaxSensors; i++)
  {
    if (SENSORS[i].id == id) ret = i;
  }
  return ret;
}

void ReadSensorsAll() {
  int i, m, h;
  DateTime now = rtc.now();
  m = now.minute() / 5;
  h = now.hour();

  memset(MODULES, 0, sizeof(MODULES));
  for (i = 0; i < MaxSensors; i++)
  {
    if (SENSORS[i].active)
    {
      if (      RF_GetSensor(SENSORS[i].id)) {
        SENSORSD[i].data24H[h] = SENSORS[i].val;
        SENSORSD[i].data1H[m] = SENSORS[i].val;
      }
    }

  }


}
uint8_t RF_GetSensor(int id)
{
  int ind;
  uint8_t ok;
  ok = 0;
  if (id <= 0) return 0;
  Serial.print(F("RF_GetSensor ")); Serial.println(id);
  Radio.powerUp();
  Radio.stopListening();

  dataSend.cmd = CMD_GET;
  dataSend.id = id;
  dataSend.val = 0;

  if (Radio.write( &dataSend, sizeof(dataSend) ))
    if ( RF_Read())
    {
      if (dataReceived.id == id)
        ind = FindSensorId(id);
      if (ind >= 0) {
        SENSORS[ind].val = dataReceived.val;
        if (SENSORS[ind].val < SENSORS[ind].min * 10) SENSORS[ind].val = SENSORS[ind].min * 10;
        if (SENSORS[ind].val > SENSORS[ind].max * 10) SENSORS[ind].val = SENSORS[ind].max * 10;

        SENSORS[ind].active++;
        MODULES[0]++;
        ok = 1;

        STxtIntLn(F(" value "), SENSORS[ind].val);
      }
    }
  Radio.startListening();
  Radio.powerDown();
  return ok;
}
uint8_t RF_SetSensor(int id, int val)
{
  int ind;
  Serial.print(F("RF_GetSensor ")); Serial.println(id);
  Radio.powerUp();
  Radio.stopListening();

  dataSend.cmd = CMD_SET;
  dataSend.id = id;
  dataSend.val = val;


  if (Radio.write( &dataSend, sizeof(dataSend) ))
    if ( RF_Read())
    {
      if (dataReceived.id == id)
        ind = FindSensorId(id);
      if (ind >= 0) {
        SENSORS[ind].val = dataReceived.val;
        if (SENSORS[ind].val < SENSORS[ind].min * 10) SENSORS[ind].val = SENSORS[ind].min * 10;
        if (SENSORS[ind].val > SENSORS[ind].max * 10) SENSORS[ind].val = SENSORS[ind].max * 10;

        SENSORS[ind].active++;

        MODULES[0]++;

        STxtIntLn(F(" value "), dataReceived.val);
      }
    }
  Radio.startListening();
  Radio.powerDown();
}

void DrawButtonSensor(byte nr, byte refresh, byte page)
{
  int ns;
  ns = (page * 8) + nr;

  DrawButtonMENU( nr , SENSORS[ns].icon, ICON_ACTIVE , SENSORS[ns].val, SENSORS[ns].type, ns + 1  , SENSORS[ns].active, refresh);

}
void RTC_Init()
{
  Wire.begin();
  rtc.begin();
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
  //  rtc.adjust(DateTime("02.02.2017","23:20:00"));
  //rtc.adjust(DateTime(__DATE__, __TIME__));
}


//TEST MODEM GSM on Serial1
void TestModem()
{
  String txt = "                         ";
  Serial.println("MODEM TEST");
  Serial1.println("AT+cREG?");
  delay(500);
  while (Serial1.available())
  {
    txt += char(Serial1.read());
  }
  Serial.print(txt);
  Serial.println("----------");
}

#ifdef ENABLE_AUDIO
void mp3_sendCommand(int8_t command, int16_t dat)
{
  delay(20);
  mp3_send_buf[0] = 0x7e;   //
  mp3_send_buf[1] = 0xff;   //
  mp3_send_buf[2] = 0x06;   // Len
  mp3_send_buf[3] = command;//
  mp3_send_buf[4] = 0x01;   // 0x00 NO, 0x01 feedback
  mp3_send_buf[5] = (int8_t)(dat >> 8);  //datah
  mp3_send_buf[6] = (int8_t)(dat);       //datal
  mp3_send_buf[7] = 0xef;   //
  // Serial.print("Sending: ");
  for (uint8_t i = 0; i < 8; i++)
  {
    mp3.write(mp3_send_buf[i]) ;
    // Serial.print(sbyte2hex(mp3_send_buf[i]));
  }
  //Serial.println();
}

String sbyte2hex(uint8_t b)
{
  String shex;

  shex = "0X";

  if (b < 16) shex += "0";
  shex += String(b, HEX);
  shex += " ";
  return shex;
}
#endif
void Play(uint16_t id)
{
#ifdef ENABLE_AUDIO
  mp3_sendCommand(CMD_PLAY_FOLDER_FILE, id);
#endif
}

//Screen Modem GSM
void EkranMODEM(int _refresh)
{

#define BW 75
#define BH 40

  int i, id;
  uint8_t line;
  boolean ok, refresh;

  EKRAN = E_MODEM;
  digitalWrite(BOOT_MODEM, 1);
  TouchClear();
  tft.fillScreen(BLACK);
  
  DrawTitle(F("SETUP MODEM"));

  tft.setTextSize (2);
  tft.setTextColor(WHITE);

  DrawButtonText(410, 2, 60, 32, K_EXIT, F("EXIT"), RED);
  DrawButtonText(1 * 24 + 0,  BW, BH, K_SMS, F("Rapor.") );
  DrawButtonText(1 * 24 + 4,  BW, BH, K_VERSION, F("Vers.") );
  DrawButtonText(1 * 24 + 8, BW, BH, K_TEST,    F("Test") );
  DrawButtonText(1 * 24 + 12,  BW, BH, K_RESET, F("Reset") );
  DrawButtonText(1 * 24 + 16,  BW, BH, K_NET, F("Netwo.") );

  tft.drawRoundRect(40, 130, 400, 170, 5, WHITE);
  tft.fillRoundRect(41, 131, 398, 168, 5, BLUEd);
  if (_refresh == R_UPDATE) return;

  refresh = 1;

  tft.setTextSize(1);
  line = 0;
  ok = false;
  GSMTest(F("ATZ"));//reset profile
  delay(100);
  //GSMTest(F("AT+CREG=1"));
  //GSMTest(F("AT+CSPN?"));
  //GSMTest(F("AT+COPN"));//Display a full list of network operator names


  while (!ok)
  {

    line = GSMShowResult(line);

    id = TouchId(0);
    if (id == K_EXIT) {
      ok = true;
      EkranSETUP();
    }
    if (id == K_TEST)
    {
      tft.fillRoundRect(41, 131, 398, 168, 5, BLUEd);

      line = 0;
      //GSMTest(F("AT+CREG=2")); line = GSMShowResult(line);
      GSMTest(F("AT+CREG?")); line = GSMShowResult(line);
      GSMTest(F("AT+COPS?")); line = GSMShowResult(line);
      GSMTest(F("AT+CFUN?")); line = GSMShowResult(line);
      GSMTest(F("AT+CEER?")); line = GSMShowResult(line);





    }
    if (id == K_VERSION)
    {
      tft.fillRoundRect(41, 131, 398, 168, 5, BLUEd);
      line = 1;
      GSMTest(F("ATI"));     line = GSMShowResult(line);
      GSMTest(F("AT+CSPN?")); line = GSMShowResult(line);
      GSMTest(F("AT+COPS=0,0,\"Plus\"")); line = GSMShowResult(line);

    }
    if (id == K_SMS)
    {
      tft.fillRoundRect(41, 131, 398, 168, 5, BLUEd);

      line = 0;
      GSM_Report(Settings.mobile1);
    }
    if (id == K_RESET)
    {
      line = 0;
      tft.fillRoundRect(41, 131, 398, 168, 5, BLUEd);
      digitalWrite(BOOT_MODEM, 0);
      delay(1000);
      digitalWrite(BOOT_MODEM, 1);
      line = GSMShowResult(line);
      GSMTest(F("AT+COPS=0,0,\"Plus\""));
    }
    if (id == K_NET) {
      InputText(F("Enter Name:"), Settings.gsm_network);
      strcpy(Settings.gsm_network, inputString);
      SettingsSave();
      GSM_Connect();
      EkranMODEM(R_UPDATE);
    }
  }
}


byte GSMTest(const __FlashStringHelper *text )
{

  GSMResultCount = 0;
  Serial1.print(text);
  Serial1.print("\r");
  return ReadGSMResult(200);
}

byte Modem(char  *text , uint16_t delay)
{
  GSMResultCount = 0;
  Serial1.print(text);
  Serial1.print("\r");
  if (delay > 0)  ReadGSMResult(delay);
}



byte Modem(const __FlashStringHelper *text , uint16_t delay)
{
  GSMResultCount = 0;
  Serial1.print(text);
  Serial1.print("\r");
  if (delay > 0)  ReadGSMResult(delay);
}

byte GSMShowResult(byte line)
{
  byte i;
  for (i = 0; i < GSMResultCount; i++) {
    if (line > 13) tft.setCursor(200 , 135 + ((line - 14) * 12));
    else tft.setCursor(45 , 135 + ((line) * 12));

    if (i == 0)tft.setTextColor(YELLOW);
    if (i == 1)tft.setTextColor(WHITE);

    tft.print(GSMResult[i]);
    line++;

  }
  GSMResultCount = 0;
  return line;
}
byte  ReadGSMResult(uint16_t _delay)
{
  char c;
  int i;
  String txt = "                                        ";


  for (i = 0; i < 10; i++) {
    GSMResult[i].reserve(2);
    GSMResult[i] = "";

  }


  GSMResultCount = 0;
  txt = "";
  delay(_delay);
  while (Serial1.available())  {

    c = (char) Serial1.read();
    if (GSMResultCount < 9) {
      if (c == '\n')
      {
        if (txt.length() > 1 ) {
          GSMResult[GSMResultCount].reserve(txt.length() + 2);
          GSMResult[GSMResultCount] = txt;
          GSMResultCount++;
          Serial.println(txt);
        }
        txt = "";


      }
      else if (c >= ' ' && txt.length() < 199) txt += c;
    }
    delay(2);
  }
  return GSMResultCount;
}
//Screen Sound not finished
void EkranSOUND()
{
#define BW 100
#define BH 40

  int i, id;
  uint8_t line;
  boolean ok, refresh;
  char c;
  String txt = "                                                     ";
  EKRAN = E_SOUND;
  TouchClear();
  tft.fillScreen(BLACK);
  DrawTitle(F("SETUP SOUND"));
  
  tft.setTextSize (2);
  tft.setTextColor(WHITE);

  DrawButtonText(410, 2, 60, 32, K_EXIT, F("EXIT"), RED);
  
}

void SMS(char*  numer)
{
  char txt[21];
  
  Modem(F("AT+CMGF=1"), 200);
  Modem(F("AT+CSCS=\"GSM\""), 200);
  sprintf(txt, "AT+CMGS=\"+48%s\"", numer);
  Modem(txt, 300);

  Serial1.print(sms_message);
  Serial1.print( char(26));
  ReadGSMResult(1000);
}

//Set default sensors config 
void SensorsSetDefault()
{
  uint8_t i, j, delta;
  Serial.println("default id:");

  for (i = 0; i < MaxSensors; i++) {
    SENSORS[i].id = 100 + ((i / 3) * 10) + i % 3;
    Serial.println(  SENSORS[i].id);
    SENSORS[i].active = 0;
    SetDefaultMaxMin(i);
    SensorRandomData(i);
    SENSORS[i].val = SENSORS[i].min;
    SENSORS[i].icon = 0;
    SENSORS[i].options = 0;
    if ((SENSORS[i].id  >= 100) && (SENSORS[i].id  <= 101)) sprintf(SENSORS[i].name, "AKU%d", i + 1);
    else    sprintf(SENSORS[i].name, "SENS%d", i + 1);
  }

  SetSensor(0, 100, 1);
  SetSensor(1, 100, 2);
  SetSensor(2, 101, 3);
  SetSensor(4, 111, 7);
  SetSensor(5, 112, 8);
  
  SensorsSave();
}

void GSM_Connect()
{
  char txt[31];
  gsm.cmd = G_CONNECT;
  sprintf(txt, "AT+COPS=0,0,\"%s\"", Settings.gsm_network);

  Modem(txt, 0); 
}

//Send Gsm Report ( O_RAPORT flag in sensors options)
void GSM_Report(char * numer)
{
  int i;
  char buff[11];

  sms_message.reserve(300);
  sms_message = "AutoCamp\n";


  for (i = 0; i < MaxSensors; i++) {
    if ( SENSORS[i].options & _BV(O_RAPORT) )
    {

      sms_message += SENSORS[i].name;
      sms_message += ": ";
      dtostrf((float)SENSORS[i].val / 10, 3, 1, buff);
      sms_message += buff;
      sms_message += "\n";
    }

  }
  Serial.print(sms_message);
  SMS(numer);
  sms_message.reserve(10);
}

void ShowDate(const char* txt, const DateTime& dt) {
  Serial.print(txt);
  Serial.print(' ');
  Serial.print(dt.year(), DEC);
  Serial.print('/');
  Serial.print(dt.month(), DEC);
  Serial.print('/');
  Serial.print(dt.day(), DEC);
  Serial.print(' ');
  Serial.print(dt.hour(), DEC);
  Serial.print(':');
  Serial.print(dt.minute(), DEC);
  Serial.print(':');
  Serial.print(dt.second(), DEC);

  Serial.print(" = ");
  Serial.print(dt.unixtime());
  Serial.print("s / ");
  Serial.print(dt.unixtime() / 86400L);
  Serial.print("d since 1970");

  Serial.println();
}

void EkranDATETIME()
{
#define BW 60
#define BH 40
  int i, id;
  int x, y;
  EKRAN = E_DATETIME;
  TouchClear();
  tft.fillScreen(BLACK);

  DrawTitle(F("DATE / TIME"));
  DrawButtonText(410, 2, 60, 32, K_EXIT, F("EXIT"), RED);

  tft.setTextSize (2);
  tft.setTextColor(WHITE);

  x = -40; y = 60;
  DrawButtonText(x += BW + 10, y, BW, BH, K_DAY_UP,    F("Day") );
  DrawButtonText(x += BW + 10, y, BW, BH, K_MONTH_UP,    F("Mont") );
  DrawButtonText(x += BW + 10, y, BW, BH, K_YEAR_UP,    F("Year") );

  x += 20;
  DrawButtonText(x += BW + 10, y, BW, BH, K_HOUR_UP,    F("Hour") );
  DrawButtonText(x += BW + 10, y, BW, BH, K_MINUTE_UP,    F("Min.") );
  
  x = -40; y = 160;
  DrawButtonText(x += BW + 10, y, BW, BH, K_DAY_DO,    F("Day") );
  DrawButtonText(x += BW + 10, y, BW, BH, K_MONTH_DO,    F("Mont") );
  DrawButtonText(x += BW + 10, y, BW, BH, K_YEAR_DO,    F("Year") );

  x += 20;
  DrawButtonText(x += BW + 10, y, BW, BH, K_HOUR_DO,    F("Hour") );
  DrawButtonText(x += BW + 10, y, BW, BH, K_MINUTE_DO,    F("Min.") );

  x = 30; y = 230;
  tft.setCursor (x , y + 12);    tft.print("Days:");
  DrawButtonText(x += BW + 10, y, BW, BH, K_DOWN,    F(" -") );
  x += BW + 10 + 20;
  DrawButtonText(x += BW + 10, y, BW, BH, K_UP,    F(" +") );
  DrawButtonText(x += BW + 10, y, BW, BH, K_NOW,    F("Now") );

  DrawButtonText(x += BW + 10, 60, 70, 3 * (BH) + 20, K_SAVE, F(" SAVE"), RED);

  uint8_t  ok = 0;
  char tmp[20];

  uint8_t h, m, s, D, M;
  uint8_t h2 = -1, m2 = -1, s2 = -1, D2 = -1, M2 = -1;
  uint16_t Y, Y2 = -1;
  long int dh, dh2 = -999;
  DateTime now = rtc.now();
  D = now.day();
  M = now.month();
  Y = now.year();
  h = now.hour();
  m = now.minute();


  dh = ( long int) ( now.unixtime() - Settings.StartTime.unixtime()) / UNIX_DAY  ;

  s = 00; //now.second();
  while (!ok) {
    if (1)
    {

      x = -40; y = 110;

      if (D2 != D) {
        sprintf(tmp, " %02d", D);
        DrawButtonText(x += BW + 10, y, BW, BH, 0, tmp  , BLUEl);
      } else x += BW + 10;
      if (M2 != M) {
        sprintf(tmp, " %02d", M);
        DrawButtonText(x += BW + 10, y, BW, BH, 0,   tmp, BLUEl);
      } else x += BW + 10;
      if (Y2 != Y) {
        sprintf(tmp, "%02d", Y);
        DrawButtonText(x += BW + 10, y, BW, BH, 0,   tmp, BLUEl);
      } else x += BW + 10;
      x += 20;
      if (h2 != h) {
        sprintf(tmp, " %02d", h);
        DrawButtonText(x += BW + 10, y, BW, BH, 0,    tmp, BLUEl );
      } else x += BW + 10;
      if (m2 != m) {
        sprintf(tmp, " %02d", m);
        DrawButtonText(x += BW + 10, y, BW, BH, 0,    tmp, BLUEl );
      } else x += BW + 10;

      x = 0; y = 230;
      if (dh2 != dh) {
        sprintf(tmp, " %02d", dh);
        DrawButtonText(180, y, BW, BH, 0, tmp  , BLUEl);
      } else x += BW + 10;

      D2 = D;
      M2 = M;
      Y2 = Y;
      h2 = h;
      m2 = m;
      dh2 = dh;

    }

    id = TouchId(0);
    if (id == K_EXIT) ok = 1;
    if (id == K_SAVE) {
      s = 0;
      rtc.adjust(DateTime(Y, M, D, h, m, s));
      now = rtc.now();
      DateTime start(now.unixtime() - (dh * UNIX_DAY));
      start.sethour(0);
      start.setminute(0);
      start.setsecond(0);
      Settings.StartTime = start;
      SettingsSave();
      ok = 1;
    }
    if (id == K_DAY_UP )if (D < 31) D++; else D = 1;
    if (id == K_DAY_DO) if ( D > 1) D--; else D = 31;
    if (id == K_MONTH_UP ) if (M < 12) M++; else M = 1;
    if (id == K_MONTH_DO ) if (M > 0) M--; else M = 59;
    if (id == K_YEAR_UP) {
      Y++;
    }
    if (id == K_YEAR_DO) {
      if (Y > 2018) Y--;
      else Y = 2017;
    }
    if (id == K_HOUR_UP ) {
      if (h < 22) h++;
      else h = 0;
    }
    if (id == K_HOUR_DO ) {
      if (h > 0) h--;
      else h = 23;
    }
    if (id == K_MINUTE_UP) {
      if (m < 59) m++;
      else m = 0;
    }
    if (id == K_MINUTE_DO ) {
      if (m > 0) m--;
      else m = 59;
    }
    if (id == K_UP) dh++;
    if (id == K_DOWN) dh--;
    if (id == K_NOW) dh = 1;
    if (id == K_UP || id == K_DOWN || id == K_NOW)
    {
      DateTime start(now.unixtime() - (dh * UNIX_DAY));
      start.sethour(0);
      start.setminute(0);
      start.setsecond(0);
      Settings.StartTime = start;
      SettingsSave();
    }
  }
}


void SetSensor(int nr, int id, int icon)
{
  SENSORS[nr].id = id;
  SENSORS[nr].active = 2;
  SENSORS[nr].val = 0;
  SENSORS[nr].icon = icon;

}

