// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuipropertynames.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"

#include <KColorUtils>
#include <QAbstractScrollArea>
#include <QApplication>
#include <QGroupBox>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QStyleOptionFrame>
#include <QStyleOptionTabBarBase>
#include <QStyleOptionTabWidgetFrame>
#include <QTabWidget>
#include <QToolBar>

namespace BlossomUI {

bool Style::drawFramePrimitive(const QStyleOption *option, QPainter *painter,
                               const QWidget *widget) const {
  if (_app.isDolphin && widget && widget->inherits("KItemListContainer"))
    return true;

  // copy palette and rect
  const auto &palette(option->palette);
  const auto &rect(option->rect);

  // from kvantum
  if (_app.isDolphin) {
    if (QWidget *pw = widget->parentWidget()) {
      if (StyleConfigData::transparentDolphinView()
          // not renaming area
          && !qobject_cast<QAbstractScrollArea *>(pw)
          // only Dolphin's view
          && QString(pw->metaObject()->className()).startsWith("Dolphin")) {
        if (widget->property("VISIBLE-SEPARATORS").toBool()) {
          QRect copy = rect.adjusted(12, 0, -12, 0);
          painter->setRenderHint(QPainter::Antialiasing);
          painter->setBrush(Qt::NoBrush);
          painter->setPen(Qt::NoPen);
          painter->drawLine(copy.topLeft(), copy.topRight());
          painter->drawLine(copy.bottomLeft(), copy.bottomRight());
        }
        return true;
      }
    }
  }

  // detect title widgets
  const bool isTitleWidget(StyleConfigData::titleWidgetDrawFrame() && widget &&
                           widget->parent() &&
                           widget->parent()->inherits("KTitleWidget"));

  // store window state
  const bool windowActive(widget && widget->isActiveWindow());

  // copy state
  const State &state(option->state);
  if (!isTitleWidget && !(state & (State_Sunken | State_Raised)))
    return true;

  const bool isInputWidget(
      (widget && widget->testAttribute(Qt::WA_Hover)) ||
      (isQtQuickControl(option, widget) &&
       option->styleObject->property("elementType").toString() ==
           QStringLiteral("edit")));

  const bool enabled(state & State_Enabled);
  const bool mouseOver(enabled && isInputWidget && (state & State_MouseOver));
  const bool hasFocus(enabled && isInputWidget && (state & State_HasFocus));

  // focus takes precedence over mouse over
  _animations->inputWidgetEngine().updateState(widget, AnimationFocus,
                                               hasFocus);
  _animations->inputWidgetEngine().updateState(widget, AnimationHover,
                                               mouseOver && !hasFocus);

  // retrieve animation mode and opacity
  _animations->inputWidgetEngine().frameAnimationMode(widget);
  _animations->inputWidgetEngine().frameOpacity(widget);

  // render
  if (!StyleConfigData::sidePanelDrawFrame() && widget &&
      widget->property(PropertyNames::sidePanelView).toBool()) {
    const auto outline(_helper->sidePanelOutlineColor(palette));
    const bool reverseLayout(option->direction == Qt::RightToLeft);
    const Side side(reverseLayout ? SideRight : SideLeft);
    if ((widget->window()->windowFlags() & Qt::WindowType_Mask) == Qt::Dialog) {
      QColor background(palette.color(QPalette::Base));

      if (StyleConfigData::dolphinSidebarOpacity() < 100 && _app.isDolphin) {
        _helper->renderTransparentArea(painter, rect);

        background.setAlphaF(StyleConfigData::dolphinSidebarOpacity() / 100.0);
      }

      painter->fillRect(rect, background);

      if (_helper->titleBarColor(true).alpha() !=
          palette.color(QPalette::Window).alpha()) {
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setPen(QColor(0, 0, 0, 30));
        painter->drawLine(rect.topLeft(), rect.topRight());
        painter->setRenderHint(QPainter::Antialiasing);
      }
    }
    _helper->renderSidePanelFrame(painter, rect, outline, side);

  } else {
    const auto background(isTitleWidget
                              ? palette.color(widget->backgroundRole())
                              : palette.color(QPalette::Base));
    _helper->renderFrame(painter, rect, background, windowActive, enabled);
  }

  return true;
}

bool Style::drawFrameLineEditPrimitive(const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // copy palette and rect
  const auto &palette(option->palette);
  const auto &rect(option->rect);

  // store window state
  const bool windowActive(widget && widget->isActiveWindow());

  auto background = palette.color(QPalette::Base);
  QColor outline;

  const auto isControl = isQtQuickControl(option, widget);

  if (_app.isDolphin && !isControl && widget)
    applyDolphinUrlNavigatorStyle(widget, background);

  // make sure there is enough room to render frame
  if (rect.height() <
      2 * Metrics::LineEdit_FrameWidth + option->fontMetrics.height()) {
    const auto &background = palette.color(QPalette::Base);

    painter->setPen(Qt::NoPen);
    painter->setBrush(background);
    painter->drawRect(rect);
    return true;

  } else {
    // copy state
    const State &state(option->state);
    const bool enabled(state & State_Enabled);
    const bool mouseOver(enabled && (state & State_MouseOver));
    const bool hasFocus(enabled && (state & State_HasFocus));

    // focus takes precedence over mouse over (default duration, not
    // AnimationLongDuration - was too slow)
    _animations->inputWidgetEngine().updateState(widget, AnimationFocus,
                                                 hasFocus);
    //_animations->inputWidgetEngine().updateState( widget, AnimationHover,
    // mouseOver && !hasFocus );

    // retrieve animation mode and opacity
    AnimationMode mode(
        _animations->inputWidgetEngine().frameAnimationMode(widget));
    qreal opacity(_animations->inputWidgetEngine().frameOpacity(widget));
    if (hasFocus) {
      outline = palette.color(QPalette::Highlight);
    } else {
      outline = palette.color(QPalette::WindowText);
    }
    _helper->renderLineEdit(painter, rect, background, outline, hasFocus,
                            mouseOver, enabled, windowActive, mode, opacity);
  }

  return true;
}

bool Style::drawFrameFocusRectPrimitive(const QStyleOption *option,
                                        QPainter *painter,
                                        const QWidget *widget) const {
  /*  Removes focus indicator from the tabs  */
  if (widget && widget->inherits("QTabBar")) {
    return true;
  }
  // no focus indicator on buttons / scrollbars, since it is rendered elsewhere
  if (qobject_cast<const QAbstractButton *>(widget) ||
      qobject_cast<const QScrollBar *>(widget) ||
      qobject_cast<const QGroupBox *>(widget)) {
    return true;
  }

  // no focus indicator on ComboBox list items
  if (widget && widget->inherits("QComboBoxListView")) {
    return true;
  }

  if (option->styleObject &&
      option->styleObject->property("elementType") == QLatin1String("button")) {
    return true;
  }

  const State &state(option->state);

  // no focus indicator on selected list items
  if ((state & State_Selected) &&
      qobject_cast<const QAbstractItemView *>(widget)) {
    return true;
  }

  const auto rect(option->rect.adjusted(0, 0, 0, 1));
  const auto &palette(option->palette);

  if (rect.width() < 10)
    return true;

  const auto outlineColor(state & State_Selected
                              ? palette.color(QPalette::HighlightedText)
                              : palette.color(QPalette::Highlight));
  painter->setRenderHint(QPainter::Antialiasing, false);
  painter->setPen(outlineColor);
  painter->drawLine(QPoint(rect.bottomLeft() - QPoint(0, 1)),
                    QPoint(rect.bottomRight() - QPoint(0, 1)));

  return true;
}

bool Style::drawFrameMenuPrimitive(const QStyleOption *option,
                                   QPainter *painter,
                                   const QWidget *widget) const {
  // Draw for (expanded) toolbars, QtQuick controls, and Slint combobox popup
  // (plain QWidget)
  const bool isToolBar(qobject_cast<const QToolBar *>(widget));
  const bool isQtQuick(isQtQuickControl(option, widget));
  const bool isSlintPopup(widget && !widget->isWindow() &&
                          !qobject_cast<const QMenu *>(widget));

  if (isToolBar || isQtQuick || isSlintPopup) {
    const auto &palette(option->palette);
    const auto background(_helper->frameBackgroundColor(palette));
    const auto outline(_helper->isDarkTheme(palette) ? QColor(255, 255, 255, 30)
                                                     : QColor(0, 0, 0, 40));
    bool hasAlpha(_helper->hasAlphaChannel(widget));
    if (isSlintPopup)
      hasAlpha = true;

    painter->save();
    // Skip renderBlurredBackground for QAbstractScrollArea (KItemListContainer
    // etc.) - grab() during paint causes recursive repaint and SEGV
    if (isSlintPopup && widget->window() &&
        !qobject_cast<const QAbstractScrollArea *>(widget)) {
      const QRect rectInWindow(
          widget->mapTo(widget->window(), option->rect.topLeft()),
          widget->mapTo(widget->window(), option->rect.bottomRight()));
      if (_helper->renderBlurredBackground(painter, widget->window(),
                                           rectInWindow.normalized(),
                                           option->rect, 12)) {
        QColor bg(background);
        bg.setAlphaF(StyleConfigData::menuOpacity() / 100.0);
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->setBrush(bg);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(option->rect,
                                 StyleConfigData::menuItemRadius(),
                                 StyleConfigData::menuItemRadius());
        if (outline.isValid()) {
          painter->setPen(outline);
          painter->setBrush(Qt::NoBrush);
          painter->drawRoundedRect(
              QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5),
              StyleConfigData::menuItemRadius() + 0.5,
              StyleConfigData::menuItemRadius() + 0.5);
        }
      } else {
        if (hasAlpha) {
          painter->setCompositionMode(QPainter::CompositionMode_Source);
          QColor bg(background);
          bg.setAlphaF(StyleConfigData::menuOpacity() / 100.0);
          _helper->renderMenuFrame(painter, option->rect, bg, outline,
                                   hasAlpha);
        } else {
          _helper->renderMenuFrame(painter, option->rect, background, outline,
                                   hasAlpha);
        }
      }
    } else if (hasAlpha) {
      painter->setCompositionMode(QPainter::CompositionMode_Source);
      QColor bg(background);
      bg.setAlphaF(StyleConfigData::menuOpacity() / 100.0);
      _helper->renderMenuFrame(painter, option->rect, bg, outline, hasAlpha);
    } else {
      _helper->renderMenuFrame(painter, option->rect, background, outline,
                               hasAlpha);
    }
    painter->restore();
  }

  return true;
}

bool Style::drawFrameGroupBoxPrimitive(const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *) const {
  // cast option and check
  const auto frameOption(qstyleoption_cast<const QStyleOptionFrame *>(option));
  if (!frameOption)
    return true;

  // no frame for flat groupboxes
  if (frameOption->features & QStyleOptionFrame::Flat)
    return true;

  // normal frame
  const auto &palette(option->palette);
  const auto background(_helper->frameBackgroundColor(palette));

  // need to reset painter's clip region to paint behind textbox label (was
  // taken out in QCommonStyle)
  painter->setClipRegion(option->rect);
  _helper->renderGroupBox(painter, option->rect, background, false);

  return true;
}

bool Style::drawFrameTabWidgetPrimitive(const QStyleOption *option,
                                        QPainter *painter,
                                        const QWidget *widget) const {
  // cast option and check
  const auto tabOption(
      qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option));
  if (!tabOption)
    return true;

  // store window state
  const bool windowActive(widget && widget->isActiveWindow());

  // do nothing if tabbar is hidden
  const bool isQtQuickControl(this->isQtQuickControl(option, widget));
  if (tabOption->tabBarSize.isEmpty() && !isQtQuickControl)
    return true;

  // define colors
  const auto &palette(option->palette);
  const auto background(_helper->frameBackgroundColor(palette));
  _helper->renderTabWidgetFrame(painter, option->rect, background, AllCorners,
                                windowActive);

  return true;
}

bool Style::drawFrameTabBarBasePrimitive(const QStyleOption *option,
                                         QPainter *painter,
                                         const QWidget *widget) const {
  // tabbar frame used either for 'separate' tabbar, or in 'document mode'

  // this is the empty part of the tab area

  // cast option and check
  const auto tabOption(
      qstyleoption_cast<const QStyleOptionTabBarBase *>(option));
  // get rect, orientation, palette
  const auto rect(option->rect);

  if (!tabOption)
    return true;

  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, false);

  // precaution don't change the alpha channel if the tabbar opacity is at 100
  if ((_app.isDolphin || _app.isKonsole) &&
      (StyleConfigData::tabBarOpacity() < 100) && !_app.isOpaque) {
    QColor backgroundColor = _helper->transparentBarBgColor(
        widget->palette().color(QPalette::Window), painter, widget->rect(),
        BarType::TabBar);
    painter->setBrush(backgroundColor);
    painter->fillRect(rect, backgroundColor);
  } else {
    const auto outline(QColor(0, 0, 0, 1));

    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(outline, 1));

    // render
    switch (tabOption->shape) {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
      painter->drawLine(rect.bottomLeft() - QPoint(1, 0),
                        rect.bottomRight() + QPoint(1, 0));
      break;

    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
      painter->drawLine(rect.topLeft() - QPoint(1, 0),
                        rect.topRight() + QPoint(1, 0));
      break;

    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
      painter->drawLine(rect.topRight() - QPoint(0, 1),
                        rect.bottomRight() + QPoint(1, 0));
      break;

    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
      painter->drawLine(rect.topLeft() - QPoint(0, 1),
                        rect.bottomLeft() + QPoint(1, 0));
      break;

    default:
      break;
    }
  }

  return true;
}

bool Style::drawFrameWindowPrimitive(const QStyleOption *option,
                                     QPainter *painter, const QWidget *) const {
  // copy rect and palette
  const auto &rect(option->rect);
  const auto &palette(option->palette);
  const State state(option->state);
  const bool selected(state & State_Selected);

  // render frame outline
  const auto outline(_helper->frameOutlineColor(palette, false, selected));
  _helper->renderMenuFrame(painter, rect, QColor(), outline);

  return true;
}
} // namespace BlossomUI
