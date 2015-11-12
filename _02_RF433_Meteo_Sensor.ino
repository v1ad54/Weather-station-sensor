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
 *    DEBUG     - вывод отладочной информации в Serial
 *    DHT_ON    - подключение модуля температуры и влажности DHT11\21\22
 *    BMP_ON    - подключение модуля давления и температуры BMP085\180
 *    FS1000_ON - подключение передатчика 433\315MHz
 *    POWER_LOW - использовать режим энергосбережения
 */

//#define DEBUG
#define DHT_ON
#define BMP_ON
#define FS1000_ON
#define POWER_LOW
/*******************************************************************************************************/

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
  #define TransmissionSpeed 2400
#endif
/*******************************************************************************************************/

/*******************************************************************************************************
 * БИБЛИОТЕКА КОДИРОВАНИЯ ЗНАЧЕНИЙ ДАТЧИКОВ
 *******************************************************************************************************/
#if defined(FS1000_ON) // || #if defined(другой передатчик)
  #include "CodeDecodeFloat.h"
#endif
/*******************************************************************************************************/

/*******************************************************************************************************
 * ЭНЕРГОСБЕРЕЖЕНИЕ
 *******************************************************************************************************/
#ifdef POWER_LOW
  #include <LowPower.h>
  #include "MeasurementPower.h"

#endif
/*******************************************************************************************************/

/*******************************************************************************************************
 * СИГНАЛЬНЫЙ СВЕТОДИОД
 *******************************************************************************************************/
#ifndef POWER_LOW
  // Сигнальный светодиод
  #define pinLEN    13
#endif
/*******************************************************************************************************/

/*******************************************************************************************************
 * Количество периодов простоя между замерами по 8 сек. Например 40 периодов - 40 * 8 = 320 сек.
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

  #ifndef POWER_LOW
    pinMode(pinLEN, OUTPUT);
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
  #if defined(DHT_USE_TEMP) || defined(BMP_USE_TEMP)
    float temp = 0.0;
  #endif

  #if defined(DHT_USE_HUM)
    float hum = 0.0;
  #endif

  #if defined(BMP_USE_PRESS)
    float pres = 0.0;
  #endif

  
  #ifdef FS1000_ON
    // Размер передаваемого сообщения
    #define BUFLEN  12
    // Передаваемое сообщение
    uint8_t buf[BUFLEN];    // буфер для передачи
  #endif

  #ifndef POWER_LOW
    // На время работы зажгем сигнальный светодиод
    digitalWrite(pinLEN, HIGH);
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
      #if defined(DHT_USE_HUM)
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
    // Соберем показания датчиков в сообщение
    #if defined(DHT_USE_TEMP) || defined(BMP_USE_TEMP)  // Температура
      float_to_buf(temp, &buf[0]);                   
    #else
      // Очистим буфер дефалтовым значением 0xFF
      buf[0] = buf[1] = buf[2] = 0xFF;
    #endif
  
    #if defined(DHT_USE_HUM)                            // Влажность
      float_to_buf(hum, &buf[3]);
    #else
      // Очистим буфер дефалтовым значением 0xFF
      buf[3] = buf[4] = buf[5] = 0xFF;
    #endif
  
    #if defined(BMP_USE_PRESS)                          // Давление
      float_to_buf(pres - 900.0, &buf[6]);
    #else
      // Очистим буфер дефалтовым значением 0xFF
      buf[6] = buf[7] = buf[8] = 0xFF;
    #endif
  
    #ifdef POWER_LOW                                    // Напряжение
      float_to_buf(power, &buf[9]);
    #else
      // Очистим буфер дефалтовым значением 0xFF
      buf[9] = buf[10] = buf[11] = 0xFF;
    #endif
    
    #ifdef DEBUG  
      Serial.print("Sending msg (DEC): \t");
      for(int i = 0 ; i < BUFLEN ; i++){
        Serial.print(buf[i], DEC);
        Serial.print(" ");
      }
      Serial.println("");

      Serial.print("Sending msg (HEX): \t");
      for(int i = 0 ; i < BUFLEN ; i++){
        Serial.print(buf[i], HEX);
        Serial.print(" ");
      }
      Serial.println("");
    #endif
    
    // Передаем в эфир показания датчиков
    vw_send((uint8_t *)buf, BUFLEN);  // Отправим сообщение
    vw_wait_tx();                     // Подождем пока все сообщение не будет отправлено  
  #endif
  
  #ifndef POWER_LOW
    // По окончании работы погасим сигнальный светодиод
    digitalWrite(pinLEN, LOW);
    // Пауза между опросами датчиков (засыпаем на IDLE_PER периодов по 8 сек.)
    for(int i = 0 ; i < IDLE_PER ; i++ )
      delay(8000);
  #else
    // Пауза между опросами датчиков (засыпаем на IDLE_PER периодов по 8 сек.)
    // Сильно желательно на Arduino убрать power led, иначе он все усилия по энергосбережению сведет на нет...
    for(int i = 0 ; i < IDLE_PER ; i++ )
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
  #endif
}
