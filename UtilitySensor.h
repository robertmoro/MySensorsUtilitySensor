#ifndef UtilitySensor_h
#define UtilitySensor_h

#include <Arduino.h>

class UtilitySensor
{
  // Variables shared by IRQ code and none IRQ code must be volatile
  private:
    volatile const byte LedPin;
    volatile const byte SensorPin;
    byte m_sensorState;
    byte m_lastSensorState = LOW;
  protected:
    volatile unsigned long m_counter = 0;
    unsigned long m_oldCounter = 0;
    unsigned long m_lastSensorReading = 0; // the last time the output pin was toggled

  public:
    UtilitySensor(int ledPin, int sensorPin);
    void initialize();
    void handleIrq(unsigned long now);
    unsigned long getCounter() const;
    void setCounter(unsigned long counter);
    void setOldCounter(unsigned long counter);
    bool counterIncremented() const;  // true when current counter higher than old counter
};

#endif // UtilitySensor_h
