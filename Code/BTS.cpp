/* Optional code segments */
#DEFINE DEBUG 1
#DEFINE DISPLAY 1

/* Needed libraries */
#include "RN52_HardwareSerial.h"
#include "HardwareSerial.h"
#include "ILI9341_t3.h"
#include "font_DroidSansMono.h"

/* Libraries that might be needed? */
//TODO - Figure out required headers (check arduino programmer code)
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

/* Globals */
#ifdef DISPLAY
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);
#endfi

void main(void){
    /* Variables */
    char c, timeOut[13] = "00:00/00:00";
    char *s = &timeOut;
    string songArtist, songAlbum, songTitle;
    int songDuration, currentDuration, previousDuration = 0;    //songDuration is current a string in RN52_HWSerial
    int startTime, currentTime;
    int durationSeconds, durationMinutes;
    int CurrentSeconds, CurrentMinutes;
    /* Setup code */
    sei();  //enable interupts
    delay(1000);    //wait before setup
    pinMode(PIN_A0, OUTPUT);
    digitalWrite(PIN_A0, HIGH);
    #ifdef DEBUG
    Serial.begin(SERIAL_BAUD_RATE); //enable USB serial for tests
    while (!Serial){}
    Serial.println("Starting HC05 Serial");
    #endif
    Serial2.begin(SERIAL_BAUD_RATE);    //enable HC05    
    #ifdef DEBUG
    Serial.println("Starting RN52 Serial");
    #endif
    RN52_Serial3.begin(RN52_BAUD_RATE); //enable RN52
    digitalWrite(PIN_A0, LOW);
    while (RN52_Serial.available() == 0);   //wait for ACK (CMD\..., AOK\r\n or AOK\n\r - I forget which)
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
    tft.print(" Title:");
    tft.setCursor(10,48);
    tft.print("Artist:");
    tft.setCursor(10,92);
    tft.print(" Album:");
    tft.setCursor(10,136);
    tft.print("  Time:");
    delay(2000);
    tft.setCursor(64,4);
    tft.print("n/a");
    tft.setCursor(64,48);
    tft.print("n/a");
    tft.setCursor(64,92);
    tft.print("n/a");
    tft.setCursor(64,136);
    tft.print(timeOut);
    #endif
    
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
        /**
            Can this be done in a more clever way (does it tell the current second of the song so that
            it would be possible to only update the rest of the meta data if the song changes?
            Otherwise run something where we only look for meta data on every millis()%100 or something.
            That way we will not require delays which would probably make it feel like there is input lag.
            
            For current time in the song we might need to make an var that resets if the duration changes
            and use millis to discover where in the song we are. I do not think that count is anything
            other than the total number of tracks.
        **/
        songAlbum = RN52_Serial3.album();
        songTitle = RN52_Serial3.title();     
        songArtist = RN52_Serial3.artist();    
        songNumber = RN52_Serial3.trackNumber();
        songDuration = RN52_Serial3.trackDuration();
        /** add protection case for this stuff? **/
        #ifdef DEBUG
        Serial.print("Title: ");
        Serial.println(songTitle);
        #endif
        if (songDuration != previousDuration){
            startTime = millis();
        }
        durationSeconds = (songDuration/1000)%60;
        durationMinutes = (songDuration/1000)%3600;
        currentTime = millis()-startTime();
        currentSeconds = (currentTime/1000)%60;
        currentMinutes = (currentTime/1000)%3600;
        delay(100);  
        sprintf(s,"%2d:%2d/%2d:%2d", currentMinutes, currentSeconds, durationMinutes, durationSeconds);
        #ifdef DEBUG
        Serial.println(timeOut);
        #endif
        #ifdef DISPLAY
        /* Using the metadata information we can now update the text on the screen. */
        /** write a bacl rect over test that is to be updated **/
        tft.setCursor(64,4);
        tft.print(songTitle);
        tft.setCursor(64,48);
        tft.print(songArtist);
        tft.setCursor(64,92);
        tft.print(songAlbum);
        tft.setCursor(64,136);
        tft.print(timeOut);
        #endif
    }      
}