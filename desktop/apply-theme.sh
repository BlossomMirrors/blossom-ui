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

configure_dolphin() {
    $KWRITECONFIG --file ~/.config/dolphinrc --group General --key ShowStatusBar --type string "Disabled"
    $KWRITECONFIG --file ~/.config/dolphinrc --group MainWindow --key MenuBar --type string "Disabled"
    $KWRITECONFIG --file ~/.config/dolphinrc --group KDE --key MenuBar --type string "Disabled"
    $KWRITECONFIG --file ~/.config/dolphinrc --key MenuBar --type string "Disabled"

    # hide recent files/locations from places panel
    if [ -f ~/.local/share/user-places.xbel ]; then
        sed -i 's|<GroupState-RecentlySaved-IsHidden>false</GroupState-RecentlySaved-IsHidden>|<GroupState-RecentlySaved-IsHidden>true</GroupState-RecentlySaved-IsHidden>|g' ~/.local/share/user-places.xbel
    fi

    # copy toolbar config
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    mkdir -p ~/.local/share/kxmlgui5/dolphin
    cp "$SCRIPT_DIR/dolphin/dolphinui.rc" ~/.local/share/kxmlgui5/dolphin/dolphinui.rc
}

echo "applying blossomui $MODE theme..."

# drop stale per-user color-scheme copies that shadow the system files,
# and clear plasma/ksycoca caches so the new schemes get picked up
rm -f ~/.local/share/color-schemes/BlossomUILight.colors \
      ~/.local/share/color-schemes/BlossomUIDark.colors \
      ~/.local/share/color-schemes/BlossomUIDarkOLED.colors

rm -rf ~/.cache/plasma-svgelements-* \
       ~/.cache/plasma_theme_*.kcache \
       ~/.cache/icon-cache.kcache \
       ~/.cache/ksycoca6* \
       ~/.cache/ksycoca5*

if command -v kbuildsycoca6 >/dev/null 2>&1; then
    kbuildsycoca6 --noincremental >/dev/null 2>&1 || true
elif command -v kbuildsycoca5 >/dev/null 2>&1; then
    kbuildsycoca5 --noincremental >/dev/null 2>&1 || true
fi

# set default light/dark/oled themes
$KWRITECONFIG --file ~/.config/kdeglobals --group General --key LightColorScheme --type string "BlossomUI Light"
if [[ "$MODE" == "oled" ]]; then
    $KWRITECONFIG --file ~/.config/kdeglobals --group General --key DarkColorScheme --type string "BlossomUI Dark OLED"
else
    $KWRITECONFIG --file ~/.config/kdeglobals --group General --key DarkColorScheme --type string "BlossomUI Dark"
fi

$KWRITECONFIG --file ~/.config/plasmarc --group Theme --key LightColorScheme --type string "BlossomUI Light"
if [[ "$MODE" == "oled" ]]; then
    $KWRITECONFIG --file ~/.config/plasmarc --group Theme --key DarkColorScheme --type string "BlossomUI Dark OLED"
    $KWRITECONFIG --file ~/.config/plasmarc --group Theme --key DarkLookAndFeel --type string "$OLED_THEME_ID"
else
    $KWRITECONFIG --file ~/.config/plasmarc --group Theme --key DarkColorScheme --type string "BlossomUI Dark"
    $KWRITECONFIG --file ~/.config/plasmarc --group Theme --key DarkLookAndFeel --type string "$DARK_THEME_ID"
fi
$KWRITECONFIG --file ~/.config/plasmarc --group Theme --key LightLookAndFeel --type string "$LIGHT_THEME_ID"


$KWRITECONFIG --file ~/.config/kdeglobals --group KDE --key LookAndFeelPackage --type string "$THEME_ID"
$KWRITECONFIG --file ~/.config/kdeglobals --group KDE --key DefaultLightLookAndFeel --type string "$LIGHT_THEME_ID"
if [[ "$MODE" == "oled" ]]; then
    $KWRITECONFIG --file ~/.config/kdeglobals --group KDE --key DefaultDarkLookAndFeel --type string "$OLED_THEME_ID"
else
    $KWRITECONFIG --file ~/.config/kdeglobals --group KDE --key DefaultDarkLookAndFeel --type string "$DARK_THEME_ID"
fi

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

configure_dolphin

echo "done"
echo "toggle light/dark in system settings"
