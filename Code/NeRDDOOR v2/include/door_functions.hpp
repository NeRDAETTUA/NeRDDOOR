#include <SPI.h>
#include <ESP8266WiFi.h>
#include <MFRC522.h>
#include <PubSubClient.h>

#define LED_PIN LED_BUILTIN
#define SS_PIN 15
#define RESET_PIN 4 // NOT USED

void callback(char* topic,byte* payload, unsigned int length);
bool verify_connections();
bool try_to_connect(const int wifi_timeout);
bool setup_connections();
bool check_card();
void open_door();