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
int min_pwm = 175;

//Variables para mini bomba de agua
int Volumen_deseado = 0;// en mL | MODIFIQUE PREVIAMENTE CON EL VALOR DE VOLUMEN DESEADO!!!
float caudal_Max = 10.682;
float caudal_Min = 5.116;

boolean fin = false;

void setup() {
  Calibracion();
  Serial.println(" ");
  Serial.println("¡¡¡LISTO PARA PESAR!!!");  
  Serial.println(" ");
  Serial.println(" ");
  Serial.println("Presione ENTER para conocer el peso inicial");
  while (Serial.available() == 0)   {}  
  confirmar = Serial.read(); //Obtener confirmacion.  

  
  //while(pesoInicial == 0){
    //pesar();
  //}
  Serial.println(" ");
  Serial.println("Peso Inicial = " + String(pesoInicial) + "g");
  
  Serial.println(" ");  
  pinMode(motorPin, OUTPUT);

  Serial.println("Ingrese Volumen en ml");
  while (Serial.available() == 0)   {} 
  String input =  Serial.readString();
  Volumen_deseado = input.toFloat();
  Volumen_deseado = Volumen_deseado - 7;
  
  Serial.println(" ");
  Serial.println("Iniciando Dosificación...");
}

void loop() {
  //float peso = pesar() - pesoInicial;
  //Serial.println("peso: " + String(peso) + "g");
  if(!fin){
    DosificarB(); 
  }else{  
    Serial.print("------ Cantidad: " + String(pesar() - pesoInicial)+ " ml"); 
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
void Calibracion(void){
  Serial.begin(9600);
  Serial.println("~ CALIBRACIÓN DE LA BALANZA ~");
  Serial.println(" ");
  delay(100);
  Serial.println("Procesando escalas");
  Serial.println(" ");
  balanza.set_scale(Escala0_100);  
  balanza.tare(50);  //El peso actual es considerado "Tara". Se toman 20 mediciones del peso actual
  delay(200);
  Serial.print("...Destarando...");  
}

void DosificarB(void)
{
  float pesoLiquido = pesar() - pesoInicial;// Entrega el peso del liquido actualmente medido en gramos

  //while ((Volumen_deseado) + 0.5 >= pesoLiquido){
  
  while ((Volumen_deseado) + 0.5 >= pesoLiquido){
    if( pesoLiquido >=  Volumen_deseado*0.8){
      analogWrite(motorPin, min_pwm);
    }else{
      analogWrite(motorPin, 254); 
    }
    pesoLiquido = pesar() - pesoInicial;  
    Serial.print("------ Cantidad: " + String(pesoLiquido)+ " ml");  
  }

  if ((Volumen_deseado) + 0.1 <= pesoLiquido){
    analogWrite(motorPin, 0);
    fin = true;
    pesoLiquido = pesar() - pesoInicial;  
    Serial.print("------ Cantidad: " + String(pesoLiquido)+ " ml"); 
  }
}
