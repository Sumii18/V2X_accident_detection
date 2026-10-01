#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#include <SPI.h>
#include <RF24.h>

#include <TinyGPS++.h>

#include "config.h"   // phone numbers: copy config.example.h to config.h


#define TRIG_PIN PA0
#define ECHO_PIN PA1

long duration;
float distance;
float distanceThreshold = 7.0;  // cm (adjust)



RF24 radio(PB0, PB1);
const byte address[6] = "00001";

Adafruit_MPU6050 mpu;
TinyGPSPlus gps;

HardwareSerial gpsSerial(PA10, PA9);
HardwareSerial gsmSerial(PA3,PA2);

String phoneNumber = ALERT_PHONE_1;
String phoneNumberr= ALERT_PHONE_2;

float getDistance();
void setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println("GY87 MPU6050 Starting...");

  Wire.setSCL(PB8);
  Wire.setSDA(PB9);
  Wire.begin();

  if (!mpu.begin(0x68))
  {
    Serial.println("MPU6050 not detected!");
    while (1);
  }

  Serial.println("MPU6050 Connected");

  Serial.println("STM32 Starting");

  if (!radio.begin())
  {
    Serial.println("NRF not detected");
    while (1);
  }

                  // radio.openWritingPipe(address);
                  // radio.setPALevel(RF24_PA_LOW);
                  // radio.setDataRate(RF24_1MBPS);
                  // radio.stopListening();


 radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.setChannel(108);
  radio.setAutoAck(false);
  radio.stopListening();

  Serial.println("Transmitter Ready");






// radio.openWritingPipe(address);
// radio.setPALevel(RF24_PA_LOW);
// radio.setDataRate(RF24_1MBPS);
// radio.setChannel(108);
// radio.setAutoAck(false);
// radio.stopListening();
 // Serial.println("Transmitter Ready");
  

  
  gsmSerial.begin(9600);

  delay(3000);

  gsmSerial.println("AT");
  delay(1000);
  gsmSerial.println("AT+CMGF=1");
  delay(1000);


     
  gpsSerial.begin(9600);     // GPS baud rate

  Serial.println("NEO-M8N GPS Test Started...");



pinMode(TRIG_PIN, OUTPUT);
pinMode(ECHO_PIN, INPUT);

}

const float threshold = 15.0;
float latitude = 0;
float longitude = 0;


void GPS(float lat, float lon);
void SMS();
void sendSMS(String message);

void loop()
{


 // Read GPS data
 // ALWAYS decode GPS
while (gpsSerial.available())
{
  gps.encode(gpsSerial.read());
}

// ALWAYS show last known location
if (gps.location.isValid())
{

latitude = gps.location.lat();
longitude = gps.location.lng();
  Serial.print("Latitude: ");
  Serial.println(gps.location.lat(), 6);

  Serial.print("Longitude: ");
  Serial.println(gps.location.lng(), 6);

  Serial.print("Satellites: ");
  Serial.println(gps.satellites.value());

 

  Serial.println("----------------------");
}
else
{
  Serial.println("Waiting for GPS fix...");
}

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;

  float total = sqrt(ax*ax + ay*ay + az*az);

  // Serial.print("AX: ");
  // Serial.print(ax);

  // Serial.print(" AY: ");
  // Serial.print(ay);

  // Serial.print(" AZ: ");
  // Serial.print(az);

  Serial.print(" Total : ");
  Serial.println(total);


float gx = g.gyro.x;
float gy = g.gyro.y;
float gz = g.gyro.z;

gx = gx * 180 / PI;
gy = gy * 180 / PI;
gz = gz * 180 / PI;

Serial.print(" GX: ");
Serial.print(gx);

Serial.print(" GY: ");
Serial.print(gy);

Serial.print(" GZ: ");
Serial.println(gz);



  delay(200);
  distance = getDistance();

// Serial.print("Distance: ");
// Serial.print(distance);
// Serial.println(" cm");

  if(total > threshold || distance < distanceThreshold  || gx>30 || gy>30 || gz> 30)
  {
    const char text[] = "Accident Detected";

    radio.write(text, sizeof(text));

    Serial.println("Sent");

    delay(1000);
    int i=10;
while(i>0)
{
    GPS(latitude, longitude);
    const char text[] = "Accident Detected";

    radio.write(text, sizeof(text));

    Serial.println("Sent");

    delay(2000);
    SMS();
     delay(1000);
  

    radio.write(text, sizeof(text));

    Serial.println("Sent");

  i-=1;
}
     int count=30;
     while(count > 0)
     {


      const char text[] = "Accident Detected";

    radio.write(text, sizeof(text));
   delay(1000);
    Serial.println("Sent");
    count--;
     }

  }
}




 void GPS(float lat,float lon)
{

latitude = gps.location.lat();
longitude = gps.location.lng();
  Serial.print("Latitude: ");
  Serial.println(gps.location.lat(), 6);

  Serial.print("Longitude: ");
  Serial.println(gps.location.lng(), 6);

  Serial.print("Satellites: ");
  Serial.println(gps.satellites.value());



  Serial.println("----------------------");


    String googleLink = "Accident__Happened__\n";
    googleLink += "https://www.google.com/maps?q=";
    googleLink += String(lat, 6);
    googleLink += ",";
    googleLink += String(lon, 6);

    Serial.println(googleLink);

    sendSMS(googleLink);
  
}






void sendSMS(String message)
{
  gsmSerial.println("AT");
  delay(1000);

  gsmSerial.println("AT+CMGF=1");
  delay(1000);

  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(phoneNumber);
  gsmSerial.println("\"");
  delay(1000);

  gsmSerial.print(message);
  delay(500);

  gsmSerial.write(26);   // CTRL+Z
  delay(500);

  Serial.println("SMS link Sent");
}



void SMS()
{


  gsmSerial.println("AT");
  delay(1000);

  gsmSerial.println("AT+CMGF=1");
  delay(1000);

  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(phoneNumberr);
  gsmSerial.println("\"");
  delay(1000);

  gsmSerial.print("ALERT");
  delay(500);

  gsmSerial.write(26);   // CTRL+Z
  delay(500);

  Serial.println("SMS for call Sent");



}


float getDistance()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);

  float dist = duration * 0.034 / 2; // cm
delay(400);
  return dist;
}

//2..
// float getDistance()
// {
//   digitalWrite(TRIG_PIN, LOW);
//   delayMicroseconds(2);
  
//   digitalWrite(TRIG_PIN, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(TRIG_PIN, LOW);

//   long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout

//   if (duration == 0)
//   {
//     Serial.println("No echo");
//     return -1;
//   }

//   float dist = duration * 0.034 / 2;

//   if (dist > 400) return -1; // out of range (HC-SR04 max ~400 cm)

//   return dist;
// }

//2
// float getDistance()
// {
//   float sum = 0;

//   for(int i = 0; i < 5; i++)
//   {
//     digitalWrite(TRIG_PIN, LOW);
//     delayMicroseconds(2);

//     digitalWrite(TRIG_PIN, HIGH);
//     delayMicroseconds(10);
//     digitalWrite(TRIG_PIN, LOW);

//     long duration = pulseIn(ECHO_PIN, HIGH, 30000);

//     if(duration == 0) continue;

//     float dist = duration * 0.034 / 2;
//     sum += dist;

//     delay(50);
//   }

//   return sum / 5.0;
// }


// float getDistance()
// {
//   digitalWrite(TRIG_PIN, LOW);
//   delayMicroseconds(2);

//   digitalWrite(TRIG_PIN, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(TRIG_PIN, LOW);

//   long duration = pulseIn(ECHO_PIN, HIGH, 30000);

//   if (duration == 0)
//   {
//     Serial.println("No echo received!");
//     return -1;
//   }

//   float dist = duration * 0.034 / 2;

//   Serial.print("Distance: ");
//   Serial.println(dist);

//   delay(60);

//   return dist;
// }
