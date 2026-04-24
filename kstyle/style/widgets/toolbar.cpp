// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "blossomuitoolsareamanager.h"

#include <KColorUtils>
#include <QApplication>
#include <QMainWindow>
#include <QPainter>
#include <QToolBar>

namespace BlossomUI {

bool Style::drawToolBarBackgroundControl(const QStyleOption *option,
                                         QPainter *painter,
                                         const QWidget *widget) const {
  if (!widget)
    return true;

  const auto &rect(option->rect);
  auto &palette(option->palette);

  painter->setRenderHint(QPainter::Antialiasing, false);

  bool sideToolbarDolphin = false;

  if (_app.isDolphin && StyleConfigData::dolphinSidebarOpacity() < 100 &&
      !(option->state & State_Horizontal)) {
    sideToolbarDolphin = true;
  }

  else if (!isStylableToolbar(widget)) {
    return true;
  }

  // paint background
  QColor backgroundColor = palette.color(QPalette::Window);

  // menubar and toolbar background should match for consistency
  QColor opacityBackground(
      _toolsAreaManager->palette().color(QPalette::Window));

  // changes toolbar background opacity

  if (StyleConfigData::toolBarOpacity() < 100 && !_app.isOpaque) {
    opacityBackground = _helper->transparentBarBgColor(
        opacityBackground, painter, rect, BarType::ToolBar);
    painter->fillRect(rect, opacityBackground);
  }

  if (sideToolbarDolphin && _app.isDolphin) {
    backgroundColor.setAlphaF(StyleConfigData::dolphinSidebarOpacity() / 100.0 -
                              0.15);
    painter->fillRect(rect, backgroundColor);

    bool darkTheme(_helper->isDarkTheme(palette));

    // top shadow
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QColor(0, 0, 0, darkTheme ? 80 : 40));
    painter->drawLine(rect.topLeft(), rect.topRight());

    painter->drawLine(rect.topRight(), rect.bottomRight());

    painter->setPen(QColor(0, 0, 0, darkTheme ? 28 : 16));
    painter->drawLine(rect.topLeft() + QPoint(0, 1),
                      rect.topRight() + QPoint(0, 1));

    painter->setPen(QColor(0, 0, 0, darkTheme ? 6 : 3));
    painter->drawLine(rect.topLeft() + QPoint(0, 2),
                      rect.topRight() + QPoint(0, 2));

    painter->setPen(QColor(0, 0, 0, darkTheme ? 2 : 1));
    painter->drawLine(rect.topLeft() + QPoint(0, 3),
                      rect.topRight() + QPoint(0, 3));

    return true;
  }

  if (StyleConfigData::toolBarDrawSeparator() && !_app.isDolphin) {
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QColor(0, 0, 0, 40));
    painter->drawLine(rect.bottomLeft(), rect.bottomRight());
  }

  // stop here if the window is more transparent
  if (widget->window()->palette().color(QPalette::Window).alpha() <
      (StyleConfigData::toolBarOpacity() / 100.0) * 255)
    return true;

  if (!StyleConfigData::widgetDrawShadow())
    return true;

  // inner shadow effect
  if (option->state & State_Horizontal) {
    bool bellowMenuBar = false;
    if (QMainWindow *mw = qobject_cast<QMainWindow *>(widget->parentWidget())) {
      if (QWidget *mb = mw->menuWidget()) {
        if (mb->isVisible()) {
          if (mb->y() + mb->height() == widget->y())
            bellowMenuBar = true;
        }
      }
    }

    // top toolbar
    if (bellowMenuBar || widget->y() == 0) {
      QRect copy(rect);

      // adjust shadow rect if there is no widget "above" (z) the toolbar
      if (_app.isDolphin && StyleConfigData::dolphinSidebarOpacity() < 100) {
        QList<QWidget *> sidebars = widget->window()->findChildren<QWidget *>(
            QRegularExpression("^(places|terminal|info|folders)Dock$"),
            Qt::FindDirectChildrenOnly);
        for (auto sb : sidebars) {
          // directly bellow the toolbar
          if (sb->isVisible() && sb->y() == widget->y() + widget->height()) {
            // left sidebar
            if (sb->x() == 0)
              copy.adjust(sb->width(), 0, 0, 0);
            // right sidebar
            else
              copy.adjust(0, 0, -sb->width(), 0);
          }
        }
      }

      if (StyleConfigData::widgetToolBarShadow()) {
        bool darkTheme = _helper->isDarkTheme(palette);

        if (!darkTheme) {
          int shadowSize = 4;
          QRect shadowRect =
              QRect(copy.bottomLeft() - QPoint(shadowSize, -1),
                    QSize(copy.width() + shadowSize * 2, shadowSize));
          _helper->renderBoxShadow(painter, shadowRect, 0, 0, shadowSize,
                                   QColor(0, 0, 0, 160), 2, true);
        }

        else {
          QRect shadowRect(copy.bottomLeft() + QPoint(-1, 1),
                           QSize(copy.width(), 50));
          _helper->renderBoxShadow(painter, shadowRect, 0, 0, 8,
                                   QColor(0, 0, 0, 160), 2, true);
          _helper->renderBoxShadow(painter, shadowRect, 0, 0, 3,
                                   QColor(0, 0, 0, 160), 2, true);
        }
      }

    }
    // bottom toolbar
    else {
      painter->setBrush(Qt::NoBrush);
      painter->setPen(QColor(0, 0, 0, 40));
      painter->drawLine(rect.topLeft(), rect.topRight());
      painter->setPen(QColor(0, 0, 0, 12));
      painter->drawLine(rect.topLeft() + QPoint(0, 1),
                        rect.topRight() + QPoint(0, 1));
      painter->setPen(QColor(0, 0, 0, 3));
      painter->drawLine(rect.topLeft() + QPoint(0, 2),
                        rect.topRight() + QPoint(0, 2));
    }
  }

  else {
    // left toolbar
    if (widget->x() == 0) {
      painter->setBrush(Qt::NoBrush);
      QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
      gradient.setColorAt(0, QColor(0, 0, 0, 0));
      gradient.setColorAt(0.1, QColor(0, 0, 0, 40));
      gradient.setColorAt(1, QColor(0, 0, 0, 40));
      painter->setPen(QPen(gradient, 1));

      painter->drawLine(rect.topRight(), rect.bottomRight());

      gradient.setColorAt(0.1, QColor(0, 0, 0, 12));
      gradient.setColorAt(1, QColor(0, 0, 0, 12));
      painter->setPen(QPen(gradient, 1));

      painter->drawLine(rect.topRight() - QPoint(1, 0),
                        rect.bottomRight() - QPoint(1, 0));

      gradient.setColorAt(0.1, QColor(0, 0, 0, 3));
      gradient.setColorAt(1, QColor(0, 0, 0, 3));
      painter->setPen(QPen(gradient, 1));

      painter->drawLine(rect.topRight() - QPoint(2, 0),
                        rect.bottomRight() - QPoint(2, 0));
    }
    // right toolbar
    else {
      painter->setBrush(Qt::NoBrush);
      painter->setPen(QColor(0, 0, 0, 40));
      painter->drawLine(rect.topRight(), rect.bottomRight());
      painter->setPen(QColor(0, 0, 0, 12));
      painter->drawLine(rect.topRight() + QPoint(1, 0),
                        rect.bottomRight() + QPoint(1, 0));
      painter->setPen(QColor(0, 0, 0, 2));
      painter->drawLine(rect.topRight() + QPoint(2, 0),
                        rect.bottomRight() + QPoint(2, 0));
    }
  }

  return true;
}
} // namespace BlossomUI
