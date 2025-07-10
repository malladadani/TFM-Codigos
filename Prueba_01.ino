#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <string.h>
#include <Encoder.h>

#include "Stepper.h"

#define MS1 4
#define MS2 3
#define MS3 2

#define ENCODER_PIN_B 6
#define ENCODER_PIN_A 8

#define DIR 0
#define STEP 1

#define BROKEN_POINT_01 20
#define BROKEN_POINT_02 21

#define MODE_STEPS 1
#define MODE_VEL 2
#define MODE_DIR 3
#define MODE_CHNG_STEP 4
#define MODE_RESUME 5
#define MODE_STOP 6
#define MODE_INFORMATION 89
#define MODE_RESET 123

Stepper MyStepper(1, 0, 2, 3, 4);

int _mode_pap = MODE_STEPS;
char* _DATA = NULL;
int _LEN_DATA = 0;
int _DATA_INT = 0;
bool _flag_mqtt = false; 
bool _STATE_DIR = false;
bool _STATE_CAL = true;
bool _flag_event = true;

int _position = 0;
int _aState;
int _aLastState;

volatile bool evento = false;
unsigned long cont_event = 0;
unsigned long ultimaInterrupcion = 0;
const unsigned long debounceDelay = 50; 

// ----------------- WIFI -----------------------------------
const char *ssid = "LUZ_NIVEL_2"; // SSID WiFi
const char *password = "manuel_rico";  // Contraseña WiFi

//const char *ssid = "Tenda_Central"; // SSID WiFi
//const char *password = "Canga_WiFi_2k24";  // Contraseña WiFi

// ----------------- MQTT Broker ----------------------------
const char *mqtt_broker = "neutrino.edv.uniovi.es"; // IP del broker
const char *topic = "ae/ordenes"; // Topico 
const char *mqtt_username = "motores";  // Usuario
const char *mqtt_password = "2motores3";  // Contraseña
const int mqtt_port = 8883;


const char* mqtt_topic_steps = "ae/motor_pap/steps";
const char* mqtt_topic_vel = "ae/motor_pap/vel";
const char* mqtt_topic_dir = "ae/motor_pap/dir";
const char* mqtt_topic_change_step = "ae/motor_pap/change_step";
const char* mqtt_topic_stop = "ae/motor_pap/stop";
const char* mqtt_topic_resume = "ae/motor_pap/resume";
const char* mqtt_topic_information = "ae/encoder/information";
const char* mqtt_topic_reset = "ae/encoder/reset";

const char* mqtt_topic_overlape = "ae/TSCP/overlape";

static const char CA_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIEETCCAvmgAwIBAgIUebjUqWHarDoBKLzH78XkunwNlrEwDQYJKoZIhvcNAQEL
BQAwgZcxCzAJBgNVBAYTAkVTMREwDwYDVQQIDAhBc3R1cmlhczEOMAwGA1UEBwwF
R2lqb24xDzANBgNVBAoMBlVOSU9WSTEOMAwGA1UECwwFQ0UzSTIxIjAgBgNVBAMM
GWVsZWN0cm9uaWNvLmVkdi51bmlvdmkuZXMxIDAeBgkqhkiG9w0BCQEWEWVsb3Bl
emNAdW5pb3ZpLmVzMB4XDTIxMDYyMjIxNTgwOFoXDTMxMDYyMDIxNTgwOFowgZcx
CzAJBgNVBAYTAkVTMREwDwYDVQQIDAhBc3R1cmlhczEOMAwGA1UEBwwFR2lqb24x
DzANBgNVBAoMBlVOSU9WSTEOMAwGA1UECwwFQ0UzSTIxIjAgBgNVBAMMGWVsZWN0
cm9uaWNvLmVkdi51bmlvdmkuZXMxIDAeBgkqhkiG9w0BCQEWEWVsb3BlemNAdW5p
b3ZpLmVzMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA0VFz0NxAxerK
aSA50KRFLaGP/tMLBhyJdfpgtmFy61Yi+q5ppqmQBh9+VMxVtfKZ8RZlcteYMSRy
KHWHUyy/t0aMvaB4d/ftwX4X8xzAt6DayTmhp7cuqJU/Tdfje0iWI4J1yN7vDnKt
//jzBqloLnnuqljKBHc/lSJm+kAAYRPDoOdsSYtiHubgMIIEWp7p7OXRPZurHoF3
GnBwqTOE3+a9/RxtUTck1g+wv5b8CK7RWtmMtMXlp6eUdgk48rDAPv8r+zpxPQdc
xFYK6CnGa1Sv6hMERQ26GuopfbiDfEmSoV8b9qonIcjlQW5UvJGuLJsYJo+befM5
YfPF5VIB+QIDAQABo1MwUTAdBgNVHQ4EFgQULPRJpQp3iLG0y3WwzJhXz3g2WaIw
HwYDVR0jBBgwFoAULPRJpQp3iLG0y3WwzJhXz3g2WaIwDwYDVR0TAQH/BAUwAwEB
/zANBgkqhkiG9w0BAQsFAAOCAQEASPAq1oNcMyK/6DvGHCKlujrctBLOTx9kkdpE
YcLV7s0ySyIvaMadraK4tAJr/dFqFCIJb23ie229SJN3x9e4H52a7twHFlK/wZOA
B1Q/sZq2uTKS1Pw99gzQKYiycIqnRUnD/TACaGTr+6TEzOnm4Q3VuR75WWPE8bcl
APLFyPF5wviOy4E9TPRxYqAdfMSUFUm7oh1Fv92wFUgWC02ycfKtkMaa/oZoNchF
lyk81V8EnfhmZseDVRyRvGxlNYG9312uLQLzqhf8C/Czsco0pI0K7W6Sd1XQ1MkF
eEATGimuO2alKYek4upq3jZkSe3dJiPeu6J7fyhow6bI1WGcxQ==
-----END CERTIFICATE-----
)EOF";


WiFiClientSecure espClient;

PubSubClient client(espClient);

#include "V1_Callback.h"

// ------------------ FUNCION PARA CONECTARSE AL WIFI --------------
void Conect_Wifi()
{
  WiFi.begin(ssid, password);
  Serial.println("-------------------------");

  Serial.println("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("...................");
  }

  Serial.println("Conexión a la Wifi exitosa");
}


void Conect_Broker()
{
  espClient.setCACert(CA_cert);

  // ------------ CONEXION CON EL BROKER -------------
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(Callback_MQTT);
    while (!client.connected()) {
        String client_id = "mallada";
        client_id += String(random(0xffff), HEX);
        client_id += String(WiFi.macAddress());
        Serial.printf("El cliente %s, se esta conectando con el servidor\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Conexión logradaa");
        } else {
            Serial.print("Conexión fallida");
            Serial.print(client.state());
            delay(2000);
        }
    }
}


void Subscribe_Topic()
{
  client.subscribe(mqtt_topic_dir);
  client.subscribe(mqtt_topic_vel);
  client.subscribe(mqtt_topic_steps);
  client.subscribe(mqtt_topic_change_step);
  client.subscribe(mqtt_topic_resume);
  client.subscribe(mqtt_topic_stop);
  client.subscribe(mqtt_topic_information);
  client.subscribe(mqtt_topic_reset);
}

void IRAM_ATTR BROKEN_POINT()
{
  if(_flag_event){evento = true;}
}

void ISR_encoder() 
{
  _aState = digitalRead(ENCODER_PIN_A);
  if (digitalRead(ENCODER_PIN_B) == _aState) 
  {
    _position++;
  } 
  
  else 
  {
    _position--;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // Inicializa la comunicación serial
  Conect_Wifi();
  Conect_Broker();
  Subscribe_Topic();
  Serial.println("Begin");

  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), ISR_encoder, CHANGE);

  pinMode(BROKEN_POINT_01, INPUT);
  pinMode(BROKEN_POINT_02, INPUT);
  attachInterrupt(digitalPinToInterrupt(BROKEN_POINT_01), BROKEN_POINT, RISING);
  attachInterrupt(digitalPinToInterrupt(BROKEN_POINT_02), BROKEN_POINT, RISING);
  client.publish("ae/TSCP/overlape", "BEGIN");

  MyStepper.Go(20);

}

void loop() {
  // put your main code here, to run repeatedly:
  client.loop();

  static long lastPosition = 0;
  if (_aState != _aLastState)
  {
    lastPosition = _position;
  }

  if (evento) {
    evento = false;
    _flag_event = false;
    if (_STATE_CAL)
    {
      _position = 0;
    }

    _STATE_CAL = false;
    Serial.println("Boton presionado");
    client.publish(mqtt_topic_stop, "1");
    cont_event++;

    if(_STATE_DIR == false)
    {
      MyStepper.Spin(true);
      _STATE_DIR = true;
    }
    else
    {
      MyStepper.Spin(false);
      _STATE_DIR = false;
    }
/*
    digitalWrite(MS1,0);
    digitalWrite(MS2,0);
    digitalWrite(MS3,0);

    for(int i = 0; i<10; i++)
    {
      digitalWrite(STEP, 1);
      delay(20);
      digitalWrite(STEP, 1);
      delay(20);
    }
    */

    MyStepper.Go(10);
    delay(50);
    _flag_event = true;
  }

  MyStepper.Update();

  if(_flag_mqtt)
  {
    Serial.println("Mode PaP -->");
    Serial.println(_mode_pap);
    switch(_mode_pap)
    {
      case(MODE_STEPS):
        MyStepper.Go(_DATA_INT);
      break;

      case(MODE_VEL):
        MyStepper.Change_Vel(_DATA_INT);
      break;

      case(MODE_DIR):
        if(_DATA_INT == 1){MyStepper.Spin(true); _STATE_DIR = true;}
        else if(_DATA_INT == 0){MyStepper.Spin(false); _STATE_DIR = false;}
      break;

      case(MODE_CHNG_STEP):
        if(_DATA_INT > 0 && _DATA_INT < 6){MyStepper.Change_Step(_DATA_INT);}
      break;

      case(MODE_STOP):
        MyStepper.Stop();
      break;

      case(MODE_RESUME):
        MyStepper.Resume();
      break;

      case(MODE_INFORMATION):
        char buffer[20]; 
        int angle_position;
        Serial.print("Posicion en pasos = ");
        Serial.println(_position);
        Serial.print("Grados = ");
        angle_position = _position*360/4096;
        itoa(angle_position, buffer, 10);
        Serial.print("=====================");
        Serial.println(buffer);
        client.publish("ae/TSCP/overlape", buffer);
        _flag_mqtt=false;
      break;

      case(MODE_RESET):
        _position = 0;
      break;

      default:
        _flag_mqtt=false;
      break;
    }

    _flag_mqtt=false;
  }

}
