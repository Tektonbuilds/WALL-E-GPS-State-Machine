#include <SPI.h>
#include <SD.h>
#include <Wire.h>

int buttonPin = A0;                 // the number of the input pin
int gpsLed = 15;                    // the led that will flash when the GPS has a signal, also flashes 3 times when recording starts and twice when it stops
int redPin = 2;                     // the red LEDs used for synchronizing the two cameras
int camera = 16;
unsigned long time_now = 0;         // for our delay in state 5
unsigned long time_now2 = 0;        // for our delay in state 5
unsigned long previousMillis = 0;   // will store last time LED was updated
unsigned long gps_lock_millis = 0;
int gpsLedState = LOW;              // gpsLedState used to set the LED
const long interval = 1000;         // interval at which to blink (milliseconds)
bool gpsLock;

// GPS stuff
int addr = 66;
int num_bytes = 8;

// GPS buffer status
bool buffer_filled;
String buf;
char gps_buffer[500];

// Variables to store most up to date information
String current_gps_buffer;
char current_gps_string[500];
String gps_no_lock_buffer;
char gps_no_lock_string[500];
double latitude;
double longitude;

int reading;                        // the current reading from the input pin
int previous = LOW;                 // the previous reading from the input pin
int state = 1;                      // the possible 4 states are waiting for GPS lock, waiting for button press, recording, and stopping recording

// Timekeeping
unsigned long timeNow = 0;
unsigned long timeLast = 0;

// Time start Settings:
int startingHour = 12;              // set your starting hour here, not below at int hour. This ensures accurate daily correction of time
int seconds = 0;
int minutes = 52;
int hours = startingHour;
int days = 0;
//Date start Settings:
int month = 0;
int day = 0;
int year = 0;
//Accuracy settings
int dailyErrorFast = 0;             // set the average number of milliseconds your microcontroller's time is fast on a daily basis
int dailyErrorBehind = 0;           // set the average number of milliseconds your microcontroller's time is behind on a daily basis
int correctedToday = 1;             // do not change this variable, one means that the time has already been corrected today for the error in your boards crystal. This is true for the first day because you just set the time when you uploaded the sketch.


// the follow variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long time = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers

void delayMillis(int time) {
  time_now = millis();
  while (millis() < time_now + time) {
    delay(0);
  }
}

// Keeping track of the first delay period
void delayMillis(int pin, int timeInMs, int state) {
  if (millis() > (time_now + timeInMs)) {
    // reset the time_now to reflect the next period
    time_now = millis();
    digitalWrite(pin, state);
  }
}

// For keeping track of the another delay
void delayMillis2(int pin, int timeInMs, int state) {
  if (millis() > (time_now2 + timeInMs)) {
    // reset the time_now to reflect the next period
    time_now2 = millis();
    digitalWrite(pin, state);
  }
}

void keepTime() {
  timeNow = millis()/1000;        // the number of milliseconds that have passed since boot
  seconds = timeNow - timeLast;   //the number of seconds that have passed since the last time 60 seconds was reached.
  if (seconds >= 60) {
    timeLast = timeNow;
    minutes = minutes + 1;
  }
  // If one minute has passed, start counting milliseconds from zero again and add one minute to the clock.
  if (minutes >= 60){
    minutes = 0;
    hours = hours + 1;
  }
  // If one hour has passed, start counting minutes from zero and add one hour to the clock
  if (hours >= 24){
    hours = 0;
    days = days + 1;
    //======================UNTESTED======================
    // TODO: account for change in month?
    day = day + 1;
    //======================UNTESTED======================
  }

  //======================UNTESTED======================
  // account for going over to the next month
  // uncomment when the testing is done to test this too
  // int months[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
  // if (days == 1) {
  //   if ((day + 1) > months[month-1]) {
  //     if (month == 12) {
  //       month = 1;
  //     } else {
  //       month = month + 1;
  //     }
  //     day = 1
  //   } else {
  //     day = day + 1;
  //   }
  // }
  //======================UNTESTED======================

  //if 24 hours have passed , add one day
  if (hours ==(24 - startingHour) && correctedToday == 0){
    delay(dailyErrorFast*1000);
    seconds = seconds + dailyErrorBehind;
    correctedToday = 1;
  }
  Serial.print("The time is:           ");
  Serial.print(days);
  Serial.print(":");
  Serial.print(hours);
  Serial.print(":");
  Serial.print(minutes);
  Serial.print(":");
  Serial.println(seconds);
}


void writeToSD(char buffer[]) {
  // File myFile = SD.open("test.txt", FILE_WRITE);
  // TODO: MAKE SURE THIS FILE EXISTS ON THE SD CARD
//  File myFile = SD.open("GPSRecordInfo.txt", FILE_WRITE);
  File myFile = SD.open("test.txt", FILE_WRITE);

  // If the file opened okay, write to it:
  if (myFile) {
    // Serial.print("Writing to GPSRecordInfo.txt...");
    Serial.print("Writing to test.txt...");
//    myFile.println("testing 1, 2, 3.");
    Serial.println(buffer);
    myFile.println(buffer);
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening GPSRecordInfo.txt");
  }
}

void updateGPSCharArray(){
  int debug = 1;
  if (debug) {
    printLatestGPSInfo();
  }
  String temp = "Started recording on ";
  temp.concat(month);
  temp.concat("/");
  temp.concat(day);
  temp.concat("/");
  temp.concat(year);
  temp.concat(" at ");
  temp.concat(hours);
  temp.concat(":");
  temp.concat(minutes);
  temp.concat(":");
  temp.concat(seconds);
  temp.concat(", at Latitude: ");
  temp.concat(abs(latitude));
  if (latitude < 0) { // North
    temp.concat(",N, ");
  } else { // South
    temp.concat(",S, ");
  }
  temp.concat("at Longitude: ");
  temp.concat(abs(longitude));
  if (latitude < 0) { // East
    temp.concat(",E, ");
  } else { // West
    temp.concat(",W, ");
  }
  temp.toCharArray(current_gps_string, 500);
}

// use this for debugging
void printLatestGPSInfo() {
  Serial.print("Started recording on ");
  Serial.print(month);
  Serial.print("/");
  Serial.print(day);
  Serial.print("/");
  Serial.print(year);
  Serial.print(" at ");
  Serial.print(hours);
  Serial.print(":");
  Serial.print(minutes);
  Serial.print(":");
  Serial.print(seconds);
  Serial.print(", at Latitude: ");
  Serial.print(abs(latitude));
  if (latitude < 0) { // South
    Serial.print(",S, ");
  } else { // North
    Serial.print(",N, ");
  }
  Serial.print("at Longitude: ");
  Serial.print(abs(longitude));
  if (latitude < 0) { // West
    Serial.println(",E, ");
  } else { // East
    Serial.println(",W, ");
  }
}

void setup()
{
  gpsLock = false;
  buffer_filled = false;
  buf = "";
  Wire.begin();
  Serial.begin(9600);
    while (!Serial) {
    ; // Wait for serial port to connect. Needed for Leonardo only
  }


  Serial.print("Initializing SD card...");

  if (!SD.begin(0)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  pinMode(buttonPin, INPUT);
  pinMode(gpsLed, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(camera, OUTPUT);
}

void loop() {
  keepTime();
  Wire.requestFrom(addr, num_bytes);    // Request 6 bytes from slave device #8
  while (Wire.available()) {
    char c = Wire.read();               // Receive a byte
    if (c != (char)0xff) {
      if (c == '\r' || c == '\n') {
        buffer_filled = true;
        buf.toCharArray(gps_buffer, 500);
        buf = "";
        break;
      } else {
        buf.concat(c);
      }
    }
  }
  // keep updating coords to get most recent location
  // TODO: We need keep track of already finding a GPS lock and ignoring this condition
  if (buffer_filled && isGpsLocked(gps_buffer)) {
    latitude = getLatitude(gps_buffer);
    longitude = getLongitude(gps_buffer);
  }
  switch (state) {
    case 1:
        Serial.println("State 1!");
        //state = 2;
        // If GPS acquires signal, move on
        Serial.println(gps_buffer);
        if (buffer_filled) {
          gpsLock = isGpsLocked(gps_buffer);
        }
        if (gpsLock) {
          state = 2;
          printGpsInfo(gps_buffer);
          timeLast = millis()/1000;
          seconds = getSeconds(gps_buffer);
          minutes = getMinutes(gps_buffer);
          hours = getHours(gps_buffer);
          //======================UNTESTED======================
          month   = getMonth(gps_buffer);
          day     = getDay(gps_buffer);
          year = getYear(gps_buffer);
          //======================UNTESTED======================
          memset(&gps_buffer[0], 0, sizeof(gps_buffer));
          buffer_filled = false;

          // Start setting the time now to flash later
          time_now = millis();
        }
        // If the GPS doesn't get signal, a button press can override, skip to state 3, and allow us to start recording anyway
        else if (analogRead(buttonPin) > 150) {
          state = 3;
        }
        break;
    case 2:
        Serial.println("Numbah 2!!!!");
        Serial.println(current_gps_string);
        // led will blink, signaling that the GPS is ready and user can record
        // waiting for button to be pushed
        reading = analogRead(buttonPin);
        if (reading > 150) {
          digitalWrite(gpsLed, LOW);
          state = 3;
        }
        else {
          if (gpsLedState == LOW) {
            gpsLedState = HIGH;
          }
          else {
            gpsLedState = LOW;
          }
          delayMillis(gpsLed, 1000, gpsLedState);
        }
        break;
     case 3:
        Serial.println("made it to case 3!");
        if (gpsLock) {
          updateGPSCharArray();
          writeToSD(current_gps_string);
        }
        else {
           gps_no_lock_buffer = "No GPSLock - button override. GPS Buffer: ";
           gps_no_lock_buffer.concat(gps_buffer);
           gps_no_lock_buffer.concat("\n");
           gps_no_lock_buffer.toCharArray(gps_no_lock_string, 500);
           writeToSD(gps_no_lock_string);
        }
        
        // Write to the SD card with the time and GPS coords
        // Start recording (for now, turn on an LED)
        digitalWrite(camera, HIGH);
        
        // Flash the gps led three times quickly to indicate that recording has started
        for (int i = 0; i < 3; i ++) {
            digitalWrite(gpsLed, HIGH);
            delayMillis(200);
            digitalWrite(gpsLed, LOW);
            delayMillis(200);
        }
        // Flash 5 times
        for (int i = 0; i < 5; i ++) {
            digitalWrite(redPin, HIGH);
            delayMillis(500);
            digitalWrite(redPin, LOW);
            delayMillis(1000);
        }
        state = 4;
        break;
     case 4:
        Serial.println("State 4: waiting to turn off camera!");
        reading = analogRead(buttonPin);
        if (reading > 150) {
          digitalWrite(camera, LOW);
          
          // Flash the gps led to indicate recording has stopped
          for (int i = 0; i < 2; i ++) {
            digitalWrite(gpsLed, HIGH);
            delayMillis(200);
            digitalWrite(gpsLed, LOW);
            delayMillis(200);
          }
          state = 5;
        }
        break;
     case 5:
        Serial.println("Yay we're home! Let's go back to state 1.");
        delayMillis(500);
        state = 1;
        break;
  }
}

