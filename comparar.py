"""
Compara a PD com o guloso (qualidade da solucao) e valida a PD por forca bruta.

Usa os PROPRIOS executaveis do projeto (caixa-preta):
    ./caverna  <mapa> <saida>          -> caminho da PD
    ./caverna --guloso <mapa> <saida>  -> caminho do guloso
Os caminhos gravados sao relidos e simulados sobre o mapa (PV, movimentos,
inicio/fim), entao um caminho invalido seria detectado.

Uso: python3 comparar.py [--bin ./caverna] [--k 300] [--saida comparacao]
Gera em <saida>/: resultados.csv, tabelas.md, grafico_qualidade.png
REQUER MATPLOTLIB para o grafico (pip install matplotlib).
"""
import argparse, csv, itertools, os, random, subprocess, sys, tempfile

# Dois ambientes de teste (a lista de valores e sorteada uniformemente):
#  - generoso: media dos valores > 0 (muitas pocoes), quase todos sobrevivem
#  - hostil:   media dos valores < 0 (muitos monstros), sobreviver e disputado
GENEROSO = [-10, -5, 0, 0, 5, 10, 20]
HOSTIL = [-30, -20, -20, -10, -10, 0, 0, 10, 20]
# ambiente, nome, n (caverna n x n), PV iniciais, valores possiveis das celulas
CONFIGS = [
    ("Generoso", "5x5",     5,  15, GENEROSO),
    ("Generoso", "10x10",  10,  30, GENEROSO),
    ("Generoso", "20x20",  20,  60, GENEROSO),
    ("Generoso", "50x50",  50, 120, GENEROSO),
    ("Generoso", "100x100", 100, 250, GENEROSO),
    ("Hostil", "5x5",     5,  30, HOSTIL),
    ("Hostil", "10x10",  10,  60, HOSTIL),
    ("Hostil", "20x20",  20,  80, HOSTIL),
    ("Hostil", "50x50",  50, 200, HOSTIL),
    ("Hostil", "100x100", 100, 400, HOSTIL),
]


def gerar_mapa(rng, n, pv, valores):
    g = [[rng.choice(valores) for _ in range(n)] for _ in range(n)]
    g[0][0] = g[n - 1][n - 1] = 0   # F e I valem 0 (como em lerCaverna)
    return g, pv


def escrever_mapa(caminho, g, pv):
    n = len(g)
    with open(caminho, "w") as f:
        f.write(f"{n} {n} {pv}\n")
        for i in range(n):
            f.write(" ".join("F" if (i, j) == (0, 0) else "I" if (i, j) == (n - 1, n - 1) else f"{g[i][j]:+d}" for j in range(n)) + "\n")


def ler_caminho(arq):
    txt = open(arq).read().strip()
    if txt.startswith("imposs"):
        return None
    return [tuple(map(int, ln.split())) for ln in txt.splitlines()]


def simular(g, pv0, cam):
    """Valida o caminho e devolve os PV finais. Levanta AssertionError se invalido."""
    n = len(g)
    assert cam[0] == (n - 1, n - 1), "caminho nao comeca em I"
    assert cam[-1] == (0, 0), "caminho nao termina em F"
    pv = pv0
    for (a, b), (c, d) in zip(cam, cam[1:]):
        assert (c, d) in ((a - 1, b), (a, b - 1)), "movimento invalido"
        pv += g[c][d]
        assert pv > 0, "caminho passa por PV <= 0"
    return pv


def forca_bruta(g, pv0):
    """Melhor PV final enumerando TODOS os caminhos cima/esquerda (so p/ mapas pequenos)."""
    n = len(g)
    melhor = None
    for subidas in itertools.combinations(range(2 * (n - 1)), n - 1):
        s = set(subidas)
        i = j = n - 1
        pv, ok = pv0, True
        for k in range(2 * (n - 1)):
            if k in s: i -= 1
            else: j -= 1
            pv += g[i][j]
            if pv <= 0: ok = False; break
        if ok and (melhor is None or pv > melhor):
            melhor = pv
    return melhor


def rodar(binario, args):
    subprocess.run([binario] + args, check=True, stdout=subprocess.DEVNULL)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin", default="./caverna")
    ap.add_argument("--k", type=int, default=300, help="mapas aleatorios por configuracao")
    ap.add_argument("--saida", default="comparacao")
    a = ap.parse_args()
    os.makedirs(a.saida, exist_ok=True)
    rng = random.Random(2026)  # semente fixa: resultados reprodutiveis
    tmp = tempfile.mkdtemp()
    mapa, sd, sg = (os.path.join(tmp, x) for x in ("mapa.txt", "pd.txt", "gu.txt"))

    verif = []
    for n, qtd in ((3, 200), (4, 200), (5, 200), (6, 200)):
        ok = 0
        for _ in range(qtd):
            g, pv = gerar_mapa(rng, n, rng.choice([5, 10, 20]), [-10, -5, 0, 0, 5, 10, 20])
            escrever_mapa(mapa, g, pv)
            rodar(a.bin, [mapa, sd])
            cam = ler_caminho(sd)
            fb = forca_bruta(g, pv)
            pd = simular(g, pv, cam) if cam else None
            if pd == fb:
                ok += 1
            else:
                print(f"DIVERGENCIA n={n}: PD={pd} forca bruta={fb}", file=sys.stderr)
        verif.append((n, qtd, ok))

    linhas = []
    for amb, nome, n, pv, valores in CONFIGS:
        k = a.k
        pd_ok = gu_ok = gu_otimo = ambos = 0
        perdas, perdas_rel = [], []
        for _ in range(k):
            g, _ = gerar_mapa(rng, n, pv, valores)
            escrever_mapa(mapa, g, pv)
            rodar(a.bin, [mapa, sd])
            rodar(a.bin, ["--guloso", mapa, sg])
            cp, cg = ler_caminho(sd), ler_caminho(sg)
            vp = simular(g, pv, cp) if cp else None
            vg = simular(g, pv, cg) if cg else None
            if vg is not None:
                assert vp is not None and vg <= vp, "guloso nao pode superar a PD"
            if vp is not None:
                pd_ok += 1
                if vg is None:
                    pass
                else:
                    ambos += 1
                    gu_ok += 1
                    if vg == vp: gu_otimo += 1
                    perdas.append(vp - vg)
                    perdas_rel.append(100.0 * (vp - vg) / vp)
        linhas.append(dict(
            ambiente=amb, mapa=nome, pv_inicial=pv, k=k, pd_resolve=pd_ok, gu_resolve=gu_ok,
            gu_otimo=gu_otimo,
            perda_media=(sum(perdas) / len(perdas)) if perdas else 0.0,
            perda_max=max(perdas) if perdas else 0,
            perda_rel_media=(sum(perdas_rel) / len(perdas_rel)) if perdas_rel else 0.0))
        print(f"{amb} {nome}: PD resolve {pd_ok}/{k}, guloso resolve {gu_ok}, otimo {gu_otimo}")

    with open(os.path.join(a.saida, "resultados.csv"), "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(linhas[0].keys()))
        w.writeheader(); w.writerows(linhas)

    md = ["## Validacao da PD (forca bruta)\n",
          "| Caverna | Mapas testados | PD = forca bruta |", "|---|---|---|"]
    for n, qtd, ok in verif:
        md.append(f"| {n}x{n} | {qtd} | {ok} |")
    for amb in ("Generoso", "Hostil"):
        md += ["", f"## Guloso x PD - ambiente {amb.lower()} (mapas n x n, I no canto inferior direito, F no superior esquerdo)\n",
               "| Caverna | PV iniciais | Mapas | PD escapa | Guloso escapa | Guloso falha mas PD escapa | Guloso = otimo | Perda media (PV) | Perda maxima (PV) | Perda media (%) |",
               "|---|---|---|---|---|---|---|---|---|---|"]
        for r in (x for x in linhas if x["ambiente"] == amb):
            md.append(f"| {r['mapa']} | {r['pv_inicial']} | {r['k']} | {r['pd_resolve']} | {r['gu_resolve']} | "
                      f"{r['pd_resolve'] - r['gu_resolve']} | {r['gu_otimo']} | {r['perda_media']:.1f} | "
                      f"{r['perda_max']} | {r['perda_rel_media']:.1f} |")
    open(os.path.join(a.saida, "tabelas.md"), "w").write("\n".join(md) + "\n")

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        fig, eixos = plt.subplots(1, 2, figsize=(11, 4.5), sharey=True)
        for ax, amb in zip(eixos, ("Generoso", "Hostil")):
            ls = [r for r in linhas if r["ambiente"] == amb]
            x = list(range(len(ls))); lar = 0.27
            ax.bar([i - lar for i in x], [100 * r["pd_resolve"] / r["k"] for r in ls], lar, label="PD escapa")
            ax.bar(x, [100 * r["gu_resolve"] / r["k"] for r in ls], lar, label="Guloso escapa")
            ax.bar([i + lar for i in x], [100 * r["gu_otimo"] / r["k"] for r in ls], lar, label="Guloso acha o otimo")
            ax.set_xticks(x); ax.set_xticklabels([r["mapa"] for r in ls])
            ax.set_xlabel("Tamanho da caverna"); ax.set_title(f"Ambiente {amb.lower()}")
            ax.grid(True, axis="y", alpha=0.3)
        eixos[0].set_ylabel("% dos mapas aleatorios"); eixos[0].legend(loc="center left")
        fig.suptitle("PD x guloso: qualidade da solucao")
        fig.tight_layout(); fig.savefig(os.path.join(a.saida, "grafico_qualidade.png"), dpi=150)
    except ImportError:
        print("matplotlib nao instalado: grafico nao gerado.")
    print(f"\nArquivos em {a.saida}/ (resultados.csv, tabelas.md, grafico_qualidade.png)")


if __name__ == "__main__":
    main()
