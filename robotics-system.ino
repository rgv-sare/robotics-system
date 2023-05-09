#include "SystemInterface.hpp"

#include <stdint.h>

uint8_t theByte = 55;
uint16_t theShort = 1234;
uint32_t theInt = 12345678;
float theFloat = 123.456f;
const char* theString = "Hello World!";

hrssi::Variable variables[5] =
{
  {hrssi::BYTE, "theByte", &theByte},
  {hrssi::SHORT, "theShort", &theShort},
  {hrssi::INT, "theInt", &theInt},
  {hrssi::FLOAT, "theFloat", &theFloat},
  {hrssi::STRING, "theString", const_cast<char*>(theString)}
};

void setup()
{
  Serial.begin(115200);
  hrssi::setVariablesArray(variables, 5);
}

void loop()
{
  hrssi::poll();
}
