typedef struct s_data_module {
  int id;
  int value;
  char type;//Volt , C % Analog
  uint8_t MORE;
}
typedef struct s_command {
uint8_t cmd;
int id;
int value;
}
#define CMD_GETFIRST 1
#define CMD_GETNEXT 2
#define CMD_POWER 3

