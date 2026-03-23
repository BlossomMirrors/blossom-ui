#!/usr/bin/env python3
import json
import os
from pathlib import Path
from collections import defaultdict

ICONS_DIR = Path(__file__).parent

PHOSPHOR_NAMES = {
    "ac-adapter-symbolic":                   ("battery-charging-vertical", True),
    "alarm-symbolic":                        ("bell", False),
    "application-menu":                      ("list", False),
    "applications-education":               ("books", False),
    "applications-games":                   ("game-controller", False),
    "applications-graphics":                ("paint-brush", False),
    "applications-multimedia":              ("film-strip", False),
    "applications-office":                  ("briefcase", False),
    "applications-other":                   ("squares-four", False),
    "applications-science":                 ("flask", False),
    "applications-utilities":               ("wrench", False),
    "appointment-symbolic":                 ("calendar", False),
    "arrow-down":                           ("arrow-down", False),
    "arrow-up":                             ("arrow-up", False),
    "audio-headphones-symbolic":            ("headphones", False),
    "audio-speakers-symbolic":              ("speaker-high", False),
    "audio-volume-low-symbolic":            ("speaker-low", False),
    "audio-volume-medium-symbolic":         ("speaker-simple-medium", True),
    "audio-volume-muted-symbolic":          ("speaker-x", False),
    "avatar-default-symbolic":              ("user", False),
    "battery-000-charging":                 ("battery-charging", False),
    "battery-010":                          ("battery-empty", False),
    "battery-caution-symbolic":             ("battery-warning", False),
    "battery-full-symbolic":                ("battery-full", False),
    "battery-low-symbolic":                 ("battery-low", False),
    "bluetooth-offline":                    ("bluetooth-slash", False),
    "bluetooth-paired":                     ("bluetooth-connected", False),
    "bluetooth":                            ("bluetooth", False),
    "bolt-symbolic":                        ("lightning", False),
    "bookmarks-symbolic":                   ("bookmarks", False),
    "bookmark":                             ("bookmark", False),
    "book-search":                          ("book-open", True),
    "book":                                 ("book-open", False),
    "brightness-low-symbolic":              ("sun-dim", False),
    "brightness-medium-symbolic":           ("sun", False),
    "bug":                                  ("bug", False),
    "calendar-go-today-symbolic":           ("calendar-check", False),
    "call-start-symbolic":                  ("phone-call", False),
    "call-stop-symbolic":                   ("phone-x", False),
    "camera-symbolic":                      ("camera", False),
    "cancel":                               ("x-circle", False),
    "circle-question-mark":                 ("question", False),
    "close":                                ("x", False),
    "cloud-download":                       ("cloud-arrow-down", False),
    "compass":                              ("compass", False),
    "computer-symbolic":                    ("monitor", False),
    "configure":                            ("sliders-horizontal", False),
    "copy":                                 ("copy", False),
    "cpu-symbolic":                         ("cpu", False),
    "daytime-sunset-symbolic":              ("sun-horizon", False),
    "delete":                               ("trash", False),
    "dialog-cancel-symbolic":               ("x-circle", True),
    "dialog-password-symbolic":             ("password", False),
    "dialog-warning-symbolic":              ("warning", False),
    "documentation-symbolic":               ("file-text", False),
    "document-edit-symbolic":               ("note-pencil", False),
    "document-export-symbolic":             ("export", False),
    "document-properties-symbolic":         ("file-magnifying-glass", False),
    "download-symbolic":                    ("download-simple", False),
    "draw-arrow-back":                      ("arrow-left", False),
    "draw-arrow-down":                      ("arrow-down-left", False),
    "draw-arrow-forward":                   ("arrow-right", False),
    "draw-arrow":                           ("arrow-right", True),
    "draw-arrow-up":                        ("arrow-up-right", False),
    "drive-harddisk":                       ("hard-drives", False),
    "drive-harddisk-symbolic":              ("hard-drive", False),
    "edit-copy-symbolic":                   ("copy", True),
    "edit-cut-symbolic":                    ("scissors", False),
    "edit-delete-symbolic":                 ("trash", True),
    "edit-find-mail":                       ("magnifying-glass", False),
    "edit-paste":                           ("clipboard", False),
    "edit-redo-symbolic":                   ("arrow-clockwise", False),
    "edit-select-all-symbolic":             ("selection-all", False),
    "edit-select-symbolic":                 ("selection", False),
    "edit-symbolic":                        ("pencil-simple", False),
    "edit-undo-symbolic":                   ("arrow-counter-clockwise", False),
    "emblem-shared-symbolic":               ("share-network", False),
    "emoji-flags-symbolic":                 ("flag", False),
    "error-symbolic":                       ("x-circle", True),
    "eye-off":                              ("eye-slash", False),
    "file-down":                            ("file-arrow-down", False),
    "file-up":                              ("file-arrow-up", False),
    "firewall-applet-shields_up-symbolic":  ("shield-check", False),
    "fm-details":                           ("list-bullets", False),
    "fm-icons":                             ("grid-four", False),
    "folder-development":                   ("folder-simple", True),
    "folder-documents":                     ("folder-simple", True),
    "folder-documents-symbolic":            ("folder-simple", True),
    "folder-downloads":                     ("folder-simple-arrow-down", False),
    "folder-downloads-symbolic":            ("folder-simple-arrow-down", True),
    "folder-drag-accept-symbolic":          ("folder-simple-plus", True),
    "folder-music":                         ("folder-simple-music", False),
    "folder-music-symbolic":                ("folder-simple-music", True),
    "folder-network":                       ("folder-simple", True),
    "folder-new-symbolic":                  ("folder-simple-plus", True),
    "folder-open":                          ("folder-open", False),
    "folder-open-symbolic":                 ("folder-open", True),
    "folder-pictures":                      ("folder-simple-image", False),
    "folder-pictures-symbolic":             ("folder-simple-image", True),
    "folder-public":                        ("folder-simple-user", False),
    "folder-public-symbolic":               ("folder-simple-user", True),
    "folders":                              ("folders", False),
    "folder":                               ("folder-simple", False),
    "folder-symbolic":                      ("folder-simple", True),
    "folder-templates":                     ("folder-simple-star", False),
    "folder-templates-symbolic":            ("folder-simple-star", True),
    "folder-up":                            ("folder-simple-arrow-up", False),
    "folder-videos":                        ("folder-simple-video", False),
    "folder-videos-symbolic":               ("folder-simple-video", True),
    "folder-visiting-symbolic":             ("folder-simple", True),
    "format-justify-center-symbolic":       ("text-align-center", False),
    "format-justify-fill-symbolic":         ("text-align-justify", False),
    "format-justify-left-symbolic":         ("text-align-left", False),
    "format-justify-right-symbolic":        ("text-align-right", False),
    "format-text-bold-symbolic":            ("text-b", False),
    "format-text-italic-symbolic":          ("text-italic", False),
    "format-text-strikethrough-symbolic":   ("text-strikethrough", False),
    "format-text-underline-symbolic":       ("text-underline", False),
    "globe-symbolic":                       ("globe", False),
    "go-down-symbolic":                     ("arrow-down", True),
    "go-first-symbolic":                    ("skip-back", False),
    "go-home-symbolic":                     ("house", False),
    "go-jump-symbolic":                     ("arrow-bend-right-down", False),
    "go-last-symbolic":                     ("skip-forward", False),
    "go-next-symbolic":                     ("arrow-right", True),
    "go-previous-symbolic":                 ("arrow-left", True),
    "go-up-symbolic":                       ("arrow-up", True),
    "help-about-symbolic":                  ("info", False),
    "help-contents-symbolic":               ("book-open", True),
    "help-faq-symbolic":                    ("chat-circle-dots", False),
    "highlighter":                          ("highlighter", False),
    "image-crop-symbolic":                  ("crop", False),
    "image-up":                             ("image", True),
    "input-mouse-symbolic":                 ("mouse-simple", False),
    "insert-image-symbolic":                ("image", False),
    "insert-text-symbolic":                 ("text-t", False),
    "keyboard-symbolic":                    ("keyboard", False),
    "klipper-symbolic":                     ("clipboard", True),
    "laptop-symbolic":                      ("laptop", False),
    "link-2":                               ("link-simple-horizontal", False),
    "link":                                 ("link", False),
    "link-symbolic":                        ("link", True),
    "list-remove-symbolic":                 ("minus-circle", False),
    "lock":                                 ("lock", False),
    "mail-attachment-symbolic":             ("paperclip", False),
    "mail-flag-symbolic":                   ("flag", True),
    "mail-forward-symbolic":                ("envelope", True),
    "mail-mark-read-symbolic":              ("envelope-open", False),
    "mail-message-new-symbolic":            ("envelope", False),
    "mail-reply-symbolic":                  ("arrow-bend-up-left", False),
    "maximize":                             ("arrows-out", False),
    "media-eject-symbolic":                 ("eject", False),
    "media-record-symbolic":                ("record", False),
    "media-removable-symbolic":             ("usb", False),
    "media-seek-backward-symbolic":         ("rewind", False),
    "media-seek-forward-symbolic":          ("fast-forward", False),
    "media-skip-backward-symbolic":         ("skip-back", True),
    "media-skip-forward-symbolic":          ("skip-forward", True),
    "message-circle-question-mark":         ("chat-circle", False),
    "microphone-sensitivity-high-symbolic": ("microphone", False),
    "microphone-sensitivity-muted-symbolic":("microphone-slash", False),
    "minimize":                             ("arrows-in", False),
    "monitor-down":                         ("monitor-arrow-down", True),
    "network-server-symbolic":              ("hard-drives", True),
    "network-vpn-symbolic":                 ("shield", False),
    "network-wired-disconnected-symbolic":  ("plugs", True),
    "network-wired-error-symbolic":         ("plugs", True),
    "network-wired":                        ("plugs", False),
    "network-wireless-0-symbolic":          ("wifi-none", True),
    "network-wireless-20-symbolic":         ("wifi-low", False),
    "network-wireless-40-symbolic":         ("wifi-medium", False),
    "network-wireless-60-symbolic":         ("wifi-high", True),
    "network-wireless-80-symbolic":         ("wifi-high", True),
    "network-wireless-symbolic":            ("wifi-high", False),
    "network-workgroup":                    ("network", False),
    "network-workgroup-symbolic":           ("network", True),
    "normalize":                            ("arrows-in", True),
    "notification-disabled-symbolic":       ("bell-slash", False),
    "notification-new-symbolic":            ("bell-ringing", False),
    "notification-symbolic":                ("bell", True),
    "object-flip-horizontal-symbolic":      ("arrows-left-right", False),
    "object-flip-vertical-symbolic":        ("arrows-down-up", False),
    "open-menu":                            ("list", True),
    "open-menu-symbolic":                   ("list", True),
    "package-plus":                         ("package", False),
    "peek-desktop-symbolic":                ("desktop-tower", True),
    "phone-symbolic":                       ("phone", False),
    "pin":                                  ("push-pin", False),
    "power-profile-balanced-symbolic":      ("scales", False),
    "power-profile-performance-symbolic":   ("rocket", False),
    "power-profile-powersave-symbolic":     ("leaf", False),
    "preferences-other-symbolic":           ("dots-three", False),
    "printer-symbolic":                     ("printer", False),
    "process-working-symbolic":             ("spinner", False),
    "radio-symbolic":                       ("radio", False),
    "scanner-symbolic":                     ("scan", False),
    "search-symbolic":                      ("magnifying-glass", True),
    "semi-starred-symbolic":                ("star-half", False),
    "send-to-symbolic":                     ("paper-plane-tilt", False),
    "session-properties-symbolic":          ("user-gear", False),
    "share":                                ("share-network", False),
    "show-desktop-symbolic":                ("squares-four", True),
    "sidebar-hide-symbolic":                ("sidebar-simple", False),
    "sidebar-show-symbolic":                ("sidebar-simple", True),
    "starred-symbolic":                     ("star", False),
    "system-log-out":                       ("sign-out", False),
    "system-reboot":                        ("arrows-clockwise", False),
    "system-reboot-symbolic":               ("arrows-clockwise", True),
    "system-run-symbolic":                  ("play", False),
    "system-shutdown":                      ("power", False),
    "system-suspend":                       ("moon", False),
    "system-users":                         ("users", False),
    "tab-close-symbolic":                   ("x", True),
    "tablet-symbolic":                      ("device-tablet", False),
    "tab-new-symbolic":                     ("plus", False),
    "tag-symbolic":                         ("tag", False),
    "timer-symbolic":                       ("timer", False),
    "tools-check-spelling-symbolic":        ("spell-check", True),
    "tv-symbolic":                          ("television-simple", False),
    "unlock":                               ("lock-open", False),
    "user-available-symbolic":              ("circle", False),
    "user-away-symbolic":                   ("moon", True),
    "user-busy-symbolic":                   ("minus-circle", True),
    "user-desktop":                         ("desktop-tower", True),
    "user-desktop-symbolic":                ("desktop-tower", True),
    "user-home":                            ("house", True),
    "user-home-symbolic":                   ("house", True),
    "user-trash":                           ("trash", True),
    "user-trash-full":                      ("trash", True),
    "user-trash-symbolic":                  ("trash", True),
    "view-dual-symbolic":                   ("columns", False),
    "view-filter-symbolic":                 ("funnel", False),
    "view-grid-symbolic":                   ("grid-four", True),
    "view-hidden-symbolic":                 ("eye-slash", True),
    "view-list-icons-symbolic":             ("grid-four", True),
    "view-list-symbolic":                   ("list", True),
    "view-list-tree-symbolic":              ("tree-structure", True),
    "view-more-symbolic":                   ("dots-three", True),
    "view-pin-symbolic":                    ("push-pin", True),
    "view-refresh-symbolic":                ("arrows-clockwise", True),
    "view-restore-symbolic":                ("arrows-in", True),
    "view-sort-ascending-symbolic":         ("sort-ascending", False),
    "view-sort-descending-symbolic":        ("sort-descending", False),
    "view-sort-symbolic":                   ("sort-ascending", True),
    "view_tree":                            ("tree-structure", True),
    "weather-clear-symbolic":               ("sun", True),
    "weather-few-clouds-symbolic":          ("cloud-sun", False),
    "window-unpin-symbolic":                ("push-pin-slash", False),
    "xsi-bluetooth-acquiring-symbolic":     ("bluetooth", True),
    "ymuse-pause-symbolic":                 ("pause", False),
    "ymuse-save-symbolic":                  ("floppy-disk", False),
    "zoom-fit-best-symbolic":               ("arrows-out", True),
    "zoom-in-symbolic":                     ("magnifying-glass-plus", False),
    "zoom-original-symbolic":               ("magnifying-glass", True),
    "zoom-out-symbolic":                    ("magnifying-glass-minus", False),
}


def collect_all_symlinks():
    """Walk every .svgz symlink, resolve full chain, group by canonical source path."""
    source_to_links = defaultdict(list)

    for link in sorted(ICONS_DIR.rglob("*.svgz")):
        if not link.is_symlink():
            continue
        rel_link = str(link.relative_to(ICONS_DIR))
        if rel_link.startswith("source/") or rel_link.startswith("__") or rel_link.startswith("apps/"):
            continue

        current = link
        visited = set()
        final = None
        for _ in range(10):
            s = str(current)
            if s in visited:
                break
            visited.add(s)
            raw = os.readlink(current)
            nxt = (current.parent / raw).resolve()
            if not nxt.is_symlink():
                final = nxt
                break
            current = nxt

        if final is None:
            continue

        try:
            rel_final = str(final.relative_to(ICONS_DIR))
        except ValueError:
            continue

        if not rel_final.startswith("source/"):
            continue

        source_to_links[rel_final].append(rel_link)

    return source_to_links


def build_mapping():
    source_to_links = collect_all_symlinks()

    # Also pick up source files that exist but have no symlinks yet
    source_dir = ICONS_DIR / "source"
    if source_dir.exists():
        for sf in source_dir.rglob("*.svgz"):
            rel = str(sf.relative_to(ICONS_DIR))
            if rel not in source_to_links:
                source_to_links[rel] = []

    icons = []
    for source_file in sorted(source_to_links.keys()):
        stem = Path(source_file).stem
        links = sorted(source_to_links[source_file])
        phosphor_name, links_only = PHOSPHOR_NAMES.get(stem, (stem, True))
        icons.append({
            "source_file": source_file,
            "phosphor_name": phosphor_name,
            "links_only": links_only,
            "target_paths": links,
        })

    return {"icons": icons}


if __name__ == "__main__":
    mapping = build_mapping()
    output = ICONS_DIR / "icon_mapping.json"
    with open(output, "w") as f:
        json.dump(mapping, f, indent=2)
        f.write("\n")
    total_links = sum(len(e["target_paths"]) for e in mapping["icons"])
    fetchable = [e for e in mapping["icons"] if not e["links_only"]]
    links_only = [e for e in mapping["icons"] if e["links_only"]]
    print(f"Generated {output}")
    print(f"  {len(mapping['icons'])} source icons, {total_links} symlink paths")
    print(f"  {len(fetchable)} fetchable, {len(links_only)} links_only")
