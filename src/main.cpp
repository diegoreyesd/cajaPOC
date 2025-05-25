#include <Arduino.h>
#include <Wire.h> 
#define HELTEC_WIRELESS_STICK

// Definición de pines para los sensores
const int SENSOR1_PIN = 1;    // GPIO1
const int SENSOR2_PIN = 2;    // GPIO2
const int RELAY_PIN = 3;      // GPIO3 para el relé
const int FLOW_PIN = 4;       // GPIO4 para el sensor de flujo

// Variables para almacenar los valores
int sensor1Value = 0;
int sensor2Value = 0;
int sensor1Percent = 0;
int sensor2Percent = 0;

// Variables para el sensor de flujo
volatile long pulseCount = 0;
float flowRate = 0.0;
unsigned long oldTime = 0;
const float FLOW_CALIBRATION = 7.5;  // Factor de calibración del YF-S201

// Rangos de los sensores
const int SENSOR1_DRY = 2500;    // Valor cuando está seco
const int SENSOR1_WET = 1125;    // Valor cuando está mojado
const int SENSOR2_DRY = 3360;    // Valor cuando está seco
const int SENSOR2_WET = 1225;    // Valor cuando está mojado

// Umbral de humedad para activar el relé
const int HUMIDITY_THRESHOLD = 50;

// Función de interrupción para contar pulsos
void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

void setup() {
  // Inicializar comunicación serial
  Serial.begin(9600);
  Serial.println("Iniciando lectura de sensores...");
  Serial.println("Sensor sustrato1          Sensor sustrato2          Caudal");
  Serial.println("--------------------------------------------------------");
  
  // Configurar pines como entradas con pull-down
  pinMode(SENSOR1_PIN, INPUT_PULLDOWN);
  pinMode(SENSOR2_PIN, INPUT_PULLDOWN);
  
  // Configurar pin del relé como salida
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Iniciar con el relé apagado
  
  // Configurar pin del sensor de flujo
  pinMode(FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), pulseCounter, FALLING);
}

void loop() {
  // Leer valores de los sensores
  sensor1Value = analogRead(SENSOR1_PIN);
  sensor2Value = analogRead(SENSOR2_PIN);
  
  // Calcular porcentajes usando los rangos específicos
  sensor1Percent = map(sensor1Value, SENSOR1_DRY, SENSOR1_WET, 0, 100);
  sensor2Percent = map(sensor2Value, SENSOR2_DRY, SENSOR2_WET, 0, 100);
  
  // Asegurar que los porcentajes estén dentro del rango 0-100
  sensor1Percent = constrain(sensor1Percent, 0, 100);
  sensor2Percent = constrain(sensor2Percent, 0, 100);
  
  // Control del relé basado en el promedio de humedad
  int averageHumidity = (sensor1Percent + sensor2Percent) / 2;
  if (averageHumidity <= HUMIDITY_THRESHOLD) {
    digitalWrite(RELAY_PIN, HIGH);  // Activar relé cuando está seco
  } else {
    digitalWrite(RELAY_PIN, LOW);   // Desactivar relé cuando está húmedo
  }
  
  // Calcular caudal cada segundo
  if ((millis() - oldTime) >= 1000) {
    // Desactivar interrupciones mientras calculamos
    detachInterrupt(digitalPinToInterrupt(FLOW_PIN));
    
    // Calcular caudal en L/min
    flowRate = (pulseCount * 60.0) / FLOW_CALIBRATION;
    
    // Reiniciar contador
    pulseCount = 0;
    
    // Reactivar interrupciones
    attachInterrupt(digitalPinToInterrupt(FLOW_PIN), pulseCounter, FALLING);
    
    // Actualizar tiempo
    oldTime = millis();
  }
  
  // Imprimir resultados en columnas
  if (sensor1Value < 1000) {
    Serial.print(" ");
  }
  Serial.print(sensor1Value);
  Serial.print(" (");
  Serial.print(sensor1Percent);
  Serial.print("%)        ");
  if (sensor2Value < 1000) {
    Serial.print(" ");
  }
  Serial.print(sensor2Value);
  Serial.print(" (");
  Serial.print(sensor2Percent);
  Serial.print("%)    ");
  Serial.print(flowRate, 2);  // Mostrar caudal con 2 decimales
  Serial.print(" L/min    Relé: ");
  Serial.println(averageHumidity <= HUMIDITY_THRESHOLD ? "ON" : "OFF");
  
  // Esperar 1 segundo antes de la siguiente lectura
  delay(500);
} 