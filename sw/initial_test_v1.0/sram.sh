#!/bin/bash
#echo $PWD
#source $PWD/source_this
prog=$1
port="${2:-4444}" # if no port given, use default CMSIS telnet port
entry=$(grep _entry_point $1.map | awk '{ print $1 }')
inFlash=$(echo $entry | cut -b-6)
#echo "inflash: $inFlash"
if [ $inFlash == "0x1000" ]; then
    echo "FAIL! Compiled for flash, not SRAM"
    exit
fi
#export OPENOCD_PATH=$PICO_SDK_PATH/../../openocd
#echo -e "reset\nexit\n" | ncat localhost 4444
#echo -e "halt\nexit\n" | ncat localhost 4444
#sleep 1
echo -e "reset halt\nload_image $prog\nresume $entry\nexit\n" | ncat localhost $port




