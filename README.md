# CCS811 InfluxDB
Sending measurement data from CCS811(ex. CJMCU-811 CCS811) connected to ESP8266(Wemos D1, NodeMCU etc.) to your InfluxDB database.

Based on [CCS811 library](https://github.com/AKstudios/CCS811-library) and [ESP8266Influxdb](https://github.com/hwwong/ESP8266Influxdb).

### Wiring
 Wemos D1 or NodeMCU 1.0:
 * VCC - 3.3V
 * GND - G
 * SCL - D1
 * SDA - D2
 * WAK - D5
 * INT - D6

![CJMCU-811 CCS811](https://raw.githubusercontent.com/bfaliszek/CCS811_InfluxDB/master/CJMCU-811_CCS811.jpg)
