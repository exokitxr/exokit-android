#!/bin/bash

adb uninstall com.mafintosh.nodeonandroid
adb install ./app/build/outputs/apk/app-arm-debug.apk 
