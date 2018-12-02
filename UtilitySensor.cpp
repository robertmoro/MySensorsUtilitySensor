#include "UtilitySensor.h"

UtilitySensor::UtilitySensor(int ledPin, int sensorPin)
  : LedPin(ledPin), SensorPin(sensorPin)
{
}

void UtilitySensor::initialize()
{
  //pinMode(SensorPin, INPUT);
  pinMode(SensorPin, INPUT_PULLUP);
  pinMode(LedPin, OUTPUT);

  // set initial LED state
  digitalWrite(LedPin, LOW);
}

void UtilitySensor::handleIrq(unsigned long now)
{
  digitalWrite(LedPin, LOW);
  int sensorReading = digitalRead(SensorPin);

  // Reset the last sensor reading if the value changed, due to noise
  if (sensorReading != m_lastSensorState)
  {
    m_lastSensorReading = now;
  }

  // Whatever the sensor reading is at, it's been there for longer than
  // the time between timer interrupts, take it as the actual current state
  if (((now - m_lastSensorReading) > 0) &&
      (sensorReading != m_sensorState))
  {
    m_sensorState = sensorReading;

    if (sensorReading == HIGH)
    {
      ++m_counter;
      digitalWrite(LedPin, HIGH);
    }
  }

  // Save the sensor reading. Next time through the loop, it'll be the last sensor state
  m_lastSensorState = sensorReading;
}

unsigned long UtilitySensor::getCounter() const
{
  noInterrupts();
  return m_counter;
  interrupts();
}

void UtilitySensor::setCounter(unsigned long counter)
{
  noInterrupts();
  m_counter = counter;
  interrupts();
}

void UtilitySensor::setOldCounter(unsigned long counter)
{
  m_oldCounter = counter;
}

bool UtilitySensor::counterIncremented() const
{
  return getCounter() > m_oldCounter;
}
