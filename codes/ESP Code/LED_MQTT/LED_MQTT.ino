//Using ESP8266
//Created by Yaser Ali Husen
//Youtube: https://www.youtube.com/c/YaserAliHusen
//Receive message from mqtt broker to turn on device
//Reference GPIO  https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
#include <iostream>

//Setup for MQTT and WiFi============================
#include <ESP8266WiFi.h>
//Library for MQTT:
#include <PubSubClient.h>
//Library for Json format using version 5:
#include <ArduinoJson.h>

//declare topic for mqtt
const char* topic_pub = "esp_to_node";
const char* topic_sub = "node_to_esp";

// Update these with values suitable for your network.
const char* ssid = "YOUR SSID WIFI";
const char* password = "PASSWORD WIFI";
const char* mqtt_server = "123.456.789.100"; //MQTT BROKER ADDRESS

//Setup for LED Max7219==============================
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 15
// Hardware SPI connection
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

WiFiClient espClient;
PubSubClient client(espClient);

//Variables=========================================
int BUTTON_PIN = 4; //button D1
bool status_button = false;
String message;
String json_received;
String msg = "Standby...";

//function for WIFI connection=======================================
void setup_wifi() {
  delay(100);
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//receive message as subscriber=====================================
void callback(char* topic, byte* payload, unsigned int length)
{
  //Receiving message as subscriber
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("JSON Received:");
  for (int i = 0; i < length; i++) {
    json_received += ((char)payload[i]);
    //Serial.print((char)payload[i]);
  }
  msg = json_received;
  Serial.println(json_received);
  json_received = "";
}

//function reconnect to MQTT broker=================================
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if you MQTT broker has clientID,username and password
    //please change following line to    if (client.connect(clientId,userName,passWord))
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      //once connected to MQTT broker, subscribe command if any
      client.subscribe(topic_sub);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//function setup====================================================
void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //subscribe topic
  client.subscribe(topic_sub);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  P.begin();

}

//funtion looping check button and display LED====================
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //check button, if pressed, send trigger to node-red
  int buttonValue = digitalRead(BUTTON_PIN);
  if (buttonValue == LOW )
  {
    if (status_button == false)
    {
      message = "TRIGGER";
      //char message[58];
      //msg.toCharArray(message, 58);
      Serial.println(message);
      //publish sensor data to MQTT broker
      client.publish(topic_pub, "TRIGGER");
      status_button = true;
      msg = "Analyzing";
    }
  }
  else if (buttonValue == HIGH)
  {
    if (status_button == true)
    {
      status_button = false;
    }
  }

  if (msg == "Analyzing")
  {
    P.print("Wait...");
  }
  else
  {
    //display to LED matrix================================
    if (P.displayAnimate())
      P.displayText(msg.c_str(), PA_LEFT, 100, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  }


}
