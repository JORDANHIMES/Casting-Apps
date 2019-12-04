#include "LedControl.h"
LedControl dispMaster = LedControl(27,26,25,3);

#include <WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include "esp_system.h"

#define EEPROM_SIZE 64

const int wdtTimeout = 5000;  //time in ms to trigger the watchdog
hw_timer_t *timer = NULL;

void IRAM_ATTR resetModule() {
  ets_printf("reboot\n");
  esp_restart();
}


String clientName;
const char* ssid = "TAKTL";
const char* password = "ConcreteConnection2016#";
char* topic = "CASTING/THICKNESS";
char* birthmsg = "MCU Reset";
char* server = "172.16.60.13";

long currentMillis = 0;
long previousMillis = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.println("received some data");
}

WiFiClient espClient;

//STATIC IP SETTINGS
IPAddress local_IP(172, 16, 20, 10);
IPAddress gateway(172, 16, 20, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(172, 16, 30,110); 
IPAddress secondaryDNS(172, 16, 30, 104);

PubSubClient client(server, 1883, callback, espClient);

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

int analogpins[] = {34, 39, 36};
float zero[] = {122.4,122.4,122.4};

const int numreadings = 750;
float ma50[]={0,0,0};
float ma250[]={0,0,0};
float ma750[]={0,0,0};
float output[] = {0,0,0};
float readingmtx[3][numreadings] = {};
float copymtx[3][numreadings]= {};

void setup() {
  int i;
  int j;
  Serial.begin(115200);

  //WATCHDOG
  timer = timerBegin(0, 80, true);                  //timer 0, div 80
  timerAttachInterrupt(timer, &resetModule, true);  //attach callback
  timerAlarmWrite(timer, wdtTimeout * 1000, false); //set time in us
  timerAlarmEnable(timer);   

  timerWrite(timer, 0); //reset timer (feed watchdog)
  
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
    ESP.restart();
  }
  
  //LOAD ZEROES
  EEPROM.begin(512);
  EEPROM.get(0,zero[0]);
  EEPROM.get(16,zero[1]);
  EEPROM.get(32,zero[2]);
  EEPROM.end();
  
  timerWrite(timer, 0); //reset timer (feed watchdog)

  //LOAD LAST READINGS??

  for (i = 0; i <= 2; i++){
    adcAttachPin(analogpins[i]);
    adcStart(analogpins[i]);
    pinMode(analogpins[i],INPUT);
    dispMaster.shutdown(i, false);
    dispMaster.setIntensity(i, 15);
  }

  //FILL WITH FIRST/LAST(?) READING
  for (i = 0; i <= 2; i++){
    for (j = 0; j <= numreadings - 1; j++){
      readingmtx[i][j] = 0;
    }
  }

  timerWrite(timer, 0); //reset timer (feed watchdog)

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  
  //REL VS ABS
  pinMode(18,INPUT_PULLUP);
  //ZERO BUTTON
  pinMode(19,INPUT_PULLUP);
  //WAKE UP DISPLAYS
  pinMode(21,INPUT_PULLUP);
  //MOLD OFFSET -1MM
  pinMode(12,INPUT_PULLUP);
  //MOLD OFFSET -1.5MM
  pinMode(13,INPUT_PULLUP);

  timerWrite(timer, 0); //reset timer (feed watchdog)
  
  conn();

  for (int i = 1; i <= 6; i++){
    digitalWrite(2,HIGH);
    delay(100);
    digitalWrite(2,LOW);
    delay(50);
  }
}


void conn(){
  
  timerWrite(timer, 0); //reset timer (feed watchdog)
  
  WiFi.begin(ssid, password);
  previousMillis = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi..");
    digitalWrite(2,HIGH);
    delay(25);
    digitalWrite(2,LOW);
    delay(25);
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
    if (client.publish("CASTING/THICKNESS", birthmsg)) {
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
  timerWrite(timer, 0); //reset timer (feed watchdog)
}



void loop() {
  //ADJUST TIMING HERE
  //delay(250);
  timerWrite(timer, 0); //reset timer (feed watchdog)
  client.loop();
  
  float y;
  float sum = 0;
  int i;
  int j;

  //CHECK FOR ZERO BUTTON
  if(digitalRead(19) == LOW){
   for (i = 0; i <= 2; i++){
      zero[i] = ma750[i];
   }
   //ADJUST ZERO 'PAUSE' TIME HERE
   delay(2000);
   String strzeroout = "Zero[0]: " + String(zero[0]) + ", Zero[1]: " + String(zero[1]) + ", Zero[2]: " + String(zero[2]);

   Serial.println(strzeroout);

   EEPROM.begin(512);
   EEPROM.put(0,zero[0]);
   EEPROM.put(16,zero[1]);
   EEPROM.put(32,zero[2]);
   EEPROM.end();
   
   char zeromqtt[strzeroout.length()+1];
   strzeroout.toCharArray(zeromqtt,strzeroout.length()+1);
   client.publish("CASTING/THICKNESS/ZERO",zeromqtt);
   Serial.print("Published: ");
   Serial.println(strzeroout);
  }

  timerWrite(timer, 0); //reset timer (feed watchdog)
  
  //CHECK FOR WAKE UP DISPLAYS BUTTON
  if(digitalRead(21) == LOW){
   for (i = 0; i <= 2; i++){
    dispMaster.shutdown(i, false);
    dispMaster.setIntensity(i, 15);
    dispMaster.clearDisplay(i);
   }
  }

  //WRITE NEW VALS TO FIRST ROW OF COPYMTX, WRITE READINGMTX TO REMAINING ROWS
  for (i = 0; i <= 2; i++){
    y = analogRead(analogpins[i]);
    y = (5.0*y)/409.6;
    copymtx[i][0]= y + 105;

    for (j = 1; j <= numreadings - 1; j++){
      copymtx[i][j] = readingmtx[i][j-1];
    }
  }

  //COPYMTX BACK TO READINGMTX (WITH NEW VALUES), CALCULATE AVGS
  for (i = 0; i <= 2; i++){
    
    //Serial.println("COPYMTX TO READINGMTX");
    
    sum = 0;
    for (j = 0; j <= numreadings - 1; j++){
      readingmtx[i][j] = copymtx[i][j];
      sum = sum + readingmtx[i][j];
      if(j == 49){
        ma50[i]= sum/50;
//        Serial.print("ma50[");
//        Serial.print(i);
//        Serial.print("]:");
//        Serial.println(ma50[i]);
      }
      else if(j==249){
        ma250[i] = sum/250;
//        Serial.print("ma250[");
//        Serial.print(i);
//        Serial.print("]:");
//        Serial.println(ma250[i]);
      }
    }
    ma750[i] = sum / numreadings;
//    Serial.print("ma750[");
//    Serial.print(i);
//    Serial.print("]:");
//    Serial.println(ma750[i]);

    //REL OR ABS READING
    if (digitalRead(18) == LOW){
      output[i] = -(ma750[i] - zero[i]);
    }
    else{
      output[i] = ma750[i];
    }

    //MOLD OFFSET
    if (digitalRead(12) == LOW){
      output[i] = output[i] - 1.0;
    }
    else if(digitalRead(13) == LOW){
      output[i] = output[i] -1.5;
    }

//    Serial.print("output[");
//    Serial.print(i);
//    Serial.print("]:");
//    Serial.println(output[i]);
  }


  timerWrite(timer, 0); //reset timer (feed watchdog)

  //MQTT OUT
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > 2000) {
    previousMillis = currentMillis;

    Serial.println("MQTT OUT");
    
    String strreadings = "Raw:" + String(readingmtx[0][0]) + "," + String(readingmtx[1][0]) + "," + String(readingmtx[2][0]);
    Serial.println(strreadings);
    char rdgsmqtt[strreadings.length()+1];
    strreadings.toCharArray(rdgsmqtt,strreadings.length()+1);
    client.publish("CASTING/THICKNESS/RAW",rdgsmqtt);
    
    String str50ma = "50ma:" + String(ma50[0]) + "," + String(ma50[1]) + "," + String(ma50[2]);
    Serial.println(str50ma);
    char mamqtt50[str50ma.length()+1];
    str50ma.toCharArray(mamqtt50,str50ma.length()+1);
    client.publish("CASTING/THICKNESS/50MA",mamqtt50);

    String str250ma = "250ma:" + String(ma250[0]) + "," + String(ma250[1]) + "," + String(ma250[2]);
    Serial.println(str250ma);
    char mamqtt250[str50ma.length()+1];
    str250ma.toCharArray(mamqtt250,str250ma.length()+1);
    client.publish("CASTING/THICKNESS/250MA",mamqtt250);

    String str750ma = "750ma:" + String(ma750[0]) + "," + String(ma750[1]) + "," + String(ma750[2]);
    Serial.println(str750ma);
    char mamqtt750[str750ma.length()+1];
    str750ma.toCharArray(mamqtt750,str750ma.length()+1);
    client.publish("CASTING/THICKNESS/750MA",mamqtt750);
    

    for(i = 0; i <=5; i++){
      client.loop();
      //delay(5);
    }

  }
  
  for(i = 0; i <=5; i++){
    client.loop();
    //delay(5);
  }
  
  timerWrite(timer, 0); //reset timer (feed watchdog)

  //CONVERTING READINGS TO CHARS FOR DISPLAYS
  for(i = 0; i<=2; i++){
    char first = 0;
    char sec = 0;
    char third = 0;
    char tenth = 0;
    float remainder = output[i] - int(output[i]);
    remainder = remainder * 10;

    //TENTHS PLACE IS A NUMBER OR ZERO
    if (remainder > 0){
      tenth = String(remainder).charAt(0);
    }
    else{
      tenth = String(remainder).charAt(1);
    }

    //POSITIVE OR NEGATIVE FOR 1ST DIGIT
    if (output[i] < 0){
      dispMaster.setChar(i,0,'-',0);
    }
    else{
      dispMaster.setChar(i,0,' ', 0);
    }

    //DECIDE WHERE DECIMAL GOES AND DISPLAY REMAINING 3 DIGITS
    if (abs(output[i]) >= 100){
      first = String(int(abs(output[i]))).charAt(0);
      sec = String(int(abs(output[i]))).charAt(1);
      third = String(int(abs(output[i]))).charAt(2);
      dispMaster.setDigit(i,1,first,0);
      dispMaster.setDigit(i, 2, sec, 0);
      dispMaster.setDigit(i, 3, third, 0);
    }
    else if(abs(output[i]) < 100 && abs(output[i]) >=10){
      first = String(abs(output[i])).charAt(0);
      sec = String(abs(output[i])).charAt(1);
      third = tenth;
      dispMaster.setChar(i,1,first,0);
      dispMaster.setChar(i, 2, sec, 1);
      dispMaster.setChar(i, 3, third, 0);
    }
    else if(abs(output[i]) <10){
      first = 48;
      //first = String(int(abs(output[i]))).charAt(0);
      sec = String(int(abs(output[i]))).charAt(0);
      third = tenth;
      dispMaster.setChar(i,1,first,0);
      dispMaster.setChar(i, 2, sec, 1);
      dispMaster.setChar(i, 3, third, 0);
    }
  }

  if(!WiFi.status() == WL_CONNECTED | client.state() != 0){
    Serial.println("Disconnect detected");
    birthmsg = "Disconnect detected, conn()";
    client.disconnect();
    WiFi.disconnect();
    delay(100);
    conn();
  }

  timerWrite(timer, 0); //reset timer (feed watchdog)
  
}
