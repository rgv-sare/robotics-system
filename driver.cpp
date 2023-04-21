#include "driver.h"
s
#include "Arduino.h"

// branching test
// moike skaterboy

#define ENCA 2 // YELLOW
#define ENCB 3 // WHITE
#define PWM 5
#define IN2 6
#define IN1 7
#define SW1 A1

static volatile int posi = 0; // specify posi as volatile: https://www.arduino.cc/reference/en/language/variables/variable-scope-qualifiers/volatile/
static long prevT = 0;
static float eprev = 0;
static float eintegral = 0;
static int target = 0;

void setMotor(int dir, int pwmVal, int pwm, int in1, int in2);
void readEncoder();
void initDriver()
{
  pinMode(ENCA,INPUT);
  pinMode(ENCB,INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCA),readEncoder,RISING);
  
  //pinMode(SW1,INPUT_PULLUP);
  pinMode(PWM,OUTPUT);
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  target = 0;
}

int lastTime = 0;

void pulleyMotor()
{
  // set target position
  //int target = 1200;

  int pot = analogRead(A0);
  int time = millis();
  if(pot < 100 && 500 < (time - lastTime)) 
  {
    target += 96/ 2.647f;
    lastTime = time;
  }
  if(pot > 924 && 500 < (time - lastTime)) 
  {
    target -= 96/ 2.647f;
    lastTime = time;
  }

  // PID constants
  
  float kp = 4;
  float kd = 0.025;
  float ki = 0.05;

  // time difference
  long currT = micros();
  float deltaT = ((float) (currT - prevT))/( 1.0e6 );
  prevT = currT;

  // Read the position
  int pos = 0;  
  noInterrupts(); // disable interrupts temporarily while reading
  pos = posi;
  interrupts(); // turn interrupts back on
  
  // error
  int e = pos - target;

  // derivative
  float dedt = (e-eprev)/(deltaT);

  // integral
  eintegral = eintegral + e*deltaT;

  // control signal
  float u = kp*e + kd*dedt + ki*eintegral;

  // motor power
  float pwr = fabs(u);
  if( pwr > 255 ){
    pwr = 255;
  }

  // motor direction
  int dir = 1;
  if(u<0){
    dir = -1;
  }

  // signal the motor
  setMotor(dir,pwr,PWM,IN1,IN2);


  // store previous error
  eprev = e;

  Serial.print(target);
  Serial.print(" ");
  Serial.print(pos * 2.647f);
  Serial.println();
}


void moveAngle(int angle)
{
  Serial.println("Test moveAngle function");
}
void setMotor(int dir, int pwmVal, int pwm, int in1, int in2){
  analogWrite(pwm,pwmVal);
  if(dir == 1){
    digitalWrite(in1,HIGH);
    digitalWrite(in2,LOW);
  }
  else if(dir == -1){
    digitalWrite(in1,LOW);
    digitalWrite(in2,HIGH);
  }
  else{
    digitalWrite(in1,LOW);
    digitalWrite(in2,LOW);
  }  
}

void readEncoder(){
  int b = digitalRead(ENCB);
  if(b > 0){
    posi++;
  }
  else{
    posi--;
  }
}
