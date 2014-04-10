#!/bin/sh
if [ ! -e /dev/sx1213_data ]
then 
	mknod /dev/sx1213_data c 123 0
fi

if [ ! -e /dev/sx1213_config ]
then
	mknod /dev/sx1213_config c 123 1
fi

rmmod sx1213
insmod sx1213.ko
