# quantum-sim · web console

A static web console that replays real output from the quantum-sim C++/CUDA
engine — ODE, PDE (wave/heat), Schrödinger, and the GPU residency benchmark.
Every trace is recorded solver output; nothing re-simulates in the browser.

## Stack

Node is used as **build tooling only** — the deployed site is fully static.

- **Vite + TypeScript**, ES modules
- Build-time data prep (`scripts/prep-data.ts`) bakes the raw CSVs into a
  single `public/data/panels.json` with precomputed extents (no client-side
  CSV parsing)
- **GitHub Actions** builds and deploys the static `dist/` to GitHub Pages

## Layout

```
src/
  data/     types + manifest loader
  clock/    master time-base (shared playback)
  render/   pure, DOM-free draw functions
  panels/   panel wiring (Phase 3)
  main.ts   boot
scripts/
  prep-data.ts   CSV -> panels.json (build-time)
public/data/     baked output + raw CSVs
```

## Commands

```
npm install
npm run dev      # Vite dev server
npm run prep     # bake CSV -> panels.json
npm run build    # prep + typecheck + static build -> dist/
npm run preview  # serve the built site
```

## Deploy

Pushing to `main` triggers `.github/workflows/deploy.yml`. It builds with
`BASE_PATH=/Quantum-Simulation/` (the project-page subpath) and publishes
`dist/` to Pages. Rename that if the repo name changes.
