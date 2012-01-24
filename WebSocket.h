#ifndef WEBSOCKET_H_
#define WEBSOCKET_H_
#include <Ethernet.h>

/**
 * A class to handle WebSockets with Arduino.
 */
class WebSocket {
 public:
  WebSocket(IPAddress ip);
  
  void begin();
  
  void    HandleSocket();
  boolean HandShake();
  
  // Sends a single character to the client
  void SendMessage(char message);
  // Sends a message to the client
  void SendMessage(char *message, uint16_t length);

 private:
  EthernetClient client;
  char keyInputBuffer[61];
  IPAddress ip;
};

#endif
