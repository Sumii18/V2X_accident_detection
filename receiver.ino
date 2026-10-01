#include <SPI.h>
#include <RF24.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BUZZER PC14
#define CE_PIN PB0
#define CSN_PIN PB1
#define LED_PIN PC13   // Inbuilt LED (Black Pill)

RF24 radio(CE_PIN, CSN_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

HardwareSerial Serial2(PA3, PA2);

const byte address[6] = "00001";

bool accidentDetected = false;

void setup()
{


  
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LED OFF (active LOW)

  Serial2.begin(9600);
  delay(1000);

  lcd.init();
  lcd.backlight();
  lcd.clear(); // Keep LCD blank initially

  if (!radio.begin())
  {
    Serial2.println("NRF not detected");
    while (1);
  }

  Serial2.println("NRF detected");

  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.setChannel(108);
  radio.setAutoAck(false);
  radio.startListening();
}

void loop()
{ 
  if (!accidentDetected)
  {
    if (radio.available())
    {
      char text[32] = {0};
      radio.read(&text, sizeof(text));

      Serial2.print("Received: ");
      Serial2.println(text);

      // Check for accident message
      if (strcmp(text, "Accident Detected") == 0)
      {
        accidentDetected = true;

        // Display on LCD
        // lcd.clear();
        // lcd.setCursor(0, 0);
        // lcd.print("ACCIDENT ALERT");
        // lcd.setCursor(0, 1);
        // lcd.print("HELP NEEDED!");

         digitalWrite(BUZZER, HIGH);
      delay(2000);
      digitalWrite(BUZZER, LOW);

delay(1000);
       digitalWrite(BUZZER, HIGH);
      delay(2000);
      digitalWrite(BUZZER, LOW);
delay(1000);
       digitalWrite(BUZZER, HIGH);
      delay(2000);
      digitalWrite(BUZZER, LOW);
delay(1000);
       digitalWrite(BUZZER, HIGH);
      delay(2000);
      digitalWrite(BUZZER, LOW);
      }
    }
  }
 

}
