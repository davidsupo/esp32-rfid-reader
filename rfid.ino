#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

// Configuración de pines
#define SDA_PIN 4
#define SCL_PIN 15
#define SS_PIN 21
#define RST_PIN 22
#define LED_PIN 25
#define BUZZER_PIN 26

// Configuración del LCD
LiquidCrystal_PCF8574 lcd(0x27);

// Configuración de red WiFi
const char* ssid = "SSID";
const char* password = "PASSWORD";
const String googleScriptUrl = "SCRIPT_URL";

// Configuración de NTP (Hora actual)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -5 * 3600, 60000);  // UTC-5 (Lima)

// Configuración del lector RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  SPI.begin(18, 19, 23, 21);
  mfrc522.PCD_Init();
  lcd.begin(20, 4);
  lcd.setBacklight(255);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Conectar a WiFi
  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  lcd.setCursor(0, 0);
  lcd.print("WiFi Conectado!   ");
  
  // Iniciar cliente NTP
  timeClient.begin();

  // Inicializar clave RFID
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
}

void loop() {
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  updateLCDLine(2);
  // Actualizar hora NTP y mostrar en LCD
  timeClient.update();
  int horas = timeClient.getHours();
  int minutos = timeClient.getMinutes();
  int segundos = timeClient.getSeconds();

  lcd.setCursor(0, 1);
  lcd.print("Hora: ");
  lcd.printf("%02d:%02d:%02d  ", horas, minutos, segundos);

  // Verificar tarjeta RFID
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String cardData = read_block(5, key);
    if (cardData.length() != 8) {
      updateLCDLine(2, "Lectura incorrecta");
      blinkBuzzer(2, 200);
    } else {
      updateLCDLine(2, "Enviando datos...");
      digitalWrite(LED_PIN, HIGH);
      sendToGoogleSheets(cardData);
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }

  delay(1000); // Actualizar cada segundo
}

// Función para leer un bloque de la tarjeta
String read_block(byte block, MFRC522::MIFARE_Key &key) {
  byte len = 18;
  byte myBuffer[18];
  String response = "";
  MFRC522::StatusCode status;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) return "ERROR";

  status = mfrc522.MIFARE_Read(block, myBuffer, &len);
  if (status != MFRC522::STATUS_OK) return "ERROR";

  response = (char *)myBuffer;
  response = response.substring(0, 16);
  response.trim();

  return response;
}

// Función para enviar datos a Google Sheets
void sendToGoogleSheets(String idTarjeta) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = googleScriptUrl + "?id=" + idTarjeta;

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      updateLCDLine(2, "Bienvenido!      ");
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      updateLCDLine(2, "Error: pasa de nuevo");
      blinkBuzzer(2, 200);
    }

    http.end();
  } else {
    updateLCDLine(2, "No hay conexión WiFi");
  }
}

// Función para limpiar o escribir en una línea específica del LCD
void updateLCDLine(int line, const String &text = "") {
  lcd.setCursor(0, line);
  if (text.isEmpty()) {
    lcd.print("                    "); // Limpia la línea si no se pasa texto
  } else {
    lcd.print(text); // Escribe el texto proporcionado
  }
}

// Función para parpadear el buzzer
void blinkBuzzer(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(delayMs);
    digitalWrite(BUZZER_PIN, LOW);
    delay(delayMs);
  }
}