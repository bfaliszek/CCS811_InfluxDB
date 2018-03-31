/***************************************************
  CCS811 digital TVOC/eCO2 Sensor by CCMOSS/AMS
  http://www.ccmoss.com/gas-sensors#CCS811

  Wemos D1 mini or NodeMCU 1.0
  VCC - 3.3V
  GND - G
  SCL - D1
  SDA - D2
  WAK - D5
  INT - D6
  DeepSleep: D0 - RTS
  
 ****************************************************/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include "src/CCS811.h"
#include "src/ESPinfluxdb.h"



// ********************** Config **********************

// Send data every 60 seconds
const int sleepTimeS = 60;

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

// replace temperature and humidity with values from external sensor - ex. BME280, DHT22
float temperature = 22.56;
float humidity = 40.73;

// Turn ON/OFF DeepSleep Mode
#define DeepSleep_ON  true

// ******************** Config End ********************



#define WAKE_PIN  14

CCS811 sensor;
ESP8266WiFiMulti WiFiMulti;

Influxdb influxdb(INFLUXDB_HOST, INFLUXDB_PORT);

void setup()
{
  Serial.begin(115200);
  delay(10);
  
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
    
  Serial.println("CCS811 test");
  configure_ccs811();
  
  if (influxdb.opendb(DATABASE)!=DB_SUCCESS) {
        Serial.println("Connecting to database failed");
      } 
}

void configure_ccs811() {
  int count = 0;
  while (count < 100) {
    count++;
    if (!sensor.begin(uint8_t(ADDR), uint8_t(WAKE_PIN))) {
      Serial.println("Initialization failed");
      delay(100);
    } else {
      Serial.println("Success in " + String(count));
      break;
    }
    if (count == 99) {
      ESP.reset(); //reset and try again
      delay(5000);
    }
    delay(100);
  }
}

void loop()
{ 
  sensor.compensate(temperature, humidity);  
  sensor.getData(ADDR, WAKE_PIN);
  float co2 = sensor.readCO2();
  float tvoc = sensor.readTVOC();
  if (co2 >= 65021) {
    configure_ccs811();
    co2 = 400;
    tvoc = 0;
  } else if (co2 < 1) {
    co2 = 400;
    tvoc = 0;
  }
  
  Serial.print("eCO2 concentration : "); Serial.print(co2); Serial.println(" ppm");
  Serial.print("TVOC concentration : "); Serial.print(tvoc); Serial.println(" ppb");

  dbMeasurement row(DEVICE_NAME);
  row.addField("eCO2", (int(co2)));
  row.addField("TVOC", (int(tvoc)));
  if(influxdb.write(row) == DB_SUCCESS){
    Serial.println("Data send to InfluxDB");
  }
  row.empty();
  Serial.println();

  delay(100);

  if (DeepSleep_ON == true) {
    Serial.println("ESP8266 in sleep mode");
    Serial.println("Back in 60 seconds...");
    ESP.deepSleep(sleepTimeS * 1000000);
  }else{
    Serial.println("Waiting...");
    delay(sleepTimeS * 1000);
  }
  
}
