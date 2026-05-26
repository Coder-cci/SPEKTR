#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// -----------------------------
// Определение пинов управления транзисторами катушек
// -----------------------------
const int trnPin1 = 2;  // Транзистор катушки 1
const int trnPin2 = 3;  // Транзистор катушки 2
const int trnPin3 = 4;  // Транзистор катушки 3
const int trnPin4 = 5;  // Транзистор катушки 4
const int trnPin5 = 6;  // Транзистор катушки 5

// -----------------------------
// Определение пинов управления
// -----------------------------
const int shtButton = 7;  // Кнопка выстрела
const int chrgTrans = 8;  // Управление зарядом конденсаторов

// -----------------------------
// Пины потенциометров для задержек
// -----------------------------
const int pot1 = A0;  // Задержка между кат.1-2
const int pot2 = A1;  // Задержка между кат.2-3
const int pot3 = A2;  // Задержка между кат.3-4
const int pot4 = A3;  // Задержка между кат.4-5

// -----------------------------
// Глобальные переменные для управления состоянием
// -----------------------------
bool charging = false;     // Флаг состояния зарядки
bool readyToFire = false;  // Флаг готовности к выстрелу

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
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Адрес 0x27, дисплей 16x2 символа

// -----------------------------
// Функция начальной настройки
// -----------------------------
void setup() {
    // Инициализация LCD
    lcd.init();                      // Инициализация
    lcd.backlight();                 // Включение подсветки

    // Сообщение о запуске прошивки
    centerText("Loading firmware", 0);  // Центрированный текст на строке 0
    centerText("Please wait", 1);       // Центрированный текст на строке 1
    delay(2000);                        // Задержка для отображения текста

    // Настройка режимов пинов реле
    pinMode(trnPin1, OUTPUT);  // Транзистор 1
    pinMode(trnPin2, OUTPUT);  // Транзистор 2
    pinMode(trnPin3, OUTPUT);  // Транзистор 3
    pinMode(trnPin4, OUTPUT);  // Транзистор 4
    pinMode(trnPin5, OUTPUT);  // Транзистор 5

    // Настройка режимов пинов кнопки и транзистора преобразователя
    pinMode(shtButton, INPUT_PULLUP);  // Кнопка выстрела
    pinMode(chrgTrans, OUTPUT);        // Управление зарядом

    // Настройка режимов пинов потенциометров
    pinMode(pot1, INPUT);  // Потенциометр 1
    pinMode(pot2, INPUT);  // Потенциометр 2
    pinMode(pot3, INPUT);  // Потенциометр 3
    pinMode(pot4, INPUT);  // Потенциометр 4

    // Сообщение о версии прошивки
    lcd.clear();                          // Очистка экрана
    centerText("Firmware version", 0);    // Текст на строке 0
    centerText("26052026", 1);            // Версия на строке 1
    delay(2000);                          // Задержка
    lcd.clear();                          // Очистка экрана

    // Начальная разрядка конденсаторов
    dischargeCapacitors();
}

// -----------------------------
// Основной цикл программы
// -----------------------------
void loop() {
    // Всегда читаем текущие значения задержек
    readPotentiometers();
    
    // Показываем текущие задержки на дисплее
    displayCurrentDelays();
    
    // Проверяем состояние кнопки выстрела
    if (digitalRead(shtButton) == LOW) {
        delay(50); // Дебаунс
        if (digitalRead(shtButton) == LOW) {
            // Если зарядка не начата и выстрел не готов
            if (!charging && !readyToFire) {
                startCharging();             // Начинаем зарядку
            } else if (readyToFire) {
                fireSequence();              // Выполняем выстрел
            }
            while (digitalRead(shtButton) == LOW); // Ждем отпускания
        }
    }
    
    delay(10); // Небольшая задержка для стабильности
}

// -----------------------------
// Чтение значений с потенциометров
// -----------------------------
void readPotentiometers() {
    delay1 = map(analogRead(pot1), 0, 1023, 0, 3000);  // 0-3000 мкс
    delay2 = map(analogRead(pot2), 0, 1023, 0, 3000);  // 0-3000 мкс
    delay3 = map(analogRead(pot3), 0, 1023, 0, 3000);  // 0-3000 мкс
    delay4 = map(analogRead(pot4), 0, 1023, 0, 3000);  // 0-3000 мкс
}

// -----------------------------
// Отображение текущих задержек на дисплее
// -----------------------------
void displayCurrentDelays() {
    lcd.clear();
    
    // Первая строка: задержки 1 и 2
    lcd.setCursor(0, 0);
    lcd.print("D1:");
    lcd.print(delay1);
    lcd.print("D2:");
    lcd.print(delay2);
    
    // Вторая строка: задержки 3 и 4
    lcd.setCursor(0, 1);
    lcd.print("D3:");
    lcd.print(delay3);
    lcd.print("D4:");
    lcd.print(delay4);
}

// -----------------------------
// Функция разрядки конденсаторов
// -----------------------------
void dischargeCapacitors() {
    lcd.clear();                          // Очистка экрана
    centerText("Discharging", 0);    // Сообщение о разрядке
    centerText("capacitors...", 1);

    // Включаем все транзисторы для разрядки
    digitalWrite(trnPin1, HIGH);
    digitalWrite(trnPin2, HIGH);
    digitalWrite(trnPin3, HIGH);
    digitalWrite(trnPin4, HIGH);
    digitalWrite(trnPin5, HIGH);

    delayMicroseconds(500);  // Задержка для разрядки

    // Закрываем транзисторы
    digitalWrite(trnPin1, LOW);
    digitalWrite(trnPin2, LOW);
    digitalWrite(trnPin3, LOW);
    digitalWrite(trnPin4, LOW);
    digitalWrite(trnPin5, LOW);

    // Завершаем разрядку
    delay(500);                           // Задержка для завершения
    lcd.clear();                           // Очистка экрана
    centerText("Ready!", 0);                // Сообщение о готовности
}

// -----------------------------
// Функция начала зарядки конденсаторов
// -----------------------------
void startCharging() {
    charging = true;                      // Устанавливаем флаг зарядки
    lcd.clear();                          // Очистка экрана
    centerText("Charging...", 0);         // Сообщение о зарядке

    digitalWrite(chrgTrans, HIGH);        // Запуск зарядки

    // Анимация прогресса заряда
    for (int i = 0; i <= 16; i++) {
        lcd.setCursor(0, 1);
        lcd.print("[");
        for (int j = 0; j < 16; j++) {
            if (j < i) lcd.print("#");
            else lcd.print(" ");
        }
        lcd.print("]");
        delay(400); // Общее время зарядки ~6 сек
    }

    digitalWrite(chrgTrans, LOW);         // Завершение зарядки
    charging = false;                     // Сбрасываем флаг зарядки
    readyToFire = true;                   // Устанавливаем флаг готовности
    
    lcd.clear();
    centerText("Ready!", 0);     // Сообщение о завершении
    lcd.setCursor(0, 1);
    centerText("Press to FIRE!", 1);
}

// -----------------------------
// Функция выполнения выстрела
// -----------------------------
void fireSequence() {
    lcd.clear();
    centerText("Firing...", 0);           // Сообщение о выстреле

    // Катушка 1 - включаем сразу
    digitalWrite(trnPin1, HIGH);
    delayMicroseconds(pulseDuration);
    digitalWrite(trnPin1, LOW);
    
    // Задержка перед катушкой 2
    delayMicroseconds(delay1);
    
    // Катушка 2
    digitalWrite(trnPin2, HIGH);
    delayMicroseconds(pulseDuration);
    digitalWrite(trnPin2, LOW);
    
    // Задержка перед катушкой 3
    delayMicroseconds(delay2);
    
    // Катушка 3
    digitalWrite(trnPin3, HIGH);
    delayMicroseconds(pulseDuration);
    digitalWrite(trnPin3, LOW);
    
    // Задержка перед катушкой 4
    delayMicroseconds(delay3);
    
    // Катушка 4
    digitalWrite(trnPin4, HIGH);
    delayMicroseconds(pulseDuration);
    digitalWrite(trnPin4, LOW);
    
    // Задержка перед катушкой 5
    delayMicroseconds(delay4);
    
    // Катушка 5
    digitalWrite(trnPin5, HIGH);
    delayMicroseconds(pulseDuration);
    digitalWrite(trnPin5, LOW);
    
    // Завершение выстрела
    readyToFire = false;
    
    lcd.clear();
    centerText("Shot complete!", 0);       // Сообщение о завершении
    delay(1000);
    
    // Автоматическая разрядка после выстрела
    dischargeCapacitors();
}

// -----------------------------
// Функция центрирования текста
// -----------------------------
void centerText(const char* text, int row) {
    int len = strlen(text);              // Длина текста
    int pos = (16 - len) / 2;            // Вычисление позиции
    lcd.setCursor(pos, row);             // Установка курсора
    lcd.print(text);                     // Вывод текста
}