#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x20, 16, 2);
Servo myservo;

//necessary variables
int maxPersonCount=0;
int personCount=0;
int delay_time=200;
float temp;
bool locked = false; // for simulation purpose

//Buzzer pin
const int buzzer = 13;

//Entry ultrasonic sensor Pins
const int entryPinOut = 12;
const int entryPinIn = 10;

//Exit ultrasonic sensor Pins
const int exitPinOut = 11;
const int exitPinIn = 9;

// 7-segment pins for person count
const int Apin = 4;
const int Bpin = 3;
const int Cpin = 2;
const int Dpin = 0;

// 7-segment pins for maximum limit
const int A = 8;
const int B = 7;
const int C = 6;
const int D = 5;

//Push-button pins
const int incPin = A0;
const int decPin = A1;
const int setPin = A2;

//LM35 pin
const int tempPin = A7;

//servo motor pin
const int servoMotor = A3;

//to take readings for the buttons
 bool upButton;
 bool downButton;
 bool setButton;

 bool tempStatus;


void setup() {  

  //Setting push-button pins
  pinMode(incPin, INPUT_PULLUP);
  pinMode(decPin, INPUT_PULLUP);
  pinMode(setPin, INPUT_PULLUP);

  // setting 7-segment pins
    // person count 7-seg
    pinMode(Dpin,OUTPUT);
    pinMode(Cpin,OUTPUT);
    pinMode(Bpin,OUTPUT);
    pinMode(Apin,OUTPUT);
  
    // maximum limit 7-seg
    pinMode(D,OUTPUT);
    pinMode(C,OUTPUT);
    pinMode(B,OUTPUT);
    pinMode(A,OUTPUT);

  //Setting the entry ultrasonic sesnor pins
  pinMode(entryPinOut, OUTPUT);
  pinMode(entryPinIn, INPUT);

  //Setting the exit ultrasonic sesnor pins
  pinMode(exitPinOut, OUTPUT);
  pinMode(exitPinIn, INPUT);

  //Setting LM35 sensor
  pinMode(tempPin,INPUT);
  analogReference(INTERNAL);

  //Setting Buzzer
  pinMode(buzzer,OUTPUT);

  //Setting micro-servo motor
  pinMode(servoMotor,OUTPUT);
  myservo.attach(servoMotor); 

  //Setting LCD display
  lcd.init();                        // Initialize LCD module
  lcd.backlight();                   // Turn backlight ON
  lcd.setCursor(0, 0); 
  lcd.print("   Starting    ");      
  lcd.setCursor(0, 1);
  lcd.print("      ");
  delay (200);  
  lcd.clear();  

  Serial.begin(9600);

}

void loop() {
  //variables to be used
  long entryDuration, entryCm, exitDuration, exitCm;
  int value;
  
  //Any distance under this value (cm) is a confirmed scan
  int sensorLimit = 15;
  
  //Writing the final digit of remaining spaces to the 7-segment display
  segmentNumber(personCount,0);
  segmentNumber(0,maxPersonCount);

  // set max for the first time
   if (maxPersonCount==0){
    Serial.println("Set maximum limit of customers.");
    while(true){
      upButton = digitalRead(incPin);
      downButton = digitalRead(decPin);
      setButton = digitalRead(setPin);
      if (!upButton) {
        maxPersonCount++;
        Serial.println("Max person increased to:");
        Serial.print(maxPersonCount);
        Serial.println();
        segmentNumber(0,maxPersonCount);
        delay(200);
      }
      else if (!downButton & maxPersonCount > 0) {
        maxPersonCount--;
        Serial.println("Max person decreased to:");
        Serial.print(maxPersonCount);
        Serial.println();
        segmentNumber(0,maxPersonCount);
        delay(200);
      }
      else if (!setButton){
        break;
      }
    }
  }

  // Measuring temperature and displaying on LCD screen
  value = analogRead(tempPin);                                                   
  temp=(long) value*1100/(1024*10);
  float fah = temp * 9 / 5 +32;
 // Serial.println(temp);

   // Temp>37 indicates fever --> door locks, buzzer sounds
   if (temp>37){
      lcd.clear();
      locked = true;
      Serial.println("DOOR LOCKED");
   }   
   while (temp>37){
      tone(buzzer,10);
      lcd.setCursor(0, 0);
      lcd.print("TEMP: ");
      lcd.print(fah);
      lcd.println(" " "\xDF" "F");
      lcd.setCursor(1,6);
      lcd.print("DON'T ENTER"); 
      myservo.write(0);  // locks door
      value = analogRead(tempPin);                                                    
      temp=(long) value*1100/(1024*10);
      fah = temp * 9 / 5 +32;
      delay(500);
    }
      
   if (personCount < maxPersonCount && temp<=37){
        noTone(buzzer);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("TEMP: ");
        lcd.print(fah);
        lcd.println(" " "\xDF" "F");
        lcd.setCursor(1, 6);
        lcd.print("YOU MAY ENTER");  
        myservo.write(180);  // lock open
        if (locked){
          Serial.println("DOOR UNLOCKED");
          locked = false;
        }
        delay(800);
   }
  
  //taking readings for the buttons
   upButton = digitalRead(incPin);
   downButton = digitalRead(decPin);

  //If the increase button has been pressed, increase the person count
  if (!upButton) {
    maxPersonCount++;
    segmentNumber(0,maxPersonCount);
    Serial.println("Max person increased to:");
    Serial.print(maxPersonCount);
    Serial.println();
    delay(200);
  }
  //If the decrease button has been pressed, decrease the person count if possible
  else if (!downButton & maxPersonCount > 0) {
    maxPersonCount--;
    segmentNumber(0,maxPersonCount);
    Serial.println("Max person decreased to:");
    Serial.print(maxPersonCount);
    Serial.println();
    delay(200);
  }

  //If current customer count is greater or equal to max allowance, show the red sign and lock door
  if (personCount >= maxPersonCount) {
      Serial.println("Wait for person to leave"); 
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("TEMP: ");
      lcd.print(fah);
      lcd.println(" " "\xDF" "F");
      lcd.setCursor(1,6);
      lcd.print("PLEASE WAIT");
      myservo.write(0); // locks door
      Serial.println("DOOR LOCKED");
      locked = true;
      delay(1000);
  }
  else {
    myservo.write(180); // lock open
    
    //Taking a distance sensor reading
    digitalWrite(entryPinOut, LOW);
    delayMicroseconds(2);
    digitalWrite(entryPinOut, HIGH);
    delayMicroseconds(10);
    digitalWrite(entryPinOut, LOW);
    entryDuration = pulseIn(entryPinIn, HIGH);

    //Converting to cm
    entryCm = entryDuration / 29 / 2;
    Serial.println("Entry Sensor: ");
    Serial.print(entryCm);
    Serial.print(" cm");
    Serial.println();
    delay(100);

    //If the distance is below the sensorLimit and there is space, the green light will turn on
    if (entryCm <= sensorLimit & personCount < maxPersonCount) {
      personCount++; //Noting the entry of a person
      segmentNumber(personCount,0);

      Serial.println("A person entered");
      Serial.println("Current person count:");
      Serial.println(personCount);

      if (personCount>=maxPersonCount){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("TEMP: ");
        lcd.print(fah);
        lcd.println(" " "\xDF" "F");
        lcd.setCursor(1,6);
        lcd.print("PLEASE WAIT");
        myservo.write(0); // locks door
        Serial.println("DOOR LOCKED");
        locked = true;
        delay(500);
      }

      delay(400);
    }
  }

  //If a person can leave, detect for one
  if (personCount > 0) {

    //Taking a distance sensor reading for exiting sensors
    digitalWrite(exitPinOut, LOW);
    delayMicroseconds(2);
    digitalWrite(exitPinOut, HIGH);
    delayMicroseconds(10);
    digitalWrite(exitPinOut, LOW);
    exitDuration = pulseIn(exitPinIn, HIGH);

    //Converting to cm
    exitCm = exitDuration / 29 / 2;
    Serial.println("Exit Sensor: ");
    Serial.print(exitCm);
    Serial.print(" cm");
    Serial.println();
    delay(100);

    if (exitCm <= sensorLimit) {
      personCount--;
      segmentNumber(personCount,0);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("TEMP: ");
        lcd.print(fah);
        lcd.println(" " "\xDF" "F");
        lcd.setCursor(1, 6);
        lcd.print("YOU MAY ENTER");  
        myservo.write(180); // lock open
        Serial.println("A person left the room");
        Serial.println("Current person count:");
        Serial.println(personCount);
        if (locked){
          Serial.println("DOOR UNLOCKED");
          locked = false;
        }
        delay(500);
    }

  }

}

  
  void segmentNumber(int personCount, int maxLimit) {
    int desiredNumber;
    int a,b,c,d;
    if (personCount==0){
      if (maxLimit!=0){
        desiredNumber = maxLimit;
        a = A;
        b = B;
        c = C;
        d = D;
      }else{
        desiredNumber = personCount;
        a = Apin;
        b = Bpin;
        c = Cpin;
        d = Dpin;
      }
    }
    else{
      desiredNumber = personCount;
      a = Apin;
      b = Bpin;
      c = Cpin;
      d = Dpin;
    }
    switch (desiredNumber) {
      case 0:
        digitalWrite(a,LOW);
        digitalWrite(b,LOW);
        digitalWrite(c,LOW);
        digitalWrite(d,LOW);
        break;
      case 1:
        digitalWrite(a,HIGH);
        digitalWrite(b,LOW);
        digitalWrite(c,LOW);
        digitalWrite(d,LOW);
        break;
      case 2:
        digitalWrite(a,LOW);
        digitalWrite(b,HIGH);
        digitalWrite(c,LOW);
        digitalWrite(d,LOW);
        break;
      case 3:
        digitalWrite(a,HIGH);
        digitalWrite(b,HIGH);
        digitalWrite(c,LOW);
        digitalWrite(d,LOW);
        break;
      case 4:
        digitalWrite(a,LOW);
        digitalWrite(b,LOW);
        digitalWrite(c,HIGH);
        digitalWrite(d,LOW);
        break;
      case 5:
        digitalWrite(a,HIGH);
        digitalWrite(b,LOW);
        digitalWrite(c,HIGH);
        digitalWrite(d,LOW);
        break;
      case 6:
        digitalWrite(a,LOW);
        digitalWrite(b,HIGH);
        digitalWrite(c,HIGH);
        digitalWrite(d,LOW);
        break;
      case 7:
        digitalWrite(a,HIGH);
        digitalWrite(b,HIGH);
        digitalWrite(c,HIGH);
        digitalWrite(d,LOW);
        break;
      case 8:
        digitalWrite(a,LOW);
        digitalWrite(Bpin,LOW);
        digitalWrite(c,LOW);
        digitalWrite(d,HIGH);
        break;
      case 9:
        digitalWrite(a,HIGH);
        digitalWrite(b,LOW);
        digitalWrite(c,LOW);
        digitalWrite(d,HIGH);
        break;
    }
  }
