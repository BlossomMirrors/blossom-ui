// SPDX-License-Identifier: GPL-2.0-or-later
#include "progressbarcontrol.h"
#include "blossomuianimations.h"
#include "blossomuistyle.h"
#include "private.h"
#include "progressbar.h"

#include <KColorUtils>
#include <QPainter>
#include <QStyleOptionProgressBar>

namespace BlossomUI {

bool Render::ProgressBarControl::drawProgressBarControl(const QStyleOption *option,
                                   QPainter *painter,
                                   const QWidget *widget) const {
  const auto progressBarOption(
      qstyleoption_cast<const QStyleOptionProgressBar *>(option));
  if (!progressBarOption)
    return true;

  // render groove
  QStyleOptionProgressBar progressBarOption2 = *progressBarOption;
  progressBarOption2.rect =
      _style->subElementRect(QStyle::SE_ProgressBarGroove, progressBarOption, widget);
  _style->drawControl(QStyle::CE_ProgressBarGroove, &progressBarOption2, painter, widget);

  const QObject *styleObject(widget ? widget : progressBarOption->styleObject);

  // enable busy animations
  // need to check both widget and passed styleObject, used for QML
  if (styleObject && _style->_animations->busyIndicatorEngine().enabled()) {
    // register QML object if defined
    if (!widget && progressBarOption->styleObject) {
      _style->_animations->busyIndicatorEngine().registerWidget(
          progressBarOption->styleObject);
    }

    _style->_animations->busyIndicatorEngine().setAnimated(
        styleObject,
        progressBarOption->maximum == 0 && progressBarOption->minimum == 0);
  }

  // check if animated and pass to option
  if (_style->_animations->busyIndicatorEngine().isAnimated(styleObject)) {
    progressBarOption2.progress = _style->_animations->busyIndicatorEngine().value();
  }

  // render contents
  progressBarOption2.rect =
      _style->subElementRect(QStyle::SE_ProgressBarContents, progressBarOption, widget);
  _style->drawControl(QStyle::CE_ProgressBarContents, &progressBarOption2, painter, widget);

  // render text
  const bool textVisible(progressBarOption->textVisible);
  const bool busy(progressBarOption->minimum == 0 &&
                  progressBarOption->maximum == 0);
  if (textVisible && !busy) {
    progressBarOption2.rect =
        _style->subElementRect(QStyle::SE_ProgressBarLabel, progressBarOption, widget);
    _style->drawControl(QStyle::CE_ProgressBarLabel, &progressBarOption2, painter, widget);
  }

  return true;
}

bool Render::ProgressBarControl::drawProgressBarContentsControl(const QStyleOption *option,
                                           QPainter *painter,
                                           const QWidget *) const {
  const auto progressBarOption(
      qstyleoption_cast<const QStyleOptionProgressBar *>(option));
  if (!progressBarOption)
    return true;

  // copy rect and palette
  auto rect(option->rect);
  const auto &palette(option->palette);

  // get direction
  const bool horizontal(
      BlossomUIPrivate::isProgressBarHorizontal(progressBarOption));
  const bool inverted(progressBarOption->invertedAppearance);
  bool reverse = horizontal && option->direction == Qt::RightToLeft;
  if (inverted)
    reverse = !reverse;

  // check if anything is to be drawn
  const bool busy(
      (progressBarOption->minimum == 0 && progressBarOption->maximum == 0));
  if (busy) {
    const qreal progress(_style->_animations->busyIndicatorEngine().value());

    const auto first = Render::progressBarBusyFirst(palette).brush.color();
    const auto second = Render::progressBarBusySecond(palette).brush.color();
    _style->_helper->renderProgressBarBusyContents(painter, rect, first, second,
                                           horizontal, reverse, progress);

  } else {
    const QRegion oldClipRegion(painter->clipRegion());
    if (horizontal) {
      if (rect.width() < Render::ProgressBar_Thickness) {
        painter->setClipRect(rect, Qt::IntersectClip);
        if (reverse)
          rect.setLeft(rect.left() - Render::ProgressBar_Thickness +
                       rect.width());
        else
          rect.setWidth(Render::ProgressBar_Thickness);
      }

    } else {
      if (rect.height() < Render::ProgressBar_Thickness) {
        painter->setClipRect(rect, Qt::IntersectClip);
        if (reverse)
          rect.setHeight(Render::ProgressBar_Thickness);
        else
          rect.setTop(rect.top() - Render::ProgressBar_Thickness +
                      rect.height());
      }
    }

    auto contentsColor(option->state.testFlag(QStyle::State_Selected)
                           ? palette.color(QPalette::HighlightedText)
                           : palette.color(QPalette::Highlight));

    _style->_helper->renderProgressBarContents(painter, rect, contentsColor);
    painter->setClipRegion(oldClipRegion);
  }

  return true;
}

bool Render::ProgressBarControl::drawProgressBarGrooveControl(const QStyleOption *option,
                                         QPainter *painter,
                                         const QWidget *) const {
  const auto &palette(option->palette);
  const auto color(
      _style->_helper->alphaColor(palette.color(QPalette::WindowText), 0.3));
  _style->_helper->renderProgressBarGroove(painter, option->rect.adjusted(1, 1, -1, -1),
                                   color);
  return true;
}

bool Render::ProgressBarControl::drawProgressBarLabelControl(const QStyleOption *option,
                                        QPainter *painter,
                                        const QWidget *) const {
  // cast option and check
  const auto progressBarOption(
      qstyleoption_cast<const QStyleOptionProgressBar *>(option));
  if (!progressBarOption)
    return true;

  // get direction and check
  const bool horizontal(
      BlossomUIPrivate::isProgressBarHorizontal(progressBarOption));
  if (!horizontal)
    return true;

  // store rect and palette
  const auto &rect(option->rect);
  const auto &palette(option->palette);

  // store state and direction
  const QStyle::State &state(option->state);
  const bool enabled(state & QStyle::State_Enabled);

  // define text rect
  const Qt::Alignment hAlign((progressBarOption->textAlignment == Qt::AlignLeft)
                                 ? Qt::AlignHCenter
                                 : progressBarOption->textAlignment);
  const QPalette::ColorRole role(
      progressBarOption->state.testFlag(QStyle::State_Selected)
          ? QPalette::HighlightedText
          : QPalette::Text);
  _style->drawItemText(painter, rect, Qt::AlignVCenter | hAlign, palette, enabled,
               progressBarOption->text, role);

  return true;
}
void Helper::renderProgressBarGroove(QPainter *painter, const QRect &rect,
                                     const QColor &color) const {
  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, true);

  const QRectF baseRect(rect);

  const qreal radius(0.5 * static_cast<qreal>(Render::ProgressBar_Thickness));

  // content
  if (color.isValid()) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawRoundedRect(baseRect, radius, radius);
  }
}

void Helper::renderProgressBarBusyContents(QPainter *painter, const QRect &rect,
                                           const QColor &first,
                                           const QColor &second,
                                           bool horizontal, bool reverse,
                                           int progress) const {
  // setup painter
  painter->setRenderHint(QPainter::Antialiasing, true);

  const QRectF baseRect(rect);
  const qreal radius(0.5 * static_cast<qreal>(Render::ProgressBar_Thickness));

  // setup brush
  QPixmap pixmap(horizontal ? 2 * Render::ProgressBar_BusyIndicatorSize : 1,
                 horizontal ? 1 : 2 * Render::ProgressBar_BusyIndicatorSize);
  pixmap.fill(second);
  if (horizontal) {
    QPainter painter(&pixmap);
    painter.setBrush(first);
    painter.setPen(Qt::NoPen);

    progress %= 2 * Render::ProgressBar_BusyIndicatorSize;
    if (reverse)
      progress = 2 * Render::ProgressBar_BusyIndicatorSize - progress - 1;
    painter.drawRect(QRect(0, 0, Render::ProgressBar_BusyIndicatorSize, 1)
                         .translated(progress, 0));

    if (progress > Render::ProgressBar_BusyIndicatorSize) {
      painter.drawRect(
          QRect(0, 0, Render::ProgressBar_BusyIndicatorSize, 1)
              .translated(progress - 2 * Render::ProgressBar_BusyIndicatorSize,
                          0));
    }

  } else {
    QPainter painter(&pixmap);
    painter.setBrush(first);
    painter.setPen(Qt::NoPen);

    progress %= 2 * Render::ProgressBar_BusyIndicatorSize;
    progress = 2 * Render::ProgressBar_BusyIndicatorSize - progress - 1;
    painter.drawRect(QRect(0, 0, 1, Render::ProgressBar_BusyIndicatorSize)
                         .translated(0, progress));

    if (progress > Render::ProgressBar_BusyIndicatorSize) {
      painter.drawRect(
          QRect(0, 0, 1, Render::ProgressBar_BusyIndicatorSize)
              .translated(0, progress -
                                 2 * Render::ProgressBar_BusyIndicatorSize));
    }
  }

  painter->setPen(Qt::NoPen);
  painter->setBrush(pixmap);
  painter->drawRoundedRect(baseRect, radius, radius);
}
bool Style::drawProgressBarControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::ProgressBarControl(this).drawProgressBarControl(option, painter, widget);
}

bool Style::drawProgressBarContentsControl(const QStyleOption *option, QPainter *painter, const QWidget * a2) const {
  return Render::ProgressBarControl(this).drawProgressBarContentsControl(option, painter, a2);
}

bool Style::drawProgressBarGrooveControl(const QStyleOption *option, QPainter *painter, const QWidget * a2) const {
  return Render::ProgressBarControl(this).drawProgressBarGrooveControl(option, painter, a2);
}

bool Style::drawProgressBarLabelControl(const QStyleOption *option, QPainter *painter, const QWidget * a2) const {
  return Render::ProgressBarControl(this).drawProgressBarLabelControl(option, painter, a2);
}
} // namespace BlossomUI
