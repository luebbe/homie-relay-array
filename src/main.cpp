#define FW_NAME "quad-relay"
#define FW_VERSION "1.0.0"

#include <Homie.h>
#include <PCF8574.h>
#include "RelayNode.hpp"

PCF8574 pcf8574(0x20);

bool pcf8574Found;

bool OnGetRelayState(int8_t id);
void OnSetRelayState(int8_t id, bool on);

RelayNode valve1("valve1", 0, OnGetRelayState, OnSetRelayState, true);
RelayNode valve2("valve2", 1, OnGetRelayState, OnSetRelayState, true);
RelayNode valve3("valve3", 2, OnGetRelayState, OnSetRelayState, true);
RelayNode valve4("valve4", 3, OnGetRelayState, OnSetRelayState, true);

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

  // Set all pins to OUTPUT
  for (int i = 0; i < 8; i++)
  {
    pcf8574.pinMode(i, OUTPUT);
  }

  Homie.getLogger() << "• PCF8574 ";
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

  Homie.disableLedFeedback()
      .disableResetTrigger();

  Homie.setup();
}

void loop()
{
  Homie.loop();
}
