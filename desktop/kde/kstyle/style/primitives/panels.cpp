// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuiblurhelper.h"
#include "blossomuipropertynames.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"

#include <KColorUtils>
#include <QAbstractScrollArea>
#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QStyleOptionButton>
#include <QStyleOptionFrame>
#include <QStyleOptionViewItem>
#include <QTableView>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>

#if BLOSSOMUI_HAVE_QTQUICK
#include <QQuickItem>
#endif

namespace BlossomUI {

bool Style::drawPanelButtonCommandPrimitive(const QStyleOption *option,
                                            QPainter *painter,
                                            const QWidget *widget) const {
  // cast option and check
  const auto buttonOption(
      qstyleoption_cast<const QStyleOptionButton *>(option));
  if (!buttonOption)
    return true;

  const QObject *styleObject(widget ? widget : option->styleObject);
  const bool isQtQuick = isQtQuickControl(option, widget);

  // store window state
  const bool windowActive(widget ? widget->isActiveWindow() : true);

  // rect and palette
  const auto &rect(option->rect);

  // button state
  const State &state(option->state);
  const bool enabled(state & State_Enabled);
  const bool mouseOver(enabled && (state & State_MouseOver));
  const bool hasFocus((enabled && (state & State_HasFocus)) &&
                      !(widget && widget->focusProxy()));
  const bool sunken(state & (State_On | State_Sunken));
  const bool flat(buttonOption->features & QStyleOptionButton::Flat);

#if BLOSSOMUI_HAVE_QTQUICK
  if (isQtQuick && styleObject) {
    auto *item = const_cast<QQuickItem *>(static_cast<const QQuickItem *>(styleObject));
    if (enabled)
      item->setCursor(Qt::PointingHandCursor);
    else
      item->unsetCursor();
  }
#endif

  // update animation state
  // mouse over takes precedence over focus
  _animations->widgetStateEngine().updateState(styleObject, AnimationHover,
                                               mouseOver);
  _animations->widgetStateEngine().updateState(styleObject, AnimationPressed,
                                               sunken,
                                               AnimationForwardOnly |
                                                   AnimationLongDuration);

  const AnimationMode mode(
      _animations->widgetStateEngine().buttonAnimationMode(styleObject));
  const qreal opacity(_animations->widgetStateEngine().buttonOpacity(styleObject));

  if (flat) {
    // define colors and render
    const auto &palette(option->palette);
    const auto color(_helper->toolButtonColor(palette, mouseOver, hasFocus,
                                              sunken, opacity, mode));
    _helper->renderToolButtonFrame(painter, rect, color, sunken);

  } else {
    // update button color from palette in case button is default
    QPalette palette(option->palette);
    if (enabled && buttonOption->features & QStyleOptionButton::DefaultButton) {
      const auto button(palette.color(QPalette::Button));
      const auto base(palette.color(QPalette::Base));
      palette.setColor(QPalette::Button, KColorUtils::mix(button, base, 0.7));
    }

    const qreal pressOpacity =
        _animations->widgetStateEngine().opacity(styleObject, AnimationPressed);
    const QPointF ripplePos =
        styleObject ? styleObject->property("blossomui-ripple-pos").toPointF()
                    : QPointF();
    const bool pressAnimActive = pressOpacity >= 0.0;

    const bool hasFocusForRender = hasFocus && !sunken && !pressAnimActive;
    const auto background(_helper->buttonBackgroundColor(
        palette, mouseOver, hasFocusForRender, sunken, opacity, mode));

    // render
    _helper->renderButtonFrame(painter, rect, background, palette,
                               hasFocusForRender, sunken, mouseOver, enabled,
                               windowActive, mode, opacity, ripplePos,
                               pressOpacity);
  }

  return true;
}

bool Style::drawPanelButtonToolPrimitive(const QStyleOption *option,
                                         QPainter *painter,
                                         const QWidget *widget) const {
  // copy palette and rect
  const auto &palette(option->palette);
  auto rect(option->rect);

  const QObject *styleObject(widget ? widget : option->styleObject);
  const bool isQtQuick = isQtQuickControl(option, widget);

  // store relevant flags
  const bool windowActive(widget ? widget->isActiveWindow() : true);
  const State &state(option->state);
  const bool autoRaise(state & State_AutoRaise);
  const bool enabled(state & State_Enabled);

#if BLOSSOMUI_HAVE_QTQUICK
  if (isQtQuick && styleObject) {
    auto *item = const_cast<QQuickItem *>(static_cast<const QQuickItem *>(styleObject));
    if (enabled)
      item->setCursor(Qt::PointingHandCursor);
    else
      item->unsetCursor();
  }
#endif

  const bool sunken(state & (State_On | State_Sunken));
  const bool mouseOver(enabled && (option->state & State_MouseOver));
  const bool hasFocus(enabled &&
                      (option->state & (State_HasFocus | State_Sunken)));

  /*
   * get animation state
   * no need to update hover/focus, this was already done in drawToolButtonComplexControl
   */
  _animations->widgetStateEngine().updateState(widget, AnimationPressed, sunken,
                                               AnimationForwardOnly |
                                                   AnimationLongDuration);
  const AnimationMode mode(
      _animations->widgetStateEngine().buttonAnimationMode(widget));
  const qreal opacity(_animations->widgetStateEngine().buttonOpacity(widget));
  const qreal pressOpacity =
      _animations->widgetStateEngine().opacity(widget, AnimationPressed);
  const bool pressAnimActive = pressOpacity >= 0.0;

  if (!autoRaise) {
    // need to check widget for popup mode, because option is not set properly
    const auto toolButton(qobject_cast<const QToolButton *>(widget));
    const bool hasPopupMenu(toolButton && toolButton->popupMode() ==
                                              QToolButton::MenuButtonPopup);

    const bool hasFocusForRender = hasFocus && !sunken && !pressAnimActive;
    const auto background(_helper->buttonBackgroundColor(
        palette, mouseOver, hasFocusForRender, sunken, opacity, mode));
    const QPointF ripplePos =
        widget ? widget->property("blossomui-ripple-pos").toPointF()
               : QPointF();

    // adjust frame in case of menu
    if (hasPopupMenu) {
      painter->setClipRect(rect);
      rect.adjust(0, 0, StyleConfigData::cornerRadius() + 2, 0);
      rect = visualRect(option, rect);
    }

    // render
    _helper->renderButtonFrame(painter, rect, background, palette,
                               hasFocusForRender, sunken, mouseOver, enabled,
                               windowActive, mode, opacity, ripplePos,
                               pressOpacity);

  } else {
    if (widget && widget->parentWidget()) {
      const QWidget *p = widget->parentWidget();
      if (qobject_cast<const QAbstractItemView *>(p) ||
          qobject_cast<const QAbstractItemView *>(p->parentWidget())) {
        return true;
      }
    }
    const auto color(_helper->toolButtonColor(palette, mouseOver, hasFocus,
                                              sunken, opacity, mode));
    QRect hoverRect = rect;
    const auto tbOption =
        qstyleoption_cast<const QStyleOptionToolButton *>(option);
    const bool isSplitButton =
        tbOption &&
        (tbOption->features & QStyleOptionToolButton::MenuButtonPopup);
    if (isSplitButton) {
      hoverRect.adjust(0, 0, 1, 0);
    } else if (tbOption &&
               tbOption->toolButtonStyle == Qt::ToolButtonIconOnly &&
               rect.width() != rect.height()) {
      const int size = qMin(rect.width(), rect.height());
      hoverRect = QRect(rect.x() + (rect.width() - size) / 2,
                        rect.y() + (rect.height() - size) / 2, size, size);
    }
    _helper->renderToolButtonFrame(painter, hoverRect, color, sunken);
  }

  return true;
}

bool Style::drawTabBarPanelButtonToolPrimitive(const QStyleOption *option,
                                               QPainter *painter,
                                               const QWidget *widget) const {
  // copy palette and rect
  auto rect(option->rect);

  // static_cast is safe here since check was already performed in calling
  // function
  const QTabBar *tabBar(static_cast<QTabBar *>(widget->parentWidget()));

  // overlap.
  // subtract 1, because of the empty pixel left the tabwidget frame
  const int overlap(Metrics::TabBar_BaseOverlap - 1);

  // adjust rect based on tabbar shape
  switch (tabBar->shape()) {
  case QTabBar::RoundedNorth:
  case QTabBar::TriangularNorth:
    rect.adjust(0, 0, 0, -overlap);
    break;

  case QTabBar::RoundedSouth:
  case QTabBar::TriangularSouth:
    rect.adjust(0, overlap, 0, 0);
    break;

  case QTabBar::RoundedWest:
  case QTabBar::TriangularWest:
    rect.adjust(0, 0, -overlap, 0);
    break;

  case QTabBar::RoundedEast:
  case QTabBar::TriangularEast:
    rect.adjust(overlap, 0, 0, 0);
    break;

  default:
    break;
  }

  // get the relevant palette
  const QWidget *parent(tabBar->parentWidget());
  if (qobject_cast<const QTabWidget *>(parent))
    parent = parent->parentWidget();
  const auto &palette(parent ? parent->palette() : QApplication::palette());
  const auto color = hasAlteredBackground(parent)
                         ? _helper->frameBackgroundColor(palette)
                         : palette.color(QPalette::Window);

  // render flat background
  painter->setPen(Qt::NoPen);

  if (_app.isDolphin || _app.isKonsole) {
    // change background color alpha channel
    if ((StyleConfigData::tabBarOpacity() < 100) && !_app.isOpaque) {
      // override the tab background color
      QColor tabBgColor = widget->palette().color(QPalette::Window);
      tabBgColor = _helper->transparentBarBgColor(tabBgColor, painter, rect,
                                                  BarType::TabBar);
      painter->setBrush(tabBgColor);
    } else {
      painter->setBrush(color);
    }

  } else {
    painter->setBrush(color);
  }
  painter->drawRect(rect);

  return true;
}

bool Style::drawPanelScrollAreaCornerPrimitive(const QStyleOption *option,
                                               QPainter *painter,
                                               const QWidget *widget) const {
  if (_app.isDolphin)
    return true;
  // make sure background role matches viewport
  const QAbstractScrollArea *scrollArea;
  if ((scrollArea = qobject_cast<const QAbstractScrollArea *>(widget)) &&
      scrollArea->viewport()) {
    // need to adjust clipRect in order not to render outside of frame
    const int frameWidth(
        pixelMetric(PM_DefaultFrameWidth, nullptr, scrollArea));
    painter->setClipRect(insideMargin(scrollArea->rect(), frameWidth));
    painter->setBrush(scrollArea->viewport()->palette().color(
        scrollArea->viewport()->backgroundRole()));
    painter->setPen(Qt::NoPen);
    painter->drawRect(option->rect);
    return true;

  } else {
    return false;
  }
}
bool Style::drawPanelMenuPrimitive(const QStyleOption *option,
                                   QPainter *painter,
                                   const QWidget *widget) const {
  /*
   * Skip drawing only for embedded QMenu (transparent background).
   * Draw for toplevel menus and for non-menu widgets (e.g. Slint Qt backend
   * NativeComboBoxPopup uses PE_PanelMenu with a plain QWidget, not a window).
   */
  if (widget && qobject_cast<const QMenu *>(widget) && !widget->isWindow())
    return true;

  const bool isSlintPopup(widget && !widget->isWindow() &&
                          !qobject_cast<const QMenu *>(widget));
  if (isSlintPopup && widget->window()) {
    const QRect rectInWindow(
        widget->mapTo(widget->window(), option->rect.topLeft()),
        widget->mapTo(widget->window(), option->rect.bottomRight()));
    _blurHelper->setSlintPopupRegion(widget->window(),
                                     rectInWindow.normalized());
  }

  const auto &palette(option->palette);
  const auto outline(_helper->isDarkTheme(palette) ? QColor(255, 255, 255, 20)
                                                   : QColor(0, 0, 0, 40));
  bool hasAlpha(_helper->hasAlphaChannel(widget));
  if (isSlintPopup)
    hasAlpha = true;
  auto background(_helper->frameBackgroundColor(palette));

  painter->save();

  if (isSlintPopup && widget->window()) {
    const QRect rectInWindow(
        widget->mapTo(widget->window(), option->rect.topLeft()),
        widget->mapTo(widget->window(), option->rect.bottomRight()));
    if (_helper->renderBlurredBackground(painter, widget->window(),
                                         rectInWindow.normalized(),
                                         option->rect, 12)) {
      background.setAlphaF(StyleConfigData::menuOpacity() / 100.0);
      painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
      painter->setBrush(background);
      painter->setPen(Qt::NoPen);
      painter->drawRoundedRect(option->rect, StyleConfigData::menuItemRadius(),
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
        background.setAlphaF(StyleConfigData::menuOpacity() / 100.0);
      }
      _helper->renderMenuFrame(painter, option->rect, background, outline,
                               hasAlpha);
    }
  } else {
    if (hasAlpha) {
      painter->setCompositionMode(QPainter::CompositionMode_Source);
      background.setAlphaF(StyleConfigData::menuOpacity() / 100.0);
    }
    _helper->renderMenuFrame(painter, option->rect, background, outline,
                             hasAlpha);
  }

  painter->restore();

  return true;
}

bool Style::drawPanelTipLabelPrimitive(const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // force registration of widget
  if (widget && widget->window()) {
    _shadowHelper->registerWidget(widget->window(), true);
  }

  const auto &palette(option->palette);
  const auto &background = palette.color(QPalette::ToolTipBase);
  // const auto outline( KColorUtils::mix( palette.color( QPalette::ToolTipBase
  // ), palette.color( QPalette::ToolTipText ), 0.25 ) );
  const bool hasAlpha(_helper->hasAlphaChannel(widget));

  _helper->renderMenuFrame(painter, option->rect, background, QColor(),
                           hasAlpha);
  return true;
}

bool Style::drawPanelItemViewItemPrimitive(const QStyleOption *option,
                                           QPainter *painter,
                                           const QWidget *widget) const {
  // cast option and check
  const auto viewItemOption =
      qstyleoption_cast<const QStyleOptionViewItem *>(option);
  if (!viewItemOption)
    return false;

  // try cast widget
  const auto abstractItemView = qobject_cast<const QAbstractItemView *>(widget);

  // store palette and rect
  const auto &palette(option->palette);
  auto rect(option->rect);

  // For table/tree views with a viewport: keep drawing inside the visible area
  // so we don't overflow right/bottom and so right/bottom rounded corners have
  // room to show. Only apply to QTableView/QTreeView (main content); skip
  // QListView (e.g. Dolphin Places sidebar). Don't clip for Dolphin - clipping
  // cuts off the right corner and causes asymmetric rounding.
  if (abstractItemView &&
      (qobject_cast<const QTableView *>(widget) ||
       qobject_cast<const QTreeView *>(widget)) &&
      !(_app.isDolphin && qobject_cast<const QTableView *>(widget))) {
    if (QWidget *vp = abstractItemView->viewport()) {
      const int radius = StyleConfigData::itemViewRadius();
      const QRect vpRect(0, 0, vp->width(), vp->height());
      const QRect safeRect =
          vpRect.adjusted(0, 0, -qMax(1, radius), -qMax(1, radius));
      rect = rect.intersected(safeRect);
      if (!rect.isValid())
        return true;
    }
  }

  // Clip to item rect so we never draw outside the row/card
  painter->save();
  painter->setClipRect(rect, Qt::IntersectClip);

  // store flags
  const State &state(option->state);
  const bool mouseOver(
      (state & State_MouseOver) &&
      (!abstractItemView ||
       abstractItemView->selectionMode() != QAbstractItemView::NoSelection));
  const bool selected(state & State_Selected);
  const bool enabled(state & State_Enabled);

  const bool hasCustomBackground =
      viewItemOption->backgroundBrush.style() != Qt::NoBrush &&
      !(state & State_Selected);
  const bool hasSolidBackground =
      !hasCustomBackground ||
      viewItemOption->backgroundBrush.style() == Qt::SolidPattern;
  const bool hasAlternateBackground(viewItemOption->features &
                                    QStyleOptionViewItem::Alternate);

  // do nothing if no background is to be rendered
  if (!(mouseOver || selected || hasCustomBackground ||
        hasAlternateBackground)) {
    painter->restore();
    return true;
  }

  // define color group
  QPalette::ColorGroup colorGroup;
  if (enabled)
    colorGroup = QPalette::Active;
  else
    colorGroup = QPalette::Disabled;

  // render alternate background
  if (hasAlternateBackground) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(palette.brush(colorGroup, QPalette::AlternateBase));
    painter->drawRect(rect);
  }

  // stop here if no highlight is needed
  if (!(mouseOver || selected || hasCustomBackground)) {
    painter->restore();
    return true;
  }

  // render custom background
  if (hasCustomBackground && !hasSolidBackground) {
    painter->setBrushOrigin(viewItemOption->rect.topLeft());
    painter->setBrush(viewItemOption->backgroundBrush);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(viewItemOption->rect,
                             StyleConfigData::itemViewRadius(),
                             StyleConfigData::itemViewRadius());
    painter->restore();
    return true;
  }

  QColor color;
  if (hasCustomBackground && hasSolidBackground) {
    color = viewItemOption->backgroundBrush.color();
  } else if (selected) {
    color = palette.color(colorGroup, QPalette::Highlight);
    color.setAlphaF(mouseOver ? 0.28 : 0.18);
  } else if (mouseOver) {
    color = palette.color(colorGroup, QPalette::Button);
  }

  if (widget) {
    if (widget->property(PropertyNames::sidePanelView).toBool()) {
      painter->setRenderHint(QPainter::Antialiasing);
      painter->setPen(Qt::NoPen);
      painter->setBrush(color);
      const qreal r = qMin(4.0, 0.5 * qMin(rect.width(), rect.height()));
      painter->drawRoundedRect(QRectF(rect), r, r);
      painter->restore();
      return true;
    }

    // Dolphin file list (KItemListContainer): bottom corners only, small radius
    // for symmetric corners (avoids "left circle, right none" from
    // viewItemPosition + large radius)
    if (_app.isDolphin && (qobject_cast<const QTableView *>(widget) ||
                           widget->inherits("KItemListWidget"))) {
      const qreal radius = 4;
      _helper->renderSelection(painter, rect, color, CornersBottom, radius);
      painter->restore();
      return true;
    }

    if (!(qobject_cast<const QTableView *>(widget) ||
          qobject_cast<const QListWidget *>(widget) ||
          qobject_cast<const QListView *>(widget))) {
      Corners corners;
      if (!viewItemOption->rect.isNull()) {
        if (viewItemOption->viewItemPosition ==
                QStyleOptionViewItem::Beginning ||
            viewItemOption->viewItemPosition == QStyleOptionViewItem::OnlyOne)
          corners |= CornersLeft;
        if (viewItemOption->viewItemPosition == QStyleOptionViewItem::End ||
            viewItemOption->viewItemPosition == QStyleOptionViewItem::OnlyOne)
          corners |= CornersRight;
      }

      _helper->renderSelection(painter, rect, color, corners);
      painter->restore();
      return true;
    }
  }

  _helper->renderSelection(painter, rect, color, AllCorners);

  painter->restore();
  return true;
}

bool Style::drawItemViewItemControl(const QStyleOption *option,
                                    QPainter *painter,
                                    const QWidget *widget) const {
  if (!widget || !widget->property(PropertyNames::sidePanelView).toBool())
    return false;

  const auto viewItemOption =
      qstyleoption_cast<const QStyleOptionViewItem *>(option);
  if (!viewItemOption)
    return false;

  if (!(option->state & State_Selected))
    return false;

  QStyleOptionViewItem opt = *viewItemOption;
  opt.font.setBold(true);
  opt.fontMetrics = QFontMetrics(opt.font);

  const QColor accent = option->palette.color(QPalette::Highlight);
  opt.palette.setColor(QPalette::Active, QPalette::HighlightedText, accent);
  opt.palette.setColor(QPalette::Inactive, QPalette::HighlightedText, accent);
  opt.palette.setColor(QPalette::Disabled, QPalette::HighlightedText, accent);

  ParentStyleClass::drawControl(CE_ItemViewItem, &opt, painter, widget);
  return true;
}

} // namespace BlossomUI
