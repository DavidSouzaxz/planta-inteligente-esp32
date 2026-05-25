#include <WiFi.h>
#include <WiFiClientSecure.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <BH1750.h>
#include "DHT.h"

// --- CONFIGURAÇÕES DE REDE ---
const char* ssid = "rede_name";
const char* password = "rede_password";

// --- CONFIGURAÇÕES DO HIVEMQ CLOUD ---
const char* mqtt_server = "mtqq_url"; 
const int mqtt_port = mtqq_port; 
const char* mqtt_user = "mtqq_user"; 
const char* mqtt_pass = "mtqq_password"; 
const char* mqtt_topic = "planta/sensores";

// --- CONFIGURAÇÃO DA BARRA DE LED ---
// Vetor com os pinos dos 8 LEDs em ordem (do pior sinal para o melhor)
const int pinosLed[] = {13, 12, 14, 27, 26, 25, 19, 18};
const int qtdLeds = 8;

// --- CONFIGURAÇÕES DOS SENSORES ---
#define DHTPIN 23     
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;
#define SOIL_PIN 34
const int VALOR_SECO = 2592;    
const int VALOR_MOLHADO = 1071; 

WiFiClientSecure espClient; 
PubSubClient client(espClient);
unsigned long lastMsg = 0;

// Função para controlar quantos LEDs acendem na barra (0 a 8)
void atualizarBarraLed(int numLedsAcesos) {
  for (int i = 0; i < qtdLeds; i++) {
    if (i < numLedsAcesos) {
      digitalWrite(pinosLed[i], HIGH); // Acende
    } else {
      digitalWrite(pinosLed[i], LOW);  // Apaga
    }
  }
}

// Animação de carregamento (loop) enquanto tenta conectar
void animacaoCarregando() {
  for (int i = 0; i < qtdLeds; i++) {
    digitalWrite(pinosLed[i], HIGH);
    delay(100);
    digitalWrite(pinosLed[i], LOW);
  }
}

void setup_wifi() {
  delay(10);
  Serial.println("\nConectando ao Wi-Fi...");

  WiFi.begin(ssid, password);

  // Enquanto não conectar, fica rodando a animação de loop na barra
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED) {
    animacaoCarregando();
    Serial.print(".");
    tentativas++;
    
    // Se passar de 30 tentativas (Erro de conexão)
    if (tentativas > 30) {
      Serial.println("\nErro: Não foi possível conectar ao Wi-Fi.");
      atualizarBarraLed(0); // Deixa a barra toda apagada
      return; 
    }
  }

  Serial.println("\nWi-Fi conectado!");
  // Quando conecta, dá um feedback rápido piscando a barra toda 2 vezes
  for(int i=0; i<2; i++) {
    atualizarBarraLed(8); delay(200);
    atualizarBarraLed(0); delay(200);
  }
}

// Função que mede o sinal do Wi-Fi e atualiza os LEDs
void atualizarSinalMqtt() {
  long rssi = WiFi.RSSI(); // Mede o sinal em dBm (ex: -60)
  
  // Mapeia o sinal de dBm para a quantidade de LEDs (8 a 0)
  // -50 dBm ou melhor = sinal excelente (8 LEDs)
  // -90 dBm ou pior = sinal horrível (1 LED)
  int ledsSinal = map(rssi, -90, -50, 1, qtdLeds);
  ledsSinal = constrain(ledsSinal, 0, qtdLeds); 

  atualizarBarraLed(ledsSinal);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT no HiveMQ Cloud...");
    String clientId = "ESP32Planta-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Conectado com sucesso!");
    } else {
      Serial.print("falou, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Configura os pinos da barra de LED como saídas
  for (int i = 0; i < qtdLeds; i++) {
    pinMode(pinosLed[i], OUTPUT);
    digitalWrite(pinosLed[i], LOW); // Inicia apagada
  }

  dht.begin();
  Wire.begin();
  lightMeter.begin();
  pinMode(SOIL_PIN, INPUT);

  setup_wifi();
  espClient.setInsecure(); 
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Mantém a barra atualizada com a intensidade do sinal Wi-Fi em tempo real
  if (WiFi.status() == WL_CONNECTED) {
    atualizarSinalMqtt();
  } else {
    atualizarBarraLed(0); // Apaga se cair a rede
  }

  unsigned long now = millis();
  if (now - lastMsg > 50000) { 
    lastMsg = now;

    float umidadeAr = dht.readHumidity();
    float temperatura = dht.readTemperature();
    float lux = lightMeter.readLightLevel();
    int valorBrutoSolo = analogRead(SOIL_PIN);
    int umidadeSoloPct = map(valorBrutoSolo, VALOR_SECO, VALOR_MOLHADO, 0, 100);
    umidadeSoloPct = constrain(umidadeSoloPct, 0, 100);

    if (isnan(umidadeAr) || isnan(temperatura)) {
      return;
    }

    StaticJsonDocument<200> doc;
    doc["temperatura"] = temperatura;
    doc["umidadeAr"] = umidadeAr;
    doc["luminosidade"] = lux;
    doc["umidadeSolo"] = umidadeSoloPct;

    char buffer[200];
    serializeJson(doc, buffer);

    client.publish(mqtt_topic, buffer);
  }
}