#!/usr/bin/env bash

# This script will run package.sh to create the complete Ultimaker 2 family

multi=0
mode=""

# test for switches
while [ "$1" != '' ]
do
    [ "$1" == "-multi" ]   && multi=1 && shift
    [ "$1" == "-release" ] && mode=$1 && shift
    [ "$1" == "-debug" ]   && mode=$1 && shift
done

if [ $multi -eq 1 ]; then
    $BASH ./package.sh -standard $mode&
    $BASH ./package.sh -extended $mode&
    $BASH ./package.sh -2go $mode&
else
    $BASH ./package.sh -all $mode
fi

