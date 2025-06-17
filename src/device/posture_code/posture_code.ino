#include "posture_model.h"
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <TensorFlowLite_ESP32.h>

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#define SEQ_LENGTH 11
#define N_FEATURES 6
#define N_INPUTS (SEQ_LENGTH * N_FEATURES)
#define N_OUTPUTS 3
#define TENSOR_ARENA_SIZE (24 * 1024)
#define VIBRATOR_PIN 4

float inputBuffer[SEQ_LENGTH][N_FEATURES] = {0};
int inputIndex = 0;
int strideCounter = 0;
bool sequenceReady = false;
String current_posture = "unknown";
String last_sent_posture = "";

tflite::MicroInterpreter* interpreter;
tflite::ErrorReporter* error_reporter;
tflite::AllOpsResolver resolver;
tflite::MicroErrorReporter micro_error_reporter;

const tflite::Model* model = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

uint8_t tensor_arena[TENSOR_ARENA_SIZE];

const char* ssid = "Galaxy A71 6EA4";
const char* password = "yidq0453";
//MQTT
const char* mqtt_server = "192.168.99.11"; 

WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(80);

Adafruit_MPU6050 mpu;

void handlePosture() {
  String html_page = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='3'>";
  html_page += "<style>body { font-family: Arial; text-align: center; margin-top: 50px; }";
  html_page += ".status { font-size: 2em; padding: 20px; border-radius: 10px; display: inline-block; }";
  if (current_posture == "tegak") {
    html_page += ".status { background-color: #4CAF50; color: white; }";
  } else if (current_posture == "miring") {
    html_page += ".status { background-color: #ff9800; color: white; }";
  } else {
    html_page += ".status { background-color: #f44336; color: white; }";
  }
  html_page += "</style></head><body>";
  html_page += "<h1>Status Postur Anda:</h1>";
  html_page += "<div class='status'>" + current_posture + "</div>";
  html_page += "</body></html>";

  server.send(200, "text/html", html_page);
}

void setupModel() {
  error_reporter = &micro_error_reporter;
  model = tflite::GetModel(posture_model_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report("Model schema version mismatch!");
    while (1);
  }

  interpreter = new tflite::MicroInterpreter(model, resolver, tensor_arena, TENSOR_ARENA_SIZE, error_reporter);

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    error_reporter->Report("AllocateTensors() failed");
    while (1);
  }

  input = interpreter->input(0);
  output = interpreter->output(0);
}
//Setup MQTT
void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32PostureClient")) {
      Serial.println("Connected to MQTT broker");
    } else {
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    while (1) delay(10);
  }

  pinMode(VIBRATOR_PIN, OUTPUT);
  digitalWrite(VIBRATOR_PIN, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi Connected!");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);

  setupModel();
  server.on("/posture", handlePosture);
  server.begin();
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
  server.handleClient();

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  inputBuffer[inputIndex][0] = a.acceleration.x;
  inputBuffer[inputIndex][1] = a.acceleration.y;
  inputBuffer[inputIndex][2] = a.acceleration.z;
  inputBuffer[inputIndex][3] = g.gyro.x;
  inputBuffer[inputIndex][4] = g.gyro.y;
  inputBuffer[inputIndex][5] = g.gyro.z;

  inputIndex++;

  if (inputIndex >= SEQ_LENGTH) {
    int index = 0;
    for (int i = 0; i < SEQ_LENGTH; i++) {
      for (int j = 0; j < N_FEATURES; j++) {
        input->data.f[index++] = inputBuffer[i][j];
      }
    }

    if (interpreter->Invoke() == kTfLiteOk) {
      int maxIndex = 0;
      float maxScore = output->data.f[0];
      for (int i = 1; i < N_OUTPUTS; i++) {
        if (output->data.f[i] > maxScore) {
          maxScore = output->data.f[i];
          maxIndex = i;
        }
      }

      current_posture = (maxIndex == 0) ? "bungkuk" : (maxIndex == 1) ? "miring" : "tegak";
      digitalWrite(VIBRATOR_PIN, current_posture == "tegak" ? LOW : HIGH);

      // Create JSON payload with posture + sensor data
      char payload[256];
      snprintf(payload, sizeof(payload),
        "{\"kelas\":\"%s\", \"accel\":[%.2f, %.2f, %.2f], \"gyro\":[%.2f, %.2f, %.2f]}",
        current_posture.c_str(),
        a.acceleration.x, a.acceleration.y, a.acceleration.z,
        g.gyro.x, g.gyro.y, g.gyro.z
      );

      // Publish to single topic
      client.publish("posture/data", payload);
      Serial.println("Published: " + String(payload));

      last_sent_posture = current_posture;
    }

    // Shift buffer for next prediction
    for (int i = 1; i < SEQ_LENGTH; i++) {
      for (int j = 0; j < N_FEATURES; j++) {
        inputBuffer[i - 1][j] = inputBuffer[i][j];
      }
    }
    inputIndex = SEQ_LENGTH - 1;
  }

  delay(100);
}