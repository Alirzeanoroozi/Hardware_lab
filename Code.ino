#include <TinyGPS++.h>
#define GPS_BAUDRATE 9600  // The default baudrate of NEO-6M is 9600

TinyGPSPlus gps;  // the TinyGPS++ object

void setup() {
  Serial.begin(9600);
  Serial2.begin(GPS_BAUDRATE);

  Serial.println(F("ESP32 - GPS module"));
}

void loop() {
  if (Serial2.available() > 0) {
    if (gps.encode(Serial2.read())) {
      if (gps.location.isValid()) {
        Serial.print(F("- latitude: "));
        Serial.println(gps.location.lat());

        Serial.print(F("- longitude: "));
        Serial.println(gps.location.lng());

        Serial.print(F("- altitude: "));
        if (gps.altitude.isValid())
          Serial.println(gps.altitude.meters());
        else
          Serial.println(F("INVALID"));
      } else {
        Serial.println(F("- location: INVALID"));
      }

      Serial.print(F("- speed: "));
      if (gps.speed.isValid()) {
        Serial.print(gps.speed.kmph());
        Serial.println(F(" km/h"));
      } else {
        Serial.println(F("INVALID"));
      }

      Serial.print(F("- GPS date&time: "));
      if (gps.date.isValid() && gps.time.isValid()) {
        Serial.print(gps.date.year());
        Serial.print(F("-"));
        Serial.print(gps.date.month());
        Serial.print(F("-"));
        Serial.print(gps.date.day());
        Serial.print(F(" "));
        Serial.print(gps.time.hour());
        Serial.print(F(":"));
        Serial.print(gps.time.minute());
        Serial.print(F(":"));
        Serial.println(gps.time.second());
      } else {
        Serial.println(F("INVALID"));
      }

      Serial.println();
    }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
}

#include <stdint.h>
#include <Wire.h>
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "MAX30100_PulseOximeter.h"


#define WLAN_SSID  "alireza noroozi"
#define WLAN_PASS  "alireza1111"


#define AIO_UPDATE_RATE_SEC 5
#define AIO_SERVER  "io.adafruit.com"
#define AIO_SERVERPORT  1883                
#define AIO_USERNAME  "noroozi"
#define AIO_KEY "aio_GyLd629YBIxs1Xg3ODaXX4oK7Seu"

#define I2C_SDA 32
#define I2C_SCL 33


uint32_t tsLastReport = 0;
float bpm_dt=0;
float spo2_dt = 0;

PulseOximeter pox;
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);         
Adafruit_MQTT_Subscribe sw_sub = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/switch");
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish bpm_pub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/bpm");
Adafruit_MQTT_Publish spo2_pub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/SpO2");

void onBeatDetected() {
  // This function is called when a heartbeat is detected
  Serial.println("Heartbeat detected!");

  // Add your custom code here to respond to the heartbeat detection
  // For example, you could increment a counter, toggle an LED, etc.
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  mqtt.subscribe(&sw_sub);
  Serial.print("Initializing pulse oximeter..");
  // Initialize the PulseOximeter instance
  // Failures are generally due to an improper I2C wiring, missing power supply
  // or wrong target chip
  if (!pox.begin()) {
    Serial.println("FAILED");
    for(;;);
  } else {
    Serial.println("SUCCESS");
  }
  // The default current for the IR LED is 50mA and it could be changed
  //   by uncommenting the following line. Check MAX30100_Registers.h for all the
  //   available options.
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  // Register a callback for the beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected);
  stopReadPOX();
}

void loop() {
  MQTT_connect();
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))){
    if (subscription == &sw_sub){
      Serial.print(F("Got: "));
      Serial.println((char *)sw_sub.lastread);
      if (!strcmp((char*) sw_sub.lastread, "ON")){
        Serial.print(("Starting POX... "));
        startReadPOX();
        BaseType_t xReturned;
        if(poxReadTaskHld == NULL){
          xReturned = xTaskCreate(
                      poxReadTask, /* Function that implements the task. */
                      "pox_read", /* Text name for the task. */
                      1024*3,    /* Stack size in words, not bytes. */
                      NULL,  /* Parameter passed into the task. */
                      2,/* Priority at which the task is created. */
                      &poxReadTaskHld ); /* Used to pass out the created task's handle. */
        }
        delay(100);
        if(mqttPubTaskHld == NULL){
        xReturned = xTaskCreate(
                    mqttPubTask, /* Function that implements the task. */
                    "mqttPub",  /* Text name for the task. */
                    1024*3,    /* Stack size in words, not bytes. */
                    NULL, /* Parameter passed into the task. */
                    2,/* Priority at which the task is created. */
                    &mqttPubTaskHld ); /* Used to pass out the created task's handle. */
        }
      }
    }
    else{
      Serial.print(("Stoping POX... "));
      // Detele POX read task
      if(poxReadTaskHld != NULL)
      vTaskDelete(poxReadTaskHld);
      poxReadTaskHld = NULL;
    }
    // Delete the MQTT Pub Task
    if(mqttPubTaskHld != NULL){
      vTaskDelete(mqttPubTaskHld);
      mqttPubTaskHld = NULL;
    }
    stopReadPOX();
  }
}


/*
Arduino-MAX30100 oximetry / heart rate integrated sensor library
Copyright (C) 2016  OXullo Intersecans <x@brainrapers.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS     1000

// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation
PulseOximeter pox;

uint32_t tsLastReport = 0;

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("Beat!");
}

void setup()
{
    Serial.begin(115200);

    Serial.print("Initializing pulse oximeter..");

    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }

    // The default current for the IR LED is 50mA and it could be changed
    //  by uncommenting the following line. Check MAX30100_Registers.h for all the
    //  available options.
    // pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

    // Register a callback for the beat detection
   pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
    // Make sure to call update as fast as possible
    pox.update();

    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
       Serial.print("bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.println("%");
 tsLastReport = millis();
}
}
