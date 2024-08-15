# Caving Automated Rapid Survey Tool (CARST)
# For implementation on Raspberry Pi devices
# 
# Version 1.0.0
# Created by Tyler Finney

# required libraries
import time
import board
import math
import gpiozero as gp
import adafruit_lis3mdl
from adafruit_lsm6ds.lsm6ds3 import LSM6DS3
import displayio
from i2cdisplaybus import I2CDisplayBus
import terminalio
from adafruit_display_text import bitmap_label as label
from adafruit_displayio_sh1107 import SH1107, DISPLAY_OFFSET_ADAFRUIT_128x128_OLED_5297

displayio.release_displays()

# i2c initialization and peripheral setup
i2c = board.I2C()
mag = adafruit_lis3mdl.LIS3MDL(i2c)
accel = LSM6DS3(i2c)
display_bus = I2CDisplayBus(i2c, device_address=0x3D)

# Display setup
WIDTH = 128
HEIGHT = 256
ROTATION = 90
BORDER = 2
display = SH1107(display_bus,width=WIDTH,height=HEIGHT,display_offset=DISPLAY_OFFSET_ADAFRUIT_128x128_OLED_5297,rotation=ROTATION,)
splash = displayio.Group()
display.root_group = splash
color_bitmap = displayio.Bitmap(WIDTH, HEIGHT, 1)
color_palette = displayio.Palette(1)
color_palette[0] = 0xFFFFFF  # White


record = gp.Button(4)
menu = gp.Button(17)
buttonUp = gp.Button(27)
buttonDown = gp.Button(22)

inclineHistory = []
headingHistory = []
magcaldata = []

# allows selection of menu items
menuData = ["", "Measure", "Show Data", "Clear Data", "Calibrate", ""]

# getters for all important sensor data
def accelX():
    accel_x, _, _ = accel.acceleration
    return accel_x
def accelY():
    _, accel_y,_ = accel.acceleration
    return accel_y
def accelZ():
    _, _,accel_z = accel.acceleration
    return accel_z
def magX():
    mag_x, _, _ = mag.magnetic
    return mag_x
def magY():
    _, mag_y, _ = mag.magnetic
    return mag_y
def magZ():
    _, _, mag_z = mag.magnetic
    return mag_z

# records data into arrays
def recordData():
    incline = inclination()
    f = open("inclinedata.txt", "a")
    f.write(str(incline)+'\n')
    f.close()
    inclineHistory.append(inclination())
    head = heading()
    f = open("headingdata.txt", "a")
    f.write(str(head)+'\n')
    f.close()
    headingHistory.append(heading())

# finds inclination from accel sensor data
def inclination():
    theta = math.atan(accelX() / accelZ()) * 180 / 3.1415
    return theta

# overwrites save file and variable to prevent overloading memory / storage
def inclineReset():
    inclineHistory = []
    f = open("inclinedata.txt", "w")        
    f.write('')

# TO FIX, NEEDS SERIOUS WORK
# calculates magnetometer heading
def heading():
    return vector_2_degrees(magX(), magY())
def vector_2_degrees(x, y):
    angle = math.degrees(math.atan2(y, x))
    if angle < 0:
        angle += 360
    return angle


# hard iron magnetometer calibration 
def magCal():
    mag_x, mag_y, mag_z = mag.magnetic
    min_x = max_x = mag_x
    min_y = max_y = mag_y
    min_z = max_z = mag_z

    SAMPLE_SIZE = 2000

    for i in range(SAMPLE_SIZE):
        # Capture the samples and show the progress
        if not i % (SAMPLE_SIZE / 20):
            print("*", end="")

        mag_x, mag_y, mag_z = mag.magnetic

        min_x = min(min_x, mag_x)
        min_y = min(min_y, mag_y)
        min_z = min(min_z, mag_z)

        max_x = max(max_x, mag_x)
        max_y = max(max_y, mag_y)
        max_z = max(max_z, mag_z)

        time.sleep(0.01)

    # Calculate the middle of the min/max range
    offset_x = (max_x + min_x) / 2
    offset_y = (max_y + min_y) / 2
    offset_z = (max_z + min_z) / 2

    f = open("magcaldata.txt", "w")
    f.write(offset_x+"\n")
    f.write(offset_y+"\n")
    f.write(offset_z+"\n")
    f.close()

# getters for calibration offsets
def offsetx():
    x = magcaldata[0]
    return x
def offsety():
    x = magcaldata[1]
    return x
def offfsetz():
    x = magcaldata[2]
    return x


# code for passing startup screen to display
def startUpScreen():
    name_text = "CARST V1"
    name_text_area = label.Label(terminalio.FONT, text=name_text, scale=2, color=0xFFFFFF, x=8, y=8)
    splash.append(name_text_area)
    name_text = "Caving"
    name_text_area = label.Label(terminalio.FONT, text=name_text, color=0xFFFFFF, x=8, y=26)
    splash.append(name_text_area)
    name_text = "Automated"
    name_text_area = label.Label(terminalio.FONT, text=name_text, color=0xFFFFFF, x=8, y=38)
    splash.append(name_text_area)
    name_text = "Rapid"
    name_text_area = label.Label(terminalio.FONT, text=name_text, color=0xFFFFFF, x=8, y=50)
    splash.append(name_text_area)
    name_text = "Survey"
    name_text_area = label.Label(terminalio.FONT, text=name_text, color=0xFFFFFF, x=8, y=62)
    splash.append(name_text_area)
    name_text = "Tool"
    name_text_area = label.Label(terminalio.FONT, text=name_text, color=0xFFFFFF, x=8, y=74)
    splash.append(name_text_area)
    name_text = "2024 Tyler Finney"
    name_text_area = label.Label(terminalio.FONT, text=name_text, color=0xFFFFFF, x=8, y=120)
    splash.append(name_text_area)

def menu(menuNum):
    name_text = menuData[menuNum-1]
    name_text_area = label.Label(terminalio.FONT, text=name_text, color=0xFFFFFF, x=8, y=8)
    splash.append(name_text_area)

    name_text = menuData[menuNum+1]
    name_text_area = label.Label(terminalio.FONT, text=name_text, color=0xFFFFFF, x=8, y=120)
    splash.append(name_text_area)

    name_text = menuData[menuNum]
    name_text_area = label.Label(terminalio.FONT, text=name_text, scale = 2, color=0xFFFFFF, x=8, y=58)
    splash.append(name_text_area)

startUpScreen()
time.sleep(3)
length = splash.__len__()
for i in range(length):
    splash.pop(i)

# if no data in inclineHistory, grab it from the save file
if(inclineHistory == []):
    f = open("inclinedata.txt", "r")
    for x in f:
        x = x.strip() # removes new line character
        x = float(x) # convert to integer
        inclineHistory.append(x)
    f.close()
    print(inclineHistory)

# if no data in headingHistory, grab it from the save file
if(headingHistory == []):
    f = open("headingdata.txt", "r")
    for x in f:
        x = x.strip() # removes new line character
        x = float(x) # convert to integer
        headingHistory.append(x)
    f.close()
    print(headingHistory)

# if not data in magcaldata, grab it from save file
if(magcaldata == []):
    f = open("magcaldata.txt", "r")
    for x in f:
        x = x.strip() # removes new line character
        x = float(x) # convert to integer
        magcaldata.append(x)
    f.close()
    print(magcaldata)

# running loop
while True:
    record.when_pressed = recordData
