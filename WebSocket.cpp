#include "WebSocket.h"
#include <sha1.h>
#include <Base64.h>

// 36 Ziffern
const char* key = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";


WebSocket::WebSocket(IPAddress _ip) 
  : ip(_ip)
{
    int counter = 0;
    // append the magic number to our key buffer
    for(int i = 24; i < 60; i++)
    {
        keyInputBuffer[i] = key[counter];
        counter++;
    }
}

void WebSocket::ActionWrite(char *buf)
{
    // check if client is really connected...
    if (client.connected()) 
    {
        client.write((uint8_t) 0x00);
        client.write(buf);
        client.write((uint8_t) 0xFF);
    }
}

void WebSocket::begin()
{
  
}

void WebSocket::HandleSocket()
{
    // listen for incoming clients
  client ;//= server.available();
  if (client) 
  {
    Serial.println(">> Client connected");


    Serial.println(">> client disconnected");
    // close the connection:
    client.stop();
  }
}


boolean WebSocket::HandShake()
{
    boolean processLine = false;
    int msgBufPtr = 0;
    char msgBuffer[80];
    boolean gotNewLine = false;

    while (client.connected())
    {
        if (client.available())
        {
            char c = client.read();
            //Serial.print(c);
            
            // try to establish a new webSocket connection.
      
            // new char is \n and old char is also \n
            if(c == '\n' && gotNewLine)
            {
                // let's crypt it
                sha1_hash_t hash;    // output goes here
                sha1(&hash, keyInputBuffer, 60*8);
                int num = base64_encode(&msgBuffer[0], (char*)hash, 20);
                // und den string schoen abschliessen
                msgBuffer[num] = '\0';

                // answer with server response
                client.println("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket");
                client.print("Connection: Upgrade\r\nSec-WebSocket-Accept: ");
                client.println(msgBuffer);
                client.println();

                // connected :)
                return true;
            }
            else
            {
                // received end of line
                if(c == '\n')
                {
                    gotNewLine = true;
                    if(processLine)
                    {
                        msgBuffer[msgBufPtr] = '\0';
                        //Serial.println(msgBuffer);
                        msgBufPtr = 0;
                        processLine = false;
                        
                        // is there a key inside?
                        if(msgBuffer[14] == 'K' && msgBuffer[15] == 'e' && msgBuffer[16] == 'y')
                        {
                            int counter = 0;
                            // copy our key to the keybuffer
                            for(int i = 19; i < 43; i++)
                            {
                                keyInputBuffer[counter] = msgBuffer[i];
                                counter++;
                            }
                        }
                    }
                }
                // got a character
                else if(c != '\r')
                {
                    // check if this is the first character
                    if(gotNewLine)
                    {
                        // we only dump lines with S for Sec or U for Upgrade
                        if(c == 'S')// || c == 'U')
                        {
                            processLine = true;
                        }
                    }
                    gotNewLine = false;

                    //copy our char to the buffer
                    if(processLine)
                    {
                        msgBuffer[msgBufPtr++] = c;
                    }
                }
            }
        }
    }

    // no HandShake performed
    return false;
}

void WebSocket::SendChar(char message)
{
    if(client.connected())
    {
        // erstes byte ist 129
        client.write((uint8_t) 0x81);
        client.write(1);
        // and don't forget the message itself
        client.write(message);
    }
#ifdef DEBUG
    else
    {
        Serial.println("Cannot send message, client disconnected");
    }
#endif
}

void WebSocket::SendMessage(char *message, uint16_t length)
{
    // check if client is really connected
    if(client.connected())
    {
        // Das Startbyte ist immer 129
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
#ifdef DEBUG
    else
    {
        Serial.println("Cannot send message, client disconnected");
    }
#endif
}
