#ifndef CodeDecodeFloat_h
#define CodeDecodeFloat_h

/*
 * Переделка скетча взятого отсюда: http://bigbarrel.ru/float-to-byte/
 * 
 * Данные скетч были написан для передачи комнатной температуры с контроллера на контроллер при программировании «Умного Дома».
 * Значение кодируем в 3 байта данных.
 * 1 байт — знак(+/-)
 * 1 байт — целая часть до 255
 * 1 байт дробная часть до 0.255
 * На входе функции значение типа float на выходе uint8_t buff[3];
*/

// Кодирование
uint8_t* float_to_buf(float val, uint8_t* buff)
{
    #ifdef DEBUG
      Serial.println("uint8_t* float_to_buf(float val)");
      Serial.print("val = ");
      Serial.println(val);
    #endif

    if (val > 255.99 || val < -255.99){
      buff[0] = buff[1] = buff[2] = 0x00;
      return 0;
    }

    if (val >= 0) //передаем знак значения + или -
        buff[0] = 0x00;
    else
        buff[0] = 0x01;
    #ifdef DEBUG
      Serial.print("buff[0] = ");
      Serial.print(buff[0]);
    #endif

    buff[1] = (uint8_t)val;
    #ifdef DEBUG
      Serial.print("\tbuff[1] = ");
      Serial.print(buff[1]);
    #endif
    
    buff[2] = (uint8_t)((val - (int)val) * 100.0);
    #ifdef DEBUG
      Serial.print("\tbuff[2] = ");
      Serial.println(buff[2]);
    #endif

    return buff;
}

// Декодирование в String
String buf_to_string(uint8_t* buff)
{
    #ifdef DEBUG
      Serial.println("String buf_to_string(uint8_t* buff)");
    #endif

    String float_s;
    int val = 0;
    val = buff[0];
    #ifdef DEBUG
      Serial.print("buff[0] = ");
      Serial.println(buff[0]);
    #endif
    
    if (val == 0x01) float_s = "-";
    val = buff[1];
    #ifdef DEBUG
      Serial.print("buff[1] = ");
      Serial.println(buff[1]);
    #endif

    float_s = float_s + val + ".";
    val = buff[2];
    #ifdef DEBUG
      Serial.print("buff[2] = ");
      Serial.println(buff[2]);
    #endif

    if (val < 10) float_s = float_s + "0" + val;
    else float_s = float_s + val;
            
    #ifdef DEBUG
      Serial.print("return ");
      Serial.println(float_s);
    #endif

    return float_s;
}

// Декодирование в float
float buf_to_float(uint8_t* buff)
{
    float float_;
    int val = 0;
    val = buff[0];
    if (val == 0x01) float_ = -1;
    val = buff[1];
    float_ = float_ * val;
    val = buff[2];
    float_ = float_ + val / 1000;
    return float_;
}

#endif
