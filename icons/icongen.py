#!/usr/bin/env python3
import sys
import os
import json
import argparse
from pathlib import Path
from xml.etree import ElementTree as ET


TABLER_VERSION = "3.46.0"
TABLER_VARIANT = "outline"
TABLER_BASE_URL = f"https://cdn.jsdelivr.net/npm/@tabler/icons@{TABLER_VERSION}/icons/{TABLER_VARIANT}"

STROKE_WIDTH = "1.25"

SVG_NS = 'http://www.w3.org/2000/svg'

ROOT = Path(__file__).parent
MANIFEST = ROOT / 'icons.json'
REPROCESS_DIRS = (ROOT / 'source', ROOT / 'source' / 'custom' / 'symbols')


def fetch_tabler_icon(icon_name):
    import requests
    url = f"{TABLER_BASE_URL}/{icon_name}.svg"
    try:
        response = requests.get(url, timeout=10)
        response.raise_for_status()
        return response.text
    except requests.exceptions.RequestException as e:
        print(f"Error fetching icon '{icon_name}': {e}", file=sys.stderr)
        return None


def strip_invisible(root):
    for parent in [root] + list(root.iter()):
        for child in list(parent):
            if child.get('stroke') == 'none' and child.get('fill') == 'none':
                parent.remove(child)


def apply_kde_theme_colors(svg_content, size=16, stroke_width=None):
    stroke_width = stroke_width or STROKE_WIDTH
    ET.register_namespace('', 'http://www.w3.org/2000/svg')
    root = ET.fromstring(svg_content)

    ns = {'svg': 'http://www.w3.org/2000/svg'}

    strip_invisible(root)

    original_viewbox = root.get('viewBox', '0 0 24 24')
    vb_parts = [float(x) for x in original_viewbox.split()]
    vb_x, vb_y, vb_w, vb_h = vb_parts[0], vb_parts[1], vb_parts[2], vb_parts[3]

    padding = 3
    new_viewbox = f"{vb_x - padding} {vb_y - padding} {vb_w + padding * 2} {vb_h + padding * 2}"

    root.set('width', '16')
    root.set('height', '16')
    root.set('viewBox', new_viewbox)
    root.set('stroke-width', stroke_width)

    if 'stroke' in root.attrib:
        del root.attrib['stroke']
    if 'fill' in root.attrib and root.get('fill') != 'none':
        del root.attrib['fill']
    if 'class' in root.attrib:
        del root.attrib['class']

    defs = root.find('svg:defs', ns)
    if defs is None:
        defs = ET.Element('defs')
        root.insert(0, defs)

    style = ET.SubElement(defs, 'style')
    style.set('id', 'current-color-scheme')
    style.set('type', 'text/css')
    style.text = '.ColorScheme-Text { color: #232629; }'

    for elem in root.iter():
        if elem.tag.endswith('svg') or elem.tag.endswith('defs') or elem.tag.endswith('style'):
            continue

        if 'stroke-width' in elem.attrib:
            elem.set('stroke-width', stroke_width)

        elem.set('class', 'ColorScheme-Text')

        if 'stroke' in elem.attrib:
            if elem.get('stroke') != 'none':
                elem.set('stroke', 'currentColor')
        else:
            elem.set('stroke', 'currentColor')

        if 'fill' in elem.attrib and elem.get('fill') != 'none':
            elem.set('fill', 'currentColor')

        style_attr = elem.get('style', '')
        if style_attr:
            style_parts = [s.strip() for s in style_attr.split(';') if s.strip()]
            new_style_parts = []

            for part in style_parts:
                if ':' in part:
                    key, value = part.split(':', 1)
                    key = key.strip()
                    value = value.strip()

                    if key == 'stroke' and value != 'none':
                        new_style_parts.append('stroke:currentColor')
                    elif key == 'fill' and value != 'none':
                        new_style_parts.append('fill:currentColor')
                    elif key == 'stroke-width':
                        new_style_parts.append(f'stroke-width:{stroke_width}')
                    else:
                        new_style_parts.append(part)
                else:
                    new_style_parts.append(part)

            if new_style_parts:
                elem.set('style', ';'.join(new_style_parts))

    return ET.tostring(root, encoding='unicode', method='xml')


def outline_strokes(svg_content):
    try:
        from picosvg.svg import SVG as PicoSVG
    except ImportError:
        print("Error: picosvg is required (pip install picosvg)", file=sys.stderr)
        sys.exit(1)

    ET.register_namespace('', SVG_NS)
    root = ET.fromstring(svg_content)

    width = root.get('width', '16')
    height = root.get('height', '16')
    viewbox = root.get('viewBox')

    strip_invisible(root)


    inherited = {}
    for attr in ('stroke', 'stroke-width', 'stroke-linecap', 'stroke-linejoin', 'fill'):
        if attr in root.attrib:
            inherited[attr] = root.attrib.pop(attr)

    for defs in root.findall(f'{{{SVG_NS}}}defs'):
        root.remove(defs)

    shape_tags = {'path', 'circle', 'rect', 'ellipse', 'line', 'polyline', 'polygon'}
    has_stroke = False
    for elem in root.iter():
        tag = elem.tag.split('}')[-1]
        if tag not in shape_tags:
            continue
        elem.attrib.pop('class', None)
        elem.attrib.pop('style', None)
        for attr, value in inherited.items():
            if attr not in elem.attrib:
                elem.set(attr, value)
        for attr in ('stroke', 'fill'):
            if elem.get(attr) == 'currentColor':
                elem.set(attr, '#000000')
        if elem.get('stroke', 'none') != 'none':
            has_stroke = True

    if not has_stroke:
        return svg_content

    pico = PicoSVG.fromstring(ET.tostring(root, encoding='unicode')).topicosvg()
    out = ET.fromstring(pico.tostring())
    for empty_defs in out.findall(f'{{{SVG_NS}}}defs'):
        if len(empty_defs) == 0:
            out.remove(empty_defs)
    out.set('width', width)
    out.set('height', height)
    if viewbox:
        out.set('viewBox', viewbox)

    defs = ET.Element('defs')
    style = ET.SubElement(defs, 'style')
    style.set('id', 'current-color-scheme')
    style.set('type', 'text/css')
    style.text = '.ColorScheme-Text { color: #232629; }'
    out.insert(0, defs)

    for elem in out.iter():
        if elem.tag.split('}')[-1] != 'path':
            continue
        elem.set('class', 'ColorScheme-Text')
        elem.set('fill', 'currentColor')
        elem.attrib.pop('stroke', None)

    return ET.tostring(out, encoding='unicode', method='xml')


def reprocess_sources(src_dir):
    converted = 0
    failed = []
    for svg_path in sorted(Path(src_dir).glob('*.svg')):
        content = svg_path.read_text()
        try:
            result = outline_strokes(content)
        except Exception as e:
            failed.append((svg_path.name, e))
            continue
        if result != content:
            svg_path.write_text(result)
            converted += 1
    print(f"Outlined {converted} stroke-based icons in {src_dir}")
    for name, err in failed:
        print(f"  FAILED {name}: {err}", file=sys.stderr)
    return len(failed) == 0


def build_icon(tabler_name, stroke_width=None):
    svg_content = fetch_tabler_icon(tabler_name)
    if not svg_content:
        return None
    return outline_strokes(apply_kde_theme_colors(svg_content,
                                                  stroke_width=stroke_width))


def save_icon(svg_content, target_path):
    path = Path(target_path)
    path.parent.mkdir(parents=True, exist_ok=True)

    svg_path = path.with_suffix('.svg')
    svgz_path = path.with_suffix('.svgz')

    if svg_path.is_symlink() or svg_path.exists():
        svg_path.unlink()
    if svgz_path.is_symlink() or svgz_path.exists():
        svgz_path.unlink()


    with open(svg_path, 'w', encoding='utf-8') as f:
        f.write(svg_content)

    return svg_path


def create_symlink(source, target):
    source_path = Path(source).resolve()
    target_path = Path(target).with_suffix(source_path.suffix)
    target_path.parent.mkdir(parents=True, exist_ok=True)

    for ext in ['.svg', '.svgz']:
        old = target_path.with_suffix(ext)
        if old.exists() or old.is_symlink():
            old.unlink()

    try:
        rel_source = os.path.relpath(source_path, target_path.parent)
        target_path.symlink_to(rel_source)
        print(f"Symlinked: {target_path} -> {rel_source}")
    except Exception as e:
        print(f"Error creating symlink {target}: {e}", file=sys.stderr)


def links_to(target, source):
    target = Path(target)
    return target.is_symlink() and os.path.realpath(target) == str(Path(source).resolve())


def load_manifest():
    try:
        with open(MANIFEST) as f:
            return json.load(f)
    except Exception as e:
        print(f"Error: could not load {MANIFEST}: {e}", file=sys.stderr)
        sys.exit(1)


def save_manifest(manifest):
    with open(MANIFEST, 'w') as f:
        json.dump(manifest, f, indent=2)
        f.write('\n')


def sync(only=None, relink=True):
    manifest = load_manifest()
    entries = manifest['icons']
    if only:
        entries = [e for e in entries if e['name'] in only]
        missing = sorted(set(only) - {e['name'] for e in entries})
        if missing:
            print(f"Error: not in {MANIFEST.name}: {', '.join(missing)}", file=sys.stderr)
            return False

    cache = {}
    failed = []
    for entry in entries:
        name, tabler_name = entry['name'], entry['icon']
        if tabler_name not in cache:
            print(f"Fetching {tabler_name} ...")
            cache[tabler_name] = build_icon(tabler_name)
        svg = cache[tabler_name]
        if not svg:
            failed.append(name)
            continue

        source_path = ROOT / 'source' / f'{name}.svg'
        save_icon(svg, source_path)
        print(f"Created: source/{name}.svg ({tabler_name})")

        if not relink:
            continue


        for target in entry.get('targets', []):
            target = ROOT / target
            if not links_to(target, source_path):
                create_symlink(source_path, target)

    print(f"\nSynced {len(entries) - len(failed)}/{len(entries)} icons "
          f"from tabler {TABLER_VARIANT} {TABLER_VERSION} at stroke-width {STROKE_WIDTH}")
    for name in failed:
        print(f"  FAILED {name}", file=sys.stderr)
    return not failed


def prune():
    manifest = load_manifest()
    for entry in manifest.get('removed', []):
        source_path = ROOT / 'source' / f"{entry['name']}.svg"
        for target in entry.get('targets', []):
            target = ROOT / target
            if target.is_symlink() or target.exists():
                target.unlink()
                print(f"Removed link: {target.relative_to(ROOT)}")
        if source_path.is_symlink() or source_path.exists():
            source_path.unlink()
            print(f"Removed: source/{source_path.name}")
    return True


def main():
    parser = argparse.ArgumentParser(
        description='Tabler Icon Generator for BlossomUI Icon Theme',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  ./icongen.py --sync

  ./icongen.py --sync --only folder-open --only folder

  ./icongen.py wifi actions/16/network-wifi actions/22/network-wifi

  ./icongen.py folder-open --as open-menu-symbolic actions/symbolic/document-open-symbolic

  ./icongen.py --prune
        """
    )

    parser.add_argument('icon_name', nargs='?', help='Tabler outline icon name to fetch')
    parser.add_argument('target_paths', nargs='*', help='Paths for symlinks (source/<name>.svg is always the source)')
    parser.add_argument('--as', dest='as_name', metavar='NAME',
                        help='Save as source/<NAME>.svg instead of source/<icon-name>.svg')
    parser.add_argument('--sync', action='store_true',
                        help=f'Regenerate every icon listed in {MANIFEST.name}')
    parser.add_argument('--only', action='append', metavar='NAME',
                        help='With --sync: limit to these source names (repeatable)')
    parser.add_argument('--no-relink', action='store_true',
                        help='With --sync: only rewrite source files, never touch symlinks')
    parser.add_argument('--prune', action='store_true',
                        help=f'Delete the icons listed under "removed" in {MANIFEST.name}')
    parser.add_argument('--reprocess', action='store_true',
                        help='Convert all existing source/ icons from strokes to filled paths (GTK compatibility)')

    args = parser.parse_args()

    if args.reprocess:
        ok = all([reprocess_sources(d) for d in REPROCESS_DIRS])
        sys.exit(0 if ok else 1)

    if args.prune:
        sys.exit(0 if prune() else 1)

    if args.sync:
        sys.exit(0 if sync(only=args.only, relink=not args.no_relink) else 1)

    if not args.icon_name:
        parser.error('icon_name is required unless --sync, --prune or --reprocess is used')

    name = args.as_name or args.icon_name
    source_path = ROOT / 'source' / f'{name}.svg'

    print(f"Fetching '{args.icon_name}' from tabler {TABLER_VARIANT} {TABLER_VERSION}...")
    svg = build_icon(args.icon_name)
    if not svg:
        sys.exit(1)

    save_icon(svg, source_path)
    print(f"Created: source/{name}.svg")

    targets = []
    for target_path in args.target_paths:
        create_symlink(source_path, ROOT / target_path)
        targets.append(str(Path(target_path).with_suffix('.svg')))

    manifest = load_manifest()
    for entry in manifest['icons']:
        if entry['name'] == name:
            entry['icon'] = args.icon_name
            entry['targets'] = sorted(set(entry.get('targets', [])) | set(targets))
            break
    else:
        manifest['icons'].append({'name': name, 'icon': args.icon_name,
                                  'targets': sorted(targets)})
        manifest['icons'].sort(key=lambda e: e['name'])
    save_manifest(manifest)
    print(f"Updated {MANIFEST.name}")

    print(f"\n✓ Icon '{args.icon_name}' generated successfully!")


if __name__ == '__main__':
    main()
