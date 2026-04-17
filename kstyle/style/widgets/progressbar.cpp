// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuianimations.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "private.h"

#include <KColorUtils>
#include <QPainter>
#include <QStyleOptionProgressBar>

namespace BlossomUI {

bool Style::drawProgressBarControl(const QStyleOption *option,
                                   QPainter *painter,
                                   const QWidget *widget) const {
  const auto progressBarOption(
      qstyleoption_cast<const QStyleOptionProgressBar *>(option));
  if (!progressBarOption)
    return true;

  // render groove
  QStyleOptionProgressBar progressBarOption2 = *progressBarOption;
  progressBarOption2.rect =
      subElementRect(SE_ProgressBarGroove, progressBarOption, widget);
  drawControl(CE_ProgressBarGroove, &progressBarOption2, painter, widget);

  const QObject *styleObject(widget ? widget : progressBarOption->styleObject);

  // enable busy animations
  // need to check both widget and passed styleObject, used for QML
  if (styleObject && _animations->busyIndicatorEngine().enabled()) {
    // register QML object if defined
    if (!widget && progressBarOption->styleObject) {
      _animations->busyIndicatorEngine().registerWidget(
          progressBarOption->styleObject);
    }

    _animations->busyIndicatorEngine().setAnimated(
        styleObject,
        progressBarOption->maximum == 0 && progressBarOption->minimum == 0);
  }

  // check if animated and pass to option
  if (_animations->busyIndicatorEngine().isAnimated(styleObject)) {
    progressBarOption2.progress = _animations->busyIndicatorEngine().value();
  }

  // render contents
  progressBarOption2.rect =
      subElementRect(SE_ProgressBarContents, progressBarOption, widget);
  drawControl(CE_ProgressBarContents, &progressBarOption2, painter, widget);

  // render text
  const bool textVisible(progressBarOption->textVisible);
  const bool busy(progressBarOption->minimum == 0 &&
                  progressBarOption->maximum == 0);
  if (textVisible && !busy) {
    progressBarOption2.rect =
        subElementRect(SE_ProgressBarLabel, progressBarOption, widget);
    drawControl(CE_ProgressBarLabel, &progressBarOption2, painter, widget);
  }

  return true;
}

bool Style::drawProgressBarContentsControl(const QStyleOption *option,
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
    const qreal progress(_animations->busyIndicatorEngine().value());

    const auto &first = palette.color(QPalette::Highlight);
    const auto second(KColorUtils::mix(palette.color(QPalette::Highlight),
                                       palette.color(QPalette::Window), 0.7));
    _helper->renderProgressBarBusyContents(painter, rect, first, second,
                                           horizontal, reverse, progress);

  } else {
    const QRegion oldClipRegion(painter->clipRegion());
    if (horizontal) {
      if (rect.width() < Metrics::ProgressBar_Thickness) {
        painter->setClipRect(rect, Qt::IntersectClip);
        if (reverse)
          rect.setLeft(rect.left() - Metrics::ProgressBar_Thickness +
                       rect.width());
        else
          rect.setWidth(Metrics::ProgressBar_Thickness);
      }

    } else {
      if (rect.height() < Metrics::ProgressBar_Thickness) {
        painter->setClipRect(rect, Qt::IntersectClip);
        if (reverse)
          rect.setHeight(Metrics::ProgressBar_Thickness);
        else
          rect.setTop(rect.top() - Metrics::ProgressBar_Thickness +
                      rect.height());
      }
    }

    auto contentsColor(option->state.testFlag(QStyle::State_Selected)
                           ? palette.color(QPalette::HighlightedText)
                           : palette.color(QPalette::Highlight));

    _helper->renderProgressBarContents(painter, rect, contentsColor);
    painter->setClipRegion(oldClipRegion);
  }

  return true;
}

bool Style::drawProgressBarGrooveControl(const QStyleOption *option,
                                         QPainter *painter,
                                         const QWidget *) const {
  const auto &palette(option->palette);
  const auto color(
      _helper->alphaColor(palette.color(QPalette::WindowText), 0.3));
  _helper->renderProgressBarGroove(painter, option->rect.adjusted(1, 1, -1, -1),
                                   color);
  return true;
}

bool Style::drawProgressBarLabelControl(const QStyleOption *option,
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
  const State &state(option->state);
  const bool enabled(state & State_Enabled);

  // define text rect
  const Qt::Alignment hAlign((progressBarOption->textAlignment == Qt::AlignLeft)
                                 ? Qt::AlignHCenter
                                 : progressBarOption->textAlignment);
  const QPalette::ColorRole role(
      progressBarOption->state.testFlag(QStyle::State_Selected)
          ? QPalette::HighlightedText
          : QPalette::Text);
  drawItemText(painter, rect, Qt::AlignVCenter | hAlign, palette, enabled,
               progressBarOption->text, role);

  return true;
}
} // namespace BlossomUI
