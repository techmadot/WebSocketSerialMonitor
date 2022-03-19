/*
   WebSocketServer.ino

    Created on: 22.05.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>   //https://github.com/Links2004/arduinoWebSockets/tree/async
#include <Hash.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>

#define SSID    "yourssid"
#define PASSWD  "yourpass"

IPAddress ip(192,168,0,1);
IPAddress gateway(192,168,0,254);
IPAddress netmask(255,255,255,0);

WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %s url: %s\n", num, ip.toString().c_str(), payload);

        // send message to client
        String msg = "Connected to Serial on " + WiFi.localIP().toString() + "\n";
        webSocket.sendTXT(num, msg);
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);

      // send message to client
      // webSocket.sendTXT(num, "message here");

      // send data to all connected clients
      // webSocket.broadcastTXT("message here");
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary lenght: %u\n", num, lenght);
      hexdump(payload, lenght);

      // send message to client
      // webSocket.sendBIN(num, payload, lenght);
      break;
  }

}


String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void setup() {
  int waiting = 0;  
  Serial.begin(115200);

  //Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, netmask);

  WiFi.begin(SSID, PASSWD);
  while(WiFi.status() != WL_CONNECTED) {
    delay(100);
    if (waiting % 10 == 0) Serial.print('.'); 
  }

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  Serial.println("START MIRRORING SERIAL");
  Serial.println(WiFi.localIP());

  //  Serial.setTimeout(500);
    inputString.reserve(256);
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      stringComplete = true;
      return;
    } else {
      inputString += inChar;
    }
  }
}


void loop() {
  serialEvent();
  if (stringComplete) {
    
    String line = inputString;
       // clear the string:
    inputString = "";
    stringComplete = false;

    //line += '\n';
    webSocket.broadcastTXT(line);
    Serial.println(line);
  }
  webSocket.loop();
  /*
    String line = Serial.readStringUntil('\n');
    if (line.length() > 0) {
      // add back line ending
      line += '\n';
      webSocket.broadcastTXT(line);
      Serial.print(line);
    }
    webSocket.loop();
  */
}

