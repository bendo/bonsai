#include <RTClib.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_GFX.h>
#include <Adafruit_MAX1704X.h>
#include <Adafruit_SH110X.h>

Adafruit_AHTX0 dht;
Adafruit_MAX17048 maxlipo;
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

// OLED FeatherWing buttons map
#define BUTTON_A  15
#define BUTTON_B  32
#define BUTTON_C  14

// battery pin
#define VBATPIN A13

// light sensor pin
#define LIGHTPIN A0

RTC_PCF8523 rtc;

constexpr int DISPLAY_ON_LIMIT = 200;

int display_count = DISPLAY_ON_LIMIT;

void setup() {
	Serial.begin(115200);
	while (!Serial) delay(10);

	Serial.println("---------- Boot Start ----------");
	delay(250); // wait for the OLED to power up
	display.begin(0x3C, true); // Address 0x3C default
	Serial.println("OLED OK");

	// Show image buffer on the display hardware.
	// Since the buffer is intialized with an Adafruit splashscreen
	// internally, this will display the splashscreen.
	// TODO: maybe use custom splashscreen
	display.display();
	delay(1000);

	// Clear the buffer.
	display.clearDisplay();
	display.display();

	display.setRotation(1);
	display.setTextSize(1);
	display.setTextColor(SH110X_WHITE);
	display.setCursor(0,0);

	pinMode(BUTTON_A, INPUT_PULLUP);
	pinMode(BUTTON_B, INPUT_PULLUP);
	pinMode(BUTTON_C, INPUT_PULLUP);

	// Initialize inside the box temperature and humidity sensor
	if (!dht.begin()) {
		Serial.println(F("Couldn't find DHT.\tCheck wiring"));
		display.println("DHT20           ERROR");
		display.display();
		while (1) delay(10);
	}
	Serial.println("DHT20 OK");
	display.println("DHT20              OK");
	display.display();

	// Initialize battery health monitoring chip
	while (!maxlipo.begin()) {
		Serial.println(F("Couldn't find Adafruit MAX17048.\nMake sure a battery is plugged in!"));
		display.println("MAX17048        ERROR");
		display.setCursor(0,8);
		display.display();
		delay(2000);
	}
	Serial.println("MAX17048 OK");
	Serial.print(F("MAX17048 Chip ID: 0x"));
	Serial.println(maxlipo.getChipID(), HEX);
	display.fillRect(0, 8, 128, 16, SH110X_BLACK);
	display.println("MAX17048           OK");
	display.display();

	// Initialize RTC chip
	if (!rtc.begin()) {
		Serial.println(F("Couldn't find RTC."));
		display.println("RTC             ERROR");
		display.setCursor(0,16);
		display.display();
		while (1) delay(10);
	}
	Serial.println("RTC OK");
	display.fillRect(0, 16, 128, 24, SH110X_BLACK);
	display.println("RTC                OK");
	display.display();

	if (!rtc.initialized() || rtc.lostPower()) {
		Serial.println("RTC is NOT initialized, let's set the time!");
		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
	}
	
	rtc.start();

	// Hibernate
	maxlipo.hibernate();

	delay(5000);
	display.clearDisplay();
	display.display();
	Serial.println("----------- Boot End -----------");
}

void loop() {
	if(!digitalRead(BUTTON_A)) {
		display_count = 0;
		maxlipo.wake();
	}
	// TODO: maybe add menu to format sd card, to empty barrel, to watter plants now
	// TODO when holding button for more then 5 seconds switch to control menu
	// TODO short press to move to next item in menu, 3 second press to submit selected choise from menu
	if(!digitalRead(BUTTON_B)) display.print("B");
	if(!digitalRead(BUTTON_C)) display.print("C");
	delay(10);
	yield();
	if (display_count < DISPLAY_ON_LIMIT) {
		printScreen();
		display_count++;
		if (display_count >= DISPLAY_ON_LIMIT) {
			maxlipo.hibernate();
			display.clearDisplay();
			display.display();
		}
	}
	// Serial.print("MAX17048 is hibernating: ");
	// Serial.println(maxlipo.isHibernating());
	display.display();
}

float printVin() {
	float measuredvbat = analogReadMilliVolts(VBATPIN);
	measuredvbat *= 2;    // we divided by 2, so multiply back
	measuredvbat /= 1000; // convert to volts!
	return measuredvbat;
}

void printScreen() {
	// RTC
	DateTime now = rtc.now();
	// Inside termometer PHT20
	sensors_event_t humidity, temp;
	dht.getEvent(&humidity, &temp);// populate temp and humidity objects
	// OLED
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(SH110X_WHITE);
	display.setCursor(0,0);
	// Display current time
	display.print(now.day(), DEC);
	display.print(".");
	display.print(now.month(), DEC);
	display.print(".");
	display.print(now.year(), DEC);
	display.print(" - ");
	display.print(now.hour(), DEC);
	display.print(":");
	display.print(now.minute(), DEC);
	display.print(":");
	display.print(now.second(), DEC);
	display.println();
	// Display input voltage
	display.print("VIN:");
	display.print(printVin());
	display.print("V");
	display.println();
	// Display temp and humidity inside the box
	display.print("BOX: T:");
	display.print(temp.temperature);
	display.print(" H:");
	display.print(humidity.relative_humidity);
	display.println();
	// Display voltage of the battery
	display.print("BAT:");
	display.print(maxlipo.cellVoltage());
	display.print("V ");
	display.print(maxlipo.cellPercent());
	display.print("%");
	display.println();
	// Display data from light sensor - PT19
	int light_value = analogRead(LIGHTPIN);
	display.print("Light: ");
	display.print(light_value);
	display.print(" - ");
	display.print((int)(light_value * 0.0245));
	display.print("%");
	display.println();
	display.display(); // actually display all of the above
}
