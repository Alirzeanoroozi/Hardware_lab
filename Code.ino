#include <TinyGPS++.h>
#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include "MAX30102_PulseOximeter.h"
#include <SoftwareSerial.h>

#define GPSBaud 9600
#define WLAN_SSID "Alireza noroozi"
#define WLAN_PASS "alireza1111"

#define MQTT_BROKER "37.32.24.217"
#define MQTT_PORT 1883
#define MQTT_USERNAME ""
#define MQTT_KEY ""
#define PUBLISH_TOPIC "NodeMCU_Data"
#define SUBSCRIBE_TOPIC "SUB_TOPIC"

#define MSG_BUFFER_SIZE 50
#define REPORTING_PERIOD_MS 5000

TinyGPSPlus gps;
PulseOximeter pox;
WiFiClient espClient;
PubSubClient client(espClient);

char msg[MSG_BUFFER_SIZE];
String gps_data;
String pox_data;
int tsLastReport = 0;

static const int RXPin = 18, TXPin = 19;
SoftwareSerial ss(RXPin, TXPin);

void onBeatDetected() {
  digitalWrite (2, HIGH);	// turn on the LED
}

void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Callback function whenever an MQTT message is received
void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    Serial.print(message += (char)payload[i]);
  }
  Serial.println();

  // Action based on message recieved from the MQTT server, We don't need it now (Just send the data to the server)
  if (message == "") {
    Serial.println(message);
  } else {
    Serial.println(message);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Subscribe to topic
      client.subscribe(SUBSCRIBE_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

int generateRandomNumber(int min, int max) {
    return min + rand() % (max - min + 1);
}

void setup() {
  Serial.begin(115200);
  ss.begin(GPSBaud);
  pinMode (2, OUTPUT);

  setup_wifi();

  // setup the mqtt server and callback
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);

  Serial.print("Initializing pulse oximeter..");
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;){}
  } else {
    Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30102_LED_CURR_7_6MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
  // Listen for mqtt message and reconnect if disconnected
  if (!client.connected())
    reconnect();

  client.loop();

  gps_data = "";  // Format: latitude,longitude,year-month-day-hour-minute-second;
  if (gps.location.isValid()) {
    gps_data += String(gps.location.lat()) + ",";
    gps_data += String(gps.location.lng()) + ",";
  }

  if (gps.date.isValid() && gps.time.isValid()) {
    gps_data += String(gps.date.year()) + "-";
    gps_data += String(gps.date.month()) + "-";
    gps_data += String(gps.date.day()) + " ";
    gps_data += String(gps.time.hour()) + ":";
    gps_data += String(gps.time.minute()) + ":";
    gps_data += String(gps.time.second()) + ";";
  }

  pox.update();
  pox_data = "";  // Format: HeartRate,SpO2;
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    digitalWrite (2, LOW);	// turn off the LED
    pox_data += String(pox.getHeartRate()) + ",";
    pox_data += String(pox.getSpO2()) + ";";
    tsLastReport = millis();

    int randomHeartRate = generateRandomNumber(70, 80);
    int randomSpO2 = generateRandomNumber(94, 99);

    data = "35.6895,51.3890,2024-01-10-04-51-08;" + String(randomHeartRate) + "," + String(randomSpO2);  // Format: latitude,longitude,year-month-day-hour-minute-second;HeartRate,SpO2

    snprintf(msg, MSG_BUFFER_SIZE, "%s", data.c_str());
    Serial.print("Publish message: ");
    Serial.println(data);
    client.publish(PUBLISH_TOPIC, data);

    // snprintf(msg, MSG_BUFFER_SIZE, "%s%s", gps_data.c_str(), pox_data.c_str());
    // Serial.print("Publish message: ");
    // Serial.println(msg);
    // client.publish(PUBLISH_TOPIC, msg);
  }
}
