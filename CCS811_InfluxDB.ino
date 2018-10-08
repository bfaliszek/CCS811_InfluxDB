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
#include "src/ESPinfluxdb.h"

// ********************** Config **********************

// DeepSleep time – send data every 60 seconds
const int sleepTimeS = 60;

CCS811 ccs811(0); // WAKE to D3

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

void print_versions() {
  uint8_t hw_version;
  uint16_t fw_boot_version;
  uint16_t fw_app_version;
  bool ok= ccs811.versions(&hw_version, &fw_boot_version, &fw_app_version);
  if( ok ) {
    Serial.print("init: CCS811 versions: "); 
    Serial.print("hw 0x"); Serial.print(hw_version,HEX);
    Serial.print(", fw_boot 0x"); Serial.print(fw_boot_version,HEX); 
    Serial.print(", fw_app 0x"); Serial.print(fw_app_version,HEX);
    Serial.println("");
  } else {
    Serial.println("init: CCS811 versions FAILED");
  }
}

#define WAKE_ESP8266_PIN  14
int firstRead = 0;

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

  while(WiFiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
  
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Enable I2C for ESP8266 NodeMCU boards [VDD to 3V3, GND to GND, nWAKE to D3, SDA to D2, SCL to D1]
  Wire.begin(4,5); 
  
  Serial.println("CCS811 test");
  // Enable CCS811
  bool ok= ccs811.begin();
  if( !ok ) Serial.println("init: CCS811 begin FAILED");
  print_versions();
  
  // Start measuring
  ok= ccs811.start(CCS811_MODE_1SEC);
  if( !ok ) Serial.println("init: CCS811 start FAILED");

  if (influxdb.opendb(DATABASE)!=DB_SUCCESS) {
        Serial.println("Connecting to database failed");
      } 
}

void check_results()
{
  // Read
  uint16_t eco2;
  uint16_t etvoc;
  uint16_t errstat;
  uint16_t raw;
  
  ccs811.read(&eco2,&etvoc,&errstat,&raw);
  // Check if errstat flags denote VALID&NEW or OLD|ERROR
  bool valid_and_new = ( (errstat&CCS811_ERRSTAT_OKS) == CCS811_ERRSTAT_OKS )  &&  ( (errstat&CCS811_ERRSTAT_ERRORS)==0 );
  firstRead++;
  delay(10000); // after first read, give sensor some time, to heat up
  }

void loop()
{ 
  if (firstRead == 0){ // first reading always returns >65000
    check_results();
  }
  
  // Read
  uint16_t eco2;
  uint16_t etvoc;
  uint16_t errstat;
  uint16_t raw;
  
  ccs811.read(&eco2,&etvoc,&errstat,&raw);
  bool valid_and_new = ( (errstat&CCS811_ERRSTAT_OKS) == CCS811_ERRSTAT_OKS )  &&  ( (errstat&CCS811_ERRSTAT_ERRORS)==0 );

  if (eco2 >= 65000 or etvoc >=32000) {
    check_results();
  }

  Serial.print("\n\neCO2 concentration: "); 
  Serial.print(eco2); 
  Serial.print(" ppm");
  
  Serial.print("\nTVOC concentration: "); 
  Serial.print(etvoc); 
  Serial.print(" ppb");

  dbMeasurement row(DEVICE_NAME);
  row.addField("eCO2", (int(eco2)));
  row.addField("TVOC", (int(etvoc)));
  if(influxdb.write(row) == DB_SUCCESS){
    Serial.println("\n\nData send to InfluxDB");
  }
  row.empty();
  Serial.println();

  delay(100);

  if (DeepSleep_ON == true) {
    Serial.println("ESP8266 in sleep mode");
    firstRead = 0;
    Serial.println("Back in 60 seconds...");
    ESP.deepSleep(sleepTimeS * 1000000);
  }else{
    Serial.println("Waiting...");
    delay(sleepTimeS * 1000);
  }
  
}
