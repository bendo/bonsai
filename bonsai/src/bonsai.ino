#include <SPI.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_AHTX0 dht;
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

// OLED FeatherWing buttons map
#define BUTTON_A  15
#define BUTTON_B  32
#define BUTTON_C  14

// battery pin
#define VBATPIN A13

constexpr int DISPLAY_ON_LIMIT = 200;

int display_count = DISPLAY_ON_LIMIT;

void setup() {
	Serial.begin(115200);

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
	display.setTextSize(1);
	display.setTextColor(SH110X_WHITE);
	display.setCursor(0,0);

	Serial.println("Button test");

	pinMode(BUTTON_A, INPUT_PULLUP);
	pinMode(BUTTON_B, INPUT_PULLUP);
	pinMode(BUTTON_C, INPUT_PULLUP);

	if (!dht.begin()) {
		Serial.println("Could not find DHT? Check wiring");
		display.println("DHT20 issue");
		display.display();
		while (1) delay(10);
	}
	Serial.println("DHT20 OK");
	display.println("DHT20              OK");

	display.display();
	delay(5000);
	display.clearDisplay();
	display.display();
}

void loop() {
	if(!digitalRead(BUTTON_A)) display_count = 0;
	if(!digitalRead(BUTTON_B)) display.print("B");
	if(!digitalRead(BUTTON_C)) display.print("C");
	delay(10);
	yield();
	if (display_count < DISPLAY_ON_LIMIT) {
		printScreen();
		display_count++;
		if (display_count >= DISPLAY_ON_LIMIT) {
			display.clearDisplay();
			display.display();
		}
	}
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
	dht.getEvent(&humidity, &temp);// populate temp and humidity objects
	// OLED
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(SH110X_WHITE);
	display.setCursor(0,0);
	display.print("VIN:");
	display.print(printBat());
	display.print("V");
	display.println();
	display.print("BOX: T:");
	display.print(temp.temperature);
	display.print(" H:");
	display.print(humidity.relative_humidity);
	display.display(); // actually display all of the above
}
