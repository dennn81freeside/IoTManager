#include "Global.h"
#include "classes/IoTItem.h"

#include <IRremote.hpp> //=============================================


#include <IRremote.h>

#include <IRremoteInt.h>
#include <Arduino.h>
//#include <WebSockets4WebServer.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <StandWebServer.h>
//#include "IRremoteESP8266.h" // библиотека для работы с ИК приёмником и ИК светодиодом
//#include "IRrecv.h" // библиотека для работы с ИК приёмником
//#include "IRsend.h" // библиотека для отправки ИК сигналов
//#include "IRutils.h" // библиотека для работы с полученными ИК сигналами
//#include "IRac.h" // библиотека для отправки команд кондиционера
#include "ESPAsyncTCP.h" // библиотека для асинхронных TCP запросов
#include <FS.h> // библиотека для работы с SPIFFS
#include <ArduinoJson.h> // библиотека для работы с JSON
void handle_OnConnect();

void handle_command();

void handle_read();

void handle_remem();

void handle_change();

void handle_NotFound();
IoTItem* globalItem = nullptr;


static const char irTypes[][14] PROGMEM = { "UNKNOWN", "UNUSED", "RC5", "RC6", "NEC", "SONY", "PANASONIC", "JVC", "SAMSUNG", "WHYNTER",
  "AIWA_RC_T501", "LG", "SANYO", "MITSUBISHI", "DISH", "SHARP", "COOLIX", "DAIKIN", "DENON", "KELVINATOR", "SHERWOOD", "MITSUBISHI_AC",
  "RCMM", "SANYO_LC7461", "RC5X", "GREE", "PRONTO", "NEC_LIKE", "ARGO", "TROTEC" };




//const int RECV_PIN = 14;
//IRrecv irrecv(RECV_PIN);
const uint8_t PIN_IR_TX = 14;//ножка подключения ИК-диода (GPIO4 (D2)-WEMOS D1 R1/GPIO14 -TIWE3S)
const uint8_t PIN_IR_RX = 5; //ножка подключения Фотодиода (GPIO13 (D7)-WEMOS D1 R1/GPIO5 -TIWE3S)
const uint8_t LED_BUILT = 2; //ножка подключения светодиода (GPIO2 (на плате)-WEMOS D1 R1/GPIO4 -TIWE3S)

const uint16_t CAPTURE_BUFFER_SIZE = 256;
const uint8_t TIMEOUT = 45; // 15



//ESP8266WiFiMulti WiFiMulti;

//WebSockets4WebServer webSocket;




decode_results results;

IRrecv irrecv = IRrecv();
IRsend irsender = IRsend();



String code1 { "ИНФРОКРАСНЫЙ" };
String code2 { "     ПУЛЬТ" };
String kn[] { "1000000001", "2000000002","3000000003","4000000004","5000000005","6000000006","7000000007","8000000008","9000000009","100000010","110000011","120000012","130000013","140000014","150000015","160000016"};  // массив кодов кнопок
String mem {"000"}; // номер кнопки при присвоении кода
String strRead = "pointer-events: none;color: #FF9033;";  // строка блокирования кнопки "СЧИТАТЬ КОД"
String strIzm = "";  // строка блокирования кнопки "ИЗМЕНИТЬ"
String strWrite = "pointer-events: none;color: #000;";  // строка блокирования кнопки "СОХРАНИТЬ В ПУЛЬТЕ"
String SendHTML();

//*******************************
void handle_OnConnect() {
  HTTP.send(200, "text/html", SendHTML()); 
}
//*******************************
void handle_change() {
  strRead = "";
  strIzm = "pointer-events: none;color: #3368FF;";
  strWrite = "";
  HTTP.send(200, "text/html", SendHTML()); 
}
//*******************************
void handle_command() {
  irrecv.disableIRIn();
  char charBuf[14];
  mem = HTTP.arg(0);  // запоминаем номер кнопки для которой надо внести код
  HTTP.arg(1).toCharArray(charBuf, 14);
  irsender.sendNECMSB(strtoul(charBuf,NULL,16),32);
  irrecv.enableIRIn();
  HTTP.send(200, "text/html", SendHTML());
}
//*******************************
void handle_read() {
  digitalWrite(LED_BUILT,LOW);  // зажигаем светодиод
  Serial.print("\n read IR_code");
  int i=0;
  
    //while ((!irrecv.decode(&results))&&i<40)// ждем сигнала с пульта 20 секунд было while ((!irrecv.decode(&results))&&i<40)
    //{
    //delay(500);
    //Serial.print(".");
    //i = i+1;
    //}
    if(i<40){
      code1 = String(FPSTR(irTypes[results.decode_type - UNKNOWN]));
      code2 = String((uint32_t)results.value,HEX);
      if(mem =="kn1") kn[0] = code2; // заносим код в соответствующую ячейку-кнопку
      else if(mem == "kn2") kn[1] = code2;
      else if(mem == "kn3") kn[2] = code2;
      else if(mem == "kn4") kn[3] = code2;
      else if(mem == "kn5") kn[4] = code2;
      else if(mem == "kn6") kn[5] = code2;
      else if(mem == "kn7") kn[6] = code2;
      else if(mem == "kn8") kn[7] = code2;
      else if(mem == "kn9") kn[8] = code2;
      else if(mem == "kn10") kn[9] = code2;
	    else if(mem == "kn10") kn[10] = code2;
	    else if(mem == "kn11") kn[11] = code2;
	    else if(mem == "kn12") kn[12] = code2;
	    else if(mem == "kn13") kn[13] = code2;
	    else if(mem == "kn14") kn[14] = code2;
	    else if(mem == "kn15") kn[15] = code2;
	    else if(mem == "kn16") kn[16] = code2;
     }
      else {
        irrecv.resume();
        code1 = "UNKNOWN";
        code2 = "0";
      }
    HTTP.send(200, "text/html", SendHTML());
  digitalWrite(LED_BUILT,HIGH);   // гасим светодиод
}

void handle_remem() {
  digitalWrite(LED_BUILT,LOW);  // зажигаем светодиод
  Serial.print("\n remem IR_code");
  File file = LittleFS.open("/d.txt", "w");
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  int ii = 0;
  while(ii < 9){
    file.write((char *)kn[ii].c_str(),12);
    ii++;
  }
  file.close();
  strRead = "pointer-events: none;color: #FF9033;"; // блокируем кнопку "СЧИТАТЬ КОД"
  strIzm = "";                                      // активируем кнопку "ИЗМЕНИТЬ"
  strWrite = "pointer-events: none;color: #000;";   // блокируем кнопку "СОХРАНИТЬ В ПУЛЬТЕ"
  HTTP.send(200, "text/html", SendHTML());
  
  digitalWrite(LED_BUILT,HIGH);   // гасим светодиод 
}
//**********************************
void handle_NotFound() {
  HTTP.send(404, "text/plain", "Not found");
}
//**********************************
String SendHTML() {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n";
  ptr +="<title>IR pult</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +=".table {width: 90%; height: 100%; border: 2px solid #000;margin: auto;}\n";
  ptr +=".button {display: block;width: auto; height: auto; background-color: #1abc9c;border: none;color: white;padding: 50px 50px;text-align: center;text-decoration: none;font-size: 35px;margin: 15px auto 15px;cursor: pointer;border-radius: 24px;}\n";
  ptr +=".button:active {background-color: #2c3e50;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>" + code1 + "|" + code2 + "</h1>\n";
  ptr +="<h3 style=\"font-size: 12px;\">Для запоминания кнопки пульта  - нажать кнопку ИЗМЕНИТЬ. Затем нажать одну из кнопок на экране, затем нажать СЧИТАТЬ КОД. Когда загориться светодиод, поднести пульт телевизора к контроллеру, нажать на нём кнопку. Посде появления кода на экране, можно проверить работу запрограмированной кнопки на экране. Повторить считывание кодов для всех кнопок. В конце нажать СОХРАНИТЬ В ПУЛЬТЕ - все коды сохраняться в контроллере.</h3>\n";
  ptr +="<table class=\"table\"><tr><td>\n";
  ptr +="<a class=\"button\" style=\"background-color: #FF9033;" + strRead + "\" href=\"/read\">СЧИТАТЬ КОД</a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn1&code=" + kn[0] + "\">НАСТРОЙКА</a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn4&code=" + kn[3] + "\">ЗВУК - ⇦ </a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn7&code=" + kn[6] + "\">ОК</a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn7&code=" + kn[9] + "\">ОК</a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn7&code=" + kn[12] + "\">ОК</a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn10&code=" + kn[15] + "\">⾕</a>\n";
  ptr +="</td><td>\n";
  ptr +="<a class=\"button\" style=\"background-color: #3368FF;" + strIzm + "\" href=\"/change\">ИЗМЕНИТЬ КОДЫ</a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn2&code=" + kn[1] + "\">КАНАЛ + </a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn5&code=" + kn[4] + "\">ВКЛ./ВЫКЛ.</a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn8&code=" + kn[7] + "\">КАНАЛ - </a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn7&code=" + kn[10] + "\">ОК</a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn7&code=" + kn[13] + "\">ОК</a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn10&code=" + kn[16] + "\">⾕</a>\n";
  ptr +="</td><td>\n";
  ptr +="<a class=\"button\" style=\"background-color: #000;" + strWrite + "\" href=\"/remem\">СОХРАНИТЬ В ПУЛЬТЕ</a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn3&code=" + kn[2] + "\">ВВЕРХ ⇧ </a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn6&code=" + kn[5] + "\">ЗВУК + ⇨ </a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn9&code=" + kn[8] + "\">ВНИЗ ⇩ </a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn10&code=" + kn[11] + "\">⾕</a>\n";
  ptr +="<a class=\"button\" href=\"/command?Type=kn10&code=" + kn[14] + "\">⾕</a>\n";
  ptr +="</td></tr></table>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}




//*******************************************************
//void loop() {
//  HTTP.handleClient();//приём http запросов
//}



class IRhub : public IoTItem {
   private:
    int _pinRx; // Ввод ИК модуля
    int _pinTx; // Выход модуля ИК передатчика
    int _intRepeat = 6;
    int _repeatCount = 0;
    unsigned long _oldValue = 0;
    unsigned long _newValue = 0;

   public:
    IRhub(String parameters): IoTItem(parameters) {
      //  jsonRead(parameters, "pinRx", _pinRx);// приемник
       // jsonRead(parameters, "pinTx", _pinTx);//передатчик

        jsonRead(parameters, "pinRx", _pinRx);
        jsonRead(parameters, "pinTx", _pinTx);
       // jsonRead(parameters, "intRepeat", _intRepeat);

		_interval = _interval / 1000;   // корректируем величину интервала int, теперь он в миллисекундах
		
      //  if (_pinRx >= 0)
/*
       IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK); // Start the receiver

    Serial.print(F("Ready to receive IR signals of protocols: "));
    printActiveIRProtocols(&Serial);
  
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  Serial.println();
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
 
  Serial.println(RECV_PIN);

  */

         if (_pinRx >= 0) {
            Serial.printf("Protocol: %d", _pinRx);
            IrReceiver.begin(_pinRx);
        }
        if (_pinTx >= 0)
        IrSender.enableIROut(_pinTx);
         {

           Serial.printf("Protocol: %d", _pinTx);
           IrSender.begin(_pinTx);
    }
    }



    


    void doByInterval() {
        pinMode(PIN_IR_TX, OUTPUT);
  pinMode(PIN_IR_RX, INPUT);
  pinMode(LED_BUILT, OUTPUT);
  digitalWrite(LED_BUILT, HIGH);

  HTTP.on("/irhub", handle_OnConnect); //исполнить handle_OnConnect при пустом запросе
  //HTTP.on("/command", handle_command);
  HTTP.on("/irhub/command", handle_command); //исполнить handle_command при запросе command
  //HTTP.on("/read", handle_read);  
  HTTP.on("/irhub/read", handle_read); //исполнить handle_read при запросе read
  //HTTP.on("/remem", handle_remem); 
  HTTP.on("/irhub/remem", handle_remem);//исполнить handle_remem при запросе remem
  //HTTP.on("/change", handle_change);
  HTTP.on("/irhub/change", handle_change);//исполнить handle_change при запросе remem
  //HTTP.onNotFound(handle_NotFound);
  HTTP.onNotFound(handle_NotFound);//исполнить handle_NotFound при неизвестном запросе
  
  //Serial.println("HTTP server started");
  
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  };
    
  File file = LittleFS.open("/d.txt", "r");
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
    }
    else
    {
      //Serial.println(file.size());
      int ii = 0;       //  для определения конца чтения
      while(ii < 9){
        file.seek(ii*12);
        kn[ii] = file.readString();
        //Serial.println(kn[ii]); 
        ii++;
      }  
      file.close();
    }
      if (_pinRx >= 0 && IrReceiver.available() && IrReceiver.decode()) {
     //Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX); // Print "old" raw data
      // USE NEW 3.x FUNCTIONS
      IrReceiver.printIRResultShort(&Serial); // Print complete received data in one line
      //IrReceiver.printIRSendUsage(&Serial);   // Print the statement required to send this data

               value.valD = IrReceiver.decodedIRData.decodedRawData;                 

            regEvent(value.valD, "IRhub");         //обязательный вызов хотяб один раз 

        IrReceiver.resume();  // Receive the next value      
         }
          
    }

    ~IRhub() {};    
};

void* getAPI_IRhub(String subtype, String param) {
    if (subtype == F("IRhub")) {
        return new IRhub(param);
    } else {
        return nullptr;  
    }
}


