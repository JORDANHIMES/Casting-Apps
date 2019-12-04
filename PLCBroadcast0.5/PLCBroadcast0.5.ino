#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_system.h"

const int wdtTimeout = 8000;  //time in ms to trigger the watchdog
hw_timer_t *timer = NULL;

void IRAM_ATTR resetModule() {
  ets_printf("reboot\n");
  esp_restart();
}

String clientName;
const char* ssid = "TAKTL";
const char* password = "ConcreteConnection2016#";
char* topic = "CASTING/PLC";
char* server = "172.16.60.13";

long currentMillis = 0;
long previousMillis = 0;


char lastchar;
String input1 = "";
//String input2 = "";

WiFiClient espClient;


//STATIC IP SETTINGS
IPAddress local_IP(172, 16, 20, 18);
IPAddress gateway(172, 16, 20, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(172, 16, 30,110); 
IPAddress secondaryDNS(172, 16, 30, 104);

//SUBSCRIBE SETUP-EMPTY FOR NOW
void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.println("received some data");
}

PubSubClient client(server, 1883, callback, espClient);






//Mac Address decoding...
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

  //OUTPUT LED PIN
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  //WATCHDOG
  timer = timerBegin(0, 80, true);                  //timer 0, div 80
  timerAttachInterrupt(timer, &resetModule, true);  //attach callback
  timerAlarmWrite(timer, wdtTimeout * 1000, false); //set time in us
  timerAlarmEnable(timer);   

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
    ESP.restart();
  }

  WiFi.begin(ssid, password);
  previousMillis = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi..");
    digitalWrite(2,HIGH);
    delay(10);
    digitalWrite(2,LOW);
    delay(50);
    timerWrite(timer, 0); //reset timer (feed watchdog)
    currentMillis = millis();
    if(currentMillis - previousMillis > 5000){ESP.restart();}
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  clientName = "";
  clientName += "esp32-";
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
    
    //EDIT BIRTH MESSAGE!
    if (client.publish("CASTING/PLC", "PLC Broadcast reconnect")) {
      Serial.println("Publish ok");
    }
    
    else {
      Serial.println("Publish failed");
      ESP.restart();
    }
  }
  else {
    Serial.println("MQTT connect failed");
    Serial.println("Will reset and try again...");
    abort();
  }

  for (int i = 1; i <= 4; i++){
    digitalWrite(2,HIGH);
    delay(50);
    digitalWrite(2,LOW);
    delay(10);
  }  
}


void loop() {
    timerWrite(timer, 0); //reset timer (feed watchdog)

    if(Serial.available()>0) {
        digitalWrite(2,HIGH);
        do{
          lastchar = Serial.read();
          Serial.println(lastchar);
          timerWrite(timer, 0); //reset timer (feed watchdog)
          input1 += lastchar;
        }while(Serial.available()>0);
        //delay(1000);
        digitalWrite(2,LOW);
        Serial.println(input1);    
    }
    
    if (input1 != ""){
      
      char payload[input1.length()+1];
      input1.toCharArray(payload,input1.length()+1);
      client.publish(topic,payload);
      Serial.print("Published: ");
      Serial.println(input1);

           
      for(int i = 1; i <5; i++){
        client.loop();
        digitalWrite(2,HIGH);
        delay(25);
        digitalWrite(2,LOW);
        delay(25);
        timerWrite(timer, 0); //reset timer (feed watchdog)
      }
    }

    if (client.state() != 0 || WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnecting...");
      ESP.restart();
    }

    //HEARTBEAT
    //digitalWrite(2,HIGH);
    //delay(500);
    //digitalWrite(2,LOW);
    //delay(500);

    //lastchar = "";
    input1 = "";
    
}
