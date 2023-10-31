#ifndef MHZ_H
#define MHZ_H

#include "Arduino.h"


// types of sensors.
extern const int MHZ14A;
extern const int MHZ19B;
extern const int MHZ19C;
extern const int MHZ_2K;
extern const int MHZ_5k;
extern const int MHZ_10K;
// status codes
extern const int STATUS_NO_RESPONSE;
extern const int STATUS_CHECKSUM_MISMATCH;
extern const int STATUS_INCOMPLETE;
extern const int STATUS_NOT_READY;

enum Ranges {RANGE_2K = 2000, RANGE_5K = 5000};

class MHZ {
 public:
  MHZ(arduino::Stream * serial, uint8_t type);

  void setDebug(boolean enable);

  boolean isPreHeating();
  boolean isReady();
  void setAutoCalibrate(boolean b);
  void calibrateZero();
  void setRange(int range);
 // void calibrateSpan(int range); //only for professional use... see implementation and Dataheet.

  int readCO2UART();
  int getLastTemperature();

 private:
  uint8_t _type, temperature;
  Ranges _range = RANGE_5K;
  boolean debug = false;

  arduino::Stream * _serial;
  byte getCheckSum(byte *packet);

  unsigned long lastRequest = 0;

  bool SerialConfigured = true;
};

#endif
