#include <ESP8266WiFi.h>

/*
 * https://techtutorialsx.com/2016/10/03/esp8266-setting-a-simple-http-webserver/
 * https://techtutorialsx.com/2017/03/26/esp8266-webserver-accessing-the-body-of-a-http-request/
 * This is the client (transmitter) device. It connects to the 24VAC relays which connect
 * to the irrigation controller.
 * See full project on: www.makeitbreakitfixit.com
 */

const char *ssid = "irrigatorfi";
const char *password = "chooseabetterpass4SECURITY";

const int led = 5;     // LED pin
const int button = 16; // push button is connected

const int leds[] = {0, 2, 4, 15, 3}; // pins the LEDs sit on to indicate when relay is triggered
const int buttons[] = {12, 13, 14, 5, 16}; // pins the relays sit on
const String messages[] = {"one", "two", "three", "four", "five"}; // messages to send to Receiver for each relay
const int numOfButtons = (sizeof(buttons) / sizeof(int)); // this is how you get size of the array itself
bool pressed = false; // hold state of button press
int counter = 0; // Count number of failed connection attempts to HTTP srv

void setup() {
  Serial.begin(115200);
  delay(10);
  for (int i = 0; i < numOfButtons; i++) {
    pinMode(leds[i], OUTPUT);   // declare LED as output
    pinMode(buttons[i], INPUT); // declare push button as input
    digitalWrite(leds[i], HIGH);
    delay(300);
  }
  for (int x = 0; x < numOfButtons; x++) {
    delay(300);
    digitalWrite(leds[x], LOW);
  }
  Serial.println("\nSTARTING...");
  connectToESP();
}

void loop() {
  for (int i = 0; i < numOfButtons; i++) {
    pressed = readButtons(i); // find out if button was pressed
    if (pressed == true) {
      digitalWrite(leds[i], HIGH);
      talkToServer(String(messages[i]) + "-on");
    } else {
      digitalWrite(leds[i], LOW);
      talkToServer(String(messages[i]) + "-off");
    }
  }
  delay(100);
}

///////////////////////////////////////////////////////////////////////////////////////

boolean readButtons(int x) {
  int butState = digitalRead(buttons[x]);
  if (butState == HIGH) {
    delay(2);
    if (butState == HIGH) { // ensure not a false positive
      Serial.println("Relay "+String(x)+" [B:"+buttons[x]+"|L:"+leds[x]+"]: Push button detected. LED On");
      return true;
    }
  }
  return false; 
}

void talkToServer(String msg) {
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const char * host = "192.168.4.1";
  const int httpPort = 80;

  if (!client.connect(host, httpPort)) {
    counter++;
    if (counter >= 200) {
      Serial.println();
      Serial.print("Reconnecting...\n");
      connectToESP();
      counter = 0;
    }
    Serial.println("Connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/data/";
  url += "?" + msg + "=";
  url += "blah";

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(100);
  //unsigned long timeout = millis();
  //while (client.available() == 0) {
  //  if (millis() - timeout > 5000) {
  //    Serial.println(">>> Client Timeout !");
  //    client.stop();
  //    return;
  //  }
  //}
  //Serial.println("Closing connection");
}

void connectToESP() {
    // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  for (int x = 0; x < 5; x++) { // flash LEDs to indicate connected to Receiver
  for (int i = 0; i < numOfButtons; i++) {
    digitalWrite(leds[i], HIGH);
  }
  delay(300);
  for (int i = 0; i < numOfButtons; i++) {
    digitalWrite(leds[i], LOW);
  }
  delay(100);
}

}
