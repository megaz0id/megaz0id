#include <SPI.h>
#include <Ethernet3.h>

#define YES_PIN D1  // Зеленый светодиод - интернет есть
#define NO_PIN D4   // Красный светодиод - интернета нет
#define Melody RX

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 2000; // Проверять каждые 5 секунд

// Создаем Ethernet клиент для проверки соединения
EthernetClient client;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Инициализация пинов светодиодов
  pinMode(YES_PIN, OUTPUT);
  pinMode(NO_PIN, OUTPUT);
  pinMode(Melody, OUTPUT);
  digitalWrite(YES_PIN, LOW);
  digitalWrite(NO_PIN, LOW);
  
  // Инициализация Ethernet
  Ethernet.setCsPin(D2);
  Ethernet.init(D2);
  
  Serial.println("Инициализация Ethernet через DHCP...");
  
  // Мигаем обоими светодиодами во время инициализации
  for (int i = 0; i < 3; i++) {
    digitalWrite(YES_PIN, HIGH);
    digitalWrite(NO_PIN, HIGH);
    delay(200);
    digitalWrite(YES_PIN, LOW);
    digitalWrite(NO_PIN, LOW);
    delay(200);
  }
  
  // Попытка получить IP через DHCP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Не удалось получить IP через DHCP");
    // Мигаем красным светодиодом при ошибке DHCP
    while(true) {
      digitalWrite(NO_PIN, HIGH);
      delay(500);
      digitalWrite(NO_PIN, LOW);
      delay(500);
    }
  }
  
  Serial.println("Ethernet инициализирован!");
  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());
  Serial.print("Шлюз: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS: ");
  Serial.println(Ethernet.dnsServerIP());
}

void Sd_ReadOK() {  // звук ОК

    for (int i=400; i<6000; i=i*1.5) { tone(Melody, i); delay(20); }
  noTone(Melody);
 
}



void Sd_ErrorBeep(){  // звук "очередной шаг"

    for (int i=2500; i<6000; i=i*1.5) { tone(Melody, i); delay(10); }
  noTone(Melody);

}


bool checkInternetConnection() {
  // Пытаемся подключиться к публичному DNS серверу Google на порт 53 (DNS)
  if (client.connect(IPAddress(8, 8, 8, 8), 53)) {
    client.stop();
    Serial.println("Интернет доступен (успешное подключение к 8.8.8.8)");
    return true;
  }
  
  // Альтернативная проверка - попытка подключения к порту 80 Google
  if (client.connect(IPAddress(8, 8, 4, 4), 53)) {
    client.stop();
    Serial.println("Интернет доступен (успешное подключение к 8.8.4.4)");
    return true;
  }
  
  Serial.println("Не удалось подключиться к проверочным серверам");
  return false;
}

bool checkEthernetCable() {
  // Простая проверка наличия IP адреса
  if (Ethernet.localIP() == IPAddress(0,0,0,0)) {
    return false;
  }
  return true;
}

void loop() {
  if (millis() - lastCheckTime >= checkInterval) {
    lastCheckTime = millis();
    
    // Проверяем статус Ethernet кабеля
    if (checkEthernetCable()) {
      Serial.println("Ethernet кабель подключен, есть IP адрес");
      
      if (checkInternetConnection()) {
        Serial.println("Интернет доступен");
        // Мигаем зеленым светодиодом
         Sd_ReadOK();
        digitalWrite(YES_PIN, HIGH);
        digitalWrite(NO_PIN, LOW);
        delay(800);
        digitalWrite(YES_PIN, LOW);
        delay(800);
       
      } else {
        Serial.println("Интернет недоступен");
        // Мигаем красным светодиодом
          Sd_ErrorBeep();
        digitalWrite(NO_PIN, HIGH);
        digitalWrite(YES_PIN, LOW);
        delay(800);
        digitalWrite(NO_PIN, LOW);
        delay(800);
      
      }
    } else {
      Serial.println("Ethernet кабель отключен или нет IP адреса");
      // Быстро мигаем обоими светодиодами если кабель отключен
      digitalWrite(YES_PIN, HIGH);
      digitalWrite(NO_PIN, HIGH);
      delay(200);
      digitalWrite(YES_PIN, LOW);
      digitalWrite(NO_PIN, LOW);
      delay(200);
    }
  }
  
  // Обновляем статус Ethernet соединения
  Ethernet.maintain();
}