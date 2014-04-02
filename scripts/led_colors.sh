#!/bin/sh
echo red            > /LED; sleep 2
echo green          > /LED; sleep 2
echo blue           > /LED; sleep 2
echo green,blue     > /LED; sleep 2 # how to set multiple leds
echo red,blue       > /LED; sleep 2
echo red,green      > /LED; sleep 2
echo red,green,blue > /LED
echo SUCCESS: cycled LED
