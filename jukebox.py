#!/usr/bin/env python

#Set up Samba server
#https://pimylifeup.com/raspberry-pi-samba/https://pimylifeup.com/raspberry-pi-samba/

#install playsound
#https://stackoverflow.com/questions/56059032/raspberry-pi-cant-play-sound-file
#sudo apt-get install python-gst-1.0
#pip3 install playsound

#Configure raspi as access point
#https://www.raspberrypi.org/documentation/configuration/wireless/access-point-routed.md

import sqlite3
from playsound import playsound

import glob
import os
import serial
#import sys
import time
import traceback

def dbFindSong (data):
    try:
        con = sqlite3.connect('Jukebox.db')
        con.row_factory = sqlite3.Row  #this for getting the column names!
        cur = con.cursor()
        data = data.split('-')
        if (len(data) != 2): return
        (letter, number) = data
        print("Searching song for letter: ", letter, "and number: ", number)
        number = int(number)
        cur.execute('SELECT * FROM tblSongs WHERE fldLetter = ? AND fldNumber = ?', (letter, number))
        records = cur.fetchall()
        for row in records:
            if (row ["fldSong"] == None ):
                print("No song defined at letter: ", letter, "and number: ", number )
            else:
                song = (os.getcwd())+"/Songs/"+row["fldSong"]
                print("Playing: ", song)
                playsound(song)
    except:
        print("Error in ")
            
def main():
    try:
        #ser = serial.Serial('COM7', '9600')
        print ("Checking serial connection...")
        if (os.name == "nt"):
            SERIAL  = 'COM7'
            ser = serial.Serial(port='COM4', baudrate=9600, timeout=.1)
        if (os.name == "posix"):
            SERIAL  = (glob.glob('/dev/ttyUSB*') + glob.glob('/dev/ttyACM*') + [''])[0]
            ser=serial.Serial('/dev/ttyACM0', baudrate = 9600, timeout = 2)
    except:
        ser = None
        traceback.print_exc()

    while True:
        try:
            # https://create.arduino.cc/projecthub/ansh2919/serial-communication-between-python-and-arduino-e7cce0
            data = ser.readline().decode('utf-8').rstrip()
            print(data)
            #data = "A-1"
            dbFindSong(data)
            time.sleep(5)
        except:
            print ("No serial data found, please connect Arduino and restart this script.")
            localtime = time.localtime()
            result = time.strftime("%I:%M:%S %p", localtime)
            print(result)
            time.sleep(5)

if __name__ == '__main__':
    main()