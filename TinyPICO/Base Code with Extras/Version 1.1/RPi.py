#!/usr/bin/env python3
# Version 1.0 - Run this code on the RaspberryPi4B to dispay the output, and return a relay value to the TinyPICO.
import serial

relay = ["1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", "32", "1", "3", "5", "7", "9", "11", "13", "15", "17", "19", "21", "23", "25", "27", "29", "31", 
"2", "4", "6", "8", "10", "12", "14", "16", "18", "20", "22", "24", "26", "28", "30", "32", "31", "29", "27", "25", "23", "21", "19", "17", "15", "13", "11", "9", "7", "5", "3", "1", "32", "30", "28", "26", "24", "22", "20", "18", "16", "14", "12", "10", "8", "6", "4", "2"]

if __name__ == '__main__':
    ser = serial.Serial('/dev/ttyS0', 9600, timeout=0.1)
    ser.flush()
    while True:
        if ser.in_waiting > 0:
            sNum = ser.readline().decode('ascii').rstrip();
            iNum = int(sNum)
            if iNum < 97:
                oNum = relay[iNum -1]
#                print(iNum, oNum);
            else:
                print("Error ", iNum);
            ser.write(b'%s\n' % oNum.encode('ascii'))
