
/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 *************************************************************************/

#include "blossomuistyleplugin.h"
#include "blossomuistyle.h"

#include <QApplication>

#if BLOSSOMUI_FLATPAK_QUICK_INTEGRATION
#include <QDirIterator>
#include <QFileInfo>
#include <QFontDatabase>
#include <QQuickStyle>
#endif

namespace BlossomUI
{

#if BLOSSOMUI_FLATPAK_QUICK_INTEGRATION
namespace
{

void appendPathEnvironment(const char *name, const QString &path)
{
    QStringList entries = qEnvironmentVariable(name).split(QLatin1Char(':'), Qt::SkipEmptyParts);
    if (entries.contains(path)) {
        return;
    }
    entries.append(path);
    qputenv(name, entries.join(QLatin1Char(':')).toLocal8Bit());
}

void setupFlatpakQuickIntegration()
{
    static bool done = false;
    if (done) {
        return;
    }
    done = true;

    QDirIterator fonts(QStringLiteral(BLOSSOMUI_FLATPAK_FONTDIR), {QStringLiteral("*.ttf"), QStringLiteral("*.otf")},
                       QDir::Files, QDirIterator::Subdirectories);
    while (fonts.hasNext()) {
        QFontDatabase::addApplicationFont(fonts.next());
    }

    const QString prefix = QStringLiteral(BLOSSOMUI_FLATPAK_PREFIX);
    const QString qmlDir = QStringLiteral(BLOSSOMUI_FLATPAK_QMLDIR);
    if (!QFileInfo::exists(qmlDir + QStringLiteral("/org/blossomos/style/qmldir"))) {
        return;
    }

    QCoreApplication::addLibraryPath(prefix);
    appendPathEnvironment("QML_IMPORT_PATH", qmlDir);

    if (qEnvironmentVariableIsSet("QT_QUICK_CONTROLS_STYLE")) {
        return;
    }
    const QString current = QQuickStyle::name();
    if (!current.isEmpty() && current != QLatin1String("org.kde.desktop") && current != QLatin1String("Fusion")) {
        return;
    }

    const QString style = QStringLiteral("org.blossomos.style");
    QQuickStyle::setStyle(style);
    qputenv("QT_QUICK_CONTROLS_STYLE", style.toLatin1());
}

}
#endif

//_
QStyle *StylePlugin::create(const QString &key)
{
    if (key.toLower() == QStringLiteral("blossomui")) {
#if BLOSSOMUI_FLATPAK_QUICK_INTEGRATION
        setupFlatpakQuickIntegration();
#endif
        return new Style;
    }
    return nullptr;
}

//_
QStringList StylePlugin::keys() const
{
    return QStringList(QStringLiteral("BlossomUI"));
}

}
