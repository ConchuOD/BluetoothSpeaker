/* Optional code segments - using this to seperately test code segments */

#define DEBUG 1
#define DISPLAY 1
//define HC05 1

/* Requried libraries */
#include <HardwareSerial.h>
#include <ILI9341_t3.h>
#include <font_DroidSansMono.h>
#include <RN52_HardwareSerial.h>
#include <WString.h>
#include <Adafruit_STMPE610.h>

/* Libraries that may be needed */
#include <SPI.h>

/* Definitions */
#define TFT_DC      21
#define TFT_CS      20
#define TFT_RST     255  // 255 = unused, connect to 3.3V
#define TFT_MOSI    11
#define TFT_SCLK    13
#define TFT_MISO    12
#define STMPE_CS    22
#define TS_MINX     150
#define TS_MINY     130
#define TS_MAXX     3800
#define TS_MAXY     4000
#define SERIAL_BAUD_RATE    115200
#define RN52_BAUD_RATE      115200
#define BUTTON_HEIGHT       60
#define BUTTON_WIDTH        64
#define METADATA_RESET      100
#define GPIO2_PIN           17
#define PIN_SHUTDOWN        2
#define GPIO9_PIN           15
#define STRING_DISP_LIMIT   17

/* Globals */
#ifdef DISPLAY
Adafruit_STMPE610 TouchScreen = Adafruit_STMPE610(STMPE_CS);
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
    char c, timeOut[12] = "", previousTimeOut[12] = "";
    String song_artist = "", song_album = "", song_title = "";
    String previous_album = "", previous_title = "", previous_artist = "";
    int song_duration = 0, current_duration = 0, previous_duration = 0;    //song_duration is current a string in RN52_HWSerial
    int start_time = 0, elapsed_time = 0, time_at_pause = 0;
    int duration_seconds = 0, duration_minutes = 0;
    int elapsed_seconds = 0, elapsed_minutes = 0;
    uint16_t event_reg_status = 0, paused_event_mask = 0b00000010, new_event_mask = 0b0000100000000000;
    TS_Point touched_location;
    bool touched_flag = false;
    /** Insert code to deal with flags in metadata handling (reset in particular) **/
    bool new_song_flag = false, paused_flag = false, previous_paused_flag = false, event_bit5_flag = false, GPIO_2_status = true;
    uint8_t paused_flag_array = 0, paused_flag_mask = 3; /** Implement this without bools **/
    /* Setup code */
    //setup();
    sei();  //enable interupts
    pinMode(GPIO9_PIN, OUTPUT);    //pin for command mode
    digitalWrite(GPIO9_PIN, HIGH);
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
    RN52_Serial3.begin(RN52_BAUD_RATE); //enable RN52
    #ifdef DEBUG
    Serial.println("Starting RN52 Serial");
    #endif
    digitalWrite(GPIO9_PIN, LOW);
    while (RN52_Serial3.available() == 0);   //wait for ACK (CMD)
    c = RN52_Serial3.read();
    if (c == 'C'){
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
    if (!TouchScreen.begin()){ 
        #ifdef DEBUG
        Serial.println("Unable to start touchscreen.");
        #endif
    } 
    TFT.fillScreen(ILI9341_BLACK);
    TFT.setTextColor(ILI9341_PINK);
    TFT.setTextSize(2);
    TFT.setFont(DroidSansMono_16);
    TFT.setRotation(3);
    /** 
        Later these can be made into bitmaps of actual buttons?
    **/
    TFT.fillRect(0,180,BUTTON_WIDTH,BUTTON_HEIGHT,ILI9341_BLUE);        //vol down
    TFT.fillRect(64,180,BUTTON_WIDTH,BUTTON_HEIGHT,ILI9341_GREEN);      //previous
    TFT.fillRect(128,180,BUTTON_WIDTH,BUTTON_HEIGHT,ILI9341_RED);       //play/pause
    TFT.fillRect(192,180,BUTTON_WIDTH,BUTTON_HEIGHT,ILI9341_ORANGE);    //next
    TFT.fillRect(256,180,BUTTON_WIDTH,BUTTON_HEIGHT,ILI9341_DARKCYAN);  //vol up
    /* Initialise the song display */
    TFT.setCursor(10,4);
    TFT.print(" Title:");
    TFT.setCursor(10,48);
    TFT.print("Artist:");
    TFT.setCursor(10,92);
    TFT.print(" Album:");
    TFT.setCursor(10,136);
    TFT.print("  Time:");
    TFT.setCursor(100,4);
    #endif
    digitalWrite(PIN_SHUTDOWN, HIGH); // turn on PA after setup complete
    /* Operational code */
    for(;;){
        #ifdef DEBUG
        delay(50);
        timer = millis();
        #endif
        #ifdef DISPLAY
        if (!TouchScreen.bufferEmpty()){   
            touched_flag = true;
            touched_location = TouchScreen.getPoint(); 
            touched_location.x = map(touched_location.x, TS_MINY, TS_MAXY, 0, TFT.height());
            touched_location.y = map(touched_location.y, TS_MINX, TS_MAXX, 0, TFT.width());
            #ifdef DEBUG
            Serial.print("X coord =");
            Serial.println(touched_location.x);
            Serial.print("Y coord =");
            Serial.println(touched_location.y);
            #endif
        }
        if(touched_flag){
            if(touched_location.x < 64 && touched_location.y > 180){
                RN52_Serial3.volumeDown();
                #ifdef DEBUG
                Serial.println("Volume decreased.");
                #endif
            }
            else if(touched_location.x < 128 && touched_location.y > 180){
                RN52_Serial3.prevTrack();    //previous
                new_song_flag = true;
                paused_flag = false;
                #ifdef DEBUG
                Serial.println("Song rewound.");
                #endif
            }
            else if(touched_location.x < 192 && touched_location.y > 180){
                RN52_Serial3.playPause();    //pause
                paused_flag = paused_flag ? false : true; //toggle paused flag
                #ifdef DEBUG
                Serial.println("Paused.");
                #endif
            }
            else if(touched_location.x < 256 && touched_location.y > 180){
                RN52_Serial3.nextTrack();    //skip
                new_song_flag = true;
                paused_flag = false;
                #ifdef DEBUG
                Serial.println("Song skipped.");
                #endif
            }
            else if(touched_location.y > 180){
                RN52_Serial3.volumeUp();    //volUp
                #ifdef DEBUG
                Serial.println("Volume increased.");
                #endif    
            }           
        }
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
                    Serial.println("Song rewound.");
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
            if((event_reg_status & new_event_mask) >> 11){
                new_song_flag = true;
                #ifdef DEBUG
                Serial.print("track event ");
                #endif 
            }
            if((event_reg_status & paused_event_mask) >> 1){
                paused_flag = true;
                #ifdef DEBUG
                Serial.println("Paused");
                #endif 
            }
            else{
                paused_flag = false;
                #ifdef DEBUG
                Serial.println("Un-paused");
                #endif 
            }
        }
        /** 
            Do some maths here in an attempt to detect if a new song is possible, if it is set the new_song_flag.
            Maybe: start time + elasped > duration.
            On second thoughts, do I need this?
        **/
        /* Now get metadata information */
        if( (strcmp(timeOut, "") == 0) || millis()%METADATA_RESET == 0 || new_song_flag){  //runs on startup, every n seconds and on changes 
            RN52_Serial3.getMetaData();
            previous_album = song_album;  //save the old versions of the text so that we can wipe screen
            song_album = RN52_Serial3.album();
            song_album.remove(STRING_DISP_LIMIT);
            previous_title = song_title;
            song_title = RN52_Serial3.trackTitle();  
            song_title.remove(STRING_DISP_LIMIT);
            previous_artist = song_artist;            
            song_artist = RN52_Serial3.artist();  
            song_artist.remove(STRING_DISP_LIMIT);
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
        //if(new_song_flag && paused_flag){
        //    new_song_flag = false;  /** workaround, think no longer required? **/
        //}
        /* Time update */
        if(new_song_flag){
            start_time = millis();
            #ifdef DEBUG          
            Serial.print("Title: ");
            Serial.println(song_title);
            #endif
        }
        duration_seconds = (song_duration/1000)%60;
        duration_minutes = ((song_duration/1000)/60)%60;
        paused_flag_array |= paused_flag;
        #ifdef DEBUG
        if(paused_flag_array){
            Serial.print("paused_flag_array: ");
            Serial.println(paused_flag_array,BIN);
        }
        #endif
        switch(paused_flag_array){
                case 0:  //keep increasing elapsed time
                    elapsed_time = millis() - start_time;
                    break;                
                case 1:  //save the time it was paused at
                   break;                
                case 2:  //compute new start time
                    start_time = millis() - elapsed_time;  //lol bodged
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
        elapsed_minutes = ((elapsed_time/1000)/60)%60;
        strcpy(previousTimeOut,timeOut);
        sprintf(timeOut,"%02d:%02d/%02d:%02d", elapsed_minutes, elapsed_seconds, duration_minutes, duration_seconds);
        #ifdef DISPLAY
        if(strcmp(previousTimeOut,timeOut) != 0){ /** modify to only update the changed character **/
            TFT.setTextColor(ILI9341_BLACK);        
            TFT.setCursor(100,136);
            TFT.print(previousTimeOut); //print the old time in black
            TFT.setTextColor(ILI9341_PINK);
            TFT.setCursor(100,136);
            TFT.print(timeOut); //print the new time
        }
        /* Metadata Update */
        if(new_song_flag){
            TFT.setTextColor(ILI9341_BLACK);
            TFT.setCursor(100,4);
            TFT.print(previous_title);
            TFT.setCursor(100,48);
            TFT.print(previous_artist);
            TFT.setCursor(100,92);
            TFT.print(previous_album);            
            TFT.setTextColor(ILI9341_PINK);
            TFT.setCursor(100,4);
            TFT.print(song_title);
            TFT.setCursor(100,48);
            TFT.print(song_artist);
            TFT.setCursor(100,92);
            TFT.print(song_album);
            TFT.setCursor(100,136);
        } 
        #endif
        paused_flag_array = (paused_flag << 1) & 0b11;	//update previously paused flag. 
        new_song_flag = false;    //reset flags
        touched_flag = false;
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
