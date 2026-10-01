# Data

Final datasets used in the paper. Each dataset directory contains:

| File | Content |
|---|---|
| `all-runs.csv` | one row per simulation run (columns: see the main README, section 4) |
| `manifest.txt` | repository commit, ns-3 version/commit, compiler, OS and all run parameters |
| `analysis/` | output of `scripts/analyze.py` (tables, statistics, figures) |

To publish a new dataset after a complete batch:

```bash
RUN=results/run-<date>
python3 scripts/analyze.py $RUN/all-runs.csv --outdir $RUN/analysis
mkdir -p data/ns-3.48
cp $RUN/all-runs.csv $RUN/manifest.txt data/ns-3.48/
cp -r $RUN/analysis data/ns-3.48/
```

Every number in the paper can be regenerated from `all-runs.csv` with `scripts/analyze.py`.
