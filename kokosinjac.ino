#include <Servo.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "pitches.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "Pushsafer.h"


#define FIVE_MIN (5*60*1000)
#define PUSH_KEY "fz6v8Xggw51E5DgfpeyS"
#define LEAVING_MESS 0
#define ENTERING_MESS 1
#define TOTAL_KOKE_CHANGED 2


char ssid[] = "9FE3B6";
char password[] = "vf3dcs0f1d";
WiFiClient client;
Pushsafer pushsafer(PUSH_KEY, client);
struct PushSaferInput input;

int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

int PIRsensor_pin = 2;
int PIRsensor_read = 0;

Servo servo;
int Servo_pin = 0;

int Piezo_pin = 4;

int DHT11_pin = 5;
float DHT11_read = 0;

DHT dht(DHT11_pin, DHT11);

int Photoresistor_pin = A0;
int Photoresistor_read = 0;

int koka_increment_pin = 13;
int koka_decrement_pin = 12;

int kokosinjac_count = 10;
int total_koke = 10;

bool isDoorOpen = 0;
bool isMorning = 0;
bool isRaining = 0;

void Pushsafer_function(int mType){
  String msg;
  if(mType == TOTAL_KOKE_CHANGED){
    msg = "Total number of chickens changed to: "+ total_koke;
  }
  else if(mType == LEAVING_MESS){
    msg = "The ckickens haven't left the henhouse.";
  }
  else if (mType == ENTERING_MESS){
    msg = "The chickens haven't entered the henhouse.";
  }
  else{
    msg = "Pushsafer messages working";
  }
  input.message = msg;

  Serial.println(pushsafer.sendEvent(input));
  Serial.println(msg);
}

bool PIR_function(bool leaving) {            
  int startTime = millis();
  while(millis()-startTime < FIVE_MIN){
    PIRsensor_read = digitalRead(PIRsensor_pin);  
    if (PIRsensor_read == HIGH) {
        if (leaving) {
          kokosinjac_count--;
          if(kokosinjac_count == 0){
            return false;
          }
        } else {
          kokosinjac_count++;
          if(kokosinjac_count == total_koke){
            return false;
          }
        }
    }
  }
  return true;
}

void Alarm_function(){
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(Piezo_pin, melody[thisNote], noteDuration);

    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(Piezo_pin);
  }
}

void setup() {
  pinMode(PIRsensor_pin, INPUT);
  servo.attach(Servo_pin);
  dht.begin();
  pinMode(koka_increment_pin, INPUT);
  pinMode(koka_decrement_pin, INPUT);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid, password);

  pushsafer.debug = true;
  input.title = "Alert!";
  input.sound = "29";
  input.vibration = "1";
  input.icon = "1";
  input.iconcolor = "#FFCCCC";
  input.priority = "2";
  input.device = "62033";
  input.url = "https://www.pushsafer.com";
  input.urlTitle = "Open Pushsafer.com";
  input.picture = "";
  input.picture2 = "";
  input.picture3 = "";
  input.time2live = "";
  input.retry = "";
  input.expire = "";
  input.confirm = "";
  input.answer = "";
  input.answeroptions = "";
  input.answerforce = "";

  Serial.begin(9600);
  delay(10000);
}

void loop() {

  Photoresistor_read = analogRead(Photoresistor_pin);
  if (Photoresistor_read > 650) {     //dan
    if (!isMorning) {                     //only open the door on sunrise
      isMorning = 1;
      Serial.println("Good Morning");
      if (!isDoorOpen) {
        servo.write(90);                  //turn 90 degrees
        delay(1000);
        isDoorOpen = 1;
        Serial.println("Door open");
        if(PIR_function(1)){              //count the chicken leaving for 5min
          Pushsafer_function(LEAVING_MESS);
          Alarm_function();
          kokosinjac_count = 0;
        }
        servo.write(0);                  //close the door
        isDoorOpen = 0;
      }
    }
  }
  else if (Photoresistor_read < 500) {  //night
    if (isMorning) {                        //close the door on sundown
      isMorning = 0;
      Serial.println("Good night");
      if (!isDoorOpen) {
        servo.write(90);                  //turn 90 degrees
        delay(1000);
        isDoorOpen = 1;
        if(PIR_function(0)){             //count the chicken coming to the henhouse for 5min
          Pushsafer_function(ENTERING_MESS);
          Alarm_function();
          kokosinjac_count = total_koke;
        }
        servo.write(0);  //return to 0 degrees
        delay(1000);
        isDoorOpen = 0;
        Serial.println("Door closed");
      }
      else{                       //door already open because of rain, chicken are already all in
        servo.write(0);           //return to 0 degrees
        delay(1000);
        isDoorOpen = 0;
        Serial.println("Door closed");
      }
    }
  }

  DHT11_read = dht.readHumidity();
  if(!isnan(DHT11_read)){
    if(DHT11_read >= 90.00){              //rain
      if (!isDoorOpen && !isRaining) {
        isRaining = 1;
        servo.write(90);                  //turn 90 degrees
        delay(1000);
        isDoorOpen = 1;
        Serial.println("Door open");
        if(PIR_function(0)){
          Pushsafer_function(ENTERING_MESS);
          Alarm_function();
          kokosinjac_count = total_koke;
        }
      }
    }
    else if(DHT11_read < 80){            //rain over, the ckickens should all come out and close the door
      if(isRaining && isDoorOpen){
        isRaining = 0;  
        if(PIR_function(1)){             //count leaving chicken for 5min
          Pushsafer_function(LEAVING_MESS);
          Alarm_function();
          kokosinjac_count = 0;
        }
        servo.write(0);           //return to 0 degrees
        delay(1000);
        isDoorOpen = 0;
        Serial.println("Door closed");        
      }      
    }
  }

  if(digitalRead(koka_increment_pin)){
    total_koke++;
    Pushsafer_function(TOTAL_KOKE_CHANGED);
  }
  if(digitalRead(koka_decrement_pin)){
    total_koke--;
    Pushsafer_function(TOTAL_KOKE_CHANGED);
  }   

  delay(FIVE_MIN/5);
}