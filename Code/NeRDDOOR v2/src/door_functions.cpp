#include <door_functions.hpp>

MFRC522 rfid_reader(SS_PIN,RESET_PIN);
WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);
void callback(char* topic,byte* payload, unsigned int length);

const char* ssid = "";      // WIFI ssid
const char* password = "";  // WIFI password

const char* mqttServer = "";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";

std::vector<String> allowed_cards = {"99D1B4B1","99D1B4B2"}; // Cards for offline mode

void callback(char* topic,byte* payload, unsigned int length){

    String msg = "";

    for(int i=0;i<length;i++){
        msg += (char)payload[i];
    }

    Serial.println(msg);

    if(msg == "Open"){
        open_door();
    }
}

bool verify_connections(){
    mqtt_client.loop();

    if(WiFi.status() != WL_CONNECTED || !mqtt_client.connected()){
        return false;
    }else{
        return true;
    }
}

bool try_to_connect(const int wifi_timeout){        // wifi_timeout -> wifi connection timeout in seconds
    Serial.println("\nTrying to connect");

    int connection_tries = 0;

    while(WiFi.status() != WL_CONNECTED && connection_tries++ < wifi_timeout*2){
        Serial.print('.');
        delay(500);
    }

    if(WiFi.status() != WL_CONNECTED){
        Serial.println("Connection unsuccessful");
        return false;
    }
    
    mqtt_client.connect("ESP-Door",mqttUser,mqttPassword);

    mqtt_client.subscribe("NeRDDOOR/nerd/State");

    if(!mqtt_client.connected()){
        Serial.print("failed with state:");
        Serial.print(mqtt_client.state());
        return false;
    }

    Serial.println("Connection successful");

    return true;
}

void open_door(){
    Serial.println("Open door!");

    delay(1000);

    Serial.println("Close door!");
}

bool setup_connections(){
    Serial.begin(115200);
    pinMode(LED_PIN,OUTPUT);
    SPI.begin();
    rfid_reader.PCD_Init();
    rfid_reader.PCD_SetAntennaGain(rfid_reader.RxGain_max);

    WiFi.begin(ssid,password);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);

    mqtt_client.setServer(mqttServer,mqttPort);
    mqtt_client.setCallback(callback);

    try_to_connect(10);

    if(verify_connections()){
        Serial.println("Connected!");
        return true;
    }else{
        Serial.println("Couldn't connect!");
        return false;
    }
}

bool check_card(){
    if(!rfid_reader.PICC_IsNewCardPresent() || !rfid_reader.PICC_ReadCardSerial()){
        delay(50);
        return false;
    }

    String content = "";

    for(byte i=0;i<rfid_reader.uid.size;i++){
        content.concat(String(rfid_reader.uid.uidByte[i] < 0x10 ? "0" : ""));
        content.concat(String(rfid_reader.uid.uidByte[i],HEX));
    }

    content.toUpperCase();

    Serial.println(content);

    mqtt_client.publish("NeRDDOOR/nerd/RFID",(char*)content.c_str());

    if (std::find(allowed_cards.begin(), allowed_cards.end(), content) != allowed_cards.end()) {
        return true;
    }else{
        return false;
    }
}