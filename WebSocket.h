/*
 * Copyright (c) 2012, Steffen Kristoph Metze. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * * Neither the name of the WebSocket++ Project nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PETER THORSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef WEBSOCKET_H_
#define WEBSOCKET_H_
#include <Ethernet.h>

/**
 * A class to handle WebSockets with Arduino.
 */
class WebSocket {
public:
    WebSocket(IPAddress ip);

    void loop();
  
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
