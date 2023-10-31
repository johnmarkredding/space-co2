#include <Arduino_LPS22HB.h>
#include <U8x8lib.h>
#include "src/MHZ.h"

U8X8_SSD1309_128X64_NONAME0_4W_HW_SPI u8x8(/* cs=*/ 8, /* dc=*/ 9, /* reset=*/ 10);

MHZ co2sensor(&Serial1, MHZ19B);

arduino::String latestPPM = ""; // Keep track of the last known co2 reading

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  while (!Serial);
  while (!Serial1);
  
  if (!BARO.begin()) {
    Serial.println("Failed to initialize pressure sensor!");
    while (1);
  }

  // Setup Display
  u8x8.begin();
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
}

void loop() {
  // CO2 STUFFS

  if (co2sensor.isPreHeating()) {
    // Here display "preheat" on screen until we have a value.
    latestPPM = "Preheating";
  } else if (co2sensor.isReady()) {
    int co2_ppm = co2sensor.readCO2UART();
    if (co2_ppm >= 0) {
      // If reading is successful, store latest reading for printing until next positive value.
      latestPPM = "";
      latestPPM += co2_ppm;
      latestPPM += "ppm";
    } else {
      // Otherwise, print error code
      Serial.print("Error reading CO2 values: ");
      Serial.println(co2_ppm);
    }
  }

  // Always log latest reading.
  Serial.print("Latest co2 concentration: ");
  Serial.println(latestPPM);

  // Print to display
  u8x8.drawString(0, 0, latestPPM.c_str());


  // BAROMETER STUFFS

  float pressure = BARO.readPressure();

  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" kPa");

  float temperature = BARO.readTemperature();

  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" C");
  Serial.println();

  // only run loop once every second
  delay(1000);
}