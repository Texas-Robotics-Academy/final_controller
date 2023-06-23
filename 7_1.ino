#include <BnrOneA.h>
#include <EEPROM.h>
#include <SPI.h>

using namespace std;

BnrOneA one;

#define SSPIN 2

float kp = 1.0, ki = 1.0, kd = 1.0, kc = 1.0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  one.spiConnect(SSPIN);
  one.stop();
  //one.obstacleEmitters(false);
  one.obstacleEmitters(true);
}

void printAsterisks(int index) {
  byte asterisks[17];
  for(int i = 0; i < 8; i++) {
    if(i == index) {
      asterisks[i * 2] = '*';
      asterisks[i * 2 + 1] = '*';
    } else {
      asterisks[i * 2] = '-';
      asterisks[i * 2 + 1] = '-';
    }
  }
  asterisks[16] = 0;
  one.lcd1(asterisks);
}

int findMaxLine() {
  byte adcVals[8];
  int maxIdx = 0;
  for(int i = 0; i < 8; i++) {
    adcVals[i] = one.readAdc(i);
  }
  
  int maxVal = adcVals[0];
  maxIdx = 0;
  
  for(int i = 1; i < 8; i++) {
    if(adcVals[i] > maxVal) {
      maxIdx = i;
      maxVal = adcVals[i]; 
    }
  }

  return maxIdx;
}

float lastError = 0.0;
float totalError = 0.0;
int lastButton = 0;
int curAdjust = 0;

void loop() {
  int maxIdx = findMaxLine();
  
  float error = ((float)maxIdx - 3.5) / 3.5;
  float d = error - lastError;
  totalError += error;
  
  float left = kc + (kp * error + kd * d + ki * totalError);
  float right = kc - (kp * error + kd * d + ki * totalError);
  
  if(left > 100)  {
    left = 100;
  } else if(left < -100) {
    left = -100;
  }
  
  if(right > 100)  {
    right = 100;
  } else if(right < -100) {
    right = -100;
  }
  
  one.move(left, right);
  lastError = error;

  int button = one.readButton();
  switch(button) {
    case 0:
      break;
    case 1:
      if(lastButton != button) {
        curAdjust += 1;
        if(curAdjust == 4) {
          curAdjust = 0;
        }
      }
      break;
    case 2:
      switch(curAdjust) {
        case 0:
          kc += 0.01;
          break;
        case 1:
          kp += 0.01;
          break;
        case 2:
          ki += 0.01;
          break;
        case 3: 
          kd += 0.01;
          break;
      }
      break;
    case 3:
      switch(curAdjust) {
        case 0:
          kc -= 0.01;
          break;
        case 1:
          kp -= 0.01;
          break;
        case 2:
          ki -= 0.01;
          break;
        case 3: 
          kd -= 0.01;
          break;
      }
      break;
  }

  String out1 = "kc " + String(kc) + " kp " + String(kp);
  String out2 = " ki " + String(ki) + " kd " + String(kd);

  one.lcd1(out1);
  one.lcd2(out2);

//  printAsterisks(maxIdx);
}
