/***************************************************
  CCS811 digital TVOC/eCO2 Sensor by CCMOSS/AMS
  http://www.ccmoss.com/gas-sensors#CCS811

  Wemos D1 mini or NodeMCU 1.0
  VCC - 3.3V
  GND - G
  SCL - D1 -- GPIO 5
  SDA - D2 -- GPIO 4
  WAK - D3 -- GPIO 0

  DeepSleep: RTS(esp8266) - D0(esp8266)

 ****************************************************/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Wire.h>

#include "src/ccs811.h" // https://github.com/maarten-pennings/CCS811
#include "src/ESPinfluxdb.h" // https://github.com/hwwong/ESP_influxdb

// ********************** Config **********************

// DeepSleep time – send data every 60 seconds
const int sleepTimeS = 60;

CCS811 ccs811(D3); // nWAKE on D3

// InfluxDB – Config
#define INFLUXDB_HOST "INFLUXDB_ADRESS"
#define INFLUXDB_PORT 8086

#define DATABASE  "mydb"
#define DB_USER "mydb_username"
#define DB_PASSWORD "mydb_password"
#define DEVICE_NAME "CCS811"

// WiFi Config
#define WiFi_SSID "WiFi_SSID"
#define WiFi_Password "WiFi_Password"

// Turn ON/OFF DeepSleep Mode
#define DeepSleep_ON  false

// ******************** Config End ********************

#define WAKE_ESP8266_PIN  14
ESP8266WiFiMulti WiFiMulti;

Influxdb influxdb(INFLUXDB_HOST, INFLUXDB_PORT);

void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println("");

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WiFi_SSID, WiFi_Password);
  Serial.println();
  Serial.print("Waiting for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Enable I2C for ESP8266 NodeMCU boards [VDD to 3V3, GND to GND, nWAKE to D3, SDA to D2, SCL to D1]
  Wire.begin(4, 5);

  Serial.println("CCS811 test");
  // Enable CCS811
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  bool ok = ccs811.begin();
  if ( !ok ) Serial.println("setup: CCS811 begin FAILED");

  // Print CCS811 versions
  Serial.print("setup: hardware    version: "); Serial.println(ccs811.hardware_version(), HEX);
  Serial.print("setup: bootloader  version: "); Serial.println(ccs811.bootloader_version(), HEX);
  Serial.print("setup: application version: "); Serial.println(ccs811.application_version(), HEX);

  // Start measuring
  ok = ccs811.start(CCS811_MODE_1SEC);
  if ( !ok ) Serial.println("init: CCS811 start FAILED");



  if (influxdb.opendb(DATABASE, DB_USER, DB_PASSWORD) != DB_SUCCESS) {
    Serial.println("Connecting to database failed");
  }
}

void loop()
{
  uint16_t eco2, etvoc, errstat, raw;

  ccs811.read(&eco2, &etvoc, &errstat, &raw);
  if ( errstat == CCS811_ERRSTAT_OK ) {

    Serial.print("\n\neCO2 concentration: ");
    Serial.print(eco2);
    Serial.print(" ppm");

    Serial.print("\nTVOC concentration: ");
    Serial.print(etvoc);
    Serial.print(" ppb");

    dbMeasurement row(DEVICE_NAME);
    row.addField("eCO2", (int(eco2)));
    row.addField("TVOC", (int(etvoc)));
    if (influxdb.write(row) == DB_SUCCESS) {
      Serial.println("\n\nData send to InfluxDB");
    }
    row.empty();
  } else if ( errstat == CCS811_ERRSTAT_OK_NODATA ) {
    Serial.println("CCS811: waiting for (new) data");
  } else if ( errstat & CCS811_ERRSTAT_I2CFAIL ) {
    Serial.println("CCS811: I2C error");
  } else {
    Serial.print("CCS811: errstat="); Serial.print(errstat, HEX);
    Serial.print("="); Serial.println( ccs811.errstat_str(errstat) );
  }
  Serial.println();

  delay(100);

  if (DeepSleep_ON == true) {
    Serial.println("ESP8266 in sleep mode");
    Serial.println("Back in 60 seconds...");
    ESP.deepSleep(sleepTimeS * 1000000);
  } else {
    Serial.println("Waiting...");
    delay(sleepTimeS * 1000);
  }

}
