# CCS811_InfluxDB
Sending measurement data from CCS811(CO2 and TVOC) connected to ESP8266(Wemos D1, NodeMCU etc.) to your InfluxDB database.
ESP.DeepSleep supprot.

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
