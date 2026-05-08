import socket
import threading

ESP32_IP = "192.168.43.167"   
ESP32_PORT = 5000

def recibir_datos(sock: socket.socket) -> None:
    buffer = ""

    while True:
        try:
            data = sock.recv(1024)
            if not data:
                print("Conexión cerrada por la ESP32")
                break

            buffer += data.decode("utf-8", errors="ignore")

            while "\n" in buffer:
                linea, buffer = buffer.split("\n", 1)
                linea = linea.strip()

                if not linea:
                    continue

                # Mostrar la trama completa
                print(f"Recibido: {linea}")

                # Intentar parsear ax;ay;az
                partes = linea.split(";")
                if len(partes) == 3:
                    try:
                        ax = float(partes[0])
                        ay = float(partes[1])
                        az = float(partes[2])
                        print(f"  ax={ax:.2f}, ay={ay:.2f}, az={az:.2f}")
                    except ValueError:
                        print("  Aviso: trama recibida pero no convertible a float")

        except ConnectionResetError:
            print("La conexión se ha reiniciado")
            break
        except OSError as e:
            print(f"Error de socket: {e}")
            break


def main() -> None:
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.connect((ESP32_IP, ESP32_PORT))
            print(f"Conectado a la ESP32 en {ESP32_IP}:{ESP32_PORT}")

            # Hilo para recibir datos continuamente
            hilo_rx = threading.Thread(target=recibir_datos, args=(sock,), daemon=True)
            hilo_rx.start()

            print("Escribe comandos para enviar a la ESP32: start, stop o texto libre")
            print("Escribe 'exit' para salir")

            while True:
                mensaje = input(">> ").strip()

                if mensaje.lower() == "exit":
                    print("Cerrando conexión...")
                    break

                # La ESP32 usa readStringUntil('\\n'), así que hay que enviar salto de línea
                sock.sendall((mensaje + "\n").encode("utf-8"))

    except ConnectionRefusedError:
        print("No se pudo conectar. Revisa la IP, el puerto y que la ESP32 esté esperando clientes.")
    except TimeoutError:
        print("Timeout al intentar conectar con la ESP32.")
    except OSError as e:
        print(f"Error de red: {e}")


if __name__ == "__main__":
    main()