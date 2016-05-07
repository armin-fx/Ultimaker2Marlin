#!/usr/bin/env bash

# This script will run package.sh to create the complete Ultimaker 2 family

if [ "$1" == "-multi" ]; then
    $BASH ./package.sh -standard&
    $BASH ./package.sh -extended&
    $BASH ./package.sh -2go&
else
    $BASH ./package.sh -all
fi

