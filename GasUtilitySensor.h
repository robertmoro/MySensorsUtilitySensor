#ifndef GasUtilitySensor_h
#define GasUtilitySensor_h

#include <Arduino.h>

#include "UtilitySensor.h"

class GasUtilitySensor : public UtilitySensor
{
  public:
    GasUtilitySensor(int ledPin, int sensorPin);
    double litersPerMinute();
    unsigned long liters();

  private:
    double milliToMinute(unsigned long milliseconds);
    double litersPerMinute(unsigned long millisPassed, double liters);
};

#endif // GasUtilitySensor_h
