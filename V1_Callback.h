
void Callback_MQTT(const char* topic, byte* payload, unsigned int length)
{
  Serial.println("ENTRO");

  char dataArray[length + 1];
  // Se crea una variable de tipo char* de tamaño length

  dataArray[length] = NULL;
  // Se hace NULO su valor

  // Convertimos a un array
  for (int i = 0; i<length; i++)
  {
    dataArray[i] = (char)payload[i];
  }
  
  // Guardamos en la variable GLOBAL 
  _DATA = dataArray;
  _LEN_DATA = length;
  char payloadString[length + 1];
  // Convierte las variables entera en tipo string
  memcpy(payloadString, payload, length);

  Serial.print("El topico que se ha enviado el mensaje ha sido:");
  Serial.print(topic);
  Serial.print("\n");

  payloadString[length] = '\0';
  Serial.print("PAYLOAD con memcpy :");
  Serial.printf("%s\n", payloadString);

  Serial.println("Dato recibido: ");
  Serial.print("-> Topic: ");
  Serial.print(topic);
  
  Serial.print(" -> longitud: ");
  Serial.print(length);
  
  Serial.print(" -> mensaje: ");
  Serial.println( _DATA);
  Serial.println("----------------------------");  


  Serial.println("El topico es: ");
  Serial.println(topic);

  if(strcmp(topic, mqtt_topic_steps) == 0)
  {
    Serial.println("MODE STEPS");
    _mode_pap = MODE_STEPS;
  }

  if (strcmp(topic, mqtt_topic_vel) == 0)
  {
    Serial.println("MODE VEL");
    _mode_pap = MODE_VEL;
  }

  if (strcmp(topic, mqtt_topic_dir) == 0)
  {
    Serial.println("MODE DIR");
    _mode_pap = MODE_DIR;
  }


  if (strcmp(topic, mqtt_topic_change_step) == 0)
  {
    Serial.println("MODE CHANGE STEP");
    _mode_pap = MODE_CHNG_STEP;
  }

  if (strcmp(topic, mqtt_topic_resume) == 0)
  {
    Serial.println("MODE RESUME");
    _mode_pap = MODE_RESUME;
  }

  if (strcmp(topic, mqtt_topic_information) == 0)
  {
    Serial.println("MODE INFORMATION");
    _mode_pap = MODE_INFORMATION;
  }

  if (strcmp(topic, mqtt_topic_stop) == 0)
  {
    Serial.println("MODE STOP");
    _mode_pap = MODE_STOP;
  }
  
  if (strcmp(topic, mqtt_topic_reset) == 0)
  {
    Serial.println("MODE RESET");
    _mode_pap = MODE_RESET;
  }

 

  _flag_mqtt = true;
  _DATA_INT = atoi(_DATA);

}

