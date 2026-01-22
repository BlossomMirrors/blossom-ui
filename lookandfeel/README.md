# BlossomUI Look and Feel Packages

This directory contains the KDE Plasma Look and Feel packages for BlossomUI, providing complete desktop theming with all components configured.

## Available Themes

### BlossomUI Light (`org.blossomos.blossomuilight.desktop`)
A modern light theme featuring:
- **Widget Style**: BlossomUI
- **Color Scheme**: BlossomUI Light
- **Icon Theme**: Breeze (light)
- **Cursor Theme**: Bibata-Modern-Classic
- **Window Decorations**: BlossomUI
- **Sound Theme**: Ocean

### BlossomUI Dark (`org.blossomos.blossomuidark.desktop`)
A modern dark theme featuring:
- **Widget Style**: BlossomUI
- **Color Scheme**: BlossomUI Dark
- **Icon Theme**: Breeze Dark
- **Cursor Theme**: Bibata-Modern-Classic
- **Window Decorations**: BlossomUI
- **Sound Theme**: Ocean

## Installation

These Look and Feel packages are automatically installed when you build and install BlossomUI using the standard installation process:

```bash
./install.sh
```

During installation, you'll be prompted to:
1. Choose which theme to apply (Light, Dark, or Skip)
2. The installer automatically configures automatic Light/Dark theme switching

Or manually with CMake:

```bash
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
```

## Automatic Light/Dark Theme Switching

BlossomUI supports Plasma's automatic theme switching feature. During installation, the script configures:
- **Light theme**: BlossomUI Light (for daytime)
- **Dark theme**: BlossomUI Dark (for nighttime)

### Manual Configuration

To manually set up automatic switching after installation:

```bash
# Set light and dark themes for automatic switching
kwriteconfig6 --file ~/.config/plasmarc --group Theme --key LightLookAndFeel "org.blossomos.blossomuilight.desktop"
kwriteconfig6 --file ~/.config/plasmarc --group Theme --key DarkLookAndFeel "org.blossomos.blossomuidark.desktop"
kwriteconfig6 --file ~/.config/plasmarc --group Theme --key LightColorScheme "BlossomUI Light"
kwriteconfig6 --file ~/.config/plasmarc --group Theme --key DarkColorScheme "BlossomUI Dark"

# Apply a theme immediately
plasma-apply-lookandfeel -a org.blossomos.blossomuilight.desktop  # For Light
plasma-apply-lookandfeel -a org.blossomos.blossomuidark.desktop   # For Dark
```

### Enable Automatic Switching

1. Go to **System Settings** → **Appearance** → **Colors**
2. Enable **"Switch with system color scheme"** or **"Apply accent color from wallpaper"**
3. Themes will automatically switch based on your schedule or wallpaper

## Manual Theme Selection

You can also manually apply these themes:

1. **System Settings** → **Appearance** → **Global Theme**
2. Select either "BlossomUI Light" or "BlossomUI Dark"
3. Click "Apply" to activate the theme

## Namespace

BlossomUI uses the `org.blossomos` namespace for its Look and Feel packages, distinguishing it from official KDE themes while maintaining compatibility with the KDE Plasma theming system.

## Theme Components

Each Global Theme configures the following components automatically:

### BlossomUI Light & Dark Include:
- ✅ **Application Style**: BlossomUI widget style
- ✅ **Color Scheme**: Coordinated light/dark colors
- ✅ **Icons**: Breeze (light) or Breeze Dark
- ✅ **Cursors**: Bibata-Modern-Classic
- ✅ **Window Decorations**: BlossomUI decorations
- ✅ **Plasma Theme**: Default Plasma theme
- ✅ **Splash Screen**: Minimal splash
- ✅ **Sound Theme**: Ocean sound effects
- ✅ **Desktop Effects**: Blur enabled

## Customization

Each theme's configuration is stored in the `contents/defaults` file within its respective directory. You can customize:

- Widget styles
- Color schemes
- Icon themes
- Cursor themes
- Window decorations
- Plasma themes
- Sound themes
- KWin effects (blur, etc.)

## Structure

```
lookandfeel/
├── org.blossomos.blossomuilight.desktop/
│   ├── metadata.json           # Theme metadata and information
│   └── contents/
│       └── defaults            # Default configuration values
├── org.blossomos.blossomuidark.desktop/
│   ├── metadata.json
│   └── contents/
│       └── defaults
└── CMakeLists.txt              # Build configuration
```

## Requirements

- Breeze icon theme (provided by KDE)
- Bibata-Modern-Classic cursor theme (optional, will fallback to breeze_cursors)
- Ocean sound theme (provided by KDE, optional)

## Notes

- The Look and Feel packages are not built when compiling for Flatpak (`-DFOR_FLATPAK=ON`)
- Custom icon theme support will be added in a future release
- Wallpaper is set to "Next" which uses the default Plasma wallpaper rotation
- The `org.blossomos` namespace is used for BlossomUI-specific components
- KDE system integration points (like `org.kde.kdecoration2`) remain unchanged for compatibility
- Automatic theme switching requires Plasma 5.24+ or Plasma 6.0+
- Theme IDs use reverse domain notation: `org.blossomos.blossomuilight.desktop` and `org.blossomos.blossomuidark.desktop`

## Troubleshooting

### Theme doesn't appear in System Settings
Ensure the packages are installed to the correct location:
```bash
ls /usr/share/plasma/look-and-feel/ | grep blossomos
```

### Automatic switching not working
Check your configuration:
```bash
kreadconfig6 --file ~/.config/plasmarc --group Theme --key LightLookAndFeel
kreadconfig6 --file ~/.config/plasmarc --group Theme --key DarkLookAndFeel
```

### Apply theme from command line
```bash
# List all available themes
plasma-apply-lookandfeel --list

# Apply BlossomUI Light
plasma-apply-lookandfeel -a org.blossomos.blossomuilight.desktop

# Apply BlossomUI Dark
plasma-apply-lookandfeel -a org.blossomos.blossomuidark.desktop
```