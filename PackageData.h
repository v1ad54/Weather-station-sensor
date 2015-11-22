#ifndef Package_h
#define Package_h

/*******************************************************************************************************
 * ID устройств
 *******************************************************************************************************/
#define BROADCAST           0x00  // Широковещательный пакет
#define WEATHER_STATION_ID  0x01  // Метеостанция
#define WEATHER_SENSOR1_ID  0x02  // Сенсор метеостанции 1
#define WEATHER_SENSOR2_ID  0x03  // Сенсор метеостанции 2
#define WEATHER_SENSOR3_ID  0x04  // Сенсор метеостанции 3
#define WEATHER_SENSOR4_ID  0x05  // Сенсор метеостанции 4
#define WEATHER_SENSOR5_ID  0x06  // Сенсор метеостанции 5
#define WEATHER_SENSOR6_ID  0x07  // Сенсор метеостанции 6
#define WEATHER_SENSOR7_ID  0x08  // Сенсор метеостанции 7
#define WEATHER_SENSOR8_ID  0x09  // Сенсор метеостанции 8

/*******************************************************************************************************
 * Типы сенсоров
 *******************************************************************************************************/
#define CMD_REPORT_WEATHER_SENSOR_TEMP   0x01  // Отчет датчика температуры
#define CMD_REPORT_WEATHER_SENSOR_HUM    0x02  // Отчет датчика влажности
#define CMD_REPORT_WEATHER_SENSOR_PRES   0x03  // Отчет датчика давления
#define CMD_REPORT_WEATHER_SENSOR_RAIN   0x04  // Отчет датчика дождя
#define CMD_REPORT_WEATHER_SENSOR_POWER  0x05  // Отчет датчика напряжения

// Данные пакета
union msg_data{
  byte  b;
  byte  ab[19];
  char  c;
  char  ac[19];
  int   i;
  float f;
};

/*******************************************************************************************************
 * Структура пакета сообщения
 *******************************************************************************************************/
struct Package_msg{
public:
  unsigned int  device_id;       // ID устройства отправителя
  unsigned int  destination_id;  // ID устройства получателя или BROADCAST
  unsigned int  packet_id;       // ID пакета
  unsigned int  command;         // Команда
  msg_data      data;            // Данные
};

#endif
