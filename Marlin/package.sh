#!/usr/bin/env bash

# This script is to package the Marlin package for Arduino
# This script should run under Linux and Mac OS X, as well as Windows with Cygwin.

# Arguments:
#
# -all        - create the firmware for the complete Ultimaker 2 family
#
# -standard   - create the firmware for Ultimaker 2, Ultimaker 2 Dual, Ultimaker 2+ (standard setting)
# -extended   - create the firmware for Ultimaker 2 Extended, Ultimaker 2 Extended Dual, Ultimaker 2 Extended+
# -2go        - create the firmware for Ultimaker 2 Go, Ultimaker 2 Go HBK
#
# -normal     - create only the normal firmware version
# -plus       - create only the plus firmware version (if this exist)
# -dual       - create only the dual extruder firmware version (if this exist)
# -dualplus   - create only the plus dual extruder firmware version (not implemented yet)
# -hbk        - create only the heated bed upgrade kit firmware version
#               (only for Ultimaker 2 Go, all other versions habe already a heated bed)
# -all-normal - create the complete normal firmware version inclusive dual (and hbk)
# -all-plus   - create the complete plus firmware version inclusive dual (and hbk)
# -all-dual   - create the complete dual extruder firmware version inclusive normal and plus
#
# -release    - create a release version of the firmware
# -debug      - create a debug version of the firmware (standard)

#############################
# CONFIGURATION
#############################

##Which version name are we appending to the final archive
export BUILD_NAME=16.03.1
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
done

# test for standard setting
[ $switch_normal -eq 0 ] && [ $switch_plus -eq 0 ] && [ $switch_dual -eq 0 ] && [ $switch_dualplus -eq 0 ] && [ $switch_hbk -eq 0 ] && switch_complete
[ $family_standard -eq 0 ] && [ $family_extended -eq 0 ] && [ $family_2go -eq 0 ] && family_standard=1


#############################
# Build the required firmwares
#############################

if [ -d "C:/arduino-1.0.3" ]; then
	ARDUINO_PATH=C:/arduino-1.0.3
	ARDUINO_VERSION=103
elif [ -d "/Applications/Arduino.app/Contents/Resources/Java" ]; then
	ARDUINO_PATH=/Applications/Arduino.app/Contents/Resources/Java
	ARDUINO_VERSION=105
elif [ -d "C:/Arduino" ]; then
	ARDUINO_PATH=C:/Arduino
	ARDUINO_VERSION=165
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

defines_all="FILAMENT_SENSOR_PIN=30 BABYSTEPPING"
if [ $release -eq 1 ]; then
  defines_all="${defines_all} RELEASE"
fi

if [ $family_standard -eq 1 ]; then
# Ultimaker 2

  if [ $switch_normal -eq 1 ]; then
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=_Ultimaker2 clean
    sleep 2
    mkdir _Ultimaker2
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=_Ultimaker2 DEFINES="'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}\"' ${defines_all} TEMP_SENSOR_1=0 EXTRUDERS=1 HEATER_0_MAXTEMP=315 HEATER_1_MAXTEMP=315 HEATER_2_MAXTEMP=315"

    cp _Ultimaker2/Marlin.hex resources/firmware/Tinker-MarlinUltimaker2-${BUILD_NAME}.hex
  fi

  if [ $switch_dual -eq 1 ]; then
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=_Ultimaker2Dual clean
    sleep 2
    mkdir _Ultimaker2Dual
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=_Ultimaker2Dual DEFINES="'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}\"' ${defines_all} TEMP_SENSOR_1=20 EXTRUDERS=2 HEATER_0_MAXTEMP=315 HEATER_1_MAXTEMP=315 HEATER_2_MAXTEMP=315"
    #cd -

    cp _Ultimaker2Dual/Marlin.hex resources/firmware/Tinker-MarlinUltimaker2-dual-${BUILD_NAME}.hex
  fi

  if [ $switch_plus -eq 1 ]; then
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=_Ultimaker2plus clean
    sleep 2
    mkdir _Ultimaker2plus
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=_Ultimaker2plus DEFINES="'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}+\"' ${defines_all} TEMP_SENSOR_1=0 EXTRUDERS=1 HEATER_0_MAXTEMP=315 HEATER_1_MAXTEMP=315 HEATER_2_MAXTEMP=315 INVERT_E0_DIR=true INVERT_E1_DIR=true INVERT_E2_DIR=true 'EEPROM_VERSION=\"V12\"'"

    cp _Ultimaker2plus/Marlin.hex resources/firmware/Tinker-MarlinUltimaker2plus-${BUILD_NAME}.hex
  fi

fi

if [ $family_extended -eq 1 ]; then
# Ultimaker 2 extended

  defines_extended="Z_MAX_POS=330"

  if [ $switch_normal -eq 1 ]; then
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=_Ultimaker2extended clean
    sleep 2
    mkdir _Ultimaker2extended
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=_Ultimaker2extended DEFINES="'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}x\"' ${defines_all} TEMP_SENSOR_1=0 EXTRUDERS=1 HEATER_0_MAXTEMP=315 HEATER_1_MAXTEMP=315 HEATER_2_MAXTEMP=315 ${defines_extended}"

    cp _Ultimaker2extended/Marlin.hex resources/firmware/Tinker-MarlinUltimaker2extended-${BUILD_NAME}.hex
  fi

  if [ $switch_dual -eq 1 ]; then
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=_Ultimaker2extendedDual clean
    sleep 2
    mkdir _Ultimaker2extendedDual
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=_Ultimaker2extendedDual DEFINES="'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}x\"' ${defines_all} TEMP_SENSOR_1=20 EXTRUDERS=2 HEATER_0_MAXTEMP=315 HEATER_1_MAXTEMP=315 HEATER_2_MAXTEMP=315 ${defines_extended}"
    #cd -

    cp _Ultimaker2extendedDual/Marlin.hex resources/firmware/Tinker-MarlinUltimaker2extended-dual-${BUILD_NAME}.hex
  fi

  if [ $switch_plus -eq 1 ]; then
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=_Ultimaker2extendedplus clean
    sleep 2
    mkdir _Ultimaker2extendedplus
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=_Ultimaker2extendedplus DEFINES="'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}x+\"' ${defines_all} TEMP_SENSOR_1=0 EXTRUDERS=1 HEATER_0_MAXTEMP=315 HEATER_1_MAXTEMP=315 HEATER_2_MAXTEMP=315 ${defines_extended} INVERT_E0_DIR=true INVERT_E1_DIR=true INVERT_E2_DIR=true 'EEPROM_VERSION=\"V12\"'"

    cp _Ultimaker2extendedplus/Marlin.hex resources/firmware/Tinker-MarlinUltimaker2extendedplus-${BUILD_NAME}.hex
  fi

fi

if [ $family_2go -eq 1 ]; then
# Ultimaker 2 go

  defines_2go="TEMP_SENSOR_BED=0 INVERT_E0_DIR=true X_MAX_POS=122 X_MIN_POS=-14 Y_MAX_POS=124 Y_MIN_POS=0 Z_MAX_POS=130 Z_MIN_POS=0 FILAMANT_BOWDEN_LENGTH=550 MAX_HEATERS=1 NO_MATERIAL_CPE NO_MATERIAL_ABS"

  if [ $switch_normal -eq 1 ]; then
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=_Ultimaker2go clean
    sleep 2
    mkdir _Ultimaker2go
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=_Ultimaker2go DEFINES="'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}go\"' ${defines_all} TEMP_SENSOR_1=0 EXTRUDERS=1 HEATER_0_MAXTEMP=275 HEATER_1_MAXTEMP=275 HEATER_2_MAXTEMP=275 ${defines_2go}"
    # cd -

    cp _Ultimaker2go/Marlin.hex resources/firmware/Tinker-MarlinUltimaker2go-${BUILD_NAME}.hex
  fi

  if [ $switch_hbk -eq 1 ]; then
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=_Ultimaker2go-HBK clean
    sleep 2
    mkdir _Ultimaker2go-HBK
    $MAKE -j 3 HARDWARE_MOTHERBOARD=72 ARDUINO_INSTALL_DIR=${ARDUINO_PATH} ARDUINO_VERSION=${ARDUINO_VERSION} BUILD_DIR=_Ultimaker2go-HBK DEFINES="'STRING_CONFIG_H_AUTHOR=\"${BUILD_NAME_PRE}${BUILD_NAME}go\"' ${defines_all} TEMP_SENSOR_1=0 TEMP_SENSOR_BED=20 EXTRUDERS=1 HEATER_0_MAXTEMP=275 HEATER_1_MAXTEMP=275 HEATER_2_MAXTEMP=275 ${defines_2go}"
    # cd -

    cp _Ultimaker2go-HBK/Marlin.hex resources/firmware/Tinker-MarlinUltimaker2go-HBK-${BUILD_NAME}.hex
  fi

fi
