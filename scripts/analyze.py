#!/usr/bin/env python3
"""
analyze.py - tables, statistics and figures of the UD-JST paper from all-runs.csv.

    python3 scripts/analyze.py results/<run>/all-runs.csv --outdir results/<run>/analysis

Outputs (in --outdir):
    table2_means.csv              mean PDR / delay / throughput / overhead / energy
    table3_4_welch_holm.csv       Welch t-test vs AODV, Holm-Bonferroni per family
    table5_normalized.csv         NRL and energy per delivered packet (exact counts)
    route_selection.csv           PMAODV route-selection statistics
    fig1_pdr.pdf/png  fig2_delay  fig3_overhead  fig4_pdr_density
    report.md                     all tables in Markdown

Test families (as in the paper): 'default' (one test per N), 'stress' (the four
load scenarios pooled), 'density' (N = 25/30/40). Within each family and for each
(variant, metric), p-values are Holm-Bonferroni adjusted. Cohen's d is computed
on the pooled data of the family and is descriptive only.

Requires: pandas, scipy, matplotlib
"""
import argparse
import os

import numpy as np
import pandas as pd
from scipy import stats

METRICS = {
    "deliveryRatio": "PDR (%)",
    "throughputMbps": "Throughput (Mbps)",
    "avgDelayMs": "Delay (ms)",
    "routingOverhead": "Routing overhead (pkts)",
    "controlPackets": "Control packets",
    "totalEnergyJ": "Energy (J)",
}
FAMILY = {
    "default": "default",
    "multi-flow": "stress",
    "stress-mob": "stress",
    "stress-load": "stress",
    "stress-all": "stress",
    "density": "density",
}
ORDER = ["AODV", "DSDV", "AOMDV(mp=3)", "PMAODV(mp=2)", "PMAODV(mp=3)", "PMAODV(mp=4)"]


def variant(row):
    if row["protocol"] in ("AOMDV", "PMAODV"):
        return f'{row["protocol"]}(mp={row["maxPaths"]})'
    return row["protocol"]


def holm(p):
    p = np.asarray(p, float)
    m = len(p)
    adj = np.empty(m)
    running = 0.0
    for rank, idx in enumerate(np.argsort(p)):
        running = max(running, (m - rank) * p[idx])
        adj[idx] = min(running, 1.0)
    return adj


def cohens_d(a, b):
    a, b = np.asarray(a, float), np.asarray(b, float)
    if len(a) < 2 or len(b) < 2:
        return np.nan
    sp = np.sqrt(((len(a) - 1) * a.var(ddof=1) + (len(b) - 1) * b.var(ddof=1))
                 / (len(a) + len(b) - 2))
    return (a.mean() - b.mean()) / sp if sp > 0 else 0.0


def ordered(vs):
    return [v for v in ORDER if v in vs] + sorted(v for v in vs if v not in ORDER)


def welch_holm(df, alpha):
    rows = []
    df = df.assign(family=df["scenario"].map(FAMILY).fillna(df["scenario"]))
    for fam, fdf in df.groupby("family"):
        base = fdf[fdf["variant"] == "AODV"]
        for var in ordered(fdf["variant"].unique()):
            if var == "AODV":
                continue
            vdf = fdf[fdf["variant"] == var]
            for metric in METRICS:
                if metric not in fdf:
                    continue
                tests = []
                for (scen, n), g in vdf.groupby(["scenario", "numNodes"]):
                    a = g[metric].values
                    b = base[(base["scenario"] == scen) & (base["numNodes"] == n)][metric].values
                    if len(a) < 2 or len(b) < 2:
                        continue
                    if a.std() == 0 and b.std() == 0:
                        p = 1.0 if a.mean() == b.mean() else 0.0
                    else:
                        p = stats.ttest_ind(a, b, equal_var=False).pvalue
                    tests.append((p, a.mean() - b.mean()))
                if not tests:
                    continue
                padj = holm([t[0] for t in tests])
                sig = [t for t, pa in zip(tests, padj) if pa < alpha]
                keys = vdf[["scenario", "numNodes"]].drop_duplicates()
                b_all = base.merge(keys, on=["scenario", "numNodes"])[metric]
                rows.append({
                    "family": fam, "variant": var, "metric": metric, "tests": len(tests),
                    "sig_raw": sum(t[0] < alpha for t in tests), "sig_holm": len(sig),
                    "sig_higher": sum(t[1] > 0 for t in sig),
                    "sig_lower": sum(t[1] < 0 for t in sig),
                    "cohens_d": round(cohens_d(vdf[metric], b_all), 3),
                })
    return pd.DataFrame(rows)


def normalized(df):
    d = df[df["dataRx"] > 0].copy()
    d["NRL_legacy"] = d["routingOverhead"] / d["dataRx"]
    d["NRL_control"] = d["controlPackets"] / d["dataRx"]
    d["energy_per_pkt_J"] = d["totalEnergyJ"] / d["dataRx"]
    out = []
    for var in ordered(d["variant"].unique()):
        g = d[d["variant"] == var]
        row = {"variant": var, "n_runs": len(g),
               "runs_without_delivery": int(((df["variant"] == var) & (df["dataRx"] == 0)).sum())}
        for c in ("NRL_legacy", "NRL_control", "energy_per_pkt_J"):
            row[c] = f"{g[c].mean():.3f} ± {g[c].std():.3f}"
        out.append(row)
    return pd.DataFrame(out)


def selection(df):
    p = df[df["protocol"] == "PMAODV"]
    if p.empty or "selEvents" not in p:
        return pd.DataFrame()
    g = p.groupby(["scenario", "variant"])[["selEvents", "selMultiCandidate", "selNonMinHop"]].sum()
    g["multi_%_of_all"] = 100 * g["selMultiCandidate"] / g["selEvents"]
    g["nonMin_%_of_all"] = 100 * g["selNonMinHop"] / g["selEvents"]
    g["nonMin_%_of_multi"] = 100 * g["selNonMinHop"] / g["selMultiCandidate"]
    return g.round(2).reset_index()


def figures(df, outdir):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    def plot(sub, metric, ylabel, name, errbars=False, title=None):
        fig, ax = plt.subplots(figsize=(6.4, 4.0))
        for var in ordered(sub["variant"].unique()):
            g = sub[sub["variant"] == var].groupby("numNodes")[metric]
            m, s = g.mean(), g.std()
            if errbars:
                ax.errorbar(m.index, m.values, yerr=s.values, marker="o", ms=3, capsize=2, label=var)
            else:
                ax.plot(m.index, m.values, marker="o", ms=3, label=var)
        ax.set_xticks(sorted(sub["numNodes"].unique()))
        ax.set_xlabel("Number of UAVs (N)")
        ax.set_ylabel(ylabel)
        if title:
            ax.set_title(title, fontsize=10)
        ax.grid(alpha=0.3)
        ax.legend(fontsize=8)
        fig.tight_layout()
        for ext in ("pdf", "png"):
            fig.savefig(os.path.join(outdir, f"{name}.{ext}"), dpi=200)
        plt.close(fig)

    dflt = df[df["scenario"] == "default"]
    if not dflt.empty:
        plot(dflt, "deliveryRatio", "Packet delivery ratio (%)", "fig1_pdr")
        plot(dflt, "avgDelayMs", "Average end-to-end delay (ms)", "fig2_delay")
        plot(dflt, "routingOverhead", "Routing overhead (packets)", "fig3_overhead",
             errbars=True, title="error bars = ±1 SD over seeds")
    dens = df[df["scenario"].isin(["default", "density"])]
    if not dens.empty and (df["scenario"] == "density").any():
        plot(dens, "deliveryRatio", "Packet delivery ratio (%)", "fig4_pdr_density")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("csv")
    ap.add_argument("--outdir", default="analysis")
    ap.add_argument("--alpha", type=float, default=0.05)
    ap.add_argument("--no-figures", action="store_true")
    a = ap.parse_args()
    os.makedirs(a.outdir, exist_ok=True)

    df = pd.read_csv(a.csv)
    df["variant"] = df.apply(variant, axis=1)
    if "multicastRrep" in df and df["multicastRrep"].nunique() > 1:
        raise SystemExit("CSV mixes MulticastRrep=0 and 1 runs; analyse them separately.")

    t2 = (df.groupby(["scenario", "variant"])[[m for m in METRICS if m in df]]
            .mean().round(3).reset_index())
    t34 = welch_holm(df, a.alpha)
    t5 = normalized(df[df["scenario"] == "default"]) if "dataRx" in df else pd.DataFrame()
    sel = selection(df)

    t2.to_csv(os.path.join(a.outdir, "table2_means.csv"), index=False)
    t34.to_csv(os.path.join(a.outdir, "table3_4_welch_holm.csv"), index=False)
    t5.to_csv(os.path.join(a.outdir, "table5_normalized.csv"), index=False)
    sel.to_csv(os.path.join(a.outdir, "route_selection.csv"), index=False)
    if not a.no_figures:
        figures(df, a.outdir)

    counts = df.groupby(["scenario", "variant"]).size().unstack(fill_value=0)
    with open(os.path.join(a.outdir, "report.md"), "w") as f:
        f.write(f"# Analysis of `{os.path.basename(a.csv)}`\n\n")
        f.write("## Runs per scenario and variant\n\n" + counts.to_markdown() + "\n\n")
        f.write("## Table 2 - means\n\n" + t2.to_markdown(index=False) + "\n\n")
        f.write(f"## Tables 3/4 - Welch t-test vs AODV, Holm-Bonferroni (alpha={a.alpha})\n\n"
                + t34.to_markdown(index=False) + "\n\n")
        if not t5.empty:
            f.write("## Table 5 - normalized load, default scenario (mean ± SD per run)\n\n"
                    "NRL_legacy = routingOverhead / delivered; NRL_control = control packets / "
                    "delivered; delivered packets are counted exactly by FlowMonitor.\n\n"
                    + t5.to_markdown(index=False) + "\n\n")
        if not sel.empty:
            f.write("## PMAODV route selection\n\n" + sel.to_markdown(index=False) + "\n")
    print(open(os.path.join(a.outdir, "report.md")).read())
    print(f"Written to {a.outdir}/")


if __name__ == "__main__":
    main()
