#include "config.h"
#include <ESP8266WiFi.h>



// Replace with your Wi-Fi network credentials
const char* ssid = "Martin Router King";
const char* password = "Routerking!1312";

// Replace with your MQTT broker address
const char* mqtt_server = "homeassistant.local";
const int mqtt_port = 1883;

// Replace with your MQTT username and password
const char* mqtt_user = "mqtt";
const char* mqtt_pass = "Dreierpack1!";



// Pin to which the sensor is connected
const int SENSOR_PIN = A0;

// Threshold value for triggering the sensor
const int threshold = 30;

const int lowestPWM = 415;
const int highestPWM = 2575;

const int rotationBeforeDrop = 15;


const int maxRate = 3;




// DISPENSER DEFINITIONS

//                   PIN         LOWEST      PREDROP     DROP        LASTPOS     SHAKEDIST   FALLDURATION
int zinc[] =        {D5,         180,        30,         2,          0,          22,         2400};
int omega[] =       {D6,         180,        40,         0,          0,          30,         1600};
int magnesium[] =   {D7,         180,        30,         10,         0,          30,         1200};

// PIN: Where the Servo PWM Pin is connected.
// LOWEST: Lowest Position  of the disk in degrees. 180 normally is the lowest Point, 0 is the highest.
// PREDROP: The Location to where the hole of the disk moves before the dropping sequence (with detection) is started. This should be chosen s.t. the Pill is just before the hole but can not fall out yet.
// DROP: The Loctaion where the drop happens. This is normally close to 0 degrees (the uppermost position).
// SHAKEDIST: from the LOWEST Point, how far up the disk should be moved during one shaking motion.
// DURATION: How long the dropping sequence with detection should take. The further up the Dispenser is, the longer the time it takes to fall down, the more DURATION is needed.




int order[] = {*zinc, *omega, *magnesium};