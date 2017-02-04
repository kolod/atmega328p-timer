#include <CRC32.h>
#include <TimerOne.h>
#include "key.h"

#define EEPROM_ADDR  0

#define SEG_A        1
#define SEG_B        2
#define SEG_C       15
#define SEG_D       14
#define SEG_E        4
#define SEG_F       16
#define SEG_G       10
#define SEG_DP       3

#define COLUMN_1    19
#define COLUMN_2    18
#define COLUMN_3    17

#define KEY_TOP      9
#define KEY_BOTTOM   8

#define RELAY        0

// Глобальные переменные
boolean isAutoShift = true;
boolean isTimerRunning = false;
boolean isTimerPaused = false;
volatile boolean save = false;

int milliseconds  = 0;

byte column   = 0; // 
byte offset   = 0;
byte cursor   = 0;
byte blink1   = 0;
byte blink2   = 0;
byte blinking = 0;

byte setpoint[5];
byte time[5];

const int segments[] = {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G};
const int columns[] = {COLUMN_3, COLUMN_2, COLUMN_1};

// Чтение уставки времени из энергонезависимой памяти
void ReadSettings() {
	byte default_data[5] = {0, 1, 0, 0, 0};
	byte temp[5 + 4];

	eeprom_read_block(temp, (void*) EEPROM_ADDR, sizeof(temp));

	CRC32 crc;
	crc.update(temp, sizeof(temp) - 4);
	if (crc.finalize() == *((unsigned long*) &temp[sizeof(temp) - 4])) {
		memcpy(setpoint, temp, sizeof(setpoint));
	} else {
		memcpy(setpoint, default_data, sizeof(setpoint));
		WriteSettings();
	}
}

// Запись уставки времени в энергонезависимую память
void WriteSettings() {
	byte temp[5 + 4];

	memcpy(temp, setpoint, sizeof(setpoint));

	CRC32 crc;
	crc.update(temp, sizeof(temp) - 4);
	*((unsigned long*) &temp[sizeof(temp) - 4]) = crc.finalize();

	eeprom_update_block(temp, (void*) EEPROM_ADDR, sizeof(temp));
}

// Обратный отсчёт
void countDown(int pos) {
	if (--time[pos] == 0xFF) {
		time[pos] = (pos == 3) ? 5 : 9;
		if (--pos >= 0) countDown(pos);
	}
}

void autoShift() {
	if (isAutoShift) {
		while ((time[offset] == 0) && (offset < 2)) {
			offset++;
		}
	}
}

boolean isTimeElapsed() {
	return !time[0] && !time[1] && !time[2] && !time[3] && !time[4];
}

void incrementSetpoint(int pos) {
	if (++setpoint[pos] > ((pos == 3) ? 5 : 9)) {
		setpoint[pos] = 0;
	}
}

void decrementSetpoint(int pos) {
	if (--setpoint[pos] == 0xFF) {
		setpoint[pos] = (pos == 3) ? 5 : 9;
	}
}

void onLeftPressed() {
	if (isTimerRunning) {
		isAutoShift = false;
		if (offset > 0) offset--;
		cursor = 0;
	} else {
		if (cursor > 0) {
			cursor--;
		} else if (offset > 0) {
			offset--;
		}
	}
}

void onRightPressed() {
	if (isTimerRunning) {
		isAutoShift = false;
		if (offset < 2) offset++;
		cursor = 0;
	} else {
		if (cursor < 2) {
			cursor++;
		} else if (offset < 2) {
			offset++;
		}
	}
}

void onPlusPressed() {
	if (!isTimerRunning) {
		incrementSetpoint(cursor + offset);
	}
}

void onMinusPressed() {
	if (!isTimerRunning) {
		decrementSetpoint(cursor + offset);
	}
}

void onStartPressed() {
	digitalWrite(RELAY, HIGH);

	if (isTimerPaused) {
		isTimerPaused = false;
		cursor = 0;
		offset = 0;
	} else if (!isTimerRunning) {
		memcpy(time, setpoint, sizeof(setpoint));
		countDown(4);
		isTimerRunning = true;
		milliseconds = 0;
		isAutoShift = true;
		cursor = 0;
		offset = 0;
		autoShift();
	}
}

void onStopPressed() {
	digitalWrite(RELAY, LOW);

	if (isTimerRunning) {
		if (isTimerPaused) {
			isTimerRunning = false;
			isTimerPaused = false;
		} else {
			isTimerPaused = true;
		}
	}
}

void onStopLongPressed() {
	if (!isTimerRunning) {
		blinking = 12;
		blink1 = 0;
		save = true;
	}
}

Key left(250, onLeftPressed, onLeftPressed);
Key right(250, onRightPressed, onRightPressed);
Key plus(250, onPlusPressed, onPlusPressed);
Key minus(250, onMinusPressed, onMinusPressed);
Key start(250, onStartPressed);
Key stop(500, onStopPressed, nullptr, onStopLongPressed);

// Показать цифру/курсор
void showDigit(byte value) {

	const byte digits[11] = {
		0b00111111,  // 0
		0b00000110,  // 1
		0b01011011,  // 2
		0b01001111,  // 3
		0b01100110,  // 4
		0b01101101,  // 5
		0b01111101,  // 6
		0b00000111,  // 7
		0b01111111,  // 8
		0b01101111,  // 9
		0b00000000   // 
	};

	for (int i = 0; i < 7; i++) {
		digitalWrite(segments[i], ((1 << i) & digits[value]) ? HIGH : LOW);
	}
}

// Показать/скрыть точку
void showPoint(boolean value) {
	digitalWrite(SEG_DP, value ? HIGH : LOW);
}

// Прерывание вызываемое каждую миллисекунду
void TimerISR() {

	// Снимаем сигнал выбора столбца
	digitalWrite(columns[column], LOW);

	// Переходим к следующему столбцу
	column++;
	if (column > 2) column = 0;

	byte position = column + offset;

	if (isTimerRunning) {
		// Таймер в режиме обратного отсчёта
		showDigit(time[position]);

		if (isTimerPaused) {
			showPoint(position == 2);
		} else {
			showPoint((position == 2) && (milliseconds < 500));
			milliseconds++;
			if (milliseconds > 999) {
				milliseconds = 0;
				countDown(4);
				autoShift();
				if (isTimeElapsed()) {
					isTimerRunning = false;
					digitalWrite(RELAY, LOW);
				}
			}
		}
	} else {
		// Таймер в режиме настройки
		if (blinking > 0) {
			blink1++;
			if (blink1 < 50) {
				showDigit(10);
				showPoint(false);
			} else {
				showDigit(setpoint[position]);
				showPoint(position == 2);
				if (blink1 >= 100) {
					blink1 = 0;
					blinking--;
				}
			}
		} else if (column == cursor) {
			showPoint(position == 2);
			blink2++;
			if (blink2 < 100) {
				showDigit(10);
			} else {
				showDigit(setpoint[position]);
				if (blink2 >= 200) blink2 = 0;
			} 
		} else {
			showPoint(position == 2);
			showDigit(setpoint[position]);
		}
	}

	// Взводим сигнал выбора колонки
	digitalWrite(columns[column], HIGH);

	// Небольшая задержка времени
	_NOP();
	_NOP();
	_NOP();

	// Читаем состояние клавиш
	boolean topButton = digitalRead(KEY_TOP);
	boolean bottomButton = digitalRead(KEY_BOTTOM);

	switch (column) {
	case 0:
		start.update(topButton);
		stop.update(bottomButton);
		break;

	case 1:
		plus.update(topButton);
		right.update(bottomButton);
		break;

	case 2:
		minus.update(topButton);
		left.update(bottomButton);
	}
}

void setup() {

	// Настраиваем порты
	pinMode(SEG_A, OUTPUT);
	pinMode(SEG_B, OUTPUT);
	pinMode(SEG_C, OUTPUT);
	pinMode(SEG_D, OUTPUT);
	pinMode(SEG_E, OUTPUT);
	pinMode(SEG_F, OUTPUT);
	pinMode(SEG_G, OUTPUT);
	pinMode(SEG_DP, OUTPUT);
	pinMode(COLUMN_1, OUTPUT);
	pinMode(COLUMN_2, OUTPUT);
	pinMode(COLUMN_3, OUTPUT);
	pinMode(KEY_TOP, INPUT);
	pinMode(KEY_BOTTOM, INPUT);
	pinMode(RELAY, OUTPUT);

	digitalWrite(SEG_A, LOW);
	digitalWrite(SEG_B, LOW);
	digitalWrite(SEG_C, LOW);
	digitalWrite(SEG_D, LOW);
	digitalWrite(SEG_E, LOW);
	digitalWrite(SEG_F, LOW);
	digitalWrite(SEG_G, LOW);
	digitalWrite(SEG_DP, LOW);
	digitalWrite(COLUMN_1, LOW);
	digitalWrite(COLUMN_2, LOW);
	digitalWrite(COLUMN_3, LOW);
	digitalWrite(RELAY, LOW);

	// Читаем уставку времени
	ReadSettings();

	// Настраиваем прерывание с интервалои 1 мсек
	Timer1.initialize(1000);
	Timer1.attachInterrupt(&TimerISR);
	Timer1.start();
}

void loop() {
	for (;;) if (save) {
		save = false;
		WriteSettings();
	}
}
