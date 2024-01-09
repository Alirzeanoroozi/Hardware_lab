#include <TinyGPS++.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

#define GPSBaud 9600
#define WLAN_SSID  "Alireza noroozi"
#define WLAN_PASS  "alireza1111"

#define AIO_UPDATE_RATE_SEC 5

#define MQTT_BROKER  "37.32.24.217"
#define MQTT_PORT  1883
#define MQTT_USERNAME  ""
#define MQTT_KEY ""
#define PUBLISH_TOPIC "NodeMCU_Data"
#define SUBSCRIBE_TOPIC "SUB_TOPIC" // We don't need it now

#define MSG_BUFFER_SIZE 50
#define REPORTING_PERIOD_MS     1000

TinyGPSPlus gps; 
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
String gps_data;

static const int RXPin = 22, TXPin = 23;
SoftwareSerial ss(RXPin, TXPin);

void setup_wifi()
{
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

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

int generateRandomNumber(int min, int max) {
    return min + rand() % (max - min + 1);
}

void setup() {
  Serial.begin(115200);
  ss.begin(GPSBaud);
  
  setup_wifi();

  // setup the mqtt server and callback
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);
}

void loop() {
  gps_data = ""; // Format: latitude,longitude,altitude,speed,Date;
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

  // Listen for mqtt message and reconnect if disconnected
  if (!client.connected())
    reconnect();
    
  client.loop();

  // publish message after certain time.
  unsigned long now = millis();
  if (now - lastMsg > 10000)
  {
    lastMsg = now;

    int randomHeartRate = generateRandomNumber(70, 80);
    int randomSpO2 = generateRandomNumber(97, 99);

    data = "35.6895,51.3890,2024-01-10-04-51-08;" + String(randomHeartRate) + "," + String(randomSpO2);  // Format: latitude,longitude,year-month-day-hour-minute-second;

    snprintf(msg, MSG_BUFFER_SIZE, "%s", data.c_str());
    Serial.print("Publish message: ");
    Serial.println(data);
    client.publish(PUBLISH_TOPIC, data);
  }
}

