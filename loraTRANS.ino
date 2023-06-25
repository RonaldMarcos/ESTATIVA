//COM 8
#include "Arduino.h" //inclui a biblioeca padrão Arduino.h
#include "heltec.h" //inclui a biblioteca da Heltec
#include <WiFi.h> //inclui a biblioteca WiFi
// Definições do rádio LoRa
#define DisplayEstado true //define se o display estará ou não ativo
#define LoRaEstado true //define se o rádio LoRa estará ou não ativo
#define SerialEstado true //define se a Serial do módulo estará ou não disponível (Baud Rate padrão 115200)
#define AmplificadorDePotencia true // define se o amplificador de potência PABOOST estará ou não ativo
#define BandaDeTransmissao 868E6 //define a frequência média de transmissão: 868E6, 915E6
// Definições do WiFi
#define SSid "POCO F4 GT" //Identificador do serviço de rede WiFi
#define Senha "12345678" //Senha de acesso ao serviço WiFi
//Variáveis de estado dos controlados
bool EstadoMotor = 0;
int cont=0;
bool Atuar = false;
//inicia o servidor na porta 80
WiFiServer servidor(80);
int btn1 = 36;

void setup()
{
  //inicia os dispositivos do devKit da Heltec (DEVE SER INICIADO ANTES DE TUDO)
  Heltec.begin(DisplayEstado, LoRaEstado, SerialEstado, AmplificadorDePotencia, BandaDeTransmissao);
  Heltec.display->clear(); //Limpa qualquer mensagem inicial do display
  SetupWiFi(); //Configura a conexão Wifi
  SetupLoRa(); //Configura o rádio LoRa
  //Sinaliza o fim dos ajustes
  Heltec.display -> drawString(0, 40, "Setup concluído!");
  Heltec.display -> display();
  servidor.begin();// inicia o servidor 
  
  //informa ip no display
  Heltec.display -> drawString(0, 50, "Servidor: " + WiFi.localIP().toString());
  Heltec.display -> display();
  delay(1000);
  Serial.begin(115200);
  pinMode(btn1, INPUT);
}

void loop() 
{
  TrataRequisicao();//trata as requisições HTTP
  enviarPacote();//Envia as mensagens LoRa
   Serial.println(WiFi.localIP().toString());
}

void SetupWiFi()
{
  //inicio dos ajustes do conexão WiFi
  WiFi.disconnect(true); //Desconecta de uma possível conexão prévia
  delay(100);
  WiFi.begin(SSid, Senha); //Informa o id do Serviço WiFi e a senha
  delay(100);
  byte count = 0; // Contador das tentativas de conexão
  
 while (WiFi.status() != WL_CONNECTED && count < 10)
  {
    count ++;
    delay(500);
    Heltec.display -> drawString(0, 0, "Conectando WiFi...");
    Heltec.display -> display();
  }
  if (WiFi.status() == WL_CONNECTED) //Se conseguiu conectar...
  {
    Heltec.display -> drawString(0, 10, "Conexão WiFi ok!");
    Heltec.display -> display();
    delay(100);
  }
  else // ... caso falhe a conexão
  {
    Heltec.display -> drawString(0, 10, "Falha na conexão!");
    Heltec.display -> display();
    delay(100);
    //pode aplicar um loop infinito ou reset AQUI;
  }
}

void SetupLoRa()
{
  //Sinaliza o início dos ajustes
  Heltec.display -> drawString(0, 20, "Configurando LoRa...");
  Heltec.display -> display();
  delay(100);
  LoRa.setSpreadingFactor(12); //define o fator de espalhamento
  LoRa.setSignalBandwidth(250E3); //define a largura de banda
  LoRa.setCodingRate4(5); //define o Codding Rate
  LoRa.setPreambleLength(6); //define o comprimento do preâmbulo
  LoRa.setSyncWord(0x12); //define a palavra de sincronização
  //LoRa.disableCrc(); //desabilita o CRC
  LoRa.crc(); //ativa o CRC
  //Sinaliza o fim dos ajustes do rádio LoRa
  Heltec.display -> drawString(0, 30, "LoRa configurado!");
  Heltec.display -> display();
  delay(100);
}

void TrataRequisicao()
{
  //cria um objeto "cliente" se há uma conexão disponível
  WiFiClient cliente = servidor.available();
  if (!cliente) return; //se não há cliente, retorna
  // se há um cliente, sinaliza no display

  Heltec.display -> clear();
  Heltec.display -> drawString(0, 0, "Servidor: " + WiFi.localIP().toString());
 
  Heltec.display -> drawString(0, 10, "Cliente conectado!");
  Heltec.display -> display();
  while (cliente.connected()) //executa enquanto houver conexão
  {
    if (cliente.available()) //se há uma requisição disponível
    {
      String req = cliente.readStringUntil('\r'); //Lê a requisição
     //A partir daqui, verificamos se a requisição possui
      //algum comando de ajuste de sinal
      if (req.indexOf("acao=0") != -1) //para o motor da bomba
      {
        Atuar = true; //enviar os dados para atuar
        EstadoMotor = 1;
        cont=cont+1;
      }
      else //nenhuma opção válida
      {
        Atuar = false; //sinaliza que não é para atuar
      }
      
//Este é o html que iremos retornar para o cliente
      
  String html =
        "<html>"
        "<head>"
        "<meta name='viewport' content='width=device-width, initial-scale=1, user-scalable=no'/>"
        "<title>Controle WiFi / LoRa</title>"
        "<style>"
        "body{"
        "text-align: center;"
        "font-family: sans-serif;"
        "font-size:15px;"
        "padding: 25px;"
        "}"
        "p{"
        "color:#444;"
        "}"
        "button{"
        "font-size:15px;"
        "outline: none;"
        "border: 2px solid #1fa3ec;"
        "border-radius:18px;"
        "background-color:#FFF;"
        "color: #000;"
"padding: 5px 25px;"
        "}"
        "button:active{"
        "color: #fff;"
        "background-color:#1fa3ec;"
        "}"
        "button:hover{"
        "border-color:#0000ff;"
        "}"
        "</style>"
        "</head>"
        "<body>"
        "<H1>Controle WiFi / LoRa</H1>"
        "<H2>Selecione Para Disparar:</H2>"
        "<p><a href='?acao=0'><button>Acionar</button></a>"
        "<p></p>"
        "<p></p>"
        "<H3>Acionamentos: " + String(cont) + "</H3>"
        "</body>"
        "</html>\r";

//Escreve o html no buffer que será enviado para o cliente
      cliente.print(html);
      //Envia os dados do buffer para o cliente
      cliente.flush();
    }
    //aguarda para que o trafego das informações seja concluído
    delay(20);
    //força a desconexão do cliente
    cliente.stop();
  }
  //sinaliza o IP do servidor no display
  Heltec.display -> clear();
  Heltec.display -> drawString(0, 0, "Servidor em:");
  Heltec.display -> drawString(0, 10, WiFi.localIP().toString());
  Heltec.display -> display();
}

void enviarPacote()
{

  if (!Atuar) return; //se não é para atuar, retorna
  //envia para a serialos dados que serão enviados pelo LoRo

  //Enviar pacote
  //inicia a montagem do pacote (parâmetro FALSE desativa o Header);
  LoRa.beginPacket();
  //enviando os estado um a um
  LoRa.write(EstadoMotor);
  LoRa.write(cont);
  LoRa.endPacket(); // finaliza o pacote
  Atuar = false; //Sinaliza que já houve a atuação
}
