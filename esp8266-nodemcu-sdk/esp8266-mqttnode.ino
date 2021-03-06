
/**
  IoT Sensor Node for any Sensor - Nodemcu code
  
  Name: my_multiply_doc
  Purpose: Here we use low power Node MSU esp8266 and following sensors
           AM2301 Humidity and Temperature sensor, BH1750FVI Light sensor, Soil moisture sensor
  @author Akila Wickey
  @version 1.0 05/01/18
   Light sensor BH1750FVI       VCC  –  Wemos 3.3v
                                GND – Wemos Gnd
                                SCL – Wemos D1
                                SDA – Wemos D2
                                
   Soil sensor BH1750FVI        VCC  –  Wemos 3.3v
                                GND – Wemos Gnd
                                A0 – Wemos A0 
                                
   AM2301 temp humidity sensor  VCC  –  Wemos 3.3v
                                GND – Wemos Gnd
                                Digital pin  – Wemos D5 
                     
*/
#include <Wire.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

void callback(char* topic, byte* payload, unsigned int length);
//EDIT THESE LINES TO MATCH YOUR SETUP
#define MQTT_SERVER "MQTT_SERVER_IP"

const char* client_name = "CLIENT_NAME";
const char* mqtt_username = "MQTT_USER_NAME";
const char* mqtt_password = "MQTT_PASSWORD";

const int lightPin = D5;

char* lightTopic   = "light";
char* tempTopic   = "temp";
char* humTopic   = "hum";
char* soilTopic   = "soil";

char charBuf_temp[50];
char charBuf_hum[50];
char charBuf_light[50];
char charBuf_soil[50];

// We will take analog input from A0 pin 
const int AnalogIn     = A0; 

WiFiClient wifiClient;

PubSubClient client(MQTT_SERVER, 1884, callback, wifiClient);

void setup() {

  //start the serial line for debugging
  Serial.begin(115200);
  
  WiFiManager wifiManager;
  wifiManager.setTimeout(180);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  wifiManager.setAPCallback(configModeCallback);

  Serial.println("connected...yeey :)");

  delay(100);
  
   if(!wifiManager.autoConnect()) {
      Serial.println("failed to connect and hit timeout");
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(1000);
    } 

  //wait a bit before starting the main loop
  delay(5000);      

}

void loop(){

  //reconnect if connection is lost
  if (!client.connected() && WiFi.status() == 3) {reconnect();}

  //maintain MQTT connection
  client.loop();

  //MUST delay to allow ESP8266 WIFI functions to run
  delay(1000); 

  String humchar = String(23);
  String tempchar = String(43);
  String soilchar = String(54);
  String lightchar = String(65);
  
  humchar.toCharArray(charBuf_hum, 50);
  tempchar.toCharArray(charBuf_temp, 50);
  soilchar.toCharArray(charBuf_soil, 50);
  lightchar.toCharArray(charBuf_light, 50);
  
  client.publish(tempTopic, charBuf_temp);
  client.publish(humTopic, charBuf_hum);
  client.publish(soilTopic, charBuf_soil);
  client.publish(lightTopic, charBuf_light);
  
  delay(5000);

}

void callback(char* topic, byte* payload, unsigned int length) {

  //convert topic to string to make it easier to work with
  String topicStr = topic; 

  //Print out some debugging info
  Serial.println("Callback update.");
  Serial.print("Topic: ");
  Serial.println(topicStr);

}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void reconnect() {
  
  //attempt to connect to the wifi if connection is lost
  if(WiFi.status() != WL_CONNECTED){
    //debug printing
    Serial.print("Connecting to ");
//    Serial.println(ssid);

    //loop while we wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    //print out some more debug once connected
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

  }

  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if(WiFi.status() == WL_CONNECTED){
  // Loop until we're reconnected to the MQTT server
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");

      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);

      //if connected, subscribe to the topic(s) we want to be notified about
      if (client.connect(client_name, mqtt_username, mqtt_password)) {
        Serial.print("\tMTQQ Connected");
        client.subscribe(lightTopic);

      }

      //otherwise print failed for debugging
      else{Serial.println("\tFailed."); abort();}
    }
  }
}

//generate unique name from MAC addr
String macToStr(const uint8_t* mac){

  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5){
      result += ':';
    }
  }

  return result;
}
