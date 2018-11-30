// Enable debug prints to serial monitor
//#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24

#include <SPI.h>
#include <MySensors.h>

#define SKETCH_NAME "Gasmeter"
#define SKETCH_MAJOR_VER "1"
#define SKETCH_MINOR_VER "1"

const int SensorPin = 4;
const int ChildID = 1;
const int LedPin = 6;
const unsigned long SendFrequency = 5000; // Minimum time between send (in milliseconds). We don't want to spam the gateway.
const unsigned long DebounceDelay = 50;

MyMessage flowMsg(ChildID, V_FLOW);
MyMessage volumeMsg(ChildID, V_VOLUME);
MyMessage lastCounterMsg(ChildID, V_VAR1);

boolean counterValueReceivedFromGateway = false;
int sensorState;
int lastSensorState = LOW;
unsigned long globalCounter = 0;
unsigned long oldGlobalCounter = 0;
unsigned long timeLastSend = 0;
unsigned long lastDebounceTime = 0; // the last time the output pin was toggled

void setup()
{
  pinMode(SensorPin, INPUT);
  pinMode(LedPin, OUTPUT);

  // set initial LED state
  digitalWrite(LedPin, LOW);
}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER "." SKETCH_MINOR_VER);

  // Register this device as Gasflow sensor
  present(ChildID, S_GAS);
}

double milliToMinute(unsigned long milliseconds)
{
  return 1.0 * milliseconds / 60000.0;
}

double litersPerMinute(unsigned long millisPassed, double liters)
{
  return 1.0 * liters / milliToMinute(millisPassed);
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
    if (globalCounter != gatewayPulseCount)
    {
      globalCounter += gatewayPulseCount;
#ifdef MY_DEBUG
      Serial.print("Received last pulse count from gateway: ");
      Serial.println(globalCounter);
#endif
      // During testing in can be handy to be able to set the values in domoticz; You can do this by using the REST request below:
      // (replace x.x.x.x and deviceid by relevant numbers
      // http://x.x.x.x:8080/json.htm?type=command&param=udevice&idx=deviceid&nvalue=0&svalue=0
      //use line below to reset your counter in domoticz; We needed it ;-)
      //globalCounter = 0;
      counterValueReceivedFromGateway = true;
      oldGlobalCounter = globalCounter;
    }
  }
#ifdef MY_DEBUG
  Serial.println("---------------------------------------------------");
#endif
}

void loop()
{
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
    bool counterIncremented = false;

    // read the state of the switch into a local variable
    int reading = digitalRead(SensorPin);

    // reset the debounce timer if the switch changed, due to noise or pressing
    if (reading != lastSensorState)
    {
      lastDebounceTime = now;
    }

    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state
    if ((now - lastDebounceTime) > DebounceDelay)
    {
      if (reading != sensorState)
      {
        sensorState = reading;

        if (sensorState == HIGH)
        {
          ++globalCounter;
          counterIncremented = true;

#ifdef MY_DEBUG
          Serial.print("Counter increment; value = ");
          Serial.println(globalCounter);
#endif

          // give LED short pulse
          digitalWrite(LedPin, HIGH);
          delay(100);
          digitalWrite(LedPin, LOW);
        }
      }
    }

    unsigned long timePastSinceLastSend = now - timeLastSend;
    unsigned long liters = globalCounter - oldGlobalCounter;
    double liters_per_minute = litersPerMinute(timePastSinceLastSend, liters);

    // If we have counted a pulse and the send frequency has passed, send message to gateway;
    // Also send a message if last message has been sent more than x seconds ago and flow is still more than 0
    if ((counterValueReceivedFromGateway) &&
        (timePastSinceLastSend > SendFrequency) &&
        (counterIncremented || liters_per_minute > 0))
    {
      send_message(globalCounter, liters_per_minute);

#ifdef MY_DEBUG
      Serial.print("Liters per Minuut: ");
      Serial.println(liters_per_minute);
      Serial.print("Minutes Passed: ");
      Serial.println(milliToMinute(timePastSinceLastSend));
      Serial.print("Aantal Liters: ");
      Serial.println(liters);
#endif

      timeLastSend = now;
      oldGlobalCounter = globalCounter;
    }

    // save the reading. Next time through the loop, it'll be the lastSensorState:
    lastSensorState = reading;
  }
}
