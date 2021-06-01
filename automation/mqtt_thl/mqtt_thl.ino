#include <ESP8266httpUpdate.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <MCP3008.h>

#include "credentials.h"

const char *ssid1 = WIFI_SSID1;
const char *ssid2 = WIFI_SSID2;
const char *ssid3 = WIFI_SSID3;
const char *pswd = WIFI_PSWD;
const char *mqtt_user = MQTT_USER;
const char *mqtt_pswd = MQTT_PSWD;

#define VERSION "nodemcu.thl_env v0.5"

uint32_t time_since_update = -1;

uint8_t clk_pin = D5;
uint8_t miso_pin = D6;
uint8_t mosi_pin = D7;
uint8_t cs_pin = D8;
MCP3008 adc(clk_pin, mosi_pin, miso_pin, cs_pin);

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
const uint32_t connectTimeoutMs = 5000;

#define DHTTYPE DHT22
uint8_t dht_pin = D2;
uint8_t update_pin = D1;
DHT dht(dht_pin, DHTTYPE);

#define MAX_MSG_LEN (256)
long lastMsg = 0;
const char *datatopic = "sensors/gautam/";
const char *logtopic = "log/gautam";
const char* host = "NodeMCU";
const IPAddress serverIPAddress(192, 168, 0, 100);
const IPAddress updateIPAddress(192, 168, 0, 101);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

float temperature;
float humidity;
float heatIndex;
float ambientLight;

void setup() {
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(2000);
  Serial.flush();
  delay(10);
  Serial.println('\n');
  Serial.println("Booting");
  Serial.print("Software Version: ");
  Serial.println(VERSION);

  pinMode(dht_pin, INPUT);
  pinMode(update_pin, INPUT_PULLUP);

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);

  mqttClient.setServer(serverIPAddress, 1883);
  mqttClient.setCallback(mqttCallback);

  connectWifi(0);

  ESPhttpUpdate.onStart(onStart);
  ESPhttpUpdate.onEnd(onEnd);
  ESPhttpUpdate.onProgress(onProgress);
  ESPhttpUpdate.onError(onError);

  checkforupdate();
  connectMQTT(0);

  dht.begin();
}

void onStart() {
  Serial.println("Updating");
}

void onEnd() {
  Serial.println("\nUpdate downloaded successfully");
}

void onProgress(unsigned int progress, unsigned int total) {
  Serial.printf("Progress: %u%%\r\n", (progress / (total / 100)));
}

void onError(int err) {
  Serial.printf("Couldn't update. Error: %d\n", err);
}

uint32_t previousTime = millis();

void blink(int n) {
  for (int i=0; i<n; ++i) {
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
    delay(100);                      // Wait for a second
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    delay(100);
  }
}

int connectWifi(int mode) {
  if (mode == 0) {
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(ssid1, pswd);
    wifiMulti.addAP(ssid2, pswd);
    wifiMulti.addAP(ssid3, pswd);
    Serial.print("\nConnecting ...");
  }
  int status;
  int ntry = 0;
  status = WiFi.status();
  if (mode == 2) {
    if (status == WL_CONNECTED) return status;
    else Serial.printf("\nWifi disconnected. Retrying connection ...\n");
  }
  do {
    status = wifiMulti.run();
    if (status) {
      Serial.print("\nConnected to ");
      Serial.println(WiFi.SSID());                              // Tell us what network we're connected to
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());                           // Send the IP address of the ESP8266 to the computer
      Serial.print("MAC Address: ");
      Serial.println(WiFi.macAddress());                        // Send the MAC address of the ESP8266 to the computer
    }
    else {
      Serial.print('.');
      blink(2);
      delay(5000);
    }
  } while ((status != WL_CONNECTED) && (++ntry <= 10));

  if (ntry > 10) {
    blink(2);
    delay(60000);
    status = connectWifi(1);
  }
  return status;
}

bool connectMQTT(int mode) {
  String clientId = "ESP8266-Gautam-Room";                 // Create a client ID
  //  clientId += String(random(0xffff), HEX);

  if (mode == 0) {
    Serial.printf("\nMQTT connecting as client %s ...", clientId.c_str());
  }

  bool status;
  int ntry = 0;

  status = mqttClient.connected();

  if (mode == 2) {
    if (status) return status;
    else Serial.printf("\nMQTT connection failed, state %d, retrying...\n", mqttClient.state());
  }

  do { // Wait for the MQTT to connect
    status = mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pswd);
    if (status) {
      Serial.println("\nMQTT connected");
      mqttClient.publish(logtopic, "ESP8266-Gautam-Room connected");
    }
    else {
      Serial.print('.');
      blink(1);
      delay(5000);
    }
  } while (!status && (++ntry <= 10));

  if (ntry > 10) {
    Serial.printf("\nMQTT connection failed, state %d, retrying...\n", mqttClient.state());
    connectWifi(2);
    blink(1);
    delay(30000);
    status = connectMQTT(1);
  }

  return status;
}

void pubMQTT(char *names[], float vals[]) {
  char value[8] = {'\0'};
  char topic[MAX_MSG_LEN] = {'\0'};

  for (int i=0; i<4; ++i) {
    dtostrf(vals[i], 3, 2, value);
    strcpy(topic, datatopic);
    strcat(topic,names[i]);
    mqttClient.publish(topic, value);
//    Serial.println(topic);
//    Serial.println(value);
  }

  // Serial.println(" ");
}


void mqttCallback(char *msgTopic, byte *msgPayload, unsigned int msgLength) {
  // copy payload to a static string
  static char message[MAX_MSG_LEN + 1];
  if (msgLength > MAX_MSG_LEN) {
    msgLength = MAX_MSG_LEN;
  }
  strncpy(message, (char *)msgPayload, msgLength);
  message[msgLength] = '\0';

  Serial.printf("topic %s, message received: %s\n", msgTopic, message);

  // decode message

}

void checkforupdate() {
  ESPhttpUpdate.rebootOnUpdate(false);
  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
  t_httpUpdate_return ret = ESPhttpUpdate.update(espClient, updateIPAddress.toString(), 80, "/update.php", VERSION);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.print("HTTP_UPDATE_FAILED Error ");
      Serial.print(ESPhttpUpdate.getLastError());
      Serial.print(" : ");
      Serial.println(ESPhttpUpdate.getLastErrorString().c_str());
      time_since_update = millis() - 85800000;  // check again in 10 minutes (86400000 - 600000)
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("[update] No update available.");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("[update] Update ok."); // may not be called due to reboot
      break;
  }
  if (ret != HTTP_UPDATE_FAILED) {
    time_since_update = millis();
    if (ret != HTTP_UPDATE_NO_UPDATES) {
      delay(1000);
      ESP.restart();
    }
  }
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  if (connectWifi(2) == WL_CONNECTED) {
    if (digitalRead(update_pin) == 0) checkforupdate();
    if (connectMQTT(2)) {
      temperature = dht.readTemperature(); // Gets the values of the temperature
      humidity = dht.readHumidity(); // Gets the values of the humidity
      heatIndex = dht.computeHeatIndex(temperature, humidity, false); // Gets Heat Index | false for Celcius
      ambientLight = adc.readADC(0); // read Channel 0 from MCP3008 ADC (pin 1)
      ambientLight *= 0.64453125; // 2 mA = 1 lx for TEMT6000
      char *names[4] = {"temperature", "humidity", "heat_index", "lighting"};
      float vals[4] = {temperature, humidity, heatIndex, ambientLight};
      long timePassed = millis() - lastMsg;
      if (timePassed - 2000 < 0) delay(2000 - timePassed);
      lastMsg = millis();
      pubMQTT(names, vals);
      if (millis() - time_since_update > 86400000) checkforupdate();
      mqttClient.loop();
    }
  }
}
