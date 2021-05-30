#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

#include "credentials.h"

const char *ssid = WIFI_SSID;
const char *pswd = WIFI_PSWD;

#define VERSION "nodemcu.hpg_flow v0.1.0.0.2"

uint32_t time_since_update = -1;

float calibrationFactor = 4.5;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
uint8_t sensor_pin = D2;
uint8_t update_pin = D1;

#define MAX_MSG_LEN (128)
#define N_LEDS 4
long lastMsg = 0;
int led_pin[] = {5, 13, 14, 15};
const char* host = "NodeMCU";
const IPAddress serverIPAddress(192, 168, 0, 100);
const char *topic = "test/message";
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');
  Serial.println("Booting");
  Serial.print("Software Version: ");
  Serial.println(VERSION);
  
  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;

  pinMode(sensor_pin, INPUT);
  digitalWrite(sensor_pin, HIGH);
  pinMode(led_pin[1], OUTPUT);
  digitalWrite(led_pin[1], LOW);

  connectWifi();

  delay(2000);
  checkforupdate();

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
    for (int i = 0; i < N_LEDS; i++) {
      analogWrite(led_pin[i], 0);
    }
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    analogWrite(led_pin[1], 0);
    delay(50);
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    analogWrite(led_pin[2], (progress / (total / 990)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    //analogWrite(led_pin[0], 990);
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  
  ArduinoOTA.begin();
  Serial.println("OTA ready");

  attachInterrupt(digitalPinToInterrupt(sensor_pin), pulseCounter, FALLING);

  client.setServer(serverIPAddress, 1883);
  client.setCallback(callback);
//  pinMode(led, OUTPUT);
//  digitalWrite(led, 1);
}

//unsigned long previousTime = millis();

//const unsigned long interval = 500;
void connectWifi() {
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PSWD);
  analogWriteRange(1000);
  Serial.println("Connecting ...");
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    analogWrite(led_pin[3], 990);
    delay(10);
    analogWrite(led_pin[3], 0);
    Serial.print('.');
  }
  analogWrite(led_pin[3], 0);
  analogWrite(led_pin[1], 990);

  ArduinoOTA.setHostname(host);  
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer
}

void listenMQTT() {
  // Wait until we're connected
  while (!client.connected()) {
    // Create a random client ID
    String clientId = "ESP8266-testclient";
    // clientId += String(random(0xffff), HEX);
    Serial.printf("MQTT connecting as client %s...\n", clientId.c_str());
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT connected");
      // Once connected, publish an announcement...
      client.publish(topic, "hello from ESP8266");
      // ... and resubscribe
      client.subscribe(topic);
    } 
    else {
      Serial.printf("MQTT failed, state %s, retrying...\n", client.state());
      // Wait before retrying
      delay(2500);
    }
  }
}

void pubMQTT() {
  char sensor_str[8];
  float sensor_value = analogRead(sensor_pin);
  dtostrf(sensor_value, 2, 3, sensor_str);
  //client.publish(topic, sensor_str);
  Serial.println(sensor_value);
}

void callback(char *msgTopic, byte *msgPayload, unsigned int msgLength) {
  // copy payload to a static string
  static char message[MAX_MSG_LEN+1];
  if (msgLength > MAX_MSG_LEN) {
    msgLength = MAX_MSG_LEN;
  }
  strncpy(message, (char *)msgPayload, msgLength);
  message[msgLength] = '\0';
  
  Serial.printf("topic %s, message received: %s\n", msgTopic, message);

  // decode message
  if (strcmp(message, "off") == 0) {
    setLedState(false);
  } 
  else if (strcmp(message, "on") == 0) {
    setLedState(true);
  }
}

void setLedState(boolean state) {
  // LED logic is inverted, low means on
  if(state) {
    analogWrite(led_pin[0], 990);
  }
  else {
    analogWrite(led_pin[0], 0);
  }
}

void checkforupdate() {
  ESPhttpUpdate.rebootOnUpdate(false);
  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
  t_httpUpdate_return ret = ESPhttpUpdate.update(serverIPAddress.toString(), 80, "/update.php", VERSION);
  switch(ret) {
    case HTTP_UPDATE_FAILED:
        Serial.print("HTTP_UPDATE_FAILED Error ");
        Serial.print(ESPhttpUpdate.getLastError());
        Serial.print(" : ");
        Serial.println(ESPhttpUpdate.getLastErrorString().c_str());
        time_since_update = millis()-85800000;    // check again in 10 minutes
        break;
    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("[update] Update no Update.");
        break;
    case HTTP_UPDATE_OK:
        Serial.println("[update] Update ok."); // may not be called due to reboot
        break;
  }
  if(ret != HTTP_UPDATE_FAILED) {
    time_since_update = millis();
    if(ret != HTTP_UPDATE_NO_UPDATES) ESP.restart();
  }
}

void loop() {
  analogWrite(led_pin[1], 0);
  ArduinoOTA.handle();
  if (!client.connected()) {
    listenMQTT();
    }
// this is ESSENTIAL for MQTT to function
  client.loop();
  if(digitalRead(update_pin) == 0) checkforupdate();
  char sensor_str[8];
  long now = millis();
  if (now - lastMsg > 1000) {
    detachInterrupt(digitalPinToInterrupt(sensor_pin));
    flowRate = ((1000.0 / (millis() - lastMsg)) * pulseCount) / calibrationFactor;
    lastMsg = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;
    unsigned int frac;
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));
    Serial.print(".");
    frac = (flowRate - int(flowRate)) * 10;
    Serial.print(frac, DEC) ;
    Serial.print("L/min");
    Serial.print("  Current Liquid Flowing: ");
    Serial.print(flowMilliLitres);
    Serial.print("mL/Sec");
    dtostrf(flowMilliLitres, 2, 3, sensor_str);
    client.publish(topic, sensor_str);
    Serial.print("  Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.println("mL");
    pulseCount = 0;
    char sensor_str[8];
    float sensor_value = analogRead(sensor_pin);
    dtostrf(sensor_value, 2, 3, sensor_str);
    // client.publish(topic, sensor_str);
    // Serial.println(sensor_value);
    attachInterrupt(digitalPinToInterrupt(sensor_pin), pulseCounter, FALLING);
    if(millis() - time_since_update > 86400000) checkforupdate();
  }
// idle
  delay(400);
// unsigned long diff = millis() - previousTime;
// if(diff > interval) {
//   digitalWrite(led, !digitalRead(led));  // Change the state of the LED
//   previousTime += diff;
}

void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
