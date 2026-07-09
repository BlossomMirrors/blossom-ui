#!/bin/env bash

# parse argument (default: light)
MODE="${1:-light}"
if [[ "$MODE" != "light" && "$MODE" != "dark" && "$MODE" != "oled" ]]; then
    echo "usage: $0 [light|dark|oled]"
    exit 1
fi

LIGHT_THEME_ID="org.blossomos.ui.light.desktop"
DARK_THEME_ID="org.blossomos.ui.dark.desktop"
OLED_THEME_ID="org.blossomos.ui.darkoled.desktop"

if [[ "$MODE" == "dark" ]]; then
    THEME_ID="$DARK_THEME_ID"
    COLOR_SCHEME="BlossomUI Dark"
    COLOR_SCHEME_NO_SPACE="BlossomUIDark"
elif [[ "$MODE" == "oled" ]]; then
    THEME_ID="$OLED_THEME_ID"
    COLOR_SCHEME="BlossomUI Dark OLED"
    COLOR_SCHEME_NO_SPACE="BlossomUIDarkOLED"
else
    THEME_ID="$LIGHT_THEME_ID"
    COLOR_SCHEME="BlossomUI Light"
    COLOR_SCHEME_NO_SPACE="BlossomUILight"
fi

if ! command -v plasma-apply-lookandfeel >/dev/null 2>&1; then
    return
fi

KWRITECONFIG=""
if command -v kwriteconfig6 >/dev/null 2>&1; then
    KWRITECONFIG="kwriteconfig6"
elif command -v kwriteconfig5 >/dev/null 2>&1; then
    KWRITECONFIG="kwriteconfig5"
else
    return
fi

echo "applying blossomui $MODE theme..."

# drop stale per-user color-scheme and desktoptheme copies that shadow the
# system files, and clear plasma/ksycoca caches so the new files get picked up
rm -f ~/.local/share/color-schemes/BlossomUILight.colors \
      ~/.local/share/color-schemes/BlossomUIDark.colors \
      ~/.local/share/color-schemes/BlossomUIDarkOLED.colors

rm -rf ~/.local/share/plasma/desktoptheme/BlossomUI

rm -rf ~/.cache/plasma-svgelements-* \
       ~/.cache/ksvg-elements \
       ~/.cache/plasma_theme_*.kcache \
       ~/.cache/icon-cache.kcache \
       ~/.cache/ksycoca6* \
       ~/.cache/ksycoca5*

if command -v kbuildsycoca6 >/dev/null 2>&1; then
    kbuildsycoca6 --noincremental >/dev/null 2>&1 || true
elif command -v kbuildsycoca5 >/dev/null 2>&1; then
    kbuildsycoca5 --noincremental >/dev/null 2>&1 || true
fi

if [[ "$MODE" == "oled" ]]; then
    $KWRITECONFIG --file ~/.config/kdeglobals --group General --key DarkColorScheme --type string "BlossomUI Dark OLED"
    $KWRITECONFIG --file ~/.config/plasmarc --group Theme --key DarkColorScheme --type string "BlossomUI Dark OLED"
    $KWRITECONFIG --file ~/.config/plasmarc --group Theme --key DarkLookAndFeel --type string "$OLED_THEME_ID"
    $KWRITECONFIG --file ~/.config/kdeglobals --group KDE --key DefaultDarkLookAndFeel --type string "$OLED_THEME_ID"
else
    $KWRITECONFIG --file ~/.config/kdeglobals --group General --key DarkColorScheme --type string "BlossomUI Dark"
    $KWRITECONFIG --file ~/.config/plasmarc --group Theme --key DarkColorScheme --type string "BlossomUI Dark"
    $KWRITECONFIG --file ~/.config/plasmarc --group Theme --key DarkLookAndFeel --type string "$DARK_THEME_ID"
    $KWRITECONFIG --file ~/.config/kdeglobals --group KDE --key DefaultDarkLookAndFeel --type string "$DARK_THEME_ID"
fi

$KWRITECONFIG --file ~/.config/kdeglobals --group KDE --key LookAndFeelPackage --type string "$THEME_ID"

plasma-apply-lookandfeel --apply "$THEME_ID" 2>/dev/null

# force-reapply the color scheme so kdeglobals' ColorSchemeHash updates and
# running apps reload the freshly installed system color file
if command -v plasma-apply-colorscheme >/dev/null 2>&1; then
    plasma-apply-colorscheme "$COLOR_SCHEME" 2>/dev/null || true
fi

# plasma-apply-colorscheme is a no-op when the scheme name is unchanged, so
# write the sha1 of the active scheme file into ColorSchemeHash ourselves —
# matches what KColorSchemeManager does and is what running apps compare against
for _dir in "$HOME/.local/share/color-schemes" /usr/share/color-schemes /usr/local/share/color-schemes; do
    if [ -f "$_dir/$COLOR_SCHEME_NO_SPACE.colors" ]; then
        _scheme_path="$_dir/$COLOR_SCHEME_NO_SPACE.colors"
        break
    fi
done
if [ -n "$_scheme_path" ] && command -v sha1sum >/dev/null 2>&1; then
    _hash=$(sha1sum "$_scheme_path" | awk '{print $1}')
    $KWRITECONFIG --file ~/.config/kdeglobals --group General --key ColorScheme --type string "$COLOR_SCHEME"
    $KWRITECONFIG --file ~/.config/kdeglobals --group General --key ColorSchemeHash --type string "$_hash"
fi


# reload KWin for decoration changes
if command -v qdbus6 >/dev/null 2>&1; then
    qdbus6 org.kde.KWin /KWin reconfigure 2>/dev/null || true
elif command -v qdbus >/dev/null 2>&1; then
    qdbus org.kde.KWin /KWin reconfigure 2>/dev/null || true
fi

# notify running Qt apps of widget style change (3 = StyleChanged)
if command -v qdbus6 >/dev/null 2>&1; then
    qdbus6 org.kde.KGlobalSettings /KGlobalSettings org.kde.KGlobalSettings.notifyChange 3 0 2>/dev/null || true
elif command -v qdbus >/dev/null 2>&1; then
    qdbus org.kde.KGlobalSettings /KGlobalSettings org.kde.KGlobalSettings.notifyChange 3 0 2>/dev/null || true
fi

if command -v flatpak >/dev/null 2>&1; then
    _ext_branches=$(flatpak list --runtime --columns=application,branch 2>/dev/null \
        | awk '$1 == "org.kde.KStyle.BlossomUI" && $2 ~ /^6/ {print $2}')
    if [ -n "$_ext_branches" ]; then
        _ext=/usr/share/runtime/lib/plugins/BlossomUI
        flatpak list --app --columns=application,runtime 2>/dev/null \
        | while read -r _app _rt; do
            case "$_rt" in
                org.kde.Platform/*) ;;
                *) continue ;;
            esac
            echo "$_ext_branches" | grep -qx "${_rt##*/}" || continue
            flatpak override --user \
                --env=QT_QUICK_CONTROLS_STYLE=org.blossomos.style \
                --env=QML2_IMPORT_PATH="/app/lib/qml:$_ext/lib/qml" \
                --env=QT_PLUGIN_PATH="/app/lib/plugins:/usr/share/runtime/lib/plugins:$_ext/lib/plugins" \
                "$_app" 2>/dev/null || true
        done
        echo "flatpak QQC2 style overrides applied (re-run after installing new apps)"
    fi
fi

systemctl --user restart plasma-plasmashell.service 2>/dev/null || (plasmashell --replace & disown)

echo "done"
echo "toggle light/dark in system settings"
