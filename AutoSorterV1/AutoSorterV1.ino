#define RAIL_LEFT 13
#define RAIL_RIGHT 12
#define STEPPER_DISABLE 8
#define STEPPER_STEP 11
#define STEPPER_DIR 10
#define BAT_PIN A5
#include <AccelStepper.h>
#include <Servo.h>

AccelStepper stepper(AccelStepper::DRIVER, STEPPER_STEP, STEPPER_DIR);
AccelStepper dropper(AccelStepper::HALF4WIRE,2,3,4,5);
Servo myservo;


void setup()
{  
  Serial.begin(9600);
  myservo.attach(9); 
  myservo.write(66);
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(10000);
  stepper.setSpeed(1000);
  dropper.setMaxSpeed(500);
  dropper.setAcceleration(1000);
  dropper.setSpeed(100);
  pinMode(RAIL_LEFT,OUTPUT);
  pinMode(RAIL_RIGHT,OUTPUT);
  pinMode(STEPPER_DISABLE,OUTPUT);
  calibrate();
  stepper.runToNewPosition(0);
  timedWrite(RAIL_LEFT,1600);
  digitalWrite(STEPPER_DISABLE,HIGH);
}

void fetch(){
  digitalWrite(STEPPER_DISABLE,LOW);
  stepper.runToNewPosition(300);
  delay(1000);
  stepper.runToNewPosition(0);
  digitalWrite(STEPPER_DISABLE,HIGH);
}
void eject(){
  digitalWrite(STEPPER_DISABLE,LOW);
  stepper.runToNewPosition(-700);
  delay(500);
  stepper.runToNewPosition(0);
  digitalWrite(STEPPER_DISABLE,HIGH);
}

int measure_bat(){
  myservo.write(33);
  delay(1000);
  int status=analogRead(BAT_PIN);
  Serial.println(status);
  int box;
  if(status<250)box=0;
  else if(status>290)box=2;
  else box= 1;
  //delay(2000);
  myservo.write(66);
  delay(500);
  return box;
}

void timedWrite(int pin, int milis){
  digitalWrite(pin,HIGH);
  delay(milis);
  digitalWrite(pin,LOW);

}

void calibrate(){
  dropper.runToNewPosition(1202);
  dropper.runToNewPosition(2);
  
}

void drop(){
  dropper.runToNewPosition(450);
  delay(100);
  dropper.runToNewPosition(0);
}


void select_box(int box){
  int dur;
  if (box==0){
    drop();
    return;
  }
  if(box==1)dur=600;
  if(box==2)dur=1200;
  timedWrite(RAIL_RIGHT,dur);
  drop();
  delay(500);
  timedWrite(RAIL_LEFT,dur*1.2);
}

void test()
{
  int box;
  fetch();
  box=measure_bat();
  eject();
  select_box(box);
}

void loop(){
  test();
}
