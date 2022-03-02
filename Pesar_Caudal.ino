/*
 * Instrumentación Electrónica _ 2022-01
 * Laboratorio N°1 
 * Profesor: Johann F. Osma
 * Asistente: Sanatiago Tovar Perilla
 * Coautora: Juliana Noguera Contreras
 */

//--------------------- LIBRERIAS -------------------- 
#include "HX711.h"

//--------------------  VARIABLES -------------------- 
#define DOUT  A1
#define CLK  A0

//-------------- Variables para calibracion ---------- 
int Peso_conocido = 50;// en gramos | MODIFIQUE PREVIAMENTE CON EL VALOR DE PESO CONOCIDO!!!
int cantidad_pesos_calibracion = 0; 

//-------------- Variable de confirmacion ------------
String confirmar = "";
String input = "";

float Escala0_100 = -827.421;
float Escala100_200 = -827.339;
float Escala200_300 = -825.173;
float Escala300_400 = -826.410;
float Escala400_500 = -826.931;
float Escala500_600 = -828.671;
float Escala600_700 = -830.720;

float pesoInicial = 0;

HX711 balanza(DOUT, CLK);

//--------------- Variables Caudal -------------------
int motorPin = 9;
int pwm = 0;
float voltage = 0;
long int t1 = 0;
long int t2 = 0;
long int tiempo_dispensar = 15000; // 15 segundos
boolean dispensando = true;

void setup() {
  Calibracion();
  Serial.println(" ");
  Serial.println("¡¡¡LISTO PARA PESAR!!!");  
  Serial.println(" ");
  Serial.println(" ");
  Serial.println("Presione ENTER para iniciar a pesar");
  while (Serial.available() == 0)   {}  
  confirmar = Serial.read(); //Obtener confirmacion.  
  Serial.println(" ");  

  pinMode(motorPin, OUTPUT);
 
  Serial.println("Ingrese PWM");
  while (Serial.available() == 0)   {} 
  String input =  Serial.readString();
  pwm = input.toFloat();
 
  Serial.println("pwm: " + String(pwm) ); //+ "     voltage: " + String(voltage) + "V");
  
  while(pesoInicial == 0){
    pesar();
  }

  analogWrite(motorPin, pwm);
  t1 = millis();
}

void loop() {
  t2 = millis();
  if ((t2 - t1) > tiempo_dispensar){
    analogWrite(motorPin, 0);
    float peso = pesar();
    Serial.println("Total dispensado = " + String(peso - pesoInicial));
  }
}

float pesar(){
  float peso = balanza.get_units(10); // Entrega el peso actualmente medido en gramos
  if(peso<0) peso = peso*-1;
  if(peso<1000){
    if(peso < 98){
      balanza.set_scale(Escala0_100);
      delay(20);
      peso = balanza.get_units(10);
      if(peso<0) peso = peso*-1;
    }else if (peso < 198){
      balanza.set_scale(Escala100_200);
      delay(20);
      peso = balanza.get_units(10);
      if(peso<0) peso = peso*-1;
    }else if (peso < 298){
      balanza.set_scale(Escala200_300);
      delay(20);
      peso = balanza.get_units(10);
      if(peso<0) peso = peso*-1;
    }else if (peso < 398){
      balanza.set_scale(Escala300_400);
      delay(20);
      peso = balanza.get_units(10);
      if(peso<0) peso = peso*-1;
    }else if (peso < 498){
      balanza.set_scale(Escala400_500);
      delay(20);
      peso = balanza.get_units(10);
      if(peso<0) peso = peso*-1;
    }else if (peso < 598){
      balanza.set_scale(Escala500_600);
      delay(20);
      peso = balanza.get_units(10);
      if(peso<0) peso = peso*-1;
    }else {
      balanza.set_scale(Escala600_700);
      delay(20);
      peso = balanza.get_units(10);
      if(peso<0) peso = peso*-1;
    }
    
    if (pesoInicial == 0 && peso > 80){
      pesoInicial = peso;
    }
    Serial.print("Peso: ");
    Serial.print(peso, 1);
    Serial.println(" g");
    delay(200);
  }
  return peso;
}
// FUNCIONES

//Función de Calibración: Permite calibrar la medida de la balanza según un peso de calibración conocido
void Calibracion(void)
{
  Serial.begin(9600);
  Serial.println("~ CALIBRACIÓN DE LA BALANZA ~");
  Serial.println(" ");
  delay(200);
  Serial.println("Procesando escalas");
  Serial.println(" ");
  balanza.set_scale(Escala0_100);  
  balanza.tare(50);  //El peso actual es considerado "Tara". Se toman 20 mediciones del peso actual
  delay(500);
  Serial.print("...Destarando...");  
}
