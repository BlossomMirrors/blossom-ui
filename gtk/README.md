# BlossomUI GTK theme

A GTK port of the BlossomUI Qt style, forked from the [Darkly GTK theme](https://github.com/Bali10050/Darkly-GTK-Theme) which itself matches the [Darkly Qt style](https://github.com/Bali10050/Darkly) by Bali10050.

The goal of this project is to provide a GTK theme that matches the BlossomUI theme for Qt applications to achieve a consistent BlossomUI-styled Plasma desktop.

![preview](preview.png?raw=true)

* Supports GTK 3.20+, GTK 4.0+ and libadwaita
* Automatically follows the Plasma color scheme
* Customization options from the BlossomUI Qt style config (corner radius, tab style, etc. ) are applied during installation (work in progress, see `sass/_blossomui_default_settings.scss` for the current status). That means that you have to re-run the install script after changing the BlossomUI Qt settings.

## Requirements
`sassc` build dependency

## Installation

Run the installation script

```
./install.sh 
```

Detailed usage of the installation script:

```
Usage: ./install.sh [OPTIONS]...

OPTIONS:
-d      Specify destination directory (Default: $HOME/.local/share/themes
                                                or $XDG_DATA_HOME/themes if $XDG_DATA_HOME is set)
-l      Libadwaita support. Copies theme to ~/.config/gtk-4.0/
                                            or $XDG_CONFIG_DIR/gtk-4.0/ if $XDG_CONFIG_DIR is set
-u      Uninstall the theme
-h      Show help
```

Note: While install script will attempt to back up/restore your `.config/gtk-4.0.css` file it is a good idea to back up any important changes you made before running the install script.

### Flatpak apps

Flatpak apps need permission to read the user's theme directory.

```
sudo flatpak override --filesystem=xdg-data/themes
```

For Flatpak apps to be able to use the libadwaita theme, you have to grant these permissions as well

 ```
sudo flatpak override --filesystem=xdg-config/gtk-4.0
 ```

## Disclaimer

Third party themes may break certain apps. If this theme breaks an app, report it here and not to the app developers.
There may be some cases where these bugs can't be fixed from within the theme, in which case we're out of luck unfortunately.


## Credits

Based on the stylesheets from [GTK](https://gitlab.gnome.org/GNOME/gtk/) and [libadwaita](https://gitlab.gnome.org/GNOME/libadwaita)
