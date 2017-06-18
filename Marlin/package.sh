#!/usr/bin/env bash

# This script is to package the Marlin package for Arduino
# This script should run under Linux and Mac OS X, as well as Windows with Cygwin.

# Arguments:
#
# -all        - create the firmware for the complete Ultimaker 2 family
#
# -standard   - create the firmware for Ultimaker 2 (standard setting), Ultimaker 2 Dual, Ultimaker 2+, Ultimaker 2+ Dual
# -extended   - create the firmware for Ultimaker 2 Extended, Ultimaker 2 Extended Dual, Ultimaker 2 Extended+, Ultimaker 2 Extended+ Dual
# -2go        - create the firmware for Ultimaker 2 Go, Ultimaker 2 Go HBK, Ultimaker 2 Go Dual, Ultimaker 2 Go HBK Dual
#
# -normal     - create only the normal firmware version
# -plus       - create only the plus firmware version (if this exist)
# -dual       - create only the dual extruder firmware version (if this exist)
# -dualplus   - create only the plus dual extruder firmware version
# -hbk        - create only the heated bed upgrade kit firmware version
#               (only for Ultimaker 2 Go, all other versions habe already a heated bed)
# -all-normal - create the complete normal firmware version inclusive dual (and hbk)
# -all-plus   - create the complete plus firmware version inclusive dual (and hbk)
# -all-dual   - create the complete dual extruder firmware version inclusive normal and plus
#
# -release    - create a release version of the firmware
# -debug      - create a debug version of the firmware (standard)
#
# -temp-normal - set Hotend maximum temperature to 260/275°C (standard)
# -temp-extra  - set Hotend maximum temperature to 300/315°C
#
# -clean      - this will clear all build-directories before creating the firmware

#############################
# CONFIGURATION
#############################

##Which version name are we appending to the final archive
export BUILD_NAME=17.03
BUILD_NAME_PRE="Tinker_"

#############################
# Actual build script
#############################

if [ -z `which make` ]; then
	MAKE=mingw32-make
else
	MAKE=make
fi


# Change working directory to the directory the script is in
# http://stackoverflow.com/a/246128
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# #For building under MacOS we need gnutar instead of tar
# if [ -z `which gnutar` ]; then
# 	TAR=tar
# else
# 	TAR=gnutar
# fi

#############################
# Arguments
#############################

# firmware switch: 1 = true, 0 = false
family_standard=0
family_extended=0
family_2go=0
switch_normal=0
switch_plus=0
switch_dual=0
switch_dualplus=0
switch_hbk=0

release=0
temperature_extra=0
clean=0

switch_complete() {
    switch_normal=1
    switch_plus=1
    switch_dual=1
    switch_dualplus=1
    switch_hbk=1
}
family_complete() {
    family_standard=1
    family_extended=1
    family_2go=1
}

# test for switches
while [ "$1" != '' ]
do
    [ "$1" == "-all" ] && family_complete && switch_complete && shift
    [ "$1" == "-standard" ] && family_standard=1 && shift
    [ "$1" == "-extended" ] && family_extended=1 && shift
    [ "$1" == "-2go" ]      && family_2go=1      && shift
    [ "$1" == "-normal" ]   && switch_normal=1   && shift
    [ "$1" == "-plus" ]     && switch_plus=1     && shift
    [ "$1" == "-dual" ]     && switch_dual=1     && shift
    [ "$1" == "-dualplus" ] && switch_dualplus=1 && shift
    [ "$1" == "-hpk" ]      && switch_hbk=1      && shift
    [ "$1" == "-all-normal" ] && switch_normal=1 && switch_dual=1 && switch_hbk=1 && shift
    [ "$1" == "-all-plus" ]   && switch_plus=1 && switch_dualplus=1 && switch_hbk=1 && shift
    [ "$1" == "-all-dual" ]   && switch_dual=1 && switch_dualplus=1 && shift
    [ "$1" == "-release" ] && release=1 && shift
    [ "$1" == "-debug" ]   && release=0 && shift
    [ "$1" == "-temp-normal" ] && temperature_extra=0 && shift
    [ "$1" == "-temp-extra" ]  && temperature_extra=1 && shift
    [ "$1" == "-clean" ] && clean=1 && shift
done

# test for standard setting
[ $switch_normal -eq 0 ] && [ $switch_plus -eq 0 ] && [ $switch_dual -eq 0 ] && [ $switch_dualplus -eq 0 ] && [ $switch_hbk -eq 0 ] && switch_complete
[ $family_standard -eq 0 ] && [ $family_extended -eq 0 ] && [ $family_2go -eq 0 ] && family_standard=1


#############################
# Build the required firmwares
#############################

if [ -d "D:/arduino-1.8.1" ]; then
    ARDUINO_PATH=D:/arduino-1.8.1
    ARDUINO_VERSION=181
elif [ -d "C:/arduino-1.0.3" ]; then
	ARDUINO_PATH=C:/arduino-1.0.3
	ARDUINO_VERSION=103
elif [ -d "/Applications/Arduino.app/Contents/Resources/Java" ]; then
	ARDUINO_PATH=/Applications/Arduino.app/Contents/Resources/Java
	ARDUINO_VERSION=105
else
	ARDUINO_PATH=/usr/share/arduino
	ARDUINO_VERSION=105
fi


#Build the Ultimaker2 firmwares.
# gitClone https://github.com/TinkerGnome/Ultimaker2Marlin.git _Ultimaker2Marlin
# cd _Ultimaker2Marlin/Marlin

# USE_CHANGE_TEMPERATURE

if [ ! -d "resources/firmware" ]; then
    mkdir resources
    mkdir resources/firmware
fi

# parameter 1:  directory into where to build
# parameter 2:  base name of hex-filename
# parameter 3:  defines for the firmware
make_firmware() {
  if [ $clean -eq 1 ]; then
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=$1 clean
    sleep 2
  fi
  if [ ! -d $1 ]; then
    mkdir $1
  fi
  #
  $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=$1 DEFINES="$3"
  #
  if [ -e $1/Marlin.hex ]; then
    cp $1/Marlin.hex resources/firmware/$2-${BUILD_NAME}.hex
  fi
}

defines_all="FILAMENT_SENSOR_PIN=30 BABYSTEPPING"
if [ $release -eq 1 ]; then
  defines_all="${defines_all} RELEASE"
fi
if [ $temperature_extra -eq 1 ]; then
  defines_all="${defines_all} HEATER_0_MAXTEMP=315 HEATER_1_MAXTEMP=315 HEATER_2_MAXTEMP=315"
fi

if [ $family_standard -eq 1 ]; then
# Ultimaker 2

  if [ $switch_normal -eq 1 ]; then
    make_firmware _Ultimaker2 Tinker-MarlinUltimaker2          "'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}\"' ${defines_all}\
        TEMP_SENSOR_1=0  EXTRUDERS=1 DEFAULT_POWER_BUDGET=175"
  fi

  if [ $switch_dual -eq 1 ]; then
    make_firmware _Ultimaker2Dual Tinker-MarlinUltimaker2-dual "'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}\"' ${defines_all}\
        TEMP_SENSOR_1=20 EXTRUDERS=2 DEFAULT_POWER_BUDGET=160"
  fi

  if [ $switch_plus -eq 1 ]; then
    make_firmware _Ultimaker2plus Tinker-MarlinUltimaker2plus  "'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}+\"' ${defines_all}\
        TEMP_SENSOR_1=0  EXTRUDERS=1 DEFAULT_POWER_BUDGET=175 UM2PLUS 'EEPROM_VERSION=\"V12\"'"
  fi

  if [ $switch_dualplus -eq 1 ]; then
    make_firmware _Ultimaker2plusDual Tinker-MarlinUltimaker2plus-dual "'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}+\"' ${defines_all}\
        TEMP_SENSOR_1=20 EXTRUDERS=2 DEFAULT_POWER_BUDGET=160 UM2PLUS 'EEPROM_VERSION=\"V12\"'"
  fi

fi

if [ $family_extended -eq 1 ]; then
# Ultimaker 2 extended

  defines_extended="Z_MAX_POS=330"

  if [ $switch_normal -eq 1 ]; then
    make_firmware _Ultimaker2extended Tinker-MarlinUltimaker2extended "'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}x\"' ${defines_all}\
        TEMP_SENSOR_1=0  EXTRUDERS=1 DEFAULT_POWER_BUDGET=175 ${defines_extended}"
  fi

  if [ $switch_dual -eq 1 ]; then
    make_firmware _Ultimaker2extendedDual Tinker-MarlinUltimaker2extended-dual "'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}x\"' ${defines_all}\
        TEMP_SENSOR_1=20 EXTRUDERS=2 DEFAULT_POWER_BUDGET=160 ${defines_extended}"
  fi

  if [ $switch_plus -eq 1 ]; then
    make_firmware _Ultimaker2extendedplus Tinker-MarlinUltimaker2extendedplus "'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}x+\"' ${defines_all}\
        TEMP_SENSOR_1=0  EXTRUDERS=1 DEFAULT_POWER_BUDGET=175 ${defines_extended} UM2PLUS 'EEPROM_VERSION=\"V12\"'"
  fi

  if [ $switch_dualplus -eq 1 ]; then
    make_firmware _Ultimaker2extendedplusDual Tinker-MarlinUltimaker2extendedplus-dual "'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}x+\"' ${defines_all}\
        TEMP_SENSOR_1=20 EXTRUDERS=2 DEFAULT_POWER_BUDGET=160 ${defines_extended} UM2PLUS 'EEPROM_VERSION=\"V12\"'"
  fi

fi

if [ $family_2go -eq 1 ]; then
# Ultimaker 2 go

  defines_2go="UM2GO X_MAX_POS=122 X_MIN_POS=-14 Y_MAX_POS=124 Y_MIN_POS=0 Z_MAX_POS=130 Z_MIN_POS=0 FILAMANT_BOWDEN_LENGTH=550 NO_MATERIAL_CPE NO_MATERIAL_ABS\
               DEFAULT_POWER_BUDGET=60 DEFAULT_POWER_EXTRUDER=35 DEFAULT_POWER_BUILDPLATE=60"

  if [ $switch_normal -eq 1 ]; then
    make_firmware _Ultimaker2go Tinker-MarlinUltimaker2go "'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}go\"' ${defines_all}\
        TEMP_SENSOR_1=0 TEMP_SENSOR_BED=0  EXTRUDERS=1 MAX_HEATERS=1 ${defines_2go}"
  fi

  if [ $switch_hbk -eq 1 ]; then
    make_firmware _Ultimaker2go-HBK Tinker-MarlinUltimaker2go-HBK "'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}go\"' ${defines_all}\
        TEMP_SENSOR_1=0 TEMP_SENSOR_BED=20 EXTRUDERS=1 MAX_HEATERS=1 BANG_MAX=220 ${defines_2go}"
  fi

  if [ $switch_dual -eq 1 ]; then
    make_firmware _Ultimaker2goDual Tinker-MarlinUltimaker2go-dual "'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}go\"' ${defines_all}\
        TEMP_SENSOR_1=0 TEMP_SENSOR_BED=0  EXTRUDERS=2 MAX_HEATERS=2 BANG_MAX=220 ${defines_2go}"
  fi

  if [ $switch_hbk -eq 1 ] && [ $switch_dual -eq 1 ]; then
    make_firmware _Ultimaker2go-HBKDual Tinker-MarlinUltimaker2go-HBK-dual "'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}go\"' ${defines_all}\
        TEMP_SENSOR_1=0 TEMP_SENSOR_BED=20 EXTRUDERS=2 MAX_HEATERS=2 BANG_MAX=220 ${defines_2go}"
  fi

fi
