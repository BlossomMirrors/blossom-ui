#!/usr/bin/env bash

if [ ! "$(which sassc 2> /dev/null)" ]; then
   echo "sassc needs to be installed to generate the CSS."
   exit 1
fi

SOURCE_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
USER_SETTINGS="$SOURCE_DIR/sass/_darkly_user_settings.scss"
SASSC_OPT="-M -t compact"

if [ -n "$XDG_DATA_HOME" ]; then
    DEST_DIR="$XDG_DATA_HOME/themes"
else
    DEST_DIR="$HOME/.local/share/themes"
fi

if [ -n "$XDG_CONFIG_HOME" ]; then
    DARKLYRC="$XDG_CONFIG_HOME/darklyrc"
    GTK4DIR="$XDG_CONFIG_HOME/gtk-4.0"
else
    DARKLYRC="$HOME/.config/darklyrc"
    GTK4DIR="$HOME/.config/gtk-4.0"
fi

show_help() {
    cat << EOF
Usage: $0 [OPTIONS]...

OPTIONS:
-d      Specify destination directory (Default: $DEST_DIR)
-l      Libadwaita support
-u      Uninstall the theme
-h      Show help
EOF
    exit 0
}

parse_darklyrc() {
    if [ -f "$DARKLYRC" ]; then
        echo "Darkly config found: $DARKLYRC"
        echo
        echo "Writing GTK theme user config:"
        echo

        awk '
            function trim(s) { gsub(/^[ \t]+|[ \t]+$/, "", s); return s }

            /^\[(Common|Style|Windeco)\]/ { keep=1; next }
            /^\[/ { keep=0 }

            keep && /^[^=]+=.*$/ {
                split($0, a, "=")
                key = trim(a[1])
                val = trim(a[2])

                if (val ~ /^[0-9]+,[ \t]*[0-9]+,[ \t]*[0-9]+$/) {
                    split(val, rgb, ",")
                    val = sprintf("rgb(%s, %s, %s)", rgb[1], rgb[2], rgb[3])
                }

                printf("$Darklyrc%s: %s;\n", key, val)
            }
            ' "$DARKLYRC" | tee "$USER_SETTINGS"
        echo
    else
        echo
        echo "darklyrc not found. Using default settings."
        echo "" > "$USER_SETTINGS"
    fi
}

install_theme() {
    echo "Generating CSS files..."
    echo
    mkdir -p "$SOURCE_DIR/build"
    sassc $SASSC_OPT "$SOURCE_DIR/sass/gtk3-light.scss" "$SOURCE_DIR/build/gtk3-light.css" || { echo "GTK 3 CSS generation failed." ; exit 1; }
    sassc $SASSC_OPT "$SOURCE_DIR/sass/gtk3-dark.scss" "$SOURCE_DIR/build/gtk3-dark.css" || { echo "GTK 3 CSS generation failed." ; exit 1; }
    sassc $SASSC_OPT "$SOURCE_DIR/sass/gtk4.scss" "$SOURCE_DIR/build/gtk4.css" || { echo "GTK 4 CSS generation failed." ; exit 1; }

    echo "Installing theme to $DEST_DIR"
    mkdir -p "$DEST_DIR/Darkly/"{assets,gtk-3.0,gtk-4.0}
    cp -r "$SOURCE_DIR/assets/"*.{png,svg} "$DEST_DIR/Darkly/assets/"
    ln -fns ../assets "$DEST_DIR/Darkly/gtk-3.0/darkly-gtk-assets"
    ln -fns ../assets "$DEST_DIR/Darkly/gtk-4.0/darkly-gtk-assets"
    ln -fs ./gtk.css "$DEST_DIR/Darkly/gtk-4.0/gtk-dark.css"

    cp "$SOURCE_DIR/build/gtk3-light.css" "$DEST_DIR/Darkly/gtk-3.0/gtk.css"
    cp "$SOURCE_DIR/build/gtk3-dark.css" "$DEST_DIR/Darkly/gtk-3.0/gtk-dark.css"
    cp "$SOURCE_DIR/build/gtk4.css" "$DEST_DIR/Darkly/gtk-4.0/gtk.css"
}

install_libadwaita() {
    local css

    echo "Enabling libadwaita support in $GTK4DIR/gtk.css"
    mkdir -p "$GTK4DIR"

    if [ -f "$GTK4DIR/colors.css" ]; then
        css=$'/* DO NOT MODIFY. THIS FILE WAS CREATED AUTOMATICALLY */\n@import \'gtk-darkly.css\';\n@import \'colors.css\';'
        #echo "colors.css found"
    else
        css=$'/* DO NOT MODIFY. THIS FILE WAS CREATED AUTOMATICALLY */\n@import \'gtk-darkly.css\';'
        #echo "colors.css not found"
    fi

    if [ -f "$GTK4DIR/gtk.css" ]; then
        #echo "gtk.css found"

        if ! cmp --silent -- "$GTK4DIR/gtk.css" <(echo -n "$css"); then
            echo "Backing up $GTK4DIR/gtk.css"
            mv "$GTK4DIR/gtk.css" "$GTK4DIR/gtk.css.created_by_darkly_installer.bak"
        fi
    fi

    echo "Writing $GTK4DIR/gtk.css"
    echo -n "$css" > "$GTK4DIR/gtk.css"

    mkdir -p "$GTK4DIR/darkly-gtk-assets"
    cp -r "$SOURCE_DIR/assets/"*.{png,svg} "$GTK4DIR/darkly-gtk-assets/"
    cp "$DEST_DIR/Darkly/gtk-4.0/gtk.css" "$GTK4DIR/gtk-darkly.css"

    echo
    echo "Installation successful"
    echo
}

uninstall () {
    echo "Uninstalling"
    if [ -d "$DEST_DIR/Darkly" ]; then
        echo "Found theme at $DEST_DIR/Darkly"
        rm -rf "$DEST_DIR/Darkly"
    else
        echo "Theme is not installed in $DEST_DIR"
    fi

    if [ -f  "$GTK4DIR/gtk-darkly.css" ]; then

        if [ -f "$GTK4DIR/gtk.css.created_by_darkly_installer.bak" ]; then
            mv "$GTK4DIR/gtk.css.created_by_darkly_installer.bak" "$GTK4DIR/gtk.css"
        else
            rm -f "$GTK4DIR/gtk.css"
        fi

        rm "$GTK4DIR/gtk-darkly.css"
        rm -rf "$GTK4DIR/darkly-gtk-assets"
    else
        echo "Libadwaita theme is not installed in $GTK4DIR"
    fi

    echo "Uninstall successful."
    exit 0
}

[ $# -eq 0 ] && show_help
while getopts ":d:lh:u" arg; do
    case $arg in
        d)
            DEST_DIR="$OPTARG"
            ;;
        l)
            LIBADWAITA=true
            ;;
        u)
            UNINSTALL=true
            ;;
        h | *)
            show_help
            ;;
    esac
done

if [ "$UNINSTALL" = true ]; then
    uninstall
fi

parse_darklyrc
install_theme

if [ "$LIBADWAITA" = true ]; then
    install_libadwaita
fi
