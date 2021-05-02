
// libraries
#include <IRremote.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <dht_nonblocking.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// IR remote
const int RECV_PIN = 12;
unsigned long key_value = 0;
IRrecv irrecv(RECV_PIN);
decode_results results;

// neoPixels

int pinNeo = 11;
int numPixel = 150;
Adafruit_NeoPixel strip(numPixel, pinNeo, NEO_GRB + NEO_KHZ800);

// timers variables and lights variables
unsigned long startTime;
long interval = 300;
unsigned long startTime1;
unsigned long endTime1;

long interval1;
int index;

long interval2;
int index2;

long interval3;
int index3;

long interval4;
int index4;

boolean isIncreasing = true;
boolean isIncreasing2 = true;

// DHT temperature

#define DHT_SENSOR_TYPE DHT_TYPE_11
static const int DHT_SENSOR_PIN = 10;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );
int tempC;
double tempK;
int tempReading;
float temperature;
float humidity;

// Real Time Clock setup
RTC_DS1307 rtc; //DS1307 i2c
byte clocksymbol[8] = {0b00000, 0b00100, 0b01110, 0b01110,
                       0b11111, 0b00000, 0b00100, 0b00000,
                      };
byte temp[8] = {  0b10000, 0b00111, 0b01000, 0b01000, 0b01000, 0b01000,
                  0b00111,  0b00000,
               };
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// buzzer pin
const int buzzer = 9;

// LCD screen variables
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
const int isBacklight = 8;

//button variables

const int adjBt = A1;
const int setBt = A2;
const int alarmBt = A3;
int btnCount = 0;

const int shakeSensor = A3;

int melody[] = { 600, 800, 1000, 1200 };

//Variables
int DD;     // days
int MM;     // month
int YY;     // year
int H;      // hour
int M;      // minutes
int S;      // seconds

int setState;
int adjState;
int alarmState;

int AH;
int AM;

int shakeState;
int shakeTimes = 0;

int i = 0;
String sDD;
String sMM;
String sYY;
String sH;
String sM;
String sS;
String aH = "12";
String aM = "00";

// alarms variables
String alarm = "     ";
long previousMillis = 0;

//Boolean flags
boolean setupScreen = false;
boolean alarmON = false;
boolean turnItOn = false;
boolean backlightON = true;
// colors
uint32_t col1 = strip.Color(0, 255, 255);
uint32_t col2 = strip.Color(255, 25, 255);
uint32_t col3 = strip.Color(255, 255, 50);

uint32_t c1gamma = strip.gamma32(col1);
uint32_t c2gamma = strip.gamma32(col2);
uint32_t c3gamma = strip.gamma32(col3);
uint32_t red = strip.gamma32(strip.Color(255, 0, 0));

int brightness = 255;
int c = 1;
int hue = 0;
int hue1;
int hue2;
int hue3;
boolean isOn1 = false;
boolean isOn2 = false;
boolean isOn3 = false;
int switchVal = 0;

void setup() {

  Serial.begin(9600);
  strip.begin();

  //Init RTC and LCD library items
  rtc.begin();
  lcd.begin(16, 2);

  //Set outputs/inputs
  pinMode(setBt, INPUT);
  pinMode(adjBt, INPUT);
  pinMode(alarmBt, INPUT);
  pinMode(buzzer, OUTPUT);

  // lcd setup
  lcd.createChar(1, clocksymbol);
  lcd.createChar(2, temp);

  digitalWrite(isBacklight, HIGH);
  digitalWrite(buzzer, LOW);

  if (! rtc.isrunning()) {  // check valid time
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(1998, 06, 05, 00, 00, 0));     // June 5, 1998 at 00:00am you would call: my birth date
  }
  delay(100);
  //Read alarm  ftimerom EEPROM memmory
  AH = EEPROM.read(0);
  AM = EEPROM.read(1);
  // (Hours:0-23 and Minutes: 0-59)
  if (AH > 23) {
    AH = 0;
  }
  if (AM > 59) {
    AM = 0;
  }

  irrecv.enableIRIn();
  irrecv.blink13(true);

#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif

  strip.setBrightness(50); // initialize brightness

  startTime = millis();
  btnCount = 0;
  interval = 100;
  index = 0;
  interval2 = 100;
  index2 = 0;
  interval3 = 10;
  index3 = 0;
  interval4 = 50;
  index4 = 0;
}

void loop() {




  if (irrecv.decode(&results)) {

    if (results.value == 0XFFFFFFFF) {
      results.value = key_value;
      Serial.println(key_value);
    }

    key_value = results.value;
    irrecv.resume();
  }

  // finite state machine
  switch (results.value) {
    case 0xFFA25D:
      Serial.println("CH-");
      break;
    case 0xFF629D:
      Serial.println("CH");
      break;
    case 0xFFE21D:
      Serial.println("CH+");
      break;
    case 0xFF22DD:
      Serial.println("|<<");
      break;
    case 0xFF02FD:
      Serial.println(">>|");
      break ;
    case 0xFFC23D:
      Serial.println(">|");
      break ;
    case 0xFFE01F:
      Serial.println("-");
      break ;
    case 0xFFA857:
      Serial.println("+");
      break ;
    case 0xFF906F:
      Serial.println("EQ");
      break ;
    case 0xFF6897:
      Serial.println("0");
      strip.clear();
      strip.show();
      break ;
    case 0xFF9867:
      Serial.println("100+");
      break ;
    case 0xFFB04F:
      Serial.println("200+");
      break ;
    case 0xFF30CF:
      Serial.println("1");
      for (int i = 0; i < numPixel; i++) {
        strip.setPixelColor(i, strip.Color(0, 150, 0));
        strip.show();
      }
      break ;
    case 0xFF18E7:
      Serial.println("2");
      for (int i = 0; i < numPixel; i++) {
        strip.setPixelColor(i, strip.Color(50, 100, 150));
        strip.show();
      }
      break ;
    case 0xFF7A85:
      Serial.println("3");
      for (int i = 0; i < numPixel; i++) {
        strip.setPixelColor(i, strip.Color(150, 0, 0));
        strip.show();
      }
      break ;
    case 0xFF10EF:
      Serial.println("4");
      colorWipe(strip.Color(0, 255, 255), 10);
      break ;
    case 0xFF38C7:
      Serial.println("5");
      colorWipe(strip.Color(255, 0, 255), 10);
      break ;
    case 0xFF5AA5:
      Serial.println("6");
      colorWipe(strip.Color(255, 0, 255), 10);
      colorWipe(strip.Color(0, 255, 255), 10);
      colorWipe(strip.Color(0, 0, 255), 10);
      colorWipe(strip.Color(255, 0, 255), 10);

      break ;
    case 0xFF42BD:
      Serial.println("7");
      colorWipe(strip.Color(255, 0, 0), 10);
      colorWipe(strip.Color(255, 100, 0 ), 10);
      break ;
    case 0xFF4AB5:
      Serial.println("8");
      colorSweep (c1gamma, c2gamma, c3gamma);
      break ;
    case 0xFF52AD:
      Serial.println("9");
      hue += 50;
      strip.fill(strip.ColorHSV(hue, 255, 20));
      strip.show();
      break ;
  }

  readBtns();       //Read buttons
  getTimeDate();    //Read time and date from RTC
  if (!setupScreen) {
    lcdPrint();     //Normanlly print the current time/date/alarm to the LCD
    if (alarmON) {
      callAlarm();   // and check the alarm if set on
    }
    else {
      lcd.setCursor(10, 0);
      lcd.write(" ");
    }
  }
  else {
    timeSetup();    //If button set is pressed then call the time setup function

  }
  if ( measure_environment( &temperature, &humidity ) == true )
  {
    tempC = temperature;
  }


}



static bool measure_environment( float *temperature, float *humidity )
{
  static unsigned long measurement_timestamp = millis( );
  // every 4 seconds
  if ( millis( ) - measurement_timestamp > 3000ul )
  {
    if ( dht_sensor.measure( temperature, humidity ) == true )
    {
      measurement_timestamp = millis( );
      return ( true );
    }
  }
  return ( false );
}


/*************** Functions ****************/
//Read buttons state
void readBtns() {
  setState = digitalRead(setBt);
  adjState = digitalRead(adjBt);
  alarmState = digitalRead(alarmBt);

  if (!setupScreen) {
    if (alarmState == LOW) {
      if (alarmON) {
        alarm = "     ";
        alarmON = false;
      }
      else {
        alarmON = true;
      }
      delay(500);
    }

    if (adjState == LOW) {
      if (backlightON == true) {
        backlightON = false;
        digitalWrite(isBacklight, LOW);
      }
      else {
        backlightON = true;
        digitalWrite(isBacklight, HIGH);
      }
      delay(500);
    }


  }
  if (setState == LOW) {
    if (btnCount < 7) {
      btnCount++;
      setupScreen = true;
      if (btnCount == 1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("-Set-Robs-Clock-");
        lcd.setCursor(0, 1);
        lcd.print("-Time and Date-");
        delay(2000);
        lcd.clear();
      }
    }
    else {
      lcd.clear();
      rtc.adjust(DateTime(YY, MM, DD, H, M, 0)); //Save time and date to RTC IC
      EEPROM.write(0, AH);  //Save the alarm hours to EEPROM 0
      EEPROM.write(1, AM);  //Save the alarm minuted to EEPROM 1
      lcd.print("Saving Changes..");
      delay(1000);
      lcd.clear(); //clear screen
      setupScreen = false;  // recheck new changes and write them on LCD
      btnCount = 0; //reset count to zero
    }
    delay(500);
  }
}

//Read time and date from rtc ic
void getTimeDate() {
  if (!setupScreen) {
    DateTime now = rtc.now();
    DD = now.day();
    MM = now.month();
    YY = now.year();
    H = now.hour();
    M = now.minute();
    S = now.second();
  }
  // setting up scree display
  if (DD < 10) {
    sDD = '0' + String(DD); // for all the days before the 10th day
  } else {
    sDD = DD;
  }
  if (MM < 10) {
    sMM = '0' + String(MM);
  } else {
    sMM = MM;
  }
  sYY = YY - 2000;
  if (H < 10) {
    sH = '0' + String(H);
  } else {
    sH = H;
  }
  if (M < 10) {
    sM = '0' + String(M);
  } else {
    sM = M;
  }
  if (S < 10) {
    sS = '0' + String(S);
  } else {
    sS = S;
  }
  if (AH < 10) {
    aH = '0' + String(AH);
  } else {
    aH = AH;
  }
  if (AM < 10) {
    aM = '0' + String(AM);
  }  else {
    aM = AM;
  }
}
//Print values to the display
void lcdPrint() {
  String line1 = sH + ":" + sM + ":" + sS + " |"; // Time line
  lcd.setCursor(0, 0); //First row
  lcd.print(line1);
  if (alarmON) {
    lcd.setCursor(10, 0);
    lcd.write(1);
  }
  String line2 = aH + ":" + aM;
  lcd.setCursor(11, 0);
  lcd.print(line2);

  String line3 = sMM + "-" + sDD + "-" + sYY + " | " + tempC ; // date+temperature line
  lcd.setCursor(0, 1); //Second row
  lcd.print(line3);
  if (setupScreen) {
    lcd.setCursor(13, 1);
    lcd.print("");
  }
  else {
    lcd.setCursor(13, 1);
    lcd.write(2);
  }

}

//Setup screen
void timeSetup() {
  int upState = adjState;
  int downState = alarmState;

  if (btnCount <= 5) {
    if (btnCount == 1) {      //Set Hour
      lcd.setCursor(4, 0);
      lcd.print(">");
      if (upState == LOW) {  //Up button +
        if (H < 23) {
          H++;
        }
        else {
          H = 0;
        }
        delay(350);
      }
      if (downState == LOW) { //Down button -
        if (H > 0) {
          H--;
        }
        else {
          H = 23;
        }
        delay(350);
      }
    }
    else if (btnCount == 2) {   //Set  Minutes
      lcd.setCursor(4, 0);
      lcd.print(" ");
      lcd.setCursor(9, 0);
      lcd.print(">");
      if (upState == LOW) {
        if (M < 59) {
          M++;
        }
        else {
          M = 0;
        }
        delay(350);
      }
      if (downState == LOW) {
        if (M > 0) {
          M--;
        }
        else {
          M = 59;
        }
        delay(350);
      }
    }
    else if (btnCount == 3) {   //Set Month
      lcd.setCursor(9, 0);
      lcd.print(" ");
      lcd.setCursor(0, 1);
      lcd.print(">");
      if (upState == LOW) {
        if (MM < 12) {
          MM++;
        }
        else {
          MM = 1;
        }
        delay(350);
      }
      if (downState == LOW) {
        if (MM > 1) {
          MM--;
        }
        else {
          MM = 12;
        }
        delay(350);
      }
    }
    else if (btnCount == 4) {   //Set Day
      lcd.setCursor(0, 1);
      lcd.print(" ");
      lcd.setCursor(5, 1);
      lcd.print(">");
      if (upState == LOW) {
        if (DD < 31) {
          DD++;
        }
        else {
          DD = 1;
        }
        delay(350);
      }
      if (downState == LOW) {
        if (DD > 1) {
          DD--;
        }
        else {
          DD = 31;
        }
        delay(350);
      }
    }
    else if (btnCount == 5) {   //Set Year
      lcd.setCursor(5, 1);
      lcd.print(" ");
      lcd.setCursor(10, 1);
      lcd.print(">");
      if (upState == LOW) {
        if (YY < 2999) {
          YY++;
        }
        else {
          YY = 2000;
        }
        delay(350);
      }
      if (downState == LOW) {
        if (YY > 2018) {
          YY--;
        }
        else {
          YY = 2999;
        }
        delay(350);
      }
    }
    lcd.setCursor(5, 0);
    lcd.print(sH);
    lcd.setCursor(8, 0);
    lcd.print(":");
    lcd.setCursor(10, 0);
    lcd.print(sM);
    lcd.setCursor(1, 1);
    lcd.print(sMM);
    lcd.setCursor(4, 1);
    lcd.print("-");
    lcd.setCursor(6, 1);
    lcd.print(sDD);
    lcd.setCursor(9, 1);
    lcd.print("-");
    lcd.setCursor(11, 1);
    lcd.print(sYY);
  }
  else {
    setAlarmTime();
  }
}

//Set alarm time
void setAlarmTime() {
  int upState = adjState;
  int downState = alarmState;
  String line2;
  lcd.setCursor(0, 0);
  lcd.print("SET ROB's ALARM");
  if (btnCount == 6) {          //Set alarm Hour
    if (upState == LOW) {
      if (AH < 23) {
        AH++;
      }
      else {
        AH = 0;
      }
      delay(350);
    }
    if (downState == LOW) {
      if (AH > 0) {
        AH--;
      }
      else {
        AH = 23;
      }
      delay(350);
    }
    line2 = "    >" + aH + " : " + aM + "    ";
  }
  else if (btnCount == 7) { //Set alarm Minutes
    if (upState == LOW) {
      if (AM < 59) {
        AM++;
      }
      else {
        AM = 0;
      }
      delay(350);
    }
    if (downState == LOW) {
      if (AM > 0) {
        AM--;
      }
      else {
        AM = 59;
      }
      delay(350);
    }
    line2 = "     " + aH + " :>" + aM + "    ";
  }
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void callAlarm() {
  if (aM == sM && aH == sH && S >= 0 && S <= 10) {
    turnItOn = true;
  }
  else if (alarmState == LOW || (S >= 59)) { //||(M==(AM+1))
    turnItOn = false;
    alarmON = true;
    delay(50);
  }

  if (turnItOn) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis > interval) {
      previousMillis = currentMillis;
      // tone(buzzer, melody[i], 100);
      i++;
      if (i > 3) {
        i = 0;
      }
      b();
      Serial.println("00000");
    }
  }
  else {

    digitalWrite(buzzer, LOW);

  }
}
void b() {

  for (int i = 0; i < 150; i++) {
    digitalWrite(buzzer, LOW);
    delay(1);//wait for 1ms
    digitalWrite(buzzer, HIGH);
    delay(1);//wait for 1ms
  }
  for (int g; g < strip.numPixels(); g++) {
    strip.setPixelColor(i, red);
    strip.show();
  }
  
}

void colorSweep( uint32_t color1, uint32_t color2, uint32_t color3) {    // case 1

  if (index4 >= strip.numPixels()) {
    index4 = 0;
    strip.show();
  }

  if (endTime1 - startTime1 >= interval4 ) {

    c++;
    startTime1 = millis();
  }
  if (c % 3 == 0) {

    uint32_t c1 = color1;
    uint32_t c2 = color2;
    uint32_t c3 = color3;


    if ( index4 % 3 == 0) {
      strip.setPixelColor(index4, c1);
    }
    if ( index4 % 3 == 1) {
      strip.setPixelColor(index4, c2);
    }
    if ( index4 % 3 == 2) {
      strip.setPixelColor(index4, c3);
    }
    strip.show();
  }

  // }
  else if (c % 3 == 1) {

    uint32_t c1 = color3;
    uint32_t c2 = color1;
    uint32_t c3 = color2;

    if ( index4 % 3 == 0) {
      strip.setPixelColor(index4, c1);
    }
    if ( index4 % 3 == 1) {
      strip.setPixelColor(index4, c2);
    }
    if ( index4 % 3 == 2) {
      strip.setPixelColor(index4, c3);
    }
    strip.show();
  } else if (c % 3 == 2) {
    uint32_t c1 = color2;
    uint32_t c2 = color3;
    uint32_t c3 = color1;

    if ( index4 % 3 == 0) {
      strip.setPixelColor(index4, c1);
    }
    if ( index4 % 3 == 1) {
      strip.setPixelColor(index4, c2);
    }
    if ( index4 % 3 == 2) {
      strip.setPixelColor(index4, c3);
    }
    strip.show();

  }
  index4++;
}
void bright() {

  index++;
  hue++;
  //strip.setPixelColor(hue, c1gamma);
  strip.ColorHSV(hue, 1, 1);

  if (index >= strip.numPixels()) {
    index = 0;

    strip.show();
  }


  if (endTime1 - startTime1 >= interval1 ) {

    if (isIncreasing) {
      strip.setBrightness(brightness);

      brightness = brightness + 15;
      startTime1 = millis();

      if (brightness >= 255 ) {
        isIncreasing = false;
      }
    }
    if (!isIncreasing) {
      strip.setBrightness(brightness);

      brightness = brightness - 15;
      startTime1 = millis();

      if (brightness <= 0) {
        isIncreasing = true;
      }
    }
  }
}
void travelOne() {   // one light jumping around stage 2
  if (endTime1 - startTime1 >= interval2 && index2 <= strip.numPixels() ) {
    strip.setPixelColor(index2 + 50, c3gamma);
    strip.setPixelColor(index2 + 4, c2gamma);

    strip.setPixelColor(index2, c3gamma);
    index2++;

    startTime1 = millis();
    strip.show();
  } else if (index2 >= strip.numPixels() ) {
    index2 = 0;
  }
  // strip.clear();

}
void jumpOne() {   // travels back and worth  stage 3
  if (endTime1 - startTime1 >= interval3 ) {
    if (isIncreasing2) {
      strip.setPixelColor(index3, c2gamma);
      strip.setPixelColor(index3 + 2, c2gamma);
      index3++;
      strip.show();
      startTime1 = millis();
      // strip.clear();

      if (index3 >= strip.numPixels() ) {
        isIncreasing2 = false;
        index3--;
      }
    }
    if (!isIncreasing2) {
      strip.setPixelColor(index3, c3gamma);
      index3--;
      strip.show();
      startTime1 = millis();
      // strip.clear();

      if (index3 <= 0) {
        isIncreasing2 = true;
      }
    }
  }
}

void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}
