#include "LedControl.h"
LedControl dispMaster = LedControl(27,26,25,3);

#include <WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>

#define EEPROM_SIZE 64

String clientName;
const char* ssid = "TAKTL";
const char* password = "ConcreteConnection2016#";
char* topic = "CASTING";
char* server = "172.16.60.13";

long currentMillis = 0;
long previousMillis = 0;

bool wififlag = false;

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.println("received some data");
}

WiFiClient espClient;
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
float avgs[]={0,0,0};
float output[] = {0,0,0};
float readingmtx[3][numreadings] = {};
float copymtx[3][numreadings]= {};

void setup() {
  int i;
  int j;
  Serial.begin(115200);

  EEPROM.begin(512);
  EEPROM.get(0,zero[0]);
  EEPROM.get(16,zero[1]);
  EEPROM.get(32,zero[2]);
  EEPROM.end();

  for (i = 0; i <= 2; i++){
    adcAttachPin(analogpins[i]);
    adcStart(analogpins[i]);
    pinMode(analogpins[i],INPUT);
    dispMaster.shutdown(i, false);
    dispMaster.setIntensity(i, 15);
  }

  //FILL WITH FIRST READING
  for (i = 0; i <= 2; i++){
    for (j = 0; j <= numreadings - 1; j++){
      readingmtx[i][j] = zero[i];
    }
  }

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


  WiFi.begin(ssid, password);
  previousMillis = millis();
  do{
    currentMillis = millis();
    delay(50);
    Serial.println("Connecting to WiFi..");
    wififlag = false;
  }while(currentMillis - previousMillis < 3000 && WiFi.status() != WL_CONNECTED);
  
  Serial.println("");
  if(WiFi.status() == WL_CONNECTED){
    wififlag = true;
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
      //Serial.println("Will reset and try again...");
      //abort();
    }
  }
  else{Serial.println("WiFi timeout");}
}


void loop() {
  if(WiFi.status() != WL_CONNECTED && wififlag == true){setup();}
  
  //ADJUST TIMING HERE
  delay(25);

  float y;
  float sum = 0;
  int i;
  int j;

  //CHECK FOR ZERO BUTTON
  if(digitalRead(19) == LOW){
   for (i = 0; i <= 2; i++){
      zero[i] = avgs[i];
   }
   //ADJUST ZERO 'PAUSE' TIME HERE
   delay(1000);
   String strzeroout = "Zero[0]: " + String(zero[0]) + ", Zero[1]: " + String(zero[1]) + ", Zero[2]: " + String(zero[2]);

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
    sum = 0;
    for (j = 0; j <= numreadings - 1; j++){
      readingmtx[i][j] = copymtx[i][j];
      sum = sum + readingmtx[i][j];
    }
    avgs[i] = sum / numreadings;

    //REL OR ABS READING
    if (digitalRead(18) == LOW){
      output[i] = -(avgs[i] - zero[i]);
    }
    else{
      output[i] = avgs[i];
    }

    //MOLD OFFSET
    if (digitalRead(12) == LOW){
      output[i] = output[i] - 1.0;
    }
    else if(digitalRead(13) == LOW){
      output[i] = output[i] -1.5;
    }
  }

  //SERIAL OUT STRING PER READING
  Serial.print("Mov avgs: ");
  Serial.print(String(avgs[0]));
  Serial.print(",");
  Serial.print(String(avgs[1]));
  Serial.print(",");
  Serial.println(String(avgs[2]));
  Serial.print("Readings: ");
  Serial.print(String(output[0]));
  Serial.print(",");
  Serial.print(String(output[1]));
  Serial.print(",");
  Serial.println(String(output[2]));

  //MQTT OUT
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > 3000) {
    
    previousMillis = currentMillis;
    
    String strmovavg = "Mov avgs: " + String(avgs[0]) + "," + String(avgs[1]) + "," + String(avgs[2]);
    char movavgmqtt[strmovavg.length()+1];
    strmovavg.toCharArray(movavgmqtt,strmovavg.length()+1);
    client.publish("CASTING/THICKNESS/AVG",movavgmqtt);
    
    String strreadings = "Readings: " + String(output[0]) + "," + String(output[1]) + "," + String(output[2]);
    char rdgsmqtt[strreadings.length()+1];
    strreadings.toCharArray(rdgsmqtt,strreadings.length()+1);
    client.publish("CASTING/THICKNESS/RAW",rdgsmqtt);
  }
  
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
}
