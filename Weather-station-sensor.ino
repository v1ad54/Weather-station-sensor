/*******************************************************************************************************
 * Author: Chizhov V.
 * 08.11.2015
 * 
 * Метеостанция. Сенсор.
 * v1.0
 *******************************************************************************************************/

/*
 * КОНФИГУРАЦИЯ СЕНСОРА
 * Раскоментируйте #define для используемых модулей
 *    DEBUG             - вывод отладочной информации в Serial
 *    DHT_ON            - подключение модуля температуры и влажности DHT11\21\22
 *    BMP_ON            - подключение модуля давления и температуры BMP085\180
 *    FS1000_ON         - подключение передатчика 433\315MHz
 *    POWER_LOW         - использовать режим энергосбережения
 *    USE_LED_WORK      - использовать светодиод для индикации работы сенсора
 *    USE_LED_LOW_POWER - использовать светодиод для индикации слабого напряжения питания (три коротких периодических моргания между снами)
 *    
 */

//#define DEBUG
#define DHT_ON
//#define BMP_ON
#define FS1000_ON
#define POWER_LOW
//#define USE_LED_WORK
#define USE_LED_LOW_POWER
/*******************************************************************************************************/

//byte _a; // Что бы обмануть препроцессор. См. http://arduino.ru/forum/programmirovanie/oshibka-kompilyatsii-pri-ispolzovanii-ifdefa

#include <Arduino.h>

/*******************************************************************************************************
 * ДАТЧИКИ ТЕМПЕРАТУРЫ И ВЛАЖНОСТИ DHT11/21/22
 *******************************************************************************************************/
#ifdef DHT_ON
  // Подключаем библиотеку работы с датчиками
  #include "DHT.h"
  
  // Датчик DHT22 подключен к пину 5
  #define DHTPin 5
  
  // Раскоментируйте одну из строк, в зависимости от того какой датчик используется
  //#define DHTTYPE DHT11   // DHT 11
  //#define DHTTYPE DHT21   // DHT 21 (AM2301)
  #define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

  // Использовать показания датчика температуры (закоментировать если не используется)
  #define DHT_USE_TEMP
  // Использовать показания датчика влажности (закоментировать если не используется)
  #define DHT_USE_HUM
  
  // Подключаем pin 1 (слева лицевой стороны) сенсора к +5V
  // Если используется плата с 3.3V логикой как например Arduino Due подключаем pin 1 к 3.3V.
  // Подключаем pin 2 сенсора к DHTPin.
  // Подключаем pin 4 (правый) сенсора GROUND.
  // Подключаем резистор 10K между pin 2 (data) и pin 1 (power) сенсора.
  
  // Инициализация DHT сенсора.
  DHT dht(DHTPin, DHTTYPE);
#endif
/*******************************************************************************************************/

/*******************************************************************************************************
 * ДАТЧИКИ ДАВЛЕНИЯ И ТЕМПЕРАТУРЫ BMP085\180
 *******************************************************************************************************/
#ifdef BMP_ON
  // Подключаем библиотеки для работы с датчиками давления
  #include <Wire.h>
  #include <Adafruit_Sensor.h>
  #include <Adafruit_BMP085_U.h>

  // Использовать показания датчика давления (закоментировать если не используется)
  #define BMP_USE_PRESS
  // Использовать показания датчика температуры (закоментировать если не используется)
  //#define BMP_USE_TEMP
  
  // Подключение датчика давления BMP085\180
  //   SCL к A5
  //   SDA к A4
  //   VDD к 3.3V
  //   GND к GND
  
  // В конструкторе указываем уникальный идентификатор датчика (10085)
  Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
#endif
/*******************************************************************************************************/

/*******************************************************************************************************
 * ПЕРЕДАТЧИК FS1000A 433MHz
 * На плате есть обозначение XY-FST. На кварце JDQ R433A.
 *******************************************************************************************************/
#ifdef FS1000_ON
  // Подключаем библиотеки для работы с датчиками давления
  #include <VirtualWire.h>
  // Передатчик подключен к pin 3
  #define pinTransmitter433 3

  // Скорость передачи
  #define TransmissionSpeed 2048
#endif
/*******************************************************************************************************/

/*******************************************************************************************************
 * БИБЛИОТЕКА КОДИРОВАНИЯ ЗНАЧЕНИЙ ДАТЧИКОВ
 *******************************************************************************************************/
#if defined(FS1000_ON) //|| defined(другой передатчик)
  #include "PackageData.h"

  Package_msg msg;  // Передаваемое сообщение
  unsigned int  packet_id = 0; // ID пакета.
#endif
/*******************************************************************************************************/

/*******************************************************************************************************
 * ЭНЕРГОСБЕРЕЖЕНИЕ
 *******************************************************************************************************/
#ifdef POWER_LOW
  #include <LowPower.h>
  #include "MeasurementPower.h"

  // При снижении напряжения до скольки вольт начинать жалобно мигать светодиодом
  #define POWER_LOW_VOLT 3.0
#endif
/*******************************************************************************************************/

/*******************************************************************************************************
 * СИГНАЛЬНЫЙ СВЕТОДИОД
 *******************************************************************************************************/
// Пин используемый светодиодом
#define pinLED    13
/*******************************************************************************************************/

/*******************************************************************************************************
 * Количество периодов простоя по 8 сек., между замерами показаний датчиков. Например 40 периодов - 40 * 8 = 320 сек.
 *******************************************************************************************************/
#define IDLE_PER  40
/*******************************************************************************************************/

void setup(){
  #ifdef DEBUG
    Serial.begin(9600);
    Serial.println("Weather station sensor.");
  #endif

  #ifdef DHT_ON
    #ifdef DEBUG
      Serial.println("DHT11/21/22 on.");
    #endif

    // Инициализируем DHT11/21/22
    dht.begin();
  #endif

  #ifdef BMP_ON
    #ifdef DEBUG
      Serial.println("BMP085\180 on.");
    #endif
    
    /* Инициализируем датчик давления */
    if(!bmp.begin()){
      /* Проблема обнаружения датчика давления BMP085\180, проверьте соединение */
      #ifdef DEBUG
        Serial.print("Ooops, no BMP085\\180 detected ... Check your wiring or I2C ADDR!");
      #endif
      while(1);
    }
  #endif

  #if defined(USE_LED_WORK) || ( defined(USE_LED_LOW_POWER) && defined(POWER_LOW) )
    pinMode(pinLED, OUTPUT);
  #endif

  #ifdef FS1000_ON
    #ifdef DEBUG
      Serial.println("Transmitter FS1000A on.");
    #endif
    
    // Initialise the IO and ISR
    //  vw_set_ptt_inverted(true);  // Required for DR3100
    vw_set_tx_pin(pinTransmitter433); // Установим pin передатчика
    vw_setup(TransmissionSpeed);             // Bits per sec
  #endif
  
}

void loop(){
  #if defined(DHT_USE_TEMP) || defined(BMP_USE_TEMP) //|| defined(другой датчик температуры)
    float temp = 0.0;
  #endif

  #if defined(DHT_USE_HUM)
    float hum = 0.0;
  #endif

  #if defined(BMP_USE_PRESS)
    float pres = 0.0;
  #endif

  #ifdef USE_LED_WORK
    // На время работы зажгем сигнальный светодиод
    digitalWrite(pinLED, HIGH);
  #endif
  
  #ifdef DHT_ON
    #ifdef DHT_USE_HUM
      // Читаем влажность
      hum = dht.readHumidity();
    #endif
    
    #ifdef DHT_USE_TEMP
      // Читаем температуру в градусах Цельсия
      temp = dht.readTemperature();
    #endif

    #ifdef DEBUG
      #ifdef DHT_USE_HUM
        // Отправим значения влажности и температуры в Serial
        Serial.print("DHTxx: Humidity: \t");
        Serial.print(hum);
        Serial.print(" %\t\t\t");
      #endif

      #if defined(DHT_USE_TEMP) || defined(BMP_USE_TEMP)
        Serial.print("Temperature: ");
        Serial.print(temp);
        Serial.println(" C");
      #endif
    #endif
  #endif

  #ifdef BMP_ON
    #if defined(BMP_USE_PRESS) || defined(BMP_USE_TEMP)
      /* Получим новое событие от датчика */ 
      sensors_event_t event;
      bmp.getEvent(&event);
      
      #ifdef DEBUG
        Serial.print("BMPxxx: \t");
      #endif

      if (event.pressure){
        #ifdef BMP_USE_PRESS
          pres = event.pressure;
          #ifdef DEBUG
            Serial.print("Pressure: \t");
            Serial.print(pres);
            Serial.print(" mm Hg \t");
          #endif
        #endif

        #ifdef BMP_USE_TEMP
          bmp.getTemperature(&temp);
          #ifdef DEBUG
            Serial.print("Temperature: ");
            Serial.print(temp);
            Serial.print(" C");
          #endif
        #endif

        #ifdef DEBUG
          Serial.println("");
        #endif
      }
    #endif
  #endif

  // Замерим напряжение питания
  #ifdef POWER_LOW
    float power = readVcc();
    #ifdef DEBUG
      Serial.print("Power: \t");
      Serial.print(power);
      Serial.println("V.");
    #endif
  #endif
  
  #ifdef FS1000_ON
    // Соберем показания датчиков в сообщения
    msg.device_id       = WEATHER_SENSOR1_ID; // ID устройства отправителя
    msg.destination_id  = BROADCAST;          // ID устройства получателя
    
    #if defined(DHT_USE_TEMP) || defined(BMP_USE_TEMP)  // Температура
      msg.packet_id       = ++packet_id;
      msg.command         = CMD_REPORT_WEATHER_SENSOR_TEMP;
      msg.data.f = temp;
      #ifdef DEBUG  
        Serial.print("Sending msg (HEX): \t");
        Serial.print("device_id = "); Serial.print(msg.device_id);
        Serial.print("\tdestination_id = "); Serial.print(msg.destination_id);
        Serial.print("\tpacket_id = "); Serial.print(msg.packet_id);
        Serial.print("\tcommand = "); Serial.println(msg.command);
        Serial.print("Data:\t");
        for(int i = 0 ; i < sizeof(msg.data) ; i++){
          Serial.print(msg.data.ab[i], HEX);
          Serial.print(" ");
        }
        Serial.println("");
      #endif
      // Отправим в эфир показание датчика
      vw_send((uint8_t *)&msg, sizeof(msg));  // Отправим сообщение
      vw_wait_tx();                           // Подождем пока все сообщение не будет отправлено  
    #endif
  
    #if defined(DHT_USE_HUM)                            // Влажность
      msg.packet_id       = ++packet_id;
      msg.command         = CMD_REPORT_WEATHER_SENSOR_HUM;
      msg.data.f = hum;
      #ifdef DEBUG  
        Serial.print("Sending msg (HEX): \t");
        Serial.print("device_id = "); Serial.print(msg.device_id);
        Serial.print("\tdestination_id = "); Serial.print(msg.destination_id);
        Serial.print("\tpacket_id = "); Serial.print(msg.packet_id);
        Serial.print("\tcommand = "); Serial.println(msg.command);
        Serial.print("Data:\t");
        for(int i = 0 ; i < sizeof(msg.data) ; i++){
          Serial.print(msg.data.ab[i], HEX);
          Serial.print(" ");
        }
        Serial.println("");
      #endif
      // Отправим в эфир показание датчика
      vw_send((uint8_t *)&msg, sizeof(msg));  // Отправим сообщение
      vw_wait_tx();   
    #endif
  
    #if defined(BMP_USE_PRESS)                          // Давление
      msg.packet_id       = ++packet_id;
      msg.command         = CMD_REPORT_WEATHER_SENSOR_PRES;
      msg.data.f = pres;
      #ifdef DEBUG  
        Serial.print("Sending msg (HEX): \t");
        Serial.print("device_id = "); Serial.print(msg.device_id);
        Serial.print("\tdestination_id = "); Serial.print(msg.destination_id);
        Serial.print("\tpacket_id = "); Serial.print(msg.packet_id);
        Serial.print("\tcommand = "); Serial.println(msg.command);
        Serial.print("Data:\t");
        for(int i = 0 ; i < sizeof(msg.data) ; i++){
          Serial.print(msg.data.ab[i], HEX);
          Serial.print(" ");
        }
        Serial.println("");
      #endif
      // Отправим в эфир показание датчика
      vw_send((uint8_t *)&msg, sizeof(msg));  // Отправим сообщение
      vw_wait_tx();                           // Подождем пока все сообщение не будет отправлено  
    #endif
  
    #ifdef POWER_LOW                                    // Напряжение
      msg.packet_id       = ++packet_id;
      msg.command         = CMD_REPORT_WEATHER_SENSOR_POWER;
      msg.data.f = power;
      #ifdef DEBUG  
        Serial.print("Sending msg (HEX): \t");
        Serial.print("device_id = "); Serial.print(msg.device_id);
        Serial.print("\tdestination_id = "); Serial.print(msg.destination_id);
        Serial.print("\tpacket_id = "); Serial.print(msg.packet_id);
        Serial.print("\tcommand = "); Serial.println(msg.command);
        Serial.print("Data:\t");
        for(int i = 0 ; i < sizeof(msg.data) ; i++){
          Serial.print(msg.data.ab[i], HEX);
          Serial.print(" ");
        }
        Serial.println("");
      #endif
      // Отправим в эфир показание датчика
      vw_send((uint8_t *)&msg, sizeof(msg));  // Отправим сообщение
      vw_wait_tx();                           // Подождем пока все сообщение не будет отправлено  
    #endif
  #endif

  #ifdef USE_LED_WORK
    // По окончании работы погасим сигнальный светодиод
    digitalWrite(pinLED, LOW);
  #endif
    
  #ifdef POWER_LOW
    // Пауза между опросами датчиков (засыпаем на IDLE_PER периодов по 8 сек.)
    // Сильно желательно на Arduino убрать power led, иначе он все усилия по энергосбережению сведет на нет...
    #ifdef DEBUG
      // Перед сном дадим немного времени дослать отладку в Serial
      delay(500);
    #endif
    
    for(int i = 0 ; i < IDLE_PER ; i++ ){
      #ifdef USE_LED_LOW_POWER
        if (power <= POWER_LOW_VOLT) // Если напряжение низкое то просигналим
          for(int i = 0 ; i < 3; i++){
            digitalWrite(pinLED, HIGH); delay(10);
            digitalWrite(pinLED, LOW);  delay(250);
          }
      #endif

      #if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega168__) 
            LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, 
                  SPI_OFF, USART0_OFF, TWI_OFF);
      #elif defined __AVR_ATmega2560__
            LowPower.idle(SLEEP_8S, ADC_OFF, TIMER5_OFF, TIMER4_OFF, TIMER3_OFF, 
                  TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART3_OFF, 
                  USART2_OFF, USART1_OFF, USART0_OFF, TWI_OFF);
      #elif defined __AVR_ATmega32U4__  
            LowPower.idle(SLEEP_8S, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, 
                  TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF);
      #else // Тут еще должен быть код засыпок для тинек и др.
        delay(8000);
      #endif
    }
  #else
    // Пауза между опросами датчиков (засыпаем на IDLE_PER периодов по 8 сек.) в случае отключения режима энергосбережения
    for(int i = 0 ; i < IDLE_PER ; i++ )
      delay(8000);
  #endif
}
