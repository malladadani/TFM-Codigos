#include <driver/pcnt.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <string.h>
#include <esp_err.h>

#define PULSE_INPUT_PIN_1 GPIO_NUM_17
#define PULSE_INPUT_PIN_2 GPIO_NUM_18
#define PULSE_INPUT_PIN_3 GPIO_NUM_19
#define INTERVAL_MS 10000

// WiFi y MQTT igual que antes...

// ----------------- WiFi Credentials -----------------------
const char* ssid = "LUZ_NIVEL_2";
const char* password = "manuel_rico";

// ----------------- MQTT Broker ----------------------------
const char *mqtt_broker = "neutrino.edv.uniovi.es"; // IP del broker
const char *topic = "ae/ordenes"; // Topico 
const char *mqtt_username = "motores";  // Usuario
const char *mqtt_password = "2motores3";  // Contraseña
const int mqtt_port = 8883;

const char *MQTT_topic_freq = "ae/frequency";
const char *MQTT_topic_freq_01 = "ae/frequency/ch_01";
const char *MQTT_topic_freq_02 = "ae/frequency/ch_02";
const char *MQTT_topic_freq_03 = "ae/frequency/ch_03";

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

// Variables para los contadores
volatile int16_t current_pulses_1 = 0;
volatile int16_t current_pulses_2 = 0;
volatile int16_t current_pulses_3 = 0;

// ... Certificado y funciones WiFi/MQTT igual que antes ...

void setup_pcnt_unit(pcnt_unit_t unit, gpio_num_t pin) {
  pcnt_config_t pcnt_config = {};
  pcnt_config.unit = unit;
  pcnt_config.channel = PCNT_CHANNEL_0;
  pcnt_config.pulse_gpio_num = pin;
  pcnt_config.ctrl_gpio_num = PCNT_PIN_NOT_USED;
  pcnt_config.pos_mode = PCNT_COUNT_INC;
  pcnt_config.neg_mode = PCNT_COUNT_DIS;
  pcnt_config.lctrl_mode = PCNT_MODE_KEEP;
  pcnt_config.hctrl_mode = PCNT_MODE_KEEP;
  pcnt_config.counter_h_lim = 32767;
  pcnt_config.counter_l_lim = 0;

  pcnt_unit_config(&pcnt_config);
  pcnt_set_filter_value(unit, 1000);
  pcnt_filter_enable(unit);
  pcnt_counter_pause(unit);
  pcnt_counter_clear(unit);
  pcnt_counter_resume(unit);
}


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



void Connect_Wifi()
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
    mqttClient.setServer(mqtt_broker, mqtt_port);
    //mqttClient.setCallback(Callback_MQTT);
    while (!mqttClient.connected()) {
        String client_id = "mallada";
        client_id += String(random(0xffff), HEX);
        client_id += String(WiFi.macAddress());
        Serial.printf("El cliente %s, se esta conectando con el servidor", client_id.c_str());
        if (mqttClient.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Conexión logradaa");
        } else {
            Serial.print("Conexión fallida");
            Serial.print(mqttClient.state());
            delay(2000);
        }
    }
}


void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando contadores de pulsos...");

  // Configura cada unidad PCNT para cada pin
  setup_pcnt_unit(PCNT_UNIT_0, PULSE_INPUT_PIN_1);
  setup_pcnt_unit(PCNT_UNIT_1, PULSE_INPUT_PIN_2);
  setup_pcnt_unit(PCNT_UNIT_2, PULSE_INPUT_PIN_3);

  Serial.println("Contadores PCNT configurados y en funcionamiento.");

  Connect_Wifi();
  Conect_Broker();
}

void loop() {
  mqttClient.loop();

  static unsigned long lastMillis = 0;

  if (millis() - lastMillis >= INTERVAL_MS) {
    lastMillis = millis();

    int16_t count_value_1 = 0, count_value_2 = 0, count_value_3 = 0;
    pcnt_get_counter_value(PCNT_UNIT_0, &count_value_1);
    pcnt_get_counter_value(PCNT_UNIT_1, &count_value_2);
    pcnt_get_counter_value(PCNT_UNIT_2, &count_value_3);

    current_pulses_1 = count_value_1;
    current_pulses_2 = count_value_2;
    current_pulses_3 = count_value_3;

    Serial.printf("Pulsos en los últimos %.1f segundos:\n", (float)INTERVAL_MS / 1000.0);
    Serial.printf("Canal 1 (GPIO 17): %d\n", current_pulses_1);
    Serial.printf("Canal 2 (GPIO 18): %d\n", current_pulses_2);
    Serial.printf("Canal 3 (GPIO 19): %d\n", current_pulses_3);

    float freq1 = (float)current_pulses_1 / ((float)INTERVAL_MS / 1000.0);
    float freq2 = (float)current_pulses_2 / ((float)INTERVAL_MS / 1000.0);
    float freq3 = (float)current_pulses_3 / ((float)INTERVAL_MS / 1000.0);

    Serial.printf("Frecuencia medida Canal 1: %.2f Hz\n", freq1);
    Serial.printf("Frecuencia medida Canal 2: %.2f Hz\n", freq2);
    Serial.printf("Frecuencia medida Canal 3: %.2f Hz\n", freq3);

    char freq_buffer[128];
     snprintf(freq_buffer, sizeof(freq_buffer),
      "{\"freq_ch_01\":%.2f,\"freq_ch_02\":%.2f,\"freq_ch_03\":%.2f}",
      freq1, freq2, freq3);

    mqttClient.publish(MQTT_topic_freq, freq_buffer);

/*
    char freq1_buffer1[16], freq2_buffer2[16], freq3_buffer3[16];
    dtostrf(freq1, 6, 2, freq1_buffer1); 
    dtostrf(freq2, 6, 2, freq2_buffer2);
    dtostrf(freq3, 6, 2, freq3_buffer3);

    mqttClient.publish(MQTT_topic_freq_01, freq1_buffer1);
    mqttClient.publish(MQTT_topic_freq_03, freq3_buffer3);
    mqttClient.publish(MQTT_topic_freq_02, freq2_buffer2);
*/
    // Limpia los contadores para el siguiente intervalo
    pcnt_counter_clear(PCNT_UNIT_0);
    pcnt_counter_clear(PCNT_UNIT_1);
    pcnt_counter_clear(PCNT_UNIT_2);
  }
}
