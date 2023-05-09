#include "SystemInterface.hpp"

#include <Arduino.h>

union endian_teller
{
  uint16_t word;
  uint8_t c[2];
};

static endian_teller i_bint = {0x0102};
static bool i_isSmallEndian = i_bint.c[0] == 2;

static inline uint8_t i_nibbleToHex(uint8_t nibble)
{
    // (0 - 9) = (0 - 9) and (10 - 15) = (A - F)
    return nibble < 10 ? 48 + nibble : 55 + nibble;
}

static inline uint8_t i_nibbleFromHex(uint8_t hex)
{
    return 65 <= hex ? hex - 55 : hex - 48;
}

static inline void i_hexNibblesByte(uint8_t byte, uint8_t* dst)
{
    dst[0] = i_nibbleToHex((0xF0 & byte) >> 4);
    dst[1] = i_nibbleToHex(0x0F & byte);
}

// ----------- HCPPacket ------------ //

HRSPacket::HRSPacket() :
  HRSPacket(0)
{

}

HRSPacket::HRSPacket(uint16_t command) :
  m_capacity(HRS_SMALL_PACKET_SIZE),
  m_command(command),
  m_dataNumBytes(0),
  m_writeable(true),
  m_getPtr(0),
  m_occupied(false)
{

}

void HRSPacket::setCommand(uint16_t command)
{
  m_command = command;
}

uint16_t HRSPacket::getCommand() const
{
  return m_command;
}

HRSPacket& HRSPacket::putByte(uint8_t byte)
{
  // Return if packet is not writeable or if byte will exceed capacity
  if(!m_writeable || m_dataNumBytes + 1 > m_capacity)
    return *this;
  
  m_data[m_dataNumBytes] = byte;
  m_dataNumBytes++;

  return *this;
}

HRSPacket& HRSPacket::putShort(uint16_t word)
{
  // Return if packet is not writeable or if word will exceed capacity
  if(!m_writeable || m_dataNumBytes + 2 > m_capacity)
    return *this;

  uint8_t* wordb = (uint8_t*) &word;

  if(i_isSmallEndian)
  {
    m_data[m_dataNumBytes] = wordb[1];
    m_data[m_dataNumBytes + 1] = wordb[0];
  }
  else
  {
    m_data[m_dataNumBytes] = wordb[0];
    m_data[m_dataNumBytes + 1] = wordb[1];
  }

  m_dataNumBytes += 2;

  return *this;
}

HRSPacket& HRSPacket::putInt(uint32_t dword)
{
  // Return if packet is not writeable or if dword will exceed capacity
  if(!m_writeable || m_dataNumBytes + 4 > m_capacity)
    return *this;

  uint8_t* dwordb = (uint8_t*) &dword;

  if(i_isSmallEndian)
  {
    m_data[m_dataNumBytes] = dwordb[3];
    m_data[m_dataNumBytes + 1] = dwordb[2];
    m_data[m_dataNumBytes + 2] = dwordb[1];
    m_data[m_dataNumBytes + 3] = dwordb[0];
  }
  else
  {
    m_data[m_dataNumBytes] = dwordb[0];
    m_data[m_dataNumBytes + 1] = dwordb[1];
    m_data[m_dataNumBytes + 2] = dwordb[2];
    m_data[m_dataNumBytes + 3] = dwordb[3];
  }

  m_dataNumBytes += 4;

  return *this;
}

HRSPacket& HRSPacket::putFloat(float fword)
{
  // Return if packet is not writeable or if fword will exceed capacity
  if(!m_writeable || m_dataNumBytes + 4 > m_capacity)
    return *this;

  uint32_t dword = *(uint32_t*) &fword;

  return putInt(dword);
}

uint8_t HRSPacket::getByte(size_t index) const
{
  // Return 0 if index is out of bounds
  if(index >= m_dataNumBytes)
    return 0;

  return m_data[index];
}

uint16_t HRSPacket::getShort(size_t index) const
{
  // Return 0 if index is out of bounds
  if(index + 1 >= m_dataNumBytes)
    return 0;

  uint8_t wordb[2];

  if(i_isSmallEndian)
  {
    wordb[0] = m_data[index + 1];
    wordb[1] = m_data[index];
  }
  else
  {
    wordb[0] = m_data[index];
    wordb[1] = m_data[index + 1];
  }

  return *(uint16_t*) wordb;
}

uint32_t HRSPacket::getInt(size_t index) const
{
  // Return 0 if index is out of bounds
  if(index + 3 >= m_dataNumBytes)
    return 0;

  uint8_t dwordb[4];

  if(i_isSmallEndian)
  {
    dwordb[0] = m_data[index + 3];
    dwordb[1] = m_data[index + 2];
    dwordb[2] = m_data[index + 1];
    dwordb[3] = m_data[index];
  }
  else
  {
    dwordb[0] = m_data[index];
    dwordb[1] = m_data[index + 1];
    dwordb[2] = m_data[index + 2];
    dwordb[3] = m_data[index + 3];
  }

  return *(uint32_t*) dwordb;
}

float HRSPacket::getFloat(size_t index) const
{
  // Return 0 if index is out of bounds
  if(index + 3 >= m_dataNumBytes)
    return 0;

  uint32_t dword = getInt(index);

  return *(float*) &dword;
}

uint8_t HRSPacket::getByte() const
{
  return getByte(m_getPtr++);
}

uint16_t HRSPacket::getShort() const
{
  uint16_t word = getShort(m_getPtr);
  m_getPtr += 2;
  return word;
}

uint32_t HRSPacket::getInt() const
{
  uint32_t dword = getInt(m_getPtr);
  m_getPtr += 4;
  return dword;
}

float HRSPacket::getFloat() const
{
  uint32_t dword = getInt(m_getPtr);
  m_getPtr += 4;
  return *(float*) &dword;
}

void HRSPacket::resetGetPtr()
{
  m_getPtr = 0;
}

bool HRSPacket::isEnd()
{
  return m_getPtr >= m_dataNumBytes;
}

void HRSPacket::setGetPtr(size_t index)
{
  m_getPtr = index;
}

size_t HRSPacket::size() const
{
  if(m_writeable) return 1 + m_dataNumBytes;
  else return 1 + m_dataNumBytes + 1;
}

size_t HRSPacket::dataNumBytes() const
{
  return m_dataNumBytes;
}

void HRSPacket::end()
{
  m_writeable = false;
}

bool HRSPacket::isWriteable() const
{
  return m_writeable;
}

// ----------- HRSLargePacket ------------ //

HRSLargePacket::HRSLargePacket() :
  HRSLargePacket(0)
{

}

HRSLargePacket::HRSLargePacket(uint16_t command) :
  HRSPacket(command)
{
  m_capacity = HRS_MAX_PACKET_SIZE;
}

// ----------- hrssi ------------ //

hrssi::Variable::Variable(Type type, const char* name, void* ptr) :
  type(type),
  name(name),
  byte((uint8_t*) ptr)
{

}

// Small packet pool
static HRSPacket i_smallPackets[HRS_NUM_SMALL_PACKETS];
static uint8_t i_sRecvPacketIndex = 1;
static uint8_t i_sPacketIndex = 0;

// Large packet pool
static HRSLargePacket i_largePackets[HRS_NUM_LARGE_PACKETS];
static uint8_t i_lRecvPacketIndex = 1;
static uint8_t i_lPacketIndex = 0;

static bool i_recvPacketIsLarge = false;

// Variable array
static hrssi::Variable* i_variables = nullptr;
static size_t i_numVariables = 0;

void hrssi::setVariablesArray(Variable* variables, size_t numVariables)
{
  i_variables = variables;
  i_numVariables = numVariables;
}

void hrssi::poll()
{
  static uint8_t byteAsHex[2];
  static uint8_t byteAsHexIndex = 0;

  HRSPacket& packet = hrssi::getReceivePacket(i_recvPacketIsLarge);

  while(Serial.available())
  {
    uint8_t byte = Serial.read();

    // Start or end of packet
    if(127 < byte)
    {
      // End of packet
      if(byte == r_noop)
      {
        packet.end();
        Serial.print("End packet");
        Serial.print(packet.m_command);
        Serial.print(" ");
        Serial.println(i_lRecvPacketIndex);
        processCommand(packet);
        continue;
      }

      // Reserve a large packet if the command is the 'ret' or 'echo' command
      i_recvPacketIsLarge = byte == r_ret || byte == r_echo;
      packet = hrssi::getReceivePacket(i_recvPacketIsLarge);
      packet.setCommand(byte);
      Serial.print("Start packet");
      Serial.print(packet.m_command);
      Serial.print(" ");
      Serial.println(i_lRecvPacketIndex);
    }
    // Data byte
    else if(127 < packet.m_command)
    {
      byteAsHex[byteAsHexIndex] = byte;
      byteAsHexIndex++;

      if(byteAsHexIndex == 2)
      {
        byteAsHexIndex = 0;
        packet.putByte(i_nibbleFromHex(byteAsHex[0]) << 4 | i_nibbleFromHex(byteAsHex[1]));
      }
    }
  }
}

void hrssi::send(HRSPacket& command)
{
  Serial.write((uint8_t) command.m_command); // Start of packet

  for(size_t i = 0; i < command.m_dataNumBytes; i++)
  {
    uint8_t byteAsHex[2];
    i_hexNibblesByte(command.m_data[i], byteAsHex);
    Serial.write(byteAsHex, 2);
  }

  Serial.write((uint8_t) s_noop); // End of packet
}

void hrssi::processCommand(HRSPacket& command)
{
  switch(command.m_command)
  {
    case r_noop:
      break;
    case r_echo:
      com_echo(command);
      break;
    case r_ret:
      com_ret(command);
      break;
    case r_getbyte:
      com_getbyte(command);
      break;
    case r_getword:
      com_getword(command);
      break;
    case r_getdword:
      com_getdword(command);
      break;
    case r_getstring:
      com_getstring(command);
      break;
    case r_getvars:
      com_getvars(command);
      break;
  }
}

HRSPacket& hrssi::getNextAvailablePacket(bool large)
{
  if(large)
  {
    i_lPacketIndex++;
    if(i_lRecvPacketIndex == i_lPacketIndex) i_lPacketIndex++;

    if(i_lPacketIndex >= HRS_NUM_LARGE_PACKETS)
      i_lPacketIndex = 0;

    i_largePackets[i_lPacketIndex] = HRSLargePacket();

    return i_largePackets[i_lPacketIndex];
  }
  else
  {
    i_sPacketIndex++;
    if(i_sRecvPacketIndex == i_sPacketIndex) i_sPacketIndex++;

    if(i_sPacketIndex >= HRS_NUM_SMALL_PACKETS)
      i_sPacketIndex = 0;

    i_smallPackets[i_sPacketIndex] = HRSPacket();

    return i_smallPackets[i_sPacketIndex];
  }
}

HRSPacket& hrssi::getReceivePacket(bool large)
{
  if(large)
  {
    if(!i_largePackets[i_lRecvPacketIndex].m_writeable)
    {
      i_lRecvPacketIndex++;
      if(i_lRecvPacketIndex == i_lPacketIndex) i_lRecvPacketIndex++;

      if(i_lRecvPacketIndex >= HRS_NUM_LARGE_PACKETS)
        i_lRecvPacketIndex = 0;

        i_largePackets[i_lRecvPacketIndex] = HRSLargePacket();
    }

    return i_largePackets[i_lRecvPacketIndex];
  }
  else
  {
    if(!i_smallPackets[i_sRecvPacketIndex].m_writeable)
    {
      i_sRecvPacketIndex++;
      if(i_sRecvPacketIndex == i_sPacketIndex) i_sRecvPacketIndex++;

      if(i_sRecvPacketIndex >= HRS_NUM_SMALL_PACKETS)
        i_sRecvPacketIndex = 0;

      i_smallPackets[i_sRecvPacketIndex] = HRSPacket();
    }

    return i_smallPackets[i_sRecvPacketIndex];
  }
}

void HRS_COMR hrssi::com_echo(HRSPacket& command)
{
  HRSPacket& response = hrssi::getNextAvailablePacket(true);

  response.setCommand(s_logstr);

  for(size_t i = 0; i < command.m_dataNumBytes; i++)
    response.putByte(command.m_data[i]);

  response.end();

  hrssi::send(response);
}

void HRS_COMR hrssi::com_ret(HRSPacket& command)
{
  // TODO
}

void HRS_COMR hrssi::com_getbyte(HRSPacket& command)
{
  HRSPacket& response = hrssi::getNextAvailablePacket(false);

  uint8_t retID = command.getByte();
  uint16_t index = command.getShort();
  response.setCommand(s_ret);
  response.putByte(retID);

  if(index < i_numVariables)
    response.putByte(*i_variables[index].byte);
  else
    response.putByte(0);

  response.end();

  hrssi::send(response);
}

void HRS_COMR hrssi::com_getword(HRSPacket& command)
{
  HRSPacket& response = hrssi::getNextAvailablePacket(false);

  uint8_t retID = command.getByte();
  uint16_t index = command.getShort();
  response.setCommand(s_ret);
  response.putByte(retID);

  if(index < i_numVariables)
    response.putShort(*i_variables[index].word);
  else
    response.putShort(0);

  response.end();

  hrssi::send(response);
}

void HRS_COMR hrssi::com_getdword(HRSPacket& command)
{
  HRSPacket& response = hrssi::getNextAvailablePacket(false);

  uint8_t retID = command.getByte();
  uint16_t index = command.getShort();
  response.setCommand(s_ret);
  response.putByte(retID);

  if(index < i_numVariables)
    response.putInt(*i_variables[index].dword);
  else
    response.putInt(0);

  response.end();

  hrssi::send(response);
}

void HRS_COMR hrssi::com_getstring(HRSPacket& command)
{
  HRSPacket& response = hrssi::getNextAvailablePacket(true);

  uint8_t retID = command.getByte();
  uint16_t index = command.getShort();
  response.setCommand(s_ret);
  response.putByte(retID);

  if(index < i_numVariables)
  {
    int len = strlen(i_variables[index].string);
    len = len > response.m_capacity - 2 ? response.m_capacity - 2 : len;
    for(int i = 0; i < len; i++)
      response.putByte(i_variables[index].string[i]);
    
    response.putByte(0);
  }
  else
    response.putByte(0);

  response.end();

  hrssi::send(response);
}

void HRS_COMR hrssi::com_getvars(HRSPacket& command)
{
  HRSPacket& response = hrssi::getNextAvailablePacket(true);

  response.setCommand(s_ret);
  response.putByte(command.getByte());

  for(size_t i = 0; i < i_numVariables; i++)
  {
    response.putByte(i_variables[i].type);

    int len = strlen(i_variables[i].name);
    len = len > response.m_capacity - 2 ? response.m_capacity - 2 : len;
    for(size_t j = 0; j < strlen(i_variables[i].name); j++)
      response.putByte(i_variables[i].name[j]);
    response.putByte(0);
  }

  response.end();

  hrssi::send(response);
}

void HRS_COMS hrssi::com_logstr(const char* str)
{
  HRSPacket& response = hrssi::getNextAvailablePacket(true);

  response.setCommand(s_logstr);

  int len = strlen(str);
  len = len > response.m_capacity - 2 ? response.m_capacity - 2 : len;
  for(int i = 0; i < len; i++)
    response.putByte(str[i]);
  response.putByte(0);

  response.end();

  hrssi::send(response);
}