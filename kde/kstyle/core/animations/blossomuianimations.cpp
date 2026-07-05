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

#include "blossomuianimations.h"
#include "blossomuipropertynames.h"
#include "blossomuistyleconfigdata.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QDial>
#include <QGroupBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QProgressBar>
#include <QRadioButton>
#include <QScrollBar>
#include <QSpinBox>
#include <QTabBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolBox>
#include <QToolButton>

namespace BlossomUI
{

//____________________________________________________________
Animations::Animations(QObject *parent)
    : QObject(parent)
{
    _widgetEnabilityEngine = new WidgetStateEngine(this);
    _busyIndicatorEngine = new BusyIndicatorEngine(this);
    _comboBoxEngine = new WidgetStateEngine(this);
    _toolButtonEngine = new WidgetStateEngine(this);
    _spinBoxEngine = new SpinBoxEngine(this);
    _toolBoxEngine = new ToolBoxEngine(this);

    registerEngine(_headerViewEngine = new HeaderViewEngine(this));
    registerEngine(_widgetStateEngine = new WidgetStateEngine(this));
    registerEngine(_inputWidgetEngine = new WidgetStateEngine(this));
    registerEngine(_scrollBarEngine = new ScrollBarEngine(this));
    registerEngine(_stackedWidgetEngine = new StackedWidgetEngine(this));
    registerEngine(_tabBarEngine = new TabBarEngine(this));
    registerEngine(_dialEngine = new DialEngine(this));
}

//____________________________________________________________
void Animations::setupEngines()
{
    // apply KDE global animation speed (kdeglobals [General] AnimationDurationFactor)
    const KConfigGroup generalGroup(KSharedConfig::openConfig(QStringLiteral("kdeglobals")), QStringLiteral("General"));
    const qreal globalFactor = generalGroup.readEntry("AnimationDurationFactor", 1.0);
    const int baseDuration = StyleConfigData::animationsDuration();
    const int animationsDuration = qRound(baseDuration * qBound(0.0, globalFactor, 10.0));

    // animation steps
    AnimationData::setSteps(animationsDuration / 1000.0 * 60);

    const bool animationsEnabled(StyleConfigData::animationsEnabled());
    // const int animationsDuration( 1000 );

    _widgetEnabilityEngine->setEnabled(animationsEnabled);
    _comboBoxEngine->setEnabled(animationsEnabled);
    _toolButtonEngine->setEnabled(animationsEnabled);
    _spinBoxEngine->setEnabled(animationsEnabled);
    _toolBoxEngine->setEnabled(animationsEnabled);

    _widgetEnabilityEngine->setDuration(animationsDuration);
    _comboBoxEngine->setDuration(animationsDuration);
    _toolButtonEngine->setDuration(animationsDuration);
    _spinBoxEngine->setDuration(animationsDuration);
    _stackedWidgetEngine->setDuration(animationsDuration);
    _toolBoxEngine->setDuration(animationsDuration);

    // registered engines
    foreach (const BaseEngine::Pointer &engine, _engines) {
        engine.data()->setEnabled(animationsEnabled);
        engine.data()->setDuration(animationsDuration);
    }

    // stacked widget transition has an extra flag for animations
    _stackedWidgetEngine->setEnabled(animationsEnabled && StyleConfigData::stackedWidgetTransitionsEnabled());

    // busy indicator
    _busyIndicatorEngine->setEnabled(StyleConfigData::progressBarAnimated());
    _busyIndicatorEngine->setDuration(StyleConfigData::progressBarBusyStepDuration());
}

//____________________________________________________________
void Animations::registerWidget(QObject *widget) const
{
    if (!widget)
        return;

    // check against noAnimations property
    QVariant propertyValue(widget->property(PropertyNames::noAnimations));
    if (propertyValue.isValid() && propertyValue.toBool())
        return;

    // Qt Quick Controls: register by elementType property
    const QString elementType = widget->property("elementType").toString();
    if (!elementType.isEmpty()) {
        if (elementType == QLatin1String("checkbox") || elementType == QLatin1String("radiobutton")) {
            _widgetStateEngine->registerWidget(widget, AnimationHover | AnimationFocus | AnimationPressed);
        } else if (elementType == QLatin1String("button") || elementType == QLatin1String("toolbutton")) {
            _widgetStateEngine->registerWidget(widget, AnimationHover | AnimationFocus | AnimationPressed);
        } else if (elementType == QLatin1String("slider") || elementType == QLatin1String("dial")) {
            _widgetStateEngine->registerWidget(widget, AnimationHover | AnimationFocus);
        } else if (elementType == QLatin1String("edit") || elementType == QLatin1String("combobox")
                   || elementType == QLatin1String("spinbox")) {
            _inputWidgetEngine->registerWidget(widget, AnimationHover | AnimationFocus);
        }
        return;
    }

    // Qt Quick controls without an elementType are not QWidgets; skip widget-only engines
    QWidget *w = qobject_cast<QWidget *>(widget);
    if (!w)
        return;

    // all widgets are registered to the enability engine.
    _widgetEnabilityEngine->registerWidget(w, AnimationEnable);

    // install animation timers
    // for optimization, one should put with most used widgets here first

    // buttons
    if (qobject_cast<QToolButton *>(w)) {
        _toolButtonEngine->registerWidget(w, AnimationHover | AnimationFocus);
        _widgetStateEngine->registerWidget(w, AnimationHover | AnimationFocus | AnimationPressed);

    } else if (qobject_cast<QCheckBox *>(w) || qobject_cast<QRadioButton *>(w)) {
        _widgetStateEngine->registerWidget(w, AnimationHover | AnimationFocus | AnimationPressed);

    } else if (qobject_cast<QAbstractButton *>(w)) {
        if (qobject_cast<QToolBox *>(w->parent()))
            _toolBoxEngine->registerWidget(w);
        _widgetStateEngine->registerWidget(w, AnimationHover | AnimationFocus | AnimationPressed);

    } else if (QGroupBox *groupBox = qobject_cast<QGroupBox *>(w)) {
        if (groupBox->isCheckable())
            _widgetStateEngine->registerWidget(w, AnimationHover | AnimationFocus);

    } else if (qobject_cast<QScrollBar *>(w)) {
        _scrollBarEngine->registerWidget(w, AnimationHover | AnimationFocus);
    } else if (qobject_cast<QSlider *>(w)) {
        _widgetStateEngine->registerWidget(w, AnimationHover | AnimationFocus);
    } else if (qobject_cast<QDial *>(w)) {
        _dialEngine->registerWidget(w, AnimationHover | AnimationFocus);
    } else if (qobject_cast<QMenu *>(w)) {
        _widgetStateEngine->registerWidget(w, AnimationHover);
    } else if (qobject_cast<QProgressBar *>(w)) {
        _busyIndicatorEngine->registerWidget(w);
    } else if (qobject_cast<QComboBox *>(w)) {
        _comboBoxEngine->registerWidget(w, AnimationHover);
        _inputWidgetEngine->registerWidget(w, AnimationHover | AnimationFocus);
    } else if (qobject_cast<QSpinBox *>(w)) {
        _spinBoxEngine->registerWidget(w);
        _inputWidgetEngine->registerWidget(w, AnimationHover | AnimationFocus);
    } else if (qobject_cast<QLineEdit *>(w) || qobject_cast<QTextEdit *>(w) || w->inherits("KTextEditor::View")) {
        _inputWidgetEngine->registerWidget(w, AnimationHover | AnimationFocus);
    } else if (qobject_cast<QHeaderView *>(w)) {
        _headerViewEngine->registerWidget(w);
    } else if (qobject_cast<QAbstractItemView *>(w)) {
        _inputWidgetEngine->registerWidget(w, AnimationHover | AnimationFocus);
    } else if (qobject_cast<QTabBar *>(w)) {
        _tabBarEngine->registerWidget(w);
    } else if (QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea *>(w)) {
        if (scrollArea->frameShadow() == QFrame::Sunken && (w->focusPolicy() & Qt::StrongFocus))
            _inputWidgetEngine->registerWidget(w, AnimationHover | AnimationFocus);
    }

    // stacked widgets
    if (QStackedWidget *stack = qobject_cast<QStackedWidget *>(widget)) {
        _stackedWidgetEngine->registerWidget(stack);
    }

    // QTabWidget: ensure tab bar is registered for sliding pill animation (tab bar may not get polished directly)
    if (QTabWidget *tabWidget = qobject_cast<QTabWidget *>(widget)) {
        if (QTabBar *tabBar = tabWidget->tabBar()) {
            _tabBarEngine->registerWidget(tabBar);
        }
    }
}

//____________________________________________________________
void Animations::unregisterWidget(QWidget *widget) const
{
    if (!widget)
        return;

    _widgetEnabilityEngine->unregisterWidget(widget);
    _spinBoxEngine->unregisterWidget(widget);
    _comboBoxEngine->unregisterWidget(widget);
    _busyIndicatorEngine->registerWidget(widget);

    // the following allows some optimization of widget unregistration
    // it assumes that a widget can be registered atmost in one of the
    // engines stored in the list.
    foreach (const BaseEngine::Pointer &engine, _engines) {
        if (engine && engine.data()->unregisterWidget(widget))
            break;
    }
}

//_______________________________________________________________
void Animations::unregisterEngine(QObject *object)
{
    int index(_engines.indexOf(qobject_cast<BaseEngine *>(object)));
    if (index >= 0)
        _engines.removeAt(index);
}

//_______________________________________________________________
void Animations::registerEngine(BaseEngine *engine)
{
    _engines.append(engine);
    connect(engine, &QObject::destroyed, this, &Animations::unregisterEngine);
}

}
