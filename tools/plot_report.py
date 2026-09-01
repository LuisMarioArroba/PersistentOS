#!/usr/bin/env python3
"""
plot_report.py

Lee un log de captura serial de main_prueba_reporte.cpp
(líneas "RPT,millis,scenario_id,scenario_name,event,packetId,value")
y genera:

  - una gráfica de energía vs. tiempo por escenario
    (out/energy_<scenario>.png)
  - una gráfica de reintentos por packetID por escenario
    (out/retries_<scenario>.png)
  - un resumen en texto: latencia de recovery, retries totales
    y tasa de confirmación por escenario (out/summary.csv)

Uso:
    pio device monitor -e persistentos1_reporte | tee run.log
    python3 tools/plot_report.py run.log
"""

import sys
import csv
import os
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def parse(path):
    rows = []
    with open(path, "r", errors="ignore") as f:
        for line in f:
            line = line.strip()
            if not line.startswith("RPT,"):
                continue
            parts = line.split(",")
            if len(parts) != 7:
                continue
            _, t, sid, sname, event, pid, value = parts
            try:
                rows.append({
                    "t": int(t),
                    "scenario_id": int(sid),
                    "scenario": sname,
                    "event": event,
                    "packetId": int(pid),
                    "value": int(value),
                })
            except ValueError:
                continue
    return rows


def by_scenario(rows):
    out = defaultdict(list)
    for r in rows:
        out[r["scenario"]].append(r)
    return out


def plot_energy(scenario, rows, outdir):
    pts = [(r["t"], r["value"] / 100.0) for r in rows if r["event"] == "ENERGY"]
    if not pts:
        return
    pts.sort()
    xs = [p[0] / 1000.0 for p in pts]
    ys = [p[1] for p in pts]

    plt.figure()
    plt.plot(xs, ys, marker="o", markersize=2)
    plt.xlabel("Tiempo (s)")
    plt.ylabel("Energía (%)")
    plt.title(f"Energía vs. tiempo - {scenario}")
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(os.path.join(outdir, f"energy_{scenario}.png"))
    plt.close()


def plot_retries(scenario, rows, outdir):
    counts = defaultdict(int)
    for r in rows:
        if r["event"] == "RETRY":
            counts[r["packetId"]] += 1

    if not counts:
        return

    ids = sorted(counts.keys())
    vals = [counts[i] for i in ids]

    plt.figure()
    plt.bar([str(i) for i in ids], vals)
    plt.xlabel("packetID")
    plt.ylabel("Reintentos")
    plt.title(f"Reintentos por paquete - {scenario}")
    plt.tight_layout()
    plt.savefig(os.path.join(outdir, f"retries_{scenario}.png"))
    plt.close()


def summarize(scenario, rows):
    latencies = [r["value"] for r in rows if r["event"] == "RECOVERY_LATENCY_MS"]
    retries = sum(1 for r in rows if r["event"] == "RETRY")
    confirms = sum(1 for r in rows if r["event"] == "CONFIRM")
    creates = sum(1 for r in rows if r["event"] == "CREATE")

    success_rate = (confirms / creates) if creates else float("nan")

    avg_latency = (sum(latencies) / len(latencies)) if latencies else ""

    return {
        "scenario": scenario,
        "creates": creates,
        "confirms": confirms,
        "success_rate": f"{success_rate:.2f}" if creates else "",
        "retries_total": retries,
        "recovery_events": len(latencies),
        "avg_recovery_latency_ms": avg_latency,
    }


def main():
    if len(sys.argv) != 2:
        print("Uso: python3 plot_report.py <run.log>")
        sys.exit(1)

    path = sys.argv[1]
    outdir = "out"
    os.makedirs(outdir, exist_ok=True)

    rows = parse(path)
    if not rows:
        print("No se encontraron líneas RPT en el log.")
        sys.exit(1)

    grouped = by_scenario(rows)
    summaries = []

    for scenario, srows in grouped.items():
        plot_energy(scenario, srows, outdir)
        plot_retries(scenario, srows, outdir)
        summaries.append(summarize(scenario, srows))

    with open(os.path.join(outdir, "summary.csv"), "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=list(summaries[0].keys()))
        writer.writeheader()
        writer.writerows(summaries)

    print(f"Listo. {len(grouped)} escenario(s) procesados en ./{outdir}/")


if __name__ == "__main__":
    main()
