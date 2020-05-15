#!/bin/bash

read WHICH  <<< $(which obs)
len=${#WHICH}
if [ $len -eq 0 ]; then
	exit
fi
echo found