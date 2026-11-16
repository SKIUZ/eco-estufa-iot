#include <Servo.h>
#include <DHT.h>

// Definição de Pinos
#define PINO_DHT 2
#define TIPO_DHT DHT11
#define PINO_SERVO 9
#define PINO_COOLER 8
#define PINO_LED_VERDE 6
#define PINO_LED_VERM 7
#define PINO_BUZZER 5
#define PINO_CHUVA A0

DHT dht(PINO_DHT, TIPO_DHT);
Servo telhadoServo;

String modoOperacao = "AUTO"; // "AUTO", "ABRIR", "FECHAR"
bool telhadoFechado = false;
bool coolerLigado = false;
unsigned long ultimoEnvio = 0;

void setup() {
  Serial.begin(9600);
  dht.begin();
  
  telhadoServo.attach(PINO_SERVO);
  telhadoServo.write(0); // Inicia aberto (0 graus)
  
  pinMode(PINO_COOLER, OUTPUT);
  pinMode(PINO_LED_VERDE, OUTPUT);
  pinMode(PINO_LED_VERM, OUTPUT);
  pinMode(PINO_BUZZER, OUTPUT);
  
  digitalWrite(PINO_LED_VERDE, HIGH);
}

void loop() {
  // 1. Verifica comandos enviados pela Raspberry Pi
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();
    if (comando == "ABRIR" || comando == "FECHAR" || comando == "AUTO") {
      modoOperacao = comando;
    }
  }

  // 2. Leitura dos 2 Sensores (DHT11 e Chuva)
  float temp = dht.readTemperature();
  float umid = dht.readHumidity();
  if (isnan(temp)) temp = 25.0;
  if (isnan(umid)) umid = 60.0;

  int leituraChuva = analogRead(PINO_CHUVA);
  bool estaChovendo = (leituraChuva < 500);

  // 3. Lógica de Decisão (Telhado)
  if (modoOperacao == "FECHAR") {
    fecharTelhado();
  } else if (modoOperacao == "ABRIR") {
    abrirTelhado();
  } else {
    if (estaChovendo) fecharTelhado();
    else abrirTelhado();
  }

  // 4. Lógica de Decisão (Onda de Calor > 30°C)
  if (temp > 30.0) {
    digitalWrite(PINO_COOLER, HIGH);
    coolerLigado = true;
  } else {
    digitalWrite(PINO_COOLER, LOW);
    coolerLigado = false;
  }

  // 5. Envia JSON para a Raspberry Pi a cada 2 segundos
  if (millis() - ultimoEnvio >= 2000) {
    ultimoEnvio = millis();
    Serial.print("{\"temp\":"); Serial.print(temp, 1);
    Serial.print(",\"umid\":"); Serial.print(umid, 1);
    Serial.print(",\"chuva\":"); Serial.print(estaChovendo ? 1 : 0);
    Serial.print(",\"telhado\":\""); Serial.print(telhadoFechado ? "FECHADO" : "ABERTO");
    Serial.print("\",\"cooler\":"); Serial.print(coolerLigado ? 1 : 0);
    Serial.print(",\"modo\":\""); Serial.print(modoOperacao);
    Serial.println("\"}");
  }
}

void fecharTelhado() {
  if (!telhadoFechado) {
    telhadoServo.write(90);
    telhadoFechado = true;
    digitalWrite(PINO_LED_VERM, HIGH);
    digitalWrite(PINO_LED_VERDE, LOW);
    tone(PINO_BUZZER, 1000, 300);
  }
}

void abrirTelhado() {
  if (telhadoFechado) {
    telhadoServo.write(0);
    telhadoFechado = false;
    digitalWrite(PINO_LED_VERM, LOW);
    digitalWrite(PINO_LED_VERDE, HIGH);
  }
}
