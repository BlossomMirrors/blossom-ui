#include "blossomuiblurhelper.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"

#include <QDockWidget>
#include <QPaintEvent>
#include <QPainter>

namespace BlossomUI {

bool Style::eventFilterDockWidget(QDockWidget *dockWidget, QEvent *event) {
  if (event->type() == QEvent::Paint) {
    QPainter painter(dockWidget);
    QPaintEvent *paintEvent = static_cast<QPaintEvent *>(event);
    painter.setClipRegion(paintEvent->region());

    const bool windowActive(dockWidget->isActiveWindow());

    const auto &palette(dockWidget->palette());
    const auto background(_helper->frameBackgroundColor(palette));
    const auto outline(_helper->frameOutlineColor(palette));
    const auto rect(dockWidget->rect());

    if (dockWidget->isFloating()) {
      _helper->renderMenuFrame(&painter, rect, background, outline, false);

    } else if (StyleConfigData::dockWidgetDrawFrame() &&
               (dockWidget->features() & (QDockWidget::DockWidgetClosable |
                                          QDockWidget::DockWidgetMovable |
                                          QDockWidget::DockWidgetFloatable))) {
      _helper->renderFrame(&painter, rect, background, windowActive);

    } else {
      if (_app.isDolphin && dockWidget->inherits("DolphinDockWidget") &&
          _translucentWidgets.contains(dockWidget->window())) {
        painter.setRenderHints(QPainter::Antialiasing, false);

        _helper->renderTransparentArea(&painter, rect);

        QColor backgroundColor = palette.color(QPalette::Window);
        backgroundColor.setAlphaF(StyleConfigData::dolphinSidebarOpacity() /
                                  100.0);
        painter.setBrush(backgroundColor);

        bool darkTheme = _helper->isDarkTheme(palette);

        painter.fillRect(rect, backgroundColor);

        // top shadow
        if (StyleConfigData::dolphinSidebarOpacity() <
                _helper->titleBarColor(true).alphaF() * 100.0 &&
            StyleConfigData::widgetDrawShadow()) {
          painter.setBrush(Qt::NoBrush);
          painter.setPen(QColor(0, 0, 0, darkTheme ? 80 : 40));
          painter.drawLine(rect.topLeft(), rect.topRight());

          painter.setPen(QColor(0, 0, 0, darkTheme ? 28 : 16));
          painter.drawLine(rect.topLeft() + QPoint(0, 1),
                           rect.topRight() + QPoint(0, 1));

          painter.setPen(QColor(0, 0, 0, darkTheme ? 6 : 3));
          painter.drawLine(rect.topLeft() + QPoint(0, 2),
                           rect.topRight() + QPoint(0, 2));

          painter.setPen(QColor(0, 0, 0, darkTheme ? 2 : 1));
          painter.drawLine(rect.topLeft() + QPoint(0, 3),
                           rect.topRight() + QPoint(0, 3));
        }

        // side shadow
        if (StyleConfigData::dolphinSidebarOpacity() <
            (palette.color(QPalette::Window).alpha() / 255.0) * 100) {
          QRect shadowRect(rect.topRight(), QSize(30, rect.height()));

          const QWidget *tabWidget =
              dockWidget->window()->findChild<const QWidget *>(
                  "tabWidget", Qt::FindDirectChildrenOnly);
          if (tabWidget) {
            if (tabWidget->x() < dockWidget->x())
              shadowRect = QRect(rect.topLeft() - QPoint(29, 0),
                                 QSize(30, rect.height()));
          }

          if (darkTheme) {
            _helper->renderBoxShadow(&painter, shadowRect.adjusted(0, 0, 0, 5),
                                     0, 0, 8, QColor(0, 0, 0, 160), 2, true);
            _helper->renderBoxShadow(&painter, shadowRect.adjusted(0, 0, 0, 5),
                                     0, 0, 3, QColor(0, 0, 0, 160), 2, true);
          } else {
            int shadowSize = 5;
            shadowRect.adjust(1, -shadowSize, 0, 0);
            _helper->renderBoxShadow(&painter, shadowRect, 0, 0, shadowSize,
                                     QColor(0, 0, 0, 120), 2, true);
          }
        }
      }
    }

  }
  // update blur region
  else if (event->type() == QEvent::Move || event->type() == QEvent::Show ||
           event->type() == QEvent::Hide) {
    if (dockWidget->inherits("DolphinDockWidget") && _app.isDolphin &&
        StyleConfigData::dolphinSidebarOpacity() < 100) {
      if (_translucentWidgets.contains(dockWidget->window()))
        _blurHelper->forceUpdate(dockWidget->window());
    }
  }

  return false;
}

} // namespace BlossomUI
