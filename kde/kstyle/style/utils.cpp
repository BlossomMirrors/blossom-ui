// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuipropertynames.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "blossomuiwindowmanager.h"
#include "menucontrol.h"
#include "private.h"
#include "spinbox.h"
#include <KColorUtils>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QAbstractItemView>
#include <QAbstractScrollArea>
#include <QApplication>
#include <QFrame>
#include <QGroupBox>
#include <QMainWindow>
#include <QMdiSubWindow>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QStyleOptionSlider>
#include <QStyleOptionSpinBox>
#include <QStyleOptionToolButton>
#include <QTabBar>
#include <QTabWidget>
#include <QToolBar>
#include <QWidgetAction>
#include <QWindow>

#if BLOSSOMUI_HAVE_QTQUICK
#include <QQuickItem>
#endif

namespace BlossomUI {

void Style::renderSpinBoxArrow(const SubControl &subControl,
                               const QStyleOptionSpinBox *option,
                               QPainter *painter, const QWidget *widget) const {
  const auto &palette(option->palette);
  const State &state(option->state);

  // enable state
  bool enabled(state & State_Enabled);

  // check steps enable step
  const bool atLimit(
      (subControl == SC_SpinBoxUp &&
       !(option->stepEnabled & QAbstractSpinBox::StepUpEnabled)) ||
      (subControl == SC_SpinBoxDown &&
       !(option->stepEnabled & QAbstractSpinBox::StepDownEnabled)));

  // update enabled state accordingly
  enabled &= !atLimit;

  // update mouse-over effect
  const bool mouseOver(enabled && (state & State_MouseOver));

  // check animation state
  const bool subControlHover(enabled && mouseOver &&
                             (option->activeSubControls & subControl));
  _animations->spinBoxEngine().updateState(widget, subControl, subControlHover);

  const bool animated(
      enabled && _animations->spinBoxEngine().isAnimated(widget, subControl));
  const qreal opacity(_animations->spinBoxEngine().opacity(widget, subControl));

  const auto color = Render::spinBoxArrowColor(_helper, palette, animated, opacity,
                                               subControlHover, atLimit)
                          .brush.color();

  // arrow orientation
  ArrowOrientation orientation((subControl == SC_SpinBoxUp) ? ArrowUp
                                                            : ArrowDown);

  // arrow rect
  const auto arrowRect(subControlRect(CC_SpinBox, option, subControl, widget));

  // render
  _helper->renderArrow(painter, arrowRect, color, orientation);
}

void Style::renderMenuTitle(const QStyleOptionToolButton *option,
                            QPainter *painter, const QWidget *) const {
  // render a separator at the bottom
  const auto &palette(option->palette);
  const auto color(_helper->separatorColor(palette));
  _helper->renderSeparator(
      painter,
      QRect(option->rect.bottomLeft() -
                QPoint(0, (Render::MenuItem_MarginHeight +
                           StyleConfigData::menuItemHeight())),
            QSize(option->rect.width(), 1)),
      color);

  // render text in the center of the rect
  // icon is discarded on purpose
  painter->setFont(option->font);
  const auto contentsRect = insideMargin(
      option->rect, Render::MenuItem_MarginWidth,
      (Render::MenuItem_MarginHeight + StyleConfigData::menuItemHeight()));
  drawItemText(painter, contentsRect, Qt::AlignCenter, palette, true,
               option->text, QPalette::WindowText);
}

qreal Style::dialAngle(const QStyleOptionSlider *sliderOption,
                       int value) const {
  // calculate angle at which handle needs to be drawn
  qreal angle(0);
  if (sliderOption->maximum == sliderOption->minimum)
    angle = M_PI / 2;
  else {
    qreal fraction(qreal(value - sliderOption->minimum) /
                   qreal(sliderOption->maximum - sliderOption->minimum));
    if (!sliderOption->upsideDown)
      fraction = 1 - fraction;

    if (sliderOption->dialWrapping)
      angle = 1.5 * M_PI - fraction * 2 * M_PI;
    else
      angle = (M_PI * 8 - fraction * 10 * M_PI) / 6;
  }

  return angle;
}

const QWidget *Style::scrollBarParent(const QWidget *widget) const {
  // check widget and parent
  if (!(widget && widget->parentWidget()))
    return nullptr;

  // try cast to scroll area. Must test both parent and grandparent
  QAbstractScrollArea *scrollArea;
  if (!(scrollArea =
            qobject_cast<QAbstractScrollArea *>(widget->parentWidget()))) {
    scrollArea = qobject_cast<QAbstractScrollArea *>(
        widget->parentWidget()->parentWidget());
  }

  // check scrollarea
  if (scrollArea && (widget == scrollArea->verticalScrollBar() ||
                     widget == scrollArea->horizontalScrollBar())) {
    return scrollArea;

  } else if (widget->parentWidget()->inherits("KTextEditor::View")) {
    return widget->parentWidget();

  } else
    return nullptr;
}

QColor Style::scrollBarArrowColor(const QStyleOptionSlider *option,
                                  const SubControl &control,
                                  const QWidget *widget) const {
  const auto &rect(option->rect);
  const auto &palette(option->palette);
  auto color(_helper->arrowColor(palette, QPalette::WindowText));

  bool widgetMouseOver((option->state & State_MouseOver));
  if (widget)
    widgetMouseOver = widget->underMouse();
  // in case this QStyle is used by QQuickControls QStyle wrapper
  else if (option->styleObject)
    widgetMouseOver = option->styleObject->property("hover").toBool();

  // check enabled state
  const bool enabled(option->state & State_Enabled);
  if (!enabled) {
    // finally, global opacity when ScrollBarShowOnMouseOver
    const qreal globalOpacity(_animations->scrollBarEngine().opacity(
        widget, QStyle::SC_ScrollBarGroove));
    if (globalOpacity >= 0)
      color.setAlphaF(globalOpacity);
    // no mouse over and no animation in progress, don't draw arrows at all
    else if (!widgetMouseOver)
      return Qt::transparent;
    return color;
  }

  if ((control == SC_ScrollBarSubLine &&
       option->sliderValue == option->minimum) ||
      (control == SC_ScrollBarAddLine &&
       option->sliderValue == option->maximum)) {
    // manually disable arrow, to indicate that scrollbar is at limit
    color =
        _helper->arrowColor(palette, QPalette::Disabled, QPalette::WindowText);
    // finally, global opacity when ScrollBarShowOnMouseOver
    const qreal globalOpacity(_animations->scrollBarEngine().opacity(
        widget, QStyle::SC_ScrollBarGroove));
    if (globalOpacity >= 0)
      color.setAlphaF(globalOpacity);
    // no mouse over and no animation in progress, don't draw arrows at all
    else if (!widgetMouseOver)
      return Qt::transparent;
    return color;
  }

  const bool mouseOver(
      _animations->scrollBarEngine().isHovered(widget, control));
  const bool animated(_animations->scrollBarEngine().isAnimated(
      widget, AnimationHover, control));
  const qreal opacity(_animations->scrollBarEngine().opacity(widget, control));

  // retrieve mouse position from engine
  QPoint position(mouseOver ? _animations->scrollBarEngine().position(widget)
                            : QPoint(-1, -1));
  if (mouseOver && rect.contains(position)) {
    /*
     * need to update the arrow controlRect on fly because there is no
     * way to get it from the styles directly, outside of repaint events
     */
    _animations->scrollBarEngine().setSubControlRect(widget, control, rect);
  }

  if (rect.intersects(
          _animations->scrollBarEngine().subControlRect(widget, control))) {
    auto highlight = _helper->hoverColor(palette);
    if (animated) {
      color = KColorUtils::mix(color, highlight, opacity);

    } else if (mouseOver) {
      color = highlight;
    }
  }

  // finally, global opacity when ScrollBarShowOnMouseOver
  const qreal globalOpacity(_animations->scrollBarEngine().opacity(
      widget, QStyle::SC_ScrollBarGroove));
  if (globalOpacity >= 0)
    color.setAlphaF(globalOpacity);
  // no mouse over and no animation in progress, don't draw arrows at all
  else if (!widgetMouseOver)
    return Qt::transparent;

  return color;
}

void Style::setTranslucentBackground(QWidget *widget) const {
  widget->setAttribute(Qt::WA_TranslucentBackground);

#ifdef Q_WS_WIN
  // FramelessWindowHint is needed on windows to make WA_TranslucentBackground
  // work properly
  widget->setWindowFlags(widget->windowFlags() | Qt::FramelessWindowHint);
#endif
}

QStyleOptionToolButton
Style::separatorMenuItemOption(const QStyleOptionMenuItem *menuItemOption,
                               const QWidget *widget) const {
  // separator can have a title and an icon
  // in that case they are rendered as sunken flat toolbuttons
  QStyleOptionToolButton toolButtonOption;
  toolButtonOption.initFrom(widget);
  toolButtonOption.rect = menuItemOption->rect;
  toolButtonOption.features = QStyleOptionToolButton::None;
  toolButtonOption.state = State_Enabled | State_AutoRaise;
  toolButtonOption.subControls = SC_ToolButton;
  toolButtonOption.icon = QIcon();
  toolButtonOption.iconSize = QSize();
  toolButtonOption.text = menuItemOption->text;

  toolButtonOption.toolButtonStyle = Qt::ToolButtonTextBesideIcon;
  return toolButtonOption;
}

QIcon Style::toolBarExtensionIcon(StandardPixmap standardPixmap,
                                  const QStyleOption *option,
                                  const QWidget *widget) const {
  // store palette
  // due to Qt, it is not always safe to assume that either option, nor widget
  // are defined
  QPalette palette;
  if (option)
    palette = option->palette;
  else if (widget)
    palette = widget->palette();
  else
    palette = QApplication::palette();

  // convenience class to map color to icon mode
  struct IconData {
    QColor _color;
    QIcon::Mode _mode;
    QIcon::State _state;
  };

  // map colors to icon states
  const QList<IconData> iconTypes = {
      {palette.color(QPalette::Active, QPalette::WindowText), QIcon::Normal,
       QIcon::Off},
      {palette.color(QPalette::Active, QPalette::WindowText), QIcon::Selected,
       QIcon::Off},
      {palette.color(QPalette::Active, QPalette::WindowText), QIcon::Active,
       QIcon::Off},
      {palette.color(QPalette::Disabled, QPalette::WindowText), QIcon::Disabled,
       QIcon::Off},

      {palette.color(QPalette::Active, QPalette::HighlightedText),
       QIcon::Normal, QIcon::On},
      {palette.color(QPalette::Active, QPalette::HighlightedText),
       QIcon::Selected, QIcon::On},
      {palette.color(QPalette::Active, QPalette::WindowText), QIcon::Active,
       QIcon::On},
      {palette.color(QPalette::Disabled, QPalette::WindowText), QIcon::Disabled,
       QIcon::On}};

  // default icon sizes
  static const QList<int> iconSizes = {8, 16, 22, 32, 48};

  // decide arrow orientation
  const ArrowOrientation orientation(
      standardPixmap == SP_ToolBarHorizontalExtensionButton ? ArrowRight
                                                            : ArrowDown);

  // create icon and fill
  QIcon icon;
  foreach (const IconData &iconData, iconTypes) {
    foreach (const int &iconSize, iconSizes) {
      // create pixmap
      QPixmap pixmap(iconSize, iconSize);
      pixmap.fill(Qt::transparent);

      // render
      QPainter painter(&pixmap);

      // icon size
      const int fixedIconSize(
          pixelMetric(QStyle::PM_SmallIconSize, option, widget));
      const QRect fixedRect(0, 0, fixedIconSize, fixedIconSize);

      painter.setWindow(fixedRect);
      painter.translate(standardPixmap == SP_ToolBarHorizontalExtensionButton
                            ? QPoint(1, 0)
                            : QPoint(0, 1));
      _helper->renderArrow(&painter, fixedRect, iconData._color, orientation);
      painter.end();

      // add to icon
      icon.addPixmap(pixmap, iconData._mode, iconData._state);
    }
  }

  return icon;
}

QIcon Style::titleBarButtonIcon(StandardPixmap standardPixmap,
                                const QStyleOption *option,
                                const QWidget *widget) const {
  // map standardPixmap to button type
  ButtonType buttonType;
  switch (standardPixmap) {
  case SP_TitleBarNormalButton:
    buttonType = ButtonRestore;
    break;
  case SP_TitleBarMinButton:
    buttonType = ButtonMinimize;
    break;
  case SP_TitleBarMaxButton:
    buttonType = ButtonMaximize;
    break;
  case SP_TitleBarCloseButton:
  case SP_DockWidgetCloseButton:
    buttonType = ButtonClose;
    break;

  default:
    return QIcon();
  }

  // store palette
  // due to Qt, it is not always safe to assume that either option, nor widget
  // are defined
  QPalette palette;
  if (option)
    palette = option->palette;
  else if (widget)
    palette = widget->palette();
  else
    palette = QApplication::palette();

  const bool isCloseButton(buttonType == ButtonClose &&
                           StyleConfigData::outlineCloseButton());

  palette.setCurrentColorGroup(QPalette::Active);
  const auto base(palette.color(QPalette::WindowText));
  const auto selected(palette.color(QPalette::HighlightedText));

  const bool invertNormalState(isCloseButton);

  // convenience class to map color to icon mode
  struct IconData {
    QColor _color;
    bool _inverted;
    QIcon::Mode _mode;
    QIcon::State _state;
  };

  // map colors to icon states

  QList<IconData> iconTypes;

  if (StyleConfigData::tabUseBrighterCloseIcon()) {
    iconTypes = {
        // brighten the tab close icons

        // state off icons
        {KColorUtils::mix(palette.color(QPalette::Window), base, 0.75),
         invertNormalState, QIcon::Normal, QIcon::Off},
        {KColorUtils::mix(palette.color(QPalette::Window), selected, 0.75),
         invertNormalState, QIcon::Selected, QIcon::Off},
        {KColorUtils::mix(palette.color(QPalette::Window), base, 1.0), false,
         QIcon::Active, QIcon::Off},
        {KColorUtils::mix(palette.color(QPalette::Window), base, 0.4),
         invertNormalState, QIcon::Disabled, QIcon::Off},

        // state on icons
        {KColorUtils::mix(palette.color(QPalette::Window), base, 1.0), false,
         QIcon::Normal, QIcon::On},
        {KColorUtils::mix(palette.color(QPalette::Window), selected, 1.0),
         false, QIcon::Selected, QIcon::On},
        {KColorUtils::mix(palette.color(QPalette::Window), base, 1.0), false,
         QIcon::Active, QIcon::On},
        {KColorUtils::mix(palette.color(QPalette::Window), base, 0.4),
         invertNormalState, QIcon::Disabled, QIcon::On}};
  } else {
    iconTypes = {
        // state off icons
        {KColorUtils::mix(palette.color(QPalette::Window), base, 0.5),
         invertNormalState, QIcon::Normal, QIcon::Off},
        {KColorUtils::mix(palette.color(QPalette::Window), selected, 0.5),
         invertNormalState, QIcon::Selected, QIcon::Off},
        {KColorUtils::mix(palette.color(QPalette::Window), base, 0.85), false,
         QIcon::Active, QIcon::Off},
        {KColorUtils::mix(palette.color(QPalette::Window), base, 0.2),
         invertNormalState, QIcon::Disabled, QIcon::Off},

        // state on icons
        {KColorUtils::mix(palette.color(QPalette::Window), base, 0.85), false,
         QIcon::Normal, QIcon::On},
        {KColorUtils::mix(palette.color(QPalette::Window), selected, 0.85),
         false, QIcon::Selected, QIcon::On},
        {KColorUtils::mix(palette.color(QPalette::Window), base, 0.85), false,
         QIcon::Active, QIcon::On},
        {KColorUtils::mix(palette.color(QPalette::Window), base, 0.2),
         invertNormalState, QIcon::Disabled, QIcon::On}};
  }

  // default icon sizes
  static const QList<int> iconSizes = {8, 16, 22, 32, 48};

  // output icon
  QIcon icon;

  foreach (const IconData &iconData, iconTypes) {
    foreach (const int &iconSize, iconSizes) {
      // create pixmap
      QPixmap pixmap(iconSize, iconSize);
      pixmap.fill(Qt::transparent);

      // create painter and render
      QPainter painter(&pixmap);
      _helper->renderDecorationButton(&painter, pixmap.rect(), iconData._color,
                                      buttonType, iconData._inverted);

      painter.end();

      // store
      icon.addPixmap(pixmap, iconData._mode, iconData._state);
    }
  }

  return icon;
}

const QAbstractItemView *Style::itemViewParent(const QWidget *widget) const {
  const QAbstractItemView *itemView(nullptr);

  // check widget directly
  if ((itemView = qobject_cast<const QAbstractItemView *>(widget)))
    return itemView;

  // check widget grand-parent
  else if (widget && widget->parentWidget() &&
           (itemView = qobject_cast<const QAbstractItemView *>(
                widget->parentWidget()->parentWidget())) &&
           itemView->viewport() == widget->parentWidget()) {
    return itemView;
  }

  // return null otherwise
  else
    return nullptr;
}

bool Style::isSelectedItem(const QWidget *widget,
                           const QPoint &localPosition) const {
  // get relevant itemview parent and check
  const QAbstractItemView *itemView(itemViewParent(widget));
  if (!(itemView && itemView->hasFocus() && itemView->selectionModel()))
    return false;

  QPoint position = widget->mapTo(itemView, localPosition);

  // get matching QModelIndex and check
  const QModelIndex index(itemView->indexAt(position));
  if (!index.isValid())
    return false;

  // check whether index is selected
  return itemView->selectionModel()->isSelected(index);
}

bool Style::isQtQuickControl(const QStyleOption *option,
                             const QWidget *widget) const {
#if BLOSSOMUI_HAVE_QTQUICK
  const bool is = (widget == nullptr) && option && option->styleObject &&
                  option->styleObject->inherits("QQuickItem");
  if (is) {
    auto *item = static_cast<QQuickItem *>(option->styleObject);
    _windowManager->registerQuickItem(item);
    _animations->registerWidget(item);
  }
  return is;
#else
  Q_UNUSED(widget);
  Q_UNUSED(option);
  return false;
#endif
}

bool Style::showIconsInMenuItems() const {
  const KConfigGroup g(KSharedConfig::openConfig(), "KDE");
  return g.readEntry("ShowIconsInMenuItems", true);
}

bool Style::showIconsOnPushButtons() const {
  const KConfigGroup g(KSharedConfig::openConfig(), "KDE");
  return g.readEntry("ShowIconsOnPushButtons", true);
}

bool Style::isMenuTitle(const QWidget *widget) const {
  // check widget
  if (!widget)
    return false;

  // check property
  const QVariant property(widget->property(PropertyNames::menuTitle));
  if (property.isValid())
    return property.toBool();

  // detect menu toolbuttons
  QWidget *parent = widget->parentWidget();
  if (qobject_cast<QMenu *>(parent)) {
    foreach (auto child, parent->findChildren<QWidgetAction *>()) {
      if (child->defaultWidget() != widget)
        continue;
      const_cast<QWidget *>(widget)->setProperty(PropertyNames::menuTitle,
                                                 true);
      return true;
    }
  }

  const_cast<QWidget *>(widget)->setProperty(PropertyNames::menuTitle, false);
  return false;
}

bool Style::hasAlteredBackground(const QWidget *widget) const {
  // check widget
  if (!widget)
    return false;

  // check property
  const QVariant property(widget->property(PropertyNames::alteredBackground));
  if (property.isValid())
    return property.toBool();

  // check if widget is of relevant type
  bool hasAlteredBackground(false);
  if (const auto groupBox = qobject_cast<const QGroupBox *>(widget))
    hasAlteredBackground = !groupBox->isFlat();
  else if (const auto tabWidget = qobject_cast<const QTabWidget *>(widget))
    hasAlteredBackground = !tabWidget->documentMode();
  else if (qobject_cast<const QMenu *>(widget))
    hasAlteredBackground = true;
  else if (StyleConfigData::dockWidgetDrawFrame() &&
           qobject_cast<const QDockWidget *>(widget))
    hasAlteredBackground = true;

  if (widget->parentWidget() && !hasAlteredBackground)
    hasAlteredBackground = this->hasAlteredBackground(widget->parentWidget());
  const_cast<QWidget *>(widget)->setProperty(PropertyNames::alteredBackground,
                                             hasAlteredBackground);
  return hasAlteredBackground;
}

// Taken from Kvantum
/*
    To make Qt windows translucent, we should set the surface format of
    their native handles BEFORE they're created but Qt5 windows are
    often polished AFTER they're created, so that setting the attribute
    "WA_TranslucentBackground" in "Style::polish()" would have no effect.

    Early creation of native handles could have unpredictable side effects,
    especially for menus. However, it seems that setting of the attribute
    "WA_TranslucentBackground" in an early stage -- before the widget is
    created -- sets the alpha buffer size to 8 safely and automatically.
*/
void Style::setSurfaceFormat(QWidget *widget) const {
  if (!widget || !_helper->compositingActive() || _app.subApp ||
      _app.isLibreoffice || _app.isKonsole)
    return;

  if (widget->testAttribute(Qt::WA_WState_Created) ||
      widget->testAttribute(Qt::WA_TranslucentBackground) ||
      widget->testAttribute(Qt::WA_NoSystemBackground) ||
      widget->autoFillBackground() // video players like kaffeine
      || _translucentWidgets.contains(widget))
    return;

  if (widget->inherits("QTipLabel"))
    return;

  else if (qobject_cast<QMenu *>(widget)) {
    QWindow *window = widget->windowHandle();
    if (window) {
      QSurfaceFormat format = window->format();
      format.setAlphaBufferSize(8);
      window->setFormat(format);
    }
  }

  else {
    // this stops flickering on transparent toolbar, menubar, tabbar
    if (_app.isBarsOpaque && !_app.isOpaque) {
      widget->setAttribute(Qt::WA_TranslucentBackground);
      widget->setAttribute(Qt::WA_NoSystemBackground, false);
    }

    if (_app.isPlasma || _app.isOpaque || !widget->isWindow() ||
        !_helper->shouldWindowHaveAlpha(widget->palette(), _app.isDolphin)) {
      return;
    }

    switch (widget->windowFlags() & Qt::WindowType_Mask) {
    case Qt::Window:
    case Qt::Dialog:
    case Qt::Popup:
    case Qt::Sheet:
      break;
    default:
      return;
    }
    if (widget->windowHandle() // too late
        || widget->windowFlags().testFlag(Qt::FramelessWindowHint) ||
        widget->windowFlags().testFlag(Qt::X11BypassWindowManagerHint) ||
        qobject_cast<QFrame *>(widget) // a floating frame, as in Filelight
        || widget->testAttribute(Qt::WA_PaintOnScreen) ||
        widget->testAttribute(Qt::WA_X11NetWmWindowTypeDesktop) ||
        widget->inherits("KScreenSaver") || widget->inherits("QSplashScreen"))
      return;

    QWidget *p = widget->parentWidget();
    if (p && (/*!p->testAttribute(Qt::WA_WState_Created) // FIXME: too soon?
            ||*/
              qobject_cast<QMdiSubWindow *>(p))) // as in linguist
    {
      return;
    }

    if (QMainWindow *mw = qobject_cast<QMainWindow *>(widget)) {
      /* it's possible that a main window is inside another one
          (like FormPreviewView in linguist), in which case,
          translucency could cause weird effects */
      if (p)
        return;
      /* stylesheets with background can cause total transparency */
      QString ss = mw->styleSheet();
      if (!ss.isEmpty() && ss.contains("background"))
        return;
      if (QWidget *cw = mw->centralWidget()) {
        if (cw->autoFillBackground())
          return;
        ss = cw->styleSheet();
        if (!ss.isEmpty() && ss.contains("background"))
          return; // as in smplayer
      }
    }
  }

  if (!_helper->compositingActive())
    return;

  widget->setAttribute(Qt::WA_TranslucentBackground);

  /* distinguish forced translucency from hard-coded translucency */
  // forcedTranslucency_.insert(widget);
  // connect(widget, &QObject::destroyed, this, &Style::noTranslucency);   // needed?
}

bool Style::isStylableToolbar(const QWidget *w,
                              bool allowInvisible) const // should be in helper
{
  if (w->isWindow())
    return false;

  const QToolBar *tb = qobject_cast<const QToolBar *>(w);
  if (!tb || w->autoFillBackground() ||
      w->testAttribute(
          Qt::WA_StyleSheetTarget) // not drawn by Kvantum (CE_ToolBar may not
                                   // be called)
      || _app.isPlasma) {
    return false;
  }

  if (QTabBar *tabBar = w->findChild<QTabBar *>()) {
    if (tb->isAncestorOf(tabBar))
      return false; // practically not a toolbar (Kaffeine's sidebar)
  }

  QWidget *p = w->parentWidget();
  if (p != w->window())
    return false; // inside a dock

  /* don't style toolbars in places like KAboutDialog (-> KAboutData ->
   * KAboutPerson) */
  if (QMainWindow *mw = qobject_cast<QMainWindow *>(p)) {
    if (tb->orientation() == Qt::Vertical) {
      if (tb->y() == 0) {
        if (BlossomUIPrivate::possibleTranslucentToolBars.size() == 0) {
          BlossomUIPrivate::possibleTranslucentToolBars.insert(w);
          return true;
        } else if (BlossomUIPrivate::possibleTranslucentToolBars.contains(w) &&
                   BlossomUIPrivate::possibleTranslucentToolBars.size() == 1)
          return true;

        else {
          BlossomUIPrivate::possibleTranslucentToolBars.insert(w);
          return false;
        }
      }
    }

    if (QWidget *mb = mw->menuWidget()) // WARNING: an empty menubar may be
                                        // created by menuBar()
    {
      if (mb->isVisible()) {
        if (mb->y() + mb->height() == tb->y()) {
          BlossomUIPrivate::possibleTranslucentToolBars.insert(w);
          return true;
        }
      } else if (tb->y() == 0 &&
                 (allowInvisible || tb->isVisible())) // FIXME: Why can KtoolBar
                                                      // be invisible here?
      {
        BlossomUIPrivate::possibleTranslucentToolBars.insert(w);
        return true;
      } else
        return false;
    } else if (tb->y() == 0)
      return true;

    // possible lone toolbar at the bottom
    else {
      if (BlossomUIPrivate::possibleTranslucentToolBars.size() == 0) {
        BlossomUIPrivate::possibleTranslucentToolBars.insert(w);
        return true;
      } else if (BlossomUIPrivate::possibleTranslucentToolBars.contains(w) &&
                 BlossomUIPrivate::possibleTranslucentToolBars.size() == 1)
        return true;

      else {
        BlossomUIPrivate::possibleTranslucentToolBars.insert(w);
        return false;
      }
    }
  }
  return false;
}

void Style::tabLayout(const QStyleOptionTab *opt, const QWidget *widget,
                      QRect *textRect, QRect *iconRect) const {
  Q_ASSERT(textRect);
  Q_ASSERT(iconRect);
  QRect tr = opt->rect;
  bool verticalTabs = opt->shape == QTabBar::RoundedEast ||
                      opt->shape == QTabBar::RoundedWest ||
                      opt->shape == QTabBar::TriangularEast ||
                      opt->shape == QTabBar::TriangularWest;
  if (verticalTabs)
    tr.setRect(0, 0, tr.height(),
               tr.width()); // 0, 0 as we will have a translate transform

  int verticalShift =
      pixelMetric(QStyle::PM_TabBarTabShiftVertical, opt, widget);
  int horizontalShift =
      pixelMetric(QStyle::PM_TabBarTabShiftHorizontal, opt, widget);
  int hpadding = (pixelMetric(QStyle::PM_TabBarTabHSpace, opt, widget) / 2) +
                 StyleConfigData::cornerRadius();
  int vpadding = pixelMetric(QStyle::PM_TabBarTabVSpace, opt, widget) / 2;
  if (opt->shape == QTabBar::RoundedSouth ||
      opt->shape == QTabBar::TriangularSouth)
    verticalShift = -verticalShift;
  tr.adjust(hpadding, verticalShift - vpadding, horizontalShift - hpadding,
            vpadding);
  bool selected = opt->state & QStyle::State_Selected;
  if (selected) {
    tr.setTop(tr.top() - verticalShift);
    tr.setRight(tr.right() - horizontalShift);
  }

  // left widget
  if (!opt->leftButtonSize.isEmpty()) {
    tr.setLeft(tr.left() + 4 +
               (verticalTabs ? opt->leftButtonSize.height()
                             : opt->leftButtonSize.width()));
  }
  // right widget
  if (!opt->rightButtonSize.isEmpty()) {
    tr.setRight(tr.right() - 4 -
                (verticalTabs ? opt->rightButtonSize.height()
                              : opt->rightButtonSize.width()));
  }

  // icon
  if (!opt->icon.isNull()) {
    QSize iconSize = opt->iconSize;
    if (!iconSize.isValid()) {
      int iconExtent = pixelMetric(QStyle::PM_SmallIconSize, opt);
      iconSize = QSize(iconExtent, iconExtent);
    }
    QSize tabIconSize = opt->icon.actualSize(
        iconSize,
        (opt->state & QStyle::State_Enabled) ? QIcon::Normal : QIcon::Disabled,
        (opt->state & QStyle::State_Selected) ? QIcon::On : QIcon::Off);
    // High-dpi icons do not need adjustment; make sure tabIconSize is not
    // larger than iconSize
    tabIconSize = QSize(qMin(tabIconSize.width(), iconSize.width()),
                        qMin(tabIconSize.height(), iconSize.height()));

    const int offsetX = (iconSize.width() - tabIconSize.width()) / 2;
    *iconRect =
        QRect(tr.left() + offsetX, tr.center().y() - tabIconSize.height() / 2,
              tabIconSize.width(), tabIconSize.height());
    if (!verticalTabs)
      *iconRect = QStyle::visualRect(opt->direction, opt->rect, *iconRect);
    tr.setLeft(tr.left() + tabIconSize.width() + 4);
  }

  if (!verticalTabs)
    tr = QStyle::visualRect(opt->direction, opt->rect, tr);

  *textRect = tr;
}

// also checks for NULL widgets
QWidget *Style::getParent(const QWidget *widget, int level) const {
  if (!widget || level <= 0)
    return nullptr;
  QWidget *w = widget->parentWidget();
  for (int i = 1; i < level && w; ++i)
    w = w->parentWidget();
  return w;
}
} // namespace BlossomUI
