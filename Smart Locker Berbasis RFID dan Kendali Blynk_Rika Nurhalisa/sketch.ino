#define BLYNK_TEMPLATE_ID "TMPL65tArFyLu"
#define BLYNK_TEMPLATE_NAME "Smart Locker"
#define BLYNK_AUTH_TOKEN "s6ijihtiNHnQNgKdddTc8AIQQ-WFjZKi"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#include <SPI.h>
#include <MFRC522.h>

#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// =====================
// WIFI
// =====================
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// =====================
// RFID
// =====================
#define SS_PIN 5
#define RST_PIN 27 
MFRC522 mfrc522(SS_PIN, RST_PIN);

// =====================
// LCD I2C
// =====================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// =====================
// SERVO
// =====================
Servo lockerServo;
#define SERVO_PIN 13

// =====================
// BUZZER & LED
// =====================
#define BUZZER_PIN 12
#define GREEN_LED 16
#define RED_LED 17

// =====================
// COUNTER
// =====================
int accessCount = 0;

// =====================
// UID
// =====================
String authorizedUID  = "01 02 03 04";
String authorizedUID2 = "11 22 33 44";
String authorizedUID3 = "AA BB CC DD";

// =====================
// BLYNK SYNC (FIX RESET COUNTER)
// =====================
BLYNK_CONNECTED() {
  accessCount = 0;
  Blynk.virtualWrite(V2, 0);
  Blynk.syncVirtual(V0, V1, V3, V4);
}

// =====================

void accessGranted(String nama);
void accessDenied();

void setup() {

  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart Locker");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  // Servo
  lockerServo.attach(SERVO_PIN);
  lockerServo.write(0);

  // RFID
  SPI.begin();
  mfrc522.PCD_Init();

  // Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  delay(1500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan RFID Card");
}

void loop() {

  Blynk.run();

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  String uid = "";

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(mfrc522.uid.uidByte[i], HEX);

    if (i < mfrc522.uid.size - 1) uid += " ";
  }

  uid.toUpperCase();

  Serial.println("UID: " + uid);

  if (uid == authorizedUID) accessGranted("Rika Nurhalisa");
  else if (uid == authorizedUID2) accessGranted("Lisa Cantik");
  else if (uid == authorizedUID3) accessGranted("Jefri Nichole");
  else accessDenied();

  mfrc522.PICC_HaltA();
}

// =====================
// ACCESS GRANTED
// =====================
void accessGranted(String nama) {

  accessCount++;

  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);

  tone(BUZZER_PIN, 1500, 200);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ACCESS GRANTED");
  lcd.setCursor(0, 1);
  lcd.print(nama);

  lockerServo.write(90);

  Blynk.virtualWrite(V0, "OPEN");
  Blynk.virtualWrite(V1, nama);
  Blynk.virtualWrite(V2, accessCount);
  Blynk.virtualWrite(V3, nama + " Login");

  Serial.println("ACCESS GRANTED - " + nama);

  delay(4000);

  lockerServo.write(0);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan RFID Card");

  digitalWrite(GREEN_LED, LOW);
}

// =====================
// ACCESS DENIED
// =====================
void accessDenied() {

  digitalWrite(RED_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);

  tone(BUZZER_PIN, 800, 800);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ACCESS DENIED");
  lcd.setCursor(0, 1);
  lcd.print("Unknown Card");

  Blynk.virtualWrite(V0, "DENIED");
  Blynk.virtualWrite(V1, "UNKNOWN");
  Blynk.virtualWrite(V3, "Unknown Card");
  Blynk.virtualWrite(V4, "SECURITY ALERT!");

  Serial.println("ACCESS DENIED");

  delay(3000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan RFID Card");

  digitalWrite(RED_LED, LOW);
}