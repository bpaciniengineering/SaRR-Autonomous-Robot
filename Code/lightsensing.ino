//*************************************************************************
//*************************************************************************

//          	Autonomous Light Tracking Code
//          	Lena Dubitsky and Jesus Serrano Cendejas
//   	Version that doesn't need USB
//readLightSensors()
//updateWheels()
//forwardRobot()
//spinRobot()
//stopRobot()
//fixPoint()
//killRobot()

//*************************************************************************
//*************************************************************************

//*************************************************************************
//  Global Variables and Constants
//*************************************************************************
// for motor values: 1000-1500 is forward, 1500-2000 is reverse, speeds increase as values move away from 1500
#include <Servo.h>

//RC controller
int Ch1, Ch2, Ch3, Ch4, Ch5, Ch6;

//variables to hold data
//these hold values related by the sensors
int LeftSensor;
int RightSensor;
int dist;   	// combined sensor output, indicates distance from light source
int direc;  	// left-right sensor output, indicates whether robot is facing towards light or not
int point;
int workingDirecThreshold;
int workingDistThreshold;

//arrays for light sensors
int LSensor[10];
int RSensor[10];
int i = 0;

int SharpValue; // Variable to hold Sharp data
const int SharpPin = A6; //Pin connecting the sharp

//these hold the "speeds" of the wheels
int LWheel;
int RWheel;

//has the first spin happened?
int firstSpin = 0;

//hard constants for when to stop initial spin
const int direcThreshold = 50;
const int distThreshold = 1600;
const int fixR = 1465;
const int fixL = 1460;

Servo L_Servo;  // Servo DC Motor Driver (Designed for RC cars)Servo L_Servo;  // Servo DC Motor Driver (Designed for RC cars)
Servo R_Servo;  // Servo DC Motor Driver (Designed for RC cars)Servo R_Servo;  // Servo DC Motor Driver (Designed for RC cars)

//*************************************************************************
//  Setup
//*************************************************************************
void setup() {
  // Set the pins that the transmitter will be connected to all to input
  pinMode(12, INPUT); //I connected this to Chan1 of the Receiver
  pinMode(11, INPUT); //I connected this to Chan2 of the Receiver
  pinMode(10, INPUT); //I connected this to Chan3 of the Receiver
  pinMode(9, INPUT); //I connected this to Chan4 of the Receiver
  pinMode(8, INPUT); //I connected this to Chan5 of the Receiver
  pinMode(7, INPUT); //I connected this to Chan6 of the Receiver
  pinMode(13, OUTPUT); //Onboard LED to output for diagnostics

  pinMode(6, INPUT); // for sharp sensor
  pinMode(A9, INPUT); // right light sensor should be connected to pin A9, it needs to be reading inputs
  pinMode(A8, INPUT); // left sensor
  R_Servo.attach(2); //Pin 2, right wheel
  L_Servo.attach(3); //Pin 3, left wheel

  Serial.begin(9600); // create port
}

//*************************************************************************
//  Main Loop
//*************************************************************************
void loop() {
 
Ch5 = pulseIn(8, HIGH, 21000);

while(Ch5 < 1900) {
  Ch5 = pulseIn(8, HIGH, 21000);
}

  //this spin will only get executed the first time around.
  if (firstSpin == 0) {
	spinRobot();
	//  Serial.println("I'm doing the first spin!");
	firstSpin = 1;
  }

  //  Serial.println("Entering loop()");

  //Robot has performed initial spin, and it's pointing at the light
  readLightSensors();

  //Robot is pointing at light
  while (dist < workingDistThreshold && abs(direc) < workingDirecThreshold) {
	//   Serial.println("I'm pointing at light! Doing nothing");
	//delay(500);
	//stopRobot();
	forwardRobot();
	readLightSensors();
  }

  //Robot is NOT pointing at light
  while (dist > workingDistThreshold || abs(direc) > workingDirecThreshold) {
	turnRobot();
	//forwardRobot();
	//delay(2000);
	// fixPoint();
	readLightSensors();
  }

  SharpValue = analogRead(SharpPin); //Read the value of the sharp sensor
  // Serial.println("Sharp =  " + (String)SharpValue);
  if (SharpValue > 300)
	killRobot();

  //When we're 1ft away from light, stop.
  //Some logic here...
  //killRobot();
}

//*************************************************************************
//  readLightSensors
//*************************************************************************
void readLightSensors() {
  //read and print values from both light sensors

  LeftSensor = analogRead(A8);
  RightSensor = analogRead(A9);
  dist = LeftSensor + RightSensor;  // associated with distance to light
  direc = LeftSensor - RightSensor; // associated with direction to light

  //Serial.println("L = " + (String)LeftSensor + " | " + (String)RightSensor + " = R");
}


//*************************************************************************
//  readLightSensors
//*************************************************************************
void readLightSensorsEdgy() {
  //read and store average of latest 10 sensor values

  //use arrays to calculate average
  int l = analogRead(A8);
  LSensor[i] = l;
  int r = analogRead(A9);
  RSensor[i] = r;

  //iterate through arrays
  i++;
  i = i % 10; //if i = 10, then restart at i = 0

  int j;
  int sumL = 0;
  int sumR = 0;
  for (j = 0; j < 10; j++) {
	sumL = sumL + LSensor[j];
	sumR = sumR + RSensor[j];
  }

  LeftSensor = sumL / 10;
  RightSensor = sumR / 10;
  dist = LeftSensor + RightSensor;  // associated with distance to light
  direc = LeftSensor - RightSensor; // associated with direction to light

  //Serial.println("L = " + (String)LeftSensor + " | " + (String)RightSensor + " = R");
}


//*************************************************************************
//  updateWheels
//*************************************************************************
void updateWheels() {

  //set limits
  if (RWheel > 2000)
	RWheel = 2000;
  if (RWheel < 1000)
	RWheel = 1000;

  if (LWheel > 2000)
	LWheel = 2000;
  if (LWheel < 1000)
	LWheel = 1000;

  //update the wheels
  R_Servo.writeMicroseconds(RWheel); // 1000-2000, 1500 should be stop
  L_Servo.writeMicroseconds(LWheel); // 1000-2000, 1500 should be stop
}


//*************************************************************************
//  turnRobot
//*************************************************************************
void turnRobot() {

  //  Serial.println("Performing turnRobot()");

  //start from a stopped position
  //forwardRobot();

  while (abs(direc) > direcThreshold || dist > distThreshold) {
	//  Serial.println("turning");

	//check to see where we are pointing: + = L, - = R
	readLightSensors();
	//  Serial.println("Point = " + (String)direc);

	//if robot is pointing right, spin counterclockwise
	if (direc < 0) {
  	//	Serial.println("I'm pointing right! Spinning CCW.");
  	RWheel = 1455; // right wheel going forwards
  	// LWheel = 1550; // left wheel going backwards
  	updateWheels();
	}

	//if robot is pointing left, spin clockwise
	if (direc > 0) {
  	//	Serial.println("I'm pointing left! Speeding up CW.");
  	LWheel = 1435; // reft wheel going forwards
  	// RWheel = 1550; // right wheel going backwards
  	updateWheels();
	}

	//try reading again
	readLightSensors();
  }

  forwardRobot();

  //success, robot is looking at light
  // Serial.println("Success! I'm pointing at light! Stopping.");
  // Serial.println("point =  " + (String)direc);

  SharpValue = analogRead(SharpPin); //Read the value of the sharp sensor
  //  Serial.println("Sharp =  " + (String)SharpValue);
  if (SharpValue > 400)
	killRobot();

}
//*************************************************************************
//  spinRobot
//*************************************************************************
void spinRobot() {

  //  Serial.println("Performing spinRobot()");

  //start from a stopped position
  stopRobot();

  // for initial spin
  readLightSensors();

  //if either condition is bad, spin
  while (abs(direc) > direcThreshold || dist > distThreshold) {
	// Serial.println("spinning");
	//try reading again
	readLightSensors();

	RWheel = 1460; // right wheel going forwards
	LWheel = 1550; // left wheel going backwards
	updateWheels();
  }

  //success, robot is looking at light
  // Serial.println("Success! I'm pointing at light! Stopping.");
  workingDirecThreshold = direc;
  workingDistThreshold = dist;
  // Serial.println("workingDirecThreshold =  " + (String)workingDirecThreshold);
  // Serial.println("workingDistThreshold =  " + (String)workingDistThreshold);
  // Serial.println("point =  " + (String)direc);

  stopRobot();
  delay(500);
}

//*************************************************************************
//  stopRobot
//*************************************************************************
void stopRobot() {
  //set wheels to zero
  RWheel = 1500;
  LWheel = 1500;
  updateWheels();
}

//*************************************************************************
//  forwardRobot
//*************************************************************************
void forwardRobot() {
  //roll forward slowly with even speeds
  RWheel = 1470;
  LWheel = 1455;
  updateWheels();
}


//*************************************************************************
//  killRobot
//*************************************************************************
void killRobot() {
  //set wheels to zero
  RWheel = 1500;
  LWheel = 1500;
  updateWheels();

  //pause forever
  while (SharpValue > 400) {
	stopRobot();
	SharpValue = analogRead(SharpPin);
  }
}
