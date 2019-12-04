#include <PubSubClient.h>
#include <ESP8266WiFi.h>


const char* ssid = "TAKTL";
const char* password = "ConcreteConnection2016#";
char* topic = "Casting";
char* server = "10.10.10.193";

String clientName;
String str;
String str2;
char payload[] = {};

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
  Serial.begin(115200);
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
    //abort();
  }


}



void loop() {

  while(WiFi.status() == WL_CONNECTED){
    Serial.begin(115200);
    str = Serial.readStringUntil('\n');
    //Serial.print("str Received: ");
    //Serial.println(str);
    Serial.flush();
    str2 = Serial.readStringUntil('\n');
    Serial.flush();
    Serial.print("str2 Received: ");
    Serial.println(str2);
    Serial.end();
    
  


    str2.toCharArray(payload,str2.length()+1);
    char payload[str2.length()+1];
    client.publish("CASTING",payload);
    //Serial.print("Published: ");
    //Serial.println(payload);
  }
  
  setup();
    
}
