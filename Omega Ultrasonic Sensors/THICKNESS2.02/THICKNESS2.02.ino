//AM I NECESSARY?
#include <SPI.h>
//AM I NECESSARY?

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
const char* ssid = "TAKTL";
const char* password = "ConcreteConnection2016#";
char* topic = "Casting";
char* server = "10.10.10.193";
String clientName;
String str;
int sensorpin[3] =  {33, 34, 35};
int minMeasure = 105;
int maxMeasure = 155;




#include <Wire.h>
#include "LedControl.h"
LedControl dispMaster = LedControl(21,22,25,3); 

#include <QuickStats.h>
QuickStats stats;

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
    if (client.publish(topic, "hello from ESP32")) {
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

  //Wake up the MAX72XX from power-saving mode
  dispMaster.shutdown(L, false);
  dispMaster.shutdown(C, false);
  dispMaster.shutdown(R, false);
  //Set full brightness for the LEDs
  dispMaster.setIntensity(L,15);
  dispMaster.setIntensity(C,15);
  dispMaster.setIntensity(R,15);


  //MAYBE NO??
  pinMode(3, INPUT); //Absolute/Relative switch
  
  
  //ATTACH 'ZERO' INTERRUPT
  byte ZeroPin = 1
  attachInterrupt(digitalPinToInterrupt(ZeroPin,ZERO,FALL);
  //ATTACH 'START/STOP' INTERRUPT
  byte LogPin = 2
  attachInterrupt(digitalPinToInterrupt(LogPin,STARTSTOP,FALL);
  //pinMode(5, INPUT); //BEGINNING/END OF CASTING  
}


void loop() {
  //FOR DISPLAYS 1 to 3 -ANALOG READ LOOP
    //LOOP WHILE DATA IS POOR QUALITY
      //FOR SAMPLES 1 to N LOOP
  int sensorRaw = 0;
  double sensorVolts = 0;
  double sensorDistance = 0;
  int numSamples = 20;
  float measurementArray[numSamples];

  for(int i = 0; i < 3; i++){
      do{
        for(int j = 0; j < numSamples; j++){
          sensorRaw = analogRead(sensorpin(i));
          sensorRaw = sensorRaw/1170
          
          measurementArray[j]= sensorRaw;
          delay(20);
        }
      }while((abs(stats.mode(measurementArray,numSamples,0.00001)- stats.average(measurementArray,numSamples) > 1.5)) || stats.CV(measurementArray,numSamples)>1.50);
  }
  


  
  //CHECK FOR ZERO FLAG

  //FOR DISPLAYS 1 to 3 - CHARACTER SPLITTING
    //CHARACTER SPLITTING
  chars[0] = analogstr.charAt(0);
  chars[1] = analogstr.charAt(1);
  chars[2] = analogstr.charAt(2);
  chars[3] = analogstr.charAt(3);
  chars[4] = analogstr.charAt(4);

  //FOR DISPLAYS 1 to 3 - CHARACTER WRITING

  //MQTT PUB
  client.publish(topic,payload);
  Serial.print("Published: ");
  Serial.println(payload);

  //DELAY

}


//void zero()
  //FOR 1 to 3 LOOP
    //ZERO FLAG

      //OOORRRRR?
    
    //ANALOGREAD = 0
    //WRITE TO EEPROM???


//void startstop()
  //MQTT MESSAGE

  
