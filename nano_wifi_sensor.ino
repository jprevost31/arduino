/*
 * Program to read temperature and humidity data
 * from DHT11 sensor board, connect to local WIFI
 * netork, then send data to an MQTT broker service.
 * 
 * Configuration information is found in the associated
 * arduino_secrets header file.
 * 
 * Written by Jeff Prevost, 3/28/2022
 */


#include "DHT.h"
#include "PubSubClient.h" // Connect and publish to the MQTT broker
#include <WiFiNINA.h> // implement the WIFI NinA library
#include "arduino_secrets.h"

#define DHTPIN 7  // Pin connected to the DHT sensor
#define DHTTYPE DHT11  // DHT11 or DHT22

DHT dht(DHTPIN, DHTTYPE);

// WiFi
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password (use for WPA, or use as key for WEP)

// MQTT
// Read data from arduino_secrets file

char mqtt_port_num[] = SECRET_MQTT_PORT;
char mqtt_server[] = SECRET_MQTT_SERVERIP;
char mqtt_username[] = SECRET_MQTT_UNAME;
char mqtt_password[] = SECRET_MQTT_UPASS;//
char clientID[] = SECRET_MQTT_CLIENTID;

// Set variables for identifying device and sensor channel names for MQTT
String location = "study";
String topic_1 = location + "/humidity";
String topic_2 = location + "/temperature";

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
// 1883 is the listener port for the Broker
PubSubClient client(mqtt_server, int(mqtt_port_num), wifiClient); 

void connect_WiFi()
{
  // Connect to the WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  // Wait until the connection has been confirmed before continuing
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Debugging - Output the IP Address of the ESP8266
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Custom function to connect to the MQTT broker via WiFi
void connect_MQTT(){
  
  connect_WiFi();

  // Connect to MQTT Broker
  Serial.print("Connecting to MQTT Broker ");
  Serial.print(mqtt_server);
  Serial.print(":");
  Serial.println(mqtt_port_num);
  // client.connect returns a boolean value to let us know if the connection was successful.
  // If the connection is failing, make sure you are using the correct MQTT Username and Password (Setup Earlier in the Instructable)
  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
}

void readTopic1()
{
  // convert reading to Farenheit
  float t = 1.8 * dht.readTemperature() + 32.0;

  Serial.print(location);
  Serial.print(" - Temperature: ");
  Serial.print(t);
  Serial.println(" *F");

   // PUBLISH topic_1 to the MQTT Broker. While publish returns false, reconnect to wifi and MQTT and try again
  while (!client.publish(topic_1.c_str(), String(t).c_str())) 
  {
    connect_MQTT();
    delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
  }
  Serial.println( "Topic_1 value sent!" );
}

void readTopic2()
{
  float h = dht.readHumidity();

  Serial.print(location);
  Serial.print(" - Humidity: ");
  Serial.print(h);
  Serial.println(" %");

   // PUBLISH topic_2 to the MQTT Broker. While publish returns false, reconnect to wifi and MQTT and try again
  while (!client.publish(topic_2.c_str(), String(h).c_str())) 
  {
    connect_MQTT();
    delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
  }
  Serial.println( "Topic_2 value sent!" );
}

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  connect_WiFi();
  connect_MQTT();
  Serial.setTimeout(2000);
  
  readTopic1();
  readTopic2();

  client.disconnect();  // disconnect from the MQTT broker
  delay(1000*10);       // print new values every 10 seconds
}