// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"

#include <KColorUtils>
#include <QAbstractScrollArea>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionFrame>

namespace BlossomUI {

bool Style::drawShapedFrameControl(const QStyleOption *option,
                                   QPainter *painter,
                                   const QWidget *widget) const {
  // cast option and check
  const auto frameOpt = qstyleoption_cast<const QStyleOptionFrame *>(option);
  if (!frameOpt)
    return false;

  switch (frameOpt->frameShape) {
  case QFrame::Box: {
    if (option->state & State_Sunken)
      return true;
    else
      break;
  }

  case QFrame::HLine:
  case QFrame::VLine: {
    const auto &rect(option->rect);
    const auto color(_helper->separatorColor(option->palette));
    const bool isVertical(frameOpt->frameShape == QFrame::VLine);
    _helper->renderSeparator(painter, rect, color, isVertical);
    return true;
  }

  case QFrame::StyledPanel: {
    if (isQtQuickControl(option, widget)) {
      // ComboBox popup frame (Qt Quick Controls)
      drawFrameMenuPrimitive(option, painter, widget);
      return true;
    }
    if (widget && qobject_cast<const QAbstractScrollArea *>(widget)) {
      return drawFramePrimitive(option, painter, widget);
    }
    if (widget && !qobject_cast<const QMenu *>(widget)) {
      drawFrameMenuPrimitive(option, painter, widget);
      return true;
    }
    break;
  }

  default:
    break;
  }

  return false;
}

bool Style::drawRubberBandControl(const QStyleOption *option, QPainter *painter,
                                  const QWidget *) const {
  painter->save();

  painter->setRenderHints(QPainter::Antialiasing);
  const auto &palette(option->palette);
  auto color = palette.color(QPalette::Highlight);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  QPen pen = KColorUtils::lighten(palette.color(QPalette::Accent));
#else
  QPen pen = KColorUtils::lighten(palette.color(QPalette::Highlight));
#endif

  if (!StyleConfigData::roundedRubberBandFrame()) {
    QPen pen = KColorUtils::mix(
        color, palette.color(QPalette::Active, QPalette::WindowText));
  }

  pen.setJoinStyle(Qt::RoundJoin);
  painter->setPen(pen);
  color.setAlpha(51); // 20% opacity
  painter->setBrush(color);
  if (StyleConfigData::roundedRubberBandFrame()) {
    painter->drawRoundedRect(_helper->strokedRect(option->rect),
                             StyleConfigData::cornerRadius(),
                             StyleConfigData::cornerRadius());

  } else {
    painter->drawRect(_helper->strokedRect(option->rect));
  }

  painter->restore();
  return true;
}
} // namespace BlossomUI
