# Data

Final datasets used in the paper. Each dataset directory contains:

| File | Content |
|---|---|
| `all-runs.csv` | one row per simulation run (columns: see the main README, section 4) |
| `manifest.txt` | repository commit, ns-3 version/commit, compiler, OS and all run parameters |
| `analysis/` | output of `scripts/analyze.py` (tables, statistics, figures) |

To publish a new dataset after a complete batch:

```bash
RUN=data/ns-3.48/runs/run-<date>
.venv/bin/python scripts/analyze.py $RUN/all-runs.csv --outdir $RUN/analysis
git add $RUN/all-runs.csv $RUN/manifest.txt $RUN/analysis
```

Per-run CSVs, logs and the job list stay local (`.gitignore`); only the merged
`all-runs.csv`, `manifest.txt` and `analysis/` are committed.

Every number in the paper can be regenerated from `all-runs.csv` with `scripts/analyze.py`.
