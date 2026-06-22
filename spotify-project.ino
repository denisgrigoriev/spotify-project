#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <WiFi.h>
#include <SpotifyEsp32.h>
#include <SPI.h>

#define TFT_CS    7
#define TFT_RST   5
#define TFT_DC    3
#define TFT_SCLK  4
#define TFT_MOSI  6

const int previous_switch_pin = 0; 
const int pause_play_switch_pin = 1;
const int skip_switch_pin = 10;

const int vol_up_switch_pin     = 2;  // Volume Up Button
const int vol_down_switch_pin   = 8;  // Volume Down Button

char* SSID = "Redacted";
const char* PASSWORD = "Redacted";
const char* CLIENT_ID = "Redacted";
const char* CLIENT_SECRET = "Redacted";

String lastArtist;
String lastTrackname;

Spotify sp(CLIENT_ID, CLIENT_SECRET);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

void setup() {
    Serial.begin(115200);

    pinMode(previous_switch_pin, INPUT_PULLDOWN);
    pinMode(pause_play_switch_pin, INPUT_PULLDOWN);
    pinMode(skip_switch_pin, INPUT_PULLDOWN);

    pinMode(vol_up_switch_pin, INPUT_PULLDOWN);
    pinMode(vol_down_switch_pin, INPUT_PULLDOWN);

    tft.initR(INITR_BLACKTAB); // the type of screen
    tft.setRotation(1); // this makes the screen landscape! remove this line for portrait
    Serial.println("TFT Initialized!");
    tft.fillScreen(ST77XX_BLACK); // make sure there is nothing in the buffer

    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi...");
    while(WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.printf("\nConnected!\n");

    tft.setCursor(0,0); // make the cursor at the top left
    tft.write(WiFi.localIP().toString().c_str()); // print out IP on the screen

    sp.set_scopes("user-read-playback-state user-modify-playback-state");

    sp.begin();
    while(!sp.is_auth()){
        sp.handle_client();
    }
    Serial.println("Authenticated");
}

void loop()
{

    static int myVolume = 50;

    int previous_switch_pin_state = digitalRead(previous_switch_pin);
    int pause_play_switch_pin_state = digitalRead(pause_play_switch_pin);
    int skip_switch_pin_state = digitalRead(skip_switch_pin);

    String currentArtist = sp.current_artist_names();
    String currentTrackname = sp.current_track_name();

    if(previous_switch_pin_state = HIGH){
        sp.skip_to_previous();
    }

    if(pause_play_switch_pin_state = HIGH){
        if(sp.is_playing() == HIGH){
            sp.pause_playback();
        } else {
            sp.start_a_users_playback();
        }
    }

    if(skip_switch_pin_state = HIGH){
        sp.skip_to_next();
    }

    int vol_up_switch_pin_state = digitalRead(vol_up_switch_pin);
    int vol_down_switch_pin_state = digitalRead(vol_down_switch_pin);

    // Volume Up (Only runs if the active device supports volume modification)
    if(vol_up_switch_pin_state == HIGH){ 
        if(sp.volume_modifyable()) {
            if(myVolume <= 90) myVolume += 10;
            sp.set_volume(myVolume); 
        } else {
            Serial.println("Volume adjustment not supported on this device.");
        }
        delay(300); 
    }

    // Volume Down (Only runs if the active device supports volume modification)
    if(vol_down_switch_pin_state == HIGH){ 
        if(sp.volume_modifyable()) {
            if(myVolume >= 10) myVolume -= 10;
            sp.set_volume(myVolume); 
        } else {
            Serial.println("Volume adjustment not supported on this device.");
        }
        delay(300); 
    }


    if (lastArtist != currentArtist && currentArtist != "Something went wrong" && !currentArtist.isEmpty()) {
        tft.fillScreen(ST77XX_BLACK);
        lastArtist = currentArtist;
        Serial.println("Artist: " + lastArtist);
        tft.setCursor(10,10);
        tft.write(lastArtist.c_str());
    }

    if (lastTrackname != currentTrackname && currentTrackname != "Something went wrong" && currentTrackname != "null") {
        lastTrackname = currentTrackname;
        Serial.println("Track: " + lastTrackname);
        tft.setCursor(10,40);
        tft.write(lastTrackname.c_str());
    }
    delay(2000);
}
