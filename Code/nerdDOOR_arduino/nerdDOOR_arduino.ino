#include <Arduino.h>
#include <MFRC522.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Servo.h"
#define DEG_CLOSED 5
#define DEG_OPEN 93
#define SERVO_PIN 5
#define LED_PIN 16

#define ss_pin 15
#define rst_pin 4 // not used

int doorstate = 0;

Servo servo;
MFRC522 mfrc522(ss_pin,rst_pin);
WiFiClient espClient;
PubSubClient client(espClient);
void callback(char* topic,byte* payload, unsigned int length);

const char* ssid = ""; // WiFi SSID
const char* password = ""; // WiFi pwd

IPAddress local_IP();
IPAddress gateway();
IPAddress subnet();
IPAddress primaryDNS();   //optional
IPAddress secondaryDNS(); //optional

const char* mqttServer = ""; //mqtt server ip
const int mqttPort = 1883;
const char* mqttUser = ""; //mqtt user
const char* mqttPassword = ""; //mqtt pwd

boolean connectMQTT(){

  client.connect("ESP-Porta",mqttUser,mqttPassword);

  return client.connected();

}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN,OUTPUT);
  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.begin(ssid,password);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println("Connecting to WiFi....");
  }

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttServer,mqttPort);
  client.setCallback(callback);

  while(!client.connected()){

    Serial.println("Connecting to MQTT....");

    if(connectMQTT()){
      Serial.println("Connected to MQTT!");
      client.subscribe("NeRDDOOR/nerd/State");
    }else{
      Serial.print("failed with state:");
      Serial.print(client.state());
      delay(500);
    }

  }

//  client.publish("nerddoor","ESP8266 connected!");

}

void doorloop(){

  client.loop();

  if(doorstate != 0){
    if(doorstate++ > 70){
      doorstate = 0;
      Serial.println("Closing door...");
      servo.attach(SERVO_PIN);
      servo.write(DEG_CLOSED);
      delay(500);
      servo.detach();
      digitalWrite(LED_PIN,HIGH);
    }
    digitalWrite(LED_PIN,!digitalRead(LED_PIN));
    delay(100);
    return;
  }
  
  if(!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()){
    delay(50);
    return;
  }

  String content = "";

  for(byte i=0;i<mfrc522.uid.size;i++){
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    content.concat(String(mfrc522.uid.uidByte[i],HEX));
  }

  content.toUpperCase();

  Serial.println(content);

  client.publish("NeRDDOOR/nerd/RFID",(char*)content.c_str());

}

void loop() {
  
  if(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println("Reconnecting to WiFi...");
  }else if(!client.connected()){
    connectMQTT();
    delay(500);
  }else{
    doorloop();
  }

}

void callback(char* topic,byte* payload, unsigned int length){

  String msg = "";

  for(int i=0;i<length;i++){
    msg += (char)payload[i];
  }

  Serial.println(msg);

  if(msg == "Open"){
    doorstate = 1;
    servo.attach(SERVO_PIN);
    servo.write(DEG_OPEN);
    delay(500);
    //servo.detach();
  }

}
