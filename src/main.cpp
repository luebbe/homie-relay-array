#define FW_NAME "quad-relay-bme280"
#define FW_VERSION "1.0.0"

#include <Homie.h>
#include <PCF8574.h>
#include "PulseNode.hpp"
#include "RelayNode.hpp"
#include "BME280Node.hpp"

#define I2C_ADDRESS_PCF8574 0x20
#define I2C_ADDRESS_BME280_OUT 0x76
#define I2C_ADDRESS_BME280_IN 0x77
#define PIN_OPTOCOUPLER D7 // on Wemos D1 Mini

PCF8574 pcf8574(I2C_ADDRESS_PCF8574);

BME280Node bme280Outdoor("outdoor", "BME280", I2C_ADDRESS_BME280_OUT);
BME280Node bme280Indoor("indoor", "BME280", I2C_ADDRESS_BME280_IN);
HomieNode pcf8574Node("pcf8574", "PCF8574", "info");
PulseNode door("door", "Tür", PIN_OPTOCOUPLER);

const char *status = "status";
bool pcf8574Found;
char *pcf8574Name;

bool OnGetRelayState(int8_t id);
void OnSetRelayState(int8_t id, bool on);

RelayNode valve1("valve1", "Ventil 1", 0, OnGetRelayState, OnSetRelayState, true);
RelayNode valve2("valve2", "Ventil 2", 1, OnGetRelayState, OnSetRelayState, true);
RelayNode valve3("valve3", "Ventil 3", 2, OnGetRelayState, OnSetRelayState, true);
RelayNode valve4("valve4", "Ventil 4", 3, OnGetRelayState, OnSetRelayState, true);

void ICACHE_RAM_ATTR onOptoCouplerPulse()
{
  door.pulseDetected();
}

bool OnGetRelayState(int8_t id)
{
  if (pcf8574Found)
  {
    return (pcf8574.digitalRead(id));
  }
  else
  {
    return false;
  }
}

void OnSetRelayState(int8_t id, bool on)
{
  if (pcf8574Found && (id >= 0) && (id <= 8))
  {
    pcf8574.digitalWrite(id, on);
  }
}

void onHomieEvent(const HomieEvent &event)
{
  switch (event.type)
  {
  // The device rebooted when attachInterrupt was called in setup()
  // before Wifi was connected and interrupts were already coming in.
  case HomieEventType::WIFI_CONNECTED:
    attachInterrupt(PIN_OPTOCOUPLER, onOptoCouplerPulse, FALLING);
    break;
  case HomieEventType::WIFI_DISCONNECTED:
    detachInterrupt(PIN_OPTOCOUPLER);
    break;
  case HomieEventType::MQTT_READY:
    pcf8574Node.setProperty(status).send(pcf8574Found ? "ok" : "error");
    break;
  default:
    break;
  }
}

void setup()
{
  Homie_setFirmware(FW_NAME, FW_VERSION);

  Serial.begin(SERIAL_SPEED);
  Serial << endl
         << endl;

  // Initialize default settings before Homie.setup()
  valve1.beforeHomieSetup();
  valve2.beforeHomieSetup();
  valve3.beforeHomieSetup();
  valve4.beforeHomieSetup();

  bme280Indoor.beforeHomieSetup();
  bme280Outdoor.beforeHomieSetup();
  pcf8574Node.advertise(status)
      .setDatatype("enum")
      .setFormat("error, ok");

  Homie.disableResetTrigger();
  Homie.onEvent(onHomieEvent);
  Homie.setup();

  asprintf(&pcf8574Name, "• PCF8574 i2c[0x%2x] ", I2C_ADDRESS_PCF8574);
  Homie.getLogger() << pcf8574Name;

  // Set all pins to OUTPUT
  for (int i = 0; i < 8; i++)
  {
    pcf8574.pinMode(i, OUTPUT);
  }

  if (pcf8574.begin())
  {
    pcf8574Found = true;
    Homie.getLogger() << "found." << endl;
  }
  else
  {
    pcf8574Found = false;
    Homie.getLogger() << "not found. Check wiring!" << endl;
  }
}

void loop()
{
  Homie.loop();
}
