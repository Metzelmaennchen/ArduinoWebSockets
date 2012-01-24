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
  
    void    HandleSocketStream(uint8_t msg);
    boolean HandShake();
  
    // Sends a single character to the client
    void SendChar(char message);

    // Sends a message to the client
    void SendMessage(char *message, uint16_t length);

    typedef void(*SocketStreamCallback)(char * msg);

    // Installs a socket stream callback to pass the decoded message back.
    void InstallSocketStreamCallback(SocketStreamCallback /*streamCallback*/);

 private:
    EthernetClient client;
    char keyInputBuffer[61];
    IPAddress ip;
    char maskBytes[4];
    boolean startByte;
    int length;
    byte counter;
    // pointer to our socket stream callback
    SocketStreamCallback streamCallback;
};

#endif
