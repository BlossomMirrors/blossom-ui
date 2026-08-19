// SPDX-License-Identifier: GPL-2.0-or-later
#include "scrollbarcontrol.h"
#include "blossomuianimations.h"
#include "blossomuipropertynames.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "scrollbar.h"

#include <KColorUtils>
#include <QAbstractItemView>
#include <QAbstractScrollArea>
#include <QApplication>
#include <QFrame>
#include <QPainter>
#include <QScrollBar>
#include <QStyleOptionSlider>
#include <QTreeView>

namespace BlossomUI {

bool Render::ScrollBarControl::drawPanelScrollAreaCornerPrimitive(const QStyleOption *option,
                                               QPainter *painter,
                                               const QWidget *widget) const {
  if (_style->_app.isDolphin)
    return true;
  // make sure background role matches viewport
  const QAbstractScrollArea *scrollArea;
  if ((scrollArea = qobject_cast<const QAbstractScrollArea *>(widget)) &&
      scrollArea->viewport()) {
    // need to adjust clipRect in order not to render outside of frame
    const int frameWidth(
        _style->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, scrollArea));
    painter->setClipRect(_style->insideMargin(scrollArea->rect(), frameWidth));
    painter->setBrush(scrollArea->viewport()->palette().color(
        scrollArea->viewport()->backgroundRole()));
    painter->setPen(Qt::NoPen);
    painter->drawRect(option->rect);
    return true;

  } else {
    return false;
  }
}

bool Render::ScrollBarControl::drawScrollBarSliderControl(const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *widget) const {
  // cast option and check
  const auto sliderOption(
      qstyleoption_cast<const QStyleOptionSlider *>(option));
  if (!sliderOption)
    return true;

  // copy rect and palette
  // const auto& rect( option->rect );
  const auto &palette(option->palette);

  // need to make it center due to the thin line separator
  QRect rect = option->rect;

  if (option->state & QStyle::State_Horizontal) {
    rect.setTop(PenWidth::Frame);
  } else if (option->direction == Qt::RightToLeft) {
    rect.setRight(rect.right() - PenWidth::Frame);
  } else {
    rect.setLeft(PenWidth::Frame);
  }

  // try to understand if anywhere the widget is under mouse, not just the
  // handle, use _style->_animations in case of QWidget, option->styleObject in case of
  // QML
  bool widgetMouseOver((option->state & QStyle::State_MouseOver));
  if (widget)
    widgetMouseOver = _style->_animations->scrollBarEngine().isHovered(
        widget, QStyle::SC_ScrollBarGroove);
  else if (option->styleObject)
    widgetMouseOver = option->styleObject->property("hover").toBool();

  qreal grooveAnimationOpacity(_style->_animations->scrollBarEngine().opacity(
      widget, QStyle::SC_ScrollBarGroove));
  if (grooveAnimationOpacity == AnimationData::OpacityInvalid)
    grooveAnimationOpacity = (widgetMouseOver ? 1 : 0);

  // define state
  const QStyle::State &state(option->state);
  const bool horizontal(state & QStyle::State_Horizontal);
  const bool enabled(state & QStyle::State_Enabled);
  const bool mouseOver(enabled && (state & QStyle::State_MouseOver));

  // check focus from relevant parent
  const QWidget *parent(_style->scrollBarParent(widget));
  const bool hasFocus(enabled && ((widget && widget->hasFocus()) ||
                                  (parent && parent->hasFocus())));

  // enable animation state
  const bool handleActive(sliderOption->activeSubControls & QStyle::SC_ScrollBarSlider);
  _style->_animations->scrollBarEngine().updateState(widget, AnimationFocus, hasFocus);

  _style->_animations->scrollBarEngine().updateState(widget, AnimationHover,
                                             mouseOver && handleActive);

  const auto mode(
      _style->_animations->scrollBarEngine().animationMode(widget, QStyle::SC_ScrollBarSlider));
  const qreal opacity(
      _style->_animations->scrollBarEngine().opacity(widget, QStyle::SC_ScrollBarSlider));
  Q_UNUSED(mode)
  Q_UNUSED(opacity)

  const QColor color = Render::scrollBarHandleFill(_style->_helper, palette,
                                                    state & QStyle::State_Sunken,
                                                    grooveAnimationOpacity)
                            .brush.color();

  // define handle rect
  QRectF handleRect;
  const qreal sliderWidth = static_cast<qreal>(Render::ScrollBar_SliderWidth) /
                            (2 - grooveAnimationOpacity);
  if (horizontal)
    handleRect = _style->centerRectF(rect, rect.width(), sliderWidth);
  else
    handleRect = _style->centerRectF(rect, sliderWidth, rect.height());

  _style->_helper->renderScrollBarHandle(painter, handleRect, color);
  return true;
}

bool Render::ScrollBarControl::drawScrollBarAddLineControl(const QStyleOption *option,
                                        QPainter *painter,
                                        const QWidget *widget) const {
  // do nothing if no buttons are defined
  if (_style->_addLineButtons == Style::NoButton)
    return true;

  // cast option and check
  const auto sliderOption(
      qstyleoption_cast<const QStyleOptionSlider *>(option));
  if (!sliderOption)
    return true;

  const QStyle::State &state(option->state);
  const bool horizontal(state & QStyle::State_Horizontal);
  const bool reverseLayout(option->direction == Qt::RightToLeft);

  // adjust rect, based on number of buttons to be drawn
  auto rect(_style->scrollBarInternalSubControlRect(sliderOption, QStyle::SC_ScrollBarAddLine));

  // need to make it center due to the thin line separator
  if (option->state & QStyle::State_Horizontal) {
    rect.setTop(PenWidth::Frame);
  } else if (option->direction == Qt::RightToLeft) {
    rect.setRight(rect.right() - PenWidth::Frame);
  } else {
    rect.setLeft(PenWidth::Frame);
  }

  QColor color;
  QStyleOptionSlider copy(*sliderOption);
  if (_style->_addLineButtons == Style::DoubleButton) {
    if (horizontal) {
      // Draw the arrows
      const QSize halfSize(rect.width() / 2, rect.height());
      const QRect leftSubButton(rect.topLeft(), halfSize);
      const QRect rightSubButton(leftSubButton.topRight() + QPoint(1, 0),
                                 halfSize);

      copy.rect = leftSubButton;
      color = _style->scrollBarArrowColor(
          &copy, reverseLayout ? QStyle::SC_ScrollBarAddLine : QStyle::SC_ScrollBarSubLine,
          widget);
      _style->_helper->renderArrow(painter, leftSubButton, color, ArrowLeft);

      copy.rect = rightSubButton;
      color = _style->scrollBarArrowColor(
          &copy, reverseLayout ? QStyle::SC_ScrollBarSubLine : QStyle::SC_ScrollBarAddLine,
          widget);
      _style->_helper->renderArrow(painter, rightSubButton, color, ArrowRight);

    } else {
      const QSize halfSize(rect.width(), rect.height() / 2);
      const QRect topSubButton(rect.topLeft(), halfSize);
      const QRect botSubButton(topSubButton.bottomLeft() + QPoint(0, 1),
                               halfSize);

      copy.rect = topSubButton;
      color = _style->scrollBarArrowColor(&copy, QStyle::SC_ScrollBarSubLine, widget);
      _style->_helper->renderArrow(painter, topSubButton, color, ArrowUp);

      copy.rect = botSubButton;
      color = _style->scrollBarArrowColor(&copy, QStyle::SC_ScrollBarAddLine, widget);
      _style->_helper->renderArrow(painter, botSubButton, color, ArrowDown);
    }

  } else if (_style->_addLineButtons == Style::SingleButton) {
    copy.rect = rect;
    color = _style->scrollBarArrowColor(&copy, QStyle::SC_ScrollBarAddLine, widget);
    if (horizontal) {
      if (reverseLayout)
        _style->_helper->renderArrow(painter, rect, color, ArrowLeft);
      else
        _style->_helper->renderArrow(painter, rect.translated(1, 0), color, ArrowRight);

    } else
      _style->_helper->renderArrow(painter, rect.translated(0, 1), color, ArrowDown);
  }

  return true;
}

bool Render::ScrollBarControl::drawScrollBarSubLineControl(const QStyleOption *option,
                                        QPainter *painter,
                                        const QWidget *widget) const {
  // do nothing if no buttons are set
  if (_style->_subLineButtons == Style::NoButton)
    return true;

  // cast option and check
  const auto sliderOption(
      qstyleoption_cast<const QStyleOptionSlider *>(option));
  if (!sliderOption)
    return true;

  const QStyle::State &state(option->state);
  const bool horizontal(state & QStyle::State_Horizontal);
  const bool reverseLayout(option->direction == Qt::RightToLeft);

  // adjust rect, based on number of buttons to be drawn
  auto rect(_style->scrollBarInternalSubControlRect(sliderOption, QStyle::SC_ScrollBarSubLine));

  // need to make it center due to the thin line separator
  if (option->state & QStyle::State_Horizontal) {
    rect.setTop(PenWidth::Frame);
  } else if (option->direction == Qt::RightToLeft) {
    rect.setRight(rect.right() - PenWidth::Frame);
  } else {
    rect.setLeft(PenWidth::Frame);
  }

  QColor color;
  QStyleOptionSlider copy(*sliderOption);
  if (_style->_subLineButtons == Style::DoubleButton) {
    if (horizontal) {
      // Draw the arrows
      const QSize halfSize(rect.width() / 2, rect.height());
      const QRect leftSubButton(rect.topLeft(), halfSize);
      const QRect rightSubButton(leftSubButton.topRight() + QPoint(1, 0),
                                 halfSize);

      copy.rect = leftSubButton;
      color = _style->scrollBarArrowColor(
          &copy, reverseLayout ? QStyle::SC_ScrollBarAddLine : QStyle::SC_ScrollBarSubLine,
          widget);
      _style->_helper->renderArrow(painter, leftSubButton, color, ArrowLeft);

      copy.rect = rightSubButton;
      color = _style->scrollBarArrowColor(
          &copy, reverseLayout ? QStyle::SC_ScrollBarSubLine : QStyle::SC_ScrollBarAddLine,
          widget);
      _style->_helper->renderArrow(painter, rightSubButton, color, ArrowRight);

    } else {
      const QSize halfSize(rect.width(), rect.height() / 2);
      const QRect topSubButton(rect.topLeft(), halfSize);
      const QRect botSubButton(topSubButton.bottomLeft() + QPoint(0, 1),
                               halfSize);

      copy.rect = topSubButton;
      color = _style->scrollBarArrowColor(&copy, QStyle::SC_ScrollBarSubLine, widget);
      _style->_helper->renderArrow(painter, topSubButton, color, ArrowUp);

      copy.rect = botSubButton;
      color = _style->scrollBarArrowColor(&copy, QStyle::SC_ScrollBarAddLine, widget);
      _style->_helper->renderArrow(painter, botSubButton, color, ArrowDown);
    }

  } else if (_style->_subLineButtons == Style::SingleButton) {
    copy.rect = rect;
    color = _style->scrollBarArrowColor(&copy, QStyle::SC_ScrollBarSubLine, widget);
    if (horizontal) {
      if (reverseLayout)
        _style->_helper->renderArrow(painter, rect.translated(1, 0), color, ArrowRight);
      else
        _style->_helper->renderArrow(painter, rect, color, ArrowLeft);

    } else
      _style->_helper->renderArrow(painter, rect, color, ArrowUp);
  }

  return true;
}
bool Render::ScrollBarControl::drawScrollBarComplexControl(const QStyleOptionComplex *option,
                                        QPainter *painter,
                                        const QWidget *widget) const {
  // the animation for QStyle::SC_ScrollBarGroove is special: it will animate
  // the opacity of everything else as well, included slider and arrows
  qreal opacity(_style->_animations->scrollBarEngine().opacity(
      widget, QStyle::SC_ScrollBarGroove));
  const bool animated(_style->_animations->scrollBarEngine().isAnimated(
      widget, AnimationHover, QStyle::SC_ScrollBarGroove));
  const bool mouseOver(option->state & QStyle::State_MouseOver);

  if (opacity == AnimationData::OpacityInvalid)
    opacity = 1;

  QRect separatorRect;
  if (option->state & QStyle::State_Horizontal) {
    separatorRect = QRect(0, 0, option->rect.width(), 1);
  } else {
    separatorRect = _style->alignedRect(option->direction, Qt::AlignLeft,
                                QSize(PenWidth::Frame, option->rect.height()),
                                option->rect);
  }

  if (StyleConfigData::renderThinSeperatorBetweenTheScrollBar()) {
    _style->_helper->renderScrollBarBorder(
        painter, separatorRect,
        _style->_helper->alphaColor(option->palette.color(QPalette::Text), 0.1));
  }

  // render full groove directly, rather than using the addPage and subPage
  // control element methods
  if ((mouseOver || animated) && option->subControls & QStyle::SC_ScrollBarGroove) {
    // retrieve groove rectangle
    auto grooveRect(
        _style->subControlRect(QStyle::CC_ScrollBar, option, QStyle::SC_ScrollBarGroove, widget));

    // need to make it center due to the thin line separator
    if (option->state & QStyle::State_Horizontal) {
      grooveRect.setTop(PenWidth::Frame);
    } else if (option->direction == Qt::RightToLeft) {
      grooveRect.setRight(grooveRect.right() - PenWidth::Frame);
    } else {
      grooveRect.setLeft(PenWidth::Frame);
    }

    const auto &palette(option->palette);
    const auto color(_style->_helper->alphaColor(palette.color(QPalette::WindowText),
                                         0.3 * (animated ? opacity : 1)));
    const auto &state(option->state);
    const bool horizontal(state & QStyle::State_Horizontal);

    if (horizontal)
      grooveRect = _style->centerRect(grooveRect, grooveRect.width(),
                              Render::ScrollBar_SliderWidth);
    else
      grooveRect = _style->centerRect(grooveRect, Render::ScrollBar_SliderWidth,
                              grooveRect.height());

    // render
    _style->_helper->renderScrollBarGroove(painter, grooveRect, color);
  }

  // call base class primitive
  _style->ParentStyleClass::drawComplexControl(QStyle::CC_ScrollBar, option, painter, widget);

  return true;
}

void Helper::renderScrollBarHandle(QPainter *painter, const QRectF &rect,
                                   const QColor &color) const {
  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, true);

  const QRectF baseRect(rect);
  const qreal radius(0.5 * std::min({baseRect.width(), baseRect.height()}));

  // content
  if (color.isValid()) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawRoundedRect(baseRect, radius, radius);
  }
}

void Helper::renderScrollBarBorder(QPainter *painter, const QRect &rect,
                                   const QColor &color) const {
  // content
  if (color.isValid()) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawRect(rect);
  }
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
      !qobject_cast<QAbstractItemView *>(scrollArea) &&
      !scrollArea->inherits("KItemListRoleEditor")) {
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

//* QScrollBar: remove opaque painting
bool Style::polishScrollBarOpaque(QWidget *widget) {
  if (!qobject_cast<QScrollBar *>(widget))
    return false;
  widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
  return true;
}
bool Style::drawPanelScrollAreaCornerPrimitive(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::ScrollBarControl(this).drawPanelScrollAreaCornerPrimitive(option, painter, widget);
}

bool Style::drawScrollBarSliderControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::ScrollBarControl(this).drawScrollBarSliderControl(option, painter, widget);
}

bool Style::drawScrollBarAddLineControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::ScrollBarControl(this).drawScrollBarAddLineControl(option, painter, widget);
}

bool Style::drawScrollBarSubLineControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::ScrollBarControl(this).drawScrollBarSubLineControl(option, painter, widget);
}

bool Style::drawScrollBarComplexControl(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const {
  return Render::ScrollBarControl(this).drawScrollBarComplexControl(option, painter, widget);
}
} // namespace BlossomUI
