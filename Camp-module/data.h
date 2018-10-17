
#include <avr/pgmspace.h>
#include<stdlib.h>
#include "Arduino.h"
#define T_VOLT 0
#define T_TEMP 1
#define T_PROC 2
#define T_ANALOG 3
#define T_OUTS 4
#define T_PWM 5

#define E_MAIN 0

#define E_EDIT 1
#define O_REVERS 0


#define A_DS18B20 0
#define A_LENGTH 1



byte type_name[][8] = {"VOLT", "TEMP.", "PROCENT", "ANALOG", "OUTS", "PWM"};
byte type_code[][2] = {"V", "C", "%", "A", " ", " "};
#define MAX_SENSORS 5
#define MAX_TYPES 6
#define MAX_VALUES 3
#define NO_VALUE -999
struct s_settings_sens {
  uint8_t nr;
  int pin;
  //char  DeviceAddress[8];
  DeviceAddress  DeviceAddress;


  uint8_t active;
  uint8_t options;// O_REVERS
  int val;
  int analog;
  int map_min;
  int map_max;


  int values[MAX_VALUES];//ostatnich kilka pomiarow
  byte values_ind;


};
#define SETTINGS_KEY 2
struct s_settings {
  byte key;
  int id_module;
  char name[11];
  byte accesory;
  byte empty1;
  byte empty2;
  byte empty3;
  byte empty4;
  s_settings_sens sensors[MAX_TYPES][MAX_SENSORS];

};



#define CMD_PING 1
#define CMD_GET 2
#define CMD_GET_ALL 3

#define CMD_SET 4

