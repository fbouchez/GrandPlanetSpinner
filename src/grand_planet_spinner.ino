// this code is written by illusionmanager in the year 2023
// it is used to drive the Planet Spinner using a ESP32-C3-0.42LCD

// set up your network
#define MYSSID "YourNetworkSSID"
#define MYPASSKEY "YourNetworkPassword"

// look for your timezone string in https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
#define TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3"

// set this to 8 if you have Uranus and Neptune in your PlanetSpinner otherwise set it to 6
#define NUM_OF_PLANETS 8

// uncomment the next line CALIBRATE if you want to move the planets to 90 degree increments orientation
//#define CALIBRATE

//#include <Arduino.h>
#include <TMC2209.h>
#include <WiFiClientSecure.h>
#include <ESPAsyncWebSrv.h>
#include <time.h>  

#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
#define SDA_PIN 5
#define SCL_PIN 6


// set by the way the controller is hardwired
#define MICRO_STEPS 8
// degree per step is 18 degrees from the motor itself, reduced by the internal gear
// by a factor 150.948 from the internal gears, and reduced by the two wooden gear 28/20
#define STEPS_PER_DEGREE 11.74

// slack in mechanisme... if it changes direction and rotate this amount of degrees, the disk
// doesn't actually rotate
#define SLACK 1.5

#define DIR_PIN 3
#define STEP_PIN 4
#define SPEED_PIN 7
#define ENABLE_PIN 8
#define SERIAL_BAUD_RATE 115200
#define PLANETSPINNER "       Planet Spinner\n\n       by illusionmanager\n (youtube.com/@illusionmanager)\n\n"

// Instantiate TMC2209
TMC2209 stepper_driver;
// Instantiate Display; use U8G2_R1 to rotate the display 9 degree
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R1, U8X8_PIN_NONE); 
// stores the absolute orientation of the device
float orient = 0;
// only possible values are 1 and 8 ( times slower!)
uint8_t speed = 1;
// if set the PlanetSpinner switches the current date
volatile bool reset = true;
// holds the orientation of each planet;
int planet[8];
char * planetName[]={"MERCURY","VENUS","EARTH","MARS","JUPITER","SATURN","URANUS","NEPTUNE"};
// during calibration, tweek these values until the planets are exactly at 90 degrees intervals.
#if NUM_OF_PLANETS == 8
  int planetOffset[]={-22,10,-14,8,-9,4,-2,1};
#else
  int planetOffset[]={-18,9,-12,9,-5,-1,0,0};
#endif 

bool geocentric = false;
uint8_t target[] = { 1,2,3,4,5,6,7,8};
int centric = 10;
void setCentric() {
  if (geocentric) {
    planetName[2] = "SUN";
    target[2] = 10;
    centric = 399;
  } else {
    planetName[2] = "EARTH";
    target[2] = 3;
    centric = 10;
  }
}
WiFiClientSecure client;
AsyncWebServer server(80);

bool firstTime = true;
const uint8_t bitmaps[] =
{
  0xAA, 0xAA, 0xA1, 0x55, 0x55, 0x55, 0x54, 0x04, 0x02, 0xAA, 0xAA, 0xC0, 0x08, 0xB0, 0x55,
  0x54, 0x10, 0xE3, 0x04, 0x2A, 0xA2, 0xAC, 0xFC, 0x36, 0x15,	0x05, 0x51, 0xFD, 0x00, 0x0A,
  0xAA, 0x15, 0x9E, 0x9E, 0x05, 0x54, 0x40, 0x47, 0x23, 0x0A,	0xA8, 0x00, 0x0F, 0x10, 0x45,
  0x54, 0x00, 0x3F, 0x83, 0x0A, 0x54, 0x17, 0x7F, 0x88, 0x0A,	0xAA, 0x78, 0xFF, 0x86, 0x01,
  0x54, 0xFF, 0xFF, 0xC0, 0x02, 0xA9, 0xFF, 0xFF, 0xC0, 0xD1, 0x5B, 0xFF, 0xFF, 0xE0, 0x20,
  0xA7, 0xFF, 0xFF, 0xF8, 0x51, 0x4F, 0xFF, 0xFF, 0xF0, 0x22,	0xAF, 0xFF, 0xFF, 0xF0, 0xC5,
  0x5F, 0xFF, 0xFF, 0xF0, 0xA2, 0xBF, 0xFF, 0xFF, 0xF1, 0x45,	0x3F, 0xFF, 0xFF, 0xF0, 0x82,
  0xBF, 0xFF, 0xFF, 0xE1, 0x40, 0x3F, 0x8F, 0xDF, 0xE2, 0xA0,	0xBF, 0x07, 0xC1, 0xE5, 0x51,
  0x3C, 0x07, 0xFC, 0xEA, 0xAA, 0x80, 0x3F, 0xFE, 0xE5, 0x55,	0x54, 0x7F, 0xFE, 0xEA, 0xAA,
  0xAA, 0x7F, 0xFE, 0xE5, 0x55, 0x54, 0xFF, 0xFE, 0xEA, 0xAA,	0xA8, 0xFF, 0xFC, 0xE5, 0x55,
  0x55, 0xFF, 0xF2, 0xEA, 0xAA, 0xA9, 0xFF, 0xE6, 0xE5, 0x55,	0x51, 0xFF, 0x9F, 0xEA, 0xAA,
  0xA9, 0xFF, 0x3F, 0xC5, 0x55, 0x55, 0xF8, 0xFF, 0xCA, 0xAA,	0xA5, 0xE3, 0xFF, 0xC5, 0x55,
  0x57, 0x0F, 0xFF, 0xCA, 0xAA, 0xA0, 0x7F, 0xFF, 0xD5, 0x55,	0x07, 0xFF, 0xFF, 0xCA, 0xAA,
  0x1F, 0xFF, 0xFF, 0xC5, 0x55, 0x00, 0xFF, 0xFF, 0xCA, 0xAA,	0x80, 0x0F, 0xFF, 0xC5, 0x55,
  0x40, 0x00, 0xFF, 0xCA, 0xAA, 0x80, 0xAA, 0x7F, 0xE5, 0x55,	0x05, 0x55, 0x3F, 0xEA, 0xAA,
  0x80, 0xAA, 0xBF, 0xE5, 0x55, 0x55, 0x55, 0x3F, 0xEA, 0xAA,	0xAA, 0xAA, 0xBF, 0xE5, 0x55,
  0x55, 0x55, 0x3F, 0xEA, 0xAA, 0xAA, 0xAA, 0xBF, 0xE5, 0x55,	0x55, 0x55, 0x3F, 0xEA, 0xAA,
  0xAA, 0xAA, 0xBF, 0xE5, 0x55, 0x55, 0x55, 0x3F, 0xF2, 0xAA,	0xAA, 0xAA, 0x7F, 0xF5, 0x55,
  0x55, 0x55, 0x7F, 0xF2, 0xAA, 0xAA, 0xAA, 0x7F, 0xF1, 0x55,	0x55, 0x54, 0xFF, 0xC0, 0xAA,
  0xAA, 0xA8, 0x00, 0x0D, 0x55, 0x55, 0x51, 0x80, 0xFC, 0xAA,	0xAA, 0xAB, 0xFF, 0xFE, 0x55,
  0x55, 0x53, 0xFF, 0xFF, 0x2A, 0xAA, 0xA7, 0xFF, 0xFF, 0x95,	0x55, 0x47, 0xFF, 0xFE, 0x8A,
  0xAA, 0x2F, 0xFF, 0xFE, 0xD5, 0x55, 0x4F, 0xFF, 0xFC, 0x8A,	0xAA, 0x2F, 0xFF, 0xF8, 0x15,
  0x40, 0x03, 0xFF, 0xF0, 0x6A, 0xA0, 0x00, 0x01, 0xE4, 0x65,	0x55, 0x55, 0x00, 0x00, 0x02 
};


void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", PLANETSPINNER);
}

int year;
uint8_t month;
uint8_t day;
// used to store current day. when it changes, new data it collected
uint8_t nowday;

void getYearMonthDay(void) {
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    year = timeinfo.tm_year+1900;
    month = timeinfo.tm_mon+1;
    day = timeinfo.tm_mday;
    Serial.printf("Local time is %d-%d-%d %02d:%02d\n",year,month,day,timeinfo.tm_hour,timeinfo.tm_min);
}

float getPlanetOrientation(uint8_t id, int lyear = 1970,uint8_t lmonth=1,uint8_t lday=1) {
  // Nasa id mercury = 1, venus = 2, earth = 3, etc
  client.setInsecure();
  delay(100);
  char *bc = "";
  if (lyear <0) {
    bc ="bc";
  }
  if (client.connect("ssd.jpl.nasa.gov",443)) {
    client.printf("GET https://ssd.jpl.nasa.gov/api/horizons.api?format=text&COMMAND=%%27%d"\
      "%%27&OBJ_DATA=%%27NO%%27&MAKE_EPHEM=%%27YES%%27&EPHEM_TYPE=%%27VECTOR%%27&CENTER=%%27500@%d%%27&TLIST=%%27"\
      "%s%d-%d-%d%%27&QUANTITIES=%%271%%27 HTTP/1.0\n", target[id], centric,bc,abs(lyear),lmonth,lday);
    client.println("Connection: close");
    client.println();
    // read all headers
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") { // Headers received
        break;
      }
    }
    // skip text until $$SOE is found
    while (client.connected() && !client.find("$$SOE"));
    if (!client.available()) {
      Serial.print("no data! for ");
      Serial.println(planetName[id]);
      return 0;
    }
    // wait until the X coordinate is received
    while (client.connected() && !client.find("X ="));
    // there is a bug in parseFloat(). It cannot read floating point numbers with 
    // many digits after the decimal point. I have to work around it
    float X = client.parseInt(); // stops at decimal point
    char c = client.read();
    int fraction = 0; // read 6 digits of the fractional part
    for (int i=0; i < 6; i++) {
      c = client.read();
      fraction = fraction *10 + c - '0';
    }
    client.parseInt(); // ignore the rest
    float exponent = client.parseFloat();
    if (X>0) {
      X = (X+fraction*0.000001) * pow(10,exponent);
    } else {
      X = (X-fraction*0.000001) * pow(10,exponent);
    }
    while (client.connected() && !client.find("Y ="));
    float Y = client.parseInt(); // stops at decimal point
    c = client.read();
    fraction = 0;
    for (int i=0; i < 6; i++) {
      c = client.read();
      fraction = fraction *10 + c - '0';
    }
    client.parseInt(); // ignore the rest
    exponent = client.parseFloat();
    if (Y>0) {
      Y = (Y+fraction*0.000001) * pow(10,exponent);
    } else {
      Y = (Y-fraction*0.000001) * pow(10,exponent);
    }
    while (client.available()) {
      char c = client.read();
    }
    client.stop();
    // convert X,Y coordinates to angle between 0 and 360
    return fmod(atan2(Y,X)/3.141592*180 + 360,360);
  } else {
    Serial.println("Connection failed");
    delay(3000);
  }
  Serial.println("what???");
  return 0;
}

void setSpeed(uint8_t val) {
  if (val == LOW) {
    speed = 8; // 8 times slower
    digitalWrite(SPEED_PIN, HIGH);
  } else {
    speed = 1;
    digitalWrite(SPEED_PIN, LOW);
  }
}
void rotate(float deg) {
  // positive is counterclockwise
  if (deg <0) {
    digitalWrite(DIR_PIN, HIGH);
  } else {
    digitalWrite(DIR_PIN, LOW);
  }
  for (int i = (abs(deg)) * STEPS_PER_DEGREE * MICRO_STEPS * speed ; i>0; i--) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(88); //160 is the recommended value
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(88);
  }

}

void rotateTo(int dir, float newOrient) {
  // rotates from the current orientation (stored in variable orient)
  // to the new orientation, in the given direction (-1 or 1)
   if (dir >0 ) { 
    if (newOrient <= orient) {
      rotate(orient-newOrient);
    } else {
      rotate(360+orient-newOrient);
    }
  } else {
    if (newOrient >= orient) {
      rotate(orient- newOrient);
    } else {
      rotate(orient- newOrient - 360);
    }
  }
  orient = newOrient;
}

void display(char *s1 = NULL, char *s2 = NULL, char *s3 = NULL) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_simple1_tf);
  u8g2.drawStr(2,9,"Planet");	
  u8g2.drawStr(0,19,"Spinner");	
  if (s1 != NULL) u8g2.drawStr(0,36,s1);
  if (s2 != NULL) u8g2.drawStr(0,46,s2);
  if (s3 != NULL) u8g2.drawStr(0,56,s3);
  u8g2.sendBuffer();
}

void setup()
{
  digitalWrite(ENABLE_PIN, HIGH);
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, HIGH);
  pinMode(SPEED_PIN, OUTPUT);
  digitalWrite(SPEED_PIN, LOW);
  Serial.begin(SERIAL_BAUD_RATE);
  delay(100);
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  display("WiFi...");
  Serial.print("Starting WiFi using ");
  Serial.println(MYSSID);
  WiFi.begin(MYSSID, MYPASSKEY);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("-");
    delay(500);
  }
  u8g2.drawStr(0,45,"IP-addr:");
  char str[10];
  // smaller font needed to fit on screen.
  u8g2.setFont(u8g2_font_timR08_tn);
  snprintf(str,9,"%d.%d.",WiFi.localIP()[0],WiFi.localIP()[1]);
  u8g2.drawStr(0,54,str);
  snprintf(str,9,"%d.%d",WiFi.localIP()[2],WiFi.localIP()[3]);
  u8g2.drawStr(0,63,str);
  u8g2.sendBuffer();
  delay(4000);
  Serial.printf("\nConnected, IP address is: ");
  Serial.println(WiFi.localIP());
  
  server.onNotFound(notFound);
  server.on("/PlanetSpinner", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (request->hasParam("reset")) {
          reset = (bool)(request->getParam("reset")->value().toInt());
          getYearMonthDay();
      }
      if (request->hasParam("geocentric")) {
        geocentric = (bool)(request->getParam("geocentric")->value().toInt());
        reset = true;
        setCentric();
        if (geocentric) {
          Serial.println("Using Geocentric model.");
        } else {
          Serial.println("Using Heliocentric model.");
        }
      }
      if (request->hasParam("newdate")) {
          String newdate = (request->getParam("newdate")->value());
          year+=100;           
          char *d = (char *) newdate.c_str();
          int newyear = 0;
          int8_t sign = 1;
          if (*d == '-') {
            sign = -1;
            d++;
          }
          if (isdigit(*d)) {
            while (isdigit(*d)) {
              newyear = newyear *10 + *d-'0';
              d++;
            }
            if (*d ==' ' || *d=='-' || *d =='/') {
              d++; 
            }
              int newmonth = 0;
              if (isdigit(*d) && newyear != 0 && newyear <= 9999) {
                newyear *=sign;
              while (isdigit(*d)) {
                newmonth = newmonth *10 + *d - '0';
                d++;
              }
              if (*d ==' ' || *d=='-' || *d =='/') {
                d++; 
              }
              if (isdigit(*d) && newmonth >0 && newmonth <=12) {
                int newday =0;
                while (isdigit(*d)) {
                  newday = newday*10 + *d -'0';
                  d++;
                }
                if (newday >0 and newday <=31) {
                  year = newyear;
                  month = newmonth;
                  day = newday;
                  reset = true;
                }
              }
            }
          }
          Serial.printf("new date is %d-%d-%d\n",year,month,day);
      }
      char answer[2350];
      char datestring[20];
      snprintf(datestring,20,"%d-%d-%d",year,month,day);
      // magical string that makes a good looking website for the planet spinner
      snprintf(answer,2350,"<head><title>Planet Spinner</title><style>body{font-family: \"Empanada\", Helvetica, Sans-serif; font-size: 40px;\
          text-align: center; background-color: lightblue;}input{color: white; background-color: #7aa8b7; ; font-size: 40px; }\
          .lnk {display: inline-block;padding: 10px 20px;text-align: center;text-decoration: none;color: #ffffff;background-color:\
          #7aa8b7;border-radius: 7px; box-sizing: border-box;width: 200px; height: 60px;transition-duration:0.7s;}\
          .lnk1:hover {background-color: #918B30} .lnk1 {background-color:#7aa8b7} .lnk2 {background-color:#B8978D }</style></head>\n\
          <h1>Planet Spinner</h1><p>Planetarium by illusionmanager<br>(youtube.com/@illusionmanager)</p>\n\
          <h6>Enter a new date to acquire planet positions from NASA<br>(year month day)</h6>\n\
          <form style=\"color: #E2948E\" action=\"/PlanetSpinner\" method=\"get\">Date<br>\n<input style=\"border-radius: 7px;outline: none;\
          box-sizing: border-box; font-size:30px; width:200px;height:60px;text-align: center;\" type=\"text\" name=\"newdate\" value=\"%s\"><br><br></form><br>\n\
          <a class=\"lnk lnk1\" href=\"/PlanetSpinner?reset=1\">Reset</a>\n\
          <br>model to use:<br><a class=\"lnk lnk1\" href=\"/PlanetSpinner?geocentric=%d\">%s</a>\n\
          <script>\nconst myForm = document.querySelector('form');\n\
          const myDateInput = document.querySelector('input[type=\"newdate\"]');myDateInput.addEventListener('keydown', function(event) {\n\
          if (event.key =='Enter') {myChangeHandler(event);}});\n\
          function myChangeHandler(event) {myForm.submit();}\n</script>\
          ",datestring,geocentric?0:1,geocentric?"Geocentric":"Heliocentric");
      request->send(200, "text/html",answer);
  });
  server.begin();
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, HIGH);
  stepper_driver.enable();

  configTzTime(TIMEZONE, "pool.ntp.org", "time.nist.gov");
  getYearMonthDay();
  nowday = day;
  setSpeed(HIGH);
  digitalWrite(ENABLE_PIN, HIGH);
}

void loop() {
  struct tm timeinfo;
  char str[10];
  getLocalTime(&timeinfo);
  if (nowday != timeinfo.tm_mday) {
    getYearMonthDay();
    nowday = day;
    reset = true;
  };   
  if (reset) {
    reset=false;
    setSpeed(HIGH);
    digitalWrite(ENABLE_PIN, LOW);
    display("Reset...");
    if (firstTime) {
      rotate(360);
      display("Program","    by"); rotate(360);
      display("illusion","manager"); rotate(360);
      u8g2.clearBuffer();
      u8g2.drawBitmap(0,0,5,70,bitmaps);
      u8g2.sendBuffer();
      rotate(360);
      display("Design","    by"); rotate(360);
      display("illusion","manager"); rotate(360);
      display(""); rotate((NUM_OF_PLANETS-6)*360);
      firstTime = false;
    } else {
      rotate(NUM_OF_PLANETS*360);
    }
    orient = 0;
#ifdef CALIBRATE
    display("Calibrt");
    planet[0]=90;
#else
    display("NASA...","MERCURY");
    planet[0]=getPlanetOrientation(0,year,month,day);
    snprintf(str,9,"to %d%c",(int)planet[0],176);
    display("spinning",planetName[0],str);
    Serial.printf("spinning %s to ",planetName[0]);
    Serial.println(planet[0]);
#endif
    rotateTo(1,planet[0]-planetOffset[0]);
    for (uint8_t i = 1; i < NUM_OF_PLANETS; i++) {     
#ifdef CALIBRATE
      planet[i]=(90*i+90) % 360;
#else
      display("NASA...",planetName[i]);
      planet[i]=getPlanetOrientation(i,year,month,day);
#endif
      snprintf(str,9,"to %d%c",(int)planet[i],176);
      display("spinning",planetName[i],str);
      Serial.printf("spinning %s to ",planetName[i]);
      Serial.println(planet[i]);
      if (i & 1) {
        //rotate counter clockwise
        rotate(-SLACK);
        rotate((NUM_OF_PLANETS -i)*-360);
        rotateTo(-1,planet[i]-planetOffset[i]);
      } else {
        rotate(SLACK);
        rotate((NUM_OF_PLANETS - i ) * 360);
        rotateTo(1, planet[i] - planetOffset[i]);
      }
      if (reset) break;
    }
    if (!reset) {
      snprintf(str,9,"to 0%c",176);
      display("spinning","Zodiac",str);
      rotate(SLACK);
      rotateTo(1,0);
    }
    digitalWrite(ENABLE_PIN, HIGH);
    
  }
  // show the date the Planet Spinner is set to
  // including the current local time    
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_simple1_tf);
  u8g2.drawStr(4,9,"Planet");	
  u8g2.drawStr(0,19,"Spinner");
  u8g2.setFont(u8g2_font_bitcasual_tu);
  if (year > 0) {
    snprintf(str,9,"%d",year);
    u8g2.drawStr(6,35,str);
  } else {
    snprintf(str,9,"%d",-year);
    u8g2.drawStr(0,35,str);
    u8g2.setFont(u8g2_font_simple1_tf);
    u8g2.drawStr(28,35,"BC");    
  }
  const char *monthName[] = { "JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};
  u8g2.setFont(u8g2_font_simple1_tf);
  u8g2.drawStr(0,50,monthName[month-1]);
  u8g2.setFont(u8g2_font_bitcasual_tu);
  snprintf(str,9,"%2d", day);
  u8g2.drawStr(24,50,str);
  snprintf(str,9," %02d:%02d",timeinfo.tm_hour,timeinfo.tm_min);
  u8g2.drawStr(0,65,str);
  u8g2.sendBuffer();
}
