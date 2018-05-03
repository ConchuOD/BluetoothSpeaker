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
#include <SPI.h>

/* Definitions */
#define TFT_DC      21
#define TFT_CS      20
#define TFT_RST    255  // 255 = unused, connect to 3.3V
#define TFT_MOSI    11
#define TFT_SCLK    13
#define TFT_MISO    12
#define SERIAL_BAUD_RATE 115200
#define RN52_BAUD_RATE 115200
#define BUTTON_HEIGHT 60
#define BUTTON_WIDTH 64
#define METADATA_RESET 100
#define GPIO2_PIN 21
#define PIN_SHUTDOWN 2

/* Globals */
#ifdef DISPLAY
ILI9341_t3 TFT = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);
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
    char *s, *previousTimeOut;
    s = timeOut;
    strcpy(timeOut, "00:00/00:00");
    String song_artist = "n/a", song_album = "n/a", song_title = "n/a";
    String previous_album = "n/a", previous_title = "n/a", previous_artist = "n/a";
    int song_duration = 0, current_duration = 0, previous_duration = 0;    //song_duration is current a string in RN52_HWSerial
    int start_time = 0, elapsed_time = 0, time_at_pause = 0;
    int duration_seconds = 0, duration_minutes = 0;
    int elapsed_seconds = 0, elapsed_minutes = 0;
    int event_reg_status = 0;
    /** Insert code to deal with flags in metadata handling (reset in particular) **/
    bool new_song_flag = false, paused_flag = false, previous_paused_flag = false, event_bit_5_flag = false, GPIO_2_status = true;
    uint8_t paused_flag_array = 0; /** Implement this without bools **/
    /* Setup code */
    //setup();
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
    while (RN52_Serial3.available() == 0);   //wait for ACK (CMD)
    c = RN52_Serial3.read();
    if (c == 'C') {
        delay(100);
        RN52_Serial3.flush();
        #ifdef DEBUG
        Serial.println("Successfully entered command mode.");
        #endif
    }
    /* Turn on track change event */
    RN52_Serial3.trackChangeEvent(1);   
    #ifdef DEBUG
    Serial.print("Ext. features: ");
    Serial.println(RN52_Serial3.getExtFeatures());
    #endif
    #ifdef DISPLAY
    /* Setup the buttons on the display */
    TFT.begin();
    TFT.fillScreen(ILI9341_BLACK);
    TFT.setTextColor(ILI9341_PINK);
    TFT.setTextSize(2);
    TFT.setFont(DroidSansMono_40);
    delay(2000);
    /** 
        Later these can be made into bitmaps of actual buttons?
    **/
    TFT.fillRect(0,180,BUTTON_WIDTH,BUTTON_HEIGHT,ILI9341_BLUE);
    TFT.fillRect(64,180,BUTTON_WIDTH,BUTTON_HEIGHT,ILI9341_GREEN);
    TFT.fillRect(128,180,BUTTON_WIDTH,BUTTON_HEIGHT,ILI9341_RED);
    TFT.fillRect(192,180,BUTTON_WIDTH,BUTTON_HEIGHT,ILI9341_ORANGE);
    TFT.fillRect(256,180,BUTTON_WIDTH,BUTTON_HEIGHT,ILI9341_DARKCYAN);
    TFT.setCursor(10,4);
    /* Initialise the song display */
    TFT.print(" Title:");
    TFT.setCursor(10,48);
    TFT.print("Artist:");
    TFT.setCursor(10,92);
    TFT.print(" Album:");
    TFT.setCursor(10,136);
    TFT.print("  Time:");
    delay(2000);
    TFT.setCursor(64,4);
    TFT.print(song_title);
    TFT.setCursor(64,48);
    TFT.print(song_artist);
    TFT.setCursor(64,92);
    TFT.print(song_album);
    TFT.setCursor(64,136);
    sprintf(s,"%2d:%2d/%2d:%2d", elapsed_minutes, elapsed_seconds, duration_minutes, duration_seconds);
    TFT.print(timeOut);
    #endif
    digitalWrite(PIN_SHUTDOWN, HIGH); // turn on PA after setup complete
    /* Operational code */
    for(;;){
        #ifdef DEBUG
        delay(50);
        timer = millis();
        #endif
        #ifdef HC05
        /* Check for HC05 commands */
        if(Serial2.available() != 0){
            c = Serial2.read();
            switch(c){
                case ' ':
                    RN52_Serial3.playPause();    //pause
                    paused_flag = paused_flag ? false : true; //toggle paused flag
                    #ifdef DEBUG
                    Serial.println("Paused.");
                    #endif
                    break;
                case '+':
                    RN52_Serial3.volumeUp();    //volUp
                    #ifdef DEBUG
                    Serial.println("Volume increased.");
                    #endif
                    break;
                case '-':
                    RN52_Serial3.volumeDown();    //volDown
                    #ifdef DEBUG
                    Serial.println("Volume decreased.");
                    #endif
                    break;
                case '>':
                    RN52_Serial3.nextTrack();    //skip
                    new_song_flag = true;
                    paused_flag = false;
                    #ifdef DEBUG
                    Serial.println("Song skipped.");
                    #endif
                    break;
                case '<':
                    RN52_Serial3.prevTrack();    //previous
                    new_song_flag = true;
                    paused_flag = false;
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
        GPIO_2_status = digitalRead(GPIO2_PIN);
        if(!GPIO_2_status){
            event_reg_status = RN52_Serial3.queryState();
            #ifdef DEBUG
            Serial.print("event_reg_status ");
            Serial.println(event_reg_status);

            #endif
            if(event_reg_status == 11277){
                new_song_flag = true;
                #ifdef DEBUG
                Serial.print("track event ");
                Serial.println(event_reg_status & (1 << 5));
                #endif
            }
        }

        /** 
            Do some maths here in an attempt to detect if a new song is possible, if it is set the new_song_flag.
            Maybe: start time + elasped > duration.
            On second thoughts, do I need this?
        **/
        /* Now get metadata information */
        /** CHECK THIS CONDITION ESP strcmp **/
        Serial.println(millis()%METADATA_RESET);
        if( (strcmp(timeOut, "00:00/00:00") == 0) || millis()%METADATA_RESET == 0 || new_song_flag){  //runs on startup, every n seconds and on changes 
            #ifdef DEBUG
            Serial.println("...");
            #endif
            previous_album = song_album;  //save the old versions of the text so that we can wipe screen
            song_album = RN52_Serial3.album();
            previous_title = song_title;
            song_title = RN52_Serial3.trackTitle();  
            previous_artist = song_artist;            
            song_artist = RN52_Serial3.artist();
            previous_duration = song_duration;         
            song_duration = RN52_Serial3.trackDuration();

            if(song_title == "" || song_artist == ""){
                new_song_flag = false; 
                continue;  //if info is blank then we have no song playing.
            }
            else if (song_title != previous_title){
                new_song_flag = true;
            } 
        }
        paused_flag = new_song_flag ? false : paused_flag;    //reset paused flag if a new song is detected (default spotify behaviour).
        /* Time update */
        if(new_song_flag){
            start_time = millis();
            #ifdef DEBUG          
            Serial.print("Title: ");
            Serial.println(song_title);
            #endif
        }
        duration_seconds = (song_duration/1000)%60;
        duration_minutes = (song_duration/1000)%3600;
        /* Account for the duration staying constant during a pause. Only for own pauses as AVRCP doesnt give pause data. */
        paused_flag_array |= paused_flag;
        switch(paused_flag_array){
                case 0:  //keep increasing elapsed time
                    elapsed_time = millis() - start_time;
                    break;                
                case 1:  //save the time it was paused at
                    time_at_pause = millis();
                    break;                
                case 2:  //compute new start time
                    start_time = time_at_pause - elapsed_time;  //lol bodged
                    break;                
                case 3:  //still paused
                    break;
                default:
                    shutdown();
                    #ifdef DEBUG
                    Serial.println("Error");
                    #endif
            }
        elapsed_seconds = (elapsed_time/1000)%60;
        elapsed_minutes = (elapsed_time/1000)%3600;
        #ifdef DISPLAY  
        TFT.setTextColor(ILI9341_BLACK);        
        TFT.setCursor(64,136);
        TFT.print(timeOut); //print the old time in black
        TFT.setTextColor(ILI9341_PINK);
        #endif
        previousTimeOut = timeOut;
        sprintf(s,"%2d:%2d/%2d:%2d", elapsed_minutes, elapsed_seconds, duration_minutes, duration_seconds);
        #ifdef DEBUG
        if(timeOut != previousTimeOut){
            Serial.println(timeOut);
        }
        #endif
        #ifdef DISPLAY
        TFT.setCursor(64,136);
        TFT.print(timeOut); //print the new time
        /* Metadata Update */
        if(new_song_flag){
            TFT.setTextColor(ILI9341_BLACK);
            TFT.setCursor(64,4);
            TFT.print(previous_title);
            TFT.setCursor(64,48);
            TFT.print(previous_artist);
            TFT.setCursor(64,92);
            TFT.print(previous_album);            
            TFT.setTextColor(ILI9341_PINK);
            TFT.setCursor(64,4);
            TFT.print(song_title);
            TFT.setCursor(64,48);
            TFT.print(song_artist);
            TFT.setCursor(64,92);
            TFT.print(song_album);
            TFT.setCursor(64,136);
        }
        #endif
        paused_flag_array = paused_flag << 1; /** check this to ensure functionality **///update previously paused flag. 
        new_song_flag = false;    //reset new song flag
        //#ifdef DEBUG
        //Serial.print("Loop time: ");
        //Serial.println(millis() - timer);
        //#endif
    }
    shutdown(); //if this is reached something really bad has happened.    
}

void shutdown(void){ // send exit cmd mode, do all shutdown stuff
    digitalWrite(PIN_SHUTDOWN, LOW); //turn off the PA
    #ifdef DEBUG
    Serial.println("-----------------------");
    Serial.println("Something bad happened.");
    Serial.println("-----------------------");
    #endif
}
