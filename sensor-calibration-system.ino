#include <EEPROM.h>

// Структура для хранения одной точки калибровки
struct CalibrationPoint {
    int rawValue;         // Сырой сигнал датчика
    int calibratedValue;  // Откалиброванное значение
};

// Структура для хранения всех точек калибровки одного датчика
struct CalibrationData {
    CalibrationPoint points[20]; // Массив точек калибровки
    int count = 0;               // Текущее количество точек калибровки
};

// Интерфейс для датчиков, предоставляющий основные методы
class ISensor {
public:
    virtual int getRawValue() = 0;               // Получение сырого значения
    virtual int getCalibratedValue() = 0;        // Получение откалиброванного значения
    virtual void addCalibrationPoint(int rawValue, int calibratedValue) = 0; // Добавление точки калибровки
    virtual void resetCalibration() = 0;         // Сброс всех данных калибровки
    virtual int getCalibrationPointCount() const = 0; // Получение количества точек
    virtual ~ISensor() {}                        // Виртуальный деструктор
};

// Класс датчика с поддержкой калибровки
class CalibratedSensor : public ISensor {
public:
    // Конструктор, инициализирующий пин и адрес EEPROM
    CalibratedSensor(int pin, int eepromAddress)
        : analogPin(pin), eepromStartAddress(eepromAddress) {
        pinMode(analogPin, INPUT_PULLUP); // Устанавливаем пин как вход с подтяжкой
        loadCalibration();               // Загружаем данные калибровки из EEPROM
    }

    // Возвращает сырое значение, считываемое с аналогового пина
    int getRawValue() override {
        return analogRead(analogPin);
    }

    // Возвращает откалиброванное значение с использованием данных калибровки
    int getCalibratedValue() override {
        int rawValue = getRawValue();
        return findCalibratedValue(rawValue);
    }

    // Добавляет новую точку калибровки
    void addCalibrationPoint(int rawValue, int calibratedValue) override {
        // Проверяем, достигнуто ли максимальное количество точек
        if (calibrationData.count >= MAX_CALIBRATION_POINTS) {
            Serial.println("Достигнуто максимальное количество точек калибровки.");
            return;
        }

        // Проверяем на существование точки с таким же сырым значением
        for (int i = 0; i < calibrationData.count; i++) {
            if (calibrationData.points[i].rawValue == rawValue) {
                calibrationData.points[i].calibratedValue = calibratedValue; // Обновляем значение
                saveCalibration();
                Serial.print("Обновлена точка калибровки: ");
                Serial.print(rawValue);
                Serial.print(" -> ");
                Serial.println(calibratedValue);
                return;
            }
        }

        // Вставляем новую точку в отсортированный массив
        int i = calibrationData.count - 1;
        while (i >= 0 && calibrationData.points[i].rawValue > rawValue) {
            calibrationData.points[i + 1] = calibrationData.points[i];
            i--;
        }
        calibrationData.points[i + 1] = {rawValue, calibratedValue}; // Добавляем новую точку
        calibrationData.count++;
        saveCalibration();

        Serial.print("Добавлена точка калибровки: ");
        Serial.print(rawValue);
        Serial.print(" -> ");
        Serial.println(calibratedValue);
    }

    // Сбрасывает данные калибровки
    void resetCalibration() override {
        calibrationData.count = 0; // Обнуляем количество точек
        saveCalibration();         // Обновляем EEPROM
        Serial.println("Калибровочные данные сброшены.");
    }

    // Возвращает количество точек калибровки
    int getCalibrationPointCount() const override {
        return calibrationData.count;
    }

private:
    const int MAX_CALIBRATION_POINTS = 20; // Максимальное число точек
    int analogPin;                         // Пин, подключенный к датчику
    int eepromStartAddress;                // Адрес EEPROM для хранения данных
    CalibrationData calibrationData;       // Данные калибровки

    // Загружает данные калибровки из EEPROM
    void loadCalibration() {
        int addr = eepromStartAddress;
        EEPROM.get(addr, calibrationData);

        // Проверяем корректность загруженных данных
        if (calibrationData.count > MAX_CALIBRATION_POINTS || calibrationData.count < 0) {
            calibrationData.count = 0; // Обнуляем данные, если они некорректны
        }
    }

    // Сохраняет данные калибровки в EEPROM
    void saveCalibration() {
        int addr = eepromStartAddress;
        EEPROM.put(addr, calibrationData);
    }

    // Находит откалиброванное значение по сырому сигналу
    int findCalibratedValue(int rawValue) {
        if (calibrationData.count == 0) {
            Serial.println("Нет данных для калибровки.");
            return rawValue;
        }

        if (calibrationData.count == 1) {
            int offset = calibrationData.points[0].calibratedValue - calibrationData.points[0].rawValue;
            return rawValue + offset;
        }

        if (rawValue <= calibrationData.points[0].rawValue) {
            return extrapolate(rawValue, 0, 1);
        }

        if (rawValue >= calibrationData.points[calibrationData.count - 1].rawValue) {
            return extrapolate(rawValue, calibrationData.count - 2, calibrationData.count - 1);
        }

        for (int i = 0; i < calibrationData.count - 1; i++) {
            if (rawValue >= calibrationData.points[i].rawValue && rawValue <= calibrationData.points[i + 1].rawValue) {
                return interpolate(rawValue, i, i + 1);
            }
        }
        return rawValue; // Если не нашли, возвращаем сырое значение
    }

    // Выполняет линейную интерполяцию между двумя точками
    int interpolate(int rawValue, int index1, int index2) {
        int raw1 = calibrationData.points[index1].rawValue;
        int raw2 = calibrationData.points[index2].rawValue;
        int cal1 = calibrationData.points[index1].calibratedValue;
        int cal2 = calibrationData.points[index2].calibratedValue;
        float slope = (float)(cal2 - cal1) / (raw2 - raw1);
        return cal1 + slope * (rawValue - raw1);
    }

    // Выполняет линейную экстраполяцию за пределами диапазона
    int extrapolate(int rawValue, int index1, int index2) {
        int raw1 = calibrationData.points[index1].rawValue;
        int raw2 = calibrationData.points[index2].rawValue;
        int cal1 = calibrationData.points[index1].calibratedValue;
        int cal2 = calibrationData.points[index2].calibratedValue;
        float slope = (float)(cal2 - cal1) / (raw2 - raw1);
        return cal1 + slope * (rawValue - raw1);
    }
};

// Класс для управления несколькими датчиками
class SensorManager {
public:
    // Добавляет датчик в список
    void addSensor(ISensor* sensor) {
        if (sensorCount < MAX_SENSORS) {
            sensors[sensorCount++] = sensor;
        }
    }

    // Возвращает датчик по индексу
    ISensor* getSensor(int index) {
        if (index >= 0 && index < sensorCount) {
            return sensors[index];
        }
        return nullptr;
    }

    // Отображает данные всех датчиков
    void displayValues() {
        Serial.print("Время: ");
        Serial.println(millis());
        for (int i = 0; i < sensorCount; i++) {
            Serial.print("Датчик ");
            Serial.print(i + 1);
            Serial.print(": Сырое ");
            Serial.print(sensors[i]->getRawValue());
            Serial.print(" Откалиброванное ");
            Serial.println(sensors[i]->getCalibratedValue());
        }
        Serial.println("-----------------------------");
    }

private:
    static const int MAX_SENSORS = 2;  // Максимальное количество датчиков
    ISensor* sensors[MAX_SENSORS];    // Список датчиков
    int sensorCount = 0;              // Текущее количество датчиков
};

// Класс для обработки пользовательских команд
class CommandProcessor {
public:
    CommandProcessor(SensorManager& manager) : sensorManager(manager) {}

    // Обрабатывает команды, полученные через Serial
    void processCommand(const String& command) {
        if (command.startsWith("cal1 ")) {
            processCalibrationCommand(0, command.substring(5));
        } else if (command.startsWith("cal2 ")) {
            processCalibrationCommand(1, command.substring(5));
        } else if (command.startsWith("reset1")) {
            processResetCommand(0);
        } else if (command.startsWith("reset2")) {
            processResetCommand(1);
        } else {
            Serial.println("Неизвестная команда.");
        }
    }

private:
    SensorManager& sensorManager;

    // Обрабатывает команду калибровки
    void processCalibrationCommand(int sensorIndex, const String& params) {
        ISensor* sensor = sensorManager.getSensor(sensorIndex);
        if (!sensor) return;

        int spaceIndex = params.indexOf(' ');
        if (spaceIndex != -1) {
            int raw = params.substring(0, spaceIndex).toInt();
            int cal = params.substring(spaceIndex + 1).toInt();
            sensor->addCalibrationPoint(raw, cal);
        }
    }

    // Обрабатывает команду сброса калибровки
    void processResetCommand(int sensorIndex) {
        ISensor* sensor = sensorManager.getSensor(sensorIndex);
        if (sensor) {
            sensor->resetCalibration();
        }
    }
};

// Создаем глобальные объекты для управления системой
SensorManager sensorManager;
CommandProcessor* commandProcessor;

void setup() {
    Serial.begin(9600);
    sensorManager.addSensor(new CalibratedSensor(A0, 0));  // Первый датчик на пине A0
    sensorManager.addSensor(new CalibratedSensor(A1, 100)); // Второй датчик на пине A1
    commandProcessor = new CommandProcessor(sensorManager); // Создаем обработчик команд
    Serial.println("Система готова. Вводите команды для управления датчиками.");
}

void loop() {
    // Обрабатываем входящие команды и отображаем данные датчиков
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        commandProcessor->processCommand(command);
    }
    sensorManager.displayValues();
    delay(500); // Задержка для уменьшения частоты вывода
}