#include <iostream>
#include "parser_dev.cpp"

using namespace std;

int main() {
  cout << "Test 1: " << endl;
  printGpsInfo("$GPRMC,004524.00,A,3424.80832,N,150.48188,W,0.047,,071217ç,,,A*65");

  cout << "Test 2: " << endl;
  printGpsInfo("$GPRMC,004524.00,A,3424.80832,N,150.48188,W,0.047,,071217,,,A*˜ç65");

  cout << "Test 3: " << endl;
  printGpsInfo("$GPRMC,004524.00,A,3424.80832,N,150.48188,W,0.047,,071217,,,A*65");

  cout << "Test 4: " << endl;
  printGpsInfo("$GPRMC,004524.00,A,246,S,991,W,0.047,,071217,,,A*65");

  cout << "Test 5: " << endl;
  printGpsInfo("$GPRMC,004524.00,A,246,S,991,E,0.047,,071217,,,A*65");

  cout << "Test 6: " << endl;
  printGpsInfo("$GPRMC,004524.00,A,246,E,991,E,0.047,,071217,,,A*65");

  cout << "isGpsLocked(input): " << endl;
  char* input = "$GPRMC,004524.00,A,3424.80832,N,150.48188,W,0.047,,071217,,,A*65";

  cout << isGpsLocked(input) << endl;
  cout << "Test 7: getHours(), ans: 00" << endl;
  cout << getHours("$GPRMC,004524.00,A,246,E,991,E,0.047,,071217,,,A*65") << endl;

  cout << "Test 8: getHours(), ans: 56" << endl;
  cout << getHours("$GPRMC,564524.00,A,246,E,991,E,0.047,,071217,,,A*65") << endl;

  cout << "Test 9: getMinutes(), ans: 45" << endl;
  cout << getMinutes("$GPRMC,004524.00,A,246,E,991,E,0.047,,071217,,,A*65") << endl;

  cout << "Test 10: getSeconds(), ans: 24" << endl;
  cout << getSeconds("$GPRMC,564524.00,A,246,E,991,E,0.047,,071217,,,A*65") << endl;

  cout << "Test 11: " << endl;
  printGpsInfo("$GPRMC,004524.00,A,246,S,991,E,0.047,,129420,,,A*65");

  cout.precision(15);

  cout << "Test 12: getLatitude(), ans: 3424.80832" << endl;
  cout << getLatitude("$GPRMC,004524.00,A,3424.80832,N,150.48188,W,0.047,,071217,,,A*˜ç65") << endl;

  cout << "Test 13: getLatitude(), ans: 3424123" << endl;
  cout << getLatitude("$GPRMC,004524.00,A,3424123,N,150.48188,W,0.047,,071217,,,A*˜ç65") << endl;

  cout << "Test 14: getLatitude(), ans: -3424.80832" << endl;
  cout << getLatitude("$GPRMC,004524.00,A,3424.80832,S,150.48188,W,0.047,,071217,,,A*˜ç65") << endl;

  cout << "Test 15: getLatitude(), ans: -3424123" << endl;
  cout << getLatitude("$GPRMC,004524.00,A,3424123,S,150.48188,W,0.047,,071217,,,A*˜ç65") << endl;

  cout << "Test 16: getLongitude(), ans: 150.48188" << endl;
  cout << getLongitude("$GPRMC,004524.00,A,3424.80832,N,150.48188,E,0.047,,071217,,,A*˜ç65") << endl;

  cout << "Test 17: getLongitude(), ans: -150.48188" << endl;
  cout << getLongitude("$GPRMC,004524.00,A,3424123,N,150.48188,W,0.047,,071217,,,A*˜ç65") << endl;

  cout << "Test 18: getLongitude(), ans: -12345" << endl;
  cout << getLongitude("$GPRMC,004524.00,A,3424.80832,N,12345,W,0.047,,071217,,,A*˜ç65") << endl;

  cout << "Test 19: getLongitude(), ans: 12345" << endl;
  cout << getLongitude("$GPRMC,004524.00,A,3424123,S,12345,E,0.047,,071217,,,A*˜ç65") << endl;

  return 0;
}