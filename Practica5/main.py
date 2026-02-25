import serial
from datetime import datetime
from collections import deque       # Para usar esta estructura de datos de python, que sirve como un buffer circular (similar funcionamiento a una FIFO)

import numpy as np              # Calculos rapidos (media, desviación estándar, convertir a arrays.)
import matplotlib.pyplot as plt # Crear y mostrar gráficos
from matplotlib.animation import FuncAnimation          # Animacion en tiempo real

# ---------------------------
# Configuración
# ---------------------------

PORT = 'COM15'          # Seleccionar puerto y velocidad.
BAUD = 115200

WINDOW_SECONDS = 5
SAMPLE_RATE_HZ = 10             # Tiempo de muestreo esperado
WINDOW_SIZE = WINDOW_SECONDS * SAMPLE_RATE_HZ  # 50  (tamaño de ventana en muestras)

# ---------------------------
# Crear fichero CSV con timestamp
# ---------------------------

filename = "datos_IMU_" + datetime.now().strftime("%Y%m%d_%H%M%S") + ".csv"     # Generacion nombre unico

with open(filename, "w", encoding="utf-8") as f:            #Abrimos en modo escritura
    f.write("timestamp_pc;Acel_x;Acel_y;Acel_z;Gyro_x;Gyro_y;Gyro_z;Magne_x;Magne_y;Magne_z\n")     #Creamos la cabecera

print("Fichero CSV creado correctamente:", filename)

# ---------------------------
# Abrir puerto serie
# ---------------------------

ser = serial.Serial(PORT, BAUD, timeout=1)      #Abrimos puerto serie
print("Conectado al puerto serie")

try:
    ser.reset_input_buffer()        #Limpiamos el buffer de entrada del puerto al comenzar, por si hay datos antiguos
except AttributeError:
    ser.flushInput()            #otra funcion usada por si acaso no funciona la anterior

# ---------------------------
# Buffers (ventana 5s) para Acel x,y,z
# ---------------------------

buffer_ax = deque(maxlen=WINDOW_SIZE)       #buffers para guardar las ultimas 50 mestras de cada eje, en modo "FIFO"
buffer_ay = deque(maxlen=WINDOW_SIZE)
buffer_az = deque(maxlen=WINDOW_SIZE)

# Series para plot (media y std por eje)
mean_ax, std_ax = [], []            # Listas para las metricas estadisticas
mean_ay, std_ay = [], []
mean_az, std_az = [], []

# ---------------------------
# Matplotlib: figura (2 gráficos)
# ---------------------------

fig, (ax_mean, ax_std) = plt.subplots(2, 1, sharex=True)        #creamos una figura con dos gráficos verticales. Comparten el eje x

# Medias
line_mean_ax, = ax_mean.plot([], [], label="Media Ax")      #Configuramos las tres lineas del gráfico (al principio vacias)
line_mean_ay, = ax_mean.plot([], [], label="Media Ay")
line_mean_az, = ax_mean.plot([], [], label="Media Az")

ax_mean.set_title("Media - Ventana 5s (Aceleración)")       #Configuramos primer gráfico
ax_mean.set_ylabel("Valor (g)")
ax_mean.set_xlim(0, 50)
ax_mean.set_ylim(-2, 2)
ax_mean.legend()

# Desviaciones estándar
line_std_ax, = ax_std.plot([], [], label="Std Ax")
line_std_ay, = ax_std.plot([], [], label="Std Ay")
line_std_az, = ax_std.plot([], [], label="Std Az")

ax_std.set_title("Desviación estándar - Ventana 5s (Aceleración)")      #Configuramos segundo gráfico
ax_std.set_xlabel("Iteración")
ax_std.set_ylabel("Valor (g)")
ax_std.set_xlim(0, 50)
ax_std.set_ylim(0, 1)
ax_std.legend()

# ---------------------------
# Función update para FuncAnimation
# ---------------------------
#Esta función la llamará Matplotlib repetidamente
def update(_frame):
    # 1) Leer todo lo que haya en el buffer serie
    while ser.in_waiting > 0:   # Esperando datos en el buffer
        line = ser.readline().decode('utf-8', errors='ignore').strip()  #leemos el puerto y dejamos bonito

        if not line:
            continue

        # Ignoramos todo tipo de letras , solo numeros
        if any(c.isalpha() for c in line):
            continue

        # Guardar en CSV con timestamp del PC
        ts = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        out_line = ts + ";" + line
        with open(filename, "a", encoding="utf-8") as f:
            f.write(out_line + "\n")

        # Parsear Ax, Ay, Az (formato: Ax;Ay;Az;Gyro_x;...)
        parts = line.split(";")
        if len(parts) >= 3:
            try:        #Convierte los 3 primeros campos a float.
                ax_value = float(parts[0])
                ay_value = float(parts[1])
                az_value = float(parts[2])

                buffer_ax.append(ax_value)  # Metemos cada muestra en el buffer circular de 50
                buffer_ay.append(ay_value)
                buffer_az.append(az_value)

            except ValueError:
                pass

    # 2) Si ventana llena (5s), calcular estadísticas y actualizar curvas
    if len(buffer_ax) == WINDOW_SIZE and len(buffer_ay) == WINDOW_SIZE and len(buffer_az) == WINDOW_SIZE:

        data_ax = np.array(buffer_ax, dtype=float)  # Convertimos las "FIFO" en arrays para un mejor calculo
        data_ay = np.array(buffer_ay, dtype=float)
        data_az = np.array(buffer_az, dtype=float)

        mean_ax.append(float(np.mean(data_ax))) #calculamos
        std_ax.append(float(np.std(data_ax)))

        mean_ay.append(float(np.mean(data_ay)))
        std_ay.append(float(np.std(data_ay)))

        mean_az.append(float(np.mean(data_az)))
        std_az.append(float(np.std(data_az)))

        # Mantener 50 puntos en pantalla eliminando el mas antiguo si hay mas
        for series in (mean_ax, mean_ay, mean_az, std_ax, std_ay, std_az):
            if len(series) > 50:
                series.pop(0)

        x = np.arange(len(mean_ax))     #creamos eje X

        # --- actualizar medias ---
        line_mean_ax.set_data(x, mean_ax)
        line_mean_ay.set_data(x, mean_ay)
        line_mean_az.set_data(x, mean_az)

        # --- actualizar std ---
        line_std_ax.set_data(x, std_ax)
        line_std_ay.set_data(x, std_ay)
        line_std_az.set_data(x, std_az)

        # Eje X común ajustamos limites
        ax_std.set_xlim(0, max(50, len(mean_ax)))

        # Autoescala Y medias
        all_mean = mean_ax + mean_ay + mean_az
        y_min_mean = min(all_mean) - 0.2        # con margen de 0.2
        y_max_mean = max(all_mean) + 0.2
        ax_mean.set_ylim(y_min_mean, y_max_mean)

        # Autoescala Y std (siempre >=0)
        all_std = std_ax + std_ay + std_az
        y_min_std = 0.0
        y_max_std = max(all_std) + 0.05
        ax_std.set_ylim(y_min_std, y_max_std)  #ajustamos limites

    return (line_mean_ax, line_mean_ay, line_mean_az,       #Devuelve las líneas que se han actualizado para que FuncAnimation sepa qué redibujar.
            line_std_ax, line_std_ay, line_std_az)

# ---------------------------
# Animación
# ---------------------------

ani = FuncAnimation(fig, update, interval=200, cache_frame_data=False, blit=False)  #crear animacion, 200 mseg

plt.tight_layout()      #ajusta margenes
plt.show(block=True)    #Muestra la ventana y bloquea el script hasta que se cierre

ser.close()             #cerramos puerto al finalizar