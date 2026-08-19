// SPDX-License-Identifier: GPL-2.0-or-later
#include "misccontrol.h"
#include "blossomuipropertynames.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "blossomuitoolsareamanager.h"
#include "frame.h"

#include <KColorUtils>
#include <QAbstractButton>
#include <QAbstractScrollArea>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFrame>
#include <QMdiSubWindow>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QQuickWidget>
#include <QStyleOptionFrame>
#include <QToolBox>

namespace BlossomUI {

//* QLineEditIconButton (Qt-internal helper widget): enable hover + event filter
void Style::polishLineEditIconButton(QWidget *widget) {
  if (!widget->inherits("QLineEditIconButton"))
    return;
  widget->setAttribute(Qt::WA_Hover);
  addEventFilter(widget);
}

//* QAbstractButton child of a QDockWidget: enable hover
bool Style::polishDockWidgetButton(QWidget *widget) {
  if (!(qobject_cast<QAbstractButton *>(widget) &&
        qobject_cast<QDockWidget *>(widget->parent())))
    return false;
  widget->setAttribute(Qt::WA_Hover);
  return true;
}

//* QFrame inside a KTitleWidget
bool Style::polishTitleWidgetFrame(QWidget *widget) {
  if (!(qobject_cast<QFrame *>(widget) && widget->parent() &&
        widget->parent()->inherits("KTitleWidget")))
    return false;
  widget->setAutoFillBackground(false);
  if (!StyleConfigData::titleWidgetDrawFrame()) {
    widget->setBackgroundRole(QPalette::Window);
  }
  return true;
}

//* KTextEditor::View integration
bool Style::polishKTextEditorView(QWidget *widget) {
  if (!widget->inherits("KTextEditor::View"))
    return false;
  addEventFilter(widget);
  return true;
}

//* QDockWidget: alter palette and margins
bool Style::polishDockWidget(QWidget *widget) {
  if (!qobject_cast<QDockWidget *>(widget))
    return false;
  widget->setAutoFillBackground(false);
  widget->setContentsMargins(5, Render::Frame_FrameWidth, 5,
                             Render::Frame_FrameWidth);
  addEventFilter(widget);
  return true;
}

//* QMdiSubWindow
bool Style::polishMdiSubWindow(QWidget *widget) {
  if (!qobject_cast<QMdiSubWindow *>(widget))
    return false;
  widget->setAutoFillBackground(false);
  addEventFilter(widget);
  return true;
}

//* QDialogButtonBox
bool Style::polishDialogButtonBox(QWidget *widget) {
  if (!qobject_cast<QDialogButtonBox *>(widget))
    return false;
  addEventFilter(widget);
  return true;
}

//* KPageView title/search widgets, styled to match KDE System Settings
void Style::polishKPageViewHeaders(QWidget *widget) {
  if (!_toolsAreaManager->hasHeaderColors())
    return;
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

//* QQuickWidget nested inside a FocusHackWidget: make it translucent
void Style::polishQuickWidget(QWidget *widget) {
  if (!widget->inherits("QQuickWidget"))
    return;

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

  if (!isChildOfFocusHack)
    return;

  auto quickWidget = qobject_cast<QQuickWidget *>(widget);
  if (quickWidget) {
    quickWidget->setClearColor(Qt::transparent);
  }

  widget->setAttribute(Qt::WA_TranslucentBackground);
  widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
}

//* splitter-style separator strip for KMultiTabBar
bool Render::MiscControl::drawMultiTabBarSeparatorPrimitive(const QStyleOption *option,
                                              QPainter *painter,
                                              const QWidget *widget) const {
  if (!(widget && widget->inherits("KMultiTabBar")))
    return false;

  enum class Position {
    Left,
    Right,
    Top,
    Bottom,
  };

  const Position position =
      static_cast<Position>(widget->property("position").toInt());
  const auto splitterWidth = Render::Splitter_SplitterWidth;
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

  const auto color(_style->_helper->separatorColor(option->palette));
  _style->_helper->renderSeparator(painter, rect, color, true);
  return true;
}

bool Render::MiscControl::drawShapedFrameControl(const QStyleOption *option,
                                   QPainter *painter,
                                   const QWidget *widget) const {
  // cast option and check
  const auto frameOpt = qstyleoption_cast<const QStyleOptionFrame *>(option);
  if (!frameOpt)
    return false;

  switch (frameOpt->frameShape) {
  case QFrame::Box: {
    if (option->state & QStyle::State_Sunken)
      return true;
    else
      break;
  }

  case QFrame::HLine:
  case QFrame::VLine: {
    const auto &rect(option->rect);
    const auto color(_style->_helper->separatorColor(option->palette));
    const bool isVertical(frameOpt->frameShape == QFrame::VLine);
    _style->_helper->renderSeparator(painter, rect, color, isVertical);
    return true;
  }

  case QFrame::StyledPanel: {
    if (_style->isQtQuickControl(option, widget)) {
      // ComboBox popup frame (Qt Quick Controls)
      _style->drawFrameMenuPrimitive(option, painter, widget);
      return true;
    }
    if (widget && qobject_cast<const QAbstractScrollArea *>(widget)) {
      return _style->drawFramePrimitive(option, painter, widget);
    }
    break;
  }

  default:
    break;
  }

  return false;
}

bool Render::MiscControl::drawRubberBandControl(const QStyleOption *option, QPainter *painter,
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
    painter->drawRoundedRect(_style->_helper->strokedRect(option->rect),
                             StyleConfigData::cornerRadius(),
                             StyleConfigData::cornerRadius());

  } else {
    painter->drawRect(_style->_helper->strokedRect(option->rect));
  }

  painter->restore();
  return true;
}
bool Style::drawMultiTabBarSeparatorPrimitive(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::MiscControl(this).drawMultiTabBarSeparatorPrimitive(option, painter, widget);
}

bool Style::drawShapedFrameControl(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
  return Render::MiscControl(this).drawShapedFrameControl(option, painter, widget);
}

bool Style::drawRubberBandControl(const QStyleOption *option, QPainter *painter, const QWidget * a2) const {
  return Render::MiscControl(this).drawRubberBandControl(option, painter, a2);
}
} // namespace BlossomUI
