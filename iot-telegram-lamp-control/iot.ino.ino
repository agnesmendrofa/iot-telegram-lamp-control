#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";        
const char* password = "YOUR_WIFI_PASSWORD"; 

// Telegram Bot Token
#define BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"

// Pin Configuration
const int relayPin = D1;   // GPIO5 - Connect to relay IN1
const int ledPin = D4;     // GPIO2 - Built-in LED

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

unsigned long lastTimeBotRan = 0;
const unsigned long botRequestDelay = 1000;  
bool lampState = false;

void relayOn() {
  digitalWrite(relayPin, LOW);   // Active LOW
  digitalWrite(ledPin, LOW);      
  lampState = true;
  Serial.println(">>> Lampu MENYALA");
}

void relayOff() {
  digitalWrite(relayPin, HIGH);   
  digitalWrite(ledPin, HIGH);    
  lampState = false;
  Serial.println(">>> Lampu MATI");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== Mulai Setup ===");
  
  pinMode(relayPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  
  relayOff();
  
  Serial.println("Konfigurasi Pin Relay untuk NodeMCU D1 Mini:");
  Serial.println("  - IN1 (Signal) -> D1 (GPIO5)");
  Serial.println("  - G (Ground) -> GND");
  Serial.println("  - 3V (VCC) -> 3V");
  Serial.println("\nRelay Logic: LOW = ON, HIGH = OFF (ACTIVE LOW)");
  
  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  client.setInsecure();
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] WiFi Terhubung!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\n[ERROR] Gagal terhubung ke WiFi!");
    Serial.println("Periksa SSID dan Password, lalu restart!");
    while(1) { 
      delay(1000);
    }
  }
  
  Serial.println("\n=== Setup Selesai ===");
  Serial.println("Bot siap menerima perintah!\n");
}

void handleNewMessages(int numNewMessages) {
  Serial.println("\n========== PESAN BARU ==========");
  Serial.println("Jumlah pesan: " + String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    Serial.print("Dari: ");
    Serial.println(from_name);
    Serial.print("Chat ID: ");
    Serial.println(chat_id);
    Serial.print("Pesan: ");
    Serial.println(text);

    text.toUpperCase();

    if (text == "RELAY ON" || text == "ON" || text == "/ON") {
      relayOn();
      bot.sendMessage(chat_id, "[OK] Lampu berhasil DINYALAKAN", "");
    }

    else if (text == "RELAY OFF" || text == "OFF" || text == "/OFF") {
      relayOff();
      bot.sendMessage(chat_id, "[OK] Lampu berhasil DIMATIKAN", "");
    }

    else if (text == "STATUS" || text == "/STATUS") {
      String status = "STATUS DEVICE\n\n";
      status += "Lampu: ";
      status += lampState ? "MENYALA [ON]" : "MATI [OFF]";
      status += "\nWiFi: " + String(ssid);
      status += "\nSignal: " + String(WiFi.RSSI()) + " dBm";
      status += "\nIP: " + WiFi.localIP().toString();
      status += "\nUptime: " + String(millis() / 1000) + " detik";
      bot.sendMessage(chat_id, status, "");
    }

    else if (text == "/START" || text == "START" || text == "HELP" || text == "/HELP") {
      String welcome = "Selamat datang di Bot Kontrol Lampu IoT\n\n";
      welcome += "Perintah yang tersedia:\n\n";
      welcome += "ON atau RELAY ON\n";
      welcome += "  Nyalakan lampu\n\n";
      welcome += "OFF atau RELAY OFF\n";
      welcome += "  Matikan lampu\n\n";
      welcome += "STATUS\n";
      welcome += "  Cek status device\n\n";
      welcome += "HELP\n";
      welcome += "  Tampilkan menu ini\n\n";
      welcome += "IP Device: " + WiFi.localIP().toString();
      bot.sendMessage(chat_id, welcome, "");
    }

    else {
      String balas = "Perintah tidak dikenal!\n\n";
      balas += "Coba kirim:\n";
      balas += "ON atau RELAY ON\n";
      balas += "OFF atau RELAY OFF\n";
      balas += "STATUS\n";
      balas += "HELP\n\n";
      balas += "Ketik HELP untuk info lengkap.";
      bot.sendMessage(chat_id, balas, "");
    }
  }
  Serial.println("================================\n");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi terputus! Mencoba reconnect...");
    WiFi.reconnect();
    delay(5000);
    return;
  }

  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    
    lastTimeBotRan = millis();
  }
}