#include <RN52_HardwareSerial.h>
#define PIN_SHUTDOWN 2
#define MAX_AMP_ON_TIME 20000
void setup() {
    char c;
    unsigned long initTime;
    unsigned long elapsedTime;
    //RN52_Serial3 object is instantiated globally in library.
    pinMode(PIN_A0, OUTPUT);
    pinMode(PIN_SHUTDOWN, OUTPUT);
    digitalWrite(PIN_A0, HIGH);
    digitalWrite(PIN_SHUTDOWN, LOW);
    Serial.begin(115200);
    while (!Serial){}
    Serial.println("Ready!");
    RN52_Serial3.begin(115200);
    delay(1000);
    Serial.println("Running RN52 setup:");
    digitalWrite(PIN_A0, LOW);  //drive low to init command mode
    while (RN52_Serial3.available() == 0){}  //wait for ACK (AOK\r\n or AOK\n\r, I forget which)
    Serial.println("Recieved something.");
    c = RN52_Serial3.read();
    if (c == 'C') {
       delay(100);
       RN52_Serial3.flush();
       Serial.println("Successfully entered command mode");
    }
    Serial.print("Please connect to the module and begin to play some music\n");
    while(Serial.available() == 0){}
    Serial.println("Recieved something.");
    while(Serial.available() > 0){
          c = Serial.read();
    }
    Serial.println(RN52_Serial3.getMetaData());
    Serial.println("Running PA init:");
    digitalWrite(PIN_A0, LOW);
    initTime = millis();
    Serial.print("Send a character when you wish to turn off the amp.\n");
    while(Serial.available() == 0 && elapsedTime < MAX_AMP_ON_TIME){
        elapsedTime = millis() - initTime;
        if(elapsedTime < MAX_AMP_ON_TIME/2){
            Serial.print("Half max on-time elapsed.\n");
        }
    }
    digitalWrite(PIN_A0, HIGH);
    Serial.println("Amp turned off.");
}

void loop() {
  //Do nothing
}