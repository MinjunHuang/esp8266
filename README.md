# ESP8266 Sample

Sample to test ESP8266 NoOS SDK 

## Feature

* Connect to Wi-Fi hotspot, LED blink until connection ok
* Create mini HTTP server to on/off internal LED, go to http://ip/on or off
* Read ADC and send value to remote web server

## Configration

* Create user_config.h file in user folder
* Add some content
```
#define SSID "my_ssid"
#define PASSWORD "my_passwd"

#define NET_DOMAIN "api.thingspeak.com"
#define NET_URL "GET https://api.thingspeak.com/update?api_key=MY_API_KEY&field1=%d\r\n\r\n"


