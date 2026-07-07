#!/usr/bin/env python3
"""
Folder Icon Generator for BlossomUI Icon Theme
Builds all folder type and color variants from source/custom/folder.svg.

Type folders get a matching Lucide icon overlay (white, 0.6 opacity, centered
on the folder front with margin) and keep the KDE current-color-scheme
structure so the folder body follows the accent color:

    <defs>
        <style id="current-color-scheme" type="text/css">
            .ColorScheme-Highlight { color:#1451FF; }
        </style>
    </defs>
    <path class="ColorScheme-Highlight" fill="currentColor" .../>

Forced color folders (folder-red, folder-green, ...) get their color baked
into the paths so they keep it regardless of the active color scheme.

Usage:
    python foldergen.py
"""

import gzip
import os
import re
import sys
import requests
from pathlib import Path


LUCIDE_BASE_URL = "https://cdn.jsdelivr.net/npm/lucide-static@latest/icons"

BASE_SVG = Path("source/custom/folder.svg")
OUTPUT_DIR = Path("source/custom")
SIZES = ["32", "48", "64", "96"]

BASE_COLOR = "#1451FF"

# folder body paths in the base svg carry this fill
BODY_FILL = 'fill="#1451FF"'

# geometry of the folder front panel (viewBox 0 0 64 64)
OVERLAY_SIZE = 22
OVERLAY_CENTER = (32.0, 38.4)

# type folders: name -> lucide icon candidates (first that fetches wins)
TYPES = {
    "folder-activities": ["activity"],
    "folder-bookmark": ["bookmark"],
    "folder-cloud": ["cloud"],
    "folder-design": ["pen-tool"],
    "folder-desktop": ["monitor"],
    "folder-development": ["code"],
    "folder-documents": ["file-text"],
    "folder-downloads": ["download"],
    "folder-dropbox": ["box"],
    "folder-encrypted": ["key"],
    "folder-favorites": ["star"],
    "folder-games": ["gamepad-2"],
    "folder-gdrive": ["triangle"],
    "folder-gimp": ["brush"],
    "folder-Github": ["github", "git-branch"],
    "folder-html": ["code-xml", "code"],
    "folder-images": ["image"],
    "folder-image-people": ["users"],
    "folder-important": ["circle-alert", "alert-circle"],
    "folder-locked": ["lock"],
    "folder-mail": ["mail"],
    "folder-music": ["music"],
    "folder-network": ["network"],
    "folder-print": ["printer"],
    "folder-public": ["share-2"],
    "folder-recent": ["history"],
    "folder-remote": ["globe"],
    "folder-root": ["hash"],
    "folder-script": ["scroll-text", "scroll"],
    "folder-tar": ["archive"],
    "folder-temp": ["hourglass"],
    "folder-templates": ["layout-template"],
    "folder-text": ["letter-text", "text"],
    "folder-unlocked": ["lock-open"],
    "folder-videos": ["video"],
}

# forced color folders: name -> hardcoded color
COLORS = {
    "folder-black": "#383D45",
    "folder-blue": "#1451FF",
    "folder-brown": "#9C6644",
    "folder-cyan": "#17B8CE",
    "folder-green": "#2FBF57",
    "folder-grey": "#8A929E",
    "folder-magenta": "#ED4DB2",
    "folder-orange": "#FF8324",
    "folder-red": "#F03E4D",
    "folder-violet": "#7C4DFF",
    "folder-yellow": "#FFC024",
}

# standalone designs sharing the folder body fill:
# input svg -> (output name, symlink targets)
EXTRAS = {
    "source/custom/dolphin.svg": (
        "org.kde.dolphin",
        [
            "apps/48/org.kde.dolphin.svgz",
            "apps/48/system-file-manager.svgz",
            "apps/64/org.kde.dolphin.svgz",
            "apps/64/system-file-manager.svgz",
        ],
    ),
}

# pure aliases: name -> canonical source name
ALIASES = {
    "folder-arch": "folder",
    "folder-decrypted": "folder-unlocked",
    "folder-download": "folder-downloads",
    "folder-image": "folder-images",
    "folder-KDE": "folder",
    "folder-Manjaro": "folder",
    "folder-Neon": "folder",
    "folder-onedrive": "folder-cloud",
    "folder-open": "folder",
    "folder-openSUSE": "folder",
    "folder-owncloud": "folder-cloud",
    "folder-picture": "folder-images",
    "folder-pictures": "folder-images",
    "folder-Pop_OS": "folder",
    "folder-publicshare": "folder-public",
    "folder-Reddit": "folder",
    "folder-sound": "folder-music",
    "folder-txt": "folder-text",
    "folder-video": "folder-videos",
    "folder_html": "folder-html",
    "inode-directory": "folder",
    "stock_folder": "folder",
    "user-desktop": "folder-desktop",
}

STYLE_BLOCK = (
    '<style\n'
    '            id="current-color-scheme"\n'
    '            type="text/css">\n\n'
    '            .ColorScheme-Highlight\n'
    '            {{\n'
    '                color:{color};\n'
    '            }}\n\n'
    '        </style>\n'
)


def fetch_lucide_icon(candidates):
    for name in candidates:
        url = f"{LUCIDE_BASE_URL}/{name}.svg"
        try:
            response = requests.get(url, timeout=10)
            response.raise_for_status()
            return name, response.text
        except requests.exceptions.RequestException:
            continue
    return None, None


def lucide_inner(svg_content):
    svg_content = re.sub(r'<!--.*?-->', '', svg_content, flags=re.DOTALL)
    svg_content = re.sub(r'^.*?<svg[^>]*>', '', svg_content, flags=re.DOTALL)
    svg_content = re.sub(r'</svg>\s*$', '', svg_content)
    return svg_content.strip()


def build_overlay(inner):
    cx, cy = OVERLAY_CENTER
    scale = OVERLAY_SIZE / 24.0
    tx = cx - OVERLAY_SIZE / 2.0
    ty = cy - OVERLAY_SIZE / 2.0
    return (
        f'<g opacity="0.6" transform="translate({tx:g} {ty:g}) scale({scale:g})" '
        f'fill="none" stroke="#FFFFFF" stroke-width="2" '
        f'stroke-linecap="round" stroke-linejoin="round">\n{inner}\n</g>\n'
    )


def build_folder(base_svg, color=None, overlay=None):
    svg = base_svg
    if color:
        svg = svg.replace(BODY_FILL, f'fill="{color}"')
        style_color = color
    else:
        svg = svg.replace(
            BODY_FILL, f'class="ColorScheme-Highlight" fill="currentColor"'
        )
        style_color = BASE_COLOR

    # QtSvg applies stylesheets only to elements parsed after them,
    # so the style block must precede all painted elements
    style = '<defs>\n' + STYLE_BLOCK.format(color=style_color) + '</defs>\n'
    match = re.search(r'<svg[^>]*>', svg)
    pos = match.end()
    svg = svg[:pos] + '\n' + style + svg[pos:]

    if overlay:
        svg = svg.replace('</svg>', overlay + '</svg>')

    return svg


def save_svgz(svg_content, name):
    path = OUTPUT_DIR / f"{name}.svgz"
    if path.is_symlink() or path.exists():
        path.unlink()
    with gzip.open(path, 'wt', encoding='utf-8') as f:
        f.write(svg_content)
    print(f"Created: {path}")


def create_symlink(source_name, target):
    source_path = (OUTPUT_DIR / f"{source_name}.svgz").resolve()
    target_path = Path(target)
    target_path.parent.mkdir(parents=True, exist_ok=True)

    for ext in ['.svg', '.svgz']:
        old = target_path.with_suffix(ext)
        if old.exists() or old.is_symlink():
            old.unlink()

    rel_source = os.path.relpath(source_path, target_path.parent)
    target_path.symlink_to(rel_source)
    print(f"Symlinked: {target_path} -> {rel_source}")


def main():
    if not BASE_SVG.exists():
        print(f"Error: {BASE_SVG} not found", file=sys.stderr)
        sys.exit(1)

    base_svg = BASE_SVG.read_text()
    if BODY_FILL not in base_svg:
        print(f"Error: {BODY_FILL} not found in {BASE_SVG}", file=sys.stderr)
        sys.exit(1)

    save_svgz(build_folder(base_svg), "folder")

    for name, candidates in TYPES.items():
        icon_name, icon_svg = fetch_lucide_icon(candidates)
        if not icon_svg:
            print(f"Warning: no lucide icon for {name} "
                  f"(tried {', '.join(candidates)}), using plain folder",
                  file=sys.stderr)
            save_svgz(build_folder(base_svg), name)
            continue
        overlay = build_overlay(lucide_inner(icon_svg))
        save_svgz(build_folder(base_svg, overlay=overlay), name)

    for name, color in COLORS.items():
        save_svgz(build_folder(base_svg, color=color), name)

    for src, (name, targets) in EXTRAS.items():
        svg = Path(src).read_text()
        if BODY_FILL not in svg:
            print(f"Warning: {BODY_FILL} not found in {src}, skipping",
                  file=sys.stderr)
            continue
        save_svgz(build_folder(svg), name)
        for target in targets:
            create_symlink(name, target)

    all_names = {"folder": "folder"}
    all_names.update({n: n for n in TYPES})
    all_names.update({n: n for n in COLORS})
    all_names.update(ALIASES)

    for name, source_name in sorted(all_names.items()):
        for size in SIZES:
            create_symlink(source_name, f"places/{size}/{name}.svgz")

    print(f"\n✓ Generated {len(TYPES) + len(COLORS) + 1} folder icons, "
          f"{len(all_names) * len(SIZES)} symlinks")


if __name__ == '__main__':
    main()
