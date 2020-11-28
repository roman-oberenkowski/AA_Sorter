#include <Arduino_FreeRTOS.h>
#include <AccelStepper.h>
#include <Servo.h>
#include <semphr.h>

#define CART_LEFT_PIN 13
#define CART_RIGHT_PIN 12

#define LOADER_DISABLE_PIN 8
#define LOADER_STEP_PIN 11
#define LOADER_DIRECTION_PIN 10

#define MEASURE_SERVO_PIN 9
#define MEASURE_SERVO_OPEN_POSITION 66
#define MEASURE_SERVO_CLOSE_POSITION 33

#define BATTERY_VOLTAGE_PIN A5
#define VOLTAGE_MEDIUM_TRESHOLD 250
#define VOLTAGE_HIGH_TRESHOLD 290

AccelStepper loader(AccelStepper::DRIVER, LOADER_STEP_PIN, LOADER_DIRECTION_PIN);
AccelStepper dropper(AccelStepper::HALF4WIRE, 2, 3, 4, 5);

Servo measure_servo;

SemaphoreHandle_t xCartMutex;
SemaphoreHandle_t xBatteryMutex;
SemaphoreHandle_t xDropperOpenMutex;
SemaphoreHandle_t xDropperCloseMutex;

enum BatteryBoxes { empty, partial, full };

BatteryBoxes selected_box = empty;

void CartTask( void *pvParameters );
void GaugeTask( void *pvParameters );

void setup()
{
  // Setting up Serial Port
  Serial.begin(9600);

  // Creating Mutexes
  xCartMutex = xSemaphoreCreateBinary();    // Cart is ready to accept new battery
  xBatteryMutex = xSemaphoreCreateBinary(); // Battery is ready to be loaded into a dropper
  xDropperOpenMutex = xSemaphoreCreateBinary(); // Dropper is opening
  xDropperCloseMutex = xSemaphoreCreateBinary(); // Dropper is closing
  if (xCartMutex != NULL && xBatteryMutex != NULL) {
    Serial.println("Mutex created");
  }

  // Running Threads
  xTaskCreate( GaugeTask, "GaugeTask", 128, NULL, 1, NULL );
  xTaskCreate( CartTask, "CartTask", 128, NULL, 2, NULL );
  xTaskCreate( DropperTask, "DropperTask", 128, NULL, 3, NULL );
}

void load_battery() {
  digitalWrite(LOADER_DISABLE_PIN, LOW);
  loader.runToNewPosition(300);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  loader.runToNewPosition(0);
  digitalWrite(LOADER_DISABLE_PIN, HIGH);
}

void eject_battery() {
  digitalWrite(LOADER_DISABLE_PIN, LOW);
  loader.runToNewPosition(-700);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  loader.runToNewPosition(0);
  digitalWrite(LOADER_DISABLE_PIN, HIGH);
}

BatteryBoxes measure_voltage() {
  BatteryBoxes box;
  int min = 999;
  int max = 0;
  int voltage = 0;
  
  measure_servo.write(MEASURE_SERVO_CLOSE_POSITION);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

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
  vTaskDelay(500 / portTICK_PERIOD_MS);
  return box;
}

void move_cart(int pin, int milis) {
  digitalWrite(pin, HIGH);
  vTaskDelay(milis / portTICK_PERIOD_MS);
  digitalWrite(pin, LOW);
}

void drop_battery_to_box(BatteryBoxes box) {
  int duration;
  switch (box)
  {
    case empty:
      duration = 0;
      break;
    case partial:
      duration = 600;
      break;
    case full:
      duration = 1200;
      break;
  }
  move_cart(CART_RIGHT_PIN, duration);
  xSemaphoreGive(xDropperOpenMutex);
  move_cart(CART_LEFT_PIN, duration * 1.2);
}

void loop() {
  // Every action will be performed in Tasks below
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/
void DropperTask( void *pvParameters )
{
  // Setup Dropper
  dropper.setMaxSpeed(500);
  dropper.setAcceleration(1000);
  dropper.setSpeed(100);
  dropper.runToNewPosition(1202);
  dropper.runToNewPosition(2);
  
  while(1)
  {
    if(xSemaphoreTake(xDropperOpenMutex, portMAX_DELAY) == pdTRUE)
    {
      dropper.runToNewPosition(450);
      vTaskDelay(500 / portTICK_PERIOD_MS);
      dropper.runToNewPosition(0);
      vTaskDelay(500 / portTICK_PERIOD_MS);
      xSemaphoreGive(xDropperCloseMutex);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void CartTask( void *pvParameters )
{
  // Calibrate Cart
  pinMode(CART_LEFT_PIN, OUTPUT);
  pinMode(CART_RIGHT_PIN, OUTPUT);
  move_cart(CART_LEFT_PIN, 1600);

  // Cart is in position
  xSemaphoreGive(xCartMutex);
  xSemaphoreGive(xDropperCloseMutex);
  while (1)
  {
    if( xSemaphoreTake(xBatteryMutex, portMAX_DELAY) == pdTRUE )
    {
      drop_battery_to_box(selected_box);
      xSemaphoreGive(xCartMutex);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void GaugeTask( void *pvParameters )
{
  BatteryBoxes selected_box_thread;

  // Measure Servo Setup
  measure_servo.attach(MEASURE_SERVO_PIN);
  measure_servo.write(MEASURE_SERVO_OPEN_POSITION);

  // Setup Loader
  pinMode(LOADER_DISABLE_PIN, OUTPUT);

  loader.setMaxSpeed(1000);
  loader.setAcceleration(10000);
  loader.setSpeed(1000);
  loader.runToNewPosition(0);
  
  digitalWrite(LOADER_DISABLE_PIN, HIGH);
  while (1)
  {
    load_battery();
    selected_box_thread = measure_voltage();
    while ( xSemaphoreTake(xCartMutex, portMAX_DELAY) != pdTRUE ) { vTaskDelay(500 / portTICK_PERIOD_MS); }
    selected_box = selected_box_thread;
    while ( xSemaphoreTake(xDropperCloseMutex, portMAX_DELAY) != pdTRUE ) { vTaskDelay(100 / portTICK_PERIOD_MS); }
    eject_battery();
    xSemaphoreGive(xBatteryMutex);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
