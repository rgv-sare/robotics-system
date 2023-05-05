

void initDriver();

void setMotorAngle(int angle);

void moveAngle(int angle);

void pulleyMotor();

class Motor{
  private:
  int in1;
  int in2;
  int pwm;
  int chA;
  int chB;
  int steps=0;
  int speed;
  bool cw;
  
  
  Motor(int in1, int in2, int pwm, int chA, int chB){
    this->in1=in1;
    this->in2=in2;
    this->pwm=pwm;
    this->chA=chA;
    this->chB=chB;
  }
  void onInterrupt()
  {
    bool isTurningCW = digitalRead(this->chB);
    if (isTurningCW==true)
    {
     this->steps++; 
    }
    else
    {
     this->steps--; 
    }
  }
  void setSpeed(int spd)
  {
    this->speed=spd;
  }
  
  void setDirection(bool cw)
  {
    this->cw=cw;
  }
  void update()
  {
   if(cw==true)
   {
     digitalWrite(in1, LOW);
     digitalWrite(in2, HIGH);
   }
    else
    {
     digitalWrite(in1, HIGH);
     digitalWrite(in2, LOW);
    }
    analogWrite(pwm, speed);
  }
  
};

Motor motor1(27,29,2,21,48);/*in1=27, in2=29, pdw=2, chA=21, chB=48 */
Motor motor2(25,23,3,20,46);/*in1=25, in2=23, pdw=3, chA=20, chB=46 */
Motor motor3(28,26,5,19,44);/*in1=28, in2=26, pdw=5, chA=19, chB=44 */

void up()
{
  
}
void down()
{
  
}
void right()
{
  
}
void left()
{
  
}
