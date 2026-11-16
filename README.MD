# 🌱 EcoEstufa - Estufa Urbana Resiliente a Eventos Climáticos Extremos

Projeto desenvolvido para a disciplina de **Hardware Architecture** (Ciência da Computação - ATITUS).

## 📋 Sobre o Projeto
Protótipo de estufa inteligente voltada à agricultura urbana, capaz de proteger cultivos contra chuvas intensas/granizo e ondas de calor através de monitoramento ambiental e atuação autônoma.

## 🛠️ Arquitetura e Componentes
* **Controladores:** Arduino Uno + Raspberry Pi (comunicação via USB Serial a 9600 bps)[cite: 1].
* **Sensores:** DHT11 (Temperatura e Umidade) e Módulo Sensor de Chuva[cite: 1].
* **Atuadores:** Micro Servo Motor SG90 (cobertura retrátil), Cooler 5V, Buzzer e LEDs de status[cite: 1].

## 🚀 Como Executar
1. **Arduino:** Abra `arduino/eco_estufa.ino` na Arduino IDE, instale a biblioteca `DHT sensor library` e envie para a placa.
2. **Raspberry Pi:** Conecte o Arduino via USB, entre na pasta `raspberry/`, instale as dependências (`pip install -r requirements.txt`) e execute `python3 app.py`.
3. **Dashboard:** Acesse `http://<IP_DA_RASPBERRY>:5000` no navegador.
