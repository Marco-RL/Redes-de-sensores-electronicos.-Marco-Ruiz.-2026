import serial
from datetime import datetime


# -------------------------------
#Crear fichero de prueba
# -------------------------------


filename = "datos_IMU_" + datetime.now().strftime("%Y%m%d_%H%M%S") + ".txt"     # nombre basado en la fecha/hora actual del PC.


#abre el fichero en modo escritura, no si exixte lo crea.
with open(filename, "w", encoding="utf-8") as f:    #abre el archivo y genera el handle f
    f.write("Fichero generado correctamente\n")     #comprobar que el archivo se crea.
    f.write("Acel_x;Acel_y;Acel_z;Gyro_x;Gyro_y;Gyro_z;Magne_x;Magne_y;Magne_z\n")

print("Fichero creado correctamente")

# -------------------------------
#Abrir puerto serie
# -------------------------------

ser = serial.Serial('COM15', 115200, timeout=1)     #abrimos el puerto serie

print("Conectado al puerto serie")

try:
    with open(filename, "a", encoding="utf-8") as f:  # abrimos el fichero en modo append con la "a"
        while True:
            if ser.in_waiting > 0:      #comprobar que hay bytes que han llegado
                line = ser.readline().decode('utf-8', errors='ignore').strip()  #leemos una linea del puerto serie, lee bytes hasta que encuentra \n y guardamos en line. con strp quitamos espacios

                if line:  # evitar líneas vacías
                    print(line)
                    f.write(line + "\n")  # añade al fichero en una linea nueva
                    f.flush()  # asegurar escritura inmediata. forzandolo a escribirlo en ese momento

except KeyboardInterrupt:
    print("\nCerrando puerto...")
    ser.close()