#!/usr/bin/env python3
"""
Lucide Icon Generator for BlossomUI Icon Theme
Fetches icons from Lucide icon library and applies KDE theme text color.

Usage:
    python icongen.py <icon-name> [additional/symlink/path...]

The source icon is always saved to source/<icon-name>.svg.
All target paths become symlinks pointing to the source file.

Example:
    python icongen.py wifi actions/16/network-wifi actions/22/network-wifi
"""

import sys
import os
import gzip
import json
import argparse
from pathlib import Path
from xml.etree import ElementTree as ET


LUCIDE_BASE_URL = "https://cdn.jsdelivr.net/npm/lucide-static@latest/icons"

SVG_NS = 'http://www.w3.org/2000/svg'


def fetch_lucide_icon(icon_name):
    import requests
    url = f"{LUCIDE_BASE_URL}/{icon_name}.svg"
    try:
        response = requests.get(url, timeout=10)
        response.raise_for_status()
        return response.text
    except requests.exceptions.RequestException as e:
        print(f"Error fetching icon '{icon_name}': {e}", file=sys.stderr)
        return None


def apply_kde_theme_colors(svg_content, size=16):
    ET.register_namespace('', 'http://www.w3.org/2000/svg')
    root = ET.fromstring(svg_content)
    
    ns = {'svg': 'http://www.w3.org/2000/svg'}
    
    original_viewbox = root.get('viewBox', '0 0 24 24')
    vb_parts = [float(x) for x in original_viewbox.split()]
    vb_x, vb_y, vb_w, vb_h = vb_parts[0], vb_parts[1], vb_parts[2], vb_parts[3]
    
    padding = 3
    new_viewbox = f"{vb_x - padding} {vb_y - padding} {vb_w + padding * 2} {vb_h + padding * 2}"
    
    root.set('width', '16')
    root.set('height', '16')
    root.set('viewBox', new_viewbox)
    root.set('stroke-width', '1.5')
    
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
            elem.set('stroke-width', '1.5')
        
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
                        new_style_parts.append('stroke-width:1.5')
                    else:
                        new_style_parts.append(part)
                else:
                    new_style_parts.append(part)
            
            if new_style_parts:
                elem.set('style', ';'.join(new_style_parts))
    
    return ET.tostring(root, encoding='unicode', method='xml')


def outline_strokes(svg_content):
    """Convert stroked shapes to filled paths.

    GTK recolors symbolic icons by force-filling every shape (fill: <fg>
    !important on rect/circle/path/...), which turns stroke-based Lucide icons
    into solid blobs. KDE recolors via the ColorScheme-Text class, which works
    for fills exactly like breeze, so outlined icons render right everywhere.
    """
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

    # picosvg does not inherit presentation attributes from the root element,
    # so push them down onto each shape before converting
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


def save_icon(svg_content, target_path):
    path = Path(target_path)
    path.parent.mkdir(parents=True, exist_ok=True)

    svg_path = path.with_suffix('.svg')
    svgz_path = path.with_suffix('.svgz')

    if svg_path.is_symlink() or svg_path.exists():
        svg_path.unlink()
    if svgz_path.is_symlink() or svgz_path.exists():
        svgz_path.unlink()

    # plain svg: GTK's icon lookup only accepts .png/.svg/.xpm, .svgz is a
    # KDE-only extension and makes icons invisible to GTK apps
    with open(svg_path, 'w', encoding='utf-8') as f:
        f.write(svg_content)

    print(f"Created: {svg_path}")


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


def load_mapping(mapping_file):
    """Load icon mapping from JSON file."""
    try:
        with open(mapping_file, 'r') as f:
            return json.load(f)
    except Exception as e:
        print(f"Warning: Could not load mapping file: {e}", file=sys.stderr)
        return {'icons': []}


def save_mapping(mapping, mapping_file):
    """Save icon mapping to JSON file."""
    try:
        with open(mapping_file, 'w') as f:
            json.dump(mapping, f, indent=2)
    except Exception as e:
        print(f"Warning: Could not save mapping file: {e}", file=sys.stderr)


def update_mapping(mapping, source_file, target_paths):
    """Update mapping with new source file and target paths."""
    source_file = str(source_file)

    # Find existing entry
    for icon in mapping['icons']:
        if icon['source_file'] == source_file:
            # Update target paths (merge with existing)
            existing = set(icon.get('target_paths', []))
            existing.update(target_paths)
            icon['target_paths'] = sorted(existing)
            return mapping

    # Create new entry
    mapping['icons'].append({
        'source_file': source_file,
        'target_paths': sorted(target_paths)
    })

    # Sort by source file
    mapping['icons'].sort(key=lambda x: x['source_file'])

    return mapping


def main():
    parser = argparse.ArgumentParser(
        description='Lucide Icon Generator for BlossomUI Icon Theme',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Basic usage (source saved to source/wifi.svgz automatically)
  python icongen.py wifi actions/16/network-wifi actions/22/network-wifi

  # With mapping file
  python icongen.py --map=icon_mapping.json wifi actions/16/network-wifi

  # Use existing mapping (fetch from mapping file)
  python icongen.py --map=icon_mapping.json --from-map wifi
        """
    )

    parser.add_argument('icon_name', nargs='?', help='Lucide icon name to fetch')
    parser.add_argument('target_paths', nargs='*', help='Paths for symlinks (source/<icon-name>.svg is always the source)')
    parser.add_argument('--map', metavar='FILE', help='Icon mapping JSON file to update')
    parser.add_argument('--from-map', action='store_true',
                        help='Use source and target paths from mapping file (requires --map)')
    parser.add_argument('--reprocess', action='store_true',
                        help='Convert all existing source/ icons from strokes to filled paths (GTK compatibility)')

    args = parser.parse_args()

    if args.reprocess:
        src = Path(__file__).parent / 'source'
        sys.exit(0 if reprocess_sources(src) else 1)

    if not args.icon_name:
        parser.error('icon_name is required unless --reprocess is used')

    # Handle --from-map mode
    if args.from_map:
        if not args.map:
            print("Error: --from-map requires --map=<file>", file=sys.stderr)
            sys.exit(1)

        mapping = load_mapping(args.map)

        # Find icon in mapping
        icon_entry = None
        for icon in mapping['icons']:
            source_name = Path(icon['source_file']).stem
            if source_name == args.icon_name or icon['source_file'] == f"source/{args.icon_name}.svgz":
                icon_entry = icon
                break

        if not icon_entry:
            print(f"Error: Icon '{args.icon_name}' not found in mapping file", file=sys.stderr)
            sys.exit(1)

        source_path = icon_entry['source_file']
        target_paths = icon_entry.get('target_paths', [])

        print(f"Using mapping from {args.map}:")
        print(f"  Source: {source_path}")
        print(f"  Targets: {len(target_paths)} paths")

    else:
        source_path = f"source/{args.icon_name}.svgz"
        target_paths = args.target_paths

    # Fetch and process icon
    print(f"Fetching '{args.icon_name}' from Lucide...")
    svg_content = fetch_lucide_icon(args.icon_name)

    if not svg_content:
        sys.exit(1)

    print("Applying KDE theme colors...")
    themed_svg = apply_kde_theme_colors(svg_content)

    print("Outlining strokes for GTK compatibility...")
    themed_svg = outline_strokes(themed_svg)

    # Save source icon
    save_icon(themed_svg, source_path)

    # Create symlinks
    for target_path in target_paths:
        create_symlink(source_path, target_path)

    # Update mapping file if specified
    if args.map:
        mapping = load_mapping(args.map)
        mapping = update_mapping(mapping, source_path, target_paths)
        save_mapping(mapping, args.map)
        print(f"Updated mapping file: {args.map}")

    print(f"\n✓ Icon '{args.icon_name}' generated successfully!")


if __name__ == '__main__':
    main()
