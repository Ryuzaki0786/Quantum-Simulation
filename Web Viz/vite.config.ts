import { defineConfig } from "vite";

// For a GitHub Pages *project* site the app is served from
//   https://<user>.github.io/<repo>/
// so assets must be requested under that subpath. Override with
//   BASE_PATH=/Quantum-Simulation/ npm run build
// or set it in the Actions workflow. Defaults to "/" for local dev/preview.
export default defineConfig({
  base: process.env.BASE_PATH ?? "/",
  build: {
    outDir: "dist",
    target: "es2022",
    sourcemap: false,
  },
});
