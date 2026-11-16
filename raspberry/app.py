import json
import sqlite3
import threading
import time
from flask import Flask, render_template, jsonify, request
import serial

app = Flask(__name__)
PORTA_SERIAL = '/dev/ttyACM0'
BAUD_RATE = 9600

estado_atual = {
    "temp": 0.0, "umid": 0.0, "chuva": 0,
    "telhado": "ABERTO", "cooler": 0, "modo": "AUTO",
    "status_conexao": "Desconectado"
}
arduino = None

def init_db():
    conn = sqlite3.connect('estufa.db')
    c = conn.cursor()
    c.execute('''CREATE TABLE IF NOT EXISTS leituras (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    data_hora DATETIME DEFAULT CURRENT_TIMESTAMP,
                    temp REAL, umid REAL, chuva INTEGER, telhado TEXT)''')
    conn.commit()
    conn.close()

def ler_serial():
    global arduino, estado_atual
    while True:
        try:
            if arduino is None or not arduino.is_open:
                arduino = serial.Serial(PORTA_SERIAL, BAUD_RATE, timeout=1)
                time.sleep(2)
                estado_atual["status_conexao"] = "Conectado"
            if arduino.in_waiting > 0:
                linha = arduino.readline().decode('utf-8').strip()
                if linha.startswith("{") and linha.endswith("}"):
                    dados = json.loads(linha)
                    estado_atual.update(dados)
                    estado_atual["status_conexao"] = "Conectado"
                    conn = sqlite3.connect('estufa.db')
                    c = conn.cursor()
                    c.execute("INSERT INTO leituras (temp, umid, chuva, telhado) VALUES (?, ?, ?, ?)",
                              (dados['temp'], dados['umid'], dados['chuva'], dados['telhado']))
                    conn.commit()
                    conn.close()
        except Exception as e:
            estado_atual["status_conexao"] = f"Erro Serial: {str(e)}"
            time.sleep(3)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/dados')
def get_dados():
    conn = sqlite3.connect('estufa.db')
    c = conn.cursor()
    c.execute("SELECT data_hora, temp, umid, chuva, telhado FROM leituras ORDER BY id DESC LIMIT 10")
    historico = c.fetchall()
    conn.close()
    return jsonify({"atual": estado_atual, "historico": historico})

@app.route('/api/comando', methods=['POST'])
def enviar_comando():
    cmd = request.json.get('comando')
    if cmd in ['AUTO', 'ABRIR', 'FECHAR'] and arduino and arduino.is_open:
        arduino.write(f"{cmd}\n".encode('utf-8'))
        return jsonify({"sucesso": True, "comando": cmd})
    return jsonify({"sucesso": False}), 400

if __name__ == '__main__':
    init_db()
    t = threading.Thread(target=ler_serial, daemon=True)
    t.start()
    app.run(host='0.0.0.0', port=5000)
