// Enable debug prints to serial monitor
//#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24

#include <SPI.h>
#include <MySensors.h>
#include <TimerOne.h>
#include "UtilitySensor.h"
#include "GasUtilitySensor.h"

#define SKETCH_NAME "Gasmeter"
#define SKETCH_MAJOR_VER "1"
#define SKETCH_MINOR_VER "1"

const int ChildID = 1;

const int GasSensorPin = 4;
const int GasLedPin = LED_BUILTIN; //5;

const int ElectricitySensorPin = 6;
const int ElectricityLedPin = 7;

const unsigned long SendFrequency = 30000; // Minimum time between send (in milliseconds). We don't want to spam the gateway.
const unsigned long DebounceDelay = 50;
const int TimerUs = 50000;                          // 50mS set timer duration in microseconds

MyMessage flowMsg(ChildID, V_FLOW);
MyMessage volumeMsg(ChildID, V_VOLUME);
MyMessage lastCounterMsg(ChildID, V_VAR1);

GasUtilitySensor gasSensor(GasSensorPin, GasLedPin);
UtilitySensor electricitySensor(ElectricitySensorPin, ElectricityLedPin);

boolean counterValueReceivedFromGateway = false;
unsigned long timeLastSend = 0;

void timerIsr()
{
  unsigned long now = millis();

  gasSensor.handleIrq(now);
  electricitySensor.handleIrq(now);
}

void setup()
{
  gasSensor.initialize();
  electricitySensor.initialize();

  Timer1.initialize(TimerUs);
  Timer1.attachInterrupt(timerIsr);
}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER "." SKETCH_MINOR_VER);

  // Register this device as Gasflow sensor
  present(ChildID, S_GAS);
}

void send_message(unsigned long counterValue, double liters_per_minute)
{
  send(lastCounterMsg.set(counterValue));                   // Send  globalcounter value to gateway in VAR1
  send(flowMsg.set(liters_per_minute, 2));                  // Send flow value to gateway
  send(volumeMsg.set(1.0 * counterValue / 1000, 3));        // Send volume value to gateway and convert from dm3 to m3
}

void receive(const MyMessage &message)
{
#ifdef MY_DEBUG
  Serial.println("--------------- function receive ---------------");
#endif
  unsigned long gatewayPulseCount = 0;
  if (message.type == V_VAR1) {
    gatewayPulseCount = message.getULong();
    if (gasSensor.getCounter() != gatewayPulseCount)
    {
      //gasCount += gatewayPulseCount;
#ifdef MY_DEBUG
      Serial.print("Received last pulse count from gateway: ");
      Serial.println(gasCount);
#endif
      counterValueReceivedFromGateway = true;
      // During testing in can be handy to be able to set the values in domoticz; You can do this by using the REST request below:
      // (replace x.x.x.x and deviceid by relevant numbers
      // http://x.x.x.x:8080/json.htm?type=command&param=udevice&idx=deviceid&nvalue=0&svalue=0
      //use line below to reset your counter in domoticz; We needed it ;-)
      unsigned long gasCount = 0;
      gasSensor.setCounter(gasCount);
      gasSensor.setOldCounter(gasCount);
      //oldGasCount = gasCount;
    }
  }
#ifdef MY_DEBUG
  Serial.println("---------------------------------------------------");
#endif
}

void loop()
{
  Serial.println("in loop");
  if (!counterValueReceivedFromGateway)
  {
    //Last Pulsecount not yet received from controller, request it again
#ifdef MY_DEBUG
    Serial.println("Requesting as nothing was received from gateway yet!");
#endif
    request(ChildID, V_VAR1);
    wait(1000);
    return;
  }
  else
  {
    unsigned long now = millis();

    unsigned long timePastSinceLastSend = now - timeLastSend;
    double liters_per_minute = gasSensor.litersPerMinute(); //litersPerMinute(timePastSinceLastSend, liters);

    // If we have counted a pulse and the send frequency has passed, send message to gateway;
    // Also send a message if last message has been sent more than x seconds ago and flow is still more than 0
    if ((counterValueReceivedFromGateway) &&
        (timePastSinceLastSend > SendFrequency) &&
        (gasSensor.counterIncremented() || liters_per_minute > 0))
    {
      unsigned long gasCounter = gasSensor.getCounter(); // create copy, use it later on

      send_message(gasCounter, liters_per_minute);

#ifdef MY_DEBUG
      Serial.print("Liters per Minuut: ");
      Serial.println(liters_per_minute);
      Serial.print("Minutes Passed: ");
      Serial.println(1.0 * timePastSinceLastSend / 60000.0);
      Serial.print("Aantal Liters: ");
      Serial.println(gasSensor.liters());
#endif

      timeLastSend = now;
      //oldGasCount = gasCount;
      gasSensor.setOldCounter(gasCounter);
    }

    // save the gasSensorReading. Next time through the loop, it'll be the lastGasSensorState:
    //lastGasSensorState = gasSensorReading;

    //sleep(1000);
  }
}
