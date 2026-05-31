#include <WiFi.h>
#include <WiFiClientSecure.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <BH1750.h>
#include "DHT.h"
#include <U8g2lib.h> // Biblioteca para o Display OLED 1.3"

// --- CONFIGURAÇÃO DO DISPLAY OLED 1.3" (SSH1106) ---
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// --- CONFIGURAÇÕES DE REDE ---
const char* ssid = "VISTEL";
const char* password = "David1221!";

// --- CONFIGURAÇÕES DO HIVEMQ CLOUD ---
const char* mqtt_server = "e1533cd2d90943b2b0ee895d6a733562.s1.eu.hivemq.cloud"; 
const int mqtt_port = 8883; 
const char* mqtt_user = "d4vid";  
const char* mqtt_pass = "David1221"; 
const char* mqtt_topic = "planta/sensores";

// --- CONFIGURAÇÕES DOS SENSORES ---
#define DHTPIN 23     
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;
#define SOIL_PIN 34
const int VALOR_SECO = 2592;    
const int VALOR_MOLHADO = 1071; 

// --- VARIÁVEIS GLOBAIS ---
WiFiClientSecure espClient; 
PubSubClient client(espClient);
unsigned long lastMsg = 0;
unsigned long tempoTrocaTela = 0;

// Variáveis de armazenamento das leituras
float umidadeAr = 0.0;
float temperatura = 0.0;
float lux = 0.0;
int umidadeSoloPct = 0;
String humorAtual = "FELIZ";
int telaAtual = 0; // 0 = Carinha, 1 = Status dos Sensores

// --- FUNÇÕES GRÁFICAS DO DISPLAY ---

void desenharCarinhaFeliz() {
  u8g2.drawFrame(38, 18, 6, 6); 
  u8g2.drawFrame(84, 18, 6, 6); 
  u8g2.drawCircle(64, 32, 12, U8G2_DRAW_LOWER_LEFT | U8G2_DRAW_LOWER_RIGHT);
}

void desenharCarinhaSede() {
  u8g2.drawLine(35, 15, 45, 22);
  u8g2.drawLine(93, 15, 83, 22);
  u8g2.drawFrame(38, 22, 6, 4);
  u8g2.drawFrame(84, 22, 6, 4);
  u8g2.drawCircle(64, 45, 8, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(24, 60, "Sede.. Me rega?");
}

void desenharCarinhaCalor() {
  u8g2.drawLine(35, 16, 45, 24); u8g2.drawLine(35, 24, 45, 16);
  u8g2.drawLine(93, 16, 83, 24); u8g2.drawLine(93, 24, 83, 16);
  u8g2.drawCircle(64, 40, 5);
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(26, 58, "Muito quente!");
}

void gerenciarDisplay(String statusConexao) {
  u8g2.clearBuffer();
  
  if (statusConexao == "CONECTANDO") {
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(15, 25, "Conectando WiFi...");
    
    static int barra = 10;
    u8g2.drawFrame(15, 40, 98, 10);
    u8g2.drawBox(17, 42, barra, 6);
    barra = (barra + 5) % 95;
  } 
  else {
    // Alterna entre Carinha e Dados numéricos a cada 3 segundos (3000 ms)
    if (millis() - tempoTrocaTela > 3000) {
      tempoTrocaTela = millis();
      telaAtual = (telaAtual == 0) ? 1 : 0;
    }

    if (telaAtual == 0) {
      // Exibe a expressão baseada no humor atual calculado
      if (humorAtual == "SEDE") desenharCarinhaSede();
      else if (humorAtual == "MUITO_SOL") desenharCarinhaCalor();
      else desenharCarinhaFeliz();
    } 
    else {
      // Exibe o painel de métricas em tempo real
      u8g2.setFont(u8g2_font_6x12_tf);
      u8g2.drawStr(18, 10, "--- STATUS IOT ---");
      
      char bufferStr[25];
      
      sprintf(bufferStr, "Temp. Ar:   %.1f C", temperatura);
      u8g2.drawStr(0, 28, bufferStr);
      
      sprintf(bufferStr, "Umid. Solo: %d %%", umidadeSoloPct);
      u8g2.drawStr(0, 42, bufferStr);
      
      sprintf(bufferStr, "Luminos.:   %.0f Lux", lux);
      u8g2.drawStr(0, 56, bufferStr);
    }
  }
  
  u8g2.sendBuffer();
}

void setup_wifi() {
  delay(10);
  Serial.println("\nConectando ao Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    gerenciarDisplay("CONECTANDO");
    delay(150);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    String clientId = "ESP32Planta-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Conectado ao HiveMQ!");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  Wire.begin(); 
  
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 inicializado com sucesso!"));
  } else {
    Serial.println(F("Erro ao iniciar o BH1750! Verifique os fios."));
  }

  u8g2.begin();
  
  dht.begin();
  pinMode(SOIL_PIN, INPUT);

  setup_wifi();
  espClient.setInsecure(); 
  client.setServer(mqtt_server, mqtt_port);
  
  // Realiza uma leitura inicial no setup para o display não começar zerado
  temperatura = dht.readTemperature();
  umidadeAr = dht.readHumidity();
  lux = lightMeter.readLightLevel();
  int valorBrutoSolo = analogRead(SOIL_PIN);
  umidadeSoloPct = map(valorBrutoSolo, VALOR_SECO, VALOR_MOLHADO, 0, 100);
  umidadeSoloPct = constrain(umidadeSoloPct, 0, 100);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Mantém o display rodando as animações/telas em tempo real continuamente
  if (WiFi.status() == WL_CONNECTED) {
    gerenciarDisplay("OK");
  }

  unsigned long now = millis();
  if (now - lastMsg > 300000) { // Envia dados ao servidor a cada 5 minutos (300000 ms)
    lastMsg = now;

    umidadeAr = dht.readHumidity();
    temperatura = dht.readTemperature();
    lux = lightMeter.readLightLevel();
    int valorBrutoSolo = analogRead(SOIL_PIN);
    
    umidadeSoloPct = map(valorBrutoSolo, VALOR_SECO, VALOR_MOLHADO, 0, 100);
    umidadeSoloPct = constrain(umidadeSoloPct, 0, 100);

    if (isnan(umidadeAr) || isnan(temperatura)) {
      return;
    }

    // Recalcula o humor que altera o desenho da carinha do display
    if (umidadeSoloPct < 20) {
      humorAtual = "SEDE";
    } else if (lux > 1500) {
      humorAtual = "MUITO_SOL";
    } else {
      humorAtual = "FELIZ";
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