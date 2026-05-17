# Blossom UI Registry

A shadcn-svelte component registry for [BlossomOS](https://github.com/vaporvee/blossom). Colors, radii and semantic tokens are derived from the project-wide [`colors.json`](../colors.json).

## Color pipeline

`colors.json` (at the repo root) is the single source of truth. `scripts/compile-colors.mjs` reads it and writes `src/styles/theme.css` containing `:root` (light mode) and `.dark` (dark mode) CSS custom properties consumed by `src/app.css`. The mapping follows the semantic roles documented in the top-level [`README.md`](../README.md).

```bash
bun run compile:colors
```

The script runs automatically before `dev`, `build`, and `build:registry`. The generated `src/styles/theme.css` is gitignored.

## Building the registry

```bash
bun run build:registry
```

Registry items output to `static/r/[name].json` and are consumable via the `shadcn-svelte` CLI.

## Items

| Name   | Type           |
| ------ | -------------- |
| button | `registry:ui`  |

More components will be added as the BlossomOS design system grows.
