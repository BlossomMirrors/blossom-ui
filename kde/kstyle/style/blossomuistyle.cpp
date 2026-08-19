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

#include "blossomuistyle.h"
#include "private.h"
#include "switchwidget.h"

#include "blossomui.h"
#include "blossomuianimations.h"
#include "blossomuiblurhelper.h"
#include "blossomuiframeshadow.h"
#include "blossomuimdiwindowshadow.h"
#include "blossomuimnemonics.h"
#include "blossomuipropertynames.h"
#include "blossomuishadowhelper.h"
#include "blossomuisplitterproxy.h"
#include "blossomuistyleconfigdata.h"
#include "blossomuitoolsareamanager.h"
#include "blossomuiwidgetexplorer.h"
#include "blossomuiwindowmanager.h"

#include <KColorUtils>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDBusConnection>
#include <QDial>
#include <QDockWidget>
#include <QLabel>
#include <QLineEdit>
#include <QMdiSubWindow>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSplitterHandle>
#include <QTextEdit>
#include <QToolButton>

namespace BlossomUIPrivate {

//* list of possible valid toolbars to be translucent
//* only one can be at time
QSet<const QWidget *> possibleTranslucentToolBars;

} // namespace BlossomUIPrivate

namespace BlossomUI {
Style::Style()
    :

      _helper(new Helper(StyleConfigData::self()->sharedConfig())),
      _shadowHelper(new ShadowHelper(this, *_helper)),
      _animations(new Animations(this)), _mnemonics(new Mnemonics(this)),
      _blurHelper(new BlurHelper(this)),
      _windowManager(new WindowManager(this)),
      _frameShadowFactory(new FrameShadowFactory(this)),
      _mdiWindowShadowFactory(new MdiWindowShadowFactory(this)),
      _splitterFactory(new SplitterFactory(this)),
      _toolsAreaManager(new ToolsAreaManager(_helper, this)),
      _widgetExplorer(new WidgetExplorer(this)),
      _tabBarData(new BlossomUIPrivate::TabBarData(this))
#if BLOSSOMUI_HAVE_KSTYLE
      ,
      SH_ArgbDndWindow(newStyleHint(QStringLiteral("SH_ArgbDndWindow"))),
      CE_CapacityBar(newControlElement(QStringLiteral("CE_CapacityBar")))
#endif
{
  // use DBus connection to update on blossomui configuration change
  auto dbus = QDBusConnection::sessionBus();
  dbus.connect(QString(), QStringLiteral("/BlossomUIStyle"),
               QStringLiteral("org.blossomos.ui.style"),
               QStringLiteral("reparseConfiguration"), this,
               SLOT(configurationChanged()));

  dbus.connect(QString(), QStringLiteral("/BlossomUIDecoration"),
               QStringLiteral("org.blossomos.ui.style"),
               QStringLiteral("reparseConfiguration"), this,
               SLOT(configurationChanged()));

  dbus.connect(QString(), QStringLiteral("/KGlobalSettings"),
               QStringLiteral("org.kde.KGlobalSettings"),
               QStringLiteral("notifyChange"), this,
               SLOT(configurationChanged()));

  dbus.connect(QString(), QStringLiteral("/KWin"),
               QStringLiteral("org.kde.KWin"), QStringLiteral("reloadConfig"),
               this, SLOT(configurationChanged()));
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  this->addEventFilter(qApp);
#else
  connect(qApp, &QApplication::paletteChanged, this,
          &Style::configurationChanged);
#endif
  // call the slot directly; this initial call will set up things that also
  // need to be reset when the system palette changes
  loadConfiguration();
}

Style::~Style() {
  delete _shadowHelper;
  delete _helper;
}

void Style::polish(QApplication *app) {
  const QString appName = app->applicationName();
  if (appName == "Qt-subapplication")
    _app.subApp = true;
  else if (appName == "soffice.bin")
    _app.isLibreoffice = true;
  else if (appName == "dolphin")
    _app.isDolphin = true;
  else if (appName == "konsole")
    _app.isKonsole = true;
  else if (appName == "kdevelop")
    _app.isKdevelop = true;
  else if (appName == "plasma" || appName.startsWith("plasma-") ||
           appName == "plasmashell" // Plasma5
           || appName == "kded4")   // this is for the infamous appmenu
    _app.isPlasma = true;

  if (StyleConfigData::opaqueApps().contains(appName, Qt::CaseInsensitive) ||
      StyleConfigData::forceOpaque().contains(appName, Qt::CaseInsensitive))
    _app.isOpaque = true;

  const qreal dpr = qApp->devicePixelRatio();
  bool nonIntegerScale =
      (dpr > static_cast<qreal>(1) && static_cast<qreal>(qRound(dpr)) != dpr);
  if (nonIntegerScale)
    _app.isOpaque = true;
  if (_translucentWidgets.size() > 0)
    _translucentWidgets.clear();

  _toolsAreaManager->registerApplication(app);

  // base class polishing
  ParentStyleClass::polish(app);
}

void Style::polish(QWidget *widget) {
  if (!widget)
    return;

  // register widget to animations
  _animations->registerWidget(widget);
  _windowManager->registerWidget(widget);
  _frameShadowFactory->registerWidget(widget, *_helper);
  _mdiWindowShadowFactory->registerWidget(widget);
  _shadowHelper->registerWidget(widget);
  _splitterFactory->registerWidget(widget);
  _toolsAreaManager->registerWidget(widget);

  // enable mouse over effects for all necessary widgets
  if (qobject_cast<QAbstractItemView *>(widget) ||
      qobject_cast<QAbstractSpinBox *>(widget) ||
      qobject_cast<QCheckBox *>(widget) || qobject_cast<QComboBox *>(widget) ||
      qobject_cast<QDial *>(widget) || qobject_cast<QLineEdit *>(widget) ||
      qobject_cast<QPushButton *>(widget) ||
      qobject_cast<QRadioButton *>(widget) ||
      qobject_cast<QScrollBar *>(widget) || qobject_cast<QSlider *>(widget) ||
      qobject_cast<QSplitterHandle *>(widget) ||
      qobject_cast<QTabBar *>(widget) || qobject_cast<QTextEdit *>(widget) ||
      qobject_cast<QToolButton *>(widget) ||
      widget->inherits("KTextEditor::View")) {
    widget->setAttribute(Qt::WA_Hover);
  }

  polishLineEditIconButton(widget);
  polishButton(widget);
  polishCheckableHover(widget);
  polishComboBoxHover(widget);
  polishSwitchCheckBox(widget);
  polishComboBoxPopupViewBackground(widget);

  // enforce translucency for drag and drop window
  if (widget->testAttribute(Qt::WA_X11NetWmWindowTypeDND) &&
      _helper->compositingActive()) {
    widget->setAttribute(Qt::WA_TranslucentBackground);
    widget->clearMask();
  }

  polishOpaqueBar(widget);

  if (!_app.isKonsole) {
    if (StyleConfigData::toolBarOpacity() < 100 ||
        StyleConfigData::menuBarOpacity() < 100 ||
        StyleConfigData::tabBarOpacity() < 100 ||
        StyleConfigData::dolphinSidebarOpacity() < 100) {
      _app.isBarsOpaque = true;
    }
  }

  polishDolphinView(widget);

  if (_app.isDolphin && widget->inherits("DolphinUrlNavigator"))
    polishDolphinUrlNavigator(widget);

  // translucent (window) color scheme support
  switch (widget->windowFlags() & Qt::WindowType_Mask) {
  case Qt::Window:
  case Qt::Dialog:
  case Qt::Popup:
  case Qt::ToolTip:
  case Qt::Sheet: {
    if (_app.isOpaque)
      break;
    if (qobject_cast<QMenu *>(widget))
      break;

    else if (widget->inherits("QTipLabel") ||
             qobject_cast<QLabel *>(widget) // a floating label, as in Filelight
             || widget->inherits("QComboBoxPrivateContainer") // at most, a menu
             /* like Vokoscreen's (old) QvkRegionChoise */
             ||
             (widget->windowFlags().testFlag(Qt::WindowStaysOnTopHint) &&
              widget->testAttribute(Qt::WA_NoSystemBackground) &&
              ((widget->windowFlags() & Qt::WindowType_Mask) == Qt::ToolTip ||
               (widget->windowState() & Qt::WindowFullScreen)))) {
      break;
    }

    if (!_helper->shouldWindowHaveAlpha(widget->palette(), _app.isDolphin) ||
        _app.isOpaque) {
      // register blur is required even in konsole
      if (!_app.isBarsOpaque) {
        break;
      }
    }

    /* take all precautions */
    if (!_app.subApp && !_app.isLibreoffice && widget->isWindow() &&
        !widget->testAttribute(Qt::WA_PaintOnScreen) &&
        !widget->testAttribute(Qt::WA_X11NetWmWindowTypeDesktop) &&
        !widget->inherits("KScreenSaver") &&
        !widget->inherits("QSplashScreen")) {
      if (!_helper->compositingActive())
        break;
      if (widget->windowFlags().testFlag(Qt::FramelessWindowHint))
        break;

      // konsole handle blur and translucency for menubar/toolbar/tabbar
      if (_app.isKonsole) {
        _translucentWidgets.insert(widget);
        if (widget->palette().color(widget->backgroundRole()).alpha() < 255 ||
            _helper->titleBarColor(true).alphaF() * 100.0 < 100 ||
            _app.isBarsOpaque) {
          // stop flickering on translucent background
          widget->setAttribute(Qt::WA_NoSystemBackground, false);
        }

        // paint the background in event filter
        addEventFilter(widget);
        break;
      }

      // make window translucent
      if (!widget->testAttribute(Qt::WA_TranslucentBackground))
        widget->setAttribute(Qt::WA_TranslucentBackground);

      if (!widget->testAttribute(Qt::WA_StyledBackground))
        widget->setAttribute(Qt::WA_StyledBackground);

      _translucentWidgets.insert(widget);

      // paint the background in event filter
      addEventFilter(widget);

      // blur
      if (widget->palette().color(widget->backgroundRole()).alpha() < 255 ||
          _helper->titleBarColor(true).alphaF() * 100.0 < 100 ||
          (StyleConfigData::dolphinSidebarOpacity() < 100 && _app.isDolphin)) {
        _blurHelper->registerWidget(widget, _app.isDolphin);
      }
    }
  }
  }

  polishDolphinViewAutofill(widget);
  polishQuickWidget(widget);

  // scrollarea polishing is somewhat complex. It is moved to a dedicated method
  polishScrollArea(qobject_cast<QAbstractScrollArea *>(widget));

  if (polishItemView(widget)) {
  } else if (polishCheckableGroupBox(widget)) {
  } else if (polishDockWidgetButton(widget)) {
  } else if (polishToolBoxButton(widget)) {
  } else if (polishTitleWidgetFrame(widget)) {
  }

  if (polishScrollBarOpaque(widget)) {
  } else if (polishKTextEditorView(widget)) {
  } else if (polishAutoRaiseToolButton(widget)) {
  } else if (polishDockWidget(widget)) {
  } else if (polishMdiSubWindow(widget)) {
  } else if (polishToolBox(widget)) {
  } else if (polishToolBoxChild(widget)) {
  } else if (polishMenu(widget)) {
  } else if (polishCommandLinkButton(widget)) {
  } else if (polishComboBoxListViewChild(widget)) {
  } else if (polishComboBox(widget)) {
  } else if (polishComboBoxPopupContainer(widget)) {
  } else if (polishTipLabel(widget)) {
  } else if (polishMainWindow(widget)) {
  } else if (polishDialogButtonBox(widget)) {
    // opaque menubar / toolbar / tabbar register blur
  } else if (_app.isBarsOpaque) {
    _blurHelper->registerWidget(widget->window(), _app.isDolphin);
  }

  polishKPageViewHeaders(widget);

  // base class polishing
  ParentStyleClass::polish(widget);
}

void Style::unpolish(QWidget *widget) {
  // register widget to animations
  _animations->unregisterWidget(widget);
  _frameShadowFactory->unregisterWidget(widget);
  _mdiWindowShadowFactory->unregisterWidget(widget);
  _shadowHelper->unregisterWidget(widget);
  _windowManager->unregisterWidget(widget);
  _splitterFactory->unregisterWidget(widget);
  _blurHelper->unregisterWidget(widget);
  _toolsAreaManager->unregisterWidget(widget);

  // remove event filter
  if (qobject_cast<QAbstractScrollArea *>(widget) ||
      qobject_cast<QDockWidget *>(widget) ||
      qobject_cast<QMdiSubWindow *>(widget) ||
      qobject_cast<QPushButton *>(widget) ||
      qobject_cast<QToolButton *>(widget) ||
      qobject_cast<QCheckBox *>(widget) ||
      qobject_cast<QRadioButton *>(widget) ||
      qobject_cast<QComboBox *>(widget) ||
      widget->inherits("QComboBoxPrivateContainer")) {
    widget->removeEventFilter(this);
  }

  // reset cursor set in polish() for menus
  unpolishMenu(widget);
  unpolishSwitchCheckBox(widget);

  if (_translucentWidgets.contains(widget)) {
    widget->setAttribute(Qt::WA_NoSystemBackground, false);
    widget->setAttribute(Qt::WA_TranslucentBackground, false);
    _translucentWidgets.remove(widget);
    widget->removeEventFilter(this);
  }

  unpolishOpaqueBar(widget);

  ParentStyleClass::unpolish(widget);
}

void Style::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                          QPainter *painter, const QWidget *widget) const {
  StylePrimitive fcn;
  switch (element) {
  case PE_PanelButtonCommand:
    fcn = &Style::drawPanelButtonCommandPrimitive;
    break;
  case PE_PanelButtonTool:
    fcn = &Style::drawPanelButtonToolPrimitive;
    break;
  case PE_PanelScrollAreaCorner:
    fcn = &Style::drawPanelScrollAreaCornerPrimitive;
    break;
  case PE_PanelMenu:
    fcn = &Style::drawPanelMenuPrimitive;
    break;
  case PE_PanelMenuBar:
    fcn = &Style::emptyPrimitive;
    break;
  case PE_PanelTipLabel:
    fcn = &Style::drawPanelTipLabelPrimitive;
    break;
  case PE_PanelItemViewItem:
    fcn = &Style::drawPanelItemViewItemPrimitive;
    break;
  case PE_IndicatorCheckBox:
    fcn = &Style::drawIndicatorCheckBoxPrimitive;
    break;
  case PE_IndicatorRadioButton:
    fcn = &Style::drawIndicatorRadioButtonPrimitive;
    break;
  case PE_IndicatorButtonDropDown:
    fcn = &Style::drawIndicatorButtonDropDownPrimitive;
    break;
  case PE_IndicatorTabClose:
    fcn = &Style::drawIndicatorTabClosePrimitive;
    break;
  case PE_IndicatorTabTear:
    fcn = &Style::drawIndicatorTabTearPrimitive;
    break;
  case PE_IndicatorArrowUp:
    fcn = &Style::drawIndicatorArrowUpPrimitive;
    break;
  case PE_IndicatorArrowDown:
    fcn = &Style::drawIndicatorArrowDownPrimitive;
    break;
  case PE_IndicatorArrowLeft:
    fcn = &Style::drawIndicatorArrowLeftPrimitive;
    break;
  case PE_IndicatorArrowRight:
    fcn = &Style::drawIndicatorArrowRightPrimitive;
    break;
  case PE_IndicatorHeaderArrow:
    fcn = &Style::drawIndicatorHeaderArrowPrimitive;
    break;
  case PE_IndicatorToolBarHandle:
    fcn = &Style::drawIndicatorToolBarHandlePrimitive;
    break;
  case PE_IndicatorToolBarSeparator:
    fcn = &Style::drawIndicatorToolBarSeparatorPrimitive;
    break;
  case PE_IndicatorBranch:
    fcn = &Style::drawIndicatorBranchPrimitive;
    break;
  case PE_FrameStatusBarItem:
    fcn = &Style::emptyPrimitive;
    break;
  case PE_Frame:
    fcn = &Style::drawFramePrimitive;
    break;
  case PE_FrameLineEdit:
    fcn = &Style::drawFrameLineEditPrimitive;
    break;
  case PE_FrameMenu:
    fcn = &Style::drawFrameMenuPrimitive;
    break;
  case PE_FrameGroupBox:
    fcn = &Style::drawFrameGroupBoxPrimitive;
    break;
  case PE_FrameTabWidget:
    fcn = &Style::drawFrameTabWidgetPrimitive;
    break;
  case PE_FrameTabBarBase:
    fcn = &Style::drawFrameTabBarBasePrimitive;
    break;
  case PE_FrameWindow:
    fcn = &Style::drawFrameWindowPrimitive;
    break;
  case PE_FrameFocusRect:
    fcn = _frameFocusPrimitive;
    break;
  case PE_Widget:
    fcn = &Style::drawWidgetPrimitive;
    break;

  // fallback
  default:
    break;
  }

  painter->save();

  // call function if implemented
  if (!(fcn && fcn(*this, option, painter, widget))) {
    ParentStyleClass::drawPrimitive(element, option, painter, widget);
  }

  painter->restore();
}

bool Style::drawWidgetPrimitive(const QStyleOption *option, QPainter *painter,
                                const QWidget *widget) const {
  if (drawMainWindowToolsAreaPrimitive(option, painter, widget)) {
  } else if (drawDialogHeaderSeparatorPrimitive(option, painter, widget)) {
  } else if (drawMultiTabBarSeparatorPrimitive(option, painter, widget)) {
  }
  return true;
}

void Style::drawControl(ControlElement element, const QStyleOption *option,
                        QPainter *painter, const QWidget *widget) const {
  StyleControl fcn;

#if BLOSSOMUI_HAVE_KSTYLE
  if (element == CE_CapacityBar) {
    fcn = &Style::drawProgressBarControl;

  } else
#endif

    switch (element) {
    case CE_PushButtonBevel:
      fcn = &Style::drawPanelButtonCommandPrimitive;
      break;
    case CE_PushButtonLabel:
      fcn = &Style::drawPushButtonLabelControl;
      break;
    case CE_CheckBoxLabel:
      fcn = &Style::drawCheckBoxLabelControl;
      break;
    case CE_RadioButtonLabel:
      fcn = &Style::drawCheckBoxLabelControl;
      break;
    case CE_ToolButtonLabel:
      fcn = &Style::drawToolButtonLabelControl;
      break;
    case CE_ComboBoxLabel:
      fcn = &Style::drawComboBoxLabelControl;
      break;
    case CE_MenuBarEmptyArea:
      fcn = &Style::drawMenuBarEmptyAreaControl;
      break;
    case CE_MenuBarItem:
      fcn = &Style::drawMenuBarItemControl;
      break;
    case CE_MenuItem:
      fcn = &Style::drawMenuItemControl;
      break;
    case CE_ToolBar:
      fcn = &Style::drawToolBarBackgroundControl;
      break;
    case CE_ProgressBar:
      fcn = &Style::drawProgressBarControl;
      break;
    case CE_ProgressBarContents:
      fcn = &Style::drawProgressBarContentsControl;
      break;
    case CE_ProgressBarGroove:
      fcn = &Style::drawProgressBarGrooveControl;
      break;
    case CE_ProgressBarLabel:
      fcn = &Style::drawProgressBarLabelControl;
      break;
    case CE_ScrollBarSlider:
      fcn = &Style::drawScrollBarSliderControl;
      break;
    case CE_ScrollBarAddLine:
      fcn = &Style::drawScrollBarAddLineControl;
      break;
    case CE_ScrollBarSubLine:
      fcn = &Style::drawScrollBarSubLineControl;
      break;
    case CE_ScrollBarAddPage:
      fcn = &Style::emptyControl;
      break;
    case CE_ScrollBarSubPage:
      fcn = &Style::emptyControl;
      break;
    case CE_ShapedFrame:
      fcn = &Style::drawShapedFrameControl;
      break;
    case CE_RubberBand:
      fcn = &Style::drawRubberBandControl;
      break;
    case CE_SizeGrip:
      fcn = &Style::emptyControl;
      break;
    case CE_HeaderSection:
      fcn = &Style::drawHeaderSectionControl;
      break;
    case CE_HeaderEmptyArea:
      fcn = &Style::drawHeaderEmptyAreaControl;
      break;
    case CE_TabBarTabLabel:
      fcn = &Style::drawTabBarTabLabelControl;
      break;
    case CE_TabBarTabShape:
      fcn = &Style::drawTabBarTabShapeControl;
      break;
    case CE_ToolBoxTabLabel:
      fcn = &Style::drawToolBoxTabLabelControl;
      break;
    case CE_ToolBoxTabShape:
      fcn = &Style::drawToolBoxTabShapeControl;
      break;
    case CE_DockWidgetTitle:
      fcn = &Style::drawDockWidgetTitleControl;
      break;
    case CE_ItemViewItem:
      fcn = &Style::drawItemViewItemControl;
      break;

    // fallback
    default:
      break;
    }

  painter->save();

  // call function if implemented
  if (!(fcn && fcn(*this, option, painter, widget))) {
    ParentStyleClass::drawControl(element, option, painter, widget);
  }

  painter->restore();
}

void Style::drawComplexControl(ComplexControl element,
                               const QStyleOptionComplex *option,
                               QPainter *painter, const QWidget *widget) const {
  StyleComplexControl fcn;
  switch (element) {
  case CC_GroupBox:
    fcn = &Style::drawGroupBoxComplexControl;
    break;
  case CC_ToolButton:
    fcn = &Style::drawToolButtonComplexControl;
    break;
  case CC_ComboBox:
    fcn = &Style::drawComboBoxComplexControl;
    break;
  case CC_SpinBox:
    fcn = &Style::drawSpinBoxComplexControl;
    break;
  case CC_Slider:
    fcn = &Style::drawSliderComplexControl;
    break;
  case CC_Dial:
    fcn = &Style::drawDialComplexControl;
    break;
  case CC_ScrollBar:
    fcn = &Style::drawScrollBarComplexControl;
    break;
  case CC_TitleBar:
    fcn = &Style::drawTitleBarComplexControl;
    break;

  // fallback
  default:
    break;
  }

  painter->save();

  // call function if implemented
  if (!(fcn && fcn(*this, option, painter, widget))) {
    ParentStyleClass::drawComplexControl(element, option, painter, widget);
  }

  painter->restore();
}

void Style::drawItemText(QPainter *painter, const QRect &rect, int flags,
                         const QPalette &palette, bool enabled,
                         const QString &text,
                         QPalette::ColorRole textRole) const {
  // hide mnemonics if requested
  if (!_mnemonics->enabled() && (flags & Qt::TextShowMnemonic) &&
      !(flags & Qt::TextHideMnemonic)) {
    flags &= ~Qt::TextShowMnemonic;
    flags |= Qt::TextHideMnemonic;
  }

  // make sure vertical alignment is defined
  // fallback on Align::VCenter if not
  if (!(flags & Qt::AlignVertical_Mask))
    flags |= Qt::AlignVCenter;

  if (_animations->widgetEnabilityEngine().enabled()) {
    /*
     * check if painter engine is registered to WidgetEnabilityEngine, and
     * animated if yes, merge the palettes. Note: a static_cast is safe here,
     * since only the address of the pointer is used, not the actual content.
     */
    const QWidget *widget(static_cast<const QWidget *>(painter->device()));
    if (_animations->widgetEnabilityEngine().isAnimated(widget,
                                                        AnimationEnable)) {
      const QPalette copy(_helper->disabledPalette(
          palette, _animations->widgetEnabilityEngine().opacity(
                       widget, AnimationEnable)));
      return ParentStyleClass::drawItemText(painter, rect, flags, copy, enabled,
                                            text, textRole);
    }
  }

  // fallback
  return ParentStyleClass::drawItemText(painter, rect, flags, palette, enabled,
                                        text, textRole);
}

void Style::configurationChanged() {
  // reload
  StyleConfigData::self()->load();

  // reload configuration
  loadConfiguration();
}

QIcon Style::standardIconImplementation(StandardPixmap standardPixmap,
                                        const QStyleOption *option,
                                        const QWidget *widget) const {
  // lookup cache
  if (_iconCache.contains(standardPixmap))
    return _iconCache.value(standardPixmap);

  QIcon icon;
  switch (standardPixmap) {
  case SP_TitleBarNormalButton:
  case SP_TitleBarMinButton:
  case SP_TitleBarMaxButton:
  case SP_TitleBarCloseButton:
  case SP_DockWidgetCloseButton:
    icon = titleBarButtonIcon(standardPixmap, option, widget);
    break;

  case SP_ToolBarHorizontalExtensionButton:
  case SP_ToolBarVerticalExtensionButton:
    icon = toolBarExtensionIcon(standardPixmap, option, widget);
    break;

  default:
    break;
  }

  if (icon.isNull()) {
    // do not cache parent style icon, since it may change at runtime
    return ParentStyleClass::standardIcon(standardPixmap, option, widget);

  } else {
    const_cast<IconCache *>(&_iconCache)->insert(standardPixmap, icon);
    return icon;
  }
}

void Style::loadConfiguration() {
  _helper->loadConfig();

  // update blurhelper
  _blurHelper->setTranslucentTitlebar(
      _helper->titleBarColor(true).alphaF() < 1.0 ? true : false);

  // reinitialize engines
  _animations->setupEngines();
  _windowManager->initialize();

  // mnemonics
  _mnemonics->setMode(StyleConfigData::mnemonicsMode());

  // splitter proxy
  _splitterFactory->setEnabled(StyleConfigData::splitterProxyEnabled());

  // reset shadow tiles
  _shadowHelper->loadConfig();

  // set mdiwindow factory shadow tiles
  _mdiWindowShadowFactory->setShadowHelper(_shadowHelper);

  // clear icon cache
  _iconCache.clear();

  // scrollbar buttons
  switch (StyleConfigData::scrollBarAddLineButtons()) {
  case 0:
    _addLineButtons = NoButton;
    break;
  case 1:
    _addLineButtons = SingleButton;
    break;

  default:
  case 2:
    _addLineButtons = DoubleButton;
    break;
  }

  switch (StyleConfigData::scrollBarSubLineButtons()) {
  case 0:
    _subLineButtons = NoButton;
    break;
  case 1:
    _subLineButtons = SingleButton;
    break;

  default:
  case 2:
    _subLineButtons = DoubleButton;
    break;
  }

  // frame focus
  if (StyleConfigData::viewDrawFocusIndicator())
    _frameFocusPrimitive = &Style::drawFrameFocusRectPrimitive;
  else
    _frameFocusPrimitive = &Style::emptyPrimitive;

  // widget explorer
  _widgetExplorer->setEnabled(StyleConfigData::widgetExplorerEnabled());
  _widgetExplorer->setDrawWidgetRects(StyleConfigData::drawWidgetRects());
}

} // namespace BlossomUI
