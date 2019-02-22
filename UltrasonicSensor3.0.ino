#include "LedControl.h"
LedControl dispMaster = LedControl(27,26,25,3);

int analogpins[] = {34, 39, 36};
float zero[] = {123.6,124.67,125.3};


const int numreadings = 750;
float avgs[]={0,0,0};
float output[] = {0,0,0};
float readingmtx[3][numreadings] = {};
float copymtx[3][numreadings]= {};

void setup() {
  int i;
  int j;
  Serial.begin(115200);

  for (i = 0; i <= 2; i++){
    adcAttachPin(analogpins[i]);
    adcStart(analogpins[i]);
    pinMode(analogpins[i],INPUT);
    dispMaster.shutdown(i, false);
    dispMaster.setIntensity(i, 15);
  }

  //FILL WITH ZEROS
  for (i = 0; i <= 2; i++){
    for (j = 0; j <= numreadings - 1; j++){
      readingmtx[i][j] = 0;
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
}


void loop() {
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
   delay(3000);

   //SERIAL OUT 'ZERO'
   String strzeroout = "Zero[0]: " + String(zero[0]) + ", Zero[1]: " + String(zero[1]) + ", Zero[2]: " + String(zero[2]);
   Serial.println(strzeroout);
  }

  //CHECK FOR WAKE UP DISPLAYS BUTTON
  if(digitalRead(21) == LOW){
   for (i = 0; i <= 2; i++){
    dispMaster.shutdown(i, false);
    dispMaster.setIntensity(i, 15);
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
  }

  //SERIAL OUT STRING PER READING
  String strout = "Moving average output: " + String(output[0]) + "," + String(output[1]) + "," + String(output[2]);
  Serial.println(strout);
  
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
