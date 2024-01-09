# python manage.py start_mqtt_listener

import time
import paho.mqtt.client as mqtt
from django.conf import settings
from app.models import HealthHistory

def on_connect(client, userdata, flags, rc):
    # HealthHistory.objects.all().delete()
    print("Connected with result code " + str(rc))
    client.subscribe("Backend_Data")  # The MQTT topic we want to subscribe to

def on_message(client, userdata, msg):
    payload = msg.payload.decode("utf-8")
    print(f"Received message: {payload}")

    # Parse the payload and create a new instance of HealthHistory
    coordinates, health_data = payload.split(';')

    location_x, location_y, timestamp = coordinates.split(',')
    SPO2, BPM = health_data.split(',')

    HealthHistory.objects.create(
        location_x=float(location_x),
        location_y=float(location_y),
        SPO2=int(SPO2),
        BPM=int(BPM)
    )

def start_mqtt_listener():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    # Set up MQTT broker connection details
    client.connect(settings.MQTT_BROKER_HOST, settings.MQTT_BROKER_PORT, 60)

    # Start the MQTT listener loop and run in a loop with a delay
    while True:
        try:
            client.loop_start()
            time.sleep(1)  # Adjust the delay as needed
        except KeyboardInterrupt:
            print("MQTT listener stopped by user.")
            break
