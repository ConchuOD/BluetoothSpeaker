#DEFINE DEBUG 1

#include "RN52_HardwareSerial.h"
#include "HardwareSerial.h"
//TODO - Figure out required headers (check arduino programmer code)



void main(void){
    /* Setup code */
    delay(1000);    //wait before setup
    pinMode(PIN_A0, OUTPUT);
    digitalWrite(PIN_A0, HIGH);
    Serial2.begin(9600);    //enable HC05
    #ifdef DEBUG
    Serial.begin(9600); //enable USB serial for tests
    while (!Serial){} 
    #endif
    char c;
    string songArtist, songAlbum, songTitle;
    int songNumber, songCount, songDuration;    //the latter two are not yet actually ints
    RN52_Serial3.begin(115200); //enable RN52
    while (RN52_Serial.available() == 0); //wait for ACK (AOK\r\n or AOK\n\r, I forget which)
    c = RN52_Serial3.read();
    if (c == 'C') {
        delay(100);
        RN52_Serial3.flush();
        #ifdef DEBUG
        Serial.println("Successfully entered command mode.");
        #endif
    }
    /* Operational code */
    for(;;){
        /* Check for HC05 commands */
        if(Serial2.available() != 0){
            c = Serial2.read();
            switch(c){
                case c == ' ':
                    RN52_Serial3.playPause();    //pause
                    #ifdef DEBUG
                    Serial.println("Paused.");
                    #endif
                    break;
                case c == '+':
                    RN52_Serial3.volumeUp();    //volUp
                    #ifdef DEBUG
                    Serial.println("Volume increased.");
                    #endif
                    break;
                case c == '-':
                    RN52_Serial3.volumeDown();    //volDown
                    #ifdef DEBUG
                    Serial.println("Volume decreased.");
                    #endif
                    break;
                case c == '>':
                    RN52_Serial3.nextTrack();    //skip
                    #ifdef DEBUG
                    Serial.println("Song skipped.");
                    #endif
                    break;
                case c == '<':
                    RN52_Serial3.prevTrack();    //previous
                    #ifdef DEBUG
                    Serial.println("Song rewinded.");
                    #endif
                    break;
                default:
                    #ifdef DEBUG
                    Serial.println("Another button was pressed.");
                    #endif
            }   
        }
    /* Now get metadata information */ 
    songAlbum = RN52_Serial3.album();
    #ifdef DEBUG
    Serial.print("Album: ");
    Serial.println(songAlbum);
    #endif    
    songTitle = RN52_Serial3.title();     
    songArtist = RN52_Serial3.artist();    
    songNumber = RN52_Serial3.trackNumber();
    songCount = RN52_Serial3.totalCount();     
    songDuration = RN52_Serial3.trackDuration();  
    delay(100);    
        
        
        
        
    }
}