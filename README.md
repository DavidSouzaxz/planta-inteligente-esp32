# 🌱 Sistema de Monitoramento de Planta com ESP32

Um sistema completo de IoT para monitoramento em tempo real de uma planta/jardim, utilizando um ESP32 para coletar dados de sensores e publicar via MQTT na plataforma HiveMQ Cloud.

## 📋 Descrição do Projeto

Este projeto implementa um sistema de monitoramento ambiental inteligente que coleta dados de múltiplos sensores e os transmite via MQTT para a nuvem. A barra de LEDs integrada fornece feedback visual do sinal WiFi em tempo real.

### Principais Funcionalidades

- ✅ **Conexão WiFi Segura**: Conecta automaticamente a uma rede WiFi com retry automático
- ✅ **MQTT Seguro**: Integração com HiveMQ Cloud usando conexão TLS/SSL
- ✅ **Múltiplos Sensores**: Coleta dados de temperatura, umidade, luminosidade e umidade do solo
- ✅ **Indicador Visual**: Barra de 8 LEDs que mostra a intensidade do sinal WiFi em tempo real
- ✅ **Publicação Automática**: Publica dados dos sensores a cada 50 segundos
- ✅ **Animações de Status**: Feedback visual durante a conexão e operação

---

## 🔧 Componentes de Hardware

### Microcontrolador

- **ESP32** - Microcontrolador WiFi com suporte MQTT

### Sensores

| Componente                    | Modelo               | Função                            | Pino              |
| ----------------------------- | -------------------- | --------------------------------- | ----------------- |
| Sensor de Temperatura/Umidade | DHT22                | Medir temperatura e umidade do ar | GPIO 23           |
| Sensor de Luminosidade        | BH1750               | Medir intensidade de luz (lux)    | I2C (GPIO 21, 22) |
| Sensor de Umidade do Solo     | Capacitivo/Analógico | Medir umidade do solo             | GPIO 34 (ADC)     |

### Indicador Visual

- **Barra de LEDs**: 8 LEDs através de resistores de proteção (330Ω recomendado)

### Componentes Adicionais

- Resistor de pull-up no pino do DHT22 (4.7kΩ)
- Capacitor de 100µF próximo à alimentação do DHT22
- Jumpers e placa de prototipagem

---

## 📍 Mapeamento de Pinos

```
ESP32 GPIO Mapping:
├── GPIO 23    → DHT22 (Temperatura/Umidade)
├── GPIO 21    → I2C SDA (BH1750)
├── GPIO 22    → I2C SCL (BH1750)
├── GPIO 34    → SOIL_PIN (Sensor de Umidade Solo)
├── GPIO 13    → LED 1 (Sinal mais fraco)
├── GPIO 12    → LED 2
├── GPIO 14    → LED 3
├── GPIO 27    → LED 4
├── GPIO 26    → LED 5
├── GPIO 25    → LED 6
├── GPIO 19    → LED 7
└── GPIO 18    → LED 8 (Sinal mais forte)
```

---

## 📦 Dependências do Arduino

Instale as seguintes bibliotecas pela IDE do Arduino (Sketch → Include Library → Manage Libraries):

1. **PubSubClient** (Nick O'Leary) - Cliente MQTT
2. **ArduinoJson** (Benoit Blanchon) - Parser JSON
3. **DHT sensor library** (Adafruit) - Sensor DHT22
4. **BH1750** (Christopher Laws) - Sensor de luminosidade
5. **WiFi** (Built-in) - Conectividade WiFi
6. **Wire** (Built-in) - Protocolo I2C

---

## 🚀 Instalação e Configuração

### 1. Hardware Setup

1. Conecte o ESP32 à placa de prototipagem
2. Conecte os sensores conforme o mapeamento de pinos acima
3. Conecte os LEDs aos pinos indicados com resistores em série
4. Adicione capacitor de 100µF perto da alimentação dos sensores
5. Verifique todas as conexões de terra (GND) e alimentação (3.3V)

### 2. Configuração do Firmware

Abra o arquivo `teste.ino` e configure as seguintes credenciais:

```cpp
// Credenciais WiFi
const char* ssid = "seu_wifi_name";
const char* password = "seu_wifi_password";

// Credenciais HiveMQ Cloud
const char* mqtt_server = "seu_mqtt_url.com";
const int mqtt_port = 8883;  // Porta segura TLS
const char* mqtt_user = "seu_usuario_mqtt";
const char* mqtt_pass = "sua_senha_mqtt";
```

### 3. Calibração do Sensor de Umidade do Solo

O sensor de umidade do solo funciona em range analógico. Calibre os valores:

```cpp
const int VALOR_SECO = 2592;    // Valor lido quando o solo está seco
const int VALOR_MOLHADO = 1071; // Valor lido quando o solo está molhado
```

**Como calibrar:**

1. Leia o valor do sensor com o solo completamente seco
2. Leia o valor com o solo molhado
3. Atualize as constantes com esses valores

### 4. Upload do Código

1. Abra a IDE do Arduino
2. Selecione a placa: **ESP32 Dev Module**
3. Selecione a porta COM correspondente
4. Clique em **Fazer Upload** (ou Ctrl+U)

---

## 📡 Tópicos MQTT

O sistema publica dados no seguinte tópico:

### Publicação

- **Tópico**: `planta/sensores`
- **Intervalo**: A cada 50 segundos
- **Formato**: JSON

**Exemplo de payload:**

```json
{
  "temperatura": 24.5,
  "umidadeAr": 65.3,
  "luminosidade": 1250.5,
  "umidadeSolo": 72
}
```

| Campo          | Tipo  | Unidade | Descrição                      |
| -------------- | ----- | ------- | ------------------------------ |
| `temperatura`  | float | °C      | Temperatura ambiente           |
| `umidadeAr`    | float | %       | Umidade relativa do ar         |
| `luminosidade` | float | lux     | Intensidade de luz             |
| `umidadeSolo`  | int   | %       | Porcentagem de umidade do solo |

---

## 💡 Modo de Operação

### Inicialização

1. Serial begin a 115200 baud
2. Inicializa pinos dos LEDs (todos apagados)
3. Inicializa sensores DHT22 e BH1750
4. Conecta ao WiFi com animação de carregamento (loop de LEDs)
5. Ao conectar, pisca toda a barra 2 vezes
6. Conecta ao servidor MQTT HiveMQ Cloud

### Loop Principal

1. **Verifica conexão MQTT**: Reconecta se necessário
2. **Atualiza indicador de sinal**: Barra de LEDs mostra força do sinal WiFi em tempo real
   - -50 dBm ou melhor → 8 LEDs acesos (excelente)
   - -90 dBm ou pior → 1 LED aceso (fraco)
3. **Coleta de dados**: A cada 50 segundos:
   - Lê temperatura e umidade do ar (DHT22)
   - Lê luminosidade (BH1750)
   - Lê umidade do solo (ADC)
   - Formata dados em JSON
   - Publica no tópico MQTT

---

## 📊 Indicador de Sinal WiFi com LEDs

A barra de 8 LEDs oferece feedback visual em tempo real:

```
Sinal WiFi     LEDs Acesos     Status
-50 dBm        ████████        Excelente
-60 dBm        ███████░        Muito Bom
-70 dBm        ██████░░        Bom
-75 dBm        █████░░░        Regular
-80 dBm        ████░░░░        Fraco
-90 dBm        █░░░░░░░        Muito Fraco
```

**Estados Especiais:**

- **Animação de carregamento**: Loop piscante enquanto tenta conectar ao WiFi
- **Piscar duplo**: Confirmação de conexão WiFi bem-sucedida
- **Todos apagados**: Sem conexão WiFi

---

## 🔍 Troubleshooting

### Problema: Não conecta ao WiFi

- ✓ Verifique SSID e senha (case-sensitive)
- ✓ Verifique se o WiFi está visível
- ✓ Tente reboot do ESP32
- ✓ Verifique a distância do roteador

### Problema: Não conecta ao MQTT

- ✓ Verifique URL, usuário e senha do HiveMQ
- ✓ Verifique porta (use 8883 para TLS)
- ✓ Verifique se WiFi está conectado (barra de LED)
- ✓ Verifique firewall/roteador bloqueando porta 8883

### Problema: Sensores não respondem

- ✓ DHT22: Verifique conexão, pull-up, capacitor
- ✓ BH1750: Verifique conexão I2C (SDA/SCL)
- ✓ Sensor Solo: Verifique se está no pino ADC (GPIO 34)

### Problema: Leituras incorretas

- ✓ Sensor solo: Recalibre `VALOR_SECO` e `VALOR_MOLHADO`
- ✓ DHT22: Aguarde 2 segundos entre leituras
- ✓ Verifique alimentação (especialmente DHT22 precisa de 3.3V estável)

### Serial Monitor não abre

- ✓ Verifique porta COM correta
- ✓ Baud rate deve estar em 115200
- ✓ Desconecte outros programas acessando a porta

---

## 🔐 Segurança

- ✅ MQTT usa conexão TLS/SSL (porta 8883)
- ✅ WiFi usa WPA2 password
- ✅ Credenciais no código (considere usar SPIFFS ou LittleFS em produção)
- ⚠️ **Nota**: Não compartilhe credenciais. Mude `mqtt_pass` regularmente.

---

## 📝 Alterações Recentes

- Implementação inicial com suporte completo a WiFi e MQTT
- Integração de múltiplos sensores
- Indicador visual de sinal WiFi com barra de LEDs
- Sistema de reconnect automático

---

## 👨‍💻 Desenvolvedor

**David Souza**

---

## 📄 Licença

Este projeto é disponibilizado como está para fins educacionais e pessoais.

---

## 🤝 Suporte

Para problemas ou dúvidas:

1. Verifique a seção **Troubleshooting**
2. Consulte a documentação dos sensores
3. Verifique logs do HiveMQ Cloud

---

**Última atualização**: Maio de 2026
