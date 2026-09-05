# PersistentOS
Sistema operativo de sistemas embebidos para dispositivos sin bateria

## UI Simulator

La siguiente imagen muestra el diseño conceptual de la interfaz de usuario (UI) del sistema. Este simulador representa la distribución de las principales pantallas y módulos que conforman el dispositivo. Su propósito es servir como guía visual durante el desarrollo e integración de la interfaz en el hardware.

Las secciones representadas corresponden a los principales componentes funcionales del sistema:

- **Scenery:** Visualización del entorno y estado general del dispositivo.
- **Energy:** Monitoreo y administración del estado energético.
- **Scheduler:** Gestión y seguimiento de tareas persistentes.
- **Communication:** Estado de las interfaces y protocolos de comunicación.

> **Nota:** Esta interfaz es un prototipo visual. El diseño y las funcionalidades podrán evolucionar conforme avance el desarrollo del sistema operativo.

<img width="1200" height="1600" alt="prototype_breadboard" src="https://github.com/user-attachments/assets/fa9cce5a-0c84-4ced-8124-8bfa2b481323" />

## Arquitectura

PersistentOS es un micro-núcleo cooperativo (checkpoints en FRAM, arranque en frío vs. recuperación) sobre el que corren servicios reanudables: sensado, comunicación (Bluetooth clásico, protocolo `DATA`/`ACK`), alarmas y energía. **Los tres nodos comparten el mismo firmware** (`src/main.cpp`); lo único que cambia entre ellos es el `-DPERSISTENT_OS_NODE_ID` con el que se compilan, que en `include/Config/Config.h` determina el rol de conexión Bluetooth (maestro/esclavo) y con qué otro nodo se enlaza cada uno.

| Nodo | Rol | Primary (obligatorio) | Secondary (opcional) |
|---|---|---|---|
| 1 | Maestro | 2 | 3 |
| 2 | Esclavo | 1 | 3 |
| 3 | Maestro | 2 | 1 |

El nodo 3 es opcional para que el sistema funcione (los nodos 1 y 2 se comunican igual sin él), pero cada nodo lo busca periódicamente por si aparece.

## Compilar y cargar los dispositivos

Requiere [PlatformIO](https://platformio.org/) (CLI o la extensión de VSCode). Cada nodo se sube por separado, conectado por USB, con su propio entorno:

```bash
# Nodo 1 (maestro, primary = Nodo 2)
pio run -e persistentos1 -t upload

# Nodo 2 (esclavo, primary = Nodo 1)
pio run -e persistentos2 -t upload

# Nodo 3 (maestro, primary = Nodo 2) -- opcional
pio run -e persistentos3 -t upload
```

Si tenés varias placas conectadas a la vez y PlatformIO no elige el puerto correcto solo, indicalo explícitamente:

```bash
pio run -e persistentos1 -t upload --upload-port COM5        # Windows
pio run -e persistentos1 -t upload --upload-port /dev/ttyUSB0 # Linux/Mac
```

Para ver los logs de cualquier nodo ya cargado:

```bash
pio device monitor -e persistentos1
```

### Build de reporte (para las gráficas del paper)

Reemplaza `main.cpp` por `src/report/main_prueba_reporte.cpp` sobre el mismo kernel -- agrega tageo de escenario y trazas por Serial. Por defecto compila como Nodo 1; para capturar del lado de otro nodo, cambiá el `-DPERSISTENT_OS_NODE_ID` en `platformio.ini` (sección `[env:persistentos_reporte]`) antes de subir.

```bash
pio run -e persistentos_reporte -t upload

# Captura con activación automática del ciclo de escenarios
python3 tools/capture_report.py <puerto>   # p.ej. COM5 o /dev/ttyUSB0

# Con la captura en run.log, generar las gráficas
python3 tools/plot_report.py run.log
```

`src/node2/main_node2.cpp` no se compila en ningún entorno: es el firmware mínimo original, mantenido solo como referencia histórica.
