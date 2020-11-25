// ProportionalControl.pde
// -*- mode: C++ -*-
//
// Make a single stepper follow the analog value read from a pot or whatever
// The stepper will move at a constant speed to each newly set posiiton, 
// depending on the value of the pot.
//
// Copyright (C) 2012 Mike McCauley
// $Id: ProportionalControl.pde,v 1.1 2011/01/05 01:51:01 mikem Exp mikem $

#include <AccelStepper.h>

// Define a stepper and the pins it will use
AccelStepper stepper(AccelStepper::DRIVER, 9, 8);
AccelStepper dropper(AccelStepper::HALF4WIRE,4,5,6,7);
#define RAIL_LEFT 3
#define RAIL_RIGHT 2

void setup()
{  
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(10000);
  stepper.setSpeed(1000);
  dropper.setMaxSpeed(500);
  dropper.setAcceleration(1000);
  dropper.setSpeed(100);
  pinMode(RAIL_LEFT,OUTPUT);
  pinMode(RAIL_RIGHT,OUTPUT);
  openDropper();
  closeDropper();
}

void sorter(){
  //-------no-------
  stepper.runToNewPosition(0);
  delay(1000);
  stepper.runToNewPosition(-300);
  delay(500);
  stepper.runToNewPosition(-1000);
  delay(1000);
}

void timedWrite(int pin, int milis){
  digitalWrite(pin,HIGH);
  delay(milis);
  digitalWrite(pin,LOW);

}

void closeDropper(){
  dropper.runToNewPosition(2);
}

void openDropper(){
  dropper.runToNewPosition(1201);
}

void drop(){
  openDropper();
  delay(500);
  closeDropper();
}

void loop()
{
  timedWrite(RAIL_LEFT,1600);
  drop();
  delay(1000);
  timedWrite(RAIL_RIGHT,800);
  drop();
  delay(500);
  timedWrite(RAIL_LEFT,900);
  delay(1000);
  timedWrite(RAIL_RIGHT,1500);
  drop();
  delay(500);

  
  
}
