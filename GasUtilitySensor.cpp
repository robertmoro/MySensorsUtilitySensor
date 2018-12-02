#include "GasUtilitySensor.h"

GasUtilitySensor::GasUtilitySensor(int ledPin, int sensorPin)
  : UtilitySensor(ledPin, sensorPin)
{
}

unsigned long GasUtilitySensor::liters()
{
  return getCounter() - m_oldCounter;
}

double GasUtilitySensor::litersPerMinute()
{
  return 0;
}

double GasUtilitySensor::milliToMinute(unsigned long milliseconds)
{
  return 1.0 * milliseconds / 60000.0;
}

double GasUtilitySensor::litersPerMinute(unsigned long millisPassed, double liters)
{
  return 1.0 * liters / milliToMinute(millisPassed);
}
