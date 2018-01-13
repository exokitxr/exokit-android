#!/bin/bash

adb uninstall com.mafintosh.nodeonandroid
adb install ./app/build/outputs/apk/arm8/debug/app-arm8-debug.apk
