#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Enable the globstar shell option
shopt -s globstar

# Make sure we are inside the github workspace
cd $GITHUB_WORKSPACE

# Create directories
mkdir $HOME/Arduino 
mkdir $HOME/Arduino/libraries

# Install Arduino IDE
export PATH=$PATH:$GITHUB_WORKSPACE/bin
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
arduino-cli config init
arduino-cli core update-index

# Install cores
arduino-cli core install arduino:avr
arduino-cli core install arduino:samd

# Link Arduino library
ln -s $GITHUB_WORKSPACE $HOME/Arduino/libraries/Arduino_MCP3x6x_Library

# Compile all *.ino files for the Arduino Uno
for f in examples/**/*.ino ; do
    arduino-cli compile -b arduino:avr:uno $f
    arduino-cli compile -b arduino:samd:adafruit_grandcentral_m4 $f
done
