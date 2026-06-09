#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// -----------------------------
// Определение пинов управления транзисторами катушек
// -----------------------------
const int coilTrnPin1 = 2;  // Транзистор катушки 1
const int coilTrnPin2 = 3;  // Транзистор катушки 2
const int coilTrnPin3 = 4;  // Транзистор катушки 3
const int coilTrnPin4 = 5;  // Транзистор катушки 4
const int coilTrnPin5 = 6;  // Транзистор катушки 5

// -----------------------------
// Определение пинов управления
// -----------------------------
const int shtButtonPin = 7; // Кнопка выстрела
const int chrgTransPin = 8; // Управление зарядом конденсаторов
const int debugSwitchPin = 9;  // Вкл/Выкл режим калибровки

// -----------------------------
// Пины потенциометров для задержек
// -----------------------------
const int dlyPotPin1 = A0; // Задержка между кат.1-2
const int dlyPotPin2 = A1; // Задержка между кат.2-3
const int dlyPotPin3 = A2; // Задержка между кат.3-4
const int dlyPotPin4 = A3; // Задержка между кат.4-5

// -----------------------------
// Глобальные переменные для управления состоянием
// -----------------------------
bool charging = false;  // Флаг состояния зарядки
bool readyToFire = false;   // Флаг готовности к выстрелу

// -----------------------------
// Переменные для хранения задержек (в микросекундах)
// -----------------------------
unsigned long delay1;
unsigned long delay2;
unsigned long delay3;
unsigned long delay4;

// -----------------------------
// Фиксированная длительность импульса для всех катушек (150 мкс)
// -----------------------------
const unsigned long pulseDuration = 150;

// -----------------------------
// Инициализация LCD-дисплея
// -----------------------------
LiquidCrystal_I2C lcd(0x27, 16, 2); // Адрес 0x27, дисплей 16x2 символа

// -----------------------------
// Функция начальной настройки
// -----------------------------
void setup() {
    // Инициализация LCD
    lcd.init(); // Инициализация
    lcd.backlight();    // Включение подсветки

    // Сообщение о запуске прошивки
    centerText("Loading firmware", 0);  // Центрированный текст на строке 0
    centerText("Please wait", 1);   // Центрированный текст на строке 1
    delay(2000);    // Задержка для отображения текста

    // Настройка режимов пинов реле
    pinMode(coilTrnPin1, OUTPUT);   // Транзистор 1
    pinMode(coilTrnPin2, OUTPUT);   // Транзистор 2
    pinMode(coilTrnPin3, OUTPUT);   // Транзистор 3
    pinMode(coilTrnPin4, OUTPUT);   // Транзистор 4
    pinMode(coilTrnPin5, OUTPUT);   // Транзистор 5

    // Настройка режимов пинов кнопки и транзистора преобразователя
    pinMode(shtButtonPin, INPUT_PULLUP);    // Кнопка выстрела
    pinMode(chrgTransPin, OUTPUT);  // Управление зарядом
    pinMode(debugSwitchPin, INPUT_PULLUP);  // Переключатель режима калибровки

    // Настройка режимов пинов потенциометров
    pinMode(dlyPotPin1, INPUT);    // Потенциометр 1
    pinMode(dlyPotPin2, INPUT);    // Потенциометр 2
    pinMode(dlyPotPin3, INPUT);    // Потенциометр 3
    pinMode(dlyPotPin4, INPUT);    // Потенциометр 4

    // Сообщение о версии прошивки
    lcd.clear();    // Очистка экрана
    centerText("Firmware version", 0);  // Текст на строке 0
    centerText("03062026", 1);  // Версия на строке 1
    delay(2000);    // Задержка
    lcd.clear();    // Очистка экрана

    // Начальная разрядка конденсаторов
    dischargeCapacitors();
}

// -----------------------------
// Основной цикл программы
// -----------------------------
void loop() {
    // Вкл/Выкл показ задержек
    if (digitalRead(debugSwitchPin) == LOW) {
        readPotentiometers();   // Читаем потенциометры задержек
        displayCurrentDelays(); // Показываем задержки
        lcd.clear();
    }

    // Проверяем состояние кнопки выстрела
    if (digitalRead(shtButtonPin) == LOW) {
        delay(50); // Дебаунс
        if (digitalRead(shtButtonPin) == LOW) {
            // Если зарядка не начата и выстрел не готов
            if (charging == false && readyToFire == false) {
                startCharging();    // Начинаем зарядку
            } else if (charging == false && readyToFire == true) {
                fireSequence(); // Выполняем выстрел
            }
            while (digitalRead(shtButtonPin) == LOW);   // Ждем отпускания
        }
    }

    if (readyToFire == true) {
        centerText("Ready", 0);
        centerText("to FIRE!", 1);
        lcd.clear();
    } else if (readyToFire == false) {
        centerText("Not ready", 0);
        centerText("to FIRE!", 1);
        lcd.clear();
    }

    delay(10);  // Небольшая задержка для стабильности
}

// -----------------------------
// Чтение значений с потенциометров
// -----------------------------
void readPotentiometers() {
    delay1 = map(analogRead(dlyPotPin1), 0, 1023, 0, 3000);    // 0-3000 мкс
    delay2 = map(analogRead(dlyPotPin2), 0, 1023, 0, 3000);    // 0-3000 мкс
    delay3 = map(analogRead(dlyPotPin3), 0, 1023, 0, 3000);    // 0-3000 мкс
    delay4 = map(analogRead(dlyPotPin4), 0, 1023, 0, 3000);    // 0-3000 мкс
}

// -----------------------------
// Отображение текущих задержек на дисплее
// -----------------------------
void displayCurrentDelays() {
    // Первая строка: задержки 1 и 2
    lcd.setCursor(0, 0);
    lcd.print("D1:");
    lcd.print(delay1);
    lcd.print(" D2:");
    lcd.print(delay2);
    
    // Вторая строка: задержки 3 и 4
    lcd.setCursor(0, 1);
    lcd.print("D3:");
    lcd.print(delay3);
    lcd.print(" D4:");
    lcd.print(delay4);

    lcd.clear();
}

// -----------------------------
// Функция разрядки конденсаторов
// -----------------------------
void dischargeCapacitors() {
    lcd.clear();    // Очистка экрана
    centerText("Discharging", 0);   // Сообщение о разрядке
    centerText("capacitors...", 1);

    // Включаем все транзисторы для разрядки
    digitalWrite(coilTrnPin1, HIGH);
    digitalWrite(coilTrnPin2, HIGH);
    digitalWrite(coilTrnPin3, HIGH);
    digitalWrite(coilTrnPin4, HIGH);
    digitalWrite(coilTrnPin5, HIGH);

    delayMicroseconds(500); // Задержка для разрядки

    // Закрываем транзисторы
    digitalWrite(coilTrnPin1, LOW);
    digitalWrite(coilTrnPin2, LOW);
    digitalWrite(coilTrnPin3, LOW);
    digitalWrite(coilTrnPin4, LOW);
    digitalWrite(coilTrnPin5, LOW);

    // Завершаем разрядку
    lcd.clear();    // Очистка экрана
    centerText("Discharged!", 0);    // Сообщение о готовности
    delay(1000);
    lcd.clear();
}

// -----------------------------
// Функция начала зарядки конденсаторов
// -----------------------------
void startCharging() {
    charging = true;    // Устанавливаем флаг зарядки
    lcd.clear();    // Очистка экрана
    centerText("Charging...", 0);   // Сообщение о зарядке

    digitalWrite(chrgTransPin, HIGH);   // Запуск зарядки

    // Анимация прогресса заряда
    for (int i = 0; i <= 16; i++) {
        lcd.setCursor(0, 1);
        lcd.print("[");
        for (int j = 0; j < 16; j++) {
            if (j < i) lcd.print("#");
            else lcd.print(" ");
        }
        lcd.print("]");
        delay(500); // Общее время зарядки ~8 сек
    }

    digitalWrite(chrgTransPin, LOW);    // Завершение зарядки
    charging = false;   // Сбрасываем флаг зарядки
    readyToFire = true; // Устанавливаем флаг готовности
    
    lcd.clear();
    centerText("Charged!", 0);    // Сообщение о завершении
    delay(1000);
    lcd.clear();
}

// -----------------------------
// Функция выполнения выстрела
// -----------------------------
void fireSequence() {
    lcd.clear();
    centerText("Firing...", 0); // Сообщение о выстреле

    // Катушка 1 - включаем сразу
    digitalWrite(coilTrnPin1, HIGH);
    delayMicroseconds(pulseDuration);
    digitalWrite(coilTrnPin1, LOW);
    
    // Задержка перед катушкой 2
    delayMicroseconds(delay1);
    
    // Катушка 2
    digitalWrite(coilTrnPin2, HIGH);
    delayMicroseconds(pulseDuration);
    digitalWrite(coilTrnPin2, LOW);
    
    // Задержка перед катушкой 3
    delayMicroseconds(delay2);
    
    // Катушка 3
    digitalWrite(coilTrnPin3, HIGH);
    delayMicroseconds(pulseDuration);
    digitalWrite(coilTrnPin3, LOW);
    
    // Задержка перед катушкой 4
    delayMicroseconds(delay3);
    
    // Катушка 4
    digitalWrite(coilTrnPin4, HIGH);
    delayMicroseconds(pulseDuration);
    digitalWrite(coilTrnPin4, LOW);
    
    // Задержка перед катушкой 5
    delayMicroseconds(delay4);
    
    // Катушка 5
    digitalWrite(coilTrnPin5, HIGH);
    delayMicroseconds(pulseDuration);
    digitalWrite(coilTrnPin5, LOW);
    
    // Завершение выстрела
    readyToFire = false;
    
    lcd.clear();
    centerText("Shot complete!", 0);    // Сообщение о завершении
    delay(1000);
    lcd.clear();
    
    // Автоматическая разрядка после выстрела
    dischargeCapacitors();
}

// -----------------------------
// Функция центрирования текста
// -----------------------------
void centerText(const char* text, int row) {
    int len = strlen(text); // Длина текста
    int pos = (16 - len) / 2;   // Вычисление позиции
    lcd.setCursor(pos, row);    // Установка курсора
    lcd.print(text);    // Вывод текста
}