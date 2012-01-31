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

#include <sha1.h>
#include <Base64.h>
// needed for reading sd cards
#include <SD.h>
// needed for ethernet communication
#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,177);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer webSocketServer(8080);
EthernetServer httpServer(80);
EthernetClient client;
bool gotKey = false;
File myFile;

void setup()
{  
  Serial.begin(9600);
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
   pinMode(10, OUTPUT);
   
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("SD Card initialization done.");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  webSocketServer.begin();
}

boolean startByte = false;
byte length = 0;
byte counter = 0;

// Sends data to the client. At the moment
// there is only support for messages <= 65535
void SendData(char *message, uint16_t length)
{
  if(client.connected())
  {
    // erstes byte ist 129
    client.write((uint8_t) 0x81);

    // zweites byte die laenge falls <= 125
    if(length <= 125)
    {
      client.write((uint8_t)length);
    }
    // sonst haben wir drei bytes
    else //if(length >= 125)
    {
      client.write(126);
      client.write((uint8_t)(length >> 8) & 255);
      client.write((uint8_t)length & 255);
    }
    // and don't forget the message itself
    client.write(message);
  }
}

void SendChar(char message)
{
  if(client.connected())
  {
    // erstes byte ist 129
    client.write((uint8_t) 0x81);
    client.write(1);
    // and don't forget the message itself
    client.write(message);
  }
}

uint8_t maskBytes[4];

// decodes the binary data coming from the client after succesful handshake
void HandleWebSocketStream(uint8_t data)
{
  // no startbyte found up to know
  if(!startByte)
  {
    // startbyte is 129
    if(data == 129)
    {
      startByte = true;
      length = 0;
      maskBytes[0] = 0;
      maskBytes[1] = 0;
      maskBytes[2] = 0;
      maskBytes[3] = 0;
      counter = 0;
    }
    // OxFF ist der closing handshake,
    // gefolgt von einer laenge 0x00
    else if(data == 255)
    {
    }
  }
  // start character found
  else
  {
      // the length is in the last 7bit of the second byte
      if(length == 0)
      {
        // also erstes bit weg
        length = data & 0x7F;
        // falls das jetzt 126 sein sollten, so kommt die eigentliche
        // laenge in den zewi folgenden bytes
      }
      else if(maskBytes[0] == 0) { maskBytes[0] = data; }
      else if(maskBytes[1] == 0) { maskBytes[1] = data; }
      else if(maskBytes[2] == 0) { maskBytes[2] = data; }
      else if(maskBytes[3] == 0) { maskBytes[3] = data; }
      // now we decode the real data :)
      else
      {
        // data ist xor maskbyte[index % 4]
        char c = data ^ maskBytes[counter % 4];
        counter++;
        if(counter == length)
        {
          startByte = false;
        }
        
        Serial.print(c);
      }
  }
}

boolean PerformHandShake()
{
  bool gotNewLine = false;
  char msgBuffer[60];
  byte msgBufPtr = 0;
  // 36 Ziffern
  char keyInputBuffer[] = "xxxxxxxxxxxxxxxxxxxxxxxx258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  bool processLine = false;

  while (client.connected()) 
  {
    if (client.available()) 
    {
      char c = client.read();
      // new char is \n and old char is also \n
      if(c == '\n' && gotNewLine)
      {
        gotNewLine = false;
            
        if(gotKey)
        {
              gotKey = false;
              // let's crypt it
              sha1_hash_t hash;    // output goes here
              sha1(&hash, keyInputBuffer, 60*8);
              // and now to encode to bas64
              byte num = base64_encode(&msgBuffer[0], (char*)hash, 20);
              msgBuffer[num] = '\0';
              
              // answer with server response
              client.println("HTTP/1.1 101 Switching Protocols");
              client.println("Upgrade: websocket");
              client.print("Connection: Upgrade\r\nSec-WebSocket-Accept: ");
              client.println(msgBuffer);
              client.println();
              return true;
        }
            
        // give up...
        break;
      }
      else
      {
            // received end of line
            if(c == '\n')
            {
              gotNewLine = true;
              if(processLine)
              {
                //msgBuffer[msgBufPtr] = '\0';
                //Serial.println(msgBuffer);
                msgBufPtr = 0;
                processLine = false;
                
                // is there a key inside?
                if(msgBuffer[14] == 'K' && msgBuffer[15] == 'e' && msgBuffer[16] == 'y')
                {
  	              // copy our key to the keybuffer
                  for(byte i = 19, j = 0; i < 43; i++, j++)
                  {
                    keyInputBuffer[j] = msgBuffer[i];
                  }
                  gotKey = true;
                }
              }
        }
        // got a character
        else if(c != '\r')
        {
          // check if this is the first character
          if(gotNewLine)
          {
            gotNewLine = false;
            // we only dump lines with S for Sec or U for Upgrade
            if(c == 'S')// || c == 'U')
            {
              processLine = true;
            }
          }
          
          //copy our char to the buffer
          if(processLine)
          {
            msgBuffer[msgBufPtr++] = c;
          }
        }
      }
    }
  }
  Serial.println(">> client disconnected");
  // close the connection:
  client.stop();
  return false;
}

void PrintHtmlPage()
{
  // open the file for reading:
  myFile = SD.open("index.htm");
  if (myFile) 
  {
    // read from the file until there's nothing else in it:
    while (myFile.available()) 
    {
    	client.print((char)myFile.read());
    }
    // close the file:
    myFile.close();
  }
  else
  {
    client.println("<html>could not open file <html>");
  }
}

void loop()
{
  // listen for incoming clients
  client = webSocketServer.available();
  if (client) 
  {
    Serial.print(">> Client connected");
    
    if(PerformHandShake())
    {
      while (client.connected()) 
      {
        if (client.available()) 
        {
          char c = client.read();
          Serial.print(c);
          // handle streaming of websocket data
          HandleWebSocketStream((uint8_t)c);
        }
        
        if(Serial.available())
        {
          char c = Serial.read();
          SendChar(c);
        }
      }
    }
    Serial.println(">> client disconnected");
    // close the connection:
    client.stop();
  }
}
