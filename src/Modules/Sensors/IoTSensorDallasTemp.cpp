#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"
#include "Utils/StringUtils.h"
#include "Class/IoTSensor.h"
#include "Class/IoTModule.h"


#include "DallasTemperature.h"
#include <OneWire.h>
#include <map>


//глобальные списки необходимы для хранения объектов об активных линиях 1-wire используемых разными датчиками из модуля. Ключ - номер пина
std::map<int, OneWire*> oneWireTemperatureArray;
std::map<int, DallasTemperature*> sensorsTemperatureArray;

class IoTSensorDallas: public IoTSensor {
    private:
        //описание переменных экземпляра датчика - аналог глобальных переменных из Arduino
        //для работы библиотеки с несколькими линиями  необходимо обеспечить каждый экземпляр класса ссылками на объекты настроенные на эти линии
        OneWire* oneWire;
        DallasTemperature* sensors;

        //описание параметров передаваемых из настроек датчика из веба
        String _addr;
        unsigned int _pin;
        unsigned int _index;

    public:
        //аналог setup() из Arduino
        IoTSensorDallas(String parameters) {
            //передаем часть базовых параметров в конструктор базового класса для обеспечения работы его методов
            init(jsonReadStr(parameters, "key"), jsonReadStr(parameters, "id"), jsonReadInt(parameters, "int"));  
            _pin = jsonReadInt(parameters, "pin");
            _index = jsonReadInt(parameters, "index");
            _addr = jsonReadStr(parameters, "addr");             

            //учитываем, что библиотека может работать с несколькими линиями на разных пинах, поэтому инициируем библиотеку, если линия ранее не использовалась
            if (oneWireTemperatureArray.find(_pin) == oneWireTemperatureArray.end()) {
                oneWire = new OneWire((uint8_t)_pin);
                sensors = new DallasTemperature();
                sensors->setOneWire(oneWire);
                sensors->begin();
                sensors->setResolution(12);

                oneWireTemperatureArray[_pin] = oneWire;
                sensorsTemperatureArray[_pin] = sensors;
            } else {
                oneWire = oneWireTemperatureArray[_pin];
                sensors = sensorsTemperatureArray[_pin];
            }
        }
        
        ~IoTSensorDallas() {}

        //аналог loop() из Arduino но квотируемый по времени параметром interval
        void doByInterval() {  
            //запускаем опрос измерений у всех датчиков на линии
            sensors->requestTemperatures();
            
            //Определяем адрес. Если парамтер addr не установлен, то узнаем адрес по индексу
            DeviceAddress deviceAddress;
            if (_addr == "") {
                sensors->getAddress(deviceAddress, _index);
            } else {
                string2hex(_addr.c_str(), deviceAddress);
            }
            //получаем температуру по адресу
            float value = sensors->getTempC(deviceAddress); 
           
            char addrStr[20] = "";
            hex2string(deviceAddress, 8, addrStr);

            regEvent((String)value, "addr: " + String(addrStr));  //обязательный вызов для отправки результата работы
        }
};

//технический класс для взаимодействия с ядром, меняются только названия
class IoTModuleDallasTemp: public IoTModule {
    //обязательный метод для инициализации экземпляра датчика, вызывается при чтении конфигурации. Нужно учитывать, что некоторые датчики могут обеспечивать
    //несколько измерений, для каждого будет отдельный вызов.
    void* initInstance(String parameters) {
        return new IoTSensorDallas(parameters);
    };

    //обязательный к заполнению метод, если модуль использует свои глобальные переменные. Необходимо сбросить и очистить используемую память.
    void clear() {
        // for (auto it = sensorsTemperatureArray.cbegin(), next_it = it; it != sensorsTemperatureArray.cend(); it = next_it) {
        //     ++next_it;
        //     DallasTemperature* tmpptr = it->second;  //временно сохраняем указатель на сенсор, т.к. его преждевременное удаление оставит поломаную запись в векторе, к которой может обратиться ядро и вызвать исключение
        //     sensorsTemperatureArray.erase(it);
        //     delete tmpptr;  //а далее уже удаляем объект сенсора
        // }
        
        // for (auto it = oneWireTemperatureArray.cbegin(), next_it = it; it != oneWireTemperatureArray.cend(); it = next_it) {
        //     ++next_it;
        //     OneWire* tmpptr = it->second;  //временно сохраняем указатель на сенсор, т.к. его преждевременное удаление оставит поломаную запись в векторе, к которой может обратиться ядро и вызвать исключение
        //     oneWireTemperatureArray.erase(it);
        //     delete tmpptr;  //а далее уже удаляем объект сенсора
        // }
    }

    //обязательный метод для отправки информации о модуле, 
    ModuleInfo getInfo() {
        ModuleInfo MI;
        MI.name = "dallas-temp";
        MI.title = "Датчик температуры Ds18b20";
        MI.parameters = "{\"key\": \"dallas-temp\", \"id\": \"tmp\", \"addr\": \"\", \"int\": \"10\", \"pin\": \"18\", \"index\": \"0\"}";
        MI.type = "Sensor";
        return MI;
    };
};

//точка входа в модуль для заполнения вектора, требуется только изменить имя и прописать в файле api.cpp
void* getApiIoTSensorDallasTemp() {
    return new IoTModuleDallasTemp();
}