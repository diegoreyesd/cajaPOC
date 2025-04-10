//Prueba todos los sensores conectados al dispositivo
//Incluye una pequeña logica que activa/desactiva el relay1 segun la humedad que lee el sensor de humedad1


#include <Arduino.h>
#include <Wire.h> 
#define HELTEC_WIRELESS_STICK
#include <heltec_unofficial.h>

//  se definene los pines de los sensores
int soilSensor1_pin = 1; //cable amarillo
int soilSensor2_pin = 2; //cable azul
int relay1_pin = 3;      //cable verde
int flowSensor1_pin = 4; //cable purpura

// variables sensor humedad-sustrato 1
const int valorSeco = 2300;
const int valorHumedo = 1070;

//variables sensor humedad-sustrato 2
const int valorSeco2 = 2300;
const int valorHumedo2 = 1070;

//  varialves para sensor de flujo        
float factor_conversion = 7.5; // Factor de conversión de frecuencia a caudal (se puede modificar para calibrar)

//  Funcion para obtener frecuencia de los pulsos
float LeerFlujo() {
  unsigned long duracion = pulseIn(flowSensor1_pin, HIGH, 1000000); // Espera un pulso hasta 1s
  if (duracion == 0) return 0; // Si no hay señal, retorna 0 Hz

  float frecuencia = 1000000.0 / (duracion * 2); // Calcula frecuencia en Hz
  return frecuencia;
}
void setup() {
  heltec_setup();
  Serial.begin(9600);
  Serial.print("inicio Setup()\n");
  pinMode(soilSensor1_pin, INPUT);
  pinMode(soilSensor2_pin, INPUT);
  pinMode(relay1_pin, OUTPUT);
  pinMode(flowSensor1_pin, INPUT_PULLUP); //pin como entrada con resistencia pull-up
  

}
void loop() {
  heltec_loop();
  display.clear();
  // Leer flujo SIN interrupciones
  float frecuencia = LeerFlujo();
  float caudal_L_m = frecuencia / factor_conversion;
  float caudal_L_h = caudal_L_m * 60;
  
  //lee sensor humedad-sustrato 1
  int humedadRaw = analogRead(soilSensor1_pin);
  int humedadPorcentaje = map(humedadRaw, valorSeco, valorHumedo, 0, 100);
  humedadPorcentaje = constrain(humedadPorcentaje, 0, 100);
    // Control del relé según la humedad sensor humedad-sustrato 1
  if (humedadPorcentaje < 50) {
    digitalWrite(relay1_pin, HIGH); // Activa el relay1
    Serial.println("relay1 - HIGH (activado por humedad baja)");
  } else {
    digitalWrite(relay1_pin, LOW); // Desactiva el relay1
    Serial.println("relay1 - LOW (humedad suficiente)");
  }
  
  // Imprime
  display.print("SEN-1: ");           //imprime por display dispositivo 
  display.println(humedadPorcentaje); //imprime por display dispositivo 
  
  Serial.print("Sensor sustrato1: ");
  Serial.print(humedadPorcentaje); Serial.print("% - ");
  Serial.print(humedadRaw); Serial.println(" valor raw");
  //lee sensor humedad-sustrato 2
  humedadRaw = analogRead(soilSensor2_pin);
  humedadPorcentaje = map(humedadRaw, valorSeco2, valorHumedo2, 0, 100);
  humedadPorcentaje = constrain(humedadPorcentaje, 0, 100);
  
  display.print("SEN-2: ");           //imprime por display dispositivo 
  display.println(humedadPorcentaje); //imprime por display dispositivo 
  Serial.print("Sensor sustrato2: ");
  
  Serial.print(humedadPorcentaje); Serial.print("% - ");
  Serial.print(humedadRaw); Serial.println(" valor raw");

  Serial.print("FrecuenciaPulsos: ");
  Serial.print(frecuencia, 0);
  Serial.print(" Hz\tCaudal: ");
  Serial.print(caudal_L_m, 3);
  Serial.print(" L/m\t");
  Serial.print(caudal_L_h, 3);
  Serial.println(" L/h");
  delay(1000);
}