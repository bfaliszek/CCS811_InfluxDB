# CCS811_InfluxDB
Sending measurement data from CCS811(ex. CJMCU-811 CCS811) connected to ESP8266(Wemos D1, NodeMCU etc.) to your InfluxDB database.

ESP.DeepSleep support.

### Wiring
 Wemos D1 or NodeMCU 1.0:
 * VCC - 3.3V
 * GND - G
 * SCL - D1
 * SDA - D2
 * WAK - D5
 * INT - D6
 
 For DeepSleep:
 * D0 - RTS

![CJMCU-811 CCS811](https://raw.githubusercontent.com/bfaliszek/CCS811_InfluxDB/master/CJMCU-811_CCS811.jpg)
