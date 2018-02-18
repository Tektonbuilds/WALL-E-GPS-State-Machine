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

int buttonPin = A0;                     // the number of the input pin
int gpsLed = 15;                        // the led that will flash when the GPS has a signal
int redPin = 2;                         // the red LEDs used for synchronizing the two cameras
int camera = 16; 
unsigned long previousMillis = 0;       // will store last time LED was updated
unsigned long currentMillis = 0;
int gpsLedState = LOW;                  // gpsLedState used to set the LED
const long interval = 1000;             // interval at which to blink (milliseconds)
int redPinState = LOW;
bool gpsLock;
// GPS address info for I2C reading
int addr = 66;
int num_bytes = 8;
// GPS buffer status
bool buffer_filled;
String buf;
char gps_buffer[500];
// variables to store most up to date information
String current_gps_buffer;
String current_time_string;
char current_gps_string[500];
unsigned long obtained_gps_utc  = 0;    // The time we obtained from getting the GPS lock
unsigned long gps_lock_millis   = 0;    // The ms amount when we obtained the GPS lock
double north_south_coord        = 0;
double east_west_coord          = 0; 
// Things for gps not getting a lock
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
          currentMillis = millis();
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
    	 currentMillis = 0;
    	 previousMillis = 0;
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
	     currentMillis = millis();
            if (currentMillis - previousMillis >= interval) {
            	 previousMillis = currentMillis;    
            	 if (redPinState == LOW) {
                  redPinState = HIGH;
            	 } else {
                  redPinState = LOW;
            	 }
	        digitalWrite(redPin, redPinState);
	     }
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
  
  char we = string[++i];
  if (we != 'W' && we != 'E') {
    // Serial.println("West W and East E not found");
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

    // To print North, South, East, and West properly
    if (string[i] != ',') {
      switch (numCommas) {
      case 1:
        // store the gps information into separate variables
	//          current_time_string 
        //cout << string[i];
	const char* utc = (const char*) string[i];
	obtained_gps_utc = atol(utc);
	gps_lock_millis = millis();
        current_gps_buffer.concat(string[i]);
        break;
      case 3:
//         cout << string[i];
	north_south_coord = string[i];
        current_gps_buffer.concat(string[i]);
        break;
      case 4:
//         cout << " " << string[i] << ", ";
        current_gps_buffer.concat(" ");
        current_gps_buffer.concat(string[i]);
        current_gps_buffer.concat(", ");
        break;
      case 5:
//         cout << string[i];
	east_west_coord = string[i];
        current_gps_buffer.concat(string[i]);
        break;
      case 6:
//         cout << " " << string[i] << "\n";
        current_gps_buffer.concat(" ");
        current_gps_buffer.concat(string[i]);
        current_gps_buffer.concat("\n");
        //current_gps_buffer = "Time: " + obtained_gps_utc + "," + north_south_coord + "N, " + east_west_coord + "W\n";
        current_gps_buffer.toCharArray(current_gps_string, 500);
        return;
      }
    }
    
    if (string[i] == ',') {
      numCommas++;
      
      switch (numCommas) {
      case 1:
        //cout << "Time: ";
        current_gps_buffer = "Time: ";
        break;
      case 3:
        //cout << ", ";
        current_gps_buffer.concat(", ");
        break;
      }
    }
  }
}

// update the time in unsigned long and string variables
void getLatestUTC(unsigned long current_millis) {
  unsigned long num_seconds_passed = (current_millis - gps_lock_millis)/1000;
  unsigned long current_num_seconds = obtained_gps_utc%100;
  unsigned long current_num_minutes = obtained_gps_utc/100;
  unsigned long current_num_hours   = obtained_gps_utc/10000;
  // calculate total amount of time right now
  unsigned long total_seconds       = current_num_seconds + num_seconds_passed;
  unsigned long total_minutes       = total_seconds/60;
  unsigned long total_hours         = total_minutes/60;
  // calculate new time units
  unsigned long new_num_seconds     = total_seconds % 60;
  unsigned long new_num_minutes     = total_minutes % 60;
  unsigned long new_num_hours       = total_hours % 24;
  obtained_gps_utc = new_num_hours*10000 + new_num_minutes*100 + new_num_seconds;
  char hours[3];
  if (new_num_hours == 0) {
//    hours = "00";
    sprintf(hours,"%s", "00");
  } else if (new_num_hours/10 == 0) {
    sprintf(hours, "0%lu", new_num_hours);
  } else {
    sprintf(hours, "%lu", new_num_hours); 
  }
}