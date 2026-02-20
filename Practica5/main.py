import serial
from datetime import datetime

# -------------------------------
# Crear fichero CSV con timestamp
# -------------------------------

filename = "datos_IMU_" + datetime.now().strftime("%Y%m%d_%H%M%S") + ".csv"

# Creamos el archivo y escribimos cabecera
with open(filename, "w", encoding="utf-8") as f:
    f.write("timestamp_pc;Acel_x;Acel_y;Acel_z;Gyro_x;Gyro_y;Gyro_z;Magne_x;Magne_y;Magne_z\n")

print("Fichero CSV creado correctamente:", filename)

# -------------------------------
# Abrir puerto serie
# -------------------------------

ser = serial.Serial('COM15', 115200, timeout=1)
print("Conectado al puerto serie")

try:
    with open(filename, "a", encoding="utf-8") as f:
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()

                if line:
                    # Timestamp del PC (con milisegundos)
                    ts = datetime.now().strftime("%H:%M:%S.%f")[:-3]        #solo la hora para no ensuciar demasiado los datos

                    # Guardar en CSV: timestamp + línea original
                    out_line = ts + ";" + line

                    print(out_line)
                    f.write(out_line + "\n")
                    f.flush()

except KeyboardInterrupt:
    print("\nCerrando puerto...")
    ser.close()