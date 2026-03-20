#!/bin/env bash

# parse argument (default: light)
MODE="${1:-light}"
if [[ "$MODE" != "light" && "$MODE" != "dark" && "$MODE" != "oled" ]]; then
    echo "usage: $0 [light|dark|oled]"
    exit 1
fi

LIGHT_THEME_ID="org.blossomos.blossomuilight.desktop"
DARK_THEME_ID="org.blossomos.blossomuidark.desktop"
OLED_THEME_ID="org.blossomos.blossomuidarkoled.desktop"

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

# install icon pack (requires sudo)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [ -f "${SCRIPT_DIR}/icons/install.sh" ]; then
    echo "installing BlossomUI Icons..."
    bash "${SCRIPT_DIR}/icons/install.sh"
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
plasma-apply-colorscheme "$COLOR_SCHEME_NO_SPACE" 2>/dev/null
if command -v plasma-apply-desktoptheme >/dev/null 2>&1; then
    plasma-apply-desktoptheme BlossomUI 2>/dev/null
else
    $KWRITECONFIG --file ~/.config/plasmarc --group Theme --key name --type string "BlossomUI"
fi
$KWRITECONFIG --file ~/.config/kdeglobals --group Icons --key Theme --type string "BlossomUI Icons"
$KWRITECONFIG --file ~/.config/kdeglobals --group KDE --key widgetStyle --type string "BlossomUI"
$KWRITECONFIG --file ~/.config/kwinrc --group "org.kde.kdecoration2" --key library --type string "org.kde.blossomui"
$KWRITECONFIG --file ~/.config/kwinrc --group "org.kde.kdecoration2" --key theme --type string "BlossomUI"

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
