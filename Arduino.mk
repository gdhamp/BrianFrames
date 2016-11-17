
ARDUINO_DIR  = /usr/share/arduino
AVRDUDE=/usr/bin/avrdude

#BOARD_TAG    = pro5v328
BOARD_TAG    = uno
#ARDUINO_PORT = /dev/ttyACM0
ARDUINO_PORT = /dev/ttyUSB0


AVR_TOOLS_PATH   = /usr/bin
AVRDUDE_CONF     = /etc/avrdude.conf

CXXFLAGS = -std=gnu++11

include /usr/share/arduino/Arduino.mk
