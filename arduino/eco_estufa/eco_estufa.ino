
#include <Servo.h>

Servo cobertura;

const int SENSOR_UMIDADE = A0;
const int SERVO_PIN = 9;
const int LED_VERDE = 6;
const int LED_VERMELHO = 7;

const int LIMITE_UMIDADE = 500;

String estadoTelhado = "ABERTO";
bool modoAutomatico = true;

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(50);

  cobertura.attach(SERVO_PIN);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  cobertura.write(90);
}

void loop() {
  // Recebe comandos do dashboard
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();

    if (comando == "ABRIR") {
      modoAutomatico = false;
      cobertura.write(90);
      estadoTelhado = "ABERTO";
    }
    else if (comando == "FECHAR") {
      modoAutomatico = false;
      cobertura.write(0);
      estadoTelhado = "FECHADO";
    }
    else if (comando == "AUTO") {
      modoAutomatico = true;
    }
  }

  int umidade = analogRead(SENSOR_UMIDADE);
  bool soloAdequado = umidade < LIMITE_UMIDADE;

  digitalWrite(LED_VERDE, soloAdequado ? HIGH : LOW);
  digitalWrite(LED_VERMELHO, soloAdequado ? LOW : HIGH);

  // Controle automático
  if (modoAutomatico) {
    if (umidade >= LIMITE_UMIDADE) {
      cobertura.write(0);
      estadoTelhado = "FECHADO";
    } else {
      cobertura.write(90);
      estadoTelhado = "ABERTO";
    }
  }

  // Envia dados para o Raspberry Pi
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
