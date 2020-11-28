#include <Arduino_FreeRTOS.h>
#include <AccelStepper.h>
#include <Servo.h>
#include <semphr.h>

#define CART_LEFT_PIN 13
#define CART_RIGHT_PIN 12

#define LOADER_DISABLE_PIN 8
#define LOADER_STEP_PIN 11
#define LOADER_DIRECTION_PIN 10

#define BATTERY_VOLTAGE_PIN A5

#define MEASURE_SERVO_PIN 9
#define MEASURE_SERVO_OPEN_POSITION 66
#define MEASURE_SERVO_CLOSE_POSITION 33

#define VOLTAGE_MEDIUM_TRESHOLD 250
#define VOLTAGE_HIGH_TRESHOLD 290

AccelStepper loader(AccelStepper::DRIVER, LOADER_STEP_PIN, LOADER_DIRECTION_PIN);
AccelStepper dropper(AccelStepper::HALF4WIRE, 2, 3, 4, 5);

Servo measure_servo;

SemaphoreHandle_t xCartMutex;
SemaphoreHandle_t xBatteryMutex;

enum BatteryBoxes {
  empty,
  partial,
  full
}

BatteryState selected_box;

void CartTask( void *pvParameters );
void GaugeTask( void *pvParameters );

void setup()
{
  // Setting up Serial Port
  Serial.begin(9600);

  // Creating Mutexes
  xCartMutex = xSemaphoreCreateBinary();    // Cart is ready to accept new battery
  xBatteryMutex = xSemaphoreCreateBinary(); // Battery is ready to be loaded into a dropper
  if (xCartMutex != NULL && xBatteryMu != NULL) {
    Serial.println("Mutex created");
  }

  // Measure Servo Setup
  measure_servo.attach(MEASURE_SERVO_PIN);
  measure_servo.write(MEASURE_SERVO_OPEN_POSITION);

  // Setup Loader
  loader.setMaxSpeed(1000);
  loader.setAcceleration(10000);
  loader.setSpeed(1000);
  pinMode(LOADER_DISABLE_PIN, OUTPUT);

  // Setup Dropper
  dropper.setMaxSpeed(500);
  dropper.setAcceleration(1000);
  dropper.setSpeed(100);
  
  pinMode(CART_LEFT_PIN, OUTPUT);
  pinMode(CART_RIGHT_PIN, OUTPUT);
  calibrate();
  loader.runToNewPosition(0);
  timedWrite(CART_LEFT_PIN, 1600);
  digitalWrite(LOADER_DISABLE_PIN, HIGH);

  // Running Threads
  xSemaphoreGive(xCartMutex);
  xTaskCreate( GaugeTask, "GaugeTask", 128, NULL, 2, NULL );
  xTaskCreate( CartTask, "CartTask", 128, NULL, 1, NULL );
}

// Load battery to measurement
void fetch_battery() {
  digitalWrite(LOADER_DISABLE_PIN, LOW);
  loader.runToNewPosition(300);
  delay(1000);
  loader.runToNewPosition(0);
  digitalWrite(LOADER_DISABLE_PIN, HIGH);
}

// Sent battery to cart
void eject_battery() {
  digitalWrite(LOADER_DISABLE_PIN, LOW);
  loader.runToNewPosition(-700);
  delay(500);
  loader.runToNewPosition(0);
  digitalWrite(LOADER_DISABLE_PIN, HIGH);
}

BatteryBoxes measure_voltage() {
  BatteryBoxes box;
  min = 999;
  max = 0;
  int voltage = 0;
  
  measure_servo.write(MEASURE_SERVO_CLOSE_POSITION);
  delay(1000);


  for(int i=0; i<7;i++)
  {
    int analog_value = analogRead(BATTERY_VOLTAGE_PIN);
    if(analog_value > max)
      max = analog_value;
    if(analog_value < min)
      min = analog_value;
    voltage += (analog_value);
  }
  
  voltage -= min;
  voltage -= max;
  voltage /= 5;

  if (voltage < VOLTAGE_MEDIUM_TRESHOLD) box = empty;
  else if (voltage > VOLTAGE_HIGH_TRESHOLD) box = full;
  else box = partial;
  
  measure_servo.write(MEASURE_SERVO_OPEN_POSITION);
  delay(500);
  return box;
}

// Move cart 
void timedWrite(int pin, int milis) {
  digitalWrite(pin, HIGH);
  delay(milis);
  digitalWrite(pin, LOW);

}

void calibrate() {
  dropper.runToNewPosition(1202);
  dropper.runToNewPosition(2);

}

void drop() {
  dropper.runToNewPosition(450);
  delay(100);
  dropper.runToNewPosition(0);
}


void select_box(BatteryBoxes box) {
  int dur;
  switch (box)
  case empty:
    drop();
    return;
  case partial:
    dur = 600;
    break;
  case full:
    dur = 1200;
  break;
  timedWrite(CART_RIGHT_PIN, dur);
  drop();
  delay(500);
  timedWrite(CART_LEFT_PIN, dur * 1.2);
}

void test()
{
  int box;
  fetch();
  box = measure_bat();
  eject();
  select_box(box);
}

void loop() {
  // Every action will be performed in Tasks below
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void CartTask( void *pvParameters )
{
  while (1)
  {

  }
}

void GaugeTask( void *pvParameters )
{
  while (1)
  {

  }
}
