### Общее описание программы

Программа написана на языке C++ для микроконтроллера Arduino, которая управляет двумя датчиками с поддержкой калибровки. Она включает в себя следующие основные компоненты:

1. **Классы и структуры:**

    - `CalibrationPoint` и `CalibrationData`: для хранения точек калибровки.
    - Интерфейс `ISensor`: определяет общие методы для датчиков.
    - `CalibratedSensor`: класс датчика с поддержкой калибровки, реализующий `ISensor`.
    - `SensorManager`: управление коллекцией датчиков.
    - `CommandProcessor`: обработка пользовательских команд, полученных через Serial.
2. **Основная программа:**

    - Создание объектов `SensorManager` и `CommandProcessor`.
    - Инициализация датчиков и добавление их в `SensorManager`.
    - Основной цикл `loop()`, который обрабатывает входящие команды и отображает данные датчиков.

---

### Поток данных от получения до вывода

1. **Получение данных:**

    - Программа ждет ввода пользователя через последовательный порт (Serial).
    - Когда пользователь вводит команду и нажимает "Enter", символы считываются методом `Serial.readStringUntil('\n')`.
2. **Обработка команд:**

    - Введенная строка команды очищается от лишних пробелов с помощью `command.trim()`.
    - Строка передается в метод `processCommand()` объекта `CommandProcessor`.
    - `CommandProcessor` анализирует команду и определяет, какое действие нужно выполнить (калибровка или сброс калибровки).
3. **Изменение состояния датчиков:**

    - Если это команда калибровки (`cal1` или `cal2`), `CommandProcessor` извлекает параметры команды (сырое и откалиброванное значения) и добавляет точку калибровки в соответствующий датчик через метод `addCalibrationPoint()`.
    - Если это команда сброса (`reset1` или `reset2`), `CommandProcessor` вызывает метод `resetCalibration()` у соответствующего датчика, очищая его данные калибровки.
4. **Обновление данных калибровки:**

    - При добавлении или сбросе точек калибровки, датчик обновляет свои внутренние структуры данных (`CalibrationData`).
    - Данные калибровки сохраняются или загружаются из памяти EEPROM, обеспечивая сохранность данных между перезапусками программы.
5. **Чтение данных с датчиков:**

    - В основном цикле `loop()`, после обработки команды, вызывается метод `sensorManager.displayValues()`, который:
        - Получает текущее время с помощью `millis()`.
        - Проходит по каждому датчику и:
        - Считывает сырое значение с датчика через `getRawValue()`.
        - Вычисляет откалиброванное значение через `getCalibratedValue()`, используя данные калибровки.
6. **Вычисление откалиброванного значения:**

    - Метод `getCalibratedValue()` датчика вызывает `findCalibratedValue()`, который:
        - Определяет, нужно ли выполнять интерполяцию или экстраполяцию в зависимости от текущего сырого значения и имеющихся точек калибровки.
        - Выполняет соответствующие математические вычисления (линейную интерполяцию или экстраполяцию) для получения откалиброванного значения.
7. **Вывод данных на экран:**

    - `sensorManager.displayValues()` выводит в Serial монитор информацию:
        - Метку времени.
        - Для каждого датчика:
        - Номер датчика.
        - Сырое значение.
        - Откалиброванное значение.
        - Разделительную линию для визуального разделения данных.

---

### Разбор используемых синтаксических конструкций

#### 1. **Препроцессорные директивы**

```cpp
#include <EEPROM.h>
```

- **Назначение:** Подключает библиотеку `EEPROM`, которая позволяет читать и записывать данные во встроенную энергонезависимую память микроконтроллера.
- **Директива `#include`:** Указывает компилятору включить содержимое указанного файла.

#### 2. **Структуры данных**

##### a) **`CalibrationPoint`**

```cpp
struct CalibrationPoint {
    int rawValue;          // Сырое значение датчика
    int calibratedValue;   // Соответствующее откалиброванное значение
};
```

- **Назначение:** Хранит одну точку калибровки, связывая сырое значение датчика с соответствующим откалиброванным значением.
- **Ключевые слова:**
    - `struct`: Объявление структуры, которая является пользовательским типом данных, состоящим из набора полей.

##### b) **`CalibrationData`**

```cpp
struct CalibrationData {
    CalibrationPoint points[20]; // Массив точек калибровки (до 20)
    int count = 0;               // Текущее количество точек
};
```

- **Назначение:** Хранит массив точек калибровки и текущее количество точек.
- **Инициализация поля:**
    - `int count = 0;`: Присваивает 0 полю `count` при создании объекта структуры.

#### 3. **Интерфейс `ISensor`**

```cpp
class ISensor {
public:
    virtual int getRawValue() = 0;                // Получение сырого значения
    virtual int getCalibratedValue() = 0;         // Получение откалиброванного значения
    virtual void addCalibrationPoint(int rawValue, int calibratedValue) = 0;  // Добавление точки калибровки
    virtual void resetCalibration() = 0;          // Сброс данных калибровки
    virtual int getCalibrationPointCount() const = 0;  // Получение количества точек
    virtual ~ISensor() {}                         // Виртуальный деструктор
};
```

- **Назначение:** Определяет интерфейс для датчиков, задавая набор виртуальных методов, которые должны быть реализованы в классах-наследниках.
- **Ключевые слова:**
    - `class`: Определение класса.
    - `public`: Модификатор доступа, определяет секцию открытых членов класса.
    - `virtual`: Указывает, что метод является виртуальным и может быть переопределен в подклассах.
    - `= 0;`: Делает метод чисто виртуальным, то есть класс становится абстрактным и не может быть инстанцирован.
    - `~ISensor() {}`: Виртуальный деструктор, чтобы обеспечить корректное освобождение ресурсов в наследниках.

#### 4. **Класс `CalibratedSensor`**

```cpp
class CalibratedSensor : public ISensor {
public:
    // Конструктор
    CalibratedSensor(int pin, int eepromAddress);

    // Реализация методов интерфейса ISensor
    int getRawValue() override;
    int getCalibratedValue() override;
    void addCalibrationPoint(int rawValue, int calibratedValue) override;
    void resetCalibration() override;
    int getCalibrationPointCount() const override;

private:
    // Поля данных
    const int MAX_CALIBRATION_POINTS = 20;
    int analogPin;
    int eepromStartAddress;
    CalibrationData calibrationData;

    // Вспомогательные методы
    void loadCalibration();
    void saveCalibration();
    int findCalibratedValue(int rawValue);
    int interpolate(int rawValue, int index1, int index2);
    int extrapolate(int rawValue, int index1, int index2);
};
```

- **Наследование:**
    - `: public ISensor`: Класс `CalibratedSensor` наследует интерфейс `ISensor` с публичным доступом, то есть все `public` и `protected` члены `ISensor` сохраняют свои уровни доступа в `CalibratedSensor`.
- **Ключевые слова:**
    - `override`: Указывает, что метод переопределяет виртуальный метод базового класса. Помогает компилятору обнаруживать ошибки при переопределении.
    - `const`: Указывает, что метод или переменная не изменяет состояние объекта или имеет постоянное значение.
- **Конструктор:**
    - `CalibratedSensor(int pin, int eepromAddress);`
    - Инициализирует объект с заданным пином и адресом в EEPROM.

##### a) **Метод `getRawValue()`**

```cpp
int CalibratedSensor::getRawValue() {
    return analogRead(analogPin);
}
```

- **Назначение:** Возвращает сырое значение, считанное с аналогового пина датчика.
- **Функция `analogRead()`:** Считывает аналоговое значение (0-1023) с указанного пина.

##### b) **Метод `getCalibratedValue()`**

```cpp
int CalibratedSensor::getCalibratedValue() {
    int rawValue = getRawValue();
    return findCalibratedValue(rawValue);
}
```

- **Назначение:** Вычисляет и возвращает откалиброванное значение на основе сырого значения и данных калибровки.
- **Вызов вспомогательного метода `findCalibratedValue()`:** Выполняет интерполяцию или экстраполяцию для получения откалиброванного значения.

##### c) **Метод `addCalibrationPoint()`**

```cpp
void CalibratedSensor::addCalibrationPoint(int rawValue, int calibratedValue) {
    // Проверка на максимум точек
    if (calibrationData.count >= MAX_CALIBRATION_POINTS) {
        Serial.println("Достигнуто максимальное количество точек калибровки.");
        return;
    }

    // Проверка на существование точки с таким же сырым значением
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

    // Вставка новой точки в отсортированный массив
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
```

- **Назначение:** Добавляет новую точку калибровки или обновляет существующую.
- **Используемые конструкции:**
    - **Цикл `for`:** Проверяет существующие точки на совпадение.
    - **Цикл `while`:** Вставляет новую точку в правильное место, сохраняя массив отсортированным по `rawValue`.
    - **Инициализация структуры при присваивании:**
    - `calibrationData.points[i + 1] = {rawValue, calibratedValue};`

##### d) **Метод `resetCalibration()`**

```cpp
void CalibratedSensor::resetCalibration() {
    calibrationData.count = 0; // Обнуляем количество точек
    saveCalibration();         // Сохраняем изменения в EEPROM
    Serial.println("Калибровочные данные сброшены.");
}
```

- **Назначение:** Очищает все данные калибровки датчика.

##### e) **Метод `findCalibratedValue()`**

```cpp
int CalibratedSensor::findCalibratedValue(int rawValue) {
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
    return rawValue; // Если не нашли соответствия, возвращаем сырое значение
}
```

- **Назначение:** Вычисляет откалиброванное значение путем интерполяции или экстраполяции.
- **Используемые конструкции:**
    - **Условные операторы `if`:** Проверка различных случаев (количество точек, границы диапазона).
    - **Цикл `for`:** Поиск соответствующего интервала для интерполяции.

##### f) **Методы `interpolate()` и `extrapolate()`**

```cpp
int CalibratedSensor::interpolate(int rawValue, int index1, int index2) {
    int raw1 = calibrationData.points[index1].rawValue;
    int raw2 = calibrationData.points[index2].rawValue;
    int cal1 = calibrationData.points[index1].calibratedValue;
    int cal2 = calibrationData.points[index2].calibratedValue;
    float slope = (float)(cal2 - cal1) / (raw2 - raw1);
    return cal1 + slope * (rawValue - raw1);
}

int CalibratedSensor::extrapolate(int rawValue, int index1, int index2) {
    // Логика аналогична interpolate()
}
```

- **Назначение:** Вычисляют откалиброванное значение с помощью линейной интерполяции или экстраполяции между двумя точками.
- **Используемые конструкции:**
    - **Приведение типов:**
    - `(float)(cal2 - cal1) / (raw2 - raw1);` — обеспечивает дробное деление.
    - **Арифметические операции:** Вычисление наклона (`slope`) и откалиброванного значения.

#### 5. **Класс `SensorManager`**

```cpp
class SensorManager {
public:
    void addSensor(ISensor* sensor);
    ISensor* getSensor(int index);
    void displayValues();

private:
    static const int MAX_SENSORS = 2;
    ISensor* sensors[MAX_SENSORS];
    int sensorCount = 0;
};
```

- **Назначение:** Управляет коллекцией датчиков.
- **Ключевые слова:**
    - `ISensor*`: Использование указателей на объекты интерфейса `ISensor`.
- **Методы:**
    - `addSensor()`: Добавляет датчик в массив `sensors`.
    - `getSensor()`: Возвращает указатель на датчик по индексу.
    - `displayValues()`: Отображает текущие значения всех датчиков.

#### 6. **Класс `CommandProcessor`**

```cpp
class CommandProcessor {
public:
    CommandProcessor(SensorManager& manager);
    void processCommand(const String& command);

private:
    SensorManager& sensorManager;

    void processCalibrationCommand(int sensorIndex, const String& params);
    void processResetCommand(int sensorIndex);
};
```

- **Назначение:** Обрабатывает команды, полученные через Serial, и выполняет действия с датчиками через `SensorManager`.
- **Ключевые слова:**
    - `SensorManager&`: Ссылка на объект `SensorManager`, гарантируя, что `CommandProcessor` работает с тем же экземпляром.
    - `const String&`: Ссылка на константный объект `String`, предотвращает копирование строки и изменение ее внутри метода.
- **Методы:**
    - `processCommand()`: Анализирует входную команду и вызывает соответствующий метод обработки.
    - `processCalibrationCommand()`: Обрабатывает команды калибровки.
    - `processResetCommand()`: Обрабатывает команды сброса калибровки.

#### 7. **Глобальные объекты и функции `setup()` и `loop()`**

```cpp
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
```

---
### Синтаксические конструкции, используемые в программе

#### 1. **Классы и объекты**

- **Классы:** `ISensor`, `CalibratedSensor`, `SensorManager`, `CommandProcessor`.
- **Конструкторы и деструкторы:**
    - Конструкторы позволяют инициализировать объекты при создании.
    - Виртуальные деструкторы в интерфейсе `ISensor` необходимы для корректного удаления объектов через указатель на базовый класс.
- **Наследование:**
    - `CalibratedSensor` наследует `ISensor`, реализуя его методы.
- **Полиморфизм:**
    - Использование виртуальных функций позволяет вызывать методы объектов `CalibratedSensor` через указатель на `ISensor`.

#### 2. **Указатели и ссылки**

- **Указатели:**
    - `ISensor* sensor`: Указатель на объект `ISensor`.
    - Используются для динамического выделения памяти (`new`) и передачи объектов по ссылке.
- **Ссылки:**
    - `SensorManager& sensorManager`: Ссылка на объект `SensorManager` в `CommandProcessor`.
    - Позволяет работать с оригинальным объектом без копирования.

#### 3. **Массивы**

- **Статические массивы:**
    - `ISensor* sensors[MAX_SENSORS];` в `SensorManager`.
    - `CalibrationPoint points[20];` в `CalibrationData`.
- **Динамические массивы не используются**, но возможны к внедрению для гибкости.

#### 4. **Циклы**

- **`for`-циклы:**
    - Используются для перебора элементов в массивах, например, при добавлении точек калибровки или отображении значений датчиков.
- **`while`-циклы:**
    - Используются для сдвига элементов массива при вставке новой точки калибровки в отсортированном порядке.

#### 5. **Условные операторы**

- **`if-else` конструкции:**
    - Проверяют различные условия, например, валидность введенных команд, границы индекса массива, наличие данных калибровки.
- **Использование логических операторов:**
    - `&&`, `||`, `!` для составления сложных условий.

#### 6. **Работа со строками**

- **Класс `String`:**
    - Используется для хранения и обработки строк, полученных из Serial.
- **Методы класса `String`:**
    - `startsWith()`: Проверяет, начинается ли строка с заданной подстроки.
    - `substring()`: Извлекает подстроку.
    - `trim()`: Удаляет лидирующие и завершающие пробелы.
    - `indexOf()`: Ищет символ или подстроку и возвращает ее индекс.
    - `toInt()`: Преобразует строку в целое число.

#### 7. **Ввод/вывод**

- **Serial коммуникация:**
    - `Serial.begin(9600);`: Инициализирует Serial с заданной скоростью передачи данных.
    - `Serial.available()`: Проверяет, есть ли доступные данные для чтения.
    - `Serial.readStringUntil('\n')`: Считывает строку до символа новой строки.
    - `Serial.print()`, `Serial.println()`: Выводят данные в Serial монитор.

#### 8. **Энергонезависимая память EEPROM**

- **Методы `EEPROM`:**
    - `EEPROM.get(address, data);`: Считывает данные из EEPROM по заданному адресу.
    - `EEPROM.put(address, data);`: Записывает данные в EEPROM по заданному адресу.
- **Используется для сохранения и восстановления данных калибровки между перезапусками программы.**

#### 9. **Функции Arduino**

- **`setup()`:** Выполняется один раз при запуске программы. Используется для инициализации.
- **`loop()`:** Бесконечный цикл, который выполняется после `setup()`. Основной код программы, обрабатывает ввод пользователя и обновляет состояние системы.
- **`delay(ms)`:** Останавливает выполнение программы на заданное количество миллисекунд.

#### 10. **Приведение типов**

- **Явное приведение типов к `float`:**
    - `(float)(cal2 - cal1) / (raw2 - raw1);` — необходимо для получения дробного числа при делении целых чисел.

#### 11. **Операции с памятью**

- **Динамическое выделение памяти:**
    - `new CalibratedSensor(A0, 0);`: Создает новый объект в динамической памяти.
- **Важно:** В текущей программе нет явного освобождения памяти (`delete`), что может привести к утечке памяти на долго работающих системах.

---

### Заключение

Программа организована таким образом, чтобы обеспечить модульность и расширяемость:

- **Использование интерфейса `ISensor` и полиморфизма** позволяет легко добавлять новые типы датчиков, реализующих требуемые методы.
- **Класс `SensorManager`** упрощает управление коллекцией датчиков, отделяя логику работы с ними от основной программы.
- **Класс `CommandProcessor`** позволяет обрабатывать пользовательские команды и взаимодействовать с датчиками в режиме реального времени.
- **Использование EEPROM** обеспечивает сохранение данных калибровки между запусками программы.

**Поток данных** в программе начинается с ввода пользователя, который обрабатывается и приводит к изменению состояния датчиков. Затем программа считывает данные с датчиков, применяет калибровку и выводит результаты на экран. Все это организовано с помощью описанных выше синтаксических конструкций, обеспечивающих структуру и функциональность программы.
