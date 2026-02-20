#!/usr/bin/env python3
"""
Lucide Icon Generator for BlossomUI Icon Theme
Fetches icons from Lucide icon library and applies KDE theme text color.

Usage:
    python icongen.py <icon-name> <path/to/original> [additional/symlink/path...]

Example:
    python icongen.py wifi apps/16/wifi.svg apps/22/wifi.svg
"""

import sys
import os
import gzip
import json
import requests
import argparse
from pathlib import Path
from xml.etree import ElementTree as ET


LUCIDE_BASE_URL = "https://cdn.jsdelivr.net/npm/lucide-static@latest/icons"


def fetch_lucide_icon(icon_name):
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
    
    padding = 1
    new_viewbox = f"{vb_x - padding} {vb_y - padding} {vb_w + padding * 2} {vb_h + padding * 2}"
    
    root.set('width', '16')
    root.set('height', '16')
    root.set('viewBox', new_viewbox)
    
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


def save_icon(svg_content, target_path):
    path = Path(target_path)
    path.parent.mkdir(parents=True, exist_ok=True)
    
    svg_path = path.with_suffix('.svg')
    svgz_path = path.with_suffix('.svgz')
    
    if svg_path.is_symlink() or svg_path.exists():
        svg_path.unlink()
    if svgz_path.is_symlink() or svgz_path.exists():
        svgz_path.unlink()
    
    with gzip.open(svgz_path, 'wt', encoding='utf-8') as f:
        f.write(svg_content)
    
    print(f"Created: {svgz_path}")


def create_symlink(source, target):
    import shutil
    
    source_path = Path(source).with_suffix('.svgz')
    target_path = Path(target)
    
    # If target is in a sized directory (16/, 22/, 24/, 32/), copy instead of symlink
    if any(f'/{size}/' in str(target_path) for size in ['16', '22', '24', '32', '48', '64']):
        target_path.parent.mkdir(parents=True, exist_ok=True)
        
        # Remove old files
        for ext in ['.svg', '.svgz']:
            old_file = target_path.with_suffix(ext)
            if old_file.exists() or old_file.is_symlink():
                old_file.unlink()
        
        # Copy the svgz file
        target_svgz = target_path.with_suffix('.svgz')
        shutil.copy2(source_path, target_svgz)
        print(f"Copied: {target_svgz}")
        return
    
    # For symbolic/ and other directories, use symlinks as before
    source_path = Path(source).resolve()
    # Ensure target extension matches source extension (.svgz -> .svgz)
    target_path = target_path.with_suffix(source_path.suffix)
    target_path.parent.mkdir(parents=True, exist_ok=True)

    if target_path.exists() or target_path.is_symlink():
        target_path.unlink()

    try:
        rel_source = os.path.relpath(source_path, target_path.parent)
        target_path.symlink_to(rel_source)
        print(f"Symlinked: {target_path} -> {source}")
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
  # Basic usage
  python icongen.py wifi source/wifi.svgz actions/16/network-wifi.svgz

  # With mapping file
  python icongen.py --map=icon_mapping.json wifi source/wifi.svgz actions/16/network-wifi.svgz

  # Use existing mapping (fetch from mapping file)
  python icongen.py --map=icon_mapping.json --from-map wifi
        """
    )

    parser.add_argument('icon_name', help='Lucide icon name to fetch')
    parser.add_argument('source_path', nargs='?', help='Path to save the source icon (e.g., source/wifi.svgz)')
    parser.add_argument('target_paths', nargs='*', help='Additional paths for symlinks')
    parser.add_argument('--map', metavar='FILE', help='Icon mapping JSON file to update')
    parser.add_argument('--from-map', action='store_true',
                        help='Use source and target paths from mapping file (requires --map)')

    args = parser.parse_args()

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
        if not args.source_path:
            print("Error: source_path is required (or use --from-map)", file=sys.stderr)
            parser.print_help()
            sys.exit(1)

        source_path = args.source_path
        target_paths = args.target_paths

    # Fetch and process icon
    print(f"Fetching '{args.icon_name}' from Lucide...")
    svg_content = fetch_lucide_icon(args.icon_name)

    if not svg_content:
        sys.exit(1)

    print("Applying KDE theme colors...")
    themed_svg = apply_kde_theme_colors(svg_content)

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
