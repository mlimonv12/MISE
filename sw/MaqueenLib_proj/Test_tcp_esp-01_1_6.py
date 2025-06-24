# v1.1:
# - Añadido un "except IndexError" en el "try" de lectura.
# Este error salta con los mensajes AT no esperados del módulo ESP-01, ya que no cumplen
# con la estructura Bioloid que esperamos. Concretamente, en una trama Bioloid, esperamos que el
# byte 4 leido nos indique cuantos bytes más faltan por leer, pero en un mensaje AT no es así.
# p. ej., al cabo de un rato sin actividad, el módulo ESP-01 nos desconecta, enviando un mensaje
# "...Closed...", el cual provoca el error mencionado.
# Con esta actualización, estos mensajes ya no cuelgan nuestro script python.
# - Añadido: comprobación de la presencia de la librería Pmw para los tooltips. Si no está presente,
# sigue sin tooltips, avisando al usuario de que no estarán disponibles.
# - Ahora se indica la versión en la barra de título de la ventana y en "Acerca de"

# v1.2:
# - Corregido error de cálculo en la función checksum(), que se dejaba el último byte
# - import Pmw (tooltips lib) ahora dentro de un bloque "try", de modo que se captura la excepción "import error",
#  y ya no genera una condición de error. En cambio, imprimimos un warning en la consola avisando que los tooltips
#  no estarán disponibles. Además, se asigna una variable global que condiciona la inicialización de los tooltips
#  según la presencia o no de la librería.

# v1.3:
# - Añadido control de la ID con una lista "lista_ID_Modulos" de IDs validas.
# Ahora se puede tener en cuenta la ID_Broadcast, que se ha implementado en todos los comandos, aunque
# en esta aplicacion, solo es relevante en el caso de los ADCs (valores de los LDRs).
# - Refactorizada funcion AX12_func para recibir id_modulo como parametro y pasarsela a las funciones de su lista.
# Las funciones de esta lista tambien se han refactorizado para recibir este id como parametro (en lugar de actuar con
# una variable ID global).

# v1.4:
# - libreria de tooltips Pmw sustituida por tktooltip, más actualizado respecto a python 3
# (de hecho, Pmw da errores con python 3.10)
# Esta librería se instala con
# pip install tkinter-tooltip
# en una ventana de comandos del sistema.

# v1.5:
# - comprobacion de la IP introducida:
#   - si el caracter introducido no es valido [0..9, .], no se cambia
#   - si el formato no es valido: xxx.xxx.xxx.xxx 4 bytes (max 3 caracteres cada byte y cada uno  <= 255)
#   separados por puntos, se lanza un mensaje de error.

# v1.6:
# - eliminada la comprobacion de la IP introducida, por poco práctica al uso!
# - añadido boton rojo para parar los motores a la vez rápida y comodamente.
# - cuadro para visualizar el valor de la distancia medida por el sensor de ultrasonidos.

import tkinter as tk
from tkinter import *
from tkinter import ttk
import threading
import queue
import time
import struct
from tkinter import messagebox
import socket
import os
os.system("")

ToolTip_presente = 1
CRED = '\033[31m'
CEND = '\033[0m'
try:
    from tktooltip import ToolTip
except ImportError:
    # warnings.warn("\nlib tktooltip not found, tooltips will not be available.")
    print(CRED + "Warning: lib tktooltip not found, tooltips will not be available." + CEND)
    print("On most systems, this can be installed with \"pip install tkinter-tooltip\" ")
    print("command from a terminal window\n")
    ToolTip_presente = 0


# =================================================================
Default_IP = "192.168.1.42"  # Editar con la IP del módulo ESP-01
# =================================================================

version = "1.6"
root = Tk()

DEBUG_trama_check = IntVar()
DEBUG_Moduls_check = IntVar()
DEBUG_Consola_check = IntVar()
DEBUG_trama_check.set(0)
DEBUG_Moduls_check.set(0)
DEBUG_Consola_check.set(0)
DEBUG_trama = DEBUG_trama_check.get()
DEBUG_Moduls = DEBUG_Moduls_check.get()
DEBUG_Consola = DEBUG_Consola_check.get()
texto_trama = StringVar()
texto_motor_left = StringVar()
texto_motor_right = StringVar()
valor_barra_izq = IntVar()
valor_barra_der = IntVar()
valor_distancia = DoubleVar()
string_distancia = StringVar()
Placa = StringVar()
Identificador = StringVar()
State_check = IntVar()
Velocidad = IntVar()
Sentido = IntVar()
Velocidad.set(100)
Sentido.set(1)
valor_distancia.set(0.0)
string_distancia.set("0.0")
IP_Text1 = StringVar()
IP_Text1.set(Default_IP)
HOST = IP_Text1.get()
PORT = 80
s_client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
conectado = 0
comando = ""
comandos = {"LEDS RGB": 0x19, "Motores": 0x20}
identificadores = {"Left": 1, "Right": 2, "Ambos": 0xFE}
colores = {"Apagado": 0, "Rojo": 1, "Verde": 2, "Naranja": 3, "Azul": 4, "Magenta": 5, "Cian": 6, "Blanco": 7}
estados = {"OFF": 0, "ADELANTE": 1, "ATRAS": 2}

ValoresRGB = [0, 1, 2, 3, 4, 5, 6, 7]
ColoresRGB = ["Apagado", "Rojo", "Verde", "Naranja", "Azul", "Magenta", "Cian", "Blanco"]
EstadosMotores = ["OFF", "ADELANTE", "ATRAS"]
Identificadores = ["Left", "Right", "Ambos"]
ColorRGB = 3
INSTR_IDLE = 0x00
INSTR_END = 0xFF
INSTR_STOP_THREAD = 0xEE

INSTR_PING = 0x01
INSTR_READ = 0x02
INSTR_WRITE = 0x03
INSTR_REG_WR = 0x04
INSTR_ACTION = 0x05
INSTR_RESET = 0x06
INSTR_SYNC_WRT = 0x83

ID_L = 0x01
ID_R = 0x02
ID_Broadcast = 0xFE
n_motors = 2  # numero de motores (sin modulo de sensores)
lista_ID_Modulos = [ID_L, ID_R, ID_Broadcast]
rol_modulos = {ID_L: "Left Motor", ID_R: "Right Motor"}
index_modulos = {ID_L: 0, ID_R: 1}
for index in index_modulos:
    print("ID:", index, rol_modulos[index], "@:", index_modulos[index])
CCW = 0x02  # Bioloid
CW = 0x04  # Bioloid
Adelante = 1
Atras = 2
Parado = 0
AX12_moving_L = ""
AX12_moving_R = ""
ESTAT_ROBOT = ""

ID = 0
trama = b''
timeout = 0.01  # para lectura puerto, en s
lectura = 0
leyendo = 0
seguir = 1

instruction_set = {
        0x00: "IDLE",
        0x01: "PING",
        0x02: "READ",
        0x03: "WRITE",
        0x04: "REG_WRITE",
        0x05: "ACTION",
        0x06: "RESET",
        0x83: "SYNC_WRITE",
        0xFF: "END"
}

AX12_memory = {
        0x00: "Model Number(L)",
        0x01: "Model Number(H)",
        0x02: "Firmware Version",
        0x03:  "ID",
        0x04: "Baud Rate",
        0x05: "Return Delay Time",
        0x06: "CW Angle Limit(L)",
        0x07: "CW Angle Limit(H)",
        0x08: "CCW Angle Limit(L)",
        0x09: "CCW Angle Limit(H)",
        0x0A: "Reserved",
        0x0B: "High Temp. Limit",
        0x0C: "Low Voltage Limit",
        0x0D: "High Voltage Limit",
        0x0E: "Max Torque(L)",
        0x0F: "Max Torque(H)",
        0x10: "Status Return Level",
        0x11: "Alarm LED",
        0x12: "Alarm Shoutdown",
        0x13: "Reserved",
        0x14: "Down Calibration(L)",
        0x15: "Down Calibration(H)",
        0x16: "Up Calibration(L)",
        0x17: "Up Calibration(H)",
        0x18: "Torque Enable",
        0x19: "LED",
        0x1A: "CW Compliance Margin",
        0x1B: "CCW COmpliance Margin",
        0x1C: "CW Compliance Slope",
        0x1D: "CCW Compliance Slope",
        0x1E: "Goal Position(L)",
        0x1F: "Goal Position(H)",
        0x20: "Moving Speed(L)",
        0x21: "Moving Speed(H)",
        0x22: "Torque Limit(L)",
        0x23: "Torque Limit(H)",
        0x24: "Present Position(L)",
        0x25: "Present Position(H)",
        0x26: "Present Speed(L)",
        0x27: "Present Speed(H)",
        0x28: "Distance (L)",
        0x29: "Distance (byte 1)",
        0x2A: "Distance (byte 2)",
        0x2B: "Distance (H)",
        0x2C: "Registered Instruction",
        0x2D: "ADC_value (L)",
        0x2E: "ADC_value (H)",
        0x2F: "Lock",
        0x30: "Punch(L)",
        0x31: "Punch(H)"
        }


def f_led(id_modulo):
    global ID
    if id_modulo == ID_Broadcast:
        estat_LED = AX12[index_modulos[ID_L]][0x19]
        # Cogemos cualquier ID, seran iguales, ya que se les acaba de escribir un valor con ID_Broadcast
    else:
        estat_LED = AX12[index_modulos[id_modulo]][0x19]
    if estat_LED == 1:
        text_led = "ON "
    else:
        text_led = "OFF"
    if DEBUG_Consola == 1:
        print("LED motor", id_modulo, text_led)
    return


def f_moving_speed(id_modulo):
    global AX12_moving_L
    global AX12_moving_R
    if id_modulo == ID_Broadcast:
        velocitat = AX12[index_modulos[ID_L]][0x20] + (AX12[index_modulos[ID_L]][0x21] & 0x03) * 256
        sentit = AX12[index_modulos[ID_L]][0x21] & 0x04
        # Cogemos cualquier ID, seran iguales, ya que se les acaba de escribir un valor con ID_Broadcast
        text_motor = "v=" + str(velocitat)
        if sentit == 0:
            text_sentit = "CCW"
        else:
            text_sentit = "CW"
        if DEBUG_Consola == 1:
            print("Velocitat  motor", id_modulo, "=", velocitat)
            print("Sentit gir motor", id_modulo, "=", text_sentit)
        AX12_moving_L = text_motor + " " + text_sentit
        AX12_moving_R = text_motor + " " + text_sentit
        return
    # Si no es la ID_Broadcast:
    velocitat = AX12[index_modulos[id_modulo]][0x20] + (AX12[index_modulos[id_modulo]][0x21] & 0x03) * 256
    if DEBUG_Consola == 1:
        print("Velocitat  motor", id_modulo, "=", velocitat)
    text_motor = "v=" + str(velocitat)
    sentit = AX12[index_modulos[id_modulo]][0x21] & 0x04
    if sentit == 0:
        if DEBUG_Consola == 1:
            print("Sentit gir motor", id_modulo, "= CCW")
        if id_modulo == 1:
            AX12_moving_L = text_motor + " CCW"
        else:
            AX12_moving_R = text_motor + " CCW"
    else:
        if DEBUG_Consola == 1:
            print("Sentit gir motor", id_modulo, "= CW")
        if id_modulo == 1:
            AX12_moving_L = text_motor + " CW"
        else:
            AX12_moving_R = text_motor + " CW"
    return


def f_angle_limit(id_modulo):
    if id_modulo == ID_Broadcast:
        cw_angle = AX12[index_modulos[ID_L]][0x06] + (AX12[index_modulos[ID_L]][0x07] & 0x03) * 256
        ccw_angle = AX12[index_modulos[ID_L]][0x08] + (AX12[index_modulos[ID_L]][0x09] & 0x03) * 256
        # Cogemos cualquier ID, seran iguales, ya que se les acaba de escribir un valor con ID_Broadcast
    else:
        cw_angle = AX12[index_modulos[id_modulo]][0x06] + (AX12[index_modulos[id_modulo]][0x07] & 0x03) * 256
        ccw_angle = AX12[index_modulos[id_modulo]][0x08] + (AX12[index_modulos[id_modulo]][0x09] & 0x03) * 256
    if DEBUG_Consola == 1:
        if cw_angle == 0:
            print("Motor", id_modulo, "gir continu en sentit horari")
        else:
            print("Motor", id_modulo, "angle limit en sentit horari:", cw_angle)
        if ccw_angle == 0:
            print("Motor", id_modulo, "gir continu en sentit anti-horari")
        else:
            print("Motor", id_modulo, "angle limit en sentit anti-horari:", ccw_angle)
    return


def f_ADC_value(id_modulo):
    if id_modulo == ID_Broadcast:
        ADC_val = (AX12[index_modulos[ID_L]][0x2D] | (AX12[index_modulos[ID_L]][0x2E] << 8)) & 0x0FFF
        # Cogemos cualquier ID, seran iguales, ya que se les acaba de escribir un valor con ID_Broadcast
        valor_barra_izq.set(ADC_val)
        valor_barra_der.set(ADC_val)
    else:
        ADC_val = (AX12[index_modulos[id_modulo]][0x2D] | (AX12[index_modulos[id_modulo]][0x2E] << 8)) & 0x0FFF
        if id_modulo == ID_L:
            valor_barra_izq.set(ADC_val)
        elif id_modulo == ID_R:
            valor_barra_der.set(ADC_val)

    if DEBUG_Consola == 1:
        print("Mesura del ADC", id_modulo, ":", ADC_val)
    return


def f_Distancia(id_modulo): # dato float del u-sonido
    if id_modulo == ID_Broadcast:
        d_bytes = [AX12[index_modulos[ID_L]][0x28],
                   AX12[index_modulos[ID_L]][0x29],
                   AX12[index_modulos[ID_L]][0x2A],
                   AX12[index_modulos[ID_L]][0x2B]]
    else:
        d_bytes = [AX12[index_modulos[id_modulo]][0x28],
                   AX12[index_modulos[id_modulo]][0x29],
                   AX12[index_modulos[id_modulo]][0x2A],
                   AX12[index_modulos[id_modulo]][0x2B]]
    str_bytes = bytes(d_bytes)
    [distancia] = struct.unpack('f', str_bytes[0:4])
    valor_distancia.set(distancia)
    string_distancia.set(f'{float(distancia):.02f}')
    # print(AX12[index_modulos[ID_L]][0x2B], AX12[index_modulos[ID_L]][0x2A],
    #       AX12[index_modulos[ID_L]][0x29], AX12[index_modulos[ID_L]][0x28])
    # print("Distancia string", str_bytes)
    # print("Distancia float", string_distancia.get())


def AX12_func(argument, id_modulo):
    if id_modulo not in lista_ID_Modulos:
        return
    switcher = {
        #           0x00: "Model Number(L)",
        #           0x01: "Model NUmber(H)",
        #           0x02: "Firmware Version",
        #           0x03:  "ID",
        #           0x04: "Baud Rate",
        #           0x05: "Return Delay Time",
        0x06: f_angle_limit,
        0x07: f_angle_limit,
        0x08: f_angle_limit,
        0x09: f_angle_limit,
        #           0x0A: "Reserved",
        #           0x0B: "High Temp. Limit",
        #           0x0C: "Low Voltage Limit",
        #           0x0D: "High Voltage Limit",
        #           0x0E: "Max Torque(L)",
        #           0x0F: "Max Torque(H)",
        #           0x10: "Status Return Level",
        #           0x11: "Alarm LED",
        #           0x12: "Alarm Shoutdown",
        #           0x13: "Reserved",
        #           0x14: "Down Calibration(L)",
        #           0x15: "Down Calibration(H)",
        #           0x16: "Up Calibration(L)",
        #           0x17: "Up Calibration(H)",
        #           0x18: "Torque Enable",
        0x19: f_led,
        #           0x1A: "CW Compliance Margin",
        #           0x1B: "CCW COmpliance Margin",
        #           0x1C: "CW Compliance Slope",
        #           0x1D: "CCW Compliance Slope",
        #           0x1E: "Goal Position(L)",
        #           0x1F: "Goal Position(H)",
        0x20: f_moving_speed,
        0x21: f_moving_speed,
        #           0x22: "Torque Limit(L)",
        #           0x23: "Torque Limit(H)",
        #           0x24: "Present Position(L)",
        #           0x25: "Present Position(H)",
        #           0x26: "Present Speed(L)",
        #           0x27: "Present Speed(H)",
        0x28: f_Distancia, # byte 0 de la distancia,
        #           0x29:  # byte 1 de la distancia,
        #           0x2A:  # byte 2 de la distancia,
        #           0x2B:  # byte 3 de la distancia,
        #           0x2C: "Registered Instruction",
        0x2D: f_ADC_value,
        #           0x2E: "Moving",
        #           0x2F: "Lock",
        #           0x30: "Punch(L)",
        #           0x31: "Punch(H)"
    }
    cadena_error = "Funció " + format(argument, '#04x') + " no implementada"
    func = switcher.get(argument, lambda id_modulo: print(cadena_error))
    func(id_modulo)


def reset_modul_AX12(id_modulo):
    id_modul = index_modulos[id_modulo]
    AX12[index_modulos[id_modulo]][0x00] = 0x0C  # Model Number(L)
    AX12[id_modul][0x02] = 0x01  # Model Number(H)
    AX12[id_modul][0x03] = id_modul  # ID
    AX12[id_modul][0x04] = 0x01  # Baud Rate
    AX12[id_modul][0x05] = 0xFA  # Return Delay Time
    AX12[id_modul][0x06] = 0x00  # CW Angle Limit(L)
    AX12[id_modul][0x07] = 0x00  # CW Angle Limit(H)
    AX12[id_modul][0x08] = 0xFF  # CCW Angle Limit(L)
    AX12[id_modul][0x09] = 0x03  # CW Angle Limit(H)
    AX12[id_modul][0x0B] = 0x55  # Highest Limit Temperature
    AX12[id_modul][0x0C] = 0x3C  # Lowest Limit Voltage
    AX12[id_modul][0x0D] = 0xBE  # Highest Limit Voltage
    AX12[id_modul][0x0E] = 0xFF  # Max Torque(L)
    AX12[id_modul][0x0F] = 0x03  # Max Torque(H)
    AX12[id_modul][0x10] = 0x02  # Status Return Level
    AX12[id_modul][0x11] = 0x04  # Alarm LED
    AX12[id_modul][0x12] = 0x04  # Alarm Shoutdown
    AX12[id_modul][0x22] = AX12[id_modul][0x0E]  # Torque Limit(L)
    AX12[id_modul][0x23] = AX12[id_modul][0x0F]  # Torque Limit(H)
    return


# comprova si hi ha error de checksum a la trama
def comprova_checksum(frame):
    len_trama = len(frame)
    chk_sum = 0
    for index in range(2, (len_trama - 1)):
        chk_sum = chk_sum + frame[index]
    chk_sum = chk_sum & 0xFF
    if (chk_sum | frame[len_trama - 1]) == 0xFF:
        #       ser.write(b'\x00')
        if DEBUG_trama:
            print('Checksum correcte')
        return 0x00
    else:
        print('Error de Checksum')
        #        ser.write(b'\x10')
        return 0x10


# comprova si hi ha error d'instruccio la trama
def comprova_instr(instruccio):
    if (instruccio < 0x07) or (instruccio == 0xFF) or (instruccio == 0x83):
        if DEBUG_Consola == 1:
            print("Instrucció:", instruction_set[instruccio])
        return 0x00
    else:
        print("Error d'instruccio")
        return 0x70


def send_status_packet(modul_id, error_code):
    status_frame = Status_NoError[:]
    status_frame[2] = modul_id
    status_frame[4] = error_code
    len_trama = len(status_frame)
    l_chksum = 0
    for index in range(2, (len_trama - 1)):
        l_chksum = l_chksum + status_frame[index]
    l_chksum = (~l_chksum & 0xFF)
    status_frame[len_trama - 1] = l_chksum
    string = ''.join(['0x%02X ' % b for b in status_frame])
    if DEBUG_trama:
        print("status packet in hex:", string)
        print("status packet in dec:", status_frame)
    # ser.write(status_frame) # cambiar por envio TCP
    s_client.send(bytes(status_frame))
    return


def checksum(frame):
    len_trama = len(frame)
    l_chksum = 0
    for index in range(2, len_trama):
        l_chksum = l_chksum + frame[index]
    l_chksum = (~l_chksum & 0xFF)
    return l_chksum


def generate_read_packet():
    return


def generate_status_packed(id_modul, instruc, code_error):
    if instruc != 2:
        send_status_packet(id_modul, code_error)
    else:
        generate_read_packet()
    return


# dona l'estat del robot Maqueen
def robot2_status():
    global ESTAT_ROBOT, AX12_moving_L, AX12_moving_R
    if DEBUG_Consola == 1:
        print("----------- ESTAT DEL ROBOT -----------------------")
    # Recuperamos velocidades y sentidos desde la memoria del AX12:
    v_left = AX12[index_modulos[ID_L]][0x20]
    sentit_left = AX12[index_modulos[ID_L]][0x21]
    v_right = AX12[index_modulos[ID_R]][0x20]
    sentit_right = AX12[index_modulos[ID_R]][0x21]

    # Estado de cada motor:
    # Motor izq.:
    if v_left == 0:
        AX12_moving_L = "Parat"
    else:
        text_motor = "v=" + str(v_left)
        if sentit_left == Adelante:
            AX12_moving_L = text_motor + " CCW"
        elif sentit_left == Atras:
            AX12_moving_L = text_motor + " CW"
        else:
            AX12_moving_L = "Parat"
    # Motor der.:
    if v_right == 0:
        AX12_moving_R = "Parat"
    else:
        text_motor = "v=" + str(v_right)
        if sentit_right == Adelante:
            AX12_moving_R = text_motor + " CW"
        elif sentit_right == Atras:
            AX12_moving_R = text_motor + " CCW"
        else:
            AX12_moving_R = "Parat"

    # Estado del robot:
    if (v_left == 0 and v_right == 0) or (sentit_left == 0 and sentit_right == 0):  # Tot 0, robot parat
        if DEBUG_Consola == 1:
            print("Robot Parat")
        ESTAT_ROBOT = "Robot Parat"
    elif v_left == v_right and sentit_left == sentit_right:  # mateixa velocitat y mateix sentit => robot  va recte
        if sentit_left == Adelante:
            if DEBUG_Consola == 1:
                print("Robot Marxa Endavant")
            ESTAT_ROBOT = "Robot Marxa Endavant"
        else:
            if DEBUG_Consola == 1:
                print("Robot Marxa Enrere")
            ESTAT_ROBOT = "Robot Marxa Enrere"
    elif v_left > v_right:  # velocitats diferents, motor esquerre mes rapid
        if sentit_left == Atras and sentit_right == Adelante:
            if DEBUG_Consola == 1:
                print("Robot Gira Esquerra")
            ESTAT_ROBOT = "Robot Gira Esquerra"
        else:
            if DEBUG_Consola == 1:
                print("Robot Gira Dreta")
            ESTAT_ROBOT = "Robot Gira Dreta"
    else:  # velocitats diferents, motor dret mes rapid
        if sentit_left == Adelante and sentit_right == Atras:
            if DEBUG_Consola == 1:
                print("Robot Gira Dreta")
            ESTAT_ROBOT = "Robot Gira Dreta"
        else:
            if DEBUG_Consola == 1:
                print("Robot Gira Esquerra")
            ESTAT_ROBOT = "Robot Gira Esquerra"

    # Actualizando grafica:
    texto_trama.set(ESTAT_ROBOT)
    texto_motor_left.set(AX12_moving_L)
    texto_motor_right.set(AX12_moving_R)
    return


def print_AX_MemoryMap():
    label = "=========== MODUL ======== "
    for i in range(n_motors):
        label = label + ''.join(['[%d] ======= ' % (lista_ID_Modulos[i])])
    print("---------------------------------------------------")
    print(label)
    # print("========== MOTOR ======== [1] ======= [2]")

    for i in range(50):
        mot1 = ''.join(['0x%02X ' % (AX12[0][i])])  # format Hexadecimal
        mot2 = ''.join(['0x%02X ' % (AX12[1][i])])  # format Hexadecimal
        print('{:-<23}'.format(AX12_memory[i]), ">", mot1, "     ", mot2)
    print("####################################################")
    return


def Actualitza_AX_Memory(id_modul, adressa, nparametres, trama):
    if id_modul == ID_Broadcast:
        for index in range(nparametres):
            AX12[index_modulos[ID_L]][adressa + index] = trama[index + 6]
            AX12[index_modulos[ID_R]][adressa + index] = trama[index + 6]
        return
    else:
        for index in range(nparametres):
            AX12[index_modulos[id_modul]][adressa + index] = trama[index + 6]
        return


def print_trama():
    print(" ")
    print("####################################################")
    print("Received Instruction Frame:", trama)
    for index in range(len(trama)):
        print("Received Instruction Frame[", index, "]:", '0x%02X ' % trama[index])
    print("####################################################")
    return


def print_tipus_Instruccio(instruccio, comandament):
    error_de_instr = comprova_instr(instruccio)
    if DEBUG_Consola == 1:
        if error_de_instr == 0:
            print("----------- INSTRUCCIÓ i COMANDAMENT --------------")
            print("Command:", AX12_memory[comandament])
    return error_de_instr


# iNICIALITZACIÓ
# inicialitza els moduls a valors de reset
AX12 = [[0] * 50 for i in range(n_motors)]
reset_modul_AX12(ID_L)
reset_modul_AX12(ID_R)
# crea una trama d'status per indicar que no hi ha error
Status_NoError = [0xff, 0xff, 0x01, 0x02, 0x00, 0xfc]

# aquesta instruccio es no fer res
instruccio = INSTR_IDLE
# si esta activat el debug dels moduls presenta el contingut de memoria d'aquests
if DEBUG_Moduls:
    print_AX_MemoryMap()


# Funciones para gestionar la aplicacion grafica

class Hilo(threading.Thread):
    def __init__(self, nombre_hilo, cola, cola_hilo):
        threading.Thread.__init__(self)
        self.cola = cola
        self.cola_hilo = cola_hilo
        self.nombre_hilo = nombre_hilo

    def leer_puerto(self, la_cola, la_cola_hilo):
        global instruccio
        global trama
        global ID
        global AX12_moving_L
        global AX12_moving_R
        global lectura, leyendo
        global s_client

        AX12_moving_L = "PARAT"
        AX12_moving_R = "PARAT"
        print("Lectura puerto iniciada...", instruccio)
        while instruccio != INSTR_END:
            if not la_cola.empty():
                mensaje = la_cola.get()
                if mensaje == INSTR_END:
                    instruccio = mensaje
                    if DEBUG_Consola == 1:
                        print("instruccio rebuda: ", instruccio)
                if DEBUG_Consola == 1:
                    print("mensaje recibido en leer_puerto: ", mensaje)
            elif lectura == 1:
                trama = b''
                try:
                    leyendo = 1  # Avisamos al otro hilo que ocupamos el socket
                    # trama = s_client.recv(32)
                    inicio_trama = s_client.recv(4)  # si no hay datos, saltaremos al except por timeout
                    # si han llegado datos, el byte 3 indica cuantos mas hemos de esperar, los leemos
                    resto_trama = s_client.recv(inicio_trama[3])
                    trama = b''.join([inicio_trama, resto_trama])  # Juntamos todo en una sola trama
                except socket.error:
                    pass
                except IndexError:
                    pass
                leyendo = 0  # Avisamos al otro hilo que hemos liberado el socket
                if trama != b'':
                    items_array = len(trama)
                    if DEBUG_Consola == 1:
                        print("Número de items en el array:", items_array)
                    if items_array > 7:  # en cas contrari no es un instruction packet
                        if DEBUG_Consola == 1:
                            print("")
                            print("****************************************************")
                        instruccio = trama[4]  # posicio de la trama on esta la instruccio
                        # si esta activat el debug mostra la tama que arriba
                        if DEBUG_trama:
                            print_trama()
                        # mira quina instruccio es i si no es una de les que existeix dona un error
                        instr_error = print_tipus_Instruccio(instruccio, trama[5])
                        # copmprova el checksum rebut amb el calculat
                        chk_sum_error = comprova_checksum(trama)
                        # error indicara si hi ha un error, sigui d'instruccio o de checksum
                        error = (chk_sum_error | instr_error)
                        ID = trama[2]  # posicio a la trama del identificador edl modul
                        if ID != ID_Broadcast:  # si el ID no es el de broadcast respon amb un status packet
                            send_status_packet(ID, error)
                        else:
                            if DEBUG_Consola == 1:
                                print("Broadcasting ID Instruction Packet")
                        if ID not in lista_ID_Modulos:
                            print("Error: la ID ", format(ID, '#04x'), " no es vàlida")
                        elif error == 0:  # si no hi ha hagut cap error analitza la instruccio i l'executa
                            if instruccio == INSTR_WRITE:  # per ara nomes executa la instruccio WRITE
                                n_parametres = trama[3] - 3
                                address = trama[5]  # posicio de la tama on esta l'adreçá de memoria del modula escriure
                                Actualitza_AX_Memory(ID, address, n_parametres, trama)
                                AX12_func(address, ID)  # informa quin comandament s'ha executat
                                if DEBUG_Moduls:
                                    print_AX_MemoryMap()
                                if address == 0x20 or address == 0x21:  # Si es un comando para los motores
                                    robot2_status()

                time.sleep(0.01)  # 10ms para dejar tiempo de procesador al hilo principal

        la_cola_hilo.put(INSTR_STOP_THREAD)
        if DEBUG_Consola == 1:
            print("lectura parada")
            lectura = 0  # creo que esto no es necesario?

    def run(self):
        self.leer_puerto(self.cola, self.cola_hilo)
        if DEBUG_Consola == 1:
            print("funcion run terminada")


class Application(tk.Frame):
    def __init__(self, master=None):
        super().__init__(master)
        self.master = master
        self.grid()
        self.create_widgets()
        if ToolTip_presente:
            self.set_tooltips()
        self.crear_hilo()
        self.check_queue()
        self.seleccionar()

    def create_widgets(self):
        global ColorRGB
        self.spacer_up = Label(self, width=5, height=1)
        self.spacer_up.grid(row=0, column=0)
        self.spacer_center = Label(self, width=5, height=1)
        self.spacer_center.grid(row=20, column=0)
        self.spacer_bottom = Label(self, width=5, height=1)
        self.spacer_bottom.grid(row=50, column=10)

        self.hi_there = tk.Button(self)
        self.hi_there["text"] = "About"
        self.hi_there["fg"] = "blue"
        self.hi_there["command"] = self.say_hi
        self.hi_there.grid(row=21, column=2, sticky=E)

        self.quit = tk.Button(self, text="SORTIR", fg="red",
                              command=self.salir)
        self.quit.grid(row=21, column=2, sticky=W)

        self.Debug_frame = Checkbutton(self, variable=DEBUG_trama_check)
        self.Debug_frame["text"] = "Debug Trames"
        self.Debug_frame["fg"] = "blue"
        self.Debug_frame["command"] = self.set_debug_frames
        self.Debug_frame.grid(row=1, column=1)

        self.Debug_module = Checkbutton(self, variable=DEBUG_Moduls_check)
        self.Debug_module["text"] = "Debug Moduls"
        self.Debug_module["fg"] = "blue"
        self.Debug_module["command"] = self.set_debug_moduls
        self.Debug_module.grid(row=1, column=2)

        self.Debug_consola = Checkbutton(self, variable=DEBUG_Consola_check)
        self.Debug_consola["text"] = "Verbose"
        self.Debug_consola["fg"] = "blue"
        self.Debug_consola["command"] = self.set_debug_consola
        self.Debug_consola.grid(row=1, column=3)

        self.Print_Memoria_AX = Button(self, text="Imprimir Memoria Mòduls", fg="blue", command=self.imprimir_AX_memory)
        self.Print_Memoria_AX.grid(row=2, column=2, columnspan=2, sticky=W)
        # self.Print_IR = Button(self, text="Valors IR", fg="blue", command=self.imprimir_IR)
        # self.Print_IR.grid(row=2, column=3, sticky=W)

        self.label_AX12_L = Label(self, text="MOTOR Esq.:")
        self.label_AX12_L.grid(row=3, column=1)
        self.label_motor_left = Label(self, textvariable=texto_motor_left)
        texto_motor_left.set("PARAT")
        self.label_motor_left.grid(row=3, column=2)

        self.distancia_frame = LabelFrame(self, text="Ultrasons:")
        self.distancia_frame.grid(row=4, column=4, rowspan=3, sticky=W)
        self.label_ultrasonidos_label = Label(self.distancia_frame, text="Distància (cm)")
        self.label_ultrasonidos_label.grid(row=0, column=0)
        self.label_ultrasonidos_unidad = Label(self.distancia_frame, textvariable=string_distancia)
        self.label_ultrasonidos_unidad.grid(row=1, column=0)

        self.label_izq = Label(self, text="LDR Esq.")
        self.label_izq.grid(row=4, column=1)

        self.progress_bar_izq = ttk.Progressbar(self, orient="horizontal",
                                                length=255, maximum=4095,
                                                mode="determinate", variable=valor_barra_izq)
        # self.progress_bar_izq["value"] = 0
        self.progress_bar_izq.place(relx=0.5, rely=0.5, relwidth=0.80, anchor=tk.CENTER)
        self.progress_bar_izq.grid(row=4, column=2, columnspan=2)

        self.label_robot = Label(self, text="ESTAT ROBOT:")
        self.label_robot.grid(row=5, column=1)
        self.label_trama = Label(self, textvariable=texto_trama)
        texto_trama.set("ROBOT PARAT")
        self.label_trama.grid(row=5, column=2)

        self.label_der = Label(self, text="LDR dret")
        self.label_der.grid(row=6, column=1)

        self.progress_bar_der = ttk.Progressbar(self, orient="horizontal",
                                                length=255, maximum=4095,
                                                mode="determinate", variable=valor_barra_der)
        # self.progress_bar_der["value"] = 200
        self.progress_bar_der.place(relx=0.5, rely=0.5, relwidth=0.80, anchor=tk.CENTER)
        self.progress_bar_der.grid(row=6, column=2, columnspan=2)

        self.label_AX12_R = Label(self, text="MOTOR dret:")
        self.label_AX12_R.grid(row=7, column=1)
        self.label_motor_right = Label(self, textvariable=texto_motor_right)
        texto_motor_right.set("PARAT")
        self.label_motor_right.grid(row=7, column=2)

        self.Placa1 = tk.Radiobutton(self, text="LEDS RGB", variable=Placa, value="LEDS RGB", command=self.seleccionar)
        self.Placa1.grid(row=10, column=1, sticky=W)
        self.Placa1.select()
        self.cb_RGB = ttk.Combobox(self, width=4, state="readonly",
                                   values=ValoresRGB)  # desplegable con velocidades de bps estandares
        self.cb_RGB.current(3)
        self.cb_RGB.grid(row=10, column=2, sticky=E)
        self.cb_RGB.bind('<<ComboboxSelected>>', self.on_select_RGB)
        ColorRGB = self.cb_RGB.current()
        self.label_RGB = Label(self, text="Color RGB [0..7]")
        self.label_RGB.grid(row=10, column=3, sticky=SE)

        self.Placa2 = tk.Radiobutton(self, text="Motors", variable=Placa, value="Motores", command=self.seleccionar)
        self.Placa2.grid(row=11, column=1, sticky=W)
        self.AjusteSentido = Scale(self, from_=0, to=2, label="Sentit", variable=Sentido, orient=HORIZONTAL)
        self.AjusteSentido.grid(row=11, column=2)
        self.AjusteVelocidad = Scale(self, from_=0, to=255, label="Velocitat", variable=Velocidad, orient=HORIZONTAL)
        self.AjusteVelocidad.grid(row=11, column=3)
        self.STOP = tk.Button(self, text="STOP", background="RED", foreground="WHITE", command=self.parada_emergencia)
        self.STOP.grid(row=11, column=4, sticky=SW)

        self.LEFT = tk.Radiobutton(self, text="Left", variable=Identificador, value="Left", command=self.seleccionar)
        self.LEFT.grid(row=14, column=1, sticky=W)
        self.LEFT.select()
        self.RIGHT = tk.Radiobutton(self, text="Right", variable=Identificador, value="Right", command=self.seleccionar)
        self.RIGHT.grid(row=14, column=2, sticky=W)
        self.AMBOS = tk.Radiobutton(self, text="Tots dos", variable=Identificador, value="Ambos", command=self.seleccionar)
        self.AMBOS.grid(row=14, column=3, sticky=W)

        self.label_IP = Label(self, text="IP Maqueen:")
        self.label_IP.grid(row=15, column=1, sticky=W)

        self.IP1 = Entry(self, width=15, textvariable=IP_Text1)
        self.IP1.grid(row=15, column=2, sticky=W)
#        IP_Text1.trace("w", lambda name, index, mode, IP_Text1=IP_Text1: (self.ValidarIP, '%S'))
        self.BotonEnviar = tk.Button(self, text="Enviar", fg="Blue", state=DISABLED, command=self.EnviarDatos)
        # self.BotonEnviar.grid(row=15, column=3, sticky=W)
        self.BotonEnviar.grid(row=15, column=3, sticky=E, padx=100)
        self.BotonConectar = tk.Button(self, text=" Connectar", fg="Blue", command=self.conectar)
        # self.BotonConectar.grid(row=15, column=3, sticky=E, padx=50)
        self.BotonConectar.grid(row=15, column=3, sticky=W)

        self.logo_frame = Label(text=" ")
        self.logo_frame.grid(row=40, column=0, columnspan=4, pady=10)
        try:
            self.logo = PhotoImage(file="logoUB.png")
            self.logo_frame["image"] = self.logo
        except tk.TclError:
            self.logo_frame["text"] = "Universitat de Barcelona"
            return

    def set_tooltips(self):
        fondo = "lightyellow"  # El color de fondo de los tooltips
        retraso = 0.5  # El tiempo que tarda el tooltip en aparecer despues de poner el cursor encima del elemento
        ToolTip(self.Debug_frame,
                msg="Activa/Desactiva impressió a consola de la trama rebuda \ni de la resposta enviada (Status Packet)",
                delay=retraso,  # True by default
                bg=fondo)

        ToolTip(self.Debug_module,
                msg="Activa/Desactiva impressió a consola del mapa de memòria dels mòduls \nAX12 (motors) i AX-S1 (sensors) al rebre una comanda vàlida",
                delay=retraso,  # True by default
                bg=fondo)

        ToolTip(self.Debug_consola,
                msg="Activa/Desactiva impressió a consola de \nmissatges addicional dels estats del robot",
                delay=retraso,  # True by default
                bg=fondo)

        ToolTip(self.Print_Memoria_AX,
                msg="Imprimir a la consola el mapa de memòria \ndels mòduls AX12 (motors) i AX-S1 (sensors)",
                delay=retraso,  # True by default
                bg=fondo)

        ToolTip(self.Placa1,
                msg="Seleccionem que volem enviar una comanda als Leds RGB del Maqueen",
                delay=retraso,  # True by default
                bg=fondo)

        ToolTip(self.cb_RGB,
                msg="Seleccionem el color per enviar als Leds RGB del Maqueen",
                delay=retraso,  # True by default
                bg=fondo)

        ToolTip(self.Placa2,
                msg="Seleccionem que volem enviar una comanda als motors del Maqueen",
                delay=retraso,  # True by default
                bg=fondo)

        ToolTip(self.AjusteSentido,
                msg="Seleccionem el sentit de gir:\n 0: parats, 1: endavant, 2: endarrera",
                delay=retraso,  # True by default
                bg=fondo)

        ToolTip(self.AjusteVelocidad,
                msg="Seleccionem la velocitat de gir:\n [0..255]",
                delay=retraso,  # True by default
                bg=fondo)

        ToolTip(self.STOP,
                msg="Aturada d'emergència: enviem velocitat i sentit de gir nuls a tots dos motors",
                delay=retraso,  # True by default
                bg=fondo)

        ToolTip(self.LEFT,
                msg="Seleccionem que s'enviarà la comanda al Led o Motor Esquerra",
                delay=retraso,  # True by default
                bg=fondo)

        ToolTip(self.RIGHT,
                msg="Seleccionem que s'enviarà la comanda al Led o Motor Dreta",
                delay=retraso,  # True by default
                bg=fondo)

        ToolTip(self.AMBOS,
                msg="Seleccionem que s'enviarà la comanda als Leds o Motors dels 2 costats",
                delay=retraso,  # True by default
                bg=fondo)

        ToolTip(self.IP1,
                msg="Aquí indiquem l'adreça IP del Maqueen, per poder-nos hi connectar\namb el botó 'Connectar",
                delay=retraso,  # True by default
                bg=fondo)

        ToolTip(self.BotonConectar,
                msg="Per connectar-nos al Maqueen, a l'adreça IP que haurem indicat en el camp 'IP Maqueen",
                delay=retraso,  # True by default
                bg=fondo)

        ToolTip(self.BotonEnviar,
                msg="Per enviar la comanda escollida (Leds RGB o Motors) al Maqueen",
                delay=retraso,  # True by default
                bg=fondo)

        ToolTip(self.quit,
                msg="Surt de l’emulador, tancant totes les finestres, i terminant tots els fils i processos. \n(Nota: la creueta de la part superior dreta de la finestra no surt de forma neta)",
                delay=retraso,  # True by default
                bg=fondo)

        # self.tooltip_hi_there = Pmw.Balloon(root)  # Calling the tooltip
        # el_tip = "Informació dels autors"
        # self.tooltip_hi_there.bind(self.hi_there, el_tip)  # binding it and assigning a text to it
        ToolTip(self.hi_there,
                msg="Informació dels autors",
                delay=retraso,  # True by default
                bg=fondo)

    def seleccionar(self):
        global ColorRGB
        comando = []
        datos = []
        if Placa.get() == "LEDS RGB":
            print("Seleccionado: ", Placa.get(), Identificador.get(), ColoresRGB[ColorRGB])
            comando = [0xFF, 0xFF, identificadores[Identificador.get()], 4, INSTR_WRITE,
                       comandos[Placa.get()], colores[ColoresRGB[ColorRGB]]]
            chk = checksum(comando)
            comando.append(chk)
            datos = bytes(comando)
        elif Placa.get() == "Motores":
            print("Seleccionado: ", Placa.get(), Identificador.get(), Velocidad.get(), EstadosMotores[Sentido.get()])
            sentido = Sentido.get()
            comando = [0xFF, 0xFF, identificadores[Identificador.get()], 5, INSTR_WRITE,
                       comandos[Placa.get()], Velocidad.get(), sentido]
            chk = checksum(comando)
            comando.append(chk)
            print(comando)
            datos = bytes(comando)
        elif Placa.get() == "Stop":
            print("Seleccionado: STOP")
            comando = [0xFF, 0xFF, identificadores["Ambos"], 5, INSTR_WRITE,
                                  comandos["Motores"], 0, 0]
            chk = checksum(comando)
            comando.append(chk)
            datos = bytes(comando)
            print("Parada de emergencia")
        print(comando, datos)
        return datos

    def parada_emergencia(self):
        global conectado, s_client
        comando = []
        datos = []
        Placa.set("Stop")
        # comando = [0xFF, 0xFF, identificadores["Ambos"], 5, INSTR_WRITE,
        #            comandos["Motores"], 0, 0]
        # chk = checksum(comando)
        # comando.append(chk)
        # datos = bytes(comando)
        # print("Parada de emergencia")
        # print(comando, datos)
        datos = self.seleccionar()
        if conectado:
            s_client.send(datos)
        return datos

    def ValidarIP(self, tecla):
        global HOST
        HOST = IP_Text1.get()
        if tecla == 0x0A or tecla == 0x0D:
            print("Nueva IP", HOST)

    def conectar(self):
        global conectado, lectura, HOST
        global s_client
        HOST = IP_Text1.get()
        if not conectado:
            try:
                print("Conectando a ", HOST, ":", PORT, "...")
                s_client.connect((HOST, PORT))  # blocking hasta que conecte con exito
                s_client.settimeout(0.5)  # una vez conectado,unblocking con timeout de 500ms
                conectado = 1
                time.sleep(0.1)
                self.BotonEnviar["state"] = "normal"
                self.BotonConectar["text"] = "Desconnectar"
                lectura = 1
                print("Conectado a ", HOST, ":", PORT)
                return
            except Exception as e:
                messagebox.showerror(title="Error connexió", message=e.__str__())
        # Si llegamos aquí, es que o bien queremos desconectar, o bien ha fallado el intento anterior de conexion.
        # En ambos casos, hay que reiniciar algunas variables, incluido el socket:
        conectado = 0
        lectura = 0
        s_client.close()  # se cierra y destruye el socket
        self.BotonEnviar["state"] = "disabled"
        self.BotonConectar["text"] = "Connectar"
        messagebox.showinfo("Conexió", "Desconnectat del servidor")
        # se vuelve a crear el socket, para poder reconectar:
        s_client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def EnviarDatos(self):
        global s_client, lectura, leyendo, trama
        lectura = 0  # Notificamos al hilo de lectura que pare de leer
        trama = b''
        dada = self.seleccionar()
        print("Enviando ", dada, "a: ", self.IP1.get())
        while leyendo == 1:  # Esperamos hasta que el otro hilo libere el socket
            continue
        s_client.send(dada)
        try:
            # trama = s_client.recv(32)
            inicio_trama = s_client.recv(4)  # si no hay datos, saltaremos al except por timeout
            # si han llegado datos, el byte 3 indica cuantos mas hemos de esperar, los leemos
            resto_trama = s_client.recv(inicio_trama[3])
            trama = b''.join([inicio_trama, resto_trama])  # Juntamos todo en una sola trama
            print('Recibido: ', trama)
        except socket.error:
            print('Ningun dato recibido.')
            pass
        # Faltaria actualizar el estado de los motores (memoria AX12 y Labels)
        if dada[4] == INSTR_WRITE:  # per ara nomes executa la instruccio WRITE
            n_parametres = dada[3] - 3
            address = dada[5]  # posicio de la trama on esta l'adreçá de memoria del modula escriure
            id_modulo = dada[2]
            Actualitza_AX_Memory(id_modulo, address, n_parametres, dada)
            robot2_status()
        lectura = 1  # Notificamos al hilo de lectura que puede volver a leer

    def on_select_RGB(self, event=None):
        global ColorRGB
        ColorRGB = self.cb_RGB.current()
        self.seleccionar()

    def set_debug_frames(self):
        global DEBUG_trama
        DEBUG_trama = DEBUG_trama_check.get()

    def set_debug_moduls(self):
        global DEBUG_Moduls
        DEBUG_Moduls = DEBUG_Moduls_check.get()

    def set_debug_consola(self):
        global DEBUG_Consola
        DEBUG_Consola = DEBUG_Consola_check.get()

    def imprimir_AX_memory(self):
        print_AX_MemoryMap()

    def check_queue(self):
        global instruccio
        # revisar la cola para evitar bloqueo en la interfaz
        if not self.cola_hilo.empty():
            # obtener mensaje de la cola
            instruccio = self.cola_hilo.get()
            print("get text from queue:", instruccio)
        if instruccio == INSTR_STOP_THREAD:
            return INSTR_STOP_THREAD
        root.after(200, self.check_queue)

    def say_hi(self):
        print("J. Bosch & C. Serre,")
        print("UB, 2020-2025.")
        print("versió", version)
        mensaje = "J. Bosch & C. Serre\nUB, 2020-2025.\nVer. " + version
        messagebox.showinfo("Autors", mensaje)

    def crear_hilo(self):
        # crear cola para comunicar/enviar tareas al thread
        self.cola = queue.Queue()
        self.cola_hilo = queue.Queue()
        if DEBUG_Consola:
            print("Creando colas:")
            print(self.cola, self.cola_hilo)
        # crear el thread
        print("Creando hilo puerto")
        hilo = Hilo("puerto", self.cola, self.cola_hilo)
        print("Iniciando Hilo puerto")
        # iniciar thread
        hilo.start()

    def salir(self):
        global conectado, lectura
        global s_client
        if conectado:
            s_client.close()  # se cierra y destruye el socket
        conectado = 0
        lectura = 0
        self.cola.put(INSTR_END)  # terminar funcion leer_puerto
        if instruccio == INSTR_STOP_THREAD:
            print("Hilo finalizado")
            self.master.destroy()
        else:
            root.after(200, self.salir)


# root = tk.Tk()
Titulo = "Maqueen, v." + version
root.title(Titulo)
app = Application(master=root)

app.mainloop()
print("Aplicacion terminada.")
