# Manuscript

Draft for *Robotics and Autonomous Systems* (Elsevier, `elsarticle`).

| File | Content |
|---|---|
| `main.tex` | manuscript; items to complete are marked `TODO` (red in the PDF) |
| `refs.bib` | bibliography; check every entry against the publisher's record |
| `highlights.txt` | highlights (RAS: 3–5 bullets, at most 85 characters each) |
| `make_figures.py` | regenerates `figures/` from the committed campaign outputs |
| `main.pdf` | compiled draft |

Build:

```bash
python3 paper/make_figures.py --scratch <dir with runs/ and campaign_tex*/>   # Figs. 1 and 5 need the logs
cd paper && latexmk -pdf main.tex
```

Every number in the text comes from `RESULTS.md` and the CSV files in
`confirmatory/`, `replication/` and `sweep/`.
