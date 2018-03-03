int debug = 0;

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
  if (debug) {
    //cout << string;
    Serial.print("Please change the debug flag to 0.");
  }
  else {
    current_gps_buffer.concat(string);
  }
}

void printToBuffer(char c) {
  if (debug) {
    //cout << c;
    Serial.print("Please change the debug flag to 0.");
  }
  else {
    current_gps_buffer.concat(c);
  }
}
