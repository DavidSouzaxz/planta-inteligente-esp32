# 🌱 Sistema de Monitoramento de Planta com ESP32 & Display OLED

Um sistema completo de IoT para monitoramento em tempo real de uma planta/jardim, utilizando um ESP32 para coletar dados de sensores, renderizar expressões de status em um display gráfico e publicar os dados via MQTT na plataforma HiveMQ Cloud.

## 📋 Descrição do Projeto

Este projeto implementa um sistema de monitoramento ambiental inteligente que coleta dados de múltiplos sensores e os transmite via MQTT para a nuvem. Um display OLED de 1.3" integrado elimina a necessidade de LEDs e atua como uma interface rica para o usuário, alternando de forma independente entre o humor dinâmico da planta e o painel técnico com os valores reais das métricas.

### Principais Funcionalidades

- ✅ **Conexão WiFi com Feedback**: Conecta à rede exibindo uma barra de progresso animada diretamente no display.
- ✅ **MQTT Seguro**: Integração com HiveMQ Cloud usando conexão criptografada TLS/SSL (Porta 8883).
- ✅ **Interface Visual Inteligente (Carrossel)**: O display OLED alterna a cada 3 segundos entre expressões gráficas da planta (Feliz, Sede, Muito Quente) e o painel de telemetria.
- ✅ **Otimização de Banco de Dados**: Publicação automática configurada para **5 minutos**, reduzindo drasticamente o consumo de armazenamento e tráfego de rede.
- ✅ **Barramento I2C Compartilhado**: Display OLED e Sensor de Luminosidade operando juntos nas mesmas portas físicas.

---

## 🔧 Componentes de Hardware

### Microcontrolador

- **ESP32 Dev Module** - Microcontrolador com suporte nativo a WiFi, Bluetooth e criptografia em hardware.

### Sensores e Atuadores

| Componente | Modelo | Função | Tipo de Sinal / Pino |
| :--- | :--- | :--- | :--- |
| Sensor de Temperatura/Umidade | DHT22 | Medir temperatura e umidade relativa do ar | Digital (GPIO 23) |
| Sensor de Luminosidade | BH1750 | Medir intensidade de luz em Lux | I2C (GPIO 21, 22) |
| Sensor de Umidade do Solo | Capacitivo v1.2 | Medir a umidade interna do substrato | Analógico (GPIO 34) |
| **Interface Gráfica** | **OLED 1.3" (SSH1106)** | Exibir expressões e relatório de métricas | I2C (GPIO 21, 22) |

### Componentes Adicionais

- Resistor de pull-up no pino de dados do DHT22 (4.7kΩ a 10kΩ)
- Jumpers Macho-Fêmea (Recomendado para isolar as linhas de alimentação do display da protoboard)

---

## 📍 Mapeamento de Pinos (Pinout)

ESP32 GPIO Mapping:
├── GPIO 23    → DATA (DHT22 - Temperatura/Umidade)
├── GPIO 34    → ANALOG IN (Sensor de Umidade do Solo - ADC)
├── GPIO 21    → I2C SDA (Compartilhado: Display OLED 1.3" + Sensor BH1750)
└── GPIO 22    → I2C SCL (Compartilhado: Display OLED 1.3" + Sensor BH1750)


> ⚠️ **Nota Importante de Montagem:** O barramento I2C (GPIO 21 e 22) deve ser interligado em paralelo na protoboard. Verifique a estampa do seu display OLED antes de ligar: alguns modelos possuem a ordem dos pinos de alimentação invertida (`GND` / `VDD`). 

---

## 📦 Dependências do Arduino

Instale as seguintes bibliotecas gerenciadoras pelo menu (Sketch → Incluir Biblioteca → Gerenciar Bibliotecas...):

1. **U8g2** (por oliver) - Necessária para renderizar gráficos no controlador SSH1106 do display de 1.3".
2. **PubSubClient** (Nick O'Leary) - Cliente de mensageria MQTT.
3. **ArduinoJson** (Benoit Blanchon) - Serialização e tratamento de objetos JSON.
4. **DHT sensor library** (Adafruit) - Leitura do sensor DHT22.
5. **BH1750** (Christopher Laws) - Leitura do sensor de luminosidade.

---

## 🚀 Instalação e Configuração

### 1. Configuração do Firmware

Abra o código no seu editor e configure as suas credenciais locais e de acesso à nuvem:

```cpp
// Credenciais WiFi
const char* ssid = "SEU_WIFE";
const char* password = "SUA_SENHA_AQUI";

// Credenciais HiveMQ Cloud
const char* mqtt_server = "SUA_URL_MQTT"; 
const int mqtt_port = 8883; 
const char* mqtt_user = "SEU_USUARIO_MQTT";  
const char* mqtt_pass = "SUA_SENHA_MQTT"; 
2. Calibração do Sensor de Solo
O sensor capacitivo opera lendo valores de tensão analógica mapeados no código. Caso necessário, ajuste os limites:

C++
const int VALOR_SECO = 2592;    // Leitura com o sensor totalmente ao ar livre
const int VALOR_MOLHADO = 1071; // Leitura com o sensor inserido em copo d'água
📡 Tópicos MQTT e Payload
O sistema publica dados de telemetria agregados no seguinte formato:

Tópico: planta/sensores

Frequência de Envio: A cada 5 minutos (300.000 ms)

Formato: JSON estrito

JSON
{
  "temperatura": 31.0,
  "umidadeAr": 69.5,
  "luminosidade": 185.0,
  "umidadeSolo": 65
}

```

## 💡 Modo de Operação do Display
A lógica interna gerencia a tela por meio da função millis(), garantindo que as trocas visuais ocorram de forma assíncrona sem travar os loops do MQTT.

## 🌓 Estados do Carrossel (Troca a cada 3s)
Tela de Humor (Expressão Visual):

FELIZ: Condições ideais de umidade e luz.

SEDE: Ativado automaticamente se a umidade do solo cair abaixo de 20%. Exibe mensagem "Sede.. Me rega?".

MUITO_SOL: Ativado se a luminosidade passar de 1500 Lux. Exibe carinha sofrendo e a mensagem "Muito quente!".

Tela de Status Técnico:

Exibe o texto formatado limpo contendo as variáveis reais coletadas: Temp. Ar, Umid. Solo e Luminos..

## 🔍 Troubleshooting
Problema: O ESP32 desliga/desaparece a Porta COM ao plugar o Display
Causa: Curto-circuito na alimentação.

Solução: Remova o display da protoboard imediatamente. Verifique se os fios GND e VCC/VDD não estão invertidos ou compartilhando a mesma trilha de 5 furos da placa. Utilize cabos macho-fêmea direto do ESP32 para isolar os pinos do display.

Problema: Erro [BH1750] Device is not configured! no Monitor Serial
Causa: Conflito de ordem na inicialização do barramento I2C ou mau contato.

Solução: Certifique-se de que o Wire.begin() e o lightMeter.begin() estejam sendo declarados no setup() antes do método u8g2.begin().

Problema: Tela ligada, mas completamente apagada (Sem vídeo)
Causa: Endereço I2C incorreto do controlador SSH1106.

Solução: Altere a inicialização no método setup() para forçar o endereço alternativo comum em displays de 1.3":

```C++
u8g2.setI2CAddress(0x3D * 2);
u8g2.begin();
```

## 🔐 Segurança   

- ✅ Conexão Criptografada: Envio para a nuvem blindado via camada WiFiClientSecure ignorando certificados locais inseguros.

- ✅ Eficiência de Escrita: Banco Neon protegido contra estouro de armazenamento e lentidão de queries.

### 📝 Alterações Recentes
- 🔄 Substituição da barra de 8 LEDs por Display OLED Gráfico I2C de 1.3".

- ⚙️ Implementação da biblioteca U8g2 com suporte ao chip SSH1106.

- 🕒 Alteração do delay de publicação MQTT de 50s para 5 minutos visando a sustentabilidade do banco PostgreSQL Cloud.

- 🛠️ Correção de ordem de precedência no setup do barramento de comunicação I2C.

👨‍💻 Desenvolvedor
David Souza & Grupo de Engenharia IoT

Última atualização: Maio de 2026