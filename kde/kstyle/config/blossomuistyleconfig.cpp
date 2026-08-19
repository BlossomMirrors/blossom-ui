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

#include "blossomuistyleconfig.h"

#include "blossomuistyleconfigdata.h"
#include "blossomuistyleversion.h"
#include <KLocalizedString>
#include <QDBusConnection>
#include <QDBusMessage>

extern "C" {
Q_DECL_EXPORT QWidget *allocate_kstyle_config(QWidget *parent)
{
    return new BlossomUI::StyleConfig(parent);
}
}

namespace BlossomUI
{


StyleConfig::StyleConfig(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    // load setup from configData
    load();

    connect(_windowDragMode, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(_menuOpacity, &QAbstractSlider::valueChanged, this, &StyleConfig::updateChanged);
    connect(_menuOpacity, SIGNAL(valueChanged(int)), _menuOpacitySpinBox, SLOT(setValue(int)));
    connect(_menuOpacitySpinBox, SIGNAL(valueChanged(int)), _menuOpacity, SLOT(setValue(int)));

    connect(_sidebarOpacity, &QAbstractSlider::valueChanged, this, &StyleConfig::updateChanged);
    connect(_sidebarOpacity, SIGNAL(valueChanged(int)), _sidebarOpacitySpinBox, SLOT(setValue(int)));
    connect(_sidebarOpacitySpinBox, SIGNAL(valueChanged(int)), _sidebarOpacity, SLOT(setValue(int)));

    connect(_menuBarOpacity, &QAbstractSlider::valueChanged, this, &StyleConfig::updateChanged);
    connect(_menuBarOpacity, SIGNAL(valueChanged(int)), _menuBarOpacitySpinBox, SLOT(setValue(int)));
    connect(_menuBarOpacitySpinBox, SIGNAL(valueChanged(int)), _menuBarOpacity, SLOT(setValue(int)));

    connect(_toolBarOpacity, &QAbstractSlider::valueChanged, this, &StyleConfig::updateChanged);
    connect(_toolBarOpacity, SIGNAL(valueChanged(int)), _toolBarOpacitySpinBox, SLOT(setValue(int)));
    connect(_toolBarOpacitySpinBox, SIGNAL(valueChanged(int)), _toolBarOpacity, SLOT(setValue(int)));

    connect(_tabBarOpacity, &QAbstractSlider::valueChanged, this, &StyleConfig::updateChanged);
    connect(_tabBarOpacity, SIGNAL(valueChanged(int)), _tabBarOpacitySpinBox, SLOT(setValue(int)));
    connect(_tabBarOpacitySpinBox, SIGNAL(valueChanged(int)), _tabBarOpacity, SLOT(setValue(int)));

    connect(_widgetDrawShadow, &QAbstractButton::toggled, this, &StyleConfig::updateChanged);
    connect(_widgetToolBarShadow, &QAbstractButton::toggled, this, &StyleConfig::updateChanged);
    connect(_shadowSize, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(_shadowColor, &KColorCombo::activated, this, &StyleConfig::updateChanged);
    connect(_shadowStrength, SIGNAL(valueChanged(int)), _shadowStrength, SLOT(setValue(int)));
    connect(_shadowIntensity, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));

    connect(_cornerRadius, SIGNAL(valueChanged(int)), SLOT(updateChanged()));

    connect(_buttonHeight, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(_buttonWidth, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(_menuItemHeight, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
}


void StyleConfig::save()
{
    StyleConfigData::setWindowDragMode(_windowDragMode->currentIndex());
    StyleConfigData::setMenuOpacity(_menuOpacity->value());
    StyleConfigData::setDolphinSidebarOpacity(_sidebarOpacity->value());
    StyleConfigData::setMenuBarOpacity(_menuBarOpacity->value());
    StyleConfigData::setToolBarOpacity(_toolBarOpacity->value());
    StyleConfigData::setTabBarOpacity(_tabBarOpacity->value());
    StyleConfigData::setWidgetDrawShadow(_widgetDrawShadow->isChecked());
    StyleConfigData::setWidgetToolBarShadow(_widgetToolBarShadow->isChecked());
    StyleConfigData::setShadowSize(_shadowSize->currentIndex());
    StyleConfigData::setShadowColor(_shadowColor->color());
    StyleConfigData::setShadowStrength(_shadowStrength->value());
    StyleConfigData::setShadowIntensity(_shadowIntensity->currentIndex());
    StyleConfigData::setCornerRadius(_cornerRadius->value());
    StyleConfigData::setButtonHeight(_buttonHeight->value());
    StyleConfigData::setButtonWidth(_buttonWidth->value());
    StyleConfigData::setMenuItemHeight(_menuItemHeight->value());

    StyleConfigData::self()->save();

    // emit dbus signal
    QDBusMessage message(
        QDBusMessage::createSignal(QStringLiteral("/BlossomUIStyle"), QStringLiteral("org.blossomos.ui"), QStringLiteral("reparseConfiguration")));
    QDBusConnection::sessionBus().send(message);
}


void StyleConfig::defaults()
{
    StyleConfigData::self()->setDefaults();
    load();
}


void StyleConfig::reset()
{
    // reparse configuration
    StyleConfigData::self()->load();

    load();
}


void StyleConfig::updateChanged()
{
    bool modified(false);

    // check if any value was modified
    if (_windowDragMode->currentIndex() != StyleConfigData::windowDragMode())
        modified = true;
    else if (_menuOpacity->value() != StyleConfigData::menuOpacity()) {
        modified = true;
        _menuOpacitySpinBox->setValue(_menuOpacity->value());
    } else if (_buttonHeight->value() != StyleConfigData::buttonHeight()) {
        modified = true;
    } else if (_buttonWidth->value() != StyleConfigData::buttonWidth()) {
        modified = true;
    } else if (_sidebarOpacity->value() != StyleConfigData::dolphinSidebarOpacity()) {
        modified = true;
        _sidebarOpacitySpinBox->setValue(_sidebarOpacity->value());
    } else if (_menuBarOpacity->value() != StyleConfigData::menuBarOpacity()) {
        modified = true;
        _menuBarOpacitySpinBox->setValue(_menuBarOpacity->value());
    } else if (_toolBarOpacity->value() != StyleConfigData::toolBarOpacity()) {
        modified = true;
        _toolBarOpacitySpinBox->setValue(_toolBarOpacity->value());
    } else if (_tabBarOpacity->value() != StyleConfigData::tabBarOpacity()) {
        modified = true;
        _tabBarOpacitySpinBox->setValue(_tabBarOpacity->value());
    } else if (_widgetDrawShadow->isChecked() != StyleConfigData::widgetDrawShadow())
        modified = true;
    else if (_widgetToolBarShadow->isChecked() != StyleConfigData::widgetToolBarShadow())
        modified = true;
    else if (_shadowSize->currentIndex() != StyleConfigData::shadowSize()) {
        modified = true;
    } else if (_shadowColor->color() != StyleConfigData::shadowColor())
        modified = true;
    else if (_shadowStrength->value() != StyleConfigData::shadowStrength())
        modified = true;
    else if (_shadowIntensity->currentIndex() != StyleConfigData::shadowIntensity())
        modified = true;
    else if (_cornerRadius->value() != StyleConfigData::cornerRadius())
        modified = true;
    else if (_menuItemHeight->value() != StyleConfigData::menuItemHeight())
        modified = true;

    if (_shadowSize->currentIndex() == 0) {
        _shadowColor->setEnabled(false);
        _shadowIntensity->setEnabled(false);
        _shadowStrength->setEnabled(false);
    } else {
        _shadowColor->setEnabled(true);
        _shadowIntensity->setEnabled(true);
        _shadowStrength->setEnabled(true);
    }

    if (!_widgetDrawShadow->isChecked()) {
        _widgetToolBarShadow->setEnabled(false);
    } else {
        _widgetToolBarShadow->setEnabled(true);
    }

    emit changed(modified);
}


void StyleConfig::load()
{
    _windowDragMode->setCurrentIndex(StyleConfigData::windowDragMode());
    _menuOpacity->setValue(StyleConfigData::menuOpacity());
    _menuOpacitySpinBox->setValue(StyleConfigData::menuOpacity());
    _sidebarOpacity->setValue(StyleConfigData::dolphinSidebarOpacity());
    _sidebarOpacitySpinBox->setValue(StyleConfigData::dolphinSidebarOpacity());
    _menuBarOpacity->setValue(StyleConfigData::menuBarOpacity());
    _menuBarOpacitySpinBox->setValue(StyleConfigData::menuBarOpacity());
    _toolBarOpacity->setValue(StyleConfigData::toolBarOpacity());
    _toolBarOpacitySpinBox->setValue(StyleConfigData::toolBarOpacity());
    _tabBarOpacity->setValue(StyleConfigData::tabBarOpacity());
    _tabBarOpacitySpinBox->setValue(StyleConfigData::tabBarOpacity());
    _buttonHeight->setValue(StyleConfigData::buttonHeight());
    _buttonWidth->setValue(StyleConfigData::buttonWidth());
    _widgetDrawShadow->setChecked(StyleConfigData::widgetDrawShadow());
    _widgetToolBarShadow->setChecked(StyleConfigData::widgetToolBarShadow());
    _menuItemHeight->setValue(StyleConfigData::menuItemHeight());

    if (!_widgetDrawShadow->isChecked()) {
        _widgetToolBarShadow->setEnabled(false);
    }

    for (QString &item : _shadowSizes) {
        if (item == "None") {
            _shadowSize->addItem(item, "ShadowNone");
        } else if (item == "Small") {
            _shadowSize->addItem(item, "ShadowSmall");
        } else if (item == "Medium") {
            _shadowSize->addItem(item, "ShadowMedium");
        } else if (item == "Large") {
            _shadowSize->addItem(item, "ShadowLarge");
        } else if (item == "VeryLarge") {
            _shadowSize->addItem(item, "ShadowVeryLarge");
        }
    }
    _shadowSize->setCurrentIndex(StyleConfigData::shadowSize());
    if (_shadowSize->currentIndex() == 0) {
        _shadowColor->setEnabled(false);
        _shadowIntensity->setEnabled(false);
        _shadowStrength->setEnabled(false);
    }

    _shadowColor->setColor(StyleConfigData::shadowColor());
    _shadowStrength->setValue(StyleConfigData::shadowStrength());
    _shadowIntensity->setCurrentIndex(StyleConfigData::shadowIntensity());

    _cornerRadius->setValue(StyleConfigData::cornerRadius());
    _versionNumber->setText(BLOSSOMUI_VERSION_STRING);
}

}
