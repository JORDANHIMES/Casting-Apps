#include <PubSubClient.h>
#include <ESP8266WiFi.h>
const char* ssid = "TAKTL";
const char* password = "ConcreteConnection2016#";
char* topic = "Casting";
char* server = "10.10.10.193";

String clientName;
String str;
//char payload[];

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.println("received some data");
}
WiFiClient wifiClient;

PubSubClient client(server, 1883, callback, wifiClient);

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}



void setup() {
  Serial.begin(57600);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  
  clientName += "esp8266-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(micros() & 0xff, 16);
  Serial.print("Connecting to ");
  Serial.print(server);
  Serial.print(" as ");
  Serial.println(clientName);
  if (client.connect((char*) clientName.c_str())) {
    Serial.println("Connected to MQTT broker");
    Serial.print("Topic is: ");
    Serial.println(topic);


    if (client.publish(topic, "hello from ESP8266")) {
      Serial.println("Publish ok");
    }
    else {
      Serial.println("Publish failed");
    }
  }
  else {
    Serial.println("MQTT connect failed");
    Serial.println("Will reset and try again...");
    abort();
  }


}



void loop() {
  
  while(WiFi.status() == WL_CONNECTED){
    if(Serial.available() > 0)
      {
        str = Serial.readStringUntil('\n');
        str2= Serial.readStringUntil('\n');
        Serial.print("Received: ");
        Serial.println(str2);
        char payload[str.length()+1];
        str2.toCharArray(payload,str.length()+1);
        client.publish("CASTING",payload);
        Serial.print("Published: ");
        Serial.println(payload);
        delay(2000);
      }
  }

  if(WiFi.status() != WL_CONNECTED){
    setup();
  }
  
    
}
