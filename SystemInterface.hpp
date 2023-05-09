#ifndef HRS_SYSTEM_INTERFACE_HPP
#define HRS_SYSTEM_INTERFACE_HPP

#define HRS_SMALL_PACKET_SIZE 64
#define HRS_MAX_PACKET_SIZE 256

#define HRS_NUM_SMALL_PACKETS 4
#define HRS_NUM_LARGE_PACKETS 2


// List of commands from the Control Panel
#define r_noop 128
#define r_echo 129
#define r_ret 130
#define r_getbyte 131
#define r_getword 132
#define r_getdword 133
#define r_getstring 134
#define r_getvars 135

// List of commands to the Control Panel
#define s_noop 128
#define s_logstr 129
#define s_ret 130

#include <stdint.h>
#include <stddef.h>

class HRSPacket
{
public:
  HRSPacket();
  HRSPacket(uint16_t command);

  void setCommand(uint16_t command);
  uint16_t getCommand() const;

  HRSPacket& putByte(uint8_t byte);
  HRSPacket& putShort(uint16_t word);
  HRSPacket& putInt(uint32_t dword);
  HRSPacket& putFloat(float fword);

  uint8_t getByte(size_t index) const;
  uint16_t getShort(size_t index) const;
  uint32_t getInt(size_t index) const;
  float getFloat(size_t index) const;

  uint8_t getByte() const;
  uint16_t getShort() const;
  uint32_t getInt() const;
  float getFloat() const;

  void resetGetPtr();
  bool isEnd();
  void setGetPtr(size_t index);
  size_t size() const;
  size_t dataNumBytes() const;
  void end();
  bool isWriteable() const;
protected:
  uint16_t m_capacity;
private:
  uint16_t m_command;

  bool m_writeable;
  mutable size_t m_getPtr;
  bool m_occupied;

  uint16_t m_dataNumBytes;
  uint8_t m_data[HRS_SMALL_PACKET_SIZE];

  friend class hrssi;
};

class HRSLargePacket : public HRSPacket
{
public:
  HRSLargePacket();
  HRSLargePacket(uint16_t command);
private:
  uint8_t m_extraData[HRS_MAX_PACKET_SIZE - HRS_SMALL_PACKET_SIZE];
};

#define HRS_COMS // Marker for commands from the Control Panel
#define HRS_COMR // Marker for commands to the Control Panel

class hrssi
{
public:
  enum Type
  {
    BYTE,
    SHORT,
    INT,
    FLOAT,
    STRING
  };
  
  struct Variable
  {
    Type type;
    const char* name;
    union
    {
      uint8_t* byte;
      uint16_t* word;
      uint32_t* dword;
      float* fword;
      char* string;
    };

    Variable(Type type, const char* name, void* byte);
  };

  static void setVariablesArray(Variable* variables, size_t numVariables);
  
  static void poll();
private:
  static void send(HRSPacket& packet);
  static void processCommand(HRSPacket& packet);

  static HRSPacket& getNextAvailablePacket(bool large);
  static HRSPacket& getReceivePacket(bool large);

  // Commands from the Control Panel
  static HRS_COMR void com_echo(HRSPacket& packet);
  static HRS_COMR void com_ret(HRSPacket& packet);
  static HRS_COMR void com_getbyte(HRSPacket& packet);
  static HRS_COMR void com_getword(HRSPacket& packet);
  static HRS_COMR void com_getdword(HRSPacket& packet);
  static HRS_COMR void com_getstring(HRSPacket& packet);
  static HRS_COMR void com_getvars(HRSPacket& packet);

  // Commands to the Control Panel
  static HRS_COMS void com_logstr(const char* str);
  static HRS_COMS void com_ret(size_t numBytes, const uint8_t* data);
};

#endif // HRS_SYSTEM_INTERFACE_HPP