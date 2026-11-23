#include <Servo.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Password.h>

#define SERVO_PIN 10
#define BUZZER_PIN 11
#define PIR_PIN 12
#define LED_PIN 13

Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

int motionStatus = 0;
int pirState = 0;
int passState = 0;
bool doorCon = true; //inisialisasi kondisi kunci (true = open/false = locked)
byte defCursor = 5;  //default cursor LCD

//Konfigurasi Keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' },
};
byte rowPins[ROWS] = { 9, 8, 7, 6 };
byte colPins[COLS] = { 5, 4, 3, 2 };

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//Konfigurasi Password
Password pass = Password("911");

byte maxPassLength = 6;
byte currentPassLength = 0;

void setup() {
  Serial.begin(9600);
  pinMode(PIR_PIN, INPUT);
  delay(1000);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  servo.attach(SERVO_PIN);
  servo.write(90);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(6, 0);
  lcd.print("HALO!");
  lcd.setCursor(1, 1);
  lcd.print("SELAMAT DATANG");
  delay(2000);
  lcd.clear();
}

void loop() {
  unsigned long currentTime = millis();

  //indikator PIR sensor
  motionStatus = digitalRead(PIR_PIN);

  if (motionStatus == HIGH) {
    if (pirState == LOW) {
      digitalWrite(LED_PIN, HIGH);
      pirState = 1;
    }
  } else {
    if (pirState == HIGH) {
      digitalWrite(LED_PIN, LOW);
      pirState = 0;
    }
  }

  //konfigurasi password-lcd
  lcd.setCursor(1,0);
  lcd.print("ENTER PASSWORD");

  char key = keypad.getKey();

  if (key != NO_KEY) {
    delay(60);
    
    if (key == 'D') {
      if (doorCon == true) {
        doorLocked();
        doorCon = false;
      } else if (doorCon == false) {
        doorOpen();
        doorCon = true;
      }
    } else if (key == '#') {
      resetPass();
    } else {
      processNumberKey(key);
    }
  }

  //Cetak status ke serial
  Serial.print(currentTime);
  Serial.print(", ");
  Serial.print(pirState);
  Serial.print(", ");
  Serial.println(passState);
  passState = 0;
}

//Fungsi konfigurasi key yang ditekan pada keypad
void processNumberKey(char key) {
  lcd.setCursor(defCursor, 1);
  lcd.print("*");
  defCursor++;

  if (defCursor == 11) {
    defCursor = 5;
  }

  currentPassLength++;
  pass.append(key);

  if (currentPassLength == maxPassLength) {
    doorLocked();
    doorOpen();
  }
}

//Fungsi reset password
void resetPass() {
  pass.reset();
  currentPassLength = 0;
  lcd.clear();
  defCursor = 5;
}

//Fungsi kondisi pintu terbuka
void doorOpen() {
  unsigned long currentTime = millis();

  if (pass.evaluate()) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    servo.write(90);
    delay(100);
    lcd.setCursor(0, 0);
    lcd.print("CORRECT PASSWORD");
    lcd.setCursor(0, 1);
    lcd.print("DOOR OPENED");
    delay(2000);
    lcd.clear();
    defCursor = 5;
    passState = 1;
  } else {
    for(int i = 0; i < 3; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(300);
      digitalWrite(BUZZER_PIN, LOW);
      delay(300);
    }

    lcd.setCursor(0, 0);
    lcd.print("WRONG PASSWORD!");
    delay(1000);
    lcd.clear();
    defCursor = 5;
    passState = 2;
  }

  resetPass();
}

void doorLocked() {
  unsigned long currentTime = millis();

  if (pass.evaluate()) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    servo.write(180);
    delay(100);
    lcd.setCursor(0, 0);
    lcd.print("CORRECT PASSWORD");
    lcd.setCursor(2, 1);
    lcd.print("DOOR LOCKED");
    delay(2000);
    lcd.clear();
    defCursor = 5;
    passState = 1;
  } else {
    for(int i = 0; i < 3; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(300);
      digitalWrite(BUZZER_PIN, LOW);
      delay(300);
    }

    lcd.setCursor(0, 0);
    lcd.print("WRONG PASSWORD!");
    delay(1000);
    lcd.clear();
    defCursor = 5;
    passState = 2;
  }

  resetPass();
}

