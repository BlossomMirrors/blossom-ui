# BlossomUI

BlossomUI is a fork of [Darkly](https://github.com/Bali10050/Darkly), which is a fork of [Lightly](https://github.com/Luwx/Lightly), which is a fork of [KDE Breeze](https://invent.kde.org/frameworks/breeze).

**License:** GPL-2.0-or-later. See [LICENSE.md](LICENSE.md) for details and attribution. The full license text is in [COPYING](COPYING).

### Future

We plan to adopt [KDE Union](https://invent.kde.org/plasma/union), KDE’s unified SVG-based theming engine, once it is mature—to simplify and modernize the codebase. See [“Moving KDE's styling into the future”](https://quantumproductions.info/articles/2025-02/moving-kdes-styling-future) for an overview.

## Installation

## Automatic

coming soon...

## Manual

### To ensure this application style works on applications still using QT5 both QT5 and QT6 dependencies are required to be installed before building from source.

### Installation script

> [!NOTE]
> A script called `install.sh` is now available which both builds and installs this application style.

`./install.sh` will remove if existing, build and install BlossomUI using both QT5/QT6 dependencies.

`./install.sh QT5` will build & install using only QT5/KF5 dependencies.

`./install.sh QT6` will build & install using only QT6/KF6 dependencies.

`./install.sh remove` will remove BlossomUI.

***

### Flatpak

Due to flatpak's [restriction on file system access](https://docs.flatpak.org/en/latest/sandbox-permissions.html) (in short no access to system libraries, /dev, etc), the BlossomUI flatpak bundle is only an extension of the KDE runtime.\
If you want to apply BlossomUI as the application style, you either need:
- BlossomUI installed on the system via package manager (pacman, rpm, apt-get, etc) or
- Set manually with QT_STYLE_OVERRIDE=BlossomUI env variable \[`sudo flatpak override --env=QT_QPA_PLATFORMTHEME=kde` (`sudo` only applies for flatpaks in /var/lib/flatpak, otherwise use `--user` without sudo)]

**Important (primarily) for users of immutable distros!**\
BlossomUI won't show as an option in `System settings > Theme > Application styles` without system installation. Also, both BlossomUI runtime version and the KDE runtime version must match (if they don't match, the app won't use BlossomUI)

This brings some untentended behaviour (for any application style, not BlossomUI in particular), for example:
- Changing color scheme partly applies for QWidget apps (not at all for QML-based)
- Changing application style doesn't apply until the app is restarted

Manifests should have the latest versions of KDE Runtime and SDK. Change the version accordingly. (you can find the version of runtime for each app by running `flatpak list --app --columns=app,runtime`)

#### Build with:
```
org.flatpak.Builder flatpak-build --repo=local --force-clean --ccache org.kde.KStyle.BlossomUI6.json
```

use `org.kde.KStyle.BlossomUI5.json` for KF5

#### Bundle with:
```
flatpak build-bundle local/ blossomui.flatpak runtime/org.kde.KStyle.BlossomUI/x86_64/<runtime_version>
```

***


#### <u>Arch Linux</u>

```
sudo pacman -S --noconfirm cmake extra-cmake-modules kdecoration qt6-declarative kcoreaddons \
      kcmutils kcolorscheme kconfig kguiaddons kiconthemes kwindowsystem git \
      qt5-declarative qt5-x11extras gcc make kcmutils5 \
      frameworkintegration5 kconfigwidgets5 kiconthemes5 \
      kirigami2 kwindowsystem5
```

```
git clone --single-branch --depth=1 https://git.blossomos.org/Blossom/ui.git
cd ui
./install.sh
```
