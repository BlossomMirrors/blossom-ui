# Brand Guidelines



## 1. Color System

All palette token values (neutral, primary, secondary, tertiary, quaternary) live in [`colors.json`](./colors.json). That file is the single source of truth — the `.colors` schemes are generated from it at build time via `cmake/BlossomUIColors.cmake`.

**Logo Color:** Primary 500
**Main Accent Color:** Primary 300

---

## 2. Semantic Color Roles

### Dark Mode

| Role | Token |
|------|-------|
| Window Background | Neutral 800 |
| Card / View Background | Neutral 700 |
| Button / Toolbar Background | Neutral 600 |
| Primary Text | Neutral 25 |
| Secondary / Inactive Text | Neutral 100 |
| Disabled Color Blend | Neutral 600 |
| Accent / Focus / Links | Primary 300 |
| Visited Links | Primary 50 |
| Negative / Destructive | Secondary 400 |
| Positive / Success | Tertiary 500 |
| Warning / Neutral | Quaternary 500 |

### Light Mode

| Role | Token |
|------|-------|
| Window Background | Neutral 25 |
| Card / View Background | Neutral 50 |
| Button / Toolbar Background | Neutral 50 |
| Primary Text | Neutral 800 |
| Secondary / Inactive Text | Neutral 300 |
| Disabled Color Blend | Neutral 100 |
| Accent / Focus / Links | Primary 300 |
| Visited Links | Primary 700 |
| Negative / Destructive | Secondary 600 |
| Positive / Success | Tertiary 700 |
| Warning / Neutral | amber (163, 90, 0) — Quaternary 500 is for dark only |

### OLED Dark Mode

| Role | Token |
|------|-------|
| Window Background | Black (0, 0, 0) |
| Card / View Background | Neutral 900 |
| Button / Toolbar Background | Neutral 800 |

---

## 3. Border Radius

| Use Case | Radius |
|--------|--------|
| Sidebar Items | 4px |
| Menus & Menu Items | 7px |
| Buttons & Input Fields | 8px |
| Cards / Frames | 10px |
| Windows / Modals | 14px |
| Large (lg) | 24px |
| Extra Large (xl) | 36px |

---

## 4. Window Controls

| Element | Size | Radius |
|---------|------|--------|
| Button Size | 16×16px icon area | - |
| Hover Background | Full button rect | 3px |
| Background Opacity | 60% fill | - |
| Outline Opacity | 100% stroke | - |

---

## Coming soon:

- Typography
- Shadows
- Blur
- Noise
