/* Optional code segments - using this to seperately test code segments */

#define DEBUG 1
//#define DISPLAY 1
//#define HC05 1

/* Requried libraries */
#include <HardwareSerial.h>
#include <ILI9341_t3.h>
#include <font_DroidSansMono.h>
#include <RN52_HardwareSerial.h>
#include <WString.h>

/* Libraries that may be needed */ /** TODO - Figure out required headers (check arduino programmer code) **/
#include "SPI.h"

/* Definitions */
#define TFT_DC      21
#define TFT_CS      20
#define TFT_RST    255  // 255 = unused, connect to 3.3V
#define TFT_MOSI    11
#define TFT_SCLK    13
#define TFT_MISO    12
#define SERIAL_BAUD_RATE 9600
#define RN52_BAUD_RATE 115200
#define BUTTON_HEIGHT 60
#define BUTTON_WIDTH 64
#define METADATA_RESET 10000
#define GPIO2_PIN 17
#define PIN_SHUTDOWN 2

/* Globals */
#ifdef DISPLAY
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);
#endif

/* Function delarations */
void shutdown(void); //write this function

/** Finish Metadata related AT commands in HWSerial before using **/
int main(void){
    /* Variables */
    #ifdef DEBUG
    int timer = 0;
    #endif
    char c, timeOut[12];
    char *s;
    s = timeOut;
    strcpy(timeOut, "00:00/00:00");
    String songArtist = "n/a", songAlbum = "n/a", songTitle = "n/a";
    String previousAlbum = "n/a", previousTitle = "n/a", previousArtist = "n/a";
    int songDuration, currentDuration, previousDuration = 0;    //songDuration is current a string in RN52_HWSerial
    int startTime = 0, elapsedTime = 0, timeAtPause = 0;
    int durationSeconds = 0, durationMinutes = 0;
    int elapsedSeconds = 0, elapsedMinutes = 0;
    int eventRegStatus = 0;
    /** Insert code to deal with flags in metadata handling (reset in particular) **/
    bool newSongFlag = false, pausedFlag = false, previousPausedFlag = false, eventBit5Flag = false, GPIO2Status = true;
    uint8_t pausedFlagArray = 0; /** Implement this without bools **/
    /* Setup code */
    setup();
    sei();  //enable interupts
    pinMode(PIN_A0, OUTPUT);    //pin for command mode
    digitalWrite(PIN_A0, HIGH);
    pinMode(GPIO2_PIN, INPUT); //pin for event register
    pinMode(PIN_SHUTDOWN, OUTPUT);  //pin for PA enable
    digitalWrite(PIN_SHUTDOWN, LOW);
    delay(1000);    //wait before setup
    #ifdef DEBUG
    Serial.begin(SERIAL_BAUD_RATE); //enable USB serial for tests
    while (!Serial){}
    Serial.println("Starting HC05 Serial");
    #endif
    #ifdef HC05
    Serial2.begin(SERIAL_BAUD_RATE);    //enable HC05
    #endif
    #ifdef DEBUG
    Serial.println("Starting RN52 Serial");
    #endif
    RN52_Serial3.begin(RN52_BAUD_RATE); //enable RN52
    digitalWrite(PIN_A0, LOW);
    while (RN52_Serial.available() == 0);   //wait for ACK (CMD)
    c = RN52_Serial3.read();
    if (c == 'C') {
        delay(100);
        RN52_Serial3.flush();
        #ifdef DEBUG
        Serial.println("Successfully entered command mode.");
        #endif
    }
    #ifdef DISPLAY
    /* Setup the buttons on the display */
    tft.begin();
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_PINK);
    tft.setTextSize(2);
    tft.setFont(DroidSansMono_40);
    delay(2000);
    /** 
        Later these can be made into bitmaps of actual buttons?
    **/
    tft.fillRect(0,180,BUTTON_WIDTH,BUTTON_HEIGHT,ILI9341_BLUE);
    tft.fillRect(64,180,BUTTON_WIDTH,BUTTON_HEIGHT,ILI9341_GREEN);
    tft.fillRect(128,180,BUTTON_WIDTH,BUTTON_HEIGHT,ILI9341_RED);
    tft.fillRect(192,180,BUTTON_WIDTH,BUTTON_HEIGHT,ILI9341_ORANGE);
    tft.fillRect(256,180,BUTTON_WIDTH,BUTTON_HEIGHT,ILI9341_DARKCYAN);
    tft.setCursor(10,4);
    /* Initialise the song display */
    tft.print(" Title:");
    tft.setCursor(10,48);
    tft.print("Artist:");
    tft.setCursor(10,92);
    tft.print(" Album:");
    tft.setCursor(10,136);
    tft.print("  Time:");
    delay(2000);
    tft.setCursor(64,4);
    tft.print(songTitle);
    tft.setCursor(64,48);
    tft.print(songArtist);
    tft.setCursor(64,92);
    tft.print(songAlbum);
    tft.setCursor(64,136);
    sprintf(s,"%2d:%2d/%2d:%2d", elapsedMinutes, elapsedSeconds, durationMinutes, durationSeconds);
    tft.print(timeOut);
    #endif
    digitalWrite(PIN_SHUTDOWN, HIGH); // turn on PA after setup complete
    /* Operational code */
    for(;;){
        #ifdef DEBUG
        delay(80);
        timer = millis();
        #endif
        #ifdef HC05
        /* Check for HC05 commands */
        if(Serial2.available() != 0){
            c = Serial2.read();
            switch(c){
                case c == ' ':
                    RN52_Serial3.playPause();    //pause
                    pausedFlag = pausedFlag ? false : true; //toggle paused flag
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
                    newSongFlag = true;
                    pausedFlag = false;
                    #ifdef DEBUG
                    Serial.println("Song skipped.");
                    #endif
                    break;
                case c == '<':
                    RN52_Serial3.prevTrack();    //previous
                    newSongFlag = true;
                    pausedFlag = false;
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
        #endif
        /* Check bit 5 of the status register to see if there has been a new song */
        GPIO2Status = digitalRead(GPIO2_PIN);
        if(!GPIO2Status){
            eventRegStatus = RN52_Serial3.queryState();
            if(eventRegStatus & (1 << 5)){
                newSongFlag = true;
            }
        }
        #ifdef DEBUG
        Serial.print("eventRegStatus");
        Serial.println(eventRegStatus & (1 << 5));
        #endif
        /** 
            Do some maths here in an attempt to detect if a new song is possible, if it is set the newSongFlag.
            Maybe: start time + elasped > duration.
            On second thoughts, do I need this?
        **/
        /* Now get metadata information */
        /** CHECK THIS CONDITION ESP strcmp **/
        if( (strcmp(timeOut, "00:00/00:00") == 0) || millis()%METADATA_RESET == 0 || newSongFlag){  //runs on startup, every n seconds and on changes 
            previousAlbum = songAlbum;  //save the old versions of the text so that we can wipe screen
            songAlbum = RN52_Serial3.album();
            previousTitle = songTitle;
            songTitle = RN52_Serial3.trackTitle();  
            previousArtist = songArtist;            
            songArtist = RN52_Serial3.artist();
            previousDuration = songDuration;         
            songDuration = RN52_Serial3.trackDuration();
            if(songTitle == "" || songArtist == ""){
                newSongFlag = false; 
                continue;  //if info is blank then we have no song playing.
            }
            else if (songDuration != previousDuration){
                newSongFlag = true;
            }
        }
        pausedFlag = newSongFlag ? false : pausedFlag;    //reset paused flag if a new song is detected (default spotify behaviour).
        #ifdef DEBUG
        if(newSongFlag){           
            Serial.print("Title: ");
            Serial.println(songTitle);
        }
        #endif
        /* Time update */
        if(newSongFlag){
            startTime = millis();
            #ifdef DEBUG          
            Serial.print("Title: ");
            Serial.println(songTitle);
            #endif
        }
        durationSeconds = (songDuration/1000)%60;
        durationMinutes = (songDuration/1000)%3600;
        /* Account for the duration staying constant during a pause. Only for own pauses as AVRCP doesnt give pause data. */
        pausedFlagArray |= pausedFlag;
        switch(pausedFlagArray){
                case pausedFlagArray == 0:  //keep increasing elapsed time
                    elapsedTime = millis() - startTime;
                    break;                
                case pausedFlagArray == 1:  //save the time it was paused at
                    timeAtPause = millis();
                    break;                
                case pausedFlagArray == 2:  //compute new start time
                    startTime = timeAtPause - elapsedTime;  //lol bodged
                    break;                
                case pausedFlagArray == 3:  //still paused
                    break;
                default:
                    shutdown();
                    #ifdef DEBUG
                    Serial.println("Error");
                    #endif
            }
        elapsedSeconds = (elapsedTime/1000)%60;
        elapsedMinutes = (elapsedTime/1000)%3600;
        #ifdef DISPLAY  
        tft.setTextColor(ILI9341_BLACK);        
        tft.setCursor(64,136);
        tft.print(timeOut); //print the old time in black
        tft.setTextColor(ILI9341_PINK);
        #endif
        previousTimeOut = timeOut;
        sprintf(s,"%2d:%2d/%2d:%2d", elapsedMinutes, elapsedSeconds, durationMinutes, durationSeconds);
        #ifdef DEBUG
        if(timeOut != previousTimeOut){
            Serial.println(timeOut);
        }
        #endif
        #ifdef DISPLAY
        tft.setCursor(64,136);
        tft.print(timeOut); //print the new time
        /* Metadata Update */
        if(newSongFlag){
            tft.setTextColor(ILI9341_BLACK);
            tft.setCursor(64,4);
            tft.print(previousTitle);
            tft.setCursor(64,48);
            tft.print(previousArtist);
            tft.setCursor(64,92);
            tft.print(previousAlbum);            
            tft.setTextColor(ILI9341_PINK);
            tft.setCursor(64,4);
            tft.print(songTitle);
            tft.setCursor(64,48);
            tft.print(songArtist);
            tft.setCursor(64,92);
            tft.print(songAlbum);
            tft.setCursor(64,136);
        }
        #endif
        pausedFlagArray = pausedFlag << 1; /** check this to ensure functionality **///update previously paused flag. 
        newSongFlag = false;    //reset new song flag
        #ifdef DEBUG
        print("Loop time: ")
        println(millis - timer);
        #endif
    }
    shutdown(); //if this is reached something really bad has happened.    
}

void shutdown(void){ // send exit cmd mode, do all shutdown stuff
    digitalWrite(PIN_SHUTDOWN, LOW); //turn off the PA
    #ifdef DEBUG
    println("-----------------------")
    println("Something bad happened.")
    println("-----------------------")
    #endif
}
