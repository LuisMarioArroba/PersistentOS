#!/usr/bin/env python3
"""
capture_report.py

Reemplaza el paso "pio device monitor -e persistentos1_reporte | tee run.log"
del flujo de reporte. Abre el puerto serie directamente (sin pasar por
pio device monitor), y:

  1. Al conectar, envía 'r' al ESP32 para activar el ciclo automático
     de escenarios de main_prueba_reporte.cpp (a menos que uses --no-auto).
  2. Va escribiendo cada línea recibida a run.log (y a la pantalla) a
     medida que llega -- el archivo se actualiza en vivo, sin quedar
     "bloqueado" esperando a que cierres una sesión interactiva.
  3. Sigue leyendo lo que escribas en esta misma terminal y lo reenvía
     al ESP32, por si querés tagear a mano un escenario 2/3 con 'b'/'c',
     o apagar el modo automático mandando 'r' de nuevo.

Requiere: pip install pyserial

Uso:
    python3 tools/capture_report.py <puerto> [--baud 115200] [--out run.log] [--no-auto]

Ejemplos:
    python3 tools/capture_report.py COM5
    python3 tools/capture_report.py /dev/ttyUSB0 --out run.log
    python3 tools/capture_report.py /dev/ttyUSB0 --no-auto

Luego, en otra terminal (o al terminar con Ctrl+C):
    python3 tools/plot_report.py run.log
"""

import sys
import argparse
import threading
import time

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("Falta pyserial. Instalá con: pip install pyserial")
    sys.exit(1)


def list_ports():
    ports = list(serial.tools.list_ports.comports())

    if not ports:
        print("No se encontró ningún puerto serie conectado.")
        return

    print("Puertos disponibles:")

    for p in ports:
        print(f"  {p.device}  -  {p.description}")


def reader_loop(ser, out_file, stop_event):
    """Lee del puerto serie y escribe a la vez a pantalla y a out_file."""

    while not stop_event.is_set():

        try:
            raw = ser.readline()
        except serial.SerialException:
            break

        if not raw:
            continue

        try:
            line = raw.decode("utf-8", errors="replace").rstrip("\r\n")
        except Exception:
            continue

        print(line)

        out_file.write(line + "\n")
        out_file.flush()


def writer_loop(ser, stop_event):
    """Reenvía al ESP32 lo que el usuario escriba en esta terminal."""

    while not stop_event.is_set():

        try:
            typed = sys.stdin.readline()
        except Exception:
            break

        if not typed:
            continue

        for ch in typed.rstrip("\n"):
            try:
                ser.write(ch.encode("utf-8"))
            except serial.SerialException:
                return


def main():

    parser = argparse.ArgumentParser(
        description="Captura serial para main_prueba_reporte.cpp, "
                     "con activación automática del ciclo de escenarios."
    )

    parser.add_argument(
        "port",
        nargs="?",
        default=None,
        help="Puerto serie, p.ej. COM5 o /dev/ttyUSB0"
    )

    parser.add_argument(
        "--list",
        action="store_true",
        help="Listar los puertos serie disponibles y salir."
    )

    parser.add_argument(
        "--baud",
        type=int,
        default=115200
    )

    parser.add_argument(
        "--out",
        default="run.log"
    )

    parser.add_argument(
        "--no-auto",
        action="store_true",
        help="No enviar 'r' al conectar (por defecto sí lo envía, "
             "activando el ciclo automático de escenarios)."
    )

    args = parser.parse_args()

    if args.list:
        list_ports()
        sys.exit(0)

    if not args.port:
        parser.error("falta el puerto (o usá --list para ver los disponibles)")

    try:
        ser = serial.Serial(
            args.port,
            args.baud,
            timeout=1
        )
    except serial.SerialException as e:
        print(f"[capture_report] No se pudo abrir {args.port}: {e}")
        print(
            "[capture_report] En Windows, 'Acceso denegado' casi "
            "siempre significa que otro programa tiene el puerto "
            "abierto -- cerrá cualquier Monitor Serial de PlatformIO, "
            "VSCode o Arduino IDE, y probá de nuevo."
        )
        print(
            "[capture_report] Corré 'python tools/capture_report.py "
            "--list' para ver los puertos disponibles."
        )
        sys.exit(1)

    # --------------------------------------------------
    # En muchas placas, abrir el puerto por USB reinicia
    # el ESP32 (DTR/RTS). Se da un momento a que vuelva a
    # arrancar antes de mandarle nada.
    # --------------------------------------------------

    time.sleep(2)

    if not args.no_auto:
        ser.write(b"r")
        print(
            "[capture_report] Enviado 'r' -- ciclo automático de "
            "escenarios activado."
        )

    stop_event = threading.Event()

    out_file = open(
        args.out,
        "a",
        buffering=1
    )

    print(
        f"[capture_report] Capturando {args.port} -> {args.out} "
        "(Ctrl+C para salir)"
    )

    print(
        "[capture_report] Podés seguir escribiendo teclas acá "
        "(p.ej. b, c, h, r + Enter) y se mandan al ESP32."
    )

    t_read = threading.Thread(
        target=reader_loop,
        args=(ser, out_file, stop_event),
        daemon=True
    )

    t_write = threading.Thread(
        target=writer_loop,
        args=(ser, stop_event),
        daemon=True
    )

    t_read.start()
    t_write.start()

    try:
        while t_read.is_alive():
            time.sleep(0.2)
    except KeyboardInterrupt:
        pass
    finally:
        stop_event.set()
        ser.close()
        out_file.close()
        print(f"\n[capture_report] Listo, log guardado en {args.out}")


if __name__ == "__main__":
    main()
