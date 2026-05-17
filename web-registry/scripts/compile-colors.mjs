#!/usr/bin/env node
import { readFileSync, writeFileSync, mkdirSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const __dirname = dirname(fileURLToPath(import.meta.url));
const repoRoot = resolve(__dirname, "..", "..");
const colorsPath = resolve(repoRoot, "colors.json");
const outPath = resolve(__dirname, "..", "src", "styles", "theme.css");

const palette = JSON.parse(readFileSync(colorsPath, "utf8"));

const ref = (group, step) => {
	if (!palette[group]?.[String(step)]) {
		throw new Error(`colors.json is missing ${group}.${step}`);
	}
	return `var(--${group}-${step})`;
};

const warningLight = "#A35A00";
const white = "#FFFFFF";

const primitives = {};
for (const [group, steps] of Object.entries(palette)) {
	for (const [step, hex] of Object.entries(steps)) {
		primitives[`${group}-${step}`] = hex;
	}
}

const dark = {
	background: ref("neutral", 900),
	foreground: ref("neutral", 25),
	card: ref("neutral", 850),
	"card-foreground": ref("neutral", 25),
	popover: ref("neutral", 800),
	"popover-foreground": ref("neutral", 25),
	button: ref("neutral", 800),
	"button-foreground": ref("neutral", 25),
	"button-hover": ref("neutral", 700),
	"button-accent": ref("primary", 300),
	"button-accent-hover": ref("primary", 200),
	primary: ref("primary", 300),
	"primary-foreground": white,
	muted: ref("neutral", 800),
	"muted-foreground": ref("neutral", 100),
	accent: ref("primary", 300),
	"accent-foreground": white,
	destructive: ref("secondary", 400),
	"destructive-foreground": white,
	success: ref("tertiary", 500),
	warning: ref("quaternary", 500),
	border: ref("neutral", 700),
	input: ref("neutral", 700),
	ring: ref("primary", 300),
	link: ref("primary", 300),
	"link-visited": ref("primary", 50),
	"chart-1": ref("primary", 300),
	"chart-2": ref("secondary", 400),
	"chart-3": ref("tertiary", 500),
	"chart-4": ref("quaternary", 500),
	"chart-5": ref("cyan", 300),
	sidebar: ref("neutral", 900),
	"sidebar-foreground": ref("neutral", 25),
	"sidebar-primary": ref("primary", 300),
	"sidebar-primary-foreground": white,
	"sidebar-accent": ref("neutral", 850),
	"sidebar-accent-foreground": ref("neutral", 25),
	"sidebar-border": ref("neutral", 700),
	"sidebar-ring": ref("primary", 300),
};

const light = {
	background: ref("neutral", 25),
	foreground: ref("neutral", 800),
	card: ref("neutral", 50),
	"card-foreground": ref("neutral", 800),
	popover: ref("neutral", 50),
	"popover-foreground": ref("neutral", 800),
	button: ref("neutral", 100),
	"button-foreground": ref("neutral", 800),
	"button-hover": ref("neutral", 200),
	"button-accent": ref("primary", 300),
	"button-accent-hover": ref("primary", 200),
	primary: ref("primary", 300),
	"primary-foreground": ref("neutral", 25),
	muted: ref("neutral", 50),
	"muted-foreground": ref("neutral", 300),
	accent: ref("primary", 300),
	"accent-foreground": ref("neutral", 25),
	destructive: ref("secondary", 600),
	"destructive-foreground": ref("neutral", 25),
	success: ref("tertiary", 700),
	warning: warningLight,
	border: ref("neutral", 200),
	input: ref("neutral", 200),
	ring: ref("primary", 300),
	link: ref("primary", 300),
	"link-visited": ref("primary", 700),
	"chart-1": ref("primary", 500),
	"chart-2": ref("secondary", 600),
	"chart-3": ref("tertiary", 700),
	"chart-4": warningLight,
	"chart-5": ref("cyan", 300),
	sidebar: ref("neutral", 50),
	"sidebar-foreground": ref("neutral", 800),
	"sidebar-primary": ref("primary", 300),
	"sidebar-primary-foreground": ref("neutral", 25),
	"sidebar-accent": ref("neutral", 25),
	"sidebar-accent-foreground": ref("neutral", 800),
	"sidebar-border": ref("neutral", 200),
	"sidebar-ring": ref("primary", 300),
};

const radii = {
	radius: "0.625rem",
	"radius-sidebar-item": "4px",
	"radius-menu": "7px",
	"radius-button": "8px",
	"radius-card": "10px",
	"radius-window": "14px",
};

const formatBlock = (selector, vars) => {
	const body = Object.entries(vars)
		.map(([k, v]) => `\t--${k}: ${v};`)
		.join("\n");
	return `${selector} {\n${body}\n}`;
};

const header = `/* AUTO-GENERATED from colors.json by scripts/compile-colors.mjs.\n   Do not edit by hand. Run \`bun run compile:colors\` to regenerate. */\n`;

const output = [
	header,
	formatBlock(":root", { ...primitives, ...radii, ...light }),
	"",
	formatBlock(".dark", dark),
	"",
].join("\n");

mkdirSync(dirname(outPath), { recursive: true });
writeFileSync(outPath, output);
console.log(`Wrote ${outPath}`);
