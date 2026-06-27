#include <NewPing.h>    // Ultrasonic sensor library
#include <Wire.h>       // I2C communication library
#include <LiquidCrystal_I2C.h> // Updated for your I2C LCD Module

const int echo_L = 2;   // Left Sensor
const int trig_L = 3;
const int echo_M = 4;   // Middle Sensor
const int trig_M = 5;
const int echo_R = 7;   // Right Sensor
const int trig_R = 8;

const int L1 = 6;       // Motor Driver Pins
const int L2 = 9;
const int R1 = 10;
const int R2 = 11;

const int button = 12;  // Auto/Manual Mode Switch
const int pump = 13;    // Relay for Water Pump

int motor_speed = 255;    // Speed: 125 (min) to 255 (max)
int max_distance = 200;   // Max distance set to 200cm
int distance_L = 0;
int distance_M = 0;
int distance_R = 0;
char incomingByte;

NewPing sonar_L(trig_L, echo_L, max_distance);
NewPing sonar_M(trig_M, echo_M, max_distance);
NewPing sonar_R(trig_R, echo_R, max_distance);

// Initialize the I2C LCD (0x27 is the most common address, occasionally it's 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2); 

void setup()
{  
  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(button, INPUT_PULLUP); // Fixed: Added internal pull-up resistor for stability
  pinMode(pump, OUTPUT);
  
  digitalWrite(L1, LOW); 
  digitalWrite(L2, LOW);
  digitalWrite(R1, LOW);
  digitalWrite(R2, LOW);
  digitalWrite(pump, LOW); // Ensure pump is off at startup
  
  lcd.init();       // Updated I2C startup command
  lcd.backlight();  // Turn on the LCD backlight
  lcd.print("Welcome!");
  
  Serial.begin(9600);   // Bluetooth baud rate
  delay(2000);
}

void loop()
{
  if(digitalRead(button) == HIGH)    // If switch is open (Manual Mode)
  {
    lcd.clear();
    lcd.print("Manual Mode");
    while(true)
    {
      manualMode();     
      if(digitalRead(button) == LOW) // If switch is flipped, break to Auto mode
      {
        moveStop();
        break;
      }
    }
    delay(100);
  }
  else                              // If switch is closed to GND (Auto Mode)
  {
    lcd.clear();
    lcd.print("Automatic Mode");
    while(true)
    {
      automaticMode();
      if(digitalRead(button) == HIGH) // If switch is flipped, break to Manual mode
      {
        moveStop();
        break;
      }
    }
    delay(100);
  }
}

void manualMode()
{
  if (Serial.available() > 0)
  {
    incomingByte = Serial.read();
  }

  switch(incomingByte)
  {
    case 'F':
      moveForward();
      lcd.clear();
      lcd.print("Forward");
      incomingByte='*';
      break;
    
    case 'B':
      moveBackward();
      lcd.clear();
      lcd.print("Backward");
      incomingByte='*';
      break;
    
    case 'L':
      moveLeft();
      lcd.clear();
      lcd.print("Left");
      incomingByte='*';
      break;
    
    case 'R':
      moveRight();
      lcd.clear();
      lcd.print("Right");
      incomingByte='*';
      break;
    
    case 'S':
      moveStop();
      lcd.clear();
      lcd.print("Stop");
      incomingByte='*';
      break;
    
    case 'P':
      digitalWrite(pump, HIGH);
      lcd.clear();
      lcd.print("Pump ON");
      incomingByte='*';
      break;
    
    case 'p':
      digitalWrite(pump, LOW); 
      incomingByte='*';
      break;
    
    case '1':
      motor_speed = 155;
      incomingByte='*';
      break;
    
    case '2':
      motor_speed = 205;
      incomingByte='*';
      break;
    
    case '3':
      motor_speed = 255;
      incomingByte='*';
      break;
  }
}

void automaticMode()
{
  distance_L = readSensor_L();
  distance_M = readSensor_M();
  distance_R = readSensor_R();
  
  lcd.clear();
  lcd.print("L=");
  lcd.print(distance_L);
  lcd.print("cm ");
  lcd.print("M=");
  lcd.print(distance_M);
  lcd.print("cm");
  lcd.setCursor(0, 1);
  lcd.print("R=");
  lcd.print(distance_R);
  lcd.print("cm");
  
  if(distance_M <= 20)
  {
    if(distance_R > distance_L)
    {
      if((distance_R <= 20) && (distance_L <= 20))
      {
        moveStop();
        delay(200);
        moveBackward();
        delay(2000);
      }
      else
      {
        moveBackward();
        delay(500);
        moveRight();
        delay(2000);
      }
    }
    else if(distance_R < distance_L)
    {
      if((distance_R <= 20) && (distance_L <= 20))
      {
        moveStop();
        delay(200);
        moveBackward();
        delay(2000);
      }
      else
      {
        moveBackward();
        delay(500);
        moveLeft();
        delay(2000);
      }
    }
  }
  else if(distance_R <= 15)
  {
    moveLeft();
    delay(500);
  }
  else if(distance_L <= 15)
  {
    moveRight();
    delay(500);
  }
  else
  {
    moveForward();
  }
}

int readSensor_L()
{ 
  delay(70);
  int cm_L = sonar_L.ping_cm();
  if(cm_L==0) cm_L = 250;
  return cm_L;
}

int readSensor_M()
{ 
  delay(70);
  int cm_M = sonar_M.ping_cm();
  if(cm_M==0) cm_M = 250;
  return cm_M;
}

int readSensor_R()
{ 
  delay(70);
  int cm_R = sonar_R.ping_cm();
  if(cm_R==0) cm_R = 250;
  return cm_R;
}

void moveForward()
{
  digitalWrite(L1, LOW); 
  analogWrite(L2, motor_speed);
  analogWrite(R1, motor_speed);
  digitalWrite(R2, LOW);
}

void moveBackward()
{
  analogWrite(L1, motor_speed); 
  digitalWrite(L2, LOW);
  digitalWrite(R1, LOW);
  analogWrite(R2, motor_speed);
}

void moveLeft()
{
  analogWrite(L1, motor_speed); 
  digitalWrite(L2, LOW);
  analogWrite(R1, motor_speed);
  digitalWrite(R2, LOW);
}

void moveRight()
{
  digitalWrite(L1, LOW); 
  analogWrite(L2, motor_speed);
  digitalWrite(R1, LOW);
  analogWrite(R2, motor_speed);
}

void moveStop()
{
  digitalWrite(L1, LOW); 
  digitalWrite(L2, LOW);
  digitalWrite(R1, LOW);
  digitalWrite(R2, LOW);
}