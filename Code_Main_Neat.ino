#include <TinyGPS++.h>
#include <Wire.h>
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "MAX30100_PulseOximeter.h"
#include <PubSubClient.h>

#define GPS_BAUDRATE 9600
#define WLAN_SSID  "alireza noroozi"
#define WLAN_PASS  "alireza1111"
#define AIO_UPDATE_RATE_SEC 5
#define MQTT_BROKER  "37.32.24.217"
#define MQTT_PORT  1883
#define MQTT_USERNAME  ""
#define MQTT_KEY ""
#define PUBLISH_TOPIC "NodeMCU_Data"
#define SUBSCRIBE_TOPIC "SUB_TOPIC" // We don't need it now
#define MSG_BUFFER_SIZE (5)
#define I2C_SDA 32
#define I2C_SCL 33
#define REPORTING_PERIOD_MS     1000

TinyGPSPlus gps; 
PulseOximeter pox;
WiFiClient espClient;

PubSubClient client(espClient);
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
String gps_data;
String pox_data;
uint32_t tsLastReport = 0;
// Adafruit_MQTT_Client mqtt(&espClient, MQTT_BROKER, MQTT_PORT, MQTT_USERNAME, MQTT_KEY); This is for Adafruit
// Adafruit_MQTT_Subscribe sw_sub = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "/feeds/switch"); This is for Adafruit
// Adafruit_MQTT_Publish bpm_pub = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/feeds/bpm"); This is for Adafruit
// Adafruit_MQTT_Publish spo2_pub = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/feeds/SpO2"); This is for Adafruit

void onBeatDetected() {
  Serial.println("Heartbeat detected!");
}

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Callback function whenever an MQTT message is received
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++)
  {
    Serial.print(message += (char)payload[i]);
  }
  Serial.println();

  // Action based on message recieved from the MQTT server, We don't need it now (Just send the data to the server)
  if (message == "")
  {
    Serial.println(message);
  }
  else {
    Serial.println(message);
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Subscribe to topic
      client.subscribe(SUBSCRIBE_TOPIC);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(GPS_BAUDRATE);
  Wire.begin(I2C_SDA, I2C_SCL);
  
  setup_wifi();

  // setup the mqtt server and callback
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);
  // mqtt.subscribe(&sw_sub); This is for Adafruit

  Serial.print("Initializing pulse oximeter..");

  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;) {
    }
  } else {
    Serial.println("SUCCESS");
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
  gps_data = ""; // Format: latitude,longitude,altitude,speed,Date;
  if (Serial2.available() > 0) {
    if (gps.encode(Serial2.read())) {
      if (gps.location.isValid()) {
        Serial.print("- latitude: ");
        Serial.println(gps.location.lat());
        gps_data += String(gps.location.lat()) + ",";

        Serial.print("- longitude: ");
        Serial.println(gps.location.lng());
        gps_data += String(gps.location.lng()) + ",";

        Serial.print("- altitude: ");
        if (gps.altitude.isValid()) {
          Serial.println(gps.altitude.meters());
          gps_data += String(gps.altitude.meters()) + ",";
        } else {
          Serial.println("INVALID");
        }
      } else {
        Serial.println("- location: INVALID");
      }

      Serial.print("- speed: ");
      if (gps.speed.isValid()) {
        Serial.print(gps.speed.kmph());
        Serial.println(" km/h");
        gps_data += String(gps.speed.kmph()) + ",";
      } else {
        Serial.println("INVALID");
      }

      Serial.print("- GPS date&time: ");
      if (gps.date.isValid() && gps.time.isValid()) {
        Serial.print(gps.date.year());
        gps_data += String(gps.date.year()) + "-";
        Serial.print("-");
        Serial.print(gps.date.month());
        gps_data += String(gps.date.month()) + "-";
        Serial.print("-");
        Serial.print(gps.date.day());
        gps_data += String(gps.date.day()) + "-";
        Serial.print(" ");
        Serial.print(gps.time.hour());
        gps_data += String(gps.time.hour()) + "-";
        Serial.print(":");
        Serial.print(gps.time.minute());
        gps_data += String(gps.time.minute()) + "-";
        Serial.print(":");
        Serial.println(gps.time.second());
        gps_data += String(gps.time.second()) + ";";
      } else {
        Serial.println("INVALID");
      }

      Serial.println();
    }
  }

  // Loop code for pulse oximeter goes here
  pox.update();
  pox_data = ""; // Format: HeartRate,bpm/SpO2;
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print("Heart rate:");
    Serial.print(pox.getHeartRate());
    pox_data += String(pox.getHeartRate()) + ",";
    Serial.print("bpm / SpO2:");
    Serial.print(pox.getSpO2());
    pox_data += String(pox.getSpO2()) + ";";
    Serial.println("%");
    tsLastReport = millis();
  }

  // Listen for mqtt message and reconnect if disconnected
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  // publish message after certain time.
  unsigned long now = millis();
  if (now - lastMsg > 10000)
  {
    lastMsg = now;

    snprintf(msg, MSG_BUFFER_SIZE, "%s%s", gps_data.c_str(), pox_data.c_str());
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(PUBLISH_TOPIC, msg);
  }

}
