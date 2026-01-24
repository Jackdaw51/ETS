# REQUISITES:
## sudo apt install mosquitto mosquitto-clients

# Run the MQTT broker
mosquitto

# Broker's task is to receive messages from the sender
# and route them to the receiver(s).
# This allows to move part of the work on the receiving system,
# while leaving only the sending of messages to the ESP32.

# The broker runs on localhost device