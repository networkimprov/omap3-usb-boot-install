echo 200 > /sys/class/leds/pca963x:red/brightness   ; sleep 2
echo 0   > /sys/class/leds/pca963x:red/brightness
echo 200 > /sys/class/leds/pca963x:green/brightness ; sleep 2
echo 0   > /sys/class/leds/pca963x:green/brightness
echo 200 > /sys/class/leds/pca963x:blue/brightness  ; sleep 2
echo 200 > /sys/class/leds/pca963x:red/brightness
echo 200 > /sys/class/leds/pca963x:green/brightness ; sleep 2
echo 0   > /sys/class/leds/pca963x:blue/brightness
echo 0   > /sys/class/leds/pca963x:red/brightness
echo 0   > /sys/class/leds/pca963x:green/brightness
exit 0
