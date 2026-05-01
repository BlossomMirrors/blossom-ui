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
#include "widgets/switch.h"

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

#include <QAccessible>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDBusConnection>
#include <QDial>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QDragMoveEvent>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsView>
#include <QGroupBox>
#include <QItemDelegate>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMap>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPainterPath>
#include <QPolygon>
#include <QPushButton>
#include <QQuickWidget>
#include <QRadioButton>
#include <QRegion>
#include <QScrollBar>
#include <QSplitterHandle>
#include <QStackedLayout>
#include <QStorageInfo>
#include <QStyledItemDelegate>
#include <QSurfaceFormat>
#include <QTableView>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QToolBox>
#include <QToolButton>
#include <QTreeView>
#include <QWidgetAction>
#include <QWindow>

#if BLOSSOMUI_HAVE_QTQUICK
#include <QQuickWindow>
#endif

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
               QStringLiteral("org.kde.BlossomUI.Style"),
               QStringLiteral("reparseConfiguration"), this,
               SLOT(configurationChanged()));

  dbus.connect(QString(), QStringLiteral("/BlossomUIDecoration"),
               QStringLiteral("org.kde.BlossomUI.Style"),
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

class ParentResizeFilter : public QObject {
public:
  explicit ParentResizeFilter(QWidget *overlay)
      : QObject(overlay), m_overlay(overlay) {}

protected:
  bool eventFilter(QObject *watched, QEvent *event) override {
    if (event->type() == QEvent::Resize && m_overlay &&
        m_overlay->parentWidget() == watched) {
      m_overlay->setGeometry(m_overlay->parentWidget()->rect());
      m_overlay->update();
    }
    // call base implementation
    return QObject::eventFilter(watched, event);
  }

private:
  QWidget *m_overlay;
};

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

  if (widget->inherits("QLineEditIconButton")) {
    widget->setAttribute(Qt::WA_Hover);
    addEventFilter(widget);
  }

  if (qobject_cast<QPushButton *>(widget) ||
      qobject_cast<QToolButton *>(widget)) {
    addEventFilter(widget);
    QObject::connect(
        static_cast<QAbstractButton *>(widget), &QAbstractButton::pressed,
        widget, [widget]() { widget->repaint(); }, Qt::DirectConnection);
  }

  // switch (pill) checkboxes: replace indicator with actual switch widget
  // (draggable)
  if (auto checkBox = qobject_cast<QCheckBox *>(widget)) {
    if (isSwitchWidget(checkBox)) {
      QStyleOptionButton opt;
      opt.initFrom(checkBox);
      opt.rect = checkBox->rect();
      opt.text = checkBox->text();
      opt.icon = checkBox->icon();
      opt.iconSize = checkBox->iconSize();
      QRect indRect = subElementRect(SE_CheckBoxIndicator, &opt, checkBox);
      BlossomUISwitchWidget *overlay =
          new BlossomUISwitchWidget(_helper, checkBox);
      overlay->setParent(checkBox);
      overlay->setGeometry(indRect);
      overlay->show();
      overlay->raise(); // ensure overlay is on top of any other children (e.g.
                        // label)
      checkBox->setProperty("blossomui-switch-overlay",
                            QVariant::fromValue<QObject *>(overlay));
      addEventFilter(checkBox);
      // update geometry after layout (checkbox rect may not be final at polish
      // time)
      QTimer::singleShot(0, checkBox, [this, checkBox]() {
        QObject *ov =
            checkBox->property("blossomui-switch-overlay").value<QObject *>();
        auto *o = qobject_cast<BlossomUISwitchWidget *>(ov);
        if (o) {
          QStyleOptionButton opt;
          opt.initFrom(checkBox);
          opt.rect = checkBox->rect();
          opt.text = checkBox->text();
          opt.icon = checkBox->icon();
          opt.iconSize = checkBox->iconSize();
          o->setGeometry(subElementRect(SE_CheckBoxIndicator, &opt, checkBox));
        }
      });
    }
  }

  // remove auto background from combobox popup
  if (auto view = const_cast<QAbstractItemView *>(itemViewParent(widget))) {
    QWidget *w = view;
    while (w) {
      if (qobject_cast<QComboBox *>(w)) {
        view->setAutoFillBackground(false);
        view->viewport()->setAutoFillBackground(false);
        break;
      }
      w = w->parentWidget();
    }
  }

  // enforce translucency for drag and drop window
  if (widget->testAttribute(Qt::WA_X11NetWmWindowTypeDND) &&
      _helper->compositingActive()) {
    widget->setAttribute(Qt::WA_TranslucentBackground);
    widget->clearMask();
  }

  if ((qobject_cast<QToolBar *>(widget) || qobject_cast<QMenuBar *>(widget)) &&
      _helper->titleBarColor(true).alphaF() * 100.0 < 100) {
    // only accept top most widgets, besides the main window
    if (!widget->isWindow() && widget->parentWidget()->isWindow()) {
      // this is only valid if the window is opaque (but not forced), otherwise
      // everything will be blurred
      if (widget->palette().color(QPalette::Window).alpha() == 255 &&
          !_app.isOpaque)
        addEventFilter(widget);
    }
  }

  if (!_app.isKonsole) {
    if (StyleConfigData::toolBarOpacity() < 100 ||
        StyleConfigData::menuBarOpacity() < 100 ||
        StyleConfigData::tabBarOpacity() < 100 ||
        StyleConfigData::dolphinSidebarOpacity() < 100) {
      _app.isBarsOpaque = true;
    }
  }

  if (_app.isDolphin && widget->inherits("DolphinView")) {
    widget->setContentsMargins(0, 0, 0, 0);
    if (auto layout = widget->layout())
      layout->setContentsMargins(0, 0, 0, 0);
  }

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

      // setting Qt::WA_TranslucentBackground enables Qt::WA_NoSystemBackground
      // unset here to stop flickering during repaint events on resizing
      if (StyleConfigData::transparentDolphinView() &&
          widget->testAttribute(Qt::WA_NoSystemBackground))
        widget->setAttribute(Qt::WA_NoSystemBackground, false);

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

  // hack Dolphin's view
  if ((_app.isDolphin &&
       qobject_cast<QAbstractScrollArea *>(getParent(widget, 2)) &&
       !qobject_cast<QAbstractScrollArea *>(getParent(widget, 3)))) {
    if (widget->autoFillBackground())
      widget->setAutoFillBackground(false);
  }

  if (widget->inherits("QQuickWidget")) {
    // Check if it is a child of FocusHackWidget
    QWidget *parent = widget->parentWidget();
    bool isChildOfFocusHack = false;
    while (parent) {
      if (parent->inherits("FocusHackWidget")) {
        isChildOfFocusHack = true;
        break;
      }
      parent = parent->parentWidget();
    }

    if (isChildOfFocusHack) {
      auto quickWidget = qobject_cast<QQuickWidget *>(widget);
      if (quickWidget) {
        quickWidget->setClearColor(Qt::transparent);
      }

      widget->setAttribute(Qt::WA_TranslucentBackground);
      widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
    }
  }

  // scrollarea polishing is somewhat complex. It is moved to a dedicated method
  polishScrollArea(qobject_cast<QAbstractScrollArea *>(widget));

  if (auto itemView = qobject_cast<QAbstractItemView *>(widget)) {
    // enable mouse over effects in itemviews' viewport
    itemView->viewport()->setAttribute(Qt::WA_Hover);

  } else if (auto groupBox = qobject_cast<QGroupBox *>(widget)) {
    // checkable group boxes
    if (groupBox->isCheckable()) {
      groupBox->setAttribute(Qt::WA_Hover);
    }

  } else if (qobject_cast<QAbstractButton *>(widget) &&
             qobject_cast<QDockWidget *>(widget->parent())) {
    widget->setAttribute(Qt::WA_Hover);

  } else if (qobject_cast<QAbstractButton *>(widget) &&
             qobject_cast<QToolBox *>(widget->parent())) {
    widget->setAttribute(Qt::WA_Hover);

  } else if (qobject_cast<QFrame *>(widget) && widget->parent() &&
             widget->parent()->inherits("KTitleWidget")) {
    widget->setAutoFillBackground(false);
    if (!StyleConfigData::titleWidgetDrawFrame()) {
      widget->setBackgroundRole(QPalette::Window);
    }
  }

  if (qobject_cast<QScrollBar *>(widget)) {
    // remove opaque painting for scrollbars
    widget->setAttribute(Qt::WA_OpaquePaintEvent, false);

  } else if (widget->inherits("KTextEditor::View")) {
    addEventFilter(widget);

  } else if (auto toolButton = qobject_cast<QToolButton *>(widget)) {
    if (toolButton->autoRaise()) {
      // for flat toolbuttons, adjust foreground and background role accordingly
      widget->setBackgroundRole(QPalette::NoRole);
      widget->setForegroundRole(QPalette::WindowText);
    }

    if (widget->parentWidget() && widget->parentWidget()->parentWidget() &&
        widget->parentWidget()->parentWidget()->inherits(
            "Gwenview::SideBarGroup")) {
      widget->setProperty(PropertyNames::toolButtonAlignment, Qt::AlignLeft);
    }

  } else if (qobject_cast<QDockWidget *>(widget)) {
    // add event filter on dock widgets
    // and alter palette
    widget->setAutoFillBackground(false);
    widget->setContentsMargins(
        StyleConfigData::fancyMargins() ? 5 : Metrics::Frame_FrameWidth,
        Metrics::Frame_FrameWidth,
        StyleConfigData::fancyMargins() ? 5 : Metrics::Frame_FrameWidth,
        Metrics::Frame_FrameWidth);
    addEventFilter(widget);

  } else if (qobject_cast<QMdiSubWindow *>(widget)) {
    widget->setAutoFillBackground(false);
    addEventFilter(widget);

  } else if (qobject_cast<QToolBox *>(widget)) {
    widget->setBackgroundRole(QPalette::NoRole);
    widget->setAutoFillBackground(false);

  } else if (widget->parentWidget() && widget->parentWidget()->parentWidget() &&
             qobject_cast<QToolBox *>(
                 widget->parentWidget()->parentWidget()->parentWidget())) {
    widget->setBackgroundRole(QPalette::NoRole);
    widget->setAutoFillBackground(false);
    widget->parentWidget()->setAutoFillBackground(false);

  } else if (qobject_cast<QMenu *>(widget)) {
    setTranslucentBackground(widget);

    if (widget->testAttribute(Qt::WA_TranslucentBackground) &&
        StyleConfigData::menuOpacity() < 100) {
      _blurHelper->registerWidget(widget->window(), _app.isDolphin);
    }

  } else if (qobject_cast<QCommandLinkButton *>(widget)) {
    addEventFilter(widget);

  } else if (widget->parent() &&
             widget->parent()->inherits("QComboBoxListView")) {
    widget->setAutoFillBackground(false);

  } else if (auto comboBox = qobject_cast<QComboBox *>(widget)) {
    if (!hasParent(widget, "QWebView")) {
      auto itemView(comboBox->view());
      if (itemView && itemView->itemDelegate()) {
        const QByteArray delegateClass =
            itemView->itemDelegate()->metaObject()->className();
        const bool isDefaultDelegate =
            itemView->itemDelegate()->inherits("QComboBoxDelegate") ||
            delegateClass == "QStyledItemDelegate";
        if (isDefaultDelegate) {
          itemView->setItemDelegate(
              new BlossomUIPrivate::ComboBoxItemDelegate(itemView));
        }
      }
    }
  } else if (widget->inherits("QComboBoxPrivateContainer")) {
    addEventFilter(widget);
    setTranslucentBackground(widget);

  } else if (widget->inherits("QTipLabel")) {
    setTranslucentBackground(widget);

  } else if (qobject_cast<QMainWindow *>(widget)) {
    widget->setAttribute(Qt::WA_StyledBackground);
  } else if (qobject_cast<QDialogButtonBox *>(widget)) {
    addEventFilter(widget);
    // opaque menubar / toolbar / tabbar register blur
  } else if (_app.isBarsOpaque) {
    _blurHelper->registerWidget(widget->window(), _app.isDolphin);
  }

  if (_toolsAreaManager->hasHeaderColors()) {
    // style TitleWidget and Search KPageView to look the same as KDE System
    // Settings
    if (widget->objectName() == QLatin1String("KPageView::TitleWidget")) {
      widget->setAutoFillBackground(true);
      widget->setPalette(_toolsAreaManager->palette());
      addEventFilter(widget);
    } else if (widget->objectName() == QLatin1String("KPageView::Search")) {
      widget->setBackgroundRole(QPalette::Window);
      widget->setPalette(_toolsAreaManager->palette());
      addEventFilter(widget);
    }
  }

  // base class polishing
  ParentStyleClass::polish(widget);
}

void Style::polishScrollArea(QAbstractScrollArea *scrollArea) {
  // check argument
  if (!scrollArea)
    return;

  // enable mouse over effect in sunken scrollareas that support focus
  if (scrollArea->frameShadow() == QFrame::Sunken &&
      scrollArea->focusPolicy() & Qt::StrongFocus) {
    scrollArea->setAttribute(Qt::WA_Hover);
  }

  if (scrollArea->frameShape() == QFrame::StyledPanel &&
      scrollArea->frameShadow() == QFrame::Sunken &&
      !qobject_cast<QAbstractItemView *>(scrollArea)) {
    scrollArea->setAutoFillBackground(false);
    if (scrollArea->viewport())
      scrollArea->viewport()->setAutoFillBackground(false);
  }

  if (scrollArea->viewport() && scrollArea->inherits("KItemListContainer")) {
    if (auto *frame = qobject_cast<QFrame *>(scrollArea))
      frame->setFrameShape(QFrame::NoFrame);
    scrollArea->viewport()->setBackgroundRole(QPalette::Window);
    scrollArea->viewport()->setForegroundRole(QPalette::WindowText);
  }

  if (_app.isDolphin)
    polishDolphinScrollArea(scrollArea);

  // add event filter, to make sure proper background is rendered behind
  // scrollbars
  addEventFilter(scrollArea);

  // force side panels as flat, on option
  if (scrollArea->inherits("KDEPrivate::KPageListView") ||
      scrollArea->inherits("KDEPrivate::KPageTreeView")) {
    scrollArea->setProperty(PropertyNames::sidePanelView, true);
  }

  // for all side view panels, unbold font (design choice)
  if (scrollArea->property(PropertyNames::sidePanelView).toBool()) {
    // upbold list font
    auto font(scrollArea->font());
    font.setBold(false);
    scrollArea->setFont(font);

    QWidget *viewPort = scrollArea->findChild<QWidget *>(
        QString("qt_scrollarea_viewport"), Qt::FindDirectChildrenOnly);
    if (viewPort)
      viewPort->setAutoFillBackground(false);
  }

  // disable autofill background for flat (== NoFrame) scrollareas, with
  // QPalette::Window as a background this fixes flat scrollareas placed in a
  // tinted widget, such as groupboxes, tabwidgets or framed dock-widgets
  if (!(scrollArea->frameShape() == QFrame::NoFrame ||
        scrollArea->backgroundRole() == QPalette::Window)) {
    return;
  }

  if (_app.isDolphin && scrollArea->inherits("KItemListContainer"))
    return;

  // get viewport and check background role
  auto viewport(scrollArea->viewport());
  if (!(viewport && viewport->backgroundRole() == QPalette::Window))
    return;

  // change viewport autoFill background.
  // do the same for all children if the background role is QPalette::Window
  viewport->setAutoFillBackground(false);
  QList<QWidget *> children(viewport->findChildren<QWidget *>());
  for (auto *child : std::as_const(children)) {
    if (child->parent() == viewport &&
        child->backgroundRole() == QPalette::Window) {
      child->setAutoFillBackground(false);
    }
  }

  /*
  QTreeView animates expanding/collapsing branches. It paints them into a
  temp pixmap whose background is unconditionally filled with the palette's
  *base* color which is usually different from the window's color
  cf. QTreeViewPrivate::renderTreeToPixmapForAnimation()
  */
  if (auto treeView = qobject_cast<QTreeView *>(scrollArea)) {
    if (treeView->isAnimated()) {
      QPalette pal(treeView->palette());
      pal.setColor(QPalette::Active, QPalette::Base,
                   treeView->palette().color(treeView->backgroundRole()));
      treeView->setPalette(pal);
    }
  }
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
      widget->inherits("QComboBoxPrivateContainer")) {
    widget->removeEventFilter(this);
  }
  if (auto checkBox = qobject_cast<QCheckBox *>(widget)) {
    if (isSwitchWidget(checkBox)) {
      QObject *ov =
          checkBox->property("blossomui-switch-overlay").value<QObject *>();
      if (ov) {
        ov->deleteLater();
        checkBox->setProperty("blossomui-switch-overlay", QVariant());
      }
      widget->removeEventFilter(this);
    }
  }

  if (_translucentWidgets.contains(widget)) {
    widget->setAttribute(Qt::WA_NoSystemBackground, false);
    widget->setAttribute(Qt::WA_TranslucentBackground, false);
    _translucentWidgets.remove(widget);
    widget->removeEventFilter(this);
  }

  if (BlossomUIPrivate::possibleTranslucentToolBars.contains(widget))
    BlossomUIPrivate::possibleTranslucentToolBars.remove(widget);

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
  Q_UNUSED(option)

  const auto drawBackground = _toolsAreaManager->hasHeaderColors() &&
                              _helper->shouldDrawToolsArea(widget);

  auto mw = qobject_cast<const QMainWindow *>(widget);
  if (mw && mw == mw->window()) {
    painter->save();

    auto rect = _toolsAreaManager->toolsAreaRect(*mw);

    if (rect.height() == 0) {
      if (mw->property(PropertyNames::noSeparator).toBool() ||
          mw->isFullScreen()) {
        painter->restore();
        return true;
      }
      painter->setPen(
          QPen(_helper->separatorColor(_toolsAreaManager->palette()),
               PenWidth::Frame * widget->devicePixelRatio()));
      painter->drawLine(widget->rect().topLeft(), widget->rect().topRight());
      painter->restore();
      return true;
    }

    auto color = _toolsAreaManager->palette().brush(
        mw->isActiveWindow() ? QPalette::Active : QPalette::Inactive,
        QPalette::Window);

    if (drawBackground) {
      painter->setPen(Qt::transparent);
      painter->setBrush(color);
      painter->drawRect(rect);
    }

    painter->setPen(_helper->separatorColor(_toolsAreaManager->palette()));
    if (!_app.isDolphin)
      painter->drawLine(rect.bottomLeft(), rect.bottomRight());

    painter->restore();
  } else if (auto dialog = qobject_cast<const QDialog *>(widget)) {
    if (dialog->isFullScreen()) {
      return true;
    }
    if (auto vLayout = qobject_cast<QVBoxLayout *>(widget->layout())) {
      QRect rect(0, 0, widget->width(), 0);
      const auto color = _toolsAreaManager->palette().brush(
          widget->isActiveWindow() ? QPalette::Active : QPalette::Inactive,
          QPalette::Window);

      if (vLayout->menuBar()) {
        rect.setHeight(rect.height() + vLayout->menuBar()->rect().height());
      }

      for (int i = 0, count = vLayout->count(); i < count; i++) {
        const auto layoutItem = vLayout->itemAt(i);
        if (layoutItem->widget() &&
            qobject_cast<QToolBar *>(layoutItem->widget())) {
          rect.setHeight(rect.height() + layoutItem->widget()->rect().height() +
                         vLayout->spacing());
        } else {
          break;
        }
      }

      if (rect.height() > 0) {
        // We found either a QMenuBar or a QToolBar

        // Add contentsMargins + separator
        rect.setHeight(rect.height() + widget->devicePixelRatio() +
                       vLayout->contentsMargins().top());

        if (drawBackground) {
          painter->setPen(Qt::transparent);
          painter->setBrush(color);
          painter->drawRect(rect);
        }

        painter->setPen(
            QPen(_helper->separatorColor(_toolsAreaManager->palette()),
                 widget->devicePixelRatio()));
        painter->drawLine(rect.bottomLeft(), rect.bottomRight());

        return true;
      }
    }

    painter->setPen(QPen(_helper->separatorColor(_toolsAreaManager->palette()),
                         PenWidth::Frame * widget->devicePixelRatio()));
    painter->drawLine(widget->rect().topLeft(), widget->rect().topRight());
  } else if (widget && widget->inherits("KMultiTabBar")) {
    enum class Position {
      Left,
      Right,
      Top,
      Bottom,
    };

    const Position position =
        static_cast<Position>(widget->property("position").toInt());
    const auto splitterWidth = Metrics::Splitter_SplitterWidth;
    QRect rect = option->rect;

    if (position == Position::Top || position == Position::Bottom) {
      return true;
    }

    if ((position == Position::Left &&
         widget->layoutDirection() == Qt::LeftToRight) ||
        (position == Position::Right &&
         widget->layoutDirection() == Qt::RightToLeft)) {
      rect.setX(rect.width() - splitterWidth);
    }

    rect.setWidth(splitterWidth);

    const auto color(_helper->separatorColor(option->palette));
    _helper->renderSeparator(painter, rect, color, true);
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
