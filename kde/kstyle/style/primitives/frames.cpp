// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuipropertynames.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "frame.h"

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

  // detect title widgets
  const bool isTitleWidget(StyleConfigData::titleWidgetDrawFrame() && widget &&
                           widget->parent() &&
                           widget->parent()->inherits("KTitleWidget"));

  // copy state
  const State &state(option->state);
  if (!isTitleWidget && !(state & (State_Sunken | State_Raised)))
    return true;

  // card-style rounded frame is Dolphin-only side panels keep their line frame
  if (!_app.isDolphin &&
      !(widget && widget->property(PropertyNames::sidePanelView).toBool()))
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
  if (widget && widget->property(PropertyNames::sidePanelView).toBool() &&
      (!StyleConfigData::sidePanelDrawFrame() || !_app.isDolphin)) {
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
      2 * Render::LineEdit_FrameWidth + option->fontMetrics.height()) {
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

    const QObject *styleObject(widget ? widget : option->styleObject);

    // focus takes precedence over mouse over
    _animations->inputWidgetEngine().updateState(styleObject, AnimationFocus,
                                                 hasFocus);

    // retrieve animation mode and opacity
    AnimationMode mode(
        _animations->inputWidgetEngine().frameAnimationMode(styleObject));
    qreal opacity(_animations->inputWidgetEngine().frameOpacity(styleObject));
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
    const auto outline(_helper->isDarkTheme(palette) ? QColor(255, 255, 255, 20)
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
QColor Helper::frameOutlineColor(const QPalette &palette, bool mouseOver,
                                 bool hasFocus, qreal opacity,
                                 AnimationMode mode) const {
  QColor outline = windowAlternateBackground(palette);

  // focus takes precedence over hover
  if (mode == AnimationFocus) {
    const QColor focus(focusColor(palette));
    const QColor hover(hoverColor(palette));

    if (mouseOver)
      outline = KColorUtils::mix(hover, focus, opacity);
    else
      outline = KColorUtils::mix(outline, focus, opacity);

  } else if (hasFocus) {
    outline = focusColor(palette);

  } else if (mode == AnimationHover) {
    const QColor hover(hoverColor(palette));
    outline = KColorUtils::mix(outline, hover, opacity);

  } else if (mouseOver) {
    outline = hoverColor(palette);
  }

  return outline;
}

QColor Helper::sidePanelOutlineColor(const QPalette &palette) const {
  QColor outline(qGray(palette.color(QPalette::Window).rgb()) > 150
                     ? QColor(0, 0, 0, 20)
                     : QColor(0, 0, 0, 50));
  return outline;
}

QColor Helper::frameBackgroundColor(const QPalette &palette,
                                    QPalette::ColorGroup group) const {
  return KColorUtils::mix(palette.color(group, QPalette::Window),
                          palette.color(group, QPalette::Base), 0.3);
}

QColor Helper::separatorColor(const QPalette &palette) const {
  return isDarkTheme(palette) ? QColor(255, 255, 255, 10) : QColor(0, 0, 0, 16);
}

void Helper::renderFocusLine(QPainter *painter, const QRect &rect,
                             const QColor &color) const {
  if (!color.isValid())
    return;

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, false);
  painter->setBrush(Qt::NoBrush);
  painter->setPen(color);

  painter->translate(0, 2);
  painter->drawLine(rect.bottomLeft(), rect.bottomRight());
  painter->restore();
}

void Helper::renderFrame(QPainter *painter, const QRect &rect,
                         const QColor &color, const bool windowActive,
                         const bool enabled) const {
  painter->setRenderHint(QPainter::Antialiasing);

  const qreal radius = frameRadius(PenWidth::NoPen);

  painter->setPen(Qt::NoPen);
  painter->setBrush(color);
  painter->drawRoundedRect(QRectF(rect), radius, radius);

  const QPalette appPalette(QApplication::palette());
  const bool dark(isDarkTheme(appPalette));
  const qreal borderAlpha = enabled ? (dark ? 0.08 : 0.12)
                                    : (dark ? 0.05 : 0.08);
  const QColor borderColor(
      alphaColor(appPalette.color(QPalette::WindowText), borderAlpha));
  QPen borderPen(borderColor, 1);
  borderPen.setCosmetic(true);
  painter->setPen(borderPen);
  painter->setBrush(Qt::NoBrush);
  painter->drawRoundedRect(QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5),
                           radius - 0.5, radius - 0.5);

  Q_UNUSED(windowActive)
}

void Helper::renderSidePanelFrame(QPainter *painter, const QRect &rect,
                                  const QColor &outline, Side side) const {
  // check color
  if (!outline.isValid())
    return;

  // adjust rect
  QRectF frameRect(strokedRect(rect));

  // setup painter
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setPen(outline);

  // render
  switch (side) {
  default:
  case SideLeft:
    painter->drawLine(frameRect.topRight(), frameRect.bottomRight());
    break;

  case SideTop:
    painter->drawLine(frameRect.topLeft(), frameRect.topRight());
    break;

  case SideRight:
    painter->drawLine(frameRect.topLeft(), frameRect.bottomLeft());
    break;

  case SideBottom:
    painter->drawLine(frameRect.bottomLeft(), frameRect.bottomRight());
    break;

  case AllSides: {
    const qreal radius(frameRadius(PenWidth::Frame, -1));
    painter->drawRoundedRect(frameRect, radius, radius);
    break;
  }
  }
}

void Helper::renderSeparator(QPainter *painter, const QRect &rect,
                             const QColor &color, bool vertical) const {
  painter->setRenderHint(QPainter::Antialiasing, false);
  painter->setBrush(Qt::NoBrush);
  painter->setPen(color);

  if (vertical) {
    painter->translate(rect.width() * 0.5, 0);
    painter->drawLine(rect.topLeft(), rect.bottomLeft());

  } else {
    painter->translate(0, rect.height() * 0.5);
    painter->drawLine(rect.topLeft(), rect.topRight());
  }
}

void Helper::renderLineEdit(QPainter *painter, const QRect &rect,
                            const QColor &background, const QColor &outline,
                            const bool hasFocus, const bool mouseOver,
                            bool enabled, const bool windowActive,
                            const AnimationMode mode,
                            const qreal opacity) const {
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setRenderHint(QPainter::SmoothPixmapTransform);

  QRectF frameRect(
      rect.adjusted(Render::Frame_FrameWidth, Render::Frame_FrameWidth,
                    -Render::Frame_FrameWidth, -Render::Frame_FrameWidth));
  qreal radius(inputFrameRadius(PenWidth::NoPen, -1));

  QColor border(KColorUtils::mix(background, outline, 0.15));
  if (mouseOver && !hasFocus)
    border = KColorUtils::mix(background, outline, 0.3);
  if (hasFocus && mode != AnimationFocus)
    border = KColorUtils::mix(background, outline, 0.65);
  if (mode == AnimationFocus && opacity >= 0) {
    // Smooth fade in both directions: opacity 0 = unfocused, 1 = focused
    border = KColorUtils::mix(background, outline, 0.15 + 0.5 * opacity);
  }

  if (!enabled)
    border = KColorUtils::mix(background, outline, 0.08);

  // focus ring - stroked path for smooth corners (avoids jagged fill in
  // Slint/Qt Quick)
  if (enabled && (hasFocus || (mode == AnimationFocus && opacity > 0))) {
    const qreal ringOpacity = (mode == AnimationFocus) ? opacity : 1.0;
    painter->setBrush(Qt::NoBrush);
    QPen ringPen(alphaColor(outline, 0.5 * ringOpacity), 2, Qt::SolidLine,
                 Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(ringPen);
    const QRectF ringPath(frameRect.adjusted(-1, -1, 1, 1));
    painter->drawRoundedRect(ringPath, radius + 1, radius + 1);
  }

  // base fill + border
  painter->setBrush(background.isValid() ? background : Qt::NoBrush);
  painter->setPen(border.isValid() ? QPen(border, 1) : Qt::NoPen);
  painter->drawRoundedRect(frameRect, radius, radius);

  Q_UNUSED(windowActive)
}

void Helper::renderTransparentArea(QPainter *painter, const QRect &rect) const {
  painter->setCompositionMode(QPainter::CompositionMode_DestinationOut);
  painter->fillRect(rect, Qt::black);
  painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

} // namespace BlossomUI
