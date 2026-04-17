#!/usr/bin/env python3

import sys
import os
import gzip
import json
import base64
import readline
import requests
import argparse
import subprocess
import tempfile
from pathlib import Path
from xml.etree import ElementTree as ET


PHOSPHOR_BASE = "https://unpkg.com/@phosphor-icons/core@2.1.1/assets"

WEIGHTS = ['light', 'regular', 'duotone', 'bold', 'thin', 'fill']

KDE_COLOR_SCHEME_CSS = ".ColorScheme-Text {\ncolor:#232629;\n}"


def fetch_phosphor_icon(icon_name, weight='light'):
    suffix = '' if weight == 'regular' else f'-{weight}'
    url = f"{PHOSPHOR_BASE}/{weight}/{icon_name}{suffix}.svg"
    try:
        response = requests.get(url, timeout=10)
        response.raise_for_status()
        return response.text
    except requests.exceptions.RequestException as e:
        print(f"Error fetching '{icon_name}' ({weight}): {e}", file=sys.stderr)
        return None


def apply_kde_theme_colors(svg_content):
    ET.register_namespace('', 'http://www.w3.org/2000/svg')
    root = ET.fromstring(svg_content)

    ns = {'svg': 'http://www.w3.org/2000/svg'}

    original_viewbox = root.get('viewBox', '0 0 256 256')
    vb_parts = [float(x) for x in original_viewbox.split()]
    vb_x, vb_y, vb_w, vb_h = vb_parts[0], vb_parts[1], vb_parts[2], vb_parts[3]

    padding = round(vb_w * 0.125)
    new_viewbox = f"{vb_x - padding} {vb_y - padding} {vb_w + padding * 2} {vb_h + padding * 2}"

    root.set('width', '16')
    root.set('height', '16')
    root.set('viewBox', new_viewbox)
    root.attrib.pop('class', None)
    root.attrib.pop('fill', None)
    root.attrib.pop('style', None)

    defs = root.find('svg:defs', ns)
    if defs is None:
        defs = ET.Element('defs')
        root.insert(0, defs)

    style = ET.SubElement(defs, 'style')
    style.set('id', 'current-color-scheme')
    style.set('type', 'text/css')
    style.text = KDE_COLOR_SCHEME_CSS

    for elem in root.iter():
        tag = elem.tag.split('}')[-1] if '}' in elem.tag else elem.tag
        if tag in ('svg', 'defs', 'style'):
            continue

        if 'stroke-width' in elem.attrib:
            try:
                orig_sw = float(elem.get('stroke-width'))
                scaled_sw = round((orig_sw / vb_w) * 24 * 1.5, 2)
                elem.set('stroke-width', str(scaled_sw))
            except ValueError:
                pass

        elem.set('class', 'ColorScheme-Text')

        stroke = elem.get('stroke')
        if stroke is not None and stroke != 'none':
            elem.set('stroke', 'currentColor')

        fill = elem.get('fill')
        if fill is not None and fill != 'none':
            elem.set('fill', 'currentColor')
        elif fill is None and elem.get('stroke') is None:
            elem.set('fill', 'currentColor')

        style_attr = elem.get('style', '')
        if style_attr:
            new_parts = []
            for part in (s.strip() for s in style_attr.split(';') if s.strip()):
                if ':' in part:
                    key, value = part.split(':', 1)
                    key, value = key.strip(), value.strip()
                    if key == 'stroke' and value != 'none':
                        new_parts.append('stroke:currentColor')
                    elif key == 'fill' and value != 'none':
                        new_parts.append('fill:currentColor')
                    elif key == 'stroke-width':
                        try:
                            orig_sw = float(value)
                            scaled_sw = round((orig_sw / vb_w) * 24 * 1.5, 2)
                            new_parts.append(f'stroke-width:{scaled_sw}')
                        except ValueError:
                            new_parts.append(part)
                    else:
                        new_parts.append(part)
                else:
                    new_parts.append(part)
            if new_parts:
                elem.set('style', ';'.join(new_parts))

    return ET.tostring(root, encoding='unicode', method='xml')


def save_icon(svg_content, target_path):
    path = Path(target_path)
    path.parent.mkdir(parents=True, exist_ok=True)

    svgz_path = path.with_suffix('.svgz')

    for ext in ('.svg', '.svgz'):
        old = path.with_suffix(ext)
        if old.is_symlink() or old.exists():
            old.unlink()

    with gzip.open(svgz_path, 'wt', encoding='utf-8') as f:
        f.write(svg_content)

    print(f"  Saved: {svgz_path}")
    return svgz_path


def create_symlink(source, target):
    source_path = Path(source).resolve()
    target_path = Path(target).with_suffix(source_path.suffix)
    target_path.parent.mkdir(parents=True, exist_ok=True)

    for ext in ('.svg', '.svgz'):
        old = target_path.with_suffix(ext)
        if old.exists() or old.is_symlink():
            old.unlink()

    try:
        rel_source = os.path.relpath(source_path, target_path.parent)
        target_path.symlink_to(rel_source)
        print(f"  Linked: {target_path} -> {rel_source}")
    except Exception as e:
        print(f"  Error creating symlink {target}: {e}", file=sys.stderr)


BREEZE_SEARCH_DIRS = [
    Path('/usr/share/icons/breeze'),
    Path('/usr/share/icons/breeze-dark'),
    Path('/usr/share/icons/hicolor'),
]

_PREFERRED_SIZES = ['22', '24', '32', '48', '16']

def find_breeze_icon(stem):
    for base in BREEZE_SEARCH_DIRS:
        if not base.exists():
            continue
        candidates = list(base.rglob(f'{stem}.svg'))
        if not candidates:
            continue
        for size in _PREFERRED_SIZES:
            for c in candidates:
                if f'/{size}/' in str(c) or f'/{size}x{size}/' in str(c):
                    return c
        return candidates[0]
    return None


def _svg_to_png_bytes(svg_path):
    tmp_fd, tmp_png = tempfile.mkstemp(suffix='.png')
    os.close(tmp_fd)
    try:
        for cmd in (['magick'], ['convert']):
            r = subprocess.run(
                cmd + ['-background', 'white', '-density', '384',
                       str(svg_path), '-resize', '128x128', '-flatten', tmp_png],
                capture_output=True, timeout=10
            )
            if r.returncode == 0 and os.path.getsize(tmp_png) > 0:
                with open(tmp_png, 'rb') as f:
                    return f.read()
        return None
    except Exception:
        return None
    finally:
        try:
            os.unlink(tmp_png)
        except Exception:
            pass


def preview_svg(svg_path, label=None):
    try:
        data = _svg_to_png_bytes(svg_path)
        if not data:
            return
        if label:
            print(f"  {label}")
        _kitty_display(data)
    except Exception:
        pass


def preview_svgz(svgz_path, label=None):
    try:
        with gzip.open(svgz_path, 'rt') as f:
            svg = f.read()
        with tempfile.NamedTemporaryFile(suffix='.svg', mode='w', delete=False) as f:
            f.write(svg)
            tmp_svg = f.name
        preview_svg(Path(tmp_svg), label)
    except Exception:
        pass
    finally:
        try:
            os.unlink(tmp_svg)
        except Exception:
            pass


def _kitty_display(data: bytes):
    b64 = base64.standard_b64encode(data).decode()
    chunks = [b64[i:i + 4096] for i in range(0, len(b64), 4096)]
    for i, chunk in enumerate(chunks):
        m = 0 if i == len(chunks) - 1 else 1
        params = f"a=T,f=100,m={m}" if i == 0 else f"m={m}"
        sys.stdout.buffer.write(f"\033_G{params};{chunk}\033\\".encode())
    sys.stdout.buffer.flush()
    print()


def input_prefilled(prompt, prefill='', completions=None):
    if completions:
        sorted_completions = sorted(completions)
        def completer(text, state):
            matches = [c for c in sorted_completions if c.startswith(text)]
            return matches[state] if state < len(matches) else None
        readline.set_completer(completer)
        readline.parse_and_bind('tab: complete')
    readline.set_startup_hook(lambda: readline.insert_text(prefill))
    try:
        return input(prompt)
    finally:
        readline.set_startup_hook(None)
        readline.set_completer(None)


def fetch_valid_phosphor_names():
    print("Fetching Phosphor icon list...", end=' ', flush=True)
    r = requests.get('https://unpkg.com/@phosphor-icons/core@2.1.1/?meta', timeout=30)
    files = r.json()['files']
    valid = {}
    for f in files:
        path = f['path']
        for weight in WEIGHTS:
            prefix = f'/assets/{weight}/'
            if path.startswith(prefix):
                filename = path[len(prefix):]
                suffix = f'-{weight}' if weight != 'regular' else ''
                name = filename[:-len('.svg')]
                if suffix and name.endswith(suffix):
                    name = name[:-len(suffix)]
                valid.setdefault(weight, set()).add(name)
    print(f"{len(valid.get('light', set()))} light icons loaded.")
    return valid


def prompt_weight():
    opts = '/'.join(WEIGHTS)
    while True:
        raw = input(f"  weight [{opts}] (Enter = light): ").strip().lower()
        if raw == '':
            return 'light'
        if raw in WEIGHTS:
            return raw
        print(f"  Unknown weight. Choose from: {opts}")


def load_mapping(mapping_file):
    try:
        with open(mapping_file, 'r') as f:
            return json.load(f)
    except Exception as e:
        print(f"Error: Could not load mapping file: {e}", file=sys.stderr)
        sys.exit(1)


def save_mapping(mapping, mapping_file):
    try:
        with open(mapping_file, 'w') as f:
            json.dump(mapping, f, indent=2)
    except Exception as e:
        print(f"Warning: Could not save mapping file: {e}", file=sys.stderr)


def interactive_session(mapping, mapping_file):
    valid_names = fetch_valid_phosphor_names()

    def is_valid(name, weight):
        return name in valid_names.get(weight, set())

    seen = set()
    candidates = []
    for e in mapping['icons']:
        sf = e['source_file']
        if sf in seen:
            continue
        weight = e.get('weight', 'light')
        name_valid = is_valid(e.get('phosphor_name', ''), weight)
        source_exists = Path(e['source_file']).exists()
        name_missing = not name_valid
        needs_work = name_missing or (not e.get('links_only') and not source_exists)
        if needs_work:
            candidates.append((e, name_missing))
            seen.add(sf)

    if not candidates:
        print("All icons are mapped and valid.")
        return

    total = len(candidates)
    done = 0
    skipped = 0

    print(f"\n{total} entries to review.")
    print("  Enter / Tab  | fetch using the pre-filled name (Tab to autocomplete)")
    print("  s            | skip")
    print("  l            | recreate links from existing source (no fetch)")
    print("  w            | save mapping now")
    print("  q            | quit and save\n")

    for i, (entry, name_missing) in enumerate(candidates, 1):
        source_file = entry['source_file']
        stem = Path(source_file).stem
        current_phosphor = entry.get('phosphor_name', stem)
        weight = entry.get('weight', 'light')
        targets = entry.get('target_paths', [])
        source_exists = Path(source_file).exists()
        completions = valid_names.get(weight, set())

        tag = '404' if name_missing else 'links_only'
        status = '[exists]' if source_exists else '[missing]'
        print(f"[{i}/{total}] {stem}  ({tag}, {status})")
        if targets:
            more = f" +{len(targets)-1} more" if len(targets) > 1 else ""
            print(f"  links ({len(targets)}): {targets[0]}{more}")

        breeze = find_breeze_icon(stem)
        if breeze:
            preview_svg(breeze, label=f'Breeze: {breeze.relative_to(Path("/usr/share/icons"))}')
        else:
            print("  Breeze: (no icon found)")

        prefill = current_phosphor if is_valid(current_phosphor, weight) else ''

        while True:
            try:
                raw = input_prefilled("  > ", prefill, completions).strip()
            except (EOFError, KeyboardInterrupt):
                print("\nInterrupted — saving progress.")
                save_mapping(mapping, mapping_file)
                return

            if raw == 'q':
                print("Quitting — saving progress.")
                save_mapping(mapping, mapping_file)
                return

            if raw == 'w':
                save_mapping(mapping, mapping_file)
                print(f"  Saved {mapping_file}")
                continue

            if raw == 's':
                skipped += 1
                break

            if raw == 'l':
                if not source_exists:
                    print(f"  No source file at {source_file}.")
                    continue
                for target in targets:
                    create_symlink(source_file, target)
                done += 1
                break

            if not is_valid(raw, weight):
                print(f"  '{raw}' not found in Phosphor ({weight}). Tab to autocomplete, or 's' to skip.")
                prefill = raw
                continue

            weight = prompt_weight() if not entry.get('weight') else weight

            print(f"Fetching '{raw}' ({weight})...")
            svg_content = fetch_phosphor_icon(raw, weight)
            if not svg_content:
                print("Fetch failed unexpectedly. Try again.")
                continue

            themed_svg = apply_kde_theme_colors(svg_content)
            saved = save_icon(themed_svg, source_file)
            preview_svgz(saved, label='Phosphor:')

            for target in targets:
                create_symlink(source_file, target)

            entry['phosphor_name'] = raw
            entry['weight'] = weight
            entry['links_only'] = False
            save_mapping(mapping, mapping_file)
            done += 1
            break

    print(f"\nSession complete: {done} processed, {skipped} skipped")


def process_entry(entry, links_only_override=False, skip_existing=False):
    source_file = entry['source_file']
    phosphor_name = entry['phosphor_name']
    weight = entry.get('weight', 'light')
    links_only = links_only_override or entry.get('links_only', False)
    target_paths = entry.get('target_paths', [])

    if skip_existing and Path(source_file).exists():
        print(f"\n[skip] {source_file}")
        return True

    print(f"\n[{phosphor_name}/{weight}] -> {source_file}")

    if not links_only:
        print(f"Fetching '{phosphor_name}' ({weight})...")
        svg_content = fetch_phosphor_icon(phosphor_name, weight)
        if not svg_content:
            return False

        themed_svg = apply_kde_theme_colors(svg_content)
        save_icon(themed_svg, source_file)
    else:
        source_path = Path(source_file)
        if not source_path.exists():
            print(f"  Warning: source missing, skipping: {source_file}", file=sys.stderr)
            return False
        print(f"  links-only: {source_file}")

    for target in target_paths:
        create_symlink(source_file, target)

    return True


def main():
    parser = argparse.ArgumentParser(
        description='Phosphor Icon Generator (light) for BlossomUI Icon Theme',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python icongen.py magnifying-glass actions/symbolic/search-symbolic.svgz
  python icongen.py magnifying-glass --source source/edit-find-mail.svgz actions/symbolic/edit-find.svgz
  python icongen.py magnifying-glass --weight duotone actions/symbolic/search-symbolic.svgz
  python icongen.py -c source/magnifying-glass.svgz actions/symbolic/search2.svgz
  python icongen.py --map=icon_mapping.json --from-map alarm-symbolic
  python icongen.py --generate-all --map=icon_mapping.json
  python icongen.py --interactive --map=icon_mapping.json
        """
    )

    parser.add_argument('icon_name', nargs='?', help='Phosphor icon name (or source path for -c)')
    parser.add_argument('target_paths', nargs='*', help='Paths for symlinks')
    parser.add_argument('-c', '--links-only', action='store_true',
                        help='Skip fetching, create symlinks from existing source file')
    parser.add_argument('--weight', choices=WEIGHTS,
                        help='Phosphor icon weight (default: prompt)')
    parser.add_argument('--source', metavar='FILE',
                        help='Override source file path (default: source/<icon_name>.svgz)')
    parser.add_argument('--map', metavar='FILE', help='Icon mapping JSON file')
    parser.add_argument('--from-map', action='store_true',
                        help='Load source/targets from mapping file for icon_name (requires --map)')
    parser.add_argument('--generate-all', action='store_true',
                        help='Process all entries in the mapping file (requires --map)')
    parser.add_argument('--skip-existing', action='store_true',
                        help='Skip entries whose source file already exists')
    parser.add_argument('--interactive', action='store_true',
                        help='Interactively assign Phosphor names to links_only entries (requires --map)')

    args = parser.parse_args()

    if args.interactive:
        if not args.map:
            print("Error: --interactive requires --map=<file>", file=sys.stderr)
            sys.exit(1)
        mapping = load_mapping(args.map)
        interactive_session(mapping, args.map)
        return

    if args.generate_all:
        if not args.map:
            print("Error: --generate-all requires --map=<file>", file=sys.stderr)
            sys.exit(1)
        mapping = load_mapping(args.map)
        total = len(mapping['icons'])
        ok = 0
        for i, entry in enumerate(mapping['icons'], 1):
            print(f"[{i}/{total}]", end="")
            if process_entry(entry, skip_existing=args.skip_existing):
                ok += 1
        print(f"\n✓ Done: {ok}/{total} icons generated successfully")
        return

    if args.from_map:
        if not args.map:
            print("Error: --from-map requires --map=<file>", file=sys.stderr)
            sys.exit(1)
        if not args.icon_name:
            print("Error: --from-map requires an icon name", file=sys.stderr)
            sys.exit(1)
        mapping = load_mapping(args.map)
        entry = None
        for e in mapping['icons']:
            stem = Path(e['source_file']).stem
            if stem == args.icon_name or e['phosphor_name'] == args.icon_name:
                entry = e
                break
        if not entry:
            print(f"Error: '{args.icon_name}' not found in mapping file", file=sys.stderr)
            sys.exit(1)
        process_entry(entry)
        return

    if args.links_only:
        if not args.icon_name:
            print("Error: -c requires a source file path", file=sys.stderr)
            sys.exit(1)
        source_path = Path(args.icon_name)
        if not source_path.exists():
            print(f"Error: source file does not exist: {source_path}", file=sys.stderr)
            sys.exit(1)
        print(f"links-only: {source_path}")
        for target in args.target_paths:
            create_symlink(str(source_path), target)
        print(f"\n✓ Created {len(args.target_paths)} symlink(s)")
        return

    if not args.icon_name:
        parser.print_help()
        sys.exit(1)

    weight = args.weight if args.weight else prompt_weight()
    source_path = args.source if args.source else f"source/{args.icon_name}.svgz"

    print(f"Fetching '{args.icon_name}' ({weight}) from Phosphor...")
    svg_content = fetch_phosphor_icon(args.icon_name, weight)
    if not svg_content:
        sys.exit(1)

    print("Applying KDE theme colors...")
    themed_svg = apply_kde_theme_colors(svg_content)
    saved = save_icon(themed_svg, source_path)
    preview_svgz(saved)

    for target in args.target_paths:
        create_symlink(source_path, target)

    if args.map:
        mapping = load_mapping(args.map)
        entry = None
        for e in mapping['icons']:
            if e['source_file'] == source_path:
                entry = e
                break
        if entry:
            existing = set(entry.get('target_paths', []))
            existing.update(args.target_paths)
            entry['target_paths'] = sorted(existing)
            entry['weight'] = weight
        else:
            mapping['icons'].append({
                'source_file': source_path,
                'phosphor_name': args.icon_name,
                'weight': weight,
                'links_only': False,
                'target_paths': sorted(args.target_paths),
            })
            mapping['icons'].sort(key=lambda x: x['source_file'])
        save_mapping(mapping, args.map)
        print(f"  Updated: {args.map}")

    print(f"\n✓ '{args.icon_name}' ({weight}) generated successfully!")


if __name__ == '__main__':
    main()
