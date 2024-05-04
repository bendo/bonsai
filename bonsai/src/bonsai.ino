#include <SPI.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_AHTX0 aht;
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

// OLED FeatherWing buttons map to different pins depending on board:
#if defined(ESP8266)
#define BUTTON_A  0
#define BUTTON_B 16
#define BUTTON_C  2
#elif defined(ESP32) && !defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2)
#define BUTTON_A 15
#define BUTTON_B 32
#define BUTTON_C 14
#elif defined(ARDUINO_STM32_FEATHER)
#define BUTTON_A PA15
#define BUTTON_B PC7
#define BUTTON_C PC5
#elif defined(TEENSYDUINO)
#define BUTTON_A  4
#define BUTTON_B  3
#define BUTTON_C  8
#elif defined(ARDUINO_NRF52832_FEATHER)
#define BUTTON_A 31
#define BUTTON_B 30
#define BUTTON_C 27
#else // 32u4, M0, M4, nrf52840, esp32-s2 and 328p
#define BUTTON_A  9
#define BUTTON_B  6
#define BUTTON_C  5
#endif

// battery pin
#define VBATPIN A13

bool printbat = true;
int loop_count = 0;

void setup() {
	Serial.begin(115200);

	if (!aht.begin()) {
		Serial.println("Could not find AHT? Check wiring");
		while (1) delay(10);
	}
	Serial.println("AHT10 or AHT20 found");

	Serial.println("128x64 OLED FeatherWing test");
	delay(250); // wait for the OLED to power up
	display.begin(0x3C, true); // Address 0x3C default

	Serial.println("OLED begun");

	// Show image buffer on the display hardware.
	// Since the buffer is intialized with an Adafruit splashscreen
	// internally, this will display the splashscreen.
	display.display();
	delay(1000);

	// Clear the buffer.
	display.clearDisplay();
	display.display();

	display.setRotation(1);
	Serial.println("Button test");

	pinMode(BUTTON_A, INPUT_PULLUP);
	pinMode(BUTTON_B, INPUT_PULLUP);
	pinMode(BUTTON_C, INPUT_PULLUP);

	// text display tests
}

void loop() {
	if(!digitalRead(BUTTON_A)) {
		printbat = !printbat;
		delay(250);
	}
	if(!digitalRead(BUTTON_B)) display.print("B");
	if(!digitalRead(BUTTON_C)) display.print("C");

	loop_count++;
	delay(10);
	yield();
	printScreen();
	display.display();
}

float printBat() {
	float measuredvbat = analogReadMilliVolts(VBATPIN);
	measuredvbat *= 2;    // we divided by 2, so multiply back
	measuredvbat /= 1000; // convert to volts!
	return measuredvbat;
}

void printScreen() {
	// PHT20
	sensors_event_t humidity, temp;
	aht.getEvent(&humidity, &temp);// populate temp and humidity objects
	// OLED
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(SH110X_WHITE);
	display.setCursor(0,0);
	if (loop_count < 100) {
		display.print("B:");
		display.print(printBat());
		display.print("V");
		display.print("            ");
		display.println(loop_count);
	} else {
		display.print("T:");
		display.print(temp.temperature);
		display.print(" | ");
		display.print("H:");
		display.print(humidity.relative_humidity);
		display.print("  ");
		display.println(loop_count - 100);
		if (loop_count == 199) {
			loop_count = 0;
		}
	}
	display.println("---------------------");
	display.println("NETWORK: OK");
	display.println("WATER LEVEL: OK");
	display.println("---------------------");
	display.println("FILTER: OFF");
	display.println("THERMO: ON");
	display.println("---------------------");
	display.display(); // actually display all of the above
}
