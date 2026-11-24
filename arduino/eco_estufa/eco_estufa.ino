
#include <Servo.h>

Servo cobertura;

// Pinos
const int SENSOR_UMIDADE = A0;
const int SERVO_PIN = 9;
const int LED_VERDE = 6;
const int LED_VERMELHO = 7;

// Limite inicial (precisa de calibracao)
const int LIMITE_UMIDADE = 500;

String estadoTelhado = "ABERTO";

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(50);

  cobertura.attach(SERVO_PIN);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  cobertura.write(0);
}

void loop() {
  // Leitura do sensor
  int umidade = analogRead(SENSOR_UMIDADE);

  // Verifica a condicao do solo
  bool soloAdequado = umidade < LIMITE_UMIDADE;

  // Controle dos LEDs
  digitalWrite(LED_VERDE, soloAdequado ? HIGH : LOW);
  digitalWrite(LED_VERMELHO, soloAdequado ? LOW : HIGH);

  // Recebe comandos da Raspberry Pi
  if (Serial.available()) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();

    if (comando == "ABRIR") {
      cobertura.write(90);
      estadoTelhado = "ABERTO";
    }

    else if (comando == "FECHAR") {
      cobertura.write(0);
      estadoTelhado = "FECHADO";
    }
  }

  // Envia os dados para a Raspberry Pi
  Serial.print("{\"umidade_solo\":");
  Serial.print(umidade);

  Serial.print(",\"telhado\":\"");
  Serial.print(estadoTelhado);

  Serial.print("\",\"led_verde\":");
  Serial.print(soloAdequado ? 1 : 0);

  Serial.print(",\"led_vermelho\":");
  Serial.print(soloAdequado ? 0 : 1);

  Serial.println("}");

  delay(1000);
}
