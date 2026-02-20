import serial

ser = serial.Serial('COM15', 115200, timeout=1) # EL puerto COM que sea

print("Conectado al puerto serie")

try:
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            print(line)

except KeyboardInterrupt:
    print("\nCerrando puerto...")
    ser.close()