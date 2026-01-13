#define BLYNK_TEMPLATE_ID "TMPL2TavLRlHN"
#define BLYNK_TEMPLATE_NAME "Soil Moisture Sensor" 
#define BLYNK_AUTH_TOKEN "FeQkgNemCAwy7DBmlcGKCRiDXOrcDOeq"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

char ssid[] = "iPhone";
char pass[] = "pppppppp"; 

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define SOIL_PIN 34
#define PUMP_PIN 26
#define BUZZER_PIN 27
#define LED_PIN 25


int dryThreshold = 30;  
int wetThreshold = 60;   

const int SENSOR_DRY = 3800;   
const int SENSOR_WET = 1800;  

bool pumpStatus = false;
bool autoMode = true;    
bool manualPumpOverride = false;

unsigned long lastBuzzerTime = 0;
bool buzzerState = false;
const unsigned long BUZZER_ON_TIME  = 200;
const unsigned long BUZZER_OFF_TIME = 300;
const int BUZZER_FREQUENCY = 2000;

unsigned long lastReadTime = 0;
const unsigned long READ_INTERVAL = 1000;

BlynkTimer timer;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== ESP32 Soil Monitor Starting ===");
  
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);
  
  Serial.println("Pins initialized");
  
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Soil Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Connecting WiFi");
  
  Serial.println("LCD initialized");
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, pass);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(0, 1);
    lcd.print("Attempt: ");
    lcd.print(attempts + 1);
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(2000);
    
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
    
    if (Blynk.connected()) {
      Serial.println("Blynk Connected!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Blynk Connected!");
      delay(1500);
    } else {
      Serial.println("Blynk connection failed!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Blynk Failed");
    }
  } else {
    Serial.println("\nWiFi Connection FAILED!");
    Serial.print("WiFi Status: ");
    Serial.println(WiFi.status());
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi FAILED!");
    lcd.setCursor(0, 1);
    lcd.print("Check hotspot");
  }
  
  lcd.clear();
  
  timer.setInterval(2000L, sendSensorData);
  
  Serial.println("=== Setup Complete ===\n");
  Serial.print("Initial Dry Threshold: ");
  Serial.println(dryThreshold);
  Serial.print("Initial Wet Threshold: ");
  Serial.println(wetThreshold);
}

void loop() {
  Blynk.run();
  timer.run();
  
  unsigned long now = millis();
  
  if (now - lastReadTime >= READ_INTERVAL) {
    lastReadTime = now;
    
    int rawValue = analogRead(SOIL_PIN);
    int moisturePercent = map(rawValue, SENSOR_DRY, SENSOR_WET, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100);
    
    Serial.print("Raw: ");
    Serial.print(rawValue);
    Serial.print(" | Moisture: ");
    Serial.print(moisturePercent);
    Serial.print("% | Thresholds [");
    Serial.print(dryThreshold);
    Serial.print("-");
    Serial.print(wetThreshold);
    Serial.print("] | WiFi: ");
    Serial.print(WiFi.status() == WL_CONNECTED ? "OK" : "LOST");
    Serial.print(" | Blynk: ");
    Serial.println(Blynk.connected() ? "Connected" : "Disconnected");
    
    if (autoMode) {
      controlPump(moisturePercent);
    }
    
    updateLCD(moisturePercent);
  }
  
  handleBuzzer();
}

void controlPump(int moisture) {
  if (moisture < dryThreshold && !pumpStatus) {
    pumpStatus = true;
    digitalWrite(PUMP_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    Serial.println(">>> PUMP ON - Soil is DRY");
    Blynk.virtualWrite(V1, 1);
    Blynk.virtualWrite(V4, 1); 
    Blynk.logEvent("soil_dry", String("Soil is dry! Moisture: ") + String(moisture) + "%");
  }
  else if (moisture > wetThreshold && pumpStatus) {
    pumpStatus = false;
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
    buzzerState = false;
    Serial.println(">>> PUMP OFF - Soil is WET");
    Blynk.virtualWrite(V1, 0); 
    Blynk.virtualWrite(V4, 0);
  }
}

void updateLCD(int moisture) {
  lcd.setCursor(0, 0);
  lcd.print("M:");
  lcd.print(moisture);
  lcd.print("% [");
  lcd.print(dryThreshold);
  lcd.print("-");
  lcd.print(wetThreshold);
  lcd.print("]");
  
  // Clear rest of line if needed
  int charsWritten = 4 + String(moisture).length() + 3 + 
                     String(dryThreshold).length() + 1 + 
                     String(wetThreshold).length() + 1;
  for (int i = charsWritten; i < 16; i++) {
    lcd.print(" ");
  }
  
  lcd.setCursor(0, 1);
  if (pumpStatus) {
    lcd.print("Status: DRY     ");
  } else {
    lcd.print("Status: NORMAL  ");
  }
}

void handleBuzzer() {
  if (!pumpStatus) return;
  
  unsigned long now = millis();
  
  if (!buzzerState && now - lastBuzzerTime >= BUZZER_OFF_TIME) {
    tone(BUZZER_PIN, BUZZER_FREQUENCY);
    buzzerState = true;
    lastBuzzerTime = now;
  }
  else if (buzzerState && now - lastBuzzerTime >= BUZZER_ON_TIME) {
    noTone(BUZZER_PIN);
    buzzerState = false;
    lastBuzzerTime = now;
  }
}

void sendSensorData() {
  int rawValue = analogRead(SOIL_PIN);
  int moisturePercent = map(rawValue, SENSOR_DRY, SENSOR_WET, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);
  
  Blynk.virtualWrite(V0, moisturePercent); 
  Blynk.virtualWrite(V1, pumpStatus ? 1 : 0);  
  Blynk.virtualWrite(V4, digitalRead(LED_PIN));
  Blynk.virtualWrite(V5, dryThreshold); // Send current dry threshold
  Blynk.virtualWrite(V6, wetThreshold); // Send current wet threshold
}

// Manual pump control (V2)
BLYNK_WRITE(V2) {
  if (!autoMode) { 
    int value = param.asInt();
    manualPumpOverride = value;
    
    Serial.print("Manual pump control: ");
    Serial.println(value ? "ON" : "OFF");
    
    if (value == 1) {
      pumpStatus = true;
      digitalWrite(PUMP_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(V1, 1);
      Blynk.virtualWrite(V4, 1);
    } else {
      pumpStatus = false;
      digitalWrite(PUMP_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
      noTone(BUZZER_PIN);
      buzzerState = false;
      Blynk.virtualWrite(V1, 0);
      Blynk.virtualWrite(V4, 0);
    }
  } else {
    Serial.println("Manual control blocked - Auto mode is ON");
  }
}

// Auto/Manual mode switch (V3)
BLYNK_WRITE(V3) {
  autoMode = param.asInt();
  
  if (autoMode) {
    Serial.println(">>> Auto mode ENABLED");
    manualPumpOverride = false;
    Blynk.virtualWrite(V2, 0);  
  } else {
    Serial.println(">>> Manual mode ENABLED");
    pumpStatus = false;
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V4, 0);
  }
}

// Dry Threshold Slider (V5)
BLYNK_WRITE(V5) {
  int newDryThreshold = param.asInt();
  
  // Validate: dry threshold must be less than wet threshold
  if (newDryThreshold < wetThreshold) {
    dryThreshold = newDryThreshold;
    Serial.print(">>> Dry Threshold updated to: ");
    Serial.println(dryThreshold);
  } else {
    Serial.println("ERROR: Dry threshold must be less than wet threshold!");
    Blynk.virtualWrite(V5, dryThreshold); // Reset to valid value
  }
}

// Wet Threshold Slider (V6)
BLYNK_WRITE(V6) {
  int newWetThreshold = param.asInt();
  
  // Validate: wet threshold must be greater than dry threshold
  if (newWetThreshold > dryThreshold) {
    wetThreshold = newWetThreshold;
    Serial.print(">>> Wet Threshold updated to: ");
    Serial.println(wetThreshold);
  } else {
    Serial.println("ERROR: Wet threshold must be greater than dry threshold!");
    Blynk.virtualWrite(V6, wetThreshold); // Reset to valid value
  }
}

BLYNK_CONNECTED() {
  Serial.println("✓ Blynk connected successfully!");
  Blynk.syncVirtual(V2, V3, V5, V6); // Sync all control values
}