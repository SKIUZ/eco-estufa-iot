
import json
import sqlite3
import threading
import time
from pathlib import Path

import serial
from flask import Flask, render_template, jsonify, request

app = Flask(__name__)

BASE_DIR = Path(__file__).resolve().parent
DB_PATH = BASE_DIR / "estufa.db"

PORTA_SERIAL = "/dev/ttyACM0"
BAUD_RATE = 9600

estado_atual = {
    "umidade_solo": 0,
    "telhado": "ABERTO",
    "led_verde": 0,
    "led_vermelho": 0,
    "status_conexao": "Desconectado"
}

arduino = None
serial_lock = threading.Lock()


def init_db():
    with sqlite3.connect(DB_PATH) as conn:
        conn.execute("""
            CREATE TABLE IF NOT EXISTS leituras_solo (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                data_hora DATETIME DEFAULT CURRENT_TIMESTAMP,
                umidade_solo INTEGER,
                telhado TEXT,
                led_verde INTEGER,
                led_vermelho INTEGER
            )
        """)


def ler_serial():
    global arduino

    while True:
        try:
            with serial_lock:
                if arduino is None or not arduino.is_open:
                    arduino = serial.Serial(
                        PORTA_SERIAL,
                        BAUD_RATE,
                        timeout=1
                    )
                    time.sleep(2)
                    arduino.reset_input_buffer()

                linha = arduino.readline().decode(
                    "utf-8", errors="ignore"
                ).strip()

            if not linha:
                continue

            dados = json.loads(linha)

            if "umidade_solo" not in dados:
                continue

            estado_atual.update({
                "umidade_solo": int(dados["umidade_solo"]),
                "telhado": dados["telhado"],
                "led_verde": int(dados["led_verde"]),
                "led_vermelho": int(dados["led_vermelho"]),
                "status_conexao": "Conectado"
            })

            with sqlite3.connect(DB_PATH) as conn:
                conn.execute("""
                    INSERT INTO leituras_solo
                    (umidade_solo, telhado, led_verde, led_vermelho)
                    VALUES (?, ?, ?, ?)
                """, (
                    estado_atual["umidade_solo"],
                    estado_atual["telhado"],
                    estado_atual["led_verde"],
                    estado_atual["led_vermelho"]
                ))

        except Exception as erro:
            estado_atual["status_conexao"] = str(erro)

            if arduino is not None:
                try:
                    arduino.close()
                except Exception:
                    pass

                arduino = None

            time.sleep(3)


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/api/dados")
def get_dados():
    with sqlite3.connect(DB_PATH) as conn:
        historico = conn.execute("""
            SELECT data_hora, umidade_solo, telhado
            FROM leituras_solo
            ORDER BY id DESC
            LIMIT 20
        """).fetchall()

    return jsonify({
        "atual": estado_atual,
        "historico": historico
    })


@app.route("/api/comando", methods=["POST"])
def enviar_comando():
    if not request.is_json:
        return jsonify({"sucesso": False}), 400

    cmd = request.json.get("comando")

    if cmd not in ["ABRIR", "FECHAR"]:
        return jsonify({
            "sucesso": False,
            "erro": "Comando inválido"
        }), 400

    try:
        with serial_lock:
            if arduino is None or not arduino.is_open:
                return jsonify({
                    "sucesso": False,
                    "erro": "Arduino desconectado"
                }), 503

            arduino.write((cmd + "\n").encode("utf-8"))
            arduino.flush()

        return jsonify({
            "sucesso": True,
            "comando": cmd
        })

    except Exception as erro:
        return jsonify({
            "sucesso": False,
            "erro": str(erro)
        }), 500


if __name__ == "__main__":
    init_db()

    thread = threading.Thread(
        target=ler_serial,
        daemon=True
    )
    thread.start()

    app.run(host="0.0.0.0", port=5000)
