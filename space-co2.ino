#include <Arduino_LPS22HB.h>
#include <U8x8lib.h>
#include "src/MHZ.h"
#include <string>

U8X8_SSD1309_128X64_NONAME0_4W_HW_SPI u8x8(/* cs=*/ 8, /* dc=*/ 9, /* reset=*/ 10);

MHZ co2sensor(&Serial1, MHZ19B);

const int BUTTON_PIN = 2; // the number of the pushbutton pin
const int calibrate_button_time = 7;

arduino::String latestPPM = ""; // Keep track of the last known co2 reading
arduino::String latestPressure = "";
arduino::String latestTemp = "";
arduino::String concen_scaler_disp = "";
arduino::String PPM_Scaled_str = "";

int button_state;
int calibrate_count = 0;
float reference_pressure = 101.325;
float current_pressure = 101.325;
float concentration_scaler = 1;
int PPM_scaled;
int PPM_raw;

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

  // Ensure the calibration is NOT automatic and then calibrate from given environment. WILL BE CHANGED LATER.
  //co2sensor.setAutoCalibrate(false);
  //co2sensor.calibrateZero();

  //Set digital pin pull up resistor mode
  pinMode(2, INPUT_PULLUP);
}

void loop() {
 
  // Check if the calibration button is pressed for seven cycles, and then calibrate CO2 meter and record current barometric
  // pressure for concentration scaler
  button_state = digitalRead(BUTTON_PIN);
  if(button_state == LOW){
    Serial.println("Button State Low");
    if(calibrate_count < calibrate_button_time){
      calibrate_count ++;
  }}
  else if (button_state == HIGH) {
    Serial.println("Button State High");
    calibrate_count = 0;
  }
  if(calibrate_count == calibrate_button_time){
    co2sensor.calibrateZero();
    current_pressure = BARO.readPressure();
    //reference_pressure = arduino::String(BARO.readPressure());
  }


  // CO2 STUFFS
  if (co2sensor.isPreHeating()) {
    // Here display "preheat" on screen until we have a value.
    latestPPM = "Preheating";
  } else if (co2sensor.isReady()) {
    int co2_ppm = co2sensor.readCO2UART();
    
    // Setup screen for new value
    u8x8.clear();
    if (co2_ppm >= 0) {
      // If reading is successful, store latest reading for printing until next positive value.
      latestPPM = "";
      latestPPM += co2_ppm;
      latestPPM += "ppm";
      PPM_raw = co2_ppm;
    } else {
      // Otherwise, print error code
      Serial.print("No New Valid CO2 values: ");
      Serial.println(co2_ppm);
    }
  }

  // Always log latest reading.
  Serial.print("Latest co2 concentration: ");
  Serial.println(latestPPM);

  // BAROMETER STUFFS

  //float pressure = BARO.readPressure();
  latestPressure = arduino::String(BARO.readPressure());
  latestPressure += " kPa";

  Serial.print("Pressure = ");
  Serial.println(latestPressure);

  latestTemp = arduino::String(BARO.readTemperature());
  latestTemp += " C";

  Serial.print("Temperature = ");
  Serial.println(latestTemp);


  concentration_scaler = reference_pressure/current_pressure;
  Serial.print("Concentration scaler = ");
  Serial.println(concentration_scaler);
  Serial.println();
  std::string concen_scaler_disp = std::to_string(concentration_scaler);
  concen_scaler_disp += " Scaler";

  PPM_scaled = PPM_raw * concentration_scaler;
  std::string PPM_scaled_str = std::to_string(PPM_scaled);
  PPM_scaled_str += "PPMS";


  // only run loop once every second
  delay(1000);

    // Print to display
  //u8x8.clear();
  u8x8.drawString(0, 0, latestPPM.c_str());
  u8x8.drawString(0, 1, latestPressure.c_str());
  u8x8.drawString(0, 2, latestTemp.c_str());
  u8x8.drawString(0, 3, concen_scaler_disp.c_str());
  u8x8.drawString(0, 4, PPM_scaled_str.c_str());
}
