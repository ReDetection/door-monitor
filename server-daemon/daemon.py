#!/usr/bin/env python

import serial
import json
import redis as redislib
import time

arduino = serial.Serial('/dev/tty.usbmodem1411', 57600) # todo: commandline parameter
redis = redislib.StrictRedis(host='192.168.12.20', port=6379, db=5) #todo config

while 1:
	line = arduino.readline()
	print line 

	message = json.loads(line)

	key = time.strftime("%d-%m-%Y-%H-%M") + "-" + str(message["nodeMillis"]) + "-" + ("closed" if message["doorClosed"] else "open")

	redisRecord = {"time": time.time, "message": message}

	redis.set(key, redisRecord)






