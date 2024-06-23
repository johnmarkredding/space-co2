#include <Arduino_LPS22HB.h>
#include <U8x8lib.h>
#include "src/MHZ.h"
#include <string>
#include <at24c02.h>

U8X8_SSD1309_128X64_NONAME0_4W_HW_SPI u8x8(/* cs=*/ 8, /* dc=*/ 9, /* reset=*/ 10);

MHZ co2sensor(&Serial1, MHZ19B);

bool CO2_ALARM = false; //marks state of excess CO2 for sounding alarm, etc.
bool ALARM_CANCEL = false; //marks state of alarm cancellation
const int BUTTON_PIN = 2; // the number of the pushbutton pin
const int BUZZER_PIN = 3; // the number of the buzzer pin
const int ALARM_CANCEL_PIN = 4; // the number of the button to cancel CO2 level alarm
const int calibrate_button_time = 7;

arduino::String latestPPM = ""; // Keep track of the last known co2 reading
arduino::String latestPressure = "";
arduino::String latestTemp = "";
arduino::String concen_scaler_disp = "";
arduino::String PPM_Scaled_str = "";
arduino::String calibrated_ind = "";
arduino::String reference_pressure_disp = "";

int button_state;
int cancel_button_state;
int calibrate_count = 0;
float reference_pressure = 101.325;
float current_pressure = 101.325;
float concentration_scaler = 1;
int PPM_scaled;
int PPM_raw;
int calibrated_ind_count;
int cal_ind_disp_lngth = 5;
int CO2_Concen_Threshold = 1000; //Set CO2 alarm concentration threshold here.
int refresh_rate = 1000;
int ALARM_TIME = 60; //Set alarm cancel duration here.
int ALARM_COUNT = ALARM_TIME;


AT24C02 eprom(AT24C_ADDRESS_0);

void setup() {
  
  digitalWrite(BUZZER_PIN, HIGH);  // sets BUZZER_PIN HIGH to turn off piezo buzzer
  
  Serial.begin(9600);
  Serial1.begin(9600);

 // while (!Serial);
 // while (!Serial1);
  
  // Set up AT24C02 EEPROM

  Wire.begin();
  uint8_t data = eprom.read(0);
// Check for error
  eprom.get(0, reference_pressure);
  Serial.print(reference_pressure);
  if (eprom.getLastError() != 0) {
  Serial.print("Error reading from eeprom");
  u8x8.drawString(0, 4, "EEPROM Error");
  } 


// Initialize barometer
  if (!BARO.begin()) {
    Serial.println("Failed to initialize pressure sensor!");
    while (1);
  }

  // Setup Display
  u8x8.begin();
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);

  // Ensure the calibration is NOT automatic and then calibrate from given environment. WILL BE CHANGED LATER.
  co2sensor.setAutoCalibrate(false);
  //co2sensor.calibrateZero();

  //Set digital pin pull up resistor mode
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ALARM_CANCEL_PIN, INPUT_PULLUP);
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
    reference_pressure = BARO.readPressure();
    calibrated_ind = "Calibrated";
    calibrated_ind_count = cal_ind_disp_lngth;
    eprom.put(0, reference_pressure);
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
  current_pressure = (BARO.readPressure());

  Serial.print("Pressure = ");
  Serial.println(latestPressure);

  latestTemp = arduino::String(BARO.readTemperature());
  latestTemp += " C";

  Serial.print("Temperature = ");
  Serial.println(latestTemp);

// Calculate CO2 concentration scaler to compensate for changes in barometric pressure.
  concentration_scaler = reference_pressure/current_pressure;

  Serial.print("Concentration scaler = ");
  Serial.println(concentration_scaler);
  Serial.println();
  std::string concen_scaler_disp = std::to_string(concentration_scaler);
  concen_scaler_disp += " Scaler";
  std::string reference_pressure_disp = std::to_string(reference_pressure);

//Scale the CO2 measurement to compensate for changes in pressure, and convert it to a string for display.
  PPM_scaled = PPM_raw * concentration_scaler;
  std::string PPM_scaled_str = std::to_string(PPM_scaled);
  PPM_scaled_str += "ppms";

// If calibration has occured, decrement the counter for how long to display "calibrated" on the screen. Once time out, set to blank. 
if (calibrated_ind_count >= 1){
 calibrated_ind_count --;
}
 if (calibrated_ind_count < 1){
  calibrated_ind = "          ";
 }

  reference_pressure_disp += " REF";

  //Handle alarm conditions for excess CO2 concentration
  //Check if CO2 level is above maximum safe threshold.
  if(PPM_scaled >= CO2_Concen_Threshold){
    CO2_ALARM = true;
  }
// Reset CO2 alarm if level falls below threshold
  if(PPM_scaled < CO2_Concen_Threshold){
    CO2_ALARM = false;
  }
// Start Buzzer Alarm
if (CO2_ALARM == true && ALARM_CANCEL == false){
  digitalWrite(BUZZER_PIN, LOW);
    Serial.print("Alarm on and not cancelled");
}
else {digitalWrite(BUZZER_PIN, HIGH);} //turns off alarm automatically if concentration drops below threshold

//Check for alarm cancel button push
cancel_button_state = digitalRead(ALARM_CANCEL_PIN);
Serial.print("Alarm Cancel button State: ");
Serial.println(cancel_button_state);
if(cancel_button_state == LOW){
ALARM_CANCEL = true;
    Serial.println("Alarm cancelled");
}


if (ALARM_CANCEL == true && CO2_ALARM == true){
digitalWrite(BUZZER_PIN, HIGH);
ALARM_COUNT --;
    Serial.print("ALARM_COUNT :");
    Serial.println(ALARM_COUNT);
}
if (ALARM_COUNT <= 0){
  ALARM_CANCEL = false;
  ALARM_COUNT = ALARM_TIME;
  Serial.print("Alarm cancellation ended");
}

  // only run loop once every second
  delay(refresh_rate);

    // Print to display
  //u8x8.clear();
  u8x8.drawString(0, 0, latestPPM.c_str());
  u8x8.drawString(0, 1, PPM_scaled_str.c_str());
  u8x8.drawString(0, 2, latestPressure.c_str());
  u8x8.drawString(0, 3, reference_pressure_disp.c_str());
  u8x8.drawString(0, 4, latestTemp.c_str());
  u8x8.drawString(0, 7, calibrated_ind.c_str());

 // u8x8.drawString(0, 4, concen_scaler_disp.c_str());

 //buzz for 1/8 of a second
  //digitalWrite(BUZZER_PIN, LOW); // sets the digital pin 13 on
  //delay(125);            // waits for a second



}
