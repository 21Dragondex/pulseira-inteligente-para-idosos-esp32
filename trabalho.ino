#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <MPU6050.h>

// ===== WIFI =====
const char* ssid = "Nome_da_rede";
const char* password = "Senha_da_rede";

// ===== TELEGRAM =====
String BOT_TOKEN = "token_do_bot";
String CHAT_ID = "id_da_conversa";

// ===== BOTÃO =====
#define BOTAO 26

// ===== MPU6050 =====
MPU6050 mpu;

// ===== CONTROLE =====
bool alertaQuedaEnviado = false;

// ======================================================
// ENVIAR TELEGRAM (corrigido com URL encoding básico)
// ======================================================
void enviarMensagem(String mensagem) {

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    String url = "https://api.telegram.org/bot" +
                 BOT_TOKEN +
                 "/sendMessage?chat_id=" +
                 CHAT_ID +
                 "&text=";

    // encode simples (espaço -> %20)
    for (int i = 0; i < mensagem.length(); i++) {
      if (mensagem[i] == ' ') url += "%20";
      else url += mensagem[i];
    }

    http.begin(url);

    int httpCode = http.GET();

    Serial.print("HTTP Code: ");
    Serial.println(httpCode);

    http.end();
  }
}

// ======================================================
// SETUP
// ======================================================
void setup() {

  Serial.begin(115200);

  // ===== BOTÃO =====
  pinMode(BOTAO, INPUT_PULLUP);

  // ===== I2C =====
  Wire.begin(21, 22);

  // ===== MPU6050 =====
  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("MPU6050 conectado!");
  } else {
    Serial.println("ERRO: MPU6050 nao detectado");
  }

  // ===== WIFI =====
  WiFi.begin(ssid, password);

  Serial.println("Conectando WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  Serial.println("WiFi conectado!");
  Serial.println(WiFi.localIP());

  enviarMensagem("Sistema iniciado com sucesso");
}

// ======================================================
// LOOP
// ======================================================
void loop() {

  // ===== BOTÃO =====
  if (digitalRead(BOTAO) == LOW) {

    enviarMensagem("Botao de ajuda pressionado");

    delay(1500);
  }

  // ===== MPU6050 =====
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  float AcX = ax / 16384.0;
  float AcY = ay / 16384.0;
  float AcZ = az / 16384.0;

  // magnitude (melhor que soma simples)
  float intensidade = sqrt(AcX * AcX + AcY * AcY + AcZ * AcZ);

  Serial.print("Intensidade: ");
  Serial.println(intensidade);

  // ===== DETECÇÃO DE QUEDA =====
  if (intensidade > 2.5 && !alertaQuedaEnviado) {

    enviarMensagem("ALERTA: Possivel queda detectada");

    alertaQuedaEnviado = true;

    delay(4000);
  }

  // reset do alerta
  if (intensidade < 1.5) {
    alertaQuedaEnviado = false;
  }

  delay(200);
}