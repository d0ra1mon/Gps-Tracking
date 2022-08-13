#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <SPI.h>
#include <SD.h>
int RXPin = 2;
int TXPin = 3;
const int chipSelect = 4;

TinyGPS gps; // GPS initialization
SoftwareSerial ss(RXPin, TXPin);

static void smartdelay(unsigned long ms); // delay that also takes into account GPS
bool valid_date = false; // used for naming sd file
String filename = ""; // dummy placeholder, this will be updated later based on timestamp
String prev_date = ""; // ensuring the same data points aren't saved more than once

void setup() {
  Serial.begin(9600);
  if (!SD.begin(chipSelect)) {
    while (1); // do nothing if SD is not started
  }
  ss.begin(9600); // start GPS module
  Serial.println("Wiring is correct, SD and GPS presents");
}

void loop() {
  float flat, flon;
  unsigned long age;
  String date_str;
  gps.f_get_position(&flat, &flon, &age);
  String datastring = "";
  date_str = find_date(gps,prev_date);
  prev_date = date_str;
  datastring+= date_str; datastring+=",";
  datastring+= String(flon,6); datastring+=",";
  datastring+= String(flat,6); datastring+=",";
  datastring+= String(gps.f_altitude(),6); datastring+=",";
  datastring+= String(gps.f_course(),2); datastring+=",";
  datastring+= String(gps.f_speed_kmph(),5);

  if (date_str == "NaN"){
  } 
  else {
    if (valid_date == false){
      valid_date = true;
      filename = filenamer(gps);
      Serial.println(filename);
      File dataFile = SD.open(filename, FILE_WRITE);
      if (dataFile){
        dataFile.println("Date [mm/dd/yyyy HH:MM:SS],Longitude [deg],"
        "Latitude [deg],Altitude [m],Heading [deg],Speed [kmph]"); // alter based on data
        dataFile.close();
      } 
      else {
        Serial.println("Issue with saving header");
      }
    } 
    else {
      // open file, write to it, then close it again
      File dataFile = SD.open(filename, FILE_WRITE);
      if (dataFile) {
        Serial.print("data string: ");
        Serial.println(datastring);
        dataFile.println(datastring);
        dataFile.close();
      }
    }
  }
  smartdelay(3000);
}

static void smartdelay(unsigned long ms) {
  unsigned long start = millis();
  do{
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

String find_date(TinyGPS &gps,String prev_date) {
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  String date_str = "";
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    date_str = "NaN";
  else{
    int byte_size = 19;
    char chars[byte_size];
    sprintf(chars, "%02d/%02d/%02d %02d:%02d:%02d",
          month,day, year, hour, minute, second);
    for (int i = 0; i <= byte_size-1; i++){
      date_str+= String(chars[ii]);
    }
  }
  if (date_str == prev_date){
    return "NaN";
  }
  return date_str;
}

String filenamer(TinyGPS &gps) {
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  String filename = "";
  int byte_size = 8;
  char chars[byte_size];
  sprintf(chars, "%02d%02d%02d%02d",
        day, hour, minute, second);
  for (int i = 0; i <= byte_size-1; i++){
    filename+=String(chars[i]);
  }
  return filename+".csv";
}
