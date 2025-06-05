from flask import Flask, jsonify, render_template
import threading
import json
import paho.mqtt.client as mqtt

app = Flask(__name__)

MQTT_BROKER = "192.168.99.11" 
MQTT_PORT = 1883
MQTT_TOPIC = "posture/data"

latest_posture_data = {}
data_lock = threading.Lock()

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker with result code " + str(rc))
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    global latest_posture_data
    try:
        payload = msg.payload.decode()
        data = json.loads(payload)
        with data_lock:
            latest_posture_data = data
        print(f"Received posture data: {data}")
    except Exception as e:
        print("Error parsing MQTT message:", e)

def start_mqtt():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    try:
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        client.loop_forever()
    except Exception as e:
        print(f"MQTT connection error: {e}")

@app.route('/')
def index():
    return render_template('dashboard.html')

@app.route('/api/posture')
def api_posture():
    with data_lock:
        data_copy = latest_posture_data.copy() if latest_posture_data else {}
    return jsonify(data_copy or {"posture": "No data"})

if __name__ == '__main__':
    mqtt_thread = threading.Thread(target=start_mqtt)
    mqtt_thread.daemon = True
    mqtt_thread.start()
    app.run(host='127.0.0.1', port=8080, debug=True)