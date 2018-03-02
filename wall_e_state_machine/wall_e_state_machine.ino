#include <SPI.h>
#include <SD.h>
#include <Wire.h>

void printGpsInfo(char string[]);
bool isNumber(char c);
bool isDecimalPoint(char c);
bool isComma(char c);
bool isNotComma(char c);
bool isGpsStringValid(char string[]);
void printGpsTimeAndCoords(char string[]);
bool isGpsLocked(char string[]);
int getHours(char string[]);
int getMinutes(char string[]);
int getSeconds(char string[]);
double getLatitude(char string[]);
double getLongitude(char string[]);
void printToBuffer(char string[]);
void printToBuffer(char c);

int buttonPin = A0;         // the number of the input pin
int gpsLed = 15;       // the led that will flash when the GPS has a signal, also flashes 3 times when recording starts and twice when it stops
int redPin = 2;      // the red LEDs used for synchronizing the two cameras
int camera = 16;
unsigned long time_now = 0; // for our delay in state 5
unsigned long time_now2 = 0; // for our delay in state 5
unsigned long previousMillis = 0;        // will store last time LED was updated
unsigned long gps_lock_millis = 0;
int gpsLedState = LOW;             // gpsLedState used to set the LED
const long interval = 1000;           // interval at which to blink (milliseconds)
bool gpsLock;
// GPS stuff
int addr = 66;
int num_bytes = 8;
// GPS buffer status
bool buffer_filled;
String buf;
char gps_buffer[500];
// variables to store most up to date information
String current_gps_buffer;
char current_gps_string[500];
String gps_no_lock_buffer;
char gps_no_lock_string[500];

int reading;           // the current reading from the input pin
int previous = LOW;    // the previous reading from the input pin
int state = 1;         // the possible 4 states are waiting for GPS lock, waiting for button press, recording, and stopping recording

// the following variables are for timekeeping
unsigned long timeNow = 0;
unsigned long timeLast = 0;
//Time start Settings:
int startingHour = 12; // set your starting hour here, not below at int hour. This ensures accurate daily correction of time
int seconds = 0;
int minutes = 52;
int hours = startingHour;
int days = 0;
//Accuracy settings
int dailyErrorFast = 0; // set the average number of milliseconds your microcontroller's time is fast on a daily basis
int dailyErrorBehind = 0; // set the average number of milliseconds your microcontroller's time is behind on a daily basis
int correctedToday = 1; // do not change this variable, one means that the time has already been corrected today for the error in your boards crystal. This is true for the first day because you just set the time when you uploaded the sketch.


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

// keeping track of the first delay period
void delayMillis(int pin, int timeInMs, int state) {
  if (millis() > (time_now + timeInMs)) {
    // reset the time_now to reflect the next period
    time_now = millis();
    digitalWrite(pin, state);
  }
}

// for keeping track of the another delay
void delayMillis2(int pin, int timeInMs, int state) {
  if (millis() > (time_now2 + timeInMs)) {
    // reset the time_now to reflect the next period
    time_now2 = millis();
    digitalWrite(pin, state);
  }
}

void keepTime() {
  timeNow = millis()/1000; // the number of milliseconds that have passed since boot
  seconds = timeNow - timeLast;//the number of seconds that have passed since the last time 60 seconds was reached.
  if (seconds >= 60) {
    timeLast = timeNow;
    minutes = minutes + 1;
  }
  //if one minute has passed, start counting milliseconds from zero again and add one minute to the clock.
  if (minutes >= 60){
    minutes = 0;
    hours = hours + 1;
  }
  // if one hour has passed, start counting minutes from zero and add one hour to the clock
  if (hours >= 24){
    hours = 0;
    days = days + 1;
  }
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


void writeToSD(char timestamp[]) {
  File myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
//    myFile.println("testing 1, 2, 3.");
  myFile.println(timestamp);
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  /*
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  */
}

void setup()
{
  gpsLock = false;
  buffer_filled = false;
  buf = "";
  Wire.begin();
  Serial.begin(9600);
    while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
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
  Wire.requestFrom(addr, num_bytes);    // request 6 bytes from slave device #8
  while (Wire.available()) {
    char c = Wire.read(); // receive a byte
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

  switch (state) {
    case 1:
        Serial.println("State 1!");
        //state = 2;
        // if GPS acquires signal, move on
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
          memset(&gps_buffer[0], 0, sizeof(gps_buffer));
          buffer_filled = false;

          // start setting the time now to flash later
          time_now = millis();
        }
        // if the GPS doesn't get signal, a button press can override, skip to state 3, and allow us to start recording anyway
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
          // delayMillis(1000);
          // if 1000 ms has passed, change LED state
          // update time_now to current ms.

          // #testedbitch
          /*
          if (millis() > (time_now + 1000)) {
            time_now = millis();
            if (gpsLedState == LOW) {
              gpsLedState = HIGH;
            } else {
              gpsLedState = LOW;
            }
          }
          digitalWrite(gpsLed, gpsLedState);
          */
          if (gpsLedState == LOW) {
            gpsLedState = HIGH;
          } else {
            gpsLedState = LOW;
          }
          delayMillis(gpsLed, 1000, gpsLedState);

        }
        break;
     case 3:
        Serial.println("made it to case 3!");
//        String time = "TIME!";
        if (gpsLock) {
            //
            writeToSD(current_gps_string);
        }
        else {
           gps_no_lock_buffer = "GPS did not acquire a lock, and button override was used.\n";
           gps_no_lock_buffer.toCharArray(gps_no_lock_string, 500);
           writeToSD(gps_no_lock_string);
        }
        // write to the SD card with the time and GPS coords
        // start recording (for now, turn on an LED)
        digitalWrite(camera, HIGH);
        // flash the gps led three times quickly to indicate that recording has started
        for (int i = 0; i < 3; i ++) {
            digitalWrite(gpsLed, HIGH);
            delayMillis(200);
            digitalWrite(gpsLed, LOW);
            delayMillis(200);
        }
        // flash 5 times
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
          // flash the gps led to indicate recording has stopped
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

//writeToSD(String time) {
//  myFile = SD.open("test.txt", FILE_WRITE);
//
//  // if the file opened okay, write to it:
//  if (myFile) {
//    Serial.print("Writing to test.txt...");
//    myFile.println("testing 1, 2, 3.");
//    // close the file:
//    myFile.close();
//    Serial.println("done.");
//  } else {
//    // if the file didn't open, print an error:
//    Serial.println("error opening test.txt");
//  }
//
//  // re-open the file for reading:
//  myFile = SD.open("test.txt");
//  if (myFile) {
//    Serial.println("test.txt:");
//
//    // read from the file until there's nothing else in it:
//    while (myFile.available()) {
//      Serial.write(myFile.read());
//    }
//    // close the file:
//    myFile.close();
//  } else {
//    // if the file didn't open, print an error:
//    Serial.println("error opening test.txt");
//  }
//}

// Returns false if the input is not valid or if the input is not GPS-locked
bool isGpsLocked(char string[]) {
  int i = 0;
  if (isGpsStringValid(string)) {
    for (++i; string[i] != ','; i++);
    for (++i; string[i] != ','; i++);
    return string[++i] == 'A';
  }
  return false;
}

void printGpsInfo(char string[]) {
  // Serial.println("before gpsstringvalid");
  if (isGpsStringValid(string)) {
    // Serial.println("after gpsstringvalid");
    printGpsTimeAndCoords(string);
  }
  else {
    // Serial.println("Something wrong with string");
  }
}

// Makes sure the character is a number
bool isNumber(char c) {
  int asciiNum = (int) c;

  // 0-9
  return asciiNum >= 48 && asciiNum <= 57;
}

// Makes sure the character is a dot
bool isDecimalPoint(char c) {
  int asciiNum = (int) c;

  // .
  return asciiNum == 46;
}

bool isComma(char c) {
  return c == ',';
}

bool isNotComma(char c) {
  return !isComma(c);
}

// Takes a null-terminated string and returns true or false based on whether
// or not the string contains weird characters
bool isGpsStringValid(char string[]) {
  int i;
  char* gprmc = "$GPRMC";

  for (i = 0;; i++) {
    if (string[i] == '\0') {
      // Serial.println("String has end");
      break;
    }
  }

  if (i <= 10) {
    // Serial.println("i less than 10");
    return false;
  }

  for (i = 0; ; i++) {
    if (gprmc[i] != '\0') {
      if (string[i] != gprmc[i]) {
        // Serial.println("Does not contain $GPRMC");
        return false;
      }
    }
    else {
      break;
    }
  }
  
  if (isNotComma(string[i])) {
    // Serial.println("first comma failed");
    return false;
  }
  
  // Get to the next comma (Contains the time string)
  int dotCount = 0;
  for (++i; string[i] != ','; i++) {
    char c = string[i];
    if (!(isNumber(c) || isDecimalPoint(c))) {
      // Serial.println("time string failed");
      return false;
    }
    if (c == '.') {
      dotCount++;
      if (dotCount == 2) {
  // Serial.println("time string: More than 2 dots counted");
        return false;
      }
    }
  }

  if (isNotComma(string[i])) {
    // Serial.println("second comma failed");
    return false;
  }

  // If it's not GPS-locked just return false
  if (string[++i] != 'A') {
    // Serial.println("not GPS-locked!");
    return false;
  }

  if (isNotComma(string[++i])) {
    // Serial.println("third comma failed");
    return false;
  }

  // Get to the next comma (North coords)
  dotCount = 0;
  for (++i; string[i] != ','; i++) {
    char c = string[i];
    if (!(isNumber(c) || isDecimalPoint(c))) {
      // Serial.println("north coords string failed");
      return false;
    }
    if (c == '.') {
      dotCount++;
      if (dotCount == 2) {
  // Serial.println("north coords: More than 2 dots counted");
  return false;
      }
    }
  }
  
  char ns = string[++i];
  if (ns != 'N' && ns != 'S') {
    // Serial.println("North N and South S not found");
    return false;
  }
  
  if (isNotComma(string[++i])) {
    // Serial.println("fourth comma failed");
    return false;
  }
  
  // Get to the next comma (West coords)
  dotCount = 0;
  for (++i; string[i] != ','; i++) {
    char c = string[i];
    if (!(isNumber(c) || isDecimalPoint(c))) {
      // Serial.println("west coords string failed: " + c + " (" + ((int) c) + ")");
      return false;
    }
    if (c == '.') {
      dotCount++;
      if (dotCount == 2) {
  // Serial.println("west coords: More than 2 dots counted");
  return false;
      }
    }
  }
  
  char we = string[++i];
  if (we != 'W' && we != 'E') {
    // Serial.println("West W and East E not found");
    return false;
  }

  // Get to 9th comma

  // 7th comma
  for (++i; string[i] != ','; i++);

  // 8th comma
  for (++i; string[i] != ','; i++);

  // 9th comma
  for (++i; string[i] != ','; i++);

  // Make sure date is all numbers
  int endIdxExclusive = i + 7;
  for (++i; i < endIdxExclusive; i++) {
    char c = string[i];
    if (!isNumber(c)) {
      //cout << "Datestamp verification failed." << endl;
      return false;
    }
  }
  // Make sure next char is a comma
  if (isNotComma(string[i])) {
    //cout << "Char after datestamp wasn't a comma." << endl;
    return false;
  }

  return true;
}

// Prints the time and stuff in this format:
//    "Time: {time}, {latitude} N, {longitude} W"
// *** MAKE SURE STRING IS NULL-TERMINATED ***
void printGpsTimeAndCoords(char string[]) {
  int numCommas = 0;
  current_gps_buffer = "";

  for (int i = 0;; i++) {
    if (string[i] == '\0') {
      printToBuffer("\n");
      return;
    }

    // To print North, South, East, and West properly
    if (string[i] != ',') {
      switch (numCommas) {
      case 1:
        printToBuffer(string[i]);
        break;
      case 3:
        printToBuffer(string[i]);
        break;
      case 4:
        printToBuffer(" ");
        printToBuffer(string[i]);
        printToBuffer(", ");
        break;
      case 5:
        printToBuffer(string[i]);
        break;
      case 6:
        printToBuffer(" ");
        printToBuffer(string[i]);
        break;
      }
    }
    
    if (string[i] == ',') {
      numCommas++;
      
      switch (numCommas) {
      case 1:
        printToBuffer("Time: ");
        break;
      case 3:
        printToBuffer(", ");
        break;
      case 9:
        printToBuffer(", Datestamp (DD/MM/YY): ");
        printToBuffer(string[++i]);
        printToBuffer(string[++i]);
        printToBuffer("/");
        printToBuffer(string[++i]);
        printToBuffer(string[++i]);
        printToBuffer("/");
        printToBuffer(string[++i]);
        printToBuffer(string[++i]);
        break;
      }
    }
  }
}

// Gets the index of section number, which is the char
// right after the i-th comma
int getIdxOfSectionNumber(char string[], int sectionNum) {
  int numCommas = 0;
  int i;
  for (i = 0;; i++) {
    if (string[i] == ',') {
      numCommas++;
    }

    if (numCommas == sectionNum) {
      return i + 1;
    }
  }
}

// Gets the index of a dot in a section. Returns -1 if not found.
int getIdxOfPeriodInSection(char string[], int sectionNum) {
  int i = getIdxOfSectionNumber(string, sectionNum);

  for (;; i++) {
    if (string[i] == '.') {
      return i;
    }
    else if (string[i] == ',') {
      return -1;
    }
  }
  return -1;
}

// x to the power of y, x^y
int power(int x, int y) {
  int retval = 1;

  for (int i = 0; i < y; i++) {
    retval = retval * x;
  }

  return retval;
}

// Calculate number according to # of sigfigs. Input has to be sanitized already
int calcIntFromWithinString(char string[], int sigfigs, int startIdx) {
  int retval = 0;
  int currentSigfigs = sigfigs;

  for (int i = startIdx; i < startIdx + sigfigs; i++) {
    retval += (string[i] - '0') * power(10, currentSigfigs - 1);
    currentSigfigs--;
  }
  return retval;
}

double getNumberFromSection(char string[], int sectionNum) {
  int sectionStartIdx = getIdxOfSectionNumber(string, sectionNum);
  int decimalPointIdx = getIdxOfPeriodInSection(string, sectionNum);
  int idxOfNextSection = getIdxOfSectionNumber(string, sectionNum + 1);

  // If no decimal point
  if (decimalPointIdx == -1) {
    int sigfigs = (idxOfNextSection - 2) - (sectionStartIdx) + 1;
    
    return (double) calcIntFromWithinString(string, sigfigs, sectionStartIdx);
  }
  // If decimal point exists
  else {
    double retval = 0;
    retval += calcIntFromWithinString(string, decimalPointIdx - sectionStartIdx, sectionStartIdx);
    
    double multiplicant = 0.1;
    double decimalPart = 0;
    for (int i = decimalPointIdx + 1; i < idxOfNextSection - 1; i++) {
      decimalPart += (string[i] - '0') * multiplicant;
      multiplicant /= 10;
    }

    return retval + decimalPart;
  }
}

int getHours(char string[]) {
  int timestampIdx = getIdxOfSectionNumber(string, 1);

  int tens = string[timestampIdx] - '0';
  int ones = string[timestampIdx + 1] - '0';
  return tens * 10 + ones;
}

int getMinutes(char string[]) {
  int timestampIdx = getIdxOfSectionNumber(string, 1);

  timestampIdx += 2;
  int tens = string[timestampIdx] - '0';
  int ones = string[timestampIdx + 1] - '0';
  return tens * 10 + ones;
}

int getSeconds(char string[]) {
  int timestampIdx = getIdxOfSectionNumber(string, 1);

  timestampIdx += 4;
  int tens = string[timestampIdx] - '0';
  int ones = string[timestampIdx + 1] - '0';
  return tens * 10 + ones;
}

// Latitude is NS (N - positive, S - negative)
double getLatitude(char string[]) {
  double latitude = getNumberFromSection(string, 3);
  
  if (string[getIdxOfSectionNumber(string, 4)] == 'S') {
    latitude *= -1;
  }
  return latitude;
}

// Longitude is WE (E - positive, W - negative)
double getLongitude(char string[]) {
  double longitude = getNumberFromSection(string, 5);
  
  if (string[getIdxOfSectionNumber(string, 6)] == 'W') {
    longitude *= -1;
  }
  return longitude;
}

void printToBuffer(char string[]) {
  int debug = 1;
  if (debug) {
    cout << string;
  }
  else {
    //current_gps_buffer.concat(string);
  }
}

void printToBuffer(char c) {
  int debug = 1;
  if (debug) {
    cout << c;
  }
  else {
    //current_gps_buffer.concat(c);
  }
}
