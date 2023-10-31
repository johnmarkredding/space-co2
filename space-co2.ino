#include <Arduino.h>
#include <Arduino_LPS22HB.h>
#include "src/MHZ.h"

MHZ co2sensor(&Serial1, MHZ19B);
arduino::String latestPPM = "";

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  while (!Serial);
  while (!Serial1);

  co2sensor.setAutoCalibrate(true);
  co2sensor.calibrateZero();
  
  if (!BARO.begin()) {
    Serial.println("Failed to initialize pressure sensor!");
    while (1);
  }
}

void loop() {
  
  // CO2 STUFFS
  // Read CO2 concentration via UART

  if (co2sensor.isPreHeating()) {
    // Here display "preheat" on screen until we have a value.
    latestPPM = "Preheating";
  } else if (co2sensor.isReady()) {
    int co2_ppm = co2sensor.readCO2UART();
    if (co2_ppm >= 0) {
      // If reading is successful, set latest reading
      latestPPM = "";
      latestPPM += co2_ppm;
      latestPPM += "ppm";
    } else {
      Serial.print("Error reading CO2 values: ");
      Serial.println(co2_ppm);
    }
  }

    // Always print latest reading.
    Serial.print("Latest co2 concentration: ");
    Serial.println(latestPPM);

  // BAROMETER STUFFS

  // read the sensor value
  float pressure = BARO.readPressure();

  // print the sensor pressure
  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" kPa");

  float temperature = BARO.readTemperature();

  // print the sensor temp
  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" C");

  // print an empty line
  Serial.println();

  // wait 1 second to print again
  delay(1000);
}
