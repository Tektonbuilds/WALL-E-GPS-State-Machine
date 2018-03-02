#IFNDEF PARSER_H
#DEFINE PARSER_H

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

#ENDIF
