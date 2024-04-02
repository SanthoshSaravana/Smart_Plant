#include <U8g2lib.h>
#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

const int touchSensor1 = 13;  // Pin connected to TTP224 Channel 1 (Next)
const int touchSensor2 = 27;  // Pin connected to TTP224 Channel 2 (Previous)
const int touchSensor3 = 14;  // Pin connected to TTP224 Channel 3 (Not used in this example)
const int touchSensor4 = 12;  // Pin connected to TTP224 Channel 4 (Not used in this example)
const int WaterLevelPin = 5;
const int soilMoisturePin = 2;
const int WATER_PUMP_PIN = 23; // Pin connected to the water pump
const int SOIL_MOISTURE_THRESHOLD = 50; // Define your soil moisture threshold here

DHT dht(4, DHT11);  // DHT sensor is connected to pin 4
BH1750 lightMeter(0x23);  // I2C address: 0x23

int soilMoistureValue = 0;
float temperature = 0;
float humidity = 0;
float lightIntensity = 0;
int sensorValue = 0;
int waterLevel = 0; // Renamed for clarity
int moisturePercentage = 0;
int currentPage = 1;
unsigned long lastButtonCheckTime = 0;
unsigned long buttonCheckInterval = 1000;  // Check buttons every 1 second
bool button1Pressed = false;
bool button2Pressed = false;
bool isPumpRunning = false; // Keep track of whether the pump is running
unsigned long debounceDelay = 50;  // Button debounce delay

void setup() {
  Serial.begin(115200);

  pinMode(touchSensor1, INPUT_PULLUP);
  pinMode(touchSensor2, INPUT_PULLUP);
  pinMode(touchSensor3, INPUT_PULLUP);
  pinMode(touchSensor4, INPUT_PULLUP);
  pinMode(WATER_PUMP_PIN, OUTPUT); // Set the water pump pin as output

  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  dht.begin();
  lightMeter.begin();
}

void loop() {
  handleButtonsControl();
  handleSoilMoisturePumpControl();
  handleDataOverSerial();
}

void handleButtonsControl() {
  // Check if enough time has passed since the last button check
  if (millis() - lastButtonCheckTime >= buttonCheckInterval) {
    // Update the last button check time
    lastButtonCheckTime = millis();

    // Read the state of each touch sensor
    int state1 = digitalRead(touchSensor1);
    int state2 = digitalRead(touchSensor2);
    
    // Check for Next button press
    if (state1 == LOW && !button1Pressed) {
      button1Pressed = true;
      // Ensure currentPage stays within bounds
      if (currentPage < 5) {
        currentPage++;
      }
    } else if (state1 == HIGH) {
      button1Pressed = false;
    }

    // Check for Previous button press
    if (state2 == LOW && !button2Pressed) {
      button2Pressed = true;
      // Ensure currentPage stays within bounds
      if (currentPage > 1) {
        currentPage--;
      }
    } else if (state2 == HIGH) {
      button2Pressed = false;
    }

    // Display content based on currentPage
    switch (currentPage) {
      case 1:
        displaySoilMoisture();
        break;
      case 2:
        displayTemperature();
        break;
      case 3:
        displayHumidity();
        break;
      case 4:
        displayLightIntensity();
        break;
      case 5:
        displayWaterLevel();
        break;
    }
  }
}

void handleSoilMoisturePumpControl() {
  soilMoistureValue = analogRead(soilMoisturePin);
  moisturePercentage = map(soilMoistureValue, 0, 1023, 0, 100);
  if (moisturePercentage < SOIL_MOISTURE_THRESHOLD && !isPumpRunning) {
    startPump();
  }
  if (moisturePercentage >= SOIL_MOISTURE_THRESHOLD && isPumpRunning) {
  
    stopPump();
  }
}

void startPump() {
  digitalWrite(WATER_PUMP_PIN, HIGH);
  isPumpRunning = true;
}

void stopPump() {
  digitalWrite(WATER_PUMP_PIN, LOW);
  isPumpRunning = false;
}

void displaySoilMoisture() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setCursor(10, 15);
  u8g2.print("Soil Moisture:");
  u8g2.setFont(u8g2_font_ncenB18_tr);
  u8g2.setCursor(45, 50);
  u8g2.print(moisturePercentage);
  u8g2.setFont(u8g2_font_ncenB12_tr);
  u8g2.setCursor(72, 50);
  u8g2.print("%");
  u8g2.sendBuffer();
}

void displayTemperature() {
  temperature = dht.readTemperature();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setCursor(10, 15);
  u8g2.print("Temperature:");
  u8g2.setFont(u8g2_font_ncenB18_tr);
  u8g2.setCursor(30, 50);
  u8g2.print(temperature, 1);
  u8g2.setFont(u8g2_font_ncenB12_tr);
  u8g2.setCursor(78, 50);
  u8g2.print(" °C");
  u8g2.sendBuffer();
}

void displayHumidity() {
  humidity = dht.readHumidity();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setCursor(10, 15);
  u8g2.print("Humidity :");
  u8g2.setFont(u8g2_font_ncenB18_tr);
  u8g2.setCursor(30, 50);
  u8g2.print(humidity, 1);
  u8g2.setFont(u8g2_font_ncenB12_tr);
  u8g2.setCursor(77, 50);
  u8g2.print(" %");
  u8g2.sendBuffer();
}

void displayLightIntensity() {
  lightIntensity = lightMeter.readLightLevel();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setCursor(10, 15);
  u8g2.print("Light Intensity:");
  u8g2.setFont(u8g2_font_ncenB18_tr);
  u8g2.setCursor(15, 50);
  u8g2.print(lightIntensity, 1);
  u8g2.setFont(u8g2_font_ncenB12_tr);
  u8g2.setCursor(90, 50);
  u8g2.print(" lx");
  u8g2.sendBuffer();
}

void displayWaterLevel() {
  sensorValue = analogRead(WaterLevelPin);
  waterLevel = map(sensorValue, 0, 1023, 0, 100); // Rename for clarity
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setCursor(10, 15);
  u8g2.print("Water level:");
  u8g2.setFont(u8g2_font_ncenB18_tr);
  u8g2.setCursor(35, 50);
  u8g2.print(waterLevel);
  u8g2.setFont(u8g2_font_ncenB12_tr);
  u8g2.setCursor(65, 50);
  u8g2.print("%");
  u8g2.sendBuffer();
}

void handleDataOverSerial() {
  Serial.println("Sensor Data:");
  Serial.print("Moisture: ");
  Serial.println(moisturePercentage);
  Serial.print("Water Level: ");
  Serial.println(waterLevel);
  Serial.print("Light Intensity: ");
  Serial.println(lightIntensity);
  Serial.print("Temperature: ");
  Serial.println(temperature);  
  Serial.print("Humidity: ");
  Serial.println(humidity);
}
