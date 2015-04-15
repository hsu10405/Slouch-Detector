/**
 AttinyAccelTestV2.ino

 Purpose: To read acceleration data from MMA8452 accelerometer with ATTINY85.
 Code adopted from Nathan Seidel of SparkFun Electronics
 Original code: https://github.com/dmcinnes/acceleromocity/blob/master/accel.ino

 @author Kasun Somaratne
 @version 2 Nov 15, 2014

  Hardware setup:
 MMA8452 Breakout ------------ ATTiny
 3.3V --------------------- 3.3V
 SDA -------^^(330)^^------- Pin5
 SCL -------^^(330)^^------- Pin7
 GND ---------------------- GND

 pin1 --- !RESET  (reset)
 pin2 --- s0      (3)
 pin3 --- s1      (4)
 pin4 --- GND     (-)
 pin5 --- SDA     (0)
 pin6 --- PWM     (1)
 pin7 --- SCL     (2)
 pin8 --- VCC     (+)

 */

#include <TinyWireM.h>

#define MMA8452_ADDRESS 0x1D

//define all pins
#define S0 3
#define S1 4
#define SDA 0
#define PWM 1
#define SCL 2
//Some of the registers of the MMA8452
#define OUT_X_MSB 0x01
#define XYZ_DATA_CFG 0x0E
#define WHO_AM_I 0x0D
#define CTRL_REG1 0x2A

//accelerometer sensitivity can be 2,4, or 8g
#define GSCALE 2

//store current accelerometer values for x,y,z axis
float currentAcc[3] = {0.0, 0.0, 0.0};

int s0_cur = 0;

void setup()
{
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(PWM, OUTPUT);
  digitalWrite(PWM, LOW); //always init low
  digitalWrite(S0, LOW);
  digitalWrite(S1, LOW);
  //Initialize the I2C communication bus
  TinyWireM.begin();

  //Test and initialize the MMA8452
  initMMA8452();
  digitalWrite(S0, HIGH);
  initMMA8452();
  digitalWrite(S0, LOW);
  delay(100);

}


/*calculates orienations from accel data, stores in orienations[] array */
void measure_accel(float * orientations) {
  int accelCount[3];

  readAccelData(accelCount);  // Read the x/y/z adc values

  // Now we'll calculate the accleration value into actual g's
  float accelG[3];  // Stores the real accel value in g's
  for (int i = 0 ; i < 3 ; i++)
  {
    accelG[i] = (float) accelCount[i] / ((1 << 12) / (2 * GSCALE)); // get actual g value, this depends on scale being set
  }

  //calculate degrees from g's value
  orientations[0] = 180 * atan2(accelG[0], sqrt(square(accelG[1]) + square(accelG[2]))) / 3.1415926;
  orientations[1] = 180 * atan2(accelG[1], sqrt(square(accelG[0]) + square(accelG[2]))) / 3.1415926;
  orientations[2] = 180 * atan2(sqrt(square(accelG[0]) + square(accelG[1])), accelG[2]) / 3.1415926;

}

/* buzzes user for a little bit */
void buzz(int time) {
  for (int i = 0; i < time; i++) {
    digitalWrite(PWM, HIGH);
    delay(1);
    digitalWrite(PWM, LOW);
    delay(1);
  }
}

//loop with strike system
void loop()
{
  static float accel_1[3];  // Stores 5 instances ofthe 12-bit signed value of first accelerometer
  static float accel_2[3];  // Stores the 12-bit signed value of second accelerometer
  static int strikes[3];//keeps track of strikes per axis


  //update measurement for accel1
  digitalWrite(S0, LOW);
  measure_accel(accel_1);
  
  //update measurement for accel2
  digitalWrite(S0, HIGH);
  measure_accel(accel_2);
  

    for (int i = 0; i < 3; i++) {
      if (abs(accel_1[i] - accel_2[i]) > 20) {
        strikes[i]++;
      }// add srikes if deviation >20 degrees
      else{
        strikes[i]=0;
      }//set strikes to zero if no deviation
      if(strikes[i] > 10){
        buzz(100);
      }//buzz if 10 strikes in a row
  }
}
//tried loop with rolling average
/*
#define measurements 5
void loop()
{
  static float accel_1[measurements][3];  // Stores 5 instances ofthe 12-bit signed value of first accelerometer
  static float accel_2[measurements][3];  // Stores the 12-bit signed value of second accelerometer
  static float rolling_avg_1[3];
  static float rolling_avg_2[3];
  static int cur = 0;
  static bool have_enough_measurements = false;
  //Retrieve the current accelerometer readings


  //update rolling average for accel1
  digitalWrite(S0, LOW);
  rolling_avg_1[0] -= (accel_1[cur][0]/measurements);
  rolling_avg_1[1] -= (accel_1[cur][1]/measurements);
  rolling_avg_1[2] -= (accel_1[cur][2]/measurements);
  measure_accel(accel_1[cur]);
  rolling_avg_1[0] += (accel_1[cur][0]/measurements);
  rolling_avg_1[1] += (accel_1[cur][1]/measurements);
  rolling_avg_1[2] += (accel_1[cur][2]/measurements);
  
  //update rolling average for accel2
  digitalWrite(S0, HIGH);
  rolling_avg_2[0] -= (accel_2[cur][0]/measurements);
  rolling_avg_2[1] -= (accel_2[cur][1]/measurements);
  rolling_avg_2[2] -= (accel_2[cur][2]/measurements);
  measure_accel(accel_2[cur]);
  rolling_avg_2[0] += (accel_2[cur][0]/measurements);
  rolling_avg_2[1] += (accel_2[cur][1]/measurements);
  rolling_avg_2[2] += (accel_2[cur][2]/measurements);
 
 
  cur++; 
  if(cur >= measurements)have_enough_measurements = true;
  cur = cur % measurements;
  
  
  //once we have enough measurements, we can start doing rolling average
  if (have_enough_measurements) {
    for (int i = 0; i < 3; i++) {
      if (abs(rolling_avg_1[i] - rolling_avg_2[i]) > 20) {
        buzz(100);
      }// buzz user if deviation in one way of >20 degrees
    }
  }
}*/

//Test and initialize the accelerometer
void initMMA8452() {
  //Read WHO_AM_I register. This is the first step to see if
  //communication can be estabilished with the MMA8452
  byte c = readRegister(WHO_AM_I);
  if (c == 0x2A) {
    buzz(20);
    delay(20);
  }
  else {
    digitalWrite(S0, HIGH);
    while (1) {
      buzz(500);
    }
  }

  //Must be in standby mode to change registers
  MMA8452Standby();

  //Set up full scale range to 2, 4, or 8g
  byte fsr = GSCALE;
  if (fsr > 8) fsr = 8; //Easy error check
  fsr >>= 2; //Neat trick, see page 22 of datasheet. 00 = 2G, 01 = 4A, 10 = 8G
  writeRegister(XYZ_DATA_CFG, fsr);

  //The default data rate is 800Hz and we don't modify it in this example code

  // Set to active to start reading
  MMA8452Active();
}

// Read a single byte from addressToRead and return it as a byte
byte readRegister(byte addressToRead) {
  TinyWireM.beginTransmission(MMA8452_ADDRESS);
  TinyWireM.write(addressToRead);
  //endTransmission but keep the connection active (repeated start)
  TinyWireM.endTransmission(false);

  //Ask for 1 byte, once done, bus is released by default
  TinyWireM.requestFrom(MMA8452_ADDRESS, 1);

  while (!TinyWireM.available()) ; //Wait for the data to come back
  return TinyWireM.read(); //Return this one byte
}

// Read bytesToRead sequentially, starting at addressToRead into the dest byte array
void readRegisters(byte addressToRead, int bytesToRead, byte * dest) {
  TinyWireM.beginTransmission(MMA8452_ADDRESS);
  TinyWireM.write(addressToRead);
  TinyWireM.endTransmission(false); //endTransmission but keep the connection active

  //Ask for bytes, once done, bus is released by default
  TinyWireM.requestFrom(MMA8452_ADDRESS, bytesToRead);
  //Hang out until we get the # of bytes we expect
  while (TinyWireM.available() < bytesToRead);

  for (int x = 0 ; x < bytesToRead ; x++)
    dest[x] = TinyWireM.read();
}

// Writes a single byte (dataToWrite) into addressToWrite
void writeRegister(byte addressToWrite, byte dataToWrite) {
  TinyWireM.beginTransmission(MMA8452_ADDRESS);
  TinyWireM.write(addressToWrite);
  TinyWireM.write(dataToWrite);
  TinyWireM.endTransmission(); //Stop transmitting
}

// Sets the MMA8452 to standby mode. It must be in standby to change most register settings
void MMA8452Standby() {
  byte c = readRegister(CTRL_REG1);
  writeRegister(CTRL_REG1, c & ~(0x01)); //Clear the active bit to go into standby
}

// Sets the MMA8452 to active mode. Needs to be in this mode to output data
void MMA8452Active() {
  byte c = readRegister(CTRL_REG1);
  writeRegister(CTRL_REG1, c | 0x01); //Set the active bit to begin detection
}


/*
// Updates the accelCount array with current accel readings
void updateAccelData() {
  int accelCount[3]; // Stores the 12-bit signed value

  readAccelData(accelCount); // Read the x/y/z adc values

  // Now we'll calculate the accleration value into actual g's
  float accelG[3]; // Stores the real accel value in g's
  for (byte i = 0; i < 3; i++) {
    // get actual g value, this depends on scale being set
    accelG[i] = (float) accelCount[i] / ((1 << 12) / (2 * GSCALE));

    // use a rolling filter. This is the additional line
    currentAcc[i] = 0.95 * accelG[i] + currentAcc[i] * 0.05;
  }
}*/

// Reads accel data from the MMA8452
void readAccelData(int *destination)
{
  byte rawData[6]; // x/y/z accel register data stored here

  readRegisters(OUT_X_MSB, 6, rawData); // Read the six raw data registers into data array

  // Loop to calculate 12-bit ADC and g value for each axis
  for (int i = 0; i < 3 ; i++)
  {
    //Combine the two 8 bit registers (MSB and LSB) into one 12-bit number
    int gCount = (rawData[i * 2] << 8) | rawData[(i * 2) + 1];
    gCount >>= 4; //The registers are left align, here we right align the 12-bit integer

    // If the number is negative, we have to make it so manually (no 12-bit data type)
    if (rawData[i * 2] > 0x7F) {
      // Transform into negative 2's complement #
      gCount = ~gCount + 1;
      gCount *= -1;
    }

    destination[i] = gCount; //Record this gCount into the 3 int array
  }
}
