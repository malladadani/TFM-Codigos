"""
Sistema de supervisión de la frecuencia eléctrica (Fe)
De una turbina

v2.0 Introducimos la tabla de medidas (separamos campos)

Ordenes implementadas:
fe-enviar
nuevo_pp=xx

Formato de mensaje para enviar Fe:
    /*
   * El formato propuesto para el mensaje MQTT es el siguiente:
   * CAMPO 1: Turbina_ID[6]
   * SEPARADOR: #
   * CAMPO 2: Pares de polos(pp) [3]
   * SEPARADOR: #
   * CAMPO3: Fe_max [6] 6 digitos con 2 decimal
   * SEPARADOR: #
   * CAMPO4: Fe_media [6] 6 digitos con 2 decimal
   * SEPARADOR: #
   * CAMPO5: Fe_min [6] 6 digitos con 2 decimal
   * SEPARADOR: #
   * CAMPO 5+1: Fe(1) [6] 6 digitos con 2 decimal
   * SEPARADOR: #
   * ....
   * CAMPO 5+2pp: Fe(2pp) [6] 6 digitos con 2 decimal
   * TERMINADOR:#
   */
    

Por Manuel Rico-Secades (EPI-Gijon)
Noviembre 2022
"""

# ZONA 1: Librerias
import paho.mqtt.client as mqtt     #paho es la libreria basica para trabajar con MQTT
import time                         #time permite obtener la hora del ordenador
import subprocess as sp             #subprocess nos permite abrir el fichero pdf de ayuda
import ssl                          #ssl nos permite implementar la seguridad TLS
from tkinter import * 
from tkinter import ttk              #tkinter es la libreria para trabajar con ventanas de windows
import random                       #La vamos a usar para generar un ID aleatorio
import string                       #Para trabajar con strings
import os                           #Para obtener el nombre del usuario del equipo
import sqlite3
import math
import numpy as np

# Rutinas elaboradas por Manuel Rico Secades para guardar en base de datos (BD)
#################################################################################################
#################################################################################################
#
version=2.0
# Direccion IP fija del broker MQTT del grupo ce3i2
IP_inicial="156.35.154.170"
#IP_inicial = "neutrino.edv.uniovi.es"
qos=2
usuario_mqtt="motores"
clave="2motores3"
IP_Broker = IP_inicial
# Importante tener en cuenta que cada usuario_mqtt tiene un topic raiz autorizado en el
# Broker_MQTT instalado
topic_recibir1="ae/#"
#
topic_enviar1="clase/todas/ordenes"
#
MQTT_topic_resume="ae/motor_pap/resume"
#
MQTT_topic_stop="ae/motor_pap/stop"
#
MQTT_topic_reset="ae/motor_pap/encoder"
#
MQTT_topic_steps="ae/motor_pap/steps"
#
MQTT_topic_spin="ae/motor_pap/dir"
#
MQTT_topic_change_step="ae/motor_pap/change_step"
#
MQTT_topic_recibir1="ae/#"

nombre_DB_inicial="historico"
cliente_MQTT = 'mallada_'+''.join(random.choice(string.ascii_uppercase + string.digits)
                                    for x in range(8))
mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id=cliente_MQTT)
# NOTA: Ojo con el protocolo. TLS V1 no funciona con el broker que tenemos instalado.
ca_cert_path = r"C:\Users\Admin\Desktop\Almacen\3-Codigos\HY4RES\Python_sistema_OM\ca_2021_neutrino.crt"

mqttc.tls_set(ca_certs=ca_cert_path, tls_version=ssl.PROTOCOL_TLSv1_2)
mqttc.tls_insecure_set(True)
# Carpeta raiz para guardar datos
#
# OPCION 1: en escritorio
usuario=os.getlogin()
#carpeta_raiz="C:/Users/"+usuario+"/Desktop/Mi_carpeta/"
#
# OPCION 2: en la carpeta de trabajo
carpeta_raiz=os.getcwd()+"/Sistema_OM_turbinas/"
#
raya="-------------------------------------------------------------------------"
#
################################################################################################
# Funciones de trabajo
################################################################################################
def crear_tabla1_mensajes():
    crear_carpeta_BD()
    #Creamos tablas en la base de datos
    nombreDB=nombre_DB.get()
    final=".db"
    BD=carpeta_raiz+nombreDB+final
    con = sqlite3.connect(BD)
    cursor = con.cursor()
    #
    orden="CREATE TABLE IF NOT EXISTS "
    nombre_tabla1="tabla_mensajes"
    columnas_tabla1="FECHA TEXT, HORA TEXT, USUARIO TEXT, TOPIC TEXT, MENSAJE TEXT"
    #
    orden_crear_tabla1=orden+nombre_tabla1+"("+columnas_tabla1+")"
    #
    cursor.execute(orden_crear_tabla1)
    con.commit()
    cursor.close()
    con.close()
    
################################################################################################
def borrar_tabla2_medidas():
    crear_carpeta_BD()
    nombreDB=nombre_DB.get()
    final=".db"
    BD=carpeta_raiz+nombreDB+final
    con = sqlite3.connect(BD)
    cursor = con.cursor()
    cursor.execute('DROP TABLE tabla_medidas;',);
    con.commit()
    cursor.close()
    con.close()
################################################################################################
def crear_tabla2_medidas():
    crear_carpeta_BD()
    #Creamos tablas en la base de datos
    nombreDB=nombre_DB.get()
    final=".db"
    BD=carpeta_raiz+nombreDB+final
    con = sqlite3.connect(BD)
    cursor = con.cursor()
    #
    orden="CREATE TABLE IF NOT EXISTS "
    nombre_tabla2="tabla_medidas"
    #
    columnas_tabla2="Fecha TEXT, Hora TEXT, Turbina_ID TEXT, pp INTEGER,"
    columnas_tabla2=columnas_tabla2+" Fe_max REAL, Fe_med REAL, Fe_min REAL"
    #
    # To properly create tabla_medidas, the 'pp' variable (pares de polos) needs to be defined
    # before calling this function, as it's used in the loop for creating column names.
    # For a general solution, we'll make a placeholder for 'pp' if it's not globally available
    # or passed as an argument. Assuming 'pp' is accessible via a GUI element `pp.get()`.
    try:
        current_pp_value = int(pp.get()) # Attempt to get current pp value from GUI
    except (NameError, ValueError):
        current_pp_value = 1 # Default or placeholder if pp.get() fails

    minimo=1
    maximo=2 * current_pp_value + 1
    paso=1
    # Va desde minimo hasta maximo-1 de 1 en 1
    for i in range (minimo,maximo,paso):
        columnas_tabla2=columnas_tabla2+", Fe_"+str(i)+ " REAL"
    #    
    orden_crear_tabla2=orden+nombre_tabla2+"("+columnas_tabla2+")"
    #
    cursor.execute(orden_crear_tabla2)
    con.commit()
    cursor.close()
    con.close()
    


################################################################################################    
def crear_carpeta_BD():
    global carpeta_raiz
    try:
        os.stat(carpeta_raiz)
        Lb1.insert(0, "-------------------------------")
        Lb1.insert(0, "La carpeta para BD existe")
    except:
        os.mkdir(carpeta_raiz)
        #
        Lb1.insert(0, "-------------------------------")
        Lb1.insert(0, "Se ha creado carpeta auxiliar para BD en:")
        Lb1.insert(0,carpeta_raiz)
################################################################################################
def ayuda():
    sp.Popen("Ayuda_01.pdf",shell=True)
################################################################################################
def borrar_datos_recibidos():
    Lb1.delete(0,END)
################################################################################################
# CallBack que se debe ejecutar cuando se reciba un mensaje MQTT
################################################################################################
def RecibirDato(mqttc, userdata, msg):
    topic = msg.topic
    mensaje = msg.payload.decode()

    # Mostrar siempre en consola
    print(f"TOPIC: {topic}")
    print(f"MSG: {mensaje}")

    # Muestra en el Listbox de la GUI todos los mensajes de ae/#
    Lb1.insert(0, "-------------------------------")
    Lb1.insert(0, f"TOPIC: {topic}")
    Lb1.insert(0, f"MSG: {mensaje}")


############################################################################
def Activar_sistema():
    #Primero nos conectamos al broker MQTT
    try:
        IP_Broker=MQTT_broker.get()
        puerto=8883
        timeout_seg=60
        #
        mqttc.username_pw_set(username=usuario_mqtt, password=clave)
        mqttc.connect(IP_Broker, puerto,timeout_seg)
        time.sleep(1)
        mqttc.loop_start()
        time.sleep(0.1)
        Lb1.insert(0, "-------------------------------")
        Lb1.insert(0,"Conectado a "+IP_Broker)
        Lb1.insert(0,"cliente MQTT: "+cliente_MQTT)
    except Exception as e:
        Lb1.insert(0, "-------------------------------")
        Lb1.insert(0,f"Error conexion: {e}")
    time.sleep(1)
##########################################################################
def Desactivar_sistema():
    mqttc.unsubscribe(MQTT_topic_recibir1.get()) # Unsubscribe from the specific topic
    mqttc.disconnect()
    mqttc.loop_stop()
    time.sleep(1)
    Lb1.insert(0, "-------------------------------")
    Lb1.insert(0,"Desconectado")
    #
##########################################################################
def InfoSubscribe(mqttc,userdata,mid,granted_qos):
    time.sleep(0.1)
    Lb1.insert(0, "-------------------------------")
    Lb1.insert(0,"Nos hemos subscrito. granted_qos = "+str(granted_qos))

########################################################################
def InfoConnect(mqttc,userdata,flags,rc):
    global carpeta_raiz
    global DB
    time.sleep(0.1)
    """
        El valor de rc indica el resultado de la conexión:
            0: Connection successful 
            1: Connection refused - incorrect protocol version 
            2: Connection refused - invalid client identifier 
            3: Connection refused - server unavailable 
            4: Connection refused - bad username or password 
            5: Connection refused - not authorised 
            6-255: Currently unused.
    """
    Lb1.insert(0, "-------------------------------")
    Lb1.insert(0,"Nos hemos conectado. rc = "+str(rc))
    if (rc == 0):
        # Creamos una tabla para guardar a granel todos los datos recibidos
        # Una especie de log en tabla1_mensajes
        # Crear carpeta
        crear_tabla1_mensajes()
        #
        # Esta sería una tabla donde guardariamos los datos recibidos separados
        # en columnas
        crear_tabla2_medidas()
        #Ahora nos subscribimos
        mqttc.subscribe(MQTT_topic_recibir1.get(),qos)
        Lb1.insert(0, "-------------------------------")
        Lb1.insert(0,"subscripcion a "+MQTT_topic_recibir1.get()+" lista")
    else:
        Lb1.insert(0, "-------------------------------")
        Lb1.insert(0,"La conexion con el broker ha fallado")
########################################################################

def Resume_motor():
    mqttc.publish(MQTT_topic_resume,"1",qos=2)
    Lb1.insert(0, "-------------------------------")
    Lb1.insert(0, "Resume Motor")


def Stop_motor():
    mqttc.publish(MQTT_topic_stop,"0",qos=2)
    Lb1.insert(0, "-------------------------------")
    Lb1.insert(0, "Stop Motor")

def Get_Overlape():
    # Implement your logic for Get_Overlape here
    Lb1.insert(0, "-------------------------------")
    Lb1.insert(0, "Function 'Get_Overlape' called (To be implemented)")


def Reset_Data():
    mqttc.publish(MQTT_topic_reset,"1",qos=2)
    Lb1.insert(0, "-------------------------------")
    Lb1.insert(0, "Reset Data")


def Send_Steps():
    try: 
        number = float(steps.get())
        mqttc.publish(MQTT_topic_steps, str(number), qos=2)
        Lb1.insert(0, "-------------------------------")
        Lb1.insert(0, f"Número enviado: {number}")
        Lb1.insert(0, f"Tópico: {MQTT_topic_steps}")
        Lb1.insert(0, "Orden de número manual enviada:")
    except ValueError:
        Lb1.insert(0, "-------------------------------")
        Lb1.insert(0, "Error: Por favor, introduce un número válido.")
    except Exception as e:
        Lb1.insert(0, "-------------------------------")
        Lb1.insert(0, f"Error al enviar número: {e}")    

def Send_Spin_Right():
    mqttc.publish(MQTT_topic_spin, "1", qos=2)
    Lb1.insert(0, "-------------------------------")
    Lb1.insert(0, "Turn Right")

def Send_Spin_Left():
    mqttc.publish(MQTT_topic_spin, "0", qos=2)
    Lb1.insert(0, "-------------------------------")
    Lb1.insert(0, "Turn Left")

def send_data_step_size(event):
    opt = box_options.get()
    if(opt == options_step_size[0]):
        mqttc.publish(MQTT_topic_change_step,"1",qos=2)
        Lb1.insert(0, "-------------------------------")
        Lb1.insert(0, "Size Step = FULL STEP")
    if(opt == options_step_size[1]):
        mqttc.publish(MQTT_topic_change_step,"2",qos=2)
        Lb1.insert(0, "-------------------------------")
        Lb1.insert(0, "Size Step = HALF STEP")
    if(opt == options_step_size[2]):
        mqttc.publish(MQTT_topic_change_step,"3",qos=2)
        Lb1.insert(0, "-------------------------------")
        Lb1.insert(0, "Size Step = QUARTER STEP")
    if(opt == options_step_size[3]):
        mqttc.publish(MQTT_topic_change_step,"4",qos=2)
        Lb1.insert(0, "-------------------------------")
        Lb1.insert(0, "Size Step = EIGHT STEP")
    if(opt == options_step_size[4]):
        mqttc.publish(MQTT_topic_change_step,"5",qos=2)
        Lb1.insert(0, "-------------------------------")
        Lb1.insert(0, "Size Step = SIXTEENTH STEP")
########################################################################

class MyApp:
    def __init__(self, parent):
        self.parent = parent
        self.parent.protocol("WM_DELETE_WINDOW", self.on_closing)

    def on_closing(self):
        print("Salimos en 1 segundo---")
        time.sleep(1)
        print("Quitamos subscripciones y desconectamos MQTT---")
        mqttc.unsubscribe("#") # Unsubscribe from all topics
        mqttc.disconnect()
        mqttc.loop_stop()
        #print("\014")   #Borra consola
        self.parent.destroy()	
#--------------------------------------------------------------------------------------------------------
# INICIO PROGRAMA
#--------------------------------------------------------------------------------------------------------
# Definimos los Callbacks del MQTT
mqttc.subscribe(MQTT_topic_recibir1,qos) # This subscribe might be too early if the connection isn't established yet
mqttc.on_message = RecibirDato       # Callback de recibir datos
mqttc.on_subscribe = InfoSubscribe   # Callback de susbcribirse
mqttc.on_connect = InfoConnect       # Callback de conectarse # This line was commented in your original code, but is crucial for InfoConnect to be called

#--------------------------------------------------------------------------------------------------------
# Creamos una ventana de trabajo
#--------------------------------------------------------------------------------------------------------
ventana = Tk()
app = MyApp(ventana)
ventana.config(bd=10)
ventana.config(relief="groove")
ventana.config(cursor="arrow")
ventana.config(bg="gray")
ventana.title("User Interface : MQTT Panel")
ventana.geometry('1000x625')
ventana.iconbitmap(r"C:\Users\Admin\Desktop\Almacen\3-Codigos\HY4RES\Python_sistema_OM\icono_02.ico")
#
foto_base = PhotoImage(file = r"C:\Users\Admin\Desktop\Almacen\3-Codigos\HY4RES\Python_sistema_OM\turbina_savonius.png")
foto1=foto_base.zoom(3,3)                # Aumentar
foto_final=foto1.subsample(14,14)        # Disminuir
marco=Label(image=foto_final)
marco.place(x=620,y=70)
# Activamos el cambio de temperatura cuando activemos el gemelo
# ventana.after(2000,cambia_temperatura)
#--------------------------------------------------------------------------------------------------------
#--------------------------------------------------------------------------------------------------------
# ETIQUETAS (Label)
#--------------------------------------------------------------------------------------------------------
etiqueta_1=Label(ventana, text="USER INTERFACE TURBINE SAVONIUS PITCH CONTROL", 
                  fg='black',bg='gray')
etiqueta_1.place(x=10,y=0)
etiqueta_1.config(font=("Calibri Bold",20))

etiqueta_2=Label(ventana, 
                 text=" IP BROKER MQTT",fg='white',bg='black')
etiqueta_2.place(x=240,y=50)
etiqueta_2.config(font=("Calibri Bold",9))
#------
etiqueta_3=Label(ventana, 
                 text=">>> IP 156.35.154.170 BROKER MQTT CE3I2",fg='black',bg='gray')
etiqueta_3.place(x=150,y=75)
etiqueta_3.config(font=("Calibri Bold",9))
#------
etiqueta_4=Label(ventana, text="TOPICS & DATABASE",fg='white',bg='black')
etiqueta_4.place(x=235,y=115)
etiqueta_4.config(font=("Calibri Bold",9))
#------
etiqueta_5=Label(ventana, text="Topic para subscribirse",fg='black',bg='gray')
etiqueta_5.place(x=50,y=135)
etiqueta_5.config(font=("Calibri Bold",9))
#------
etiqueta_6=Label(ventana, text="Topic para enviar",fg='black',bg='gray')
etiqueta_6.place(x=50,y=185)
etiqueta_6.config(font=("Calibri Bold",9))
#------
#------
etiqueta_7=Label(ventana, text="Nombre BD SQLite",fg='black',bg='gray')
etiqueta_7.place(x=350,y=135)
etiqueta_7.config(font=("Calibri Bold",9))
#------
etiqueta_9=Label(ventana, text="Turbine Savonius : Overlape Control",fg='white',bg='black')
etiqueta_9.place(x=200,y=250)
etiqueta_9.config(font=("Calibri Bold",9))
#------

etiqueta_panel=Label(ventana, text="LOG OUT : Informational messages",
                          fg='white',bg='black')
etiqueta_panel.place(x=210,y=410)
etiqueta_panel.config(font=("Calibri bold", 9))
#------
etiqueta_autor=Label(ventana, text="Daniel Mallada Fernández (Universidad de Oviedo - Grupo CE3I2)",fg='white',bg='black')
etiqueta_autor.place(x=620,y=580)

#--------------------------------------------------------------------------------------------------------
# ENTRADAS DE VARIABLES (Entry)
#--------------------------------------------------------------------------------------------------------

steps = StringVar()
steps_enter = Entry(ventana, justify=LEFT, width=15, textvariable=steps, bg="white")
steps_enter.place(x=250, y=350)

MQTT_broker = StringVar()
MQTT_broker.set(IP_inicial)
entrada_1=Entry(ventana, justify=LEFT, width=15, textvariable=MQTT_broker)
entrada_1.place(x=50,y=75)
#------
MQTT_topic_recibir1 = StringVar()
MQTT_topic_recibir1.set(topic_recibir1)
entrada_2=Entry(ventana, justify=LEFT, width=30, textvariable=MQTT_topic_recibir1)
entrada_2.place(x=50,y=155)
#------
MQTT_topic_enviar1 = StringVar()
MQTT_topic_enviar1.set(topic_enviar1)
entrada_3=Entry(ventana, justify=LEFT, width=30, textvariable=MQTT_topic_enviar1)
entrada_3.place(x=50,y=210)
#------
nombre_DB = StringVar()
nombre_DB.set(nombre_DB_inicial)
entrada_4=Entry(ventana, justify=LEFT,width=10, textvariable=nombre_DB)
entrada_4.place(x=350,y=155)
#------
msg_to_send = StringVar()
msg_to_send_enter=Entry(ventana, justify=LEFT, width=30, textvariable=msg_to_send)
msg_to_send_enter.place(x=350,y=210)

# Declare pp as a StringVar for the GUI to access it
pp = StringVar()
# You might want to set a default value for pp if it's used in crear_tabla2_medidas
pp.set("2") # Example default value, adjust as needed

#--------------------------------------------------------------------------------------------------------
# PANEL INFORMATIVO
#--------------------------------------------------------------------------------------------------------
scroll = Scrollbar(ventana)  # Barra de deslizamieto vertical
Lb1 = Listbox(ventana,yscrollcommand = scroll.set)
Lb1.config(width=80)
Lb1.config(height=6)
Lb1.place(x=50,y=450)
scroll.config( command = Lb1.yview) 
scroll.place(x=550,y=450)
#--------------------------------------------------------------------------------------------------------
# BOTONES (Button)
#--------------------------------------------------------------------------------------------------------
boton_1=Button(ventana, text="HELP",
                      command=ayuda,fg="white",bg="black")
boton_1.place(x=925,y=550)
boton_1.config(font=("Calibri Bold",9))
#------
boton_2=Button(ventana, text="CONECT SYSTEM", 
                           command=Activar_sistema,fg="white",bg='green')
boton_2.place(x=650,y=5)
boton_2.config(font=("Calibri Bold",12))
#------
boton_3=Button(ventana, text="DISCONNECT SYSTEM", 
                           command=Desactivar_sistema,fg="white",bg='red')
boton_3.place(x=800,y=5)
boton_3.config(font=("Calibri Bold",12))

#------
boton_4=Button(ventana, text="Delete all data", 
                       command=borrar_datos_recibidos,fg="red",bg='white')
boton_4.place(x=30,y=570)
boton_4.config(font=("Calibri Bold",9))

Button_resume=Button(ventana, text="Resume Motor", command=Resume_motor, fg="black", bg='white')
Button_resume.place(x=50, y=275)
Button_resume.config(font=("Calibri Bold",9))

Button_stop=Button(ventana, text="Stop Motor", command=Stop_motor, fg="black", bg="white")
Button_stop.place(x=50, y=310)
Button_stop.config(font=("Calibri Bold",9))


Button_overlape=Button(ventana, text="Overlape", command=Get_Overlape, bg='white', fg='black')
Button_overlape.place(x=50, y=345)
Button_overlape.config(font=("Calibri Bold",9))

Button_reset_data=Button(ventana, text="Reset Data Overlape", command=Reset_Data, bg="white", fg="black")
Button_reset_data.place(x=50, y=380)
Button_reset_data.config(font=("Calibri Bold",9))


Button_steps=Button(ventana, text="Steps", command=Send_Steps, fg='black', bg='white')
Button_steps.place(x=200, y=350)
Button_steps.config(font=("Calibri Bold",9))


Panel_Options=LabelFrame(ventana, text="Choose Direction", bg="gray", fg="black")
Panel_Options.place(x=400, y=275)
Panel_Options.config(font=("Calibri Bold",9))


option_chosen = StringVar()

option_one = Radiobutton(Panel_Options, text="Right", variable=option_chosen, value="1", command=Send_Spin_Right)
option_one.pack(anchor="w", pady=5, padx=20)

option_two = Radiobutton(Panel_Options, text="Left", variable=option_chosen, value="0", command=Send_Spin_Left)
option_two.pack(anchor="w", pady=5, padx=20)

panel_options_step=LabelFrame(ventana, text="Size Step", bg="gray", fg="black")
panel_options_step.place(x=200, y=275)

option_step="Full Step"

options_step_size = [
    "Full Step",
    "Half Step",
    "Quarter Step",
    "Eight Step",
    "Sixteenth Step"
]

box_options = ttk.Combobox(panel_options_step, textvariable=option_step, values=options_step_size, state="readonly")
box_options.pack(pady=10, padx=10)

box_options.bind("<<ComboboxSelected>>", send_data_step_size)


#--------------------------------------------------------------------------------------------------------
ventana.mainloop()
#--------------------------------------------------------------------------------------------------------
###--- FIN DEL SCRIPT --> Este codigo contiene una pantalla dentro de la pantalla tkinter para mostrar mensajes, quiero qeu me muestre el valor que llega del topico "ae/#"