
#include <Arduino.h>
#include <WebSocketsServer.h>
#include <Hash.h>

#include <Adafruit_NeoPixel.h>

#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <ESP8266mDNS.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#define PIXEL_PIN 5
#define N_LEDS 60

// Game vars
int players[] = {0,0};
int health[] = {10, 10};

bool inGame = false;


Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

ESP8266WebServer server = ESP8266WebServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
      
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
        break;
        
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            if(num > 1){
              Serial.println("Too many players!");
              webSocket.sendTXT(num, "Too many players");
            }

            Serial.printf("Player [%u] connected!", num);

            // send message to client
            webSocket.sendTXT(num, "Connected");
        }
        break;
        
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);

            if(payload[0] == '#') {
                
            }

        break;
    }

}

void gameloop(){
    
}

void setup() {
    
    //Serial.setDebugOutput(true);

    Serial.println();
    Serial.println();
    Serial.println();

    Serial.begin(9600);
  
    /*
     *  Wifi Manager
     */
    WiFiManager wifiManager;
    wifiManager.autoConnect("Miso", "atrafaatrafa");
  
    /*
     * Neopixels
     */
    strip.begin();
    strip.setBrightness(25); // 0 - 255
    strip.show(); // Initialize all pixels to 'off' 
  
    /*
     * Mdns
     */
     if (!MDNS.begin("esp8266")) {
      Serial.println("Error setting up MDNS responder!");
      while(1) { 
        delay(1000);
      }
    }
    Serial.println("mDNS responder started");
    
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);

    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    // start webSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    // handle index
    server.on("/", []() {
        // send index.html
        server.send(200, "text/html", "<!DOCTYPE html> <html> <head> <script>function sendHit(){connection.send('Hit')}var connection=new WebSocket('ws://'+location.hostname+':81/',['arduino']);connection.onopen=function(){connection.send('Connect '+new Date)},connection.onerror=function(n){console.log('WebSocket Error ',n)},connection.onmessage=function(n){console.log('Server: ',n.data)}</script> <title>1D-Pong!</title> </head> <body> <form> <button type='button' onclick='sendHit()'>Click me</button> </form> </body> </html>");
    });

    server.begin();

}

void loop() {
    webSocket.loop();
    server.handleClient();
}

