/*
 * Instrumentación Electrónica _ 2022-01
 * Laboratorio N°1 
 * Profesor: Johann F. Osma
 * Asistente: Sanatiago Tovar Perilla
 * Coautora: Juliana Noguera Contreras
 */

//--------------------- LIBRERIAS -------------------- 
#include "HX711.h"
#include <OneWire.h>
#include <DallasTemperature.h>

//--------------------  VARIABLES -------------------- 
#define DOUT  A1
#define CLK  A0

//-------------- Variable de confirmacion ------------
String confirmar = "";
String input = "";
int opcion = 0;

//----------- Variables del motor paso a paso --------
#define StepPin 4
#define ms1 5
#define ms2 6
#define ms3 7
unsigned int t = 980; // Rango: 980 - 9800 que corresponde a 150 - 15 RPMs aprox.
#define Encoder_pin 2      // The pin the encoder is connected 
#define PulsosRotacion 20  // Value of pluses per rotation
int Encoder = 0;           // Value of the Output of the encoder 
int Estado = 1;            // Current state of the encoder to recognize the pulse changes
int Pulsos = 0;            // Counter of pulses
unsigned long InitialTime = 0;
unsigned long FinalTime; 
float RPMs;
float rpmIngresado = 0;

//------------ Variables sensor de tempeura ----------
const int pinDatosDQ = 11; // Pin donde se conecta el bus 1-Wire

OneWire oneWireObjeto(pinDatosDQ);                  // Instancia a la clase OneWire
DallasTemperature sensorDS18B20(&oneWireObjeto);    // Instancia a la clase DallasTemperature


//----- Escalas de la calibracion de la balanza ------
float Escala0_100 = -827.421;
float Escala100_200 = -827.339;
float Escala200_300 = -825.173;
float Escala300_400 = -826.410;
float Escala400_500 = -826.931;
float Escala500_600 = -828.671;
float Escala600_700 = -830.720;

//-------------- Variables de los pesos --------------
float pesoRecipiente = 0; // se almacena el peso del recipiente medido
float pesoMaicena = 0; // se calcula el peso de la maicena depositado
float pesoRecipienteMaicena = 0; // peso del recipiente mas el peso de la maicena
float pesoLiquido = 0; // peso del agua dispensada

HX711 balanza(DOUT, CLK);

//--------------- Variables Caudal -------------------
int motorPin = 9;
int min_pwm = 200;

//---------- Variables Mini bomba de agua ------------
int Volumen_deseado = 0;// en mL valor ingresado por el usuario
float caudal_Max = 10.682;
float caudal_Min = 5.116;

boolean fin = false; // indica si ya se ha depositado el agua requerida

void setup() {
  Serial.begin(9600);
  //========== set up motor paso a paso =========
  pinMode( ms1 , OUTPUT);
  pinMode( ms2 , OUTPUT);
  pinMode( ms3 , OUTPUT);
  pinMode( StepPin , OUTPUT);
  pinMode( Encoder_pin , INPUT);

  //=============== set up balanza ==============
  Serial.println("Iniciando calibración...");
  Serial.println("No coloque ningún elemento en la balanza");
  Calibracion(); // Se realiza la calibracion de la balanza
  Serial.println(" ");
  Serial.println("CALIBRACIÓN FINALIZADA!");
  Serial.println(" ");  
}


void loop() {
  Serial.println(" ");
  Serial.println("Ingrese el número de la acción que desea realizar");
  Serial.println("1. Guardar el peso del reactor");
  Serial.println("2. Añadir soluto (maicena)");
  Serial.println("3. Iniciar a pesar");
  Serial.println("4. Dispensar agua");
  Serial.println("5. Revolver dado un valor de RPMs");
  Serial.println("6. Sensar temperatura");
  Serial.println("7. Balanza Mezclar Temperatura");
  
  while (Serial.available() == 0)   {}  // Se decide la acción que se va a realizar
  input =  Serial.readString();
  opcion = input.toInt();

  switch (opcion) {
    case 1:
        calcular_peso_reactor();
        break;
    case 2:
        anadir_soluto();
        break;
    case 3:
        iniciar_pesar();
        break;
    case 4:
        dispensar_agua();
        break;
    case 5:
        revolver_dado_rpms();
        break;
    case 6:
        sensar_temperatura();
        break;
    case 7:
        balanza_mezclar_temperatura();  
        break;
    default:
        Serial.println("Ingrese una opción valida");
        break;
  }
}

void calcular_peso_reactor(){
    Serial.println(" ");
    Serial.println("Coloque el reactor en la balanza y presione ENTER para confirmar");
    Serial.println(" "); 
    while (Serial.available() == 0)   {}  
    confirmar = Serial.read(); // Una vez se tiene el reactor en la balanza se inicia a pesar

    Serial.println("Presione ENTER para almacenar el peso del reactor");
    Serial.println(" "); 
    Serial.println("Peso reactor: "); 
    while (Serial.available() == 0) {
        pesoRecipiente = pesar();
        Serial.println(String(pesoRecipiente) + " g"); 
        delay(300);
    } 
    pesoRecipienteMaicena = pesoMaicena + pesoRecipiente;
    confirmar = Serial.read();
}

void anadir_soluto(){
    Serial.println(" ");
    Serial.println("Presione ENTER e inicie a añadir el soluto");
    Serial.println(" ");

    while (Serial.available() == 0)   {}  
    confirmar = Serial.read(); // Confirmacion

    Serial.println("Presione ENTER para almacenar el peso del soluto");
    Serial.println(" "); 
    Serial.println("Peso soluto: ");

    while (Serial.available() == 0) {
        pesoMaicena = pesar() - pesoRecipiente;
        Serial.println(String(pesoMaicena) + " g"); 
        delay(300);
    } 
    pesoRecipienteMaicena = pesoMaicena + pesoRecipiente;
    confirmar = Serial.read();
}

void iniciar_pesar(){
    Serial.println(" ");
    Serial.println("Presione ENTER para iniciar a pesar");
    Serial.println(" ");

    while (Serial.available() == 0)   {}  
    confirmar = Serial.read(); // Confirmacion

    Serial.println("Presione ENTER para detener el proceso");
    Serial.println(" "); 

    while (Serial.available() == 0) {
        Serial.print("Reactor: ");
        Serial.print(pesoRecipiente);
        Serial.print("g \t Soluto: ");
        Serial.print(pesoMaicena);
        Serial.print("g \t\t Actual: ");
        Serial.print(pesar());
        Serial.println("g");
        delay(300);
    }
    confirmar = Serial.read();
}

void dispensar_agua(){
    Serial.println(" ");
    Serial.println("Ingrese Volumen en ml");
    while (Serial.available() == 0)   {}  // Se ingresa el valor en ml de la cantidad de agua que se debe dosificar
    input =  Serial.readString();
    Volumen_deseado = input.toFloat();
    Volumen_deseado = Volumen_deseado + 37.5;  // En las pruebas se halla que siempre se tiene un error lineal de 38ml
                                            // por lo cual se indica que se debe depositar 38ml menos de lo solicitado
    Serial.println(" ");
    Serial.println("Iniciando Dosificación...");

    while(!fin){
        DosificarB();
    }
    pesoLiquido = pesar() - pesoRecipienteMaicena;// Peso del liquido actualmente medido en gramos
    Serial.println(" ");
    Serial.println("Total agua dispensada: " + String(pesoLiquido) + " ml");
}

void revolver_dado_rpms(){
    Serial.println(" ");
    Serial.println("Ingrese valor de RPMs");
    while (Serial.available() == 0)   {}  // Se ingresa el valor de RPMs
    input =  Serial.readString();
    rpmIngresado = input.toDouble();
    float calc_tiempo = float(149094) * pow(rpmIngresado, -1.002); // funcion obtenida de la caracterizacion del motor
    t = int(calc_tiempo);
    Serial.println("Valor de t calculado: " + String(t));

    Serial.println("Iniciando el motor...");
    ControlMotorPasoaPaso();
    
    Serial.println("Presione ENTER para detener");
    while (Serial.available() == 0) {
        activarMotorPasoPaso();
    }
    confirmar = Serial.read();
}

void sensar_temperatura(){
    Serial.println(" ");
    Serial.println("Presione ENTER para iniciar a sensar la temperatura");
    Serial.println(" ");

    sensorDS18B20.begin();

    while (Serial.available() == 0)   {}  
    confirmar = Serial.read(); // Confirmacion

    Serial.println("Presione ENTER para detener el proceso");
    Serial.println(" "); 

    while (Serial.available() == 0) {
        sensorDS18B20.requestTemperatures();// Mandamos comandos para toma de temperatura a los sensores
        Serial.print("Temperatura sensor 1: "); 
        Serial.print(sensorDS18B20.getTempCByIndex(0));// Leemos y mostramos los datos de los sensores DS18B20
        Serial.println(" C");
        delay(1000);
    }
    confirmar = Serial.read();
}

// Se indica a la bomba que debe entregar X cantidad de agua
void DosificarB(void){
    pesoLiquido = pesar() - pesoRecipienteMaicena;// Peso del liquido actualmente medido en gramos
    boolean finAgua = false;
    while ((Volumen_deseado) + 0.5 >= pesoLiquido && !finAgua){ // Se entrega agua hasta que falten 1 gramos
        if( pesoLiquido >=  Volumen_deseado*0.8){ // Se entrega agua a la minima velocidad si ya se ha depositado mas del 90% del valor solicitado
        analogWrite(motorPin, min_pwm);
        }else{
        analogWrite(motorPin, 254); // Se entrega agua a la maxima velocidad si se ha depositado menos del 80% del valor solicitado
        }
        pesoLiquido = pesar() - pesoRecipienteMaicena;  
        Serial.println("Volumen: " + String(pesoLiquido)+ " ml");  // Se indica la cantidad de agua depositada

        if(pesoLiquido >= Volumen_deseado - 38){
          Serial.println("entra");
          finAgua = true;
          fin = true;
          analogWrite(motorPin, 0);
        }
    }

    if ((Volumen_deseado) + 0.1 <= pesoLiquido){ // Si se ha entregado la cantidad de agua solicitada menos 0.1ml se detiene la bomba
        analogWrite(motorPin, 0);
        fin = true;
        pesoLiquido = pesar() - pesoRecipienteMaicena;  
        Serial.println("\t Volumen: " + String(pesoLiquido)+ " ml"); // Se indica la cantidad de agua depositada
    }
}

// Calcula el peso actual y lo calcula de acuerdo a la escala
float pesar(){
  float peso = balanza.get_units(10); // Entrega el peso actualmente medido en gramos
  if(peso<0) peso = peso*-1;
  if(peso<1000){  
    if(peso < 98){ // Se cambia la escala a menor a 100g
      balanza.set_scale(Escala0_100);
      peso = balanza.get_units(10);
      if(peso<0) peso = peso*-1;
    }else if (peso < 198){ // Se cambia la escala a 100 - 200
      balanza.set_scale(Escala100_200);
      peso = balanza.get_units(10);
      if(peso<0) peso = peso*-1;
    }else if (peso < 298){ // Se cambia la escala a 200 - 300
      balanza.set_scale(Escala200_300);
      peso = balanza.get_units(10);
      if(peso<0) peso = peso*-1;
    }else if (peso < 398){ // Se cambia la escala a 300 - 400
      balanza.set_scale(Escala300_400);
      peso = balanza.get_units(10);
      if(peso<0) peso = peso*-1;
    }else if (peso < 498){ // Se cambia la escala a 400 - 500
      balanza.set_scale(Escala400_500);
      peso = balanza.get_units(10);
      if(peso<0) peso = peso*-1;
    }else if (peso < 598){ // Se cambia la escala a 500 - 600
      balanza.set_scale(Escala500_600);
      peso = balanza.get_units(10);
      if(peso<0) peso = peso*-1;
    }else { // Se cambia la escala a mayor de 600
      balanza.set_scale(Escala600_700);
      peso = balanza.get_units(10);
      if(peso<0) peso = peso*-1;
    }
  }
  return peso;
}

//Función de Calibración: Permite calibrar la balanza con los valores de escala especificados
void Calibracion(void){
  Serial.println("~ CALIBRACIÓN DE LA BALANZA ~");
  Serial.println(" ");
  delay(100);
  Serial.println("Procesando escalas");
  Serial.println(" ");
  balanza.set_scale(Escala0_100);  // Inicialmente se asigna la menor escala y posteriormente se ajusta para mejorar la precision
  balanza.tare(50);  //El peso actual es considerado "Tara". Se toman 50 mediciones del peso actual
  delay(200);
  Serial.print("...Destarando...");  
}

void activarMotorPasoPaso(void){
  digitalWrite(StepPin, HIGH); 
  delayMicroseconds(t);           
  digitalWrite(StepPin, LOW);  
  delayMicroseconds(t);
  
  Encoder = digitalRead(Encoder_pin);  
  if(Encoder == LOW && Estado == 1)  {
     Estado = 0;             
  }
  if(Encoder == HIGH && Estado == 0)  {
     Pulsos++; 
     Estado = 1;     
  }
  if(Pulsos == PulsosRotacion)  {
    FinalTime = millis();
    RPMs = 60000/(FinalTime - InitialTime);
    Pulsos = 0; 
    InitialTime = FinalTime;
    Serial.print("RPM = "); // Se muestra en el puerto serial el valor medido de RPMs
    Serial.println(RPMs);
  }
}

//Función para Controlar el Motor Paso a Paso: Permite controlar el motor y los pasos de este
void ControlMotorPasoaPaso(void) {
  digitalWrite(ms1, LOW);
  digitalWrite(ms2, LOW);
  digitalWrite(ms3, LOW);
}

// funcion que permite pesar, mezclar y calcular temperatura de manera simultanea
void balanza_mezclar_temperatura(void) {
    int contador = 0;
    Serial.println(" ");
    Serial.println("Ingrese valor de RPMs");
    while (Serial.available() == 0)   {}  // Se ingresa el valor de RPMs
    input =  Serial.readString();
    rpmIngresado = input.toDouble();
    float calc_tiempo = float(149094) * pow(rpmIngresado, -1.002); // funcion obtenida de la caracterizacion del motor
    t = int(calc_tiempo);
    
    Serial.println("Iniciando el motor...");
    ControlMotorPasoaPaso();

    float pesoActual = pesar();
    sensorDS18B20.begin();
    sensorDS18B20.requestTemperatures();
    float temperaturaActual = sensorDS18B20.getTempCByIndex(0);

    Serial.println("Presione ENTER para detener");
    while (Serial.available() == 0) {
      if(contador == 4000){
        pesoActual = balanza.get_units(10);
        contador = 0;
      }
      if(contador == 2000){
        sensorDS18B20.requestTemperatures();
        temperaturaActual = sensorDS18B20.getTempCByIndex(0); 
      }
      activarMotorPasoPasoPrint(temperaturaActual, pesoActual);
      contador += 1;
    }
    confirmar = Serial.read();
}

void activarMotorPasoPasoPrint(float temperaturaA, float pesoA){
  digitalWrite(StepPin, HIGH); 
  delayMicroseconds(t);           
  digitalWrite(StepPin, LOW);  
  delayMicroseconds(t);
  
  Encoder = digitalRead(Encoder_pin);  
  if(Encoder == LOW && Estado == 1)  {
     Estado = 0;             
  }
  if(Encoder == HIGH && Estado == 0)  {
     Pulsos++; 
     Estado = 1;     
  }
  if(Pulsos == PulsosRotacion)  {
    FinalTime = millis();
    RPMs = 60000/(FinalTime - InitialTime);
    Pulsos = 0; 
    InitialTime = FinalTime;
    Serial.print("RPM = "); // Se muestra en el puerto serial el valor medido de RPMs
    Serial.print(RPMs);
    Serial.print(" \t Temperatura: ");
    Serial.print(temperaturaA);
    Serial.print(" \t Peso: ");
    Serial.println(pesoA);
  }
}
