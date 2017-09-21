/* by .m. / June 2017
   Drawing Maching:  with 1 'arm' with 1 joint, driven by 2 stepper-motors; and Servos for cleaning and driving pen
   - Draws Numbers (Time) on a affordable surface
   different sections:
   - Constants with the graphics-Path for the numbers, structure to stream the coordinates 
   - Basic calculations (resulting in inner and outer angle)
   - calculating steps and directions from angles
   - outer angle has to compensate inner angle due to the way the drawing machine is built
   - driving the steppers smootheley
*/

#include <Arduino.h>
// Letters
#include "Letters.h"
// Stepper
#include "BasicStepperDriver.h"
// Servo
#include "esp32-hal-ledc.h"

// Internet-Time
#include <WiFi.h>
#include <TimeLib.h>
// 195.186.1.101
IPAddress timeServer(195, 186, 1, 101);
const long timeZoneOffset = 7200L;
unsigned int ntpSyncTime = 3600;
// local port to listen for UDP packets
unsigned int localPort = 8888;
// NTP time stamp is in the first 48 bytes of the message
const int NTP_PACKET_SIZE = 48;
// Buffer to hold incoming and outgoing packets
byte packetBuffer[NTP_PACKET_SIZE];
// A UDP instance to let us send and receive packets over UDP
// UDP Udp;
WiFiUDP Udp;
// Keeps track of how long ago we updated the NTP server
unsigned long ntpLastUpdate = 0;
IPAddress zeroAdr = IPAddress(0,0,0,0);
bool validIpAdr = false;
// Check last time clock displayed (Not in Production)
time_t prevDisplay = 0;

// Access to WiFi
 const char* ssid = "UPC246574397";
 const char* password = "PFXZYFAM";

// const char* ssid = "FK-Netopia";
// const char* password = "Unsere Katze heisst Max";

// Hall Sensor
#define  HALLSENSOR_PIN 34
#define SEARCHSTEPS 400
#define SINGLESTEP 5
int sensorValue = 0;
int hallCnt = 0;
int cnt = 0;



// Neopixel
#include "ws2812.h"
//#if defined(ESP_PLATFORM)
#include "arduinoish.h""
//#endif
const int DATA_PIN = 23; // Avoid using any of the strapping pins on the ESP32
const uint16_t NUM_PIXELS = 8;  // How many pixels you want to drive
uint8_t MAX_COLOR_VAL = 32; // Limits brightness
rgbVal *pixels;
void displayOff();

// RTC
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 rtc; // 21 SDA 22 SCL

int jj = 0;
float restInner = 0;
float restOuter = 0;

#define INNER_DIR_PIN 32
#define INNER_STEP_PIN 33
#define INNER_ENABLE_PIN 19

#define OUTER_DIR_PIN 25
#define OUTER_STEP_PIN 26
#define OUTER_ENABLE_PIN 27

#define CHAIN_DIR_PIN 12
#define CHAIN_STEP_PIN 13
#define CHAIN_ENABLE_PIN 14

#define STEPS_PER_PI 12000  // Motor * Microsteps * 60 / 16 / 2  (1 Umdrehung = 2PI)
#define STEPS_PER_DEG  66.6666666 // Motor: 1.8 Deg / Step
#define MOTOR_STEPS 200

BasicStepperDriver innerStepper(MOTOR_STEPS, INNER_DIR_PIN, INNER_STEP_PIN, INNER_ENABLE_PIN);
BasicStepperDriver outerStepper(MOTOR_STEPS, OUTER_DIR_PIN, OUTER_STEP_PIN, OUTER_ENABLE_PIN);
BasicStepperDriver chainStepper(32, CHAIN_DIR_PIN, CHAIN_STEP_PIN, CHAIN_ENABLE_PIN);

#define CORR_FACT 3.75 
#define CORR_FACT_INV (1 / CORR_FACT)
#define CORR_INNER_ANGLE (PI / 4)  // 45 deg = PI/4
#define  PI_4 (PI / 4)


// Number-related vars
int currentPos;
float maxX, xAdd = 0, xFactor = 1.05, yFactor = 1.2, yAdd = 12;
LPoint lpAct;
LPoint lp;
int maxChars, points = 0;
String sTime;
DateTime mNow;
int actTimeLetter = 0;
bool inactive = true;
bool stop = true;
bool firstMove = true;

// physical layout
int innerSteps;
int outerSteps;
int innerDir;  // (1 or -1, direction)
int outerDir;  // (1 or -1, direction)
int aktInnerSteps;
int aktOuterSteps;

float oAngleDeg, iAngleDeg, iAngleOld, oAngleOld;
float oAngle = PI_4;
float iAngle = PI_4;
float zPoint_x = 80; // zPoint from mPoint; mPoint=lower left of drawable surface
float zPoint_y = -90; // dito

const float lInner = 100;
const float lOuter = 120;
const float pWidth = 150;
const float pHeight = 90;
const float xRest = -10;
const float yRest = 23;

char l0[] = "35.56,38.60,0;35.48,36.15,;35.27,33.82,;34.90,31.62,;34.39,29.53,;33.73,27.59,;32.92,25.76,;31.96,24.05,;30.87,22.48,;29.65,21.06,;28.34,19.82,;26.93,18.78,;25.42,17.94,;23.82,17.27,;22.12,16.80,;20.32,16.51,;18.44,16.42,;16.56,16.50,;14.79,16.72,;13.10,17.11,;11.52,17.63,;10.02,18.33,;8.63,19.16,;7.33,20.16,;6.13,21.31,;5.04,22.58,;4.11,23.96,;3.30,25.46,;2.66,27.05,;2.15,28.76,;1.79,30.58,;1.57,32.51,;1.50,34.54,;1.58,36.95,;1.83,39.24,;2.25,41.41,;2.85,43.46,;3.60,45.40,;4.54,47.22,;5.64,48.92,;6.90,50.50,;8.29,51.93,;9.77,53.17,;11.32,54.22,;12.96,55.08,;14.68,55.75,;16.48,56.23,;18.37,56.51,;20.33,56.61,;22.04,56.53,;23.66,56.31,;25.19,55.94,;26.62,55.41,;27.97,54.73,;29.22,53.92,;30.38,52.95,;31.45,51.82,;32.41,50.57,;33.25,49.20,;33.95,47.71,;34.53,46.12,;34.98,44.41,;35.30,42.59,;35.49,40.66,;35.56,38.60,129;29.49,37.35,0;29.44,38.94,;29.32,40.44,;29.12,41.85,;28.83,43.18,;28.45,44.43,;27.99,45.59,;27.46,46.67,;26.83,47.67,;26.14,48.56,;25.40,49.34,;24.60,49.99,;23.75,50.52,;22.85,50.95,;21.89,51.25,;20.87,51.43,;19.80,51.48,;18.53,51.42,;17.30,51.21,;16.14,50.85,;15.02,50.36,;13.97,49.73,;12.97,48.96,;12.03,48.05,;11.14,47.00,;10.30,45.78,;9.58,44.49,;8.96,43.09,;8.47,41.61,;8.08,40.03,;7.80,38.38,;7.63,36.63,;7.58,34.79,;7.62,33.30,;7.77,31.91,;8.00,30.58,;8.34,29.34,;8.75,28.17,;9.27,27.09,;9.89,26.07,;10.59,25.15,;11.35,24.33,;12.18,23.62,;13.06,23.02,;14.00,22.53,;15.00,22.15,;16.06,21.88,;17.18,21.72,;18.36,21.66,;19.49,21.73,;20.58,21.94,;21.63,22.30,;22.63,22.80,;23.59,23.44,;24.51,24.22,;25.39,25.13,;26.23,26.19,;26.99,27.37,;27.65,28.59,;28.21,29.89,;28.67,31.25,;29.03,32.68,;29.29,34.17,;29.43,35.73,;29.49,37.35,129;";
char l1[] = "27.13,19.29,0;26.96,18.38,;26.76,17.97,;26.12,17.24,;25.13,16.64,;24.52,16.38,;23.83,16.16,;23.06,15.96,;22.20,15.79,;20.25,15.55,;19.14,15.47,;17.96,15.42,;16.70,15.41,;14.90,15.44,;13.20,15.49,;11.64,15.59,;10.19,15.73,;8.85,15.90,;7.62,16.13,;6.52,16.38,;5.54,16.67,;4.65,17.02,;3.91,17.39,;3.26,17.80,;2.73,18.26,;2.33,18.76,;2.04,19.29,;1.86,19.87,;1.81,20.48,;1.84,21.04,;1.95,21.52,;2.13,21.93,;2.39,22.27,;2.71,22.52,;3.11,22.71,;3.58,22.82,;4.13,22.86,;4.99,22.75,;5.66,22.61,;6.51,22.43,;7.57,22.21,;8.56,22.01,;9.48,21.85,;10.32,21.73,;11.51,26.00,;12.53,30.11,;13.40,34.03,;14.12,37.78,;14.68,41.35,;15.08,44.74,;15.31,47.96,;15.39,50.99,;15.39,51.42,;13.55,50.58,;11.84,49.85,;10.28,49.24,;8.85,48.74,;7.58,48.35,;6.44,48.07,;5.44,47.90,;4.59,47.85,;4.07,47.89,;3.63,48.01,;3.25,48.24,;2.70,48.92,;2.52,49.40,;2.42,49.96,;2.39,50.60,;2.46,51.34,;2.67,51.99,;3.00,52.53,;3.48,52.98,;3.94,53.23,;4.63,53.53,;5.57,53.88,;6.75,54.29,;7.21,54.44,;7.79,54.65,;8.48,54.90,;9.29,55.20,;10.21,55.54,;11.25,55.92,;12.40,56.35,;13.67,56.82,;15.09,57.33,;16.26,57.69,;17.20,57.91,;17.89,57.98,;18.77,57.88,;19.53,57.58,;20.18,57.06,;20.70,56.35,;21.12,55.45,;21.41,54.33,;21.58,53.00,;21.64,51.48,;21.62,50.18,;21.56,48.79,;21.46,47.33,;21.32,45.80,;21.14,44.19,;20.92,42.49,;20.65,40.73,;20.35,38.87,;20.00,36.95,;19.61,34.94,;19.19,32.87,;18.72,30.70,;18.21,28.47,;17.67,26.16,;17.08,23.77,;16.45,21.31,;17.06,21.25,;17.95,21.24,;18.93,21.27,;19.94,21.38,;20.98,21.57,;22.06,21.83,;23.05,22.10,;23.79,22.28,;24.30,22.40,;25.18,22.39,;25.69,22.23,;26.14,21.99,;26.50,21.64,;26.77,21.21,;26.98,20.66,;27.09,20.02,;27.13,19.29,129;";
char l2[] = "30.77,22.73,0;30.45,21.27,;30.03,20.58,;29.45,19.91,;28.72,19.27,;27.81,18.65,;26.74,18.06,;25.51,17.49,;24.03,16.92,;22.43,16.42,;20.74,16.00,;18.93,15.66,;17.01,15.39,;12.86,15.08,;10.63,15.04,;9.34,15.06,;8.14,15.11,;7.02,15.19,;5.98,15.29,;5.02,15.44,;4.15,15.60,;3.36,15.80,;2.66,16.04,;2.03,16.31,;1.50,16.61,;1.04,16.93,;0.67,17.29,;0.38,17.68,;0.17,18.10,;0.04,18.56,;0.00,19.04,;0.10,19.69,;0.40,20.32,;0.90,20.94,;1.60,21.54,;5.75,24.29,;7.53,25.59,;9.19,26.88,;10.71,28.14,;12.11,29.38,;13.38,30.60,;14.52,31.81,;15.54,32.99,;16.43,34.15,;17.28,35.42,;18.01,36.73,;18.64,38.07,;19.15,39.42,;19.54,40.80,;19.83,42.21,;20.00,43.64,;20.06,45.10,;19.98,46.74,;19.73,48.18,;19.32,49.43,;18.74,50.47,;18.00,51.32,;17.10,51.96,;16.04,52.42,;14.81,52.67,;13.24,51.66,;11.79,50.78,;10.46,50.04,;9.24,49.44,;8.15,48.97,;7.17,48.64,;6.31,48.44,;5.56,48.37,;4.93,48.40,;4.39,48.52,;3.93,48.71,;3.55,48.96,;3.26,49.30,;3.05,49.71,;2.92,50.19,;2.88,50.75,;3.02,51.44,;3.19,51.82,;3.44,52.21,;3.76,52.63,;4.14,53.08,;4.61,53.55,;5.13,54.04,;6.35,55.03,;7.62,55.90,;8.97,56.63,;10.39,57.23,;11.86,57.69,;13.42,58.02,;15.02,58.23,;16.70,58.29,;17.86,58.25,;18.94,58.09,;19.94,57.85,;20.88,57.50,;21.74,57.05,;22.52,56.51,;23.23,55.87,;23.86,55.12,;24.42,54.29,;24.91,53.34,;25.31,52.30,;25.66,51.16,;25.92,49.92,;26.10,48.59,;26.22,47.15,;26.25,45.63,;26.18,43.92,;25.99,42.23,;25.68,40.57,;25.23,38.93,;24.67,37.31,;23.96,35.71,;23.14,34.15,;22.19,32.60,;21.10,31.08,;19.90,29.58,;18.57,28.10,;17.11,26.64,;15.52,25.21,;13.81,23.81,;11.96,22.42,;10.00,21.06,;11.22,20.90,;12.32,20.78,;13.30,20.71,;14.19,20.69,;15.07,20.73,;16.00,20.83,;17.00,21.00,;18.04,21.25,;19.12,21.56,;20.26,21.94,;21.44,22.39,;22.68,22.91,;26.18,24.60,;26.91,24.91,;27.54,25.12,;28.08,25.26,;28.50,25.30,;29.03,25.26,;29.50,25.15,;29.89,24.94,;30.45,24.30,;30.63,23.85,;30.74,23.34,;30.77,22.73,129";
char l3[] = "27.01,29.23,0;26.74,26.21,;26.41,24.82,;25.95,23.52,;25.35,22.30,;24.62,21.16,;23.75,20.10,;22.76,19.13,;21.64,18.26,;20.42,17.50,;19.11,16.86,;17.69,16.34,;12.81,15.47,;10.99,15.41,;9.62,15.44,;8.35,15.50,;7.15,15.61,;6.05,15.77,;5.03,15.97,;4.11,16.23,;3.27,16.52,;2.52,16.85,;1.86,17.23,;1.28,17.67,;0.80,18.14,;0.40,18.66,;0.10,19.22,;-0.12,19.83,;-0.26,20.48,;-0.30,21.18,;-0.25,21.91,;-0.07,22.57,;0.22,23.17,;0.64,23.69,;1.13,24.13,;1.69,24.44,;2.31,24.62,;2.99,24.69,;3.66,24.64,;4.24,24.51,;4.74,24.29,;5.16,23.97,;5.48,23.58,;5.73,23.09,;5.88,22.51,;5.96,21.85,;7.10,21.31,;8.30,20.92,;9.55,20.68,;10.87,20.60,;12.03,20.64,;13.11,20.75,;14.14,20.93,;15.10,21.17,;15.98,21.50,;16.81,21.89,;17.57,22.35,;18.26,22.89,;18.87,23.49,;19.41,24.14,;19.85,24.87,;20.22,25.64,;20.51,26.46,;20.71,27.35,;20.84,28.29,;20.88,29.30,;20.80,30.61,;20.59,31.85,;20.22,33.00,;19.71,34.05,;19.04,35.02,;18.24,35.92,;17.28,36.71,;16.18,37.41,;14.81,38.05,;13.25,38.42,;10.63,38.66,;9.85,38.81,;9.19,39.02,;8.62,39.29,;8.16,39.63,;7.79,40.04,;7.53,40.52,;7.39,41.07,;7.33,41.69,;7.38,42.31,;7.50,42.86,;7.70,43.33,;7.99,43.69,;8.36,44.00,;8.81,44.21,;9.34,44.33,;9.96,44.38,;10.26,44.35,;10.65,44.31,;11.13,44.22,;11.71,44.10,;12.38,43.93,;13.14,43.74,;13.98,43.52,;14.93,43.25,;15.86,43.33,;16.65,43.56,;17.33,43.94,;17.88,44.48,;18.31,45.18,;18.62,46.02,;18.81,47.02,;18.86,48.18,;18.78,49.33,;18.53,50.33,;18.11,51.18,;17.53,51.88,;16.80,52.41,;15.88,52.79,;14.80,53.02,;13.56,53.10,;12.98,53.07,;12.29,52.96,;11.51,52.77,;10.61,52.52,;9.74,52.27,;9.03,52.09,;8.44,51.98,;8.00,51.94,;7.48,51.99,;7.01,52.11,;6.63,52.32,;6.06,52.99,;5.89,53.45,;5.78,54.00,;5.75,54.62,;5.78,55.00,;6.07,55.69,;6.33,56.00,;7.04,56.58,;7.51,56.85,;8.06,57.10,;9.33,57.51,;10.83,57.80,;12.56,57.99,;14.50,58.05,;15.65,58.00,;16.75,57.88,;17.78,57.67,;18.76,57.37,;19.68,56.99,;20.54,56.53,;21.34,55.98,;22.09,55.35,;22.76,54.64,;23.34,53.90,;23.82,53.10,;24.22,52.27,;24.53,51.37,;24.75,50.44,;24.89,49.45,;24.93,48.43,;24.87,47.32,;24.67,46.25,;24.34,45.22,;23.87,44.23,;23.28,43.28,;22.56,42.38,;21.69,41.51,;20.69,40.68,;22.18,39.63,;23.46,38.46,;24.54,37.20,;25.44,35.81,;26.13,34.33,;26.62,32.73,;26.91,31.04,;27.01,29.23,129;";
char l4[] = "29.70,33.54,0;29.67,33.19,;29.56,32.88,;28.84,32.02,;28.46,31.77,;28.01,31.55,;27.50,31.34,;26.37,30.99,;25.67,30.83,;24.87,30.67,;23.98,30.51,;20.76,30.06,;20.11,25.51,;19.69,21.15,;19.44,16.88,;19.31,15.86,;19.13,15.00,;18.90,14.31,;18.60,13.77,;18.24,13.38,;17.75,13.10,;17.13,12.94,;16.40,12.88,;15.68,12.95,;15.07,13.14,;14.55,13.46,;14.13,13.91,;13.79,14.49,;13.56,15.20,;13.42,16.04,;13.37,17.00,;13.39,17.40,;13.43,17.73,;13.46,18.15,;13.52,18.66,;13.58,19.25,;13.66,19.92,;13.75,20.69,;13.85,21.54,;13.95,22.48,;14.07,23.50,;14.22,24.62,;14.36,25.81,;14.52,27.10,;14.69,28.47,;14.87,29.93,;13.20,30.09,;11.65,30.28,;10.20,30.50,;8.86,30.77,;7.62,31.06,;6.50,31.40,;5.48,31.76,;4.58,32.18,;3.76,32.61,;3.07,33.08,;2.48,33.59,;2.00,34.14,;1.62,34.72,;1.35,35.35,;1.19,35.99,;1.14,36.69,;1.17,37.41,;1.27,38.26,;1.44,39.23,;1.66,40.30,;1.96,41.51,;2.32,42.82,;2.75,44.25,;3.24,45.80,;3.96,47.70,;4.89,49.64,;6.00,51.63,;7.33,53.68,;8.19,54.89,;9.01,55.93,;9.80,56.81,;10.57,57.52,;11.29,58.08,;12.00,58.48,;12.67,58.73,;13.31,58.81,;13.93,58.77,;14.46,58.68,;14.91,58.54,;15.28,58.34,;15.77,57.75,;15.89,57.37,;15.94,56.92,;15.87,56.44,;15.66,55.86,;15.31,55.19,;14.83,54.41,;12.12,50.60,;11.16,49.11,;10.30,47.54,;9.56,45.94,;8.94,44.29,;8.43,42.57,;8.04,40.80,;7.75,38.98,;7.58,37.11,;9.33,36.32,;11.24,35.76,;13.29,35.42,;15.51,35.31,;15.94,35.31,;16.42,37.27,;16.88,39.11,;17.33,40.86,;17.77,42.51,;18.19,44.06,;18.60,45.51,;19.00,46.88,;19.39,48.13,;19.75,49.29,;20.11,50.33,;20.46,51.29,;20.79,52.15,;21.10,52.91,;21.41,53.57,;21.70,54.14,;21.98,54.60,;22.32,54.99,;22.76,55.26,;23.29,55.42,;23.93,55.48,;24.73,55.44,;25.46,55.29,;26.10,55.06,;26.67,54.72,;27.14,54.32,;27.48,53.87,;27.68,53.38,;27.75,52.86,;27.63,52.20,;27.49,51.70,;27.29,51.06,;27.03,50.31,;26.72,49.44,;26.34,48.45,;25.92,47.33,;24.74,44.31,;23.68,41.40,;22.76,38.60,;21.94,35.92,;23.53,36.18,;25.21,36.56,;25.87,36.72,;26.38,36.84,;26.74,36.91,;27.60,36.88,;28.16,36.72,;28.63,36.45,;29.02,36.08,;29.31,35.60,;29.53,35.02,;29.65,34.33,;29.70,33.54,129;";
char l5[] = "28.69,30.42,0;28.40,26.98,;28.06,25.38,;27.56,23.86,;26.91,22.43,;26.13,21.08,;25.20,19.82,;24.13,18.65,;22.95,17.58,;20.33,15.87,;18.92,15.23,;17.41,14.73,;15.81,14.38,;12.40,14.10,;11.10,14.14,;9.87,14.25,;8.68,14.45,;7.58,14.74,;6.52,15.10,;5.52,15.55,;4.60,16.06,;3.73,16.66,;2.95,17.32,;2.27,18.02,;1.69,18.77,;1.22,19.55,;0.85,20.37,;0.58,21.23,;0.42,22.12,;0.37,23.06,;0.42,23.69,;0.59,24.27,;0.87,24.80,;1.26,25.27,;1.71,25.66,;2.20,25.93,;2.70,26.09,;3.24,26.15,;3.60,26.10,;3.96,25.96,;4.29,25.73,;4.59,25.39,;4.70,25.22,;4.93,24.83,;5.30,24.23,;5.80,23.40,;6.42,22.52,;7.08,21.76,;7.80,21.12,;8.57,20.59,;9.41,20.18,;10.30,19.89,;11.24,19.71,;12.24,19.65,;13.39,19.70,;14.49,19.82,;15.51,20.04,;16.48,20.33,;17.39,20.73,;18.23,21.19,;19.01,21.75,;19.73,22.40,;20.38,23.11,;20.94,23.88,;21.42,24.73,;21.81,25.63,;22.11,26.58,;22.32,27.61,;22.46,28.69,;22.50,29.83,;22.46,31.09,;22.32,32.31,;22.10,33.48,;21.80,34.60,;21.40,35.67,;20.90,36.69,;20.33,37.66,;19.68,38.58,;18.95,39.42,;18.19,40.16,;17.40,40.77,;16.58,41.27,;15.73,41.67,;14.83,41.95,;13.91,42.12,;12.95,42.18,;11.81,42.11,;10.64,41.92,;9.44,41.61,;8.21,41.17,;7.10,40.73,;6.22,40.41,;5.58,40.22,;5.19,40.16,;4.63,40.20,;4.15,40.31,;3.74,40.51,;3.15,41.16,;2.96,41.61,;2.85,42.12,;2.81,42.72,;2.85,44.65,;2.96,46.54,;3.13,48.38,;3.37,50.18,;3.69,51.93,;4.07,53.64,;4.54,55.30,;5.07,56.92,;5.37,57.57,;5.77,58.13,;6.27,58.59,;6.89,58.97,;7.60,59.27,;8.43,59.50,;9.35,59.62,;10.38,59.67,;11.93,59.63,;13.47,59.52,;15.00,59.35,;16.53,59.11,;18.04,58.79,;19.53,58.40,;21.03,57.96,;22.50,57.43,;23.31,57.09,;24.03,56.74,;24.63,56.37,;25.12,55.99,;25.51,55.60,;25.78,55.20,;25.95,54.78,;26.00,54.35,;25.94,53.77,;25.73,53.24,;25.38,52.73,;24.89,52.27,;24.30,51.88,;23.65,51.58,;22.92,51.42,;22.13,51.36,;21.26,51.44,;20.16,51.65,;18.82,52.02,;17.23,52.52,;15.64,53.02,;14.24,53.39,;13.05,53.60,;12.06,53.68,;11.64,53.65,;11.16,53.62,;10.50,53.56,;9.85,52.00,;9.39,50.36,;9.10,48.63,;9.01,46.81,;10.42,47.15,;11.79,47.41,;13.08,47.56,;14.32,47.61,;15.86,47.53,;17.32,47.31,;18.72,46.94,;20.04,46.42,;21.29,45.75,;22.49,44.93,;23.59,43.96,;24.64,42.85,;25.59,41.61,;26.42,40.29,;27.11,38.87,;27.68,37.37,;28.13,35.77,;28.44,34.07,;28.63,32.30,;28.69,30.42,129;";
char l6[] = "28.63,26.61,0;28.57,25.11,;28.42,23.69,;28.14,22.35,;27.76,21.08,;27.27,19.89,;26.67,18.77,;25.97,17.73,;25.16,16.76,;24.25,15.89,;23.27,15.15,;22.23,14.51,;21.12,13.98,;19.92,13.58,;18.67,13.29,;17.33,13.11,;15.94,13.06,;14.39,13.14,;12.91,13.39,;11.55,13.81,;10.27,14.39,;9.08,15.13,;7.99,16.05,;6.99,17.13,;6.08,18.37,;5.28,19.78,;4.58,21.33,;3.98,23.02,;3.49,24.87,;3.11,26.86,;2.85,29.02,;2.69,31.31,;2.63,33.75,;2.70,36.92,;2.91,39.90,;3.25,42.72,;3.73,45.36,;4.35,47.83,;5.10,50.12,;5.99,52.24,;7.02,54.20,;8.14,55.89,;9.34,57.35,;10.64,58.60,;12.03,59.62,;13.52,60.41,;15.08,60.97,;16.75,61.32,;18.50,61.43,;19.65,61.37,;20.75,61.21,;21.80,60.93,;22.79,60.54,;23.73,60.03,;24.62,59.42,;25.46,58.69,;26.25,57.86,;26.99,56.91,;27.51,56.03,;27.82,55.19,;27.92,54.41,;27.85,53.83,;27.63,53.29,;27.27,52.79,;26.75,52.32,;26.15,51.93,;25.51,51.64,;24.84,51.47,;24.14,51.42,;23.67,51.48,;23.25,51.67,;22.86,51.99,;22.50,52.43,;21.92,53.25,;21.34,53.96,;20.76,54.56,;20.17,55.06,;19.58,55.45,;18.98,55.71,;18.38,55.88,;17.77,55.94,;16.90,55.86,;16.06,55.64,;15.27,55.27,;14.51,54.74,;13.79,54.07,;13.11,53.26,;12.48,52.29,;11.88,51.17,;11.28,49.85,;10.77,48.42,;10.31,46.85,;9.92,45.16,;9.60,43.34,;9.33,41.38,;9.14,39.31,;9.01,37.11,;9.72,37.85,;10.50,38.48,;11.34,39.03,;12.24,39.46,;13.20,39.81,;14.23,40.06,;15.31,40.21,;16.45,40.26,;17.75,40.20,;18.98,40.01,;20.16,39.71,;21.27,39.29,;22.33,38.74,;23.34,38.07,;24.29,37.29,;25.18,36.37,;25.98,35.38,;26.69,34.31,;27.28,33.18,;27.77,32.00,;28.15,30.75,;28.42,29.43,;28.57,28.05,;28.63,26.61,129;22.38,25.30,0;22.34,26.34,;22.24,27.32,;22.08,28.25,;21.84,29.13,;21.54,29.97,;21.17,30.75,;20.73,31.48,;20.22,32.17,;19.67,32.78,;19.07,33.31,;18.45,33.76,;17.79,34.13,;17.10,34.42,;16.37,34.62,;15.61,34.74,;14.81,34.79,;13.59,34.67,;12.50,34.32,;11.54,33.73,;10.71,32.91,;10.04,31.90,;9.58,30.71,;9.29,29.36,;9.20,27.86,;9.33,25.93,;9.73,24.16,;10.40,22.56,;11.34,21.10,;11.90,20.47,;12.48,19.91,;13.09,19.44,;13.73,19.05,;14.40,18.76,;15.09,18.54,;15.81,18.42,;16.57,18.37,;17.83,18.48,;18.96,18.83,;19.94,19.41,;20.80,20.21,;21.50,21.23,;21.99,22.41,;22.28,23.77,;22.38,25.30,129;";
char l7[] = "32.94,52.06,0;32.89,51.33,;32.73,50.44,;32.49,49.42,;32.14,48.25,;31.05,45.12,;29.82,42.05,;28.45,39.04,;26.95,36.08,;25.32,33.18,;23.56,30.33,;21.66,27.54,;19.63,24.81,;17.67,22.39,;15.69,20.20,;13.69,18.24,;11.70,16.51,;10.88,15.87,;10.12,15.32,;9.43,14.85,;8.79,14.48,;8.23,14.17,;7.71,13.96,;7.27,13.84,;6.88,13.79,;6.38,13.84,;5.93,14.00,;5.49,14.24,;5.09,14.59,;4.74,14.99,;4.50,15.42,;4.35,15.88,;4.31,16.36,;4.36,16.74,;4.55,17.20,;4.85,17.73,;5.28,18.36,;5.83,19.06,;6.50,19.84,;7.28,20.71,;8.19,21.66,;8.87,22.34,;9.62,23.13,;10.45,24.00,;11.35,24.96,;12.32,26.02,;13.37,27.15,;14.50,28.39,;15.69,29.72,;17.34,31.86,;18.88,34.11,;20.33,36.45,;21.69,38.91,;22.94,41.47,;24.07,44.14,;25.12,46.92,;26.06,49.80,;23.53,50.59,;20.90,51.15,;18.19,51.50,;15.38,51.61,;13.30,51.56,;11.21,51.41,;9.08,51.16,;6.93,50.81,;6.99,50.35,;7.00,49.74,;6.94,48.72,;6.76,47.81,;6.47,47.06,;6.06,46.44,;5.54,45.96,;4.89,45.63,;4.12,45.41,;3.24,45.35,;2.58,45.44,;2.01,45.69,;1.53,46.13,;1.14,46.74,;0.83,47.53,;0.61,48.49,;0.48,49.63,;0.44,50.94,;0.47,51.70,;0.59,52.39,;0.78,53.01,;1.05,53.58,;1.40,54.07,;1.81,54.51,;2.31,54.88,;2.88,55.18,;4.11,55.67,;5.42,56.08,;6.85,56.44,;8.38,56.73,;10.01,56.96,;11.75,57.12,;13.58,57.22,;15.51,57.25,;17.62,57.23,;19.60,57.18,;21.43,57.08,;23.14,56.93,;24.70,56.75,;26.13,56.53,;27.42,56.26,;28.57,55.96,;29.60,55.61,;30.48,55.22,;31.23,54.80,;31.84,54.33,;32.32,53.83,;32.67,53.28,;32.87,52.69,;32.94,52.06,129;";
char l8[] = "30.65,45.87,0;30.56,44.50,;30.29,43.21,;29.83,42.03,;29.21,40.94,;28.39,39.93,;27.40,39.02,;26.23,38.20,;24.88,37.48,;26.19,36.64,;27.33,35.71,;28.29,34.73,;29.08,33.66,;29.70,32.51,;30.15,31.29,;30.40,30.00,;30.49,28.63,;30.42,27.21,;30.23,25.86,;29.91,24.58,;29.46,23.37,;28.88,22.22,;28.17,21.15,;27.33,20.13,;26.37,19.20,;25.27,18.31,;24.08,17.54,;22.80,16.89,;21.45,16.36,;20.00,15.95,;18.47,15.65,;16.85,15.47,;15.15,15.41,;13.65,15.47,;12.23,15.63,;10.89,15.88,;9.62,16.25,;8.43,16.72,;7.30,17.30,;6.25,17.98,;5.27,18.77,;4.39,19.64,;3.62,20.57,;2.97,21.57,;2.44,22.63,;2.03,23.75,;1.73,24.94,;1.55,26.18,;1.50,27.50,;1.60,29.19,;1.88,30.76,;2.37,32.21,;3.04,33.54,;3.90,34.75,;4.96,35.85,;6.19,36.83,;7.63,37.69,;6.63,38.62,;5.75,39.60,;5.01,40.64,;4.41,41.74,;3.94,42.90,;3.60,44.13,;3.40,45.41,;3.34,46.75,;3.39,47.94,;3.57,49.06,;3.86,50.16,;4.26,51.18,;4.79,52.18,;5.44,53.11,;6.19,54.01,;7.06,54.84,;8.04,55.61,;9.07,56.27,;10.18,56.83,;11.35,57.29,;12.58,57.65,;13.88,57.90,;15.26,58.05,;16.70,58.10,;18.20,58.05,;19.63,57.89,;21.00,57.62,;22.30,57.24,;23.52,56.76,;24.67,56.18,;25.75,55.49,;26.75,54.69,;27.67,53.81,;28.46,52.86,;29.13,51.84,;29.68,50.77,;30.10,49.64,;30.40,48.45,;30.59,47.19,;30.65,45.87,129;24.21,45.50,0;24.07,47.09,;23.68,48.50,;23.02,49.77,;22.10,50.87,;20.97,51.76,;19.68,52.40,;18.24,52.79,;16.64,52.91,;15.15,52.80,;13.81,52.46,;12.61,51.90,;11.58,51.10,;10.75,50.12,;10.16,49.01,;9.80,47.75,;9.68,46.35,;9.80,44.94,;10.18,43.66,;10.80,42.50,;11.67,41.47,;12.75,40.64,;13.95,40.03,;15.29,39.68,;16.76,39.55,;18.37,39.65,;19.81,39.96,;21.08,40.46,;22.18,41.17,;23.06,42.04,;23.69,43.06,;24.08,44.22,;24.21,45.50,129;23.87,28.05,0;23.84,28.81,;23.74,29.53,;23.57,30.22,;23.33,30.87,;23.02,31.48,;22.66,32.06,;22.21,32.61,;21.71,33.11,;20.54,33.96,;19.17,34.58,;17.65,34.94,;15.94,35.07,;14.16,34.94,;12.59,34.56,;11.19,33.94,;9.99,33.07,;9.46,32.54,;9.02,31.99,;8.63,31.38,;8.31,30.75,;8.07,30.07,;7.90,29.35,;7.79,28.59,;7.76,27.80,;7.88,26.28,;8.26,24.92,;8.88,23.73,;9.77,22.70,;10.86,21.88,;12.13,21.28,;13.58,20.94,;15.20,20.81,;17.13,20.94,;18.84,21.31,;20.32,21.93,;21.61,22.79,;22.13,23.30,;22.60,23.86,;22.99,24.46,;23.30,25.10,;23.55,25.78,;23.73,26.50,;23.84,27.25,;23.87,28.05,129;";
char l9[] = "30.37,41.69,0;30.30,39.68,;30.12,37.70,;29.81,35.77,;28.81,32.01,;28.13,30.19,;27.31,28.40,;26.37,26.66,;25.31,24.94,;24.13,23.28,;22.81,21.64,;21.37,20.04,;19.81,18.49,;18.13,16.96,;16.32,15.49,;14.38,14.04,;13.45,13.40,;12.56,12.86,;11.70,12.40,;10.87,12.02,;10.09,11.73,;9.33,11.52,;8.62,11.40,;7.94,11.35,;7.27,11.40,;6.69,11.52,;6.19,11.72,;5.78,12.01,;5.47,12.38,;5.25,12.82,;5.11,13.36,;5.07,13.97,;5.13,14.48,;5.35,14.97,;5.69,15.45,;6.17,15.93,;6.31,16.04,;6.51,16.18,;6.77,16.38,;7.51,16.91,;7.98,17.24,;8.53,17.61,;9.13,18.04,;10.54,19.02,;11.84,19.97,;13.04,20.86,;14.13,21.71,;15.12,22.52,;16.02,23.29,;16.81,24.01,;17.49,24.69,;18.39,25.69,;19.23,26.74,;20.00,27.83,;20.70,28.96,;21.34,30.15,;21.91,31.36,;22.40,32.63,;22.83,33.94,;21.95,33.25,;21.04,32.65,;20.07,32.13,;19.06,31.72,;18.00,31.38,;16.90,31.16,;15.76,31.02,;14.56,30.97,;13.47,31.02,;12.41,31.13,;11.38,31.33,;10.40,31.62,;9.44,31.98,;8.53,32.42,;7.66,32.94,;6.81,33.54,;5.60,34.59,;4.56,35.71,;3.68,36.92,;2.97,38.20,;2.41,39.56,;2.01,41.00,;1.76,42.53,;1.69,44.13,;1.75,45.59,;1.94,47.00,;2.25,48.35,;2.70,49.63,;3.27,50.87,;3.96,52.03,;4.78,53.15,;5.73,54.20,;6.77,55.16,;7.88,55.98,;9.05,56.69,;10.29,57.27,;11.58,57.71,;12.95,58.04,;14.38,58.23,;15.87,58.29,;17.50,58.23,;19.04,58.01,;20.50,57.67,;21.86,57.18,;23.15,56.56,;24.34,55.79,;25.45,54.90,;26.46,53.86,;27.38,52.70,;28.17,51.44,;28.85,50.08,;29.40,48.60,;29.82,47.03,;30.12,45.36,;30.31,43.57,;30.37,41.69,129;23.87,44.50,0;23.84,45.39,;23.73,46.24,;23.56,47.05,;23.31,47.82,;22.99,48.55,;22.61,49.23,;22.15,49.88,;21.63,50.48,;21.05,51.03,;20.41,51.50,;19.74,51.89,;19.03,52.21,;18.26,52.47,;17.46,52.65,;16.60,52.76,;15.69,52.79,;14.88,52.76,;14.10,52.63,;13.36,52.46,;12.66,52.19,;11.98,51.84,;11.34,51.43,;10.75,50.94,;10.18,50.37,;9.67,49.74,;9.23,49.07,;8.85,48.37,;8.55,47.62,;8.30,46.84,;8.14,46.02,;8.04,45.16,;8.00,44.25,;8.04,43.44,;8.15,42.66,;8.33,41.90,;8.58,41.17,;8.91,40.47,;9.30,39.80,;9.77,39.15,;10.30,38.53,;10.89,37.96,;11.50,37.47,;12.12,37.05,;12.77,36.71,;13.44,36.44,;14.13,36.26,;14.84,36.14,;15.57,36.10,;16.38,36.14,;17.17,36.26,;17.94,36.45,;18.67,36.73,;19.38,37.08,;20.07,37.51,;20.74,38.01,;21.37,38.60,;21.95,39.24,;22.47,39.91,;22.89,40.60,;23.25,41.33,;23.52,42.08,;23.72,42.86,;23.83,43.66,;23.87,44.50,129;";
char ld[] = "12.52,44.19,0;12.43,43.35,;12.17,42.56,;11.72,41.83,;11.09,41.15,;10.36,40.58,;9.55,40.18,;8.68,39.93,;7.76,39.85,;6.81,39.93,;5.95,40.18,;5.17,40.58,;4.48,41.15,;3.91,41.85,;3.50,42.65,;3.26,43.55,;3.18,44.55,;3.26,45.46,;3.50,46.27,;3.91,46.99,;4.48,47.62,;5.18,48.14,;5.99,48.50,;6.91,48.72,;7.94,48.79,;8.87,48.72,;9.73,48.46,;10.51,48.05,;11.21,47.47,;11.77,46.76,;12.19,45.98,;12.44,45.12,;12.52,44.19,129;14.02,24.81,0;13.92,23.95,;13.64,23.14,;13.16,22.38,;12.50,21.66,;11.72,21.06,;10.87,20.64,;9.97,20.38,;9.01,20.29,;8.02,20.38,;7.12,20.64,;6.29,21.07,;5.55,21.69,;4.92,22.41,;4.49,23.21,;4.22,24.10,;4.13,25.06,;4.21,25.98,;4.46,26.82,;4.88,27.58,;5.46,28.26,;6.16,28.82,;6.96,29.22,;7.85,29.46,;8.83,29.54,;9.96,29.46,;10.96,29.22,;11.85,28.83,;12.61,28.27,;13.23,27.58,;13.67,26.77,;13.93,25.85,;14.02,24.81,129;";

char *gAct;

// time-led
uint8_t oldSecs = 0;
unsigned long waitMillis;
int maxWifi = 10;
int wifiCnt = 0;

long loopCnt = 0;

void setup()
{
	Serial.begin(115200);
	Serial.print(" inner=");
	Serial.print(iAngle);
	Serial.print(" outer=");
	Serial.println(oAngle);

	pinMode(HALLSENSOR_PIN, INPUT);

	// Neopixel
	Serial.println("Initializing Neopixel...");
	if (ws2812_init(DATA_PIN, LED_WS2812B)) {
		Serial.println("Neopixel: Init FAILURE: halting");
		while (true) {};
	}
	pixels = (rgbVal*)malloc(sizeof(rgbVal) * NUM_PIXELS);
	displayOff();
	Serial.println("Init Neopixel complete");
	SetupLed(0, false);
	SetupLed(1, false);

	// RTC
	SetupLed(2, true);
	if (!rtc.begin()) {
		Serial.println("Couldn't find RTC");
		// while (1);
	}
	SetupLed(2, false);
	SetupLed(3, true);

	// July 10, 2017 at 16:38:00 you would call:
	// rtc.adjust(DateTime(2017, 7, 10, 16, 38, 0));
	// Compile-Time
	// rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

	// Servo
	ledcSetup(1, 30, 16); // channel 1, 50 Hz, 16-bit depth
	ledcAttachPin(18, 1);   // GPIO 18 on channel 1
	SetupLed(3, false);

	// Startcoordinates
	SetupLed(4, true);
	innerOuterAngles(xRest, yRest);

	mNow = rtc.now();
	sTime = "";
	sTime += getStringHMS(mNow.hour()) + ":";
	sTime += getStringHMS(mNow.minute());

	Serial.println(sTime);

	inactive = true;
	stop = false;

	// Servo
	ledcWrite(1, 3600);
	delay(100);
	SetupLed(4, false);
	SetupLed(5, true);
	Serial.print("WiFi Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while ((WiFi.status() != WL_CONNECTED) && wifiCnt < maxWifi) {
		delay(500);
		wifiCnt++;
		Serial.print(".");
	}

	Serial.println("");
	if (WiFi.localIP() == zeroAdr) {
		validIpAdr = false;
		Serial.println("WiFi NOT connected");
	}
	else {
		validIpAdr = true;
		Serial.println("WiFi connected");
		Serial.println("IP address: ");
		Serial.println(WiFi.localIP());
		SetupLed(5, false);
	}

	SetupLed(6, true);
	if (validIpAdr) {
		int trys = 0;
		while (!getTimeAndDate() && trys < 10) {
			trys++;
		}
		if (trys < 10) {
			Serial.println("Timeserver connected");
			rtc.adjust(DateTime(year(), month(), day(), hour(), minute(), second()));
			SetupLed(6, false);
		}
		else
			Serial.println("Timeserver NOT connected");
	}

	// Steppers
	SetupLed(7, true);
	Serial.println("Stepper");
	innerStepper.setMicrostep(32);
	outerStepper.setMicrostep(32);
	innerStepper.setRPM(35);
	outerStepper.setRPM(55);
	chainStepper.setRPM(800);
	SetupLed(7, false);

	chainStepper.disable();
	innerStepper.disable();
	outerStepper.disable();

	Serial.print(" inner=");
	Serial.print(iAngle);
	Serial.print(" outer=");
	Serial.println(oAngle);

	delay(2000);
	displayOff();
}

void loop()
{	
	testNeopixel(0);
	if (!inactive) {
		while (!stop)
		{			
			testNeopixel(1);
			getNextPoint();
			points++;
			lp = lpAct;
			if (lp.upDown == -99)
			{
				stop = true;
				lp.upDown = 129;
			}
			lp.x += xAdd;
			lp.y += yAdd;
			lp.x = lp.x * xFactor;
			lp.y = lp.y * yFactor;
			iAngleOld = iAngle;
			oAngleOld = oAngle;
			innerOuterAngles(lp.x, lp.y);
			innerSteps = calcSteps(iAngle, iAngleOld, true);
			outerSteps = calcSteps(oAngle, oAngleOld, false);
			move(innerSteps, outerSteps);
			if (lp.upDown == 129) {
				ledcWrite(1, 3600);
				delay(400);
			}
			else if (lp.upDown == 0) {
				ledcWrite(1, 5200); // down
				delay(100);
			}
		}
		if (stop) {
			xAdd += maxX + 4;	
			maxX = 0;
			loadLetter(sTime[actTimeLetter]);
			Serial.print(" .");
			actTimeLetter++;
			stop = false;
			points = 0;
		}
		if (actTimeLetter > 5) {
			inactive = true;
			xAdd = 0;
			firstMove = true;
			stop = true;
			ledcWrite(1, 3600);
			iAngleOld = iAngle;
			oAngleOld = oAngle;
			innerOuterAngles(xRest, yRest);
			innerSteps = calcSteps(iAngle, iAngleOld, true);
			outerSteps = calcSteps(oAngle, oAngleOld, false);
			move(innerSteps, outerSteps);
			SearchHome();
			Serial.print(" FERTIG -> I:");
			Serial.print(restInner);
			Serial.print(" O:");
			Serial.println(restOuter);

			// Correction of rounding errors
			innerStepper.move(roundl(restInner));
			outerStepper.move(roundl(restOuter));
			restInner = 0;
			restOuter = 0;
			// End Correction of rounding errors

			outerStepper.disable();
			innerStepper.disable();
			innerOuterAngles(xRest, yRest); // reset Point
			delay(10);
			Serial.print(" inner=");
			Serial.print(iAngle);
			Serial.print(" outer=");
			Serial.println(oAngle);
		}		
		delay(10);		
	}	
	else {
		mNow = rtc.now();
		if (mNow.second() == 40) {		
			if (validIpAdr) {
				if (getTimeAndDate()) {
					rtc.adjust(DateTime(year(), month(), day(), hour(), minute(), second()));
				}
			}
			moveCleaner(3000);
			actTimeLetter = 0;
			mNow = rtc.now();
			TimeSpan ts = TimeSpan(17);
			mNow = mNow.operator+ (ts);
			sTime = "";
			sTime += getStringHMS(mNow.hour()) + ":";
			sTime += getStringHMS(mNow.minute());
			Serial.print(sTime);			
			outerStepper.enable();
			innerStepper.enable();
			inactive = false;
			stop = true;
		}
	}
}

void SearchHalf(bool outer) {
	int actStep = 0;
	int actDir = -1;
	int half = 0;
	bool doMove = true;
	int cnt = 0;
	int on = 0;
	int off = 0;	
	Serial.println("");
	// innerStepper.enable();
	if (outer) {
		outerStepper.enable();
		delay(25);
		outerStepper.move(SEARCHSTEPS / 2 * actDir);
		Serial.print("Search half outer..");
	}
	else {
		innerStepper.enable();
		delay(55);
		innerStepper.move(SEARCHSTEPS / 2 * actDir);
		Serial.print("Search half inner..");
	}

	Serial.println("nach 1. move");
	actDir *= -1;
	do {
		if (outer)
			outerStepper.move(SINGLESTEP * actDir);
		else 
			innerStepper.move(SINGLESTEP * actDir);
		actStep += SINGLESTEP;
		sensorValue = digitalRead(HALLSENSOR_PIN);
		if (sensorValue == LOW && on == 0) {
			on = actStep;
			Serial.println(" LOW !!!");
		}
		if (sensorValue == HIGH && on > 0) {
			off = actStep;
			doMove = false;
		}
		delay(20);
	} while (doMove && (actStep <= SEARCHSTEPS));
	actDir *= -1;
	half = (off - on) / 2;
	if (outer)
		outerStepper.move(half * actDir);
	else
		innerStepper.move(half * actDir);
	Serial.print("Off:");
	Serial.print(off);
	Serial.print("  On:");
	Serial.print(on);
	Serial.print("  Half:");
	Serial.println(half);
	Serial.println("Ende SearchHalf");
}

void SearchHome() {
	SearchHalf(true);
	SearchHalf(false);
	SearchHalf(true);
	SearchHalf(false);
}

void testNeopixel(int type) {

	if (waitMillis <= millis()) {
		pixels[0] = makeRGBVal(0, 0, 0);
		ws2812_setColors(1, pixels);
	}

	// Timesignal
	mNow = rtc.now();
	if (oldSecs != mNow.second()) {
		oldSecs = mNow.second();
		// Seconds , green
		waitMillis = millis() + 80;
		if (type == 0)
			pixels[0] = makeRGBVal(50, 30, 10);
		else if (type == 1)
			pixels[0] = makeRGBVal(10, 30, 50);
		else if (type == 2)
			pixels[0] = makeRGBVal(0, 0, 50);
		if (mNow.second() > 10) 
			pixels[1] = makeRGBVal(30, 0, 0);
		if (mNow.second() > 20) 
			pixels[2] = makeRGBVal(30, 0, 0);
		if (mNow.second() > 30) 
			pixels[3] = makeRGBVal(30, 0, 0);
		if (mNow.second() > 40) 
			pixels[4] = makeRGBVal(30, 0, 0);		
		else if (mNow.second() < 4)
			displayOff();

		ws2812_setColors(5, pixels);
	}
}

// Setup: show with led
void SetupLed(int iLed, bool test) {
	if (!test)
		pixels[iLed] = makeRGBVal(0, 30, 0);
	else 
		pixels[iLed] = makeRGBVal(30, 0, 0);
    ws2812_setColors(8, pixels);
}

void moveCleaner(int steps) {
	int done = 0;
	int lStep = 25;
	chainStepper.enable();

	while (done < steps) {
		chainStepper.move(-1 * lStep);
		testNeopixel(2);
		done += lStep;			
	}
	delay(50);
	done = 0;
	while (done < steps) {
		chainStepper.move(lStep);
		// delay(500);
		testNeopixel(2);
		done += lStep;
	}
	chainStepper.disable();
}

String getStringHMS(int val) {
	String tt = "";
	if (val < 10)
		tt = "0";
	tt += String(val);
	return tt;
}

void move(float _innerSteps, float _outerSteps) {	

	float actFact=0, fact, oCorr=0;
	int iCnt = 0, oCnt = 0;
	float corrSteps = _innerSteps * CORR_FACT_INV;
	_outerSteps -= corrSteps;
	if (_innerSteps < 0) {
		innerDir = -1;
		_innerSteps = -1 *_innerSteps;
	}
	else
		innerDir = 1;
	if (_outerSteps < 0) {
		outerDir = -1;
		_outerSteps = -1 * _outerSteps;
	}
	else
		outerDir = 1;

	if (_innerSteps >= _outerSteps) {		
		fact = _innerSteps / _outerSteps;	
		
		actFact = fact;		
		iCnt++;
		for (int ii=1; ii <= _innerSteps; ii++) {			
			innerStepper.move(1 * innerDir);
			//delay(1);
			if (ii >= actFact) {
				outerStepper.move(1 * outerDir);
				oCnt++;
				actFact += fact;
			}
		}		
	}
	else 	{
		fact = _outerSteps / _innerSteps;
		actFact = fact;
		oCnt++;
		for (int ii = 1; ii <= _outerSteps; ii++) {
			outerStepper.move(1 * outerDir);
			//delay(1);
			if (ii >= actFact) {
				innerStepper.move(1 * innerDir);
				iCnt++;
				actFact += fact;
			}
		}
	}	
}

int calcSteps(float angle, float oldAngle, bool isInner) {
	int diffSteps;
	float tempF = 0;
	float diffAngle = angle - oldAngle;
	tempF = diffAngle / PI * STEPS_PER_PI;
	diffSteps = roundl(tempF);
	if (diffAngle > 0)
		diffAngle = diffAngle * -1;	
	if (isInner)
		restInner += (tempF - diffSteps);
	else
		restOuter += (tempF - diffSteps);
	return diffSteps;
	
}

void innerOuterAngles(float px, float py) {
	double xx, yy, zz, alpha1, alpha2;
	if (px <= zPoint_x)
		xx = zPoint_x - px;
	else
		xx = px - zPoint_x;
	yy = py + abs(zPoint_y);
	zz = sqrt(sq(xx) + sq(yy));
	// outer Angle
	oAngle = acos((sq(lInner) + sq(lOuter) - sq(zz)) / (2 * lInner * lOuter));
	// inner Angle
	alpha1 = acos((sq(zz) + sq(lInner) - sq(lOuter)) / (2 * lInner * zz));
	alpha2 = asin(xx / zz);
	if (px < zPoint_x)
		iAngle = PI - alpha1 - alpha2;
	else
		iAngle = PI + alpha2 - alpha1;
}

void loadLetter(char letter) {
	currentPos = 0;
	switch (letter)
	{
	case '0':
		gAct = l0;
		maxChars = sizeof(l0);
		break;
	case '1':
		gAct = l1;
		maxChars = sizeof(l1);
		break;
	case '2':
		gAct = l2;
		maxChars = sizeof(l2);
		break;
	case '3':
		gAct = l3;
		maxChars = sizeof(l3);
		break;
	case '4':
		gAct = l4;
		maxChars = sizeof(l4);
		break;
	case '5':
		gAct = l5;
		maxChars = sizeof(l5);
		break;
	case '6':
		gAct = l6;
		maxChars = sizeof(l6);
		break;
	case '7':
		gAct = l7;
		maxChars = sizeof(l7);
		break;
	case '8':
		gAct = l8;
		maxChars = sizeof(l8);
		break;
	case '9':
		gAct = l9;
		maxChars = sizeof(l9);
		break;
	case ':':
		gAct = ld;
		maxChars = sizeof(ld);
		break;
	}
}

void getNextPoint() {
	String xs = "", ys = "", ts = "";
	String buff = "";
	bool stop = false;
	while (!stop) {		
		if (gAct[currentPos] == ',')
			stop = true;
		else
			xs += gAct[currentPos];
		currentPos++;
	}
	lpAct.x = xs.toFloat();
	// x read
	stop = false;
	while (!stop)
	{
		if (gAct[currentPos] == ',')
			stop = true;
		else
			ys += gAct[currentPos];
		currentPos++;
	}
	lpAct.y = ys.toFloat();
	// y read
	stop = false;
	int l = sizeof(gAct) - 1;
	while (!stop && currentPos < maxChars)
	{
		if (gAct[currentPos] == ';')
			stop = true;
		else
			ts += gAct[currentPos];
		currentPos++;
	}
	if (ts == "")
		ts = "-1";
	lpAct.upDown = ts.toInt();
	if (lpAct.upDown == 129 && currentPos >= (maxChars - 2))
		lpAct.upDown = -99;
	if (lpAct.x > maxX)
		maxX = lpAct.x;
}

// Internet-Time related

// Do not alter this function, it is used by the system
int getTimeAndDate() {
	int flag = 0;
	Udp.begin(localPort);
	sendNTPpacket(timeServer);
	delay(1000);
	if (Udp.parsePacket()) {
		Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read the packet into the buffer
		unsigned long highWord, lowWord, epoch;
		highWord = word(packetBuffer[40], packetBuffer[41]);
		lowWord = word(packetBuffer[42], packetBuffer[43]);
		epoch = highWord << 16 | lowWord;
		epoch = epoch - 2208988800 + timeZoneOffset;
		flag = 1;
		rtc.adjust(epoch);
		setTime(epoch);
		ntpLastUpdate = now();
	}
	return flag;
}

// Do not alter this function, it is used by the system
unsigned long sendNTPpacket(IPAddress& address)
{
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	packetBuffer[0] = 0b11100011;
	packetBuffer[1] = 0;
	packetBuffer[2] = 6;
	packetBuffer[3] = 0xEC;
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;
	Udp.beginPacket(address, 123);
	Udp.write(packetBuffer, NTP_PACKET_SIZE);
	Udp.endPacket();
}

// Neopixel related
void displayOff() {
	for (int i = 0; i < NUM_PIXELS; i++) {
		pixels[i] = makeRGBVal(0, 0, 0);
	}
	ws2812_setColors(NUM_PIXELS, pixels);
}

/* 
// Quadrat
Serial.println("Start Start Start Start Start Start");
for (int ii = 0; ii < pHeight; ii++) {
iAngleOld = iAngle;
oAngleOld = oAngle;
innerOuterAngles(0, ii);
iAngleDeg = RAD_TO_DEG * iAngle;
oAngleDeg = RAD_TO_DEG * oAngle;
innerSteps = calcSteps(iAngle, iAngleOld);
outerSteps = calcSteps(oAngle, oAngleOld);
//Serial.print(ii);
//Serial.print("  i:");
//Serial.print(innerSteps);
//Serial.print("  o:");
//Serial.println(outerSteps);
move(innerSteps, outerSteps, ii);
delay(1);
}
delay(1000);
Serial.println("");
for (int ii = 0; ii < pWidth; ii++) {
iAngleOld = iAngle;
oAngleOld = oAngle;
innerOuterAngles(ii, pHeight);
iAngleDeg = RAD_TO_DEG * iAngle;
oAngleDeg = RAD_TO_DEG * oAngle;
innerSteps = calcSteps(iAngle, iAngleOld);
outerSteps = calcSteps(oAngle, oAngleOld);
move(innerSteps, outerSteps,ii);
delay(1);
}
delay(1000);
Serial.println("");
for (int ii = pHeight; ii > 0; ii--) {
iAngleOld = iAngle;
oAngleOld = oAngle;
innerOuterAngles(pWidth, ii);
iAngleDeg = RAD_TO_DEG * iAngle;
oAngleDeg = RAD_TO_DEG * oAngle;
innerSteps = calcSteps(iAngle, iAngleOld);
outerSteps = calcSteps(oAngle, oAngleOld);
move(innerSteps, outerSteps, ii);
delay(1);
}
delay(1000);
for (int ii = pWidth; ii > 0; ii--) {
iAngleOld = iAngle;
oAngleOld = oAngle;
innerOuterAngles(ii, 0);
iAngleDeg = RAD_TO_DEG * iAngle;
oAngleDeg = RAD_TO_DEG * oAngle;
innerSteps = calcSteps(iAngle, iAngleOld);
outerSteps = calcSteps(oAngle, oAngleOld);
move(innerSteps, outerSteps, ii);
delay(1);
}



// Number
// loadLetter('2');

//stop = false;



*/