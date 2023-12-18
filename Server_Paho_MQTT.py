# Just run this code on the server with "python3 mqtt_listener.py" command

import paho.mqtt.client as mqtt

# MQTT broker information
MQTT_BROKER = "localhost"
MQTT_PORT = 1883
SUBSCRIBE_TOPIC = "NodeMCU_Data"
PUBLISH_TOPIC = "Backend_Data"

def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe(SUBSCRIBE_TOPIC)

def on_message(client, userdata, msg):
    received_message = msg.payload.decode("utf-8")
    print(f"Received message on {msg.topic}: {received_message}")

    # Process the received message as needed
    # For example, you can perform some logic and create a new message
    new_message = f"Processed: {received_message}"

    # Publish the new message to a different topic
    client.publish(PUBLISH_TOPIC, new_message)
    print(f"Published message on {PUBLISH_TOPIC}: {new_message}")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Blocking call that processes network traffic, dispatches callbacks, and handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a manual interface.
client.loop_forever()
