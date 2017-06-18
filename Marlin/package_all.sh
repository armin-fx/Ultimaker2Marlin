#!/usr/bin/env bash

# This script will run package.sh to create the complete Ultimaker 2 family

# Arguments:
#
# -multi      - create the firmware with multithreading
#
# Arguments from package.sh:
#
# -release    - create a release version of the firmware
# -debug      - create a debug version of the firmware (standard)
#
# -temp-normal - set Hotend maximum temperature to 260/275°C (standard)
# -temp-extra  - set Hotend maximum temperature to 300/315°C
#
# -clean      - this will clear all build-directories before creating the firmware

multi=0
mode=""
temperature_extra=0
clean=""

# test for switches
while [ "$1" != '' ]
do
    [ "$1" == "-multi" ]   && multi=1 && shift
    [ "$1" == "-release" ] && mode=$1 && shift
    [ "$1" == "-debug" ]   && mode=$1 && shift
    [ "$1" == "-temp-normal" ] && temperature_extra=0 && shift
    [ "$1" == "-temp-extra" ]  && temperature_extra=1 && shift
    [ "$1" == "-clean" ]   && clean=$1 && shift
done

if [ $multi -eq 1 ]; then
    $BASH ./package.sh -standard $mode $clean&
    $BASH ./package.sh -extended $mode $clean&
    $BASH ./package.sh -2go $mode $clean&
else
    $BASH ./package.sh -all $mode $clean
fi

