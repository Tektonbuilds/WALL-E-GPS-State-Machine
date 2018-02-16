#include <SPI.h>
#include <SD.h>
#include <Wire.h>

void printGpsInfo(char string[]);
bool isDecimalCharacter(char c);
bool isComma(char c);
bool isNotComma(char c);
bool isGpsStringValid(char string[]);
void printGpsTimeAndCoords(char string[]);
bool isGpsLocked(char string[]);

int buttonPin = A0;         // the number of the input pin
int gpsLed = 15;       // the led that will flash when the GPS has a signal
int redPin = 2;      // the red LEDs used for synchronizing the two cameras
int camera = 16; 
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


// the follow variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long time = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers

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

void loop()
{
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
        // if GPS acquires signal, move on
        if (buffer_filled) {
//          Serial.println(gps_buffer);
          gpsLock = isGpsLocked(gps_buffer);
        }
        if (gpsLock) {
          state = 2;
          printGpsInfo(gps_buffer);
          memset(&gps_buffer[0], 0, sizeof(gps_buffer));
          buffer_filled = false;
        }
        // if the GPS doesn't get signal, a button press can override, skip to state 3, and allow us to start recording anyway
        reading = analogRead(buttonPin);
        if (reading > 150) {
          state = 3;
        }
        break;
    case 2:
//        Serial.println("Numbah 2!!!!");
        Serial.print("State 2: ");
        Serial.println(current_gps_string);
        // led will blink, signaling that the GPS is ready and user can record
        // waiting for button to be pushed
        reading = analogRead(buttonPin);
        if (reading > 150) {
          digitalWrite(gpsLed, LOW);
          state = 3;
        }
        else {
          unsigned long currentMillis = millis();
          if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;    
            if (gpsLedState == LOW) {
              gpsLedState = HIGH;
            } else {
              gpsLedState = LOW;
            }
            digitalWrite(gpsLed, gpsLedState);
          }
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
        // flash 5 times
        for (int i = 0; i < 5; i ++) {   
            digitalWrite(redPin, HIGH);
            delay(1000);
            digitalWrite(redPin, LOW);
            delay(2000);
        }
        state = 4;
        break;
     case 4: 
        Serial.println("State 4: waiting to turn off camera!");
        reading = analogRead(buttonPin);
        if (reading > 150) {
          digitalWrite(camera, LOW);
          state = 5;
        }
        break;
     case 5:
        Serial.println("Yay we're home!");
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

// Makes sure the character is a dot or a number
bool isDecimalCharacter(char c) {
  int asciiNum = (int) c;
  
  // 0-9
  if (asciiNum >= 48 && asciiNum <= 57) {
    return true;
  }
  
  // .
  if (asciiNum == 46) {
    return true;
  }
  
  return false;
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
    if (!isDecimalCharacter(c)) {
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
    if (!isDecimalCharacter(c)) {
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
  
  if (string[++i] != 'N') {
    // Serial.println("North N not found");
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
    if (!isDecimalCharacter(c)) {
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
  
  if (string[++i] != 'W') {
    // Serial.println("West W not found");
    return false;
  }
  
  return true;
}

// Prints the time and stuff in this format:
//    "Time: {time}, {latitude} N, {longitude} W"
// *** MAKE SURE STRING IS NULL-TERMINATED ***
void printGpsTimeAndCoords(char string[]) {
  int numCommas = 0;
  
  for (int i = 0;; i++) {
    if (string[i] == '\0' ) {
      return;
    }
    
    if ((numCommas == 1 || numCommas == 3 || numCommas == 5) 
      && string[i] != ',') {
        if (numCommas == 1) {
            
        }
        Serial.print(string[i]);
        current_gps_buffer.concat(string[i]);
    }
    
    if (string[i] == ',') {
      numCommas++;
      
      switch (numCommas) {
      case 1:
        Serial.print("Time: ");
        current_gps_buffer = "Time: ";
        break;
      case 3:
        Serial.print(", ");
        current_gps_buffer.concat(", ");
        break;
      case 5:
        Serial.print(" N, ");
        current_gps_buffer.concat(" N, ");
        break;
      case 6:
        Serial.print(" W\n");
        current_gps_buffer.concat(" W\n");
        current_gps_buffer.toCharArray(current_gps_string, 500);
        return;
      }
    }
  }
}

