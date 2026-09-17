var panel = new Panel;
panel.location = "bottom";
panel.height = 48;

var kickoff = panel.addWidget("org.kde.plasma.kickoff");
kickoff.currentConfigGroup = ["General"];
kickoff.writeConfig("favorites", "applications:com.github.xournalpp.xournalpp.desktop,applications:org.libreoffice.LibreOffice.writer.desktop,applications:org.libreoffice.LibreOffice.calc.desktop,applications:org.libreoffice.LibreOffice.impress.desktop,applications:org.gimp.GIMP.desktop,applications:org.kde.spectacle.desktop,applications:org.kde.kamoso.desktop,applications:org.blossomos.winapps.desktop,applications:org.kde.kinfocenter.desktop");
kickoff.writeConfig("favoritesPortedToKAstats", false);

panel.addWidget("org.kde.plasma.pager");

var tasks = panel.addWidget("org.kde.plasma.icontasks");
tasks.currentConfigGroup = ["General"];
tasks.writeConfig("launchers", "applications:systemsettings.desktop,preferred://filemanager,applications:org.blossomos.Arc.desktop,preferred://browser,preferred://mailer");

panel.addWidget("org.kde.plasma.marginsseparator");

var systemtray = panel.addWidget("org.kde.plasma.systemtray");
systemtray.currentConfigGroup = ["General"];
systemtray.writeConfig("extraItems", "org.kde.kdeconnect,org.kde.plasma.vault,org.kde.kscreen,org.kde.plasma.battery,org.kde.plasma.bluetooth,org.kde.plasma.brightness,org.kde.plasma.cameraindicator,org.kde.plasma.devicenotifier,org.kde.plasma.keyboardindicator,org.kde.plasma.keyboardlayout,org.kde.plasma.manage-inputmethod,org.kde.plasma.mediacontroller,org.kde.plasma.networkmanagement,org.kde.plasma.notifications,org.kde.plasma.printmanager,org.kde.plasma.volume,org.kde.plasma.clipboard");
systemtray.writeConfig("hiddenItems", "chrome_status_icon_1@xdg-dbus-proxy,org.kde.plasma.clipboard,org.kde.merkuro.contact.applet,bitwarden");
systemtray.writeConfig("knownItems", "org.kde.kdeconnect,org.kde.plasma.vault,org.kde.kscreen,org.kde.plasma.battery,org.kde.plasma.bluetooth,org.kde.plasma.brightness,org.kde.plasma.cameraindicator,org.kde.plasma.clipboard,org.kde.plasma.devicenotifier,org.kde.plasma.keyboardindicator,org.kde.plasma.keyboardlayout,org.kde.plasma.manage-inputmethod,org.kde.plasma.mediacontroller,org.kde.plasma.networkmanagement,org.kde.plasma.notifications,org.kde.plasma.printmanager,org.kde.plasma.volume,org.kde.merkuro.contact.applet");

panel.addWidget("org.kde.plasma.digitalclock");

var desktopsArray = desktopsForActivity(currentActivity());
for (var j = 0; j < desktopsArray.length; j++) {
    desktopsArray[j].wallpaperPlugin = "org.kde.image";
    desktopsArray[j].currentConfigGroup = ["Wallpaper", "org.kde.image", "General"];
    desktopsArray[j].writeConfig("Image", "file:///usr/share/wallpapers/Blossom Rays/");
    desktopsArray[j].writeConfig("SlidePaths", "/usr/share/wallpapers/");
}
