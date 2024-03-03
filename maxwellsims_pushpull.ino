/*
 * 
 * DIGF 2002 - Physical Computing
 * Nick Puckett
 *
 * Proximity-Responsive LED Animation Speed Adjustment
 * 
 * This Arduino program dynamically adjusts the playback speed of an LED matrix animation based on the proximity
 * of an object to an HCSR04 ultrasonic sensor. The program defines a "push/pull" interaction zone, where being on
 * one side of a threshold speeds up the animation and the other side slows it down, creating an engaging interactive experience.
 * 
 * Key Features:
 * - Real-time distance measurement with HCSR04 ultrasonic sensor.
 * - Dynamic adjustment of LED animation speed based on object proximity.
 * - Implementation of a "push/pull" mechanism to increase or decrease playback speed.
 * - Use of a buffer to calculate the average distance for smoother speed transitions.
 * - Constrained playback speed to prevent exceeding maximum or minimum values.
 * 
 * Dependencies:
 * - HCSR04.h for ultrasonic distance measurement.
 * - Arduino_LED_Matrix.h for controlling the LED matrix.
 * - state1.h for the initial LED matrix animation sequence.
 * - changePlaySpeed.h for adjusting the animation speed in response to distance changes.
 * 
 * Hardware Setup:
 * - HCSR04 sensor's trigger pin connected to digital pin 14 and echo pin to digital pin 15.
 * - Arduino R4 Wifi
 * 
 * Setup Notes :
 * - Download the code for you animation as a .h file and copy it into the sketch folder
 * - Inside the .h file, remove the ' const ' from the beginning of the variable so that it can be changed in the code
 *
 * Important Variables :
 **These set the fastest and slowest possible speed for the video to play
  int maxPlaySpeed = 20; //fps in milliseconds
  int minPlaySpeed = 1000; //fps in millisecond

  int pushPullAmount = 5; //the amount added or subtracted from the current speed based on relationship to the interactionEdge
                            Lower number = slower response

  float interactionEdge = 15; //defines the threshold of push/pull based on position
 *
 * New Methods
 * - constrain() to keep a value between 2 fixed limits
 * https://www.arduino.cc/reference/en/language/functions/math/constrain/
 * 
 */

#include <HCSR04.h>
#include "Arduino_LED_Matrix.h"
#include "changePlaySpeed.h"
#include "birdanimation.h"

byte triggerPin = 14;
byte echoPin = 15;
UltraSonicDistanceSensor distanceSensor(triggerPin, echoPin);

unsigned long lastRead = 0;
int rate = 30;

const int numReadings = 10; // Number of readings to compute the average
                            // Sets the sensitivity vs smoothness : lower number = more sensitive / higher number = more smooth
float readings[numReadings]; // Buffer to store the last N readings
int readIndex = 0; // Index for inserting the next reading
float total = 0; // Total of the readings for calculating the average
float averageDistance = 0; // The average distance
float previousAverageDistance = -1;

int maxPlaySpeed = 30; //fps in milliseconds
int minPlaySpeed = 140; //fps in millisecond
int currentPlaySpeed = (minPlaySpeed - maxPlaySpeed)/2; //half way between min/max to start out. dynamically calculated after

int pushPullAmount = 2; //the amount added or subtracted from the current speed based on relationship to the interactionEdge

int playBackSpeed; //used to avoid updating the speed if it hasn't changed

//this defines the area that is the active zone of interaction that can change the speed
float interactionEdge = 6; //defines the threshold of push/pull



//set up matrix and animations
ArduinoLEDMatrix ledScreen;  
boolean loopAnimation = true;

void setup () 
{
  Serial.begin(9600);

    //turn on the screen, load the animation and play it
    ledScreen.begin();
    ledScreen.loadSequence(birdanimation);//add the name of your animation inside the ()
    ledScreen.play(loopAnimation);
  
  // Initialize all the readings to 0.0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0.0;
  }
}

void loop () 
{
  readDistanceSensor();
  determinePlaySpeed();

previousAverageDistance = averageDistance;
}

void determinePlaySpeed()
{
  //check if the position is inside the active zone
  
  if(averageDistance<=interactionEdge)
  {
    currentPlaySpeed -= pushPullAmount;
  }
  else 
  {
    currentPlaySpeed += pushPullAmount;  
  }

  //this ensures it will not go out of the defined range

  currentPlaySpeed = constrain(currentPlaySpeed, maxPlaySpeed, minPlaySpeed);



  if(playBackSpeed!=currentPlaySpeed)
  {
    playBackSpeed = currentPlaySpeed;
    changePlaySpeed(birdanimation, playBackSpeed);

  }

  Serial.print("Current Play Speed: ");
  Serial.print("\t");
  Serial.println(playBackSpeed);


}


void readDistanceSensor() 
{
  unsigned long currentTime = millis();
  
  if ((currentTime - lastRead) >= rate) {
    lastRead = currentTime;
    float distanceMeasured = distanceSensor.measureDistanceCm();
    
    if (distanceMeasured > 0) { // Filter out invalid readings
      // Update the rolling average:
      total -= readings[readIndex]; // Subtract the old reading
      readings[readIndex] = distanceMeasured; // Add the new reading
      total += readings[readIndex]; // Add the new reading to the total
      readIndex = (readIndex + 1) % numReadings; // Advance to the next position
      
      averageDistance = total / numReadings; // Calculate the new average
      
    Serial.print("Distance: ");
    Serial.print(averageDistance);
    Serial.println(" cm");
    }
  }
}


