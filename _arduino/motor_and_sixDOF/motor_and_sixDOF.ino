
#include <Wire.h>

#include <CommunicationUtils.h>
#include <DebugUtils.h>
#include <FIMU_ADXL345.h>
#include <FIMU_ITG3200.h>
#include <FreeSixIMU.h>



const int analogInPin = A0;  

int sensorValue = 0;        
int outputValue = 0;  

float inch, w, median ;
float middle, samples;
float reading[255];

const int inchMin = 10;
const int inchMax = 40;

int pwm_a = 3;  //PWM control for motor 1 on digital pin 3
int pwm_b = 11;  //PWM control for motor 2 on digital pin 11
int dir_a = 12;  //dir control for motor 1 is on digital pin 12
int dir_b = 13;  //dir control for motor 2 is on digital pin 13

float q[4]; //hold q values

// Set the FreeIMU object
FreeSixIMU my3IMU = FreeSixIMU();


int proxPin = 2;
int tiltPin = 4; 
int motorOnPin = 5;
int motorControlPin = 6;

float valSmooth = 0;


//---------------------------------------------------------------------------------
void setup() {  
  
   Wire.begin();

 delay(5);
 my3IMU.init();
 delay(5);
  
  Serial.begin(9600); 

  pinMode(analogInPin, INPUT); // A0

  samples=11;

  pinMode(pwm_a, OUTPUT);   // pin 3 - Set control pins to be outputs
  pinMode(pwm_b, OUTPUT);   // pin 11
  pinMode(dir_a, OUTPUT);   // pin 12
  pinMode(dir_b, OUTPUT);   // pin 13

  analogWrite(pwm_a, 100);        // 39% duty cycle
  analogWrite(pwm_b, 100);


  pinMode(proxPin, OUTPUT); 
  pinMode(tiltPin, OUTPUT); 
  pinMode(motorOnPin, OUTPUT); 
  pinMode(motorControlPin, INPUT_PULLUP); 
  
  
  digitalWrite(proxPin, HIGH);
  digitalWrite(tiltPin, HIGH);
  digitalWrite(motorOnPin, HIGH);
  
  
}


void loop() {

  
  if (digitalRead(motorControlPin) == HIGH){
    digitalWrite(motorOnPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  } else {              // wait for a second
    digitalWrite(motorOnPin, LOW);    // turn the LED off by making the voltage LOW
  }
  
  
  my3IMU.getQ(q);
  boolean tilted = false;
 
 //Serial.println(q[0]);
 //Serial.println(q[3]);
 
 float thresh = 0.2;
 if ( q[0] > thresh || q[0] < -thresh || 
      q[3] > thresh || q[3] < -thresh ){
       tilted = true;
       //Serial.println("tilted");
       digitalWrite(tiltPin, HIGH);
      } else {
       tilted = false;
       
       digitalWrite(tiltPin, LOW);
      }
 
  
  
  
  
  
  
  
  //begin -  find maxbotic untrasonic sensor meduim value

 
  long val = analogRead(analogInPin);
  valSmooth = 0.9 * valSmooth + 0.1 * val;
  Serial.println(valSmooth);
  /*
   case 0 is when dist sensor 1 and/or distance sensor 2 readings are less than 13 inches, 
   stop for n seconds  
   get new readings from sensor
   
   case 1 is when dist sensor 1 and/or distance sensor 2 readings are between 13 - 50 inches, 
   and distance sensor 1 readiing is greater than 2 , 
   turn left for y seconds (180 degree), 
   move forward for z seconds
   stop for n seconds 
   get new readings from sensor
   
   case 2 is when dist sensor 1 and/or distance sensor 2 readings are between 13 - 50 inches, 
   and distance sensor 2 readiing is greater than 1 , 
   turn right for y seconds (180 degree), 
   move forward for z seconds,
   stop for n seconds  
   get new readings from sensor
   */


  if (valSmooth < 100.0){
    digitalWrite(proxPin, HIGH);
    //Serial.println(inch);
    //Serial.println("stop");
    stopped();      // stop for 2 seconds
    

  }  else {
    digitalWrite(proxPin, LOW);
    //Serial.println(inch);
    //Serial.println("turn left and moveforward");

    if (!tilted && digitalRead(motorControlPin) == HIGH){
    //turnLeft();
    //delay(100);  // todo smaller steps?
    forw();
    //fadein();  
    delay(100);  // todo smaller steps?
    stopped();
    }
} 
  
  delay (20);


}


void forw() // no pwm defined
{ 
  digitalWrite(dir_a, HIGH); // set HIGH will go forward;
  digitalWrite(dir_b, HIGH);  
}

void back() // no pwm defined
{
  digitalWrite(dir_a, LOW);  // set LOW will go backward;
  digitalWrite(dir_b, LOW);  
}

void forward() //full speed forward
{ 
  digitalWrite(dir_a, HIGH);  //Reverse motor direction 
  digitalWrite(dir_b, HIGH);  //Reverse motor direction
  analogWrite(pwm_a, 255);   
  analogWrite(pwm_b, 255);
}

void backward() //full speed backward
{
  digitalWrite(dir_a, LOW);  //Set motor direction, 1 low, 2 high
  digitalWrite(dir_b, LOW);  //Set motor direction, 3 high, 4 low
  analogWrite(pwm_a, 255);   //set both motors to run at 100% duty cycle (fast)
  analogWrite(pwm_b, 255);
}

void stopped() //stop
{ 
  digitalWrite(dir_a, LOW); //Set motor direction, 1 low, 2 high
  digitalWrite(dir_b, LOW); //Set motor direction, 3 high, 4 low
  analogWrite(pwm_a, 0);    //set both motors to run at 0% duty cycle (fast)
  analogWrite(pwm_b, 0); 
}

void fadein()
{ 
  // fade in from min to max in increments of 5 points:
  for(int fadeValue = 0 ; fadeValue <= 255; fadeValue +=5) 
  { 
    // sets the value (range from 0 to 255):
    analogWrite(pwm_a, fadeValue);   
    analogWrite(pwm_b, fadeValue);    
    // wait for 30 milliseconds to see the dimming effect    
    delay(30);                            
  } 
}

void fadeout()
{ 
  // fade out from max to min in increments of 5 points:
  for(int fadeValue = 255 ; fadeValue >= 0; fadeValue -=5) 
  { 
    // sets the value (range from 0 to 255):  
    analogWrite(pwm_a, fadeValue);
    analogWrite(pwm_b, fadeValue);
    // wait for 30 milliseconds to see the dimming effect    
    delay(30);  
  }
}

void astop()                   //stop motor A
{
  analogWrite(pwm_a, 0);    //set both motors to run at 0% duty cycle
}

void bstop()                   //stop motor B
{ 
  analogWrite(pwm_b, 0);    //set both motors to run at 0% duty cycle 
}

void turnRight(){
  digitalWrite(dir_a, HIGH);  //Reverse motor direction:  high or low
  digitalWrite(dir_b, LOW);  //Reverse motor direction, high or low 
  analogWrite(pwm_a, 255);   //set both motors to run at 100% duty cycle (fast)
  analogWrite(pwm_b, 255);
}

void turnLeft() {
  digitalWrite(dir_a, LOW);  //Reverse motor direction, 1 high, 2 low
  digitalWrite(dir_b, HIGH);  //Reverse motor direction, 3 low, 4 high  
  analogWrite(pwm_a, 255);   //set both motors to run at 100% duty cycle (fast)
  analogWrite(pwm_b, 255);
}

