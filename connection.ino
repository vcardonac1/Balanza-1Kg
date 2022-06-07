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
#include <SoftwareSerial.h>

//--------------------  VARIABLES -------------------- 
#define DOUT  A1
#define CLK  A0

//-------------- Variable de confirmacion ------------
String confirmar = "";
String input = "";

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

HX711 balanza(DOUT, CLK);

//--------------- Variables Caudal -------------------
int motorPin = 9;
int min_pwm = 210;

//---------- Variables Mini bomba de agua ------------
int Volumen_deseado = 0;// en mL valor ingresado por el usuario
float caudal_Max = 10.682;
float caudal_Min = 5.116;

boolean fin = false; // indica si ya se ha depositado el agua requerida

//------------------- Bluetooth -----------------------
SoftwareSerial connection(12,13);
int opcion = 0;
String message = "";
float temp;
float peso;
float pesoRecipiente = 0; // se almacena el peso del recipiente medido
float pesoMaicena = 0; // se calcula el peso de la maicena depositado
float pesoRecipienteMaicena = 0; // peso del recipiente mas el peso de la maicena
float pesoLiquido = 0; // peso del agua dispensada
bool accion = true;
float RPMpromedio = 0;
int cont = 0;

void setup() {
  Serial.begin(9600);
  connection.begin(9600);
  inicializarSistema();
}

void loop(){
    pesar();
    sensar_temperatura();
    sendPesoTemp();
    delay(400);

    if(connection.available()){
        message = connection.readString();
        Serial.println("Recibido: " + message);

        if(message == "Stop"){
            message = "";
            detener();
        }
        else if(message.startsWith("d")){
            message.remove(0,1);
            Serial.println("--" + message);
            Volumen_deseado = message.toInt();
            Serial.println("Volumen deseado: " + String(Volumen_deseado));
            DosificarB();
        }
        else if(message.startsWith("z")){
            message.remove(0,1);
            int aux = message.indexOf(";");
            double in_rpm = message.substring(0,aux).toDouble();
            int in_time = message.substring(aux+2).toInt();
            Serial.println("RPMs: " + String(in_rpm) + " Time: " + String(in_time));
            revolver_dado_rpms(in_rpm, in_time);
            message = "";
        }
        else if(message == "envase"){
            message = "";
            pesoRecipiente = pesar();
            pesoRecipienteMaicena = pesoRecipiente;
            delay(200);
            String str = "E|"+String(pesoRecipiente)+"|E";
            int str_len = str.length() + 1;
            char char_array[str_len];
            str.toCharArray(char_array, str_len);
            connection.write(char_array);
            Serial.println("\tEnviado:" + str);
            delay(200);
        }
        else if(message == "maicena"){
            message = "";
            pesoMaicena = pesar() - pesoRecipiente;
            pesoRecipienteMaicena = pesoRecipiente + pesoMaicena;
            Serial.println("Envase: " + String(pesoRecipiente) + " maicena: " + String(pesoMaicena));
        }
        else if(message == "calibrar"){
            message = "";
            Calibracion();
        }
    }
}

void inicializarSistema(){
  //========== set up motor paso a paso =========
  pinMode( ms1 , OUTPUT);
  pinMode( ms2 , OUTPUT);
  pinMode( ms3 , OUTPUT);
  pinMode( StepPin , OUTPUT);
  pinMode( Encoder_pin , INPUT);
  sensorDS18B20.begin();

  //=============== set up balanza ==============
  Serial.println("Iniciando calibración...");
  Serial.println("No coloque ningún elemento en la balanza");
  Calibracion();
  Serial.println(" ");
  Serial.println("CALIBRACIÓN FINALIZADA!");
  Serial.println(" ");  
  ControlMotorPasoaPaso();
}

float pesar(){
  peso = balanza.get_units(10); // Entrega el peso actualmente medido en gramos
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

void sensar_temperatura(){
    sensorDS18B20.requestTemperatures();// Mandamos comandos para toma de temperatura a los sensores
    temp = sensorDS18B20.getTempCByIndex(0); // Leemos y mostramos los datos de los sensores DS18B20
}

void detener(void){
  String str = "I|STOP|STOP|F";
  
  int str_len = str.length() + 1; 
  char char_array[str_len];

  str.toCharArray(char_array, str_len);
  connection.write(char_array);
  Serial.println("\tEnviado:" + str);
  delay(500000000000000);
}

void DosificarB(void){
    pesoLiquido = pesar() - pesoRecipienteMaicena;// Peso del liquido actualmente medido en gramos
    boolean finAgua = false;
    float volIdeal = Volumen_deseado;
    Volumen_deseado = Volumen_deseado + 17.0;
    while (Volumen_deseado + 0.5 >= pesoLiquido && !finAgua && !connection.available()){ // Se entrega agua hasta que falten 1 gramos
        if( pesoLiquido >=  Volumen_deseado*0.85){ // Se entrega agua a la minima velocidad si ya se ha depositado mas del 90% del valor solicitado
        analogWrite(motorPin, min_pwm);
        Serial.println("\t\tMinima Velocidad");
        }else{
        analogWrite(motorPin, 254); // Se entrega agua a la maxima velocidad si se ha depositado menos del 80% del valor solicitado
        }
        pesoLiquido = pesar() - pesoRecipienteMaicena;  
        Serial.println("Volumen: " + String(pesoLiquido)+ " ml");  // Se indica la cantidad de agua depositada

        if(pesoLiquido >= volIdeal - 2){
          Serial.println("FIN DISPENSAR");
          finAgua = true;
          fin = true;
          analogWrite(motorPin, 0);

          String str = "D|"+String(pesoLiquido)+"|F";
          int str_len = str.length() + 1;
          char char_array[str_len];
          str.toCharArray(char_array, str_len);
          connection.write(char_array);
          Serial.println("\tEnviado vol :" + str);
        }
    }

    if (connection.available()){
        String message = connection.readString();
 
        if(message == "Stop"){ //lo que se lee
            analogWrite(motorPin, 0);
            detener();
        }
    }

    if ((Volumen_deseado) + 0.1 <= pesoLiquido){ // Si se ha entregado la cantidad de agua solicitada menos 0.1ml se detiene la bomba
        analogWrite(motorPin, 0);
        fin = true;
        pesoLiquido = pesar() - pesoRecipienteMaicena;  
  
        String str = "D|"+String(pesoLiquido)+"|F";
        int str_len = str.length() + 1;
        char char_array[str_len];
        str.toCharArray(char_array, str_len);
        connection.write(char_array);
        Serial.println("\tEnviado vol :" + str);
    }
    message = "";
}

void revolver_dado_rpms(double in_rpm, int in_time){
    float calc_tiempo = float(149094) * pow(in_rpm, -1.002); // funcion obtenida de la caracterizacion del motor
    t = int(calc_tiempo);
    Serial.println("Valor de t calculado: " + String(t));
    Serial.println("Iniciando el motor...");

    sendPesoTemp();

    uint32_t startTime;
    uint32_t endTime;
    startTime = millis();
    endTime = startTime;
    RPMpromedio = 0;
    cont = 0;
    while ( (endTime - startTime <= (in_time*1000))&& !connection.available()){
        activarMotorPasoPaso();
        endTime = millis();
    }  
    if (connection.available()){
        String message = connection.readString();
 
        if(message == "Stop"){ //lo que se lee
            detener();
        }
    }
    RPMpromedio = RPMpromedio/cont;

    delay(1000);
    String str = "R|" + String(RPMpromedio);
    int str_len = str.length() + 1;
    char char_array[str_len];
    str.toCharArray(char_array, str_len);
    connection.write(char_array);
    Serial.println("\tEnviado:" + str);    
    delay(1000);
    
    sendPesoTemp();
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
    cont += 1;
    RPMpromedio += RPMs;
    Pulsos = 0; 
    InitialTime = FinalTime;
    Serial.print("RPM = "); // Se muestra en el puerto serial el valor medido de RPMs
    Serial.println(RPMs);
  }
}

void sendPesoTemp(void){
    String str = "I|"+String(peso)+"|"+String(temp,2)+"|F";
    int str_len = str.length() + 1;
    char char_array[str_len];
    str.toCharArray(char_array, str_len);
    connection.write(char_array);
    Serial.println("\tEnviado:" + str);
}

//Función para Controlar el Motor Paso a Paso: Permite controlar el motor y los pasos de este
void ControlMotorPasoaPaso(void) {
  digitalWrite(ms1, LOW);
  digitalWrite(ms2, LOW);
  digitalWrite(ms3, LOW);
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
  delay(100);
  Serial.print("...Destarando...");  


  delay(500);
  String str = "C|C";
  int str_len = str.length() + 1;
  char char_array[str_len];
  str.toCharArray(char_array, str_len);
  connection.write(char_array);
  Serial.println("\tEnviado:" + str);
  delay(500);
}
