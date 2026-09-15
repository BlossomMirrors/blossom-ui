#!/usr/bin/env python3
"""
Folder Icon Generator for BlossomUI Icon Theme
Builds all folder type and color variants from source/custom/folder-base.svg.

Type folders get a matching Tabler icon overlay (white, 0.6 opacity, centered
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


TABLER_VERSION = "3.46.0"
TABLER_BASE_URL = f"https://cdn.jsdelivr.net/npm/@tabler/icons@{TABLER_VERSION}/icons/outline"

BASE_SVG = Path("source/custom/folder-base.svg")
OUTPUT_DIR = Path("source/custom")
SOURCE_DIR = Path("source")
SIZES = ["32", "48", "64", "96"]
SMALL_SIZES = ["16", "22", "24"]

BASE_COLOR = "#1451FF"

# folder body paths in the base svg carry this fill
BODY_FILL = 'fill="#1451FF"'

# geometry of the folder front panel (viewBox 0 0 64 64)
OVERLAY_SIZE = 22
OVERLAY_CENTER = (32.0, 38.4)

TYPES = {
    "folder-activities": ["activity"],
    "folder-bookmark": ["bookmark"],
    "folder-cloud": ["cloud"],
    "folder-design": ["vector-bezier"],
    "folder-desktop": ["device-desktop"],
    "folder-development": ["code"],
    "folder-documents": ["files"],
    "folder-downloads": ["download"],
    "folder-dropbox": ["brand-dropbox"],
    "folder-encrypted": ["key"],
    "folder-favorites": ["star"],
    "folder-games": ["device-gamepad-2"],
    "folder-gdrive": ["brand-google-drive"],
    "folder-gimp": ["brush"],
    "folder-Github": ["brand-github"],
    "folder-html": ["file-type-html"],
    "folder-images": ["photo"],
    "folder-image-people": ["users"],
    "folder-important": ["alert-circle"],
    "folder-locked": ["lock"],
    "folder-mail": ["mail"],
    "folder-music": ["music"],
    "folder-nextcloud": ["brand-nextcloud"],
    "folder-network": ["network"],
    "folder-onedrive": ["brand-onedrive"],
    "folder-print": ["printer"],
    "folder-public": ["share"],
    "folder-recent": ["history"],
    "folder-remote": ["world"],
    "folder-root": ["hash"],
    "folder-script": ["script"],
    "folder-tar": ["archive"],
    "folder-temp": ["hourglass"],
    "folder-templates": ["template"],
    "folder-text": ["typography"],
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

# small outline glyphs with a baked-in color:
COLORED_GLYPHS = {
    # kio-admin: gcr-key is the plugin metadata icon,
    # yast-auth-client is the Open as Administrator action icon
    "gcr-key": (
        ["shield"],
        "#F03E4D",
        [
            "apps/16/gcr-key.svg",
            "apps/22/gcr-key.svg",
            "apps/24/gcr-key.svg",
            "apps/48/gcr-key.svg",
            "apps/symbolic/gcr-key-symbolic.svg",
            "apps/16/yast-auth-client.svg",
            "apps/22/yast-auth-client.svg",
            "apps/24/yast-auth-client.svg",
            "apps/48/yast-auth-client.svg",
            "apps/symbolic/yast-auth-client-symbolic.svg",
        ],
    ),
}

# standalone designs sharing the folder body fill:
# input svg -> (output name, symlink targets)
EXTRAS = {
    "source/custom/dolphin.svg": (
        "org.kde.dolphin",
        [
            "apps/48/org.kde.dolphin.svg",
            "apps/48/system-file-manager.svg",
            "apps/64/org.kde.dolphin.svg",
            "apps/64/system-file-manager.svg",
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

# small colored folders keep the sidebar symbolic style but bake in the color
SMALL_TEMPLATE = (
    '<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" '
    'viewBox="-3.0 -3.0 30.0 30.0" fill="none" stroke-width="1.25" '
    'stroke-linecap="round" stroke-linejoin="round">\n'
    '  <g stroke="{color}">\n{inner}\n  </g>\n'
    '</svg>\n'
)

SMALL_SYMBOLIC_TEMPLATE = (
    '<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" '
    'viewBox="-3.0 -3.0 30.0 30.0" fill="none" stroke-width="1.25" '
    'stroke-linecap="round" stroke-linejoin="round">'
    '<defs><style id="current-color-scheme" type="text/css">'
    '.ColorScheme-Text {{ color: #232629; }}</style></defs>\n'
    '  <g class="ColorScheme-Text" stroke="currentColor">\n{inner}\n  </g>\n'
    '</svg>\n'
)

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


def fetch_tabler_icon(candidates):
    for name in candidates:
        url = f"{TABLER_BASE_URL}/{name}.svg"
        try:
            response = requests.get(url, timeout=10)
            response.raise_for_status()
            return name, response.text
        except requests.exceptions.RequestException:
            continue
    return None, None


def icon_inner(svg_content):
    svg_content = re.sub(r'<!--.*?-->', '', svg_content, flags=re.DOTALL)
    svg_content = re.sub(r'^.*?<svg[^>]*>', '', svg_content, flags=re.DOTALL)
    svg_content = re.sub(r'</svg>\s*$', '', svg_content)
    svg_content = re.sub(r'<path[^>]*stroke="none"[^>]*/>', '', svg_content)
    return svg_content.strip()


def build_overlay(inner):
    cx, cy = OVERLAY_CENTER
    scale = OVERLAY_SIZE / 24.0
    tx = cx - OVERLAY_SIZE / 2.0
    ty = cy - OVERLAY_SIZE / 2.0
    return (
        f'<g opacity="0.6" transform="translate({tx:g} {ty:g}) scale({scale:g})" '
        f'fill="none" stroke="#FFFFFF" stroke-width="1.5" '
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


def save_svgz(svg_content, name, directory=None):
    # plain svg despite the name: GTK's icon lookup only accepts
    # .png/.svg/.xpm, .svgz is a KDE-only extension
    path = (directory or OUTPUT_DIR) / f"{name}.svg"
    for ext in ('.svg', '.svgz'):
        old = path.with_suffix(ext)
        if old.is_symlink() or old.exists():
            old.unlink()
    with open(path, 'w', encoding='utf-8') as f:
        f.write(svg_content)
    print(f"Created: {path}")


def create_symlink(source_name, target, directory=None):
    source_path = ((directory or OUTPUT_DIR) / f"{source_name}.svg").resolve()
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

    small_sources = set()

    for name, candidates in TYPES.items():
        icon_name, icon_svg = fetch_tabler_icon(candidates)
        if not icon_svg:
            print(f"Warning: no tabler icon for {name} "
                  f"(tried {', '.join(candidates)}), using plain folder",
                  file=sys.stderr)
            save_svgz(build_folder(base_svg), name)
            continue
        inner = icon_inner(icon_svg)
        save_svgz(build_folder(base_svg, overlay=build_overlay(inner)), name)
        if not (SOURCE_DIR / f"{name}-symbolic.svg").exists():
            save_svgz(SMALL_SYMBOLIC_TEMPLATE.format(inner=inner),
                      f"{name}-symbolic", SOURCE_DIR)
        small_sources.add(name)

    for name, color in COLORS.items():
        save_svgz(build_folder(base_svg, color=color), name)

    icon_name, folder_glyph = fetch_tabler_icon(["folder"])
    if folder_glyph:
        inner = icon_inner(folder_glyph)
        if not (SOURCE_DIR / "folder-symbolic.svg").exists():
            save_svgz(SMALL_SYMBOLIC_TEMPLATE.format(inner=inner),
                      "folder-symbolic", SOURCE_DIR)
        small_sources.add("folder")
        for name, color in COLORS.items():
            save_svgz(SMALL_TEMPLATE.format(color=color, inner=inner), f"{name}-small")
            small_sources.add(name)
    else:
        print("Warning: could not fetch tabler folder glyph, "
              "skipping small colored folders", file=sys.stderr)

    for name, (candidates, color, targets) in COLORED_GLYPHS.items():
        icon_name, glyph = fetch_tabler_icon(candidates)
        if not glyph:
            print(f"Warning: no tabler icon for {name} "
                  f"(tried {', '.join(candidates)}), skipping",
                  file=sys.stderr)
            continue
        svg = SMALL_TEMPLATE.format(color=color, inner=icon_inner(glyph))
        save_svgz(svg, name)
        for target in targets:
            create_symlink(name, target)

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

    small_links = 0
    for name, source_name in sorted(all_names.items()):
        for size in SIZES:
            create_symlink(source_name, f"places/{size}/{name}.svg")
        if source_name not in small_sources:
            continue
        coloured = source_name in COLORS
        directory = OUTPUT_DIR if coloured else SOURCE_DIR
        small_name = f"{source_name}-small" if coloured else f"{source_name}-symbolic"
        for size in SMALL_SIZES:
            create_symlink(small_name, f"places/{size}/{name}.svg", directory)
            create_symlink(small_name, f"places/{size}/{name}-symbolic.svg",
                           directory)
            small_links += 2

    print(f"\n✓ Generated {len(TYPES) + len(COLORS) + 1} folder icons, "
          f"{len(all_names) * len(SIZES) + small_links} symlinks")


if __name__ == '__main__':
    main()
