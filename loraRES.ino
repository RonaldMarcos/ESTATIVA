#include "heltec.h" //inclui a biblioteca da Heltec
#define DisplayEstado true //define se o display estará ou não ativo
#define LoRaEstado true //define se o rádio LoRa estará ou não ativo
#define SerialEstado true //define se a Serial do módulo estará ou não disponível 
#define AmplificadorDePotencia true // define se o amplificador de potência PABOOST estará ou não ativo
#define BandaDeTransmissao 868E6 //define a frequência média de transmissão: 868E6, 915E6
#define GPIO_BOTAO 37 //pino botão
#define pinCurrent 36 // pino le a corrente
#define TEMPO_DEBOUNCE 10 //ms
#include <ESP32Servo.h> //inclui a biblioteca do servo
Servo servo;
int pinServo=22;
int volt=0;
String conter;
int contador_acionamentos = 0; 
int contador_acionamentos1 = 0;
unsigned long timestamp_ultimo_acionamento = 0;
void IRAM_ATTR funcao_ISR()
{
/* Conta acionamentos do botão considerando debounce */
if ( (millis() - timestamp_ultimo_acionamento) >= TEMPO_DEBOUNCE )
{
contador_acionamentos++;
timestamp_ultimo_acionamento = millis();
}
}

void setup()
{
Serial.begin(115200);
pinMode(GPIO_BOTAO, INPUT);
attachInterrupt(GPIO_BOTAO, funcao_ISR, RISING);

  servo.attach(pinServo);
  analogSetPinAttenuation(36,ADC_11db);
  if(analogRead(36)>0){
     volt=analogRead(36);
     }
  Serial.println(analogRead(36)*3.3/4095.0,5);
  Heltec.begin(DisplayEstado, LoRaEstado, SerialEstado, AmplificadorDePotencia, BandaDeTransmissao);
  SetupLoRa(); //faz os ajustes do rádio LoRa
  delay(100);
  //Sinaliza o fim dos ajustes
  Heltec.display -> clear();
  Heltec.display -> drawString(0, 0, "Setup concluído!");
  Heltec.display -> drawString(0, 10, "Aguardando dados...");
  Heltec.display -> display();
}

void loop()
{
  //verifica a chegada dos dados
  RecebeDados();
  if(contador_acionamentos!=contador_acionamentos1){

  contador_acionamentos = contador_acionamentos1;
  Serial.println(contador_acionamentos);

      //expandir
      servo.write(90);
      delay(400);
      servo.write(180);
      delay(1000);
      servo.write(90);
      delay(400);

      //retrai
      servo.write(90);
      delay(400);
      servo.write(0);
      delay(1500);
      servo.write(90);
      delay(400);
      }
}
void SetupLoRa()
{
  LoRa.setSpreadingFactor(12); //define o fator de espalhamento
  LoRa.setSignalBandwidth(250E3); //define a largura de banda
  LoRa.setCodingRate4(5); //define o Codding Rate
  LoRa.setPreambleLength(6); //define o comprimento do preâmbulo
  LoRa.setSyncWord(0x12); //define a palavra de sincronização
  //LoRa.disableCrc(); //desabilita o CRC
  LoRa.crc(); //ativa o CRC
}

void RecebeDados()
{
  // tenta analisar o pacote
  int packetSize = LoRa.parsePacket();
  if (packetSize)  // se o pacote não tiver tamanho zero
  {
    //matriz para armazenamento dos estados
    byte Dados[3];
    // Lê o pacote
    while (LoRa.available())
    {
      Dados[0] = LoRa.read(); //estado do motor
      Dados[1] = LoRa.read(); //contador
    }
   if(String(Dados[1])!=conter  ){
    Serial.println("Holofote: ");
    
     //expandir
      servo.write(90);
      delay(400);
      servo.write(180);
      delay(1000);
      servo.write(90);
      delay(400);

      //retrai
      servo.write(90);
      delay(400);
      servo.write(0);
      delay(1500);
      servo.write(90);
      delay(400);
      conter=String(Dados[1]);
      
   }
    
    //Sinaliza os dados recebidos e o RSSI no display
    Heltec.display -> clear();
    Heltec.display -> drawString(0, 10, "Acionamentos: " + String(Dados[1]));
    Heltec.display -> drawString(0, 30, "RSSI: " + String(LoRa.packetRssi()));
    Heltec.display -> display();
  
  }
}
