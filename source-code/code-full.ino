// Library Wire
#include <Wire.h> 
// Library LCD 20x4
#include <LiquidCrystal_I2C.h>
// Library RTC DS1307
#include <RTClib.h>
// Library DS18B20 (Sensor Suhu)
#include <OneWire.h>
#include <DallasTemperature.h>
// Library Servo
#include <Servo.h>

// Deklarasi objek LCD 20x4
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Deklarasi variabel RTC
DateTime now;
RTC_DS1307 rtc;
void displayRTC(void); 
const int SENSOR_PIN = 8;

// Deklarasi variabel DS18b20
OneWire oneWire(SENSOR_PIN);
DallasTemperature tempSensor(&oneWire);
void displayTemp(void); 
float tempCelsius;

// Deklarasi variabel Turbidity
int sensorPin = A0;
float volt;
float ntu;
void displayTurbidity(void);

// Interval kedipan dalam milidetik untuk Blink Teks
unsigned long previousMillis = 0;
const long interval = 1000; // Interval kedipan dalam milidetik
bool showText = true;

// Interval kedipan dalam milidetik untuk Blink Teks Suhu
unsigned long previousMillisTemp = 0;
const long intervalTemp = 1000; // Interval pembaruan suhu dalam milidetik

// Interval kedipan dalam milidetik untuk Blink Teks Keruh Air
unsigned long previousMillisTurbidity = 0;
const long intervalTurbidity = 1000; // Interval pembaruan turbidity dalam milidetik

// Deklarasi Fungsi untuk Interval diatas 
void displayBlinkTeks(void);
void displayUpdateTemp(void);
void displayUpdateTurbidity(void);

// // Deklarasi variabel dan fungsi Servo
Servo myservo;
// Variabel waktu pemberian pakan
int feedingTimes[3][3] = {
  {7, 0, 0},    // Pemberian pakan pukul 07:00:00
  {12, 0, 0},   // Pemberian pakan pukul 12:00:00
  {18, 0, 0}    // Pemberian pakan pukul 18:00:00
};
// Interval pengecekan waktu setiap 1 detik dengan millis()
unsigned long previousMillisServo = 0;
const long intervalServo = 1000; 
// Variabel untuk kontrol pemberian pakan
bool feedingDone[3] = {false, false, false};  // Menyimpan status pemberian pakan untuk 3 waktu
void timeFeedingMain();
void timeFeedingMillis();

void setup () {
  delay(3000);
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  if (!rtc.begin()) {
    Serial.println("DS1307 RTC Tidak Terbaca");
    while (1);
  }
  if (!rtc.isrunning()) {
    Serial.println("Gagal menjalankan RTC, mengatur ulang waktu!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  tempSensor.begin(); 

  int servoPin = 9;
  myservo.attach(servoPin);
  myservo.write(0);
}

void loop () {
  now = rtc.now();
  displayRTC();
  displayUpdateTemp();
  displayUpdateTurbidity();
  timeFeedingMillis();
}

// Fungsi Blink Text
void displayBlinkText() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Perbarui waktu sebelumnya
    showText = !showText; // Toggle status teks (tampilkan/sembunyikan)
  }
}

// Fungsi Pengganti Delay untuk Suhu
void displayUpdateTemp() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisTemp >= intervalTemp) {
    previousMillisTemp = currentMillis; // Perbarui waktu sebelumnya
    displayTemp();
  }
}

// Fungsi Pengganti Delay untuk Turbidity
void displayUpdateTurbidity() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisTurbidity >= intervalTurbidity) {
    previousMillisTurbidity = currentMillis;
    displayTurbidity();
  }
}
 
// Fungsi Millis untuk Servo
void timeFeedingMillis() {
  unsigned long currentMillis = millis();
  // Cek waktu apakah sudah saatnya memberi pakan
  if (currentMillis - previousMillisServo >= intervalServo) {
    previousMillisServo = currentMillis; // Update previousMillis setiap 1 detik
    timeFeedingMain();  // Cek apakah waktunya memberi pakan
  }
}

// Fungsi DS18B20 Menampilkan Suhu
void displayTemp() {
  tempSensor.requestTemperatures(); 
  tempCelsius = tempSensor.getTempCByIndex(0);
  // Display Serial
  Serial.print("Temperature: ");
  Serial.print(tempCelsius);    
  Serial.print("°C");
  // Display LCD
  lcd.setCursor(0,1);
  lcd.print("Suhu:");
  lcd.print(tempCelsius);
  lcd.print("C");
  // Kondisi Suhu Air
  if (tempCelsius > 30) {
    lcd.setCursor(0,2);
    displayBlinkText();
    if (showText) {
      lcd.print("Suhu Air Panas!");
    } else {
      lcd.print("                    "); 
    }
  } else if (tempCelsius > 28 && tempCelsius < 30) {
    lcd.setCursor(0,2);
    lcd.print("Suhu Air Hangat");
  } else if ( tempCelsius <= 28 && tempCelsius >= 25) {
    lcd.setCursor(0,2);
    lcd.print("Suhu Air Normal");
  } else if (tempCelsius < 25) {
    lcd.setCursor(0,2);
    lcd.print("Suhu Air Dingin");
  } 
}

// Fungsi RTC Menampilkan Waktu dan Tanggal
void displayRTC() {
  lcd.setCursor(0,0);
  if (now.hour() < 10) lcd.print('0');
  lcd.print(now.hour());
  lcd.print(':');
  if (now.minute() < 10) lcd.print('0');
  lcd.print(now.minute());
  lcd.print(':');
  if (now.second() < 10) lcd.print('0');
  lcd.print(now.second());
  lcd.print("  ");
  if (now.day() < 10) lcd.print('0');
  lcd.print(now.day());
  lcd.print('/');
  if (now.month() < 10) lcd.print('0');
  lcd.print(now.month());
  lcd.print('/');
  lcd.print(now.year());
}

// Pembulatan angka
float round_to_dp( float in_value, int decimal_place ) {
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}

// Fungsi Turbidity Menampilkan Kekeruhan Air
void displayTurbidity() {
  volt = 0;
  for(int i=0; i<800; i++) {
    volt += ((float)analogRead(sensorPin)/1023)*5;
  }
  volt = volt/800;
  volt = round_to_dp(volt,1);
  if (volt < 2.5) {
    ntu = 3000;
  } else {
    ntu = -1120.4*(volt*volt)+5742.3*volt-4353.8; 
  }
  if(volt >= 3.5) {
    Serial.print(volt);
    Serial.print(" V");
    lcd.setCursor(0,3);
    lcd.print("Kondisi Air Bersih");
  } else if (volt < 3.5) {
    Serial.print(volt);
    Serial.print(" V");
    lcd.setCursor(0,3);
    displayBlinkText();
    if (showText) {
      lcd.print("Kondisi Air Keruh!");
    } else {
      lcd.print("                    "); 
    }
  }
  Serial.print(ntu);
  Serial.print(" NTU");
}

// Fungsi Menggerakan Servo
void timeFeedingMain() {
   // Periksa apakah waktu sekarang cocok dengan salah satu waktu pemberian pakan
  for (int i = 0; i < 3; i++) {
    if (now.hour() == feedingTimes[i][0] && now.minute() == feedingTimes[i][1] && now.second() == feedingTimes[i][2] && !feedingDone[i]) {
      myservo.write(0);
      Serial.println("Pemberian Pakan Dimulai");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("  Pakan Diberikan!  ");
      // Aktifkan servo untuk memberikan pakan (servo ke 90 derajat)
      myservo.write(90);
      delay(1000);  // Biarkan servo berada di posisi 90 derajat selama 1 detik (memberi pakan)
      // Kembali ke posisi awal (servo ke 0 derajat)
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("   Pakan Selesai!   ");
      myservo.write(0);
      delay(1000);  // Servo kembali ke posisi 0 derajat selama 1 detik
      // Update tampilan pada LCD
      lcd.clear();
      // Tandai bahwa pemberian pakan sudah selesai untuk waktu ini
      feedingDone[i] = true;  
    }
  }

  // Reset ulang pemberian pakan setiap detik baru (untuk setiap waktu pemberian pakan)
  if (now.hour() == 19 && now.minute() == 0 && now.second() == 0) {
    for (int i = 0; i < 3; i++) {
      feedingDone[i] = false;  // Reset pemberian pakan agar siap untuk pemberian berikutnya
    }
  }
}