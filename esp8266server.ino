#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

/*
 * This is the server (reciever) device. It connects to the 5VDC relays which connect
 * to the solenoid valves where the water source and polypipe zones are.
 * See full project on: www.makeitbreakitfixit.com
 */

const char *ssid = "irrigatorfi";
const char *password = "chooseabetterpass4SECURITY";

const int led = 5;     // LED pin

int timeout = 30; // number of seconds to wait for Client/Transmitter before assuming disconnect
const int leds[] = {0, 2, 4, 15, 3}; // pins the LEDs sit on to indicate when relay is triggered
const int relays[] = {12, 13, 14, 5, 16}; // pins the relays sit on
const String messages[] = {"one", "two", "three", "four", "five"}; // messages to send to Receiver for each relay
int numOfRelays = (sizeof(relays) / sizeof(int)); // this is how you get size of the array itself
unsigned long currentMillis = millis(); // hold how long code been running without receiving data from Client/Transmitter

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/data/", HTTP_GET, handleSentVar); // when the server receives a request with /data/ in the string then run the handleSentVar function
  server.begin();
  Serial.println("HTTP server started");

  for (int i = 0; i < numOfRelays; i++) {
    pinMode(leds[i], OUTPUT);   // declare LED as output
    pinMode(relays[i], OUTPUT); // declar relays as output
    digitalWrite(relays[i], LOW);
    digitalWrite(leds[i], HIGH);
    delay(300);
  }
  for (int x = 0; x < numOfRelays; x++) {
    delay(300);
    digitalWrite(leds[x], LOW);
    digitalWrite(relays[x], HIGH);
  }
  Serial.println("\nSTARTING...");
  Serial.println("Configuring access point...");
}

void loop() {
  server.handleClient();
  if (millis() - currentMillis >= (timeout * 1000)) {
    Serial.println("Haven't heard from client for "+String(timeout)+" seconds. Switching off relays.");
    for (int i = 0; i < numOfRelays; i++) {
      digitalWrite(leds[i], LOW);
      digitalWrite(relays[i], HIGH);
    }
    currentMillis = millis();
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void handleSentVar() {
  currentMillis = millis(); // reset timer for inactivity
  for (int i = 0; i < numOfRelays; i++) {
    if (server.hasArg(String(messages[i]) + "-off")) { // this is the variable sent from the client.
      Serial.println("Relay "+String(i)+": Button not pressed... LED OFF");
      digitalWrite(leds[i], LOW);
      digitalWrite(relays[i], HIGH);
      server.send(200, "text/html", "Data received");
    }
  
    if (server.hasArg(String(messages[i]) + "-on")) { // this is the variable sent from the client.
      Serial.println("Relay "+String(i)+": Button pressed...LED ON");
      digitalWrite(leds[i], HIGH);
      digitalWrite(relays[i], LOW);
      server.send(200, "text/html", "Data received");
    }
  }
}
