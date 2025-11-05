#include <Stepper.h>
#include "quirc.h"
#include <WiFi.h>
#include "HTTPClient.h"

//Link para enviar a informação.
//Como ele funciona:"http://IP do computador/pasta do xampp/nome do arquivo"
String URL = "http://192.168.0.198/horus/ESP/esp_conection_x.php";

int httpCode = 0;


const char* ssid = "FSA-SALA20";
const char* password = "sl20@igv24*";


//const char* ssid = "MarcosNet";
//const char* password = "oiiiiiii";

const int SMdir = 22;          // Pino do sensor magnético da porta 22
const int SMesq = 21;          // Pino do sensor magnético da porta 22
const int B = 2;

const int SP = 23;             // Pino do sensor de presença da porta 21

const int reset = 5;

int posicao = 0;
int passo = 0;
int fileira = 0;

const int xmotorPino1 = 27;    // Pino de controle 1 do motor de passo
const int xmotorPino2 = 14;    // Pino de controle 2 do motor de passo
const int xmotorPino3 = 12;    // Pino de controle 3 do motor de passo
const int xmotorPino4 = 13;    // Pino de controle 4 do motor de passo

const int ymotorPino1 = 32;    // Pino de controle 1 do motor de passo
const int ymotorPino2 = 33;    // Pino de controle 2 do motor de passo
const int ymotorPino3 = 25;    // Pino de controle 3 do motor de passo
const int ymotorPino4 = 26;    // Pino de controle 4 do motor de passo

const int PPRT = 2048;         // Número de passos por revolução do motor
const int Vn = 10;             // Velocidade normal
const int Vc = 5;              // Velocidade caixa

Stepper xmotor(PPRT, xmotorPino1, xmotorPino3, xmotorPino2, xmotorPino4);
Stepper ymotor(PPRT, ymotorPino1, ymotorPino3, ymotorPino2, ymotorPino4);

bool motorGirando = false;     // Variável para rastrear o estado do motor
bool ON = false;

unsigned long previousMillis = 0;
const unsigned long interval = 1000;  // Intervalo de leitura e impressão (1 segundo)

void setup() {
  Serial.begin(115200);

  /* ---------------------------------------- Connect to Wi-Fi. */
  WiFi.mode(WIFI_STA);
  Serial.println("------------");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(2, HIGH);
    delay(250);
    digitalWrite(2, LOW);
    delay(250);
    if(connecting_process_timed_out > 0) connecting_process_timed_out--;
    if(connecting_process_timed_out == 0) {
      delay(1000);
      ESP.restart();
    }
  }
  digitalWrite(2, LOW);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("------------");
  Serial.println("");

  pinMode(SMdir, INPUT_PULLUP);     // Configura o sensor da porta 22 como entrada com resistor de pull-up
  pinMode(SMesq, INPUT_PULLUP);
  pinMode(B, INPUT_PULLUP);
  xmotor.setSpeed(Vn);              // Define a velocidade do motor (em RPM)
  ymotor.setSpeed(Vn);              // Define a velocidade do motor (em RPM)
}

void loop() {

  movimento();

}

void movimento() {
if(ON == 0 && digitalRead(B) == HIGH){
  ON = true;
  }
  
  if(ON == 1){
    if (!motorGirando && passo == 0) {
    posicao = 0;
    HTTP();
    xmotor.step(PPRT);
    motorGirando = true;
    fileira = 1;
    posicao = 2040;
    
    while (digitalRead(SMdir) == HIGH) {
      if (digitalRead(SP) == LOW){
        xmotor.setSpeed(Vc);
        xmotor.step(1);
        posicao = (posicao + 1);
        delay(1);
        if(posicao % 2048 == 0){
          HTTP();
        }
      }
      else {
        xmotor.setSpeed(Vn);
        xmotor.step(1);
        posicao = (posicao + 1);
        if(posicao % 2048 == 0){
          HTTP();
        }
      }
    }
    posicao = 0;
    xmotor.step(0);
    motorGirando = false;
    passo = (passo + 1) % 4;
    delay(500);
  }

  if (!motorGirando && passo == 1) {
    ymotor.step(PPRT);
    motorGirando = true;
    fileira = 2;
    posicao = 0;
    HTTP();

    while (digitalRead(SMdir) == HIGH) {
      ymotor.step(1); // Gira o motor um passo no sentido horário
      delay(1);       // Pequeno atraso entre os passos
    }
    motorGirando = false;
    passo = (passo + 1) % 4;
    delay(500);
  }

  if (!motorGirando && passo == 2) {
    posicao = 0;
    xmotor.step(-PPRT);
    motorGirando = true;
    fileira = 2;
    posicao = -2040;

  while (digitalRead(SMesq) == HIGH) {
    if (digitalRead(SP) == LOW){
      xmotor.setSpeed(Vc);
      xmotor.step(-1);
      posicao = (posicao - 1);
      delay(1);
      if(posicao % 2048 == 0){
        HTTP();
        }
    }
    else {
    xmotor.setSpeed(Vn);
    xmotor.step(-1);
    posicao = (posicao - 1);
    if(posicao % 2048 == 0){
      HTTP();
      }
    }
  }
  posicao = 0;
  xmotor.step(0);
  motorGirando = false;
  passo = (passo + 1) % 4;
  delay(500);
  }

  if (!motorGirando && passo == 3) {
    ymotor.step(-PPRT);
    motorGirando = true;
    fileira = 1;
    posicao = 0;
    HTTP();

    while (digitalRead(SMesq) == HIGH) {
      ymotor.step(-1); // Gira o motor um passo no sentido horário
      delay(1);        // Pequeno atraso entre os passos
    }
    motorGirando = false;
    ON = 0;
    passo = (passo + 1) % 4;
    delay(500);
  }
  }
}

void HTTP() {
  String postDataX = "Xresult=" + String(posicao);
  HTTPClient http; 
  http.begin(URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  httpCode = http.POST(postDataX);
  String payload = http.getString();
  Serial.print("URL : "); Serial.println(URL); 
  Serial.print("Data: "); Serial.println(postDataX); 
  Serial.print("httpCode: "); Serial.println(httpCode); 
  Serial.print("payload : "); Serial.println(payload); 
  Serial.println("--------------------------------------------------");
  Serial.println("mandou!");
}