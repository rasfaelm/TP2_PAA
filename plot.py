"""
Plota tempo x tamanho da entrada a partir do bench.csv.
Uso: python3 plot.py [bench.csv] [grafico.png]
REQUER MATPLOTLIB (pip install matplotlib)
"""
import csv, sys
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

entrada = sys.argv[1] if len(sys.argv) > 1 else "bench.csv"
saida = sys.argv[2] if len(sys.argv) > 2 else "grafico.png"

n, cel, ms, msg = [], [], [], []
with open(entrada) as f:
    for row in csv.DictReader(f):
        n.append(int(row["n"]))
        cel.append(int(row["celulas"]))
        ms.append(float(row["tempo_ms"]))
        if "tempo_guloso_ms" in row:
            msg.append(float(row["tempo_guloso_ms"]))

# Reta de referencia O(N): ajuste por minimos quadrados passando pela origem
k = sum(c * t for c, t in zip(cel, ms)) / sum(c * c for c in cel)
ref = [k * c for c in cel]

fig, ax = plt.subplots(figsize=(7, 4.5))
ax.plot(cel, ms, "o-", label="Tempo medido")
ax.plot(cel, ref, "--", label="Referencia linear  O(N)")
ax.set_xlabel("Tamanho da entrada  N = linhas x colunas (celulas)")
ax.set_ylabel("Tempo medio (ms)")
ax.set_title("PD da caverna: tempo x tamanho da entrada")
ax.grid(True, alpha=0.3)
ax.legend()
fig.tight_layout()
fig.savefig(saida, dpi=150)
print("Grafico salvo em", saida)

# Figura extra: PD x guloso (so se o CSV tiver a coluna do guloso)
if msg:
    saida2 = saida.rsplit(".", 1)[0] + "_guloso.png"
    fig, ax = plt.subplots(figsize=(7, 4.5))
    ax.plot(cel, ms, "o-", label="PD  O(N)")
    ax.plot(cel, msg, "s-", label="Guloso  O(n+m)")
    ax.set_yscale("log")
    ax.set_xlabel("Tamanho da entrada  N = linhas x colunas (celulas)")
    ax.set_ylabel("Tempo medio (ms, escala log)")
    ax.set_title("PD x guloso: tempo x tamanho da entrada")
    ax.grid(True, alpha=0.3, which="both")
    ax.legend()
    fig.tight_layout()
    fig.savefig(saida2, dpi=150)
    print("Grafico salvo em", saida2)