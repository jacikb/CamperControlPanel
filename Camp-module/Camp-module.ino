/*
  2 433MHz
  3 PWM
  4 TERMOMETRY DS18B20
  5 PWM
  6 PWM
  7 CM TRIGER
  8 CM ECHO

  9-13 NRF24L01

  - CONNECTIONS: nRF24L01 Modules See:
  http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
  Uses the RF24 Library by TMRH2o here:
  https://github.com/TMRh20/RF24
   1 - GND
   2 - VCC 3.3V !!! NOT 5V
   3 - CE to Arduino pin 7
   4 - CSN to Arduino pin 8
   5 - SCK to Arduino pin 13
   6 - MOSI to Arduino pin 11
   7 - MISO to Arduino pin 12
   8 - UNUSED

   V1.02 02/06/2016
   Questions: terry@yourduino.com */

/*-----( Import needed libraries )-----*/
#include <SPI.h>   // Comes with Arduino IDE
#include <EEPROM.h>
#include <avr/pgmspace.h>
#include<stdlib.h>
#include "Arduino.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//#define ONE_WIRE_BUS 4
#define ONE_WIRE_BUS 7
#define TEMPERATURE_PRECISION 10

#include "data.h"
#include "rf_data.h"
#include "RF24.h"  // Download and Install (See above)
//#include "printf.h" // Needed for "printDetails" Takes up some memory
/*-----( Declare Constants and Pin Numbers )-----*/
//None yet
/*-----( Declare objects )-----*/
// (Create an instance of a radio, specifying the CE and CS pins. )
#if defined(__AVR_ATmega2560__)
#define  CE_PIN  44   // The pins to be used for CE and SN
#define  CSN_PIN 45
unsigned char PCB[] = "MEGA";
#else
//stara testowa
//#define  CE_PIN  9   // The pins to be used for CE and SN
//#define  CSN_PIN 10
//nowe margol
#define  CE_PIN  10   // The pins to be used for CE and SN
#define  CSN_PIN 9
unsigned char PCB[] = "UNO";
#endif

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

RF24 Radio(CE_PIN, CSN_PIN);
/*-----( Declare Variables )-----*/
//byte addresses[][6] = {"1Node"}; // Create address for 1 pipe.
byte addresses[][6] = {"1Node", "2Node"};

s_rf_data  dataSend, dataReceived;
s_settings Settings;
#define CHANNEL 118

#define LENGTH_trigPin 2
#define LENGTH_echoPin 8




#define PIN_LED1 8
#define PIN_LED2 6
#define PIN_LED3 4
#define L1_1   digitalWrite(PIN_LED1,1)
#define L1_0   digitalWrite(PIN_LED1,0)
#define L2_1   digitalWrite(PIN_LED2,1)
#define L2_0   digitalWrite(PIN_LED2,0)
#define L3_1   digitalWrite(PIN_LED3,1)
#define L3_0   digitalWrite(PIN_LED3,0)

struct type_module_t {
  uint8_t module;
  uint8_t type;
  uint8_t nr;
};
type_module_t type_module, SELECTED;
type_module_t DecodeTypeModule(int type) {
  int t;
  type_module.module = type / 100;
  t = type % 100;
  type_module.type = t / 10;
  type_module.nr = t % 10;

  return type_module;
}
String inputString = "";
uint8_t EKRAN = 0;
char ret[21];
uint16_t refresh_sensors;


void setup()   /****** SETUP: RUNS ONCE ******/
{


  //CMD_SensorsDefault();
  // Use the serial Monitor (Symbol on far right). Set speed to 115200 (Bottom Right)
  Serial.begin(115200);
  inputString.reserve(200);

  Serial.println(F("      "));
  Serial.print(F("Module Server "));
  Serial.println((char *) PCB);
  DS18B20.begin();
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_LED3, OUTPUT);
  DS18B20.requestTemperatures();
  Load();
  Serial.print("MODULE ID ");
  Serial.println(Settings.id_module);
  if (!Radio.begin()) Serial.println("Radio FAIL"); // Start up the physical nRF24L01 Radio
  Radio.setChannel(CHANNEL);  // Above most Wifi Channels
  Radio.setDataRate(RF24_250KBPS); // Fast enough.. Better range

  Radio.setPALevel(RF24_PA_MAX);  // Uncomment for more power

  Radio.enableDynamicPayloads();
  Radio.setRetries(10, 15);
  Radio.setCRCLength(RF24_CRC_16);

  //Radio.openReadingPipe(1, addresses[0]); // Use the first entry in array 'addresses' (Only 1 right now)

  Radio.openWritingPipe(addresses[1]);
  Radio.openReadingPipe(1, addresses[0]);
  
  Radio.startListening();
  // Radio.printDetails();
  Serial.println("Type ? for help");
  Serial.println("Waiting for connection");
  //CMD_SensorsStatus();
  refresh_sensors = 0;
  ReadInput();
}//--(end setup )---


void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{




  if ( Radio.available())   RadioReceiv();

  refresh_sensors++;
  if ( refresh_sensors > 5000)
  {
    ReadInput();
    refresh_sensors = 0;
  }
  else delay(1);



}//--(end main loop )---
uint8_t RadioReceiv()
{
  int t, i;
  s_settings_sens s;

  Serial.print(F("Received "));

  while (Radio.available())  // While there is data ready
  {
    Radio.read( &dataReceived, sizeof(dataReceived) ); // Get the data payload (You must have defined that already!)
  }
  Radio.stopListening();

  
  // DO something with the data, like print it
  switch (dataReceived.cmd) {
    case CMD_GET:
      RF_SendSensor(dataReceived.id);
      break;
    case CMD_GET_ALL:
      for (t = 0; t < MAX_TYPES; t++) {
        for (i = 0; i < MAX_SENSORS; i++) {
          s = Settings.sensors[t][i];
          if (s.active && s.pin > 0)
            RF_SendSensor(dataReceived.id);
        }
      }
      break;
    case CMD_SET:
      DecodeTypeModule(dataReceived.id);
      SELECTED = type_module;
      SetSelectedVal(dataReceived.val);
      Save();
      RF_SendSensor(dataReceived.id);

      break;

    default: STxtIntLn(F("UNKNOW COMMAND"), dataReceived.cmd);


  }
  Radio.startListening();

}

/*-----( Declare User-written Functions )-----*/

//None yet
//*********( THE END )***********
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    if (inChar == '\n') UARTProcess();
    else  if (inChar >= 32)  if (inputString.length() < 20) inputString += inChar;


  }
}

int UARTProcess() {
  int nr, p;
  Serial.print("COMMAND: ");
  inputString.toUpperCase();
  Serial.println(inputString);
  if (inputString == "?")
  {
    Serial.println(F("MODULE id"));
    Serial.println(F("SCANTERM"));
    Serial.println(F("LIST"));
    Serial.println(F("READ"));
    Serial.println(F("LENGTH"));
    Serial.println(F("TEST LENGTH"));
    Serial.println(F("DEVICES ACTIVE"));
    Serial.println(F("DEFAULT"));

  }
  if (EKRAN == E_MAIN) {
    if (inputString == "LENGTH")       CMD_SetAccesory(A_LENGTH);
    if (inputString == "TEST LENGTH")       CMD_TestLength();
    if (inputString == "DEFAULT")       CMD_SensorsDefault();
    if (inputString == "LIST")       CMD_SensorsStatus(-1);
    if (inputString == "SCANTERM")       CMD_ScanTerm();



    if (inputString == "READ") {
      ReadInput();
      CMD_SensorsStatus(-1);
    }
    if (inputString.substring(0, 6) == "MODULE")
    {
      String snr;
      snr = inputString.substring(7);
      Settings.id_module = snr.toInt();
      if (Settings.id_module < 10) Settings.id_module = Settings.id_module * 100;
      strcpy(Settings.name, "MOD ");
      Settings.name[3] = char(48 + (Settings.id_module / 100));
      Save();
      Serial.print("set module id to ");
      Serial.print(Settings.id_module);
      Serial.print(" name ");
      Serial.println(Settings.name);
    }

    if (inputString.length() >= 2)
    {
      if (inputString.charAt(0) == 'A' && isDigit(inputString.charAt(1)))
      {
        inputString.setCharAt(0, ' ');
        nr = inputString.toInt();
        DecodeTypeModule(nr);
        Serial.print(nr);
        if (Settings.sensors[type_module.type][type_module.nr].active) Settings.sensors[type_module.type][type_module.nr].active = 0;
        else Settings.sensors[type_module.type][type_module.nr].active = 1;
        SBool(Settings.sensors[type_module.type][type_module.nr].active, F(" ON "), F(" OFF"));
        Save();
      }
      if (inputString.charAt(0) == 'E' && isDigit(inputString.charAt(1)))
      {
        inputString.setCharAt(0, ' ');
        nr = inputString.toInt();
        DecodeTypeModule(nr);
        SELECTED = type_module;
        CMD_SensorsEdit() ;
      }
    }
  }
  if (EKRAN == E_EDIT) {
    if (inputString == "EXIT" || inputString == "W" ) {
      Save();
      ShowEkran(E_MAIN);
    }
    else if (inputString == "READ")    {
      ReadInput();
      ShowSensorStatus(SELECTED.type, SELECTED.nr);
    }
    else if (inputString == "REVERS")
    {
      SetSelectedOptions(O_REVERS);

    }
    else if (inputString.charAt(0) == 'P' && isDigit(inputString.charAt(1)))
    {
      inputString.setCharAt(0, ' ');
      p = inputString.toInt();
      if ((p < 9 || p > 13) && p != 4) {
        SetSelectedPin(p);
      }
      else Serial.println("PIN REJECTED");
    }
    if (inputString.charAt(0) == 'V' && isDigit(inputString.charAt(1)))
    {
      inputString.setCharAt(0, ' ');
      SetSelectedVal(inputString.toInt());
    }


    if (inputString.charAt(0) == 'M' )
    {
      if (inputString.charAt(1) == 'A' && inputString.charAt(2) == 'X' &&  isDigit(inputString.charAt(3))) {
        inputString.setCharAt(0, ' ');
        inputString.setCharAt(1, ' ');
        inputString.setCharAt(2, ' ');

        SetSelectedMax(inputString.toInt());
        ReadInput();
      }
      else if (inputString.charAt(1) == 'I' && inputString.charAt(2) == 'N'  && isDigit(inputString.charAt(3))) {
        inputString.setCharAt(0, ' ');
        inputString.setCharAt(1, ' ');
        inputString.setCharAt(2, ' ');
        SetSelectedMin(inputString.toInt());
        ReadInput();
      }
    }

  }
  inputString = "";
}
void CMD_SensorsStatus(int type) {
  uint8_t i, t;
  int v;
  char tmp[10];


  for (t = 0; t < MAX_TYPES; t++) {
    if (type == -1 || type == t) {
      Serial.print(F("=====( "));
      Serial.print(GetTypeName(t));
      if (t == T_PWM) Serial.print(F(" 3,5,6"));
      Serial.println(F(" )====="));
      for (i = 0; i < MAX_SENSORS; i++) {
        ShowSensorStatus(t, i);
      }
    }
  }
  Serial.print(F("ACCESORY:"));

  if (Settings.accesory && _BV(A_DS18B20)) Serial.print(F(" DS18B20"));
  if (Settings.accesory && _BV(A_LENGTH)) Serial.print(F(" LENGTH"));
  Serial.println();
  Serial.println(F("Enr - edit, Anr - Active/Deactive, READ - read input, DEFAULT - clear settings"));
}
char *GetTypeName(uint8_t t) {

  if (t < MAX_TYPES)  strcpy(ret, type_name[t]);
  else strcpy(ret, "");

  return ret;

}
char *GetTypeCode(uint8_t t) {

  if (t < MAX_TYPES)  strcpy(ret, type_code[t]);
  else strcpy(ret, "");

  return ret;

}

void SBool(int war, char *tak, char * nie)
{
  if (war) Serial.print(tak);
  else Serial.print(nie);
}

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

void ShowEkran(int e)
{
  EKRAN = e;
  switch (EKRAN) {
    case E_MAIN:

      Serial.println(F("======( MAIN - SENSOR STATUS)===="));
      CMD_SensorsStatus(-1);
      break;
    case E_EDIT:
      Serial.println(F("=====================( EDIT )=================="));
      Serial.println(F("W-write, P[nr] - pin, V[nr], REVERS, MAX[nr] MIN[nr]"));
      break;
  }
}
void Save(void) {
  Settings.key = SETTINGS_KEY;
  EEPROM.put(0, Settings);
  Serial.println("Save...");
}
void Load(void) {
  byte t, i, v;
  s_settings_sens s;

  EEPROM.get(0, Settings);
  if (Settings.key != SETTINGS_KEY) CMD_SensorsDefault();


  for (t = 0; t < MAX_TYPES; t++) {
    for (i = 0; i < MAX_SENSORS; i++) {
      s = Settings.sensors[t][i];
      //s.val=t*10+i;
      if ((s.pin < 0) || (s.pin > 50)) {
        s.pin = -1;
        s.active = 0;
      }
      if (s.pin > 0) {
        if (t == T_OUTS || t == T_PWM) {
          pinMode(s.pin, OUTPUT);
          if (t == T_OUTS) digitalWrite(s.pin, s.val);
          if (t == T_PWM) analogWrite(s.pin, s.val);
        }
        if (t == T_ANALOG) {
          pinMode(s.pin, INPUT);
          s.val = analogRead(s.pin);
        }
      }
      Settings.sensors[t][i] = s;
      //Save();
    }
  }
}


void SInt4(int i)
{
  if (i < 999) Serial.print(F(" "));
  if (i < 99) Serial.print(F(" "));
  if (i < 9) Serial.print(F(" "));
  Serial.print(i);
}
void SInt2(int i)
{

  if (i < 9) Serial.print(F(" "));
  Serial.print(i);
}

void SFloat(int i)
{
  int a, b;
  a = i / 10;
  b = i % 10;
  if (a < 9)Serial.print(F("0"));
  Serial.print(a);
  Serial.print(F(","));
  Serial.print(b);
}

void CMD_SensorsEdit() {
  ShowEkran(E_EDIT);
  ShowSensorStatus(SELECTED.type, SELECTED.nr);
}

void ShowSensorStatus(uint8_t t, uint8_t nr) {
  int v;
  s_settings_sens s;
  s = Settings.sensors[t][nr];
  if (t == 0)       Serial.print(F("0"));
  Serial.print(t * 10 + nr);

  SBool(s.active, F(" - ON  "), F(" - off "));
  v = s.val;
  if (t == T_VOLT || t == T_TEMP) SFloat(v);
  else  SInt4(v);
  Serial.print(F(" "));
  Serial.print(GetTypeCode(t));
  Serial.print(F(" pin:"));
  if (s.pin >= 0)  Serial.print(s.pin);
  else Serial.print(F("X"));
  Serial.print(F(" "));
  if (t == T_TEMP)  {
    if (s.active)     printDeviceAddress(s.DeviceAddress);
    else Serial.print(F("----------------"));
  }
  if ( t == T_PROC)
  {
    Serial.print(" min:");
    SInt4(s.map_min);
    Serial.print(" max:");
    SInt4(s.map_max);
    Serial.print(" analog:");
    SInt4(s.analog);

    {
      Serial.print(" revers:");
      SBool(s.options && _BV(O_REVERS), F("ON"), F("OFF"));
    }
  }
  Serial.println("");
}

void SetSelectedPin(int pin) {

  Settings.sensors[SELECTED.type][ SELECTED.nr].pin = pin;
  if (SELECTED.type == T_OUTS || SELECTED.type == T_PWM  )  pinMode(pin, OUTPUT);
  else   pinMode(pin, INPUT);
  ShowSensorStatus(SELECTED.type, SELECTED.nr);
}

void SetSelectedVal(int val) {

  Settings.sensors[SELECTED.type][ SELECTED.nr].val = val;
  if (SELECTED.type == T_OUTS)  digitalWrite(Settings.sensors[SELECTED.type][ SELECTED.nr].pin, val);
  if (SELECTED.type == T_PWM)  analogWrite(Settings.sensors[SELECTED.type][ SELECTED.nr].pin, val);

  ShowSensorStatus(SELECTED.type, SELECTED.nr);
}

void SetSelectedMax(int val) {
  Settings.sensors[SELECTED.type][ SELECTED.nr].map_max = val;
  ShowSensorStatus(SELECTED.type, SELECTED.nr);
}

void SetSelectedMin(int val) {
  Settings.sensors[SELECTED.type][ SELECTED.nr].map_min = val;
  ShowSensorStatus(SELECTED.type, SELECTED.nr);
}

void SetSelectedOptions(byte options) {
  byte o;
  o = Settings.sensors[SELECTED.type][ SELECTED.nr].options;
  if (o && _BV(options)) bitClear(o, options);
  else bitSet(o, options);


  Settings.sensors[SELECTED.type][ SELECTED.nr].options = o;
  ShowSensorStatus(SELECTED.type, SELECTED.nr);
}

void CMD_SetAccesory(byte accesory) {

  byte a;
  int v;

  a = Settings.accesory;
  if (a && _BV(accesory)) bitClear(a, accesory);
  else bitSet(a, accesory);
  bitClear(a, accesory);//na stale
  Settings.accesory = a;
  Serial.print(F("LENGTH SENSOR: "));
  SBool(a && _BV(accesory), F(" ON "), F(" OFF"));
  if (a && _BV(accesory)) {
    v = GetLength();
    SFloat(v);
  }
  Save();
}

void ReadInput()
{
  int i;
  float tempC;
  s_settings_sens s;


  for (i = 0; i < MAX_SENSORS; i++) {
    s = Settings.sensors[T_VOLT][i];
    if (s.active && s.pin >= 0)
    {
      pinMode(s.pin, INPUT);
      s.analog = analogRead(s.pin);
      s.val = map(s.analog, 0, 1023, 0, 170);
      Settings.sensors[T_VOLT][i] = s;
    }
  }
  for (i = 0; i < MAX_SENSORS; i++) {
    s = Settings.sensors[T_ANALOG][i];
    if (s.active && s.pin >= 0)
    {
      pinMode(s.pin, INPUT);

      s.analog = analogRead(s.pin);
      s.val = s.analog;
      Settings.sensors[T_ANALOG][i] = s;
    }

  }
  for (i = 0; i < MAX_SENSORS; i++) {
    s = Settings.sensors[T_PROC][i];
    if (s.active && s.pin >= 0)
    {
      if (i == 4 && (Settings.accesory && _BV(A_LENGTH))) {
        GetLength();
      }
      else
      {
        pinMode(s.pin, INPUT);
        s.analog = analogRead(s.pin);
        if (s.options && _BV(O_REVERS)) s.val = 1023 - s.analog;
        else s.val = s.analog;

        if (s.val > s.map_max) s.val = s.map_max;
        if (s.val < s.map_min) s.val = s.map_min;
        s.val = map(s.val, s.map_min, s.map_max, 0, 100);
        Settings.sensors[T_PROC][i] = s;
      }
    }

  }



  if (Settings.accesory && _BV(A_DS18B20))
  {
    for (i = 0; i < MAX_SENSORS; i++) {
      s = Settings.sensors[T_TEMP][i];
      if (s.active )
      {
        tempC = DS18B20.getTempC(Settings.sensors[T_TEMP][i].DeviceAddress);
        Settings.sensors[T_TEMP][i].val = (int)(tempC * 10);
      }
    }
    DS18B20.requestTemperatures();
  }

  Serial.println(".");
}

void RF_SendSensor(int id)
{
  s_settings_sens s;
#ifdef DEBUG
  STxtIntLn(F("RF_Wanted "), id);
#endif

  dataSend.val = 0;
  DecodeTypeModule(id);
  dataSend.id = Settings.id_module + (type_module.type * 10) + type_module.nr;
#ifdef DEBUG
  STxtIntLn(F("RF_SendSensor "), dataSend.id);
#endif

  s = Settings.sensors[type_module.type][type_module.nr];
  {
    dataSend.val = s.val;


  }
  Radio.write( &dataSend, sizeof(dataSend) );
}
uint8_t CMD_ScanTerm() {
  //DeviceAddress Thermometer;
  int i, dc;
  float tempC;
  byte a;
  Serial.print(F("Devices found:"));
  Serial.print(DS18B20.getDeviceCount(), DEC);
  Serial.println(F(" devices."));
  Serial.print(F("Parasite power is: "));
  if (DS18B20.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  dc = DS18B20.getDeviceCount();
  for (i = 0; i < MAX_SENSORS; i++)
  {
    //    strcpy(Thermometer, "");
    strcpy(Settings.sensors[T_TEMP][i].DeviceAddress, "");
    Settings.sensors[T_TEMP][i].active = 0;
    Settings.sensors[T_TEMP][i].pin = 0;
    Settings.sensors[T_TEMP][i].val = 0;
    if (i < dc) {
      if (DS18B20.getAddress(Settings.sensors[T_TEMP][i].DeviceAddress, i))
      {
        DS18B20.setResolution(Settings.sensors[T_TEMP][i].DeviceAddress, TEMPERATURE_PRECISION);
        tempC = DS18B20.getTempC(Settings.sensors[T_TEMP][i].DeviceAddress);
        Serial.print("Temp C: ");
        Serial.print(tempC);
        Serial.print(" Address: ");
        printDeviceAddress(Settings.sensors[T_TEMP][i].DeviceAddress);
        Serial.println("");
        Settings.sensors[T_TEMP][i].val = (int)(tempC * 10);
        Settings.sensors[T_TEMP][i].active = 1;
        Settings.sensors[T_TEMP][i].pin = ONE_WIRE_BUS;


      }
    }
  }
  a = Settings.accesory;
  if (dc > 0)bitSet(a, A_DS18B20);
  else bitClear(a, A_DS18B20);
  Settings.accesory = a;

  CMD_SensorsStatus(T_TEMP);
  Save();
}


void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = DS18B20.getTempC(deviceAddress);
  Serial.print(F("Temp C: "));
  Serial.print(tempC);
  Serial.print(F(" Temp F: "));
  Serial.print(DallasTemperature::toFahrenheit(tempC));
  Serial.print(F(" Address: "));
  printDeviceAddress(deviceAddress);
  Serial.println("");
}

void printDeviceAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
//Sensor Length
int GetLength() {
  long czas;
  s_settings_sens s;
  s = Settings.sensors[T_PROC][4];
  pinMode(LENGTH_trigPin, OUTPUT); //Pin trig
  pinMode(LENGTH_echoPin, INPUT); //Pin echo

  digitalWrite(LENGTH_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(LENGTH_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(LENGTH_trigPin, LOW);

  czas = pulseIn(LENGTH_echoPin, HIGH);
  s.analog = czas / 58;//distance cm

  if (s.analog > s.map_max) s.analog = s.map_max;
  if (s.analog < s.map_min) s.analog = s.map_min;
  s.val = map(s.analog, s.map_min, s.map_max, 0, 100);
  if (s.options && _BV(O_REVERS)) s.val = 100 - s.val;

  Settings.sensors[T_PROC][4] = s;

  return czas / 58;
}


void CMD_TestLength() {
  int i, d, last_d;
  for (i = 0; i < 500; i++)
  {
    d = GetLength();
    delay(100);
    if (last_d != d) {
      Serial.println("");
      SInt4(d);
    }
    else   Serial.print(F(" ."));
  }
  Serial.println(F("\nEND"));
}

void CMD_SensorsDefault() {
  byte t, i;
  s_settings_sens s;


  Settings.id_module = 100;
  strcpy(Settings.name, "MOD1");



  for (t = 0; t < MAX_TYPES; t++) {
    for (i = 0; i < MAX_SENSORS; i++) {
      memset(&s, sizeof(s), 0);
      s.pin = i;
      s.active = 0;
      s.nr = 0;
      s.options = 0;
      s.analog = 0;
      s.val = 0;
      s.map_min = 0;
      s.map_max = 1024;
      Settings.sensors[t][i] = s;
    }
  }

  Settings.sensors[T_PWM][0].pin = 3;
  Settings.sensors[T_PWM][1].pin = 5;
  Settings.sensors[T_PWM][2].pin = 6;
  Settings.sensors[T_PWM][3].pin = 0;
  Settings.sensors[T_PWM][4].pin = 0;

  Save();
}
