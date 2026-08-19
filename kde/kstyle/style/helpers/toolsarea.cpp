// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuihelper.h"
#include "blossomuipropertynames.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "blossomuitoolsareamanager.h"
#include "private.h"

#include <QDialog>
#include <QMainWindow>
#include <QMenuBar>
#include <QPainter>
#include <QToolBar>
#include <QVBoxLayout>

namespace BlossomUI {

bool Helper::shouldDrawToolsArea(const QWidget *widget) const {
  if (!widget) {
    return false;
  }
  static bool isAuto = false;
  static QString borderSize;
  if (!_cachedAutoValid) {
    KConfigGroup kdecorationGroup(
        _config->group(QStringLiteral("org.kde.kdecoration3")));
    isAuto = kdecorationGroup.readEntry("BorderSizeAuto", true);
    borderSize = kdecorationGroup.readEntry("BorderSize", "Normal");
    _cachedAutoValid = true;
  }
  if (isAuto) {
    auto window = widget->window();
    if (qobject_cast<const QDialog *>(widget)) {
      return true;
    }
    if (window) {
      auto handle = window->windowHandle();
      if (handle) {
        auto toolbar = qobject_cast<const QToolBar *>(widget);
        if (toolbar) {
          if (toolbar->isFloating()) {
            return false;
          }
        }
        return true;
      }
    } else {
      return false;
    }
  }
  if (borderSize != "None" && borderSize != "NoSides") {
    return false;
  }
  return true;
}

QColor Helper::transparentBarBgColor(QColor bgColor, QPainter *painter,
                                     const QRect &rect, BarType barType) const {
  switch (barType) {
  case BarType::MenuBar: {
    if (StyleConfigData::menuBarOpacity() == 100) {
      // opacity is at 100%
      bgColor.setAlphaF(1.0);
    } else if (StyleConfigData::menuBarOpacity() == 0) {
      // fully transparent
      bgColor.setAlphaF(0.0);
      renderTransparentArea(painter, rect);
    } else if (StyleConfigData::menuBarOpacity() < 100 &&
               StyleConfigData::menuBarOpacity() > 0) {
      // lower the opacity
      bgColor.setAlphaF(StyleConfigData::menuBarOpacity() / 100.0);
      renderTransparentArea(painter, rect);
    }
    return bgColor;
  }
  case BarType::ToolBar: {
    if (StyleConfigData::toolBarOpacity() == 100) {
      // opacity is at 100%
      bgColor.setAlphaF(1.0);
    } else if (StyleConfigData::toolBarOpacity() == 0) {
      // fully transparent
      bgColor.setAlphaF(0.0);
      renderTransparentArea(painter, rect);
    } else if (StyleConfigData::toolBarOpacity() < 100 &&
               StyleConfigData::toolBarOpacity() > 0) {
      // lower the opacity
      bgColor.setAlphaF(StyleConfigData::toolBarOpacity() / 100.0);
      renderTransparentArea(painter, rect);
    }
    return bgColor;
  }
  case BarType::TabBar: {
    if (StyleConfigData::tabBarOpacity() == 100) {
      // opacity is at 100%
      bgColor.setAlphaF(1.0);
    } else if (StyleConfigData::tabBarOpacity() == 0) {
      // fully transparent
      bgColor.setAlphaF(0.0);
      renderTransparentArea(painter, rect);
    } else if (StyleConfigData::tabBarOpacity() < 100 &&
               StyleConfigData::tabBarOpacity() > 0) {
      // lower the opacity
      bgColor.setAlphaF(StyleConfigData::tabBarOpacity() / 100.0);
      renderTransparentArea(painter, rect);
    }
    return bgColor;
  }
  default:
    return bgColor;
  }
}

//* opaque QToolBar/QMenuBar: paint background in event filter
void Style::polishOpaqueBar(QWidget *widget) {
  if (!((qobject_cast<QToolBar *>(widget) || qobject_cast<QMenuBar *>(widget)) &&
        _helper->titleBarColor(true).alphaF() * 100.0 < 100))
    return;

  // only accept top most widgets, besides the main window
  if (!widget->isWindow() && widget->parentWidget()->isWindow()) {
    // this is only valid if the window is opaque (but not forced), otherwise
    // everything will be blurred
    if (widget->palette().color(QPalette::Window).alpha() == 255 &&
        !_app.isOpaque)
      addEventFilter(widget);
  }
}

//* mirrors polishOpaqueBar
void Style::unpolishOpaqueBar(QWidget *widget) {
  if (BlossomUIPrivate::possibleTranslucentToolBars.contains(widget))
    BlossomUIPrivate::possibleTranslucentToolBars.remove(widget);
}

//* QMainWindow: styled background
bool Style::polishMainWindow(QWidget *widget) {
  if (!qobject_cast<QMainWindow *>(widget))
    return false;
  widget->setAttribute(Qt::WA_StyledBackground);
  return true;
}

//* top-level QMainWindow: paint the tools-area background/separator
bool Style::drawMainWindowToolsAreaPrimitive(const QStyleOption *option,
                                             QPainter *painter,
                                             const QWidget *widget) const {
  Q_UNUSED(option)

  auto mw = qobject_cast<const QMainWindow *>(widget);
  if (!(mw && mw == mw->window()))
    return false;

  const auto drawBackground = _toolsAreaManager->hasHeaderColors() &&
                              _helper->shouldDrawToolsArea(widget);

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
  return true;
}

//* QDialog: paint a separator under its menu bar/toolbar header region
bool Style::drawDialogHeaderSeparatorPrimitive(const QStyleOption *option,
                                               QPainter *painter,
                                               const QWidget *widget) const {
  Q_UNUSED(option)

  auto dialog = qobject_cast<const QDialog *>(widget);
  if (!dialog)
    return false;

  const auto drawBackground = _toolsAreaManager->hasHeaderColors() &&
                              _helper->shouldDrawToolsArea(widget);

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
  return true;
}

} // namespace BlossomUI
