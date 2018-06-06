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
#include <XPT2046_Touchscreen.h>

/* Buttons for the screen */
#include "Pictures.h"

/* Definitions */
#define TFT_DC      21
#define TFT_CS      20
#define TFT_RST     255  // 255 = unused, connect to 3.3V
#define TFT_MOSI    11
#define TFT_SCLK    13
#define TFT_MISO    12
#define TOUCH_CS    22
#define TOUCH_IRQ   23
#define TS_MINX     300
#define TS_MINY     220
#define TS_MAXX     4000
#define TS_MAXY     3900
#define SERIAL_BAUD_RATE    115200
#define RN52_BAUD_RATE      115200
#define BUTTON_HEIGHT       60
#define BUTTON_WIDTH        64
#define METADATA_RESET      100
#define GPIO2_PIN           17
#define PIN_SHUTDOWN        2
#define GPIO9_PIN           15
#define STRING_DISP_LIMIT   17
#define TEXT_COLOUR CL(96,96,96)

/* Globals */
#ifdef DISPLAY
ILI9341_t3 TFT = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);
XPT2046_Touchscreen TouchScreen(TOUCH_CS, TOUCH_IRQ);
#endif

/* Function delarations */
void shutdown(void); //write this function

/** Finish Metadata related AT commands in HWSerial before using **/
int main(void){
    /* Variables */
    #ifdef DEBUG
    int timer = 0;
    #endif
    char c, time_out[12] = "", previous_time_out[12] = "";
    String song_artist = "", song_album = "", song_title = "";
    String previous_album = "", previous_title = "", previous_artist = "";
    int song_duration = 0;
    int start_time = 0, elapsed_time = 0;
    int duration_seconds = 0, duration_minutes = 0;
    int elapsed_seconds = 0, elapsed_minutes = 0;
    uint16_t event_reg_status = 0, paused_event_mask = 0b00000010, new_event_mask = 0b0000100000000000;
    TS_Point touched_location;
    uint8_t touched_flag_array = 0;
    bool new_song_flag = false, GPIO_2_status = true;
    uint8_t paused_flag_array = 0, paused_flag_mask = 3;
    /* Setup code */
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
    TFT.begin();    //returns void
    #ifdef DEBUG
    Serial.println("Started display");
    #endif
    if(!TouchScreen.begin()){
        #ifdef DEBUG
        Serial.println("Failed to start touchscreen.");
        #endif
    }
    delay(1000); 
    TFT.fillScreen(ILI9341_BLACK);
    TFT.setTextColor(TEXT_COLOUR);
    TFT.setTextSize(2);
    TFT.setFont(DroidSansMono_16);
    TFT.setRotation(3);
    TouchScreen.setRotation(1);
    /** 
        Later these can be made into bitmaps of actual buttons?
    **/
    TFT.writeRect(0,180,BUTTON_WIDTH,BUTTON_HEIGHT,(uint16_t*) MinusButton);        //vol down
    TFT.writeRect(64,180,BUTTON_WIDTH,BUTTON_HEIGHT,(uint16_t*) PreviousButton);      //previous
    TFT.writeRect(128,180,BUTTON_WIDTH,BUTTON_HEIGHT,(uint16_t*) PauseButton);       //play/pause
    TFT.writeRect(192,180,BUTTON_WIDTH,BUTTON_HEIGHT,(uint16_t*) NextButton);    //next
    TFT.writeRect(256,180,BUTTON_WIDTH,BUTTON_HEIGHT,(uint16_t*) PlusButton);  //vol up
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
        touched_flag_array |= (bool) TouchScreen.touched();
        if(touched_flag_array & 0b1){
            if(!(touched_flag_array & 0b11111110)){
                touched_location = TouchScreen.getPoint(); 


                #ifdef DEBUG
                Serial.print("X coord =");
                Serial.println(touched_location.x);
                Serial.print("Y coord =");
                Serial.println(touched_location.y);
                #endif
                if(touched_location.x < 960 && touched_location.y > 2900){
                    RN52_Serial3.volumeDown();
                    #ifdef DEBUG
                    Serial.println("Volume decreased.");
                    #endif
                }
                else if(touched_location.x < 1720 && touched_location.y > 2900){
                    RN52_Serial3.prevTrack();    //previous
                    new_song_flag = true;
                    paused_flag_array |= false;;
                    #ifdef DEBUG
                    Serial.println("Song rewound.");
                    #endif
                }
                else if(touched_location.x < 2480 && touched_location.y > 2900){
                    RN52_Serial3.playPause();    //pause
                    paused_flag_array |= ((paused_flag_array >> 1) & 0b1) ? false : true; //toggle paused flag
                    #ifdef DEBUG
                    Serial.println("Paused.");
                    #endif
                }
                else if(touched_location.x < 3240 && touched_location.y > 2900){
                    RN52_Serial3.nextTrack();    //skip
                    new_song_flag = true;
                    paused_flag_array |= false;;
                    #ifdef DEBUG
                    Serial.println("Song skipped.");
                    #endif
                }
                else if(touched_location.x > 3240){
                    RN52_Serial3.volumeUp();    //volUp
                    #ifdef DEBUG
                    Serial.println("Volume increased.");
                    #endif    
                }           
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
                    paused_flag_array |= false;;
                    #ifdef DEBUG
                    Serial.println("Song skipped.");
                    #endif
                    break;
                case '<':
                    RN52_Serial3.prevTrack();    //previous
                    new_song_flag = true;
                    paused_flag_array |= false;;
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
                paused_flag_array |= true;;
                #ifdef DEBUG
                Serial.println("Paused");
                #endif 
            }
            else{
                paused_flag_array |= false;;
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
        if( (strcmp(time_out, "") == 0) || millis()%METADATA_RESET == 0 || new_song_flag){  //runs on startup, every n seconds and on changes 
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
//        paused_flag_array |= paused_flag;
        switch(paused_flag_array){
            case 0:  //keep increasing elapsed time
                elapsed_time = millis() - start_time;
                break;                
            case 1:  //play button
                TFT.writeRect(128,180,BUTTON_WIDTH,BUTTON_HEIGHT,(uint16_t*) PlayButton);
                break;                
            case 2:  //compute new start time
                start_time = millis() - elapsed_time;  //lol bodged
                TFT.writeRect(128,180,BUTTON_WIDTH,BUTTON_HEIGHT,(uint16_t*) PauseButton);       //play/pause
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
        strcpy(previous_time_out,time_out);
        sprintf(time_out,"%02d:%02d/%02d:%02d", elapsed_minutes, elapsed_seconds, duration_minutes, duration_seconds);
        #ifdef DISPLAY
        if(strcmp(previous_time_out,time_out) != 0){ /** modify to only update the changed character **/
            TFT.setTextColor(ILI9341_BLACK);        
            TFT.setCursor(100,136);
            TFT.print(previous_time_out); //print the old time in black
            TFT.setTextColor(TEXT_COLOUR);
            TFT.setCursor(100,136);
            TFT.print(time_out); //print the new time
        }
        /* Metadata Update */
        if(new_song_flag){
            TFT.setTextColor(ILI9341_BLACK);
            TFT.setCursor(100,4);
            TFT.print("?");
            TFT.setCursor(100,4);
            TFT.print(previous_title);
            TFT.setCursor(100,48);
            TFT.print(previous_artist);
            TFT.setCursor(100,92);
            TFT.print("?");
            TFT.setCursor(100,92);
            TFT.print(previous_album);            
            TFT.setTextColor(TEXT_COLOUR);
            TFT.setCursor(100,4);
            TFT.print(song_title);
            TFT.setCursor(100,48);
            TFT.print(song_artist);
            TFT.setCursor(100,92);
            TFT.print(song_album);
            TFT.setCursor(100,136);
        } 
        #endif
        paused_flag_array = (paused_flag_array << 1) & paused_flag_mask;	//update previously paused flag. 
        new_song_flag = false;    //reset flags
        touched_flag_array = (touched_flag_array << 1);
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
