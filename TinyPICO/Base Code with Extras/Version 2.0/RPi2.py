#!/usr/bin/env python3
import serial
import time

# Ensures paho is in the Python PATH
import context
import paho.mqtt.client as mqtt

# The callback for when the MQTT client connects – subscribing in the on_connect() means that if we lose the connection and reconnect, then subscriptions will be renewed
def on_connect(client, userdata, flags, rc):
    client.subscribe('events/switches/out')
#   print('If you can see this subscribe is working')

# The callback for when a PUBLISH message is received from MQTT
def on_message(client, userdata, msg):
    ser.write(b'%s\n' % str(msg.payload).encode('utf-8'))

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect('127.0.0.1')

if __name__ == '__main__':
    ser = serial.Serial('/dev/ttyS0' 9600, timeout=0.001)
    ser.flush()

    While True:
        client.loop()
        if ser.in_waiting > 0:
            client.publish('events/switches/in', ser.readline().decode('utf-8').rstrip())
