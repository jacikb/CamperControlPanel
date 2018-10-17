#define LCD_W   480
#define LCD_H  320

#define R_NONE 0
#define R_FULL 1
#define R_UPDATE 2
#define R_DRAW 3

#define P_WELCOME 0x0101
#define P_CLICK 0x010A

#define O_ACTIVE 1
#define O_ALARM 2
#define O_RAPORT 3

#define K_ENTER -1
#define K_CANCEL -2
#define K_DEL -3

#define K_TAK -4
#define K_NIE -5
#define K_MENU -6
#define K_SETUP -7
#define K_MODULES -8
#define K_SENSORS -9
#define K_ALARMS -10
#define K_ALARM -11
#define K_DEBUG -12
#define K_MAIN -13
#define K_ID -14
#define K_TYPE -15
#define K_ACTIVE -16
#define K_MIN -17
#define K_MAX -18
#define K_MINUS -19
#define K_CLEAR -20
#define K_ICON -21
#define K_RANDOM -22
#define K_SAVE -23
#define K_EXIT -24
#define K_PAGE -25
#define K_MODEM -26
#define K_TEST -27
#define K_VERSION -28
#define K_SOUND -29
#define K_SMS -30
#define K_RAPORT -31
#define K_NAME -32
#define K_DEFAULT -33
#define K_RESET -34
#define K_NET -35
#define K_ON -36
#define K_OFF -37
#define K_1H -38
#define K_24H -39
#define K_DATETIME -40
#define K_OUT -41

#define K_UP -42
#define K_DOWN -43

#define K_DAY_UP -60
#define K_DAY_DO -61
#define K_MONTH_UP -62
#define K_MONTH_DO -63
#define K_YEAR_UP -64
#define K_YEAR_DO -65
#define K_HOUR_UP -66
#define K_HOUR_DO -67
#define K_MINUTE_UP -68
#define K_MINUTE_DO -69
#define K_SECOND_UP -70
#define K_SECOND_DO -71
#define K_NOW -72



#define K_NONE -999

#define T_VOLT 0
#define T_TEMP 1
#define T_PROC 2
#define T_ANALOG 3
#define T_OUTS 4
#define T_PWM 5
byte type_name[][8] = {"VOLT", "TEMP.","PROCENT","ANALOG","OUTS","PWM"};

//months
byte miesiace_pl[][5] = {"Sty.\0", "Luty\0","Marz\0","Kwie\0","Maj \0","Czer\0", "Lipe\0","Sier\0","Wrze\0","Pazd\0","List\0","Grud\0"};

#define T_PAGE 10
#define T_SETUP 11
#define T_ALARM 12



#define E_MAIN 1
#define E_DEVICE 2
#define E_SETUP 3
#define E_GRAPH 4
#define E_SOUND 5
#define E_SENSORS 6
#define E_SENSOR 7
#define E_ICONS 8
#define E_MODEM 9
#define E_DATETIME 10



#define S_AKU1 1
#define S_AKU2 2
#define S_TEMP_IN 3
#define S_TEMP_OUT 4
#define S_TEMP_ZAM 5
#define S_TEMP_LOD 6
#define S_WODA_SZARA 7
#define S_WODA_CZYSTA 8
#define S_LOD_WIATRAKI 9
#define S_OUT1 10
#define S_OUT2 11
#define S_OUT3 12

#define graph_x 40
#define graph_y 300

#define graph_w 440
#define graph_h 200


#define G_CONNECT 1;


#define UNIX_DAY 86400L
