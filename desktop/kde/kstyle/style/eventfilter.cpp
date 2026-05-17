#include "blossomuiblurhelper.h"
#include "blossomuipropertynames.h"
#include "blossomuistyle.h"
#include "blossomuitoolsareamanager.h"
#include "widgets/switch.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QHoverEvent>
#include <QList>
#include <QListView>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QStyleOptionButton>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

namespace BlossomUI {

bool Style::eventFilter(QObject *object, QEvent *event) {
  if (event->type() == QEvent::MouseButtonPress) {
    if (qobject_cast<QPushButton *>(object) ||
        qobject_cast<QToolButton *>(object)) {
      const auto *me = static_cast<QMouseEvent *>(event);
      if (me->button() == Qt::LeftButton)
        static_cast<QWidget *>(object)->setProperty("blossomui-ripple-pos",
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                                                    me->position()
#else
                                                    me->pos()
#endif
        );
    }
  }

  if (event->type() == QEvent::HoverEnter) {
    if (qobject_cast<QPushButton *>(object) ||
        qobject_cast<QToolButton *>(object) ||
        qobject_cast<QCheckBox *>(object) ||
        qobject_cast<QRadioButton *>(object) ||
        qobject_cast<QComboBox *>(object)) {
      static_cast<QWidget *>(object)->setCursor(Qt::PointingHandCursor);
    }
  } else if (event->type() == QEvent::HoverLeave) {
    if (qobject_cast<QPushButton *>(object) ||
        qobject_cast<QToolButton *>(object) ||
        qobject_cast<QCheckBox *>(object) ||
        qobject_cast<QRadioButton *>(object) ||
        qobject_cast<QComboBox *>(object)) {
      static_cast<QWidget *>(object)->unsetCursor();
    }
  }

  if (auto dockWidget = qobject_cast<QDockWidget *>(object)) {
    return eventFilterDockWidget(dockWidget, event);
  } else if (auto subWindow = qobject_cast<QMdiSubWindow *>(object)) {
    return eventFilterMdiSubWindow(subWindow, event);
  } else if (auto commandLinkButton =
                 qobject_cast<QCommandLinkButton *>(object)) {
    return eventFilterCommandLinkButton(commandLinkButton, event);
  } else if (_app.isDolphin && qobject_cast<QListView *>(object)) {
    eventFilterDolphinListView(static_cast<QListView *>(object), event);
  }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  else if (object == qApp &&
           event->type() == QEvent::ApplicationPaletteChange) {
    configurationChanged();
  }
#endif
  // cast to QWidget
  if (object->isWidgetType()) {
    QWidget *widget = static_cast<QWidget *>(object);

    if (widget->inherits("QLineEditIconButton") &&
        event->type() == QEvent::Paint) {
      if (widget->underMouse() && widget->isEnabled()) {
        const QColor hoverColor = _helper->alphaColor(
            widget->palette().color(QPalette::WindowText), 0.1);
        if (hoverColor.isValid()) {
          QPainter painter(widget);
          painter.setRenderHint(QPainter::Antialiasing);
          const QRectF r = QRectF(widget->rect()).adjusted(1, 1, -1, -1);
          const qreal radius = qMin(3.0, 0.5 * qMin(r.width(), r.height()));
          painter.setPen(Qt::NoPen);
          painter.setBrush(hoverColor);
          painter.drawRoundedRect(r, radius, radius);
        }
      }
      return false; // let Qt draw the icon on top
    }

    if (widget->property("_blossomui_card_overlay").toBool()) {
      if (event->type() == QEvent::WindowActivate ||
          event->type() == QEvent::WindowDeactivate) {
        widget->update();
        return false;
      }
      if (event->type() == QEvent::Paint) {
        const int radius = 8;
        QPainter painter(widget);
        painter.setRenderHint(QPainter::Antialiasing);

        const QPalette::ColorGroup cg =
            widget->isActiveWindow() ? QPalette::Active : QPalette::Inactive;
        const QColor win = QApplication::palette().color(cg, QPalette::Window);
        QPainterPath cardShape;
        cardShape.addRoundedRect(QRectF(widget->rect()), radius, radius);
        QPainterPath full;
        full.addRect(widget->rect());
        painter.fillPath(full.subtracted(cardShape), win);

        const bool isDark = QApplication::palette()
                                .color(QPalette::Active, QPalette::Window)
                                .lightness() < 128;
        const QColor borderColor =
            isDark ? QColor(255, 255, 255, 35) : QColor(0, 0, 0, 22);
        painter.setPen(QPen(borderColor, 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(
            QRectF(widget->rect()).adjusted(0.5, 0.5, -0.5, -0.5), radius - 0.5,
            radius - 0.5);
        return true;
      }
      return false;
    }

    // Viewport mask for non-KItemListContainer scroll areas
    if (QWidget *parent = widget->parentWidget()) {
      if (QAbstractScrollArea *sa =
              qobject_cast<QAbstractScrollArea *>(parent)) {
        if (!(_app.isDolphin && sa->inherits("KItemListContainer"))) {
          if (sa->viewport() == widget &&
              sa->property("_blossomui_viewport_inset").isValid()) {
            const QEvent::Type t = event->type();
            if (t == QEvent::Resize || t == QEvent::Show) {
              const int inset =
                  sa->property("_blossomui_viewport_inset").toInt();
              const int radius =
                  sa->property("_blossomui_viewport_radius").toInt();
              const qreal dpr = widget->devicePixelRatioF();
              const int w = widget->width();
              const int h = widget->height();
              if (w > inset && h > inset)
                widget->setMask(_helper->roundedRectRegionBottomCorners(
                    w - inset, h - inset, radius, dpr));
              else
                widget->clearMask();
            }
          }
        }
      }
    }
    if (widget->inherits("QAbstractScrollArea") ||
        widget->inherits("KTextEditor::View")) {
      return eventFilterScrollArea(widget, event);
    } else if (widget->inherits("QComboBoxPrivateContainer")) {
      return eventFilterComboBoxContainer(widget, event);
    } else if (auto dialogButtonBox =
                   qobject_cast<QDialogButtonBox *>(object)) {
      if (widget->property(PropertyNames::forceFrame).toBool() ||
          (widget->parentWidget() &&
           widget->parentWidget()->inherits("KPageView"))) {
        // QDialogButtonBox has no paintEvent
        return eventFilterDialogButtonBox(dialogButtonBox, event);
      }
    } else if (auto checkBox = qobject_cast<QCheckBox *>(object)) {
      if (isSwitchWidget(checkBox))
        return eventFilterSwitchCheckBox(checkBox, event);
    }
    // paint background
    if (widget && event->type() == QEvent::Paint) {
      if (widget->isWindow() && !_app.isKonsole &&
          widget->testAttribute(Qt::WA_StyledBackground) &&
          widget->testAttribute(Qt::WA_TranslucentBackground)) {
        switch (widget->windowFlags() & Qt::WindowType_Mask) {
        case Qt::Window:
        case Qt::Dialog:
        case Qt::Sheet: {
          if (qobject_cast<QMenu *>(widget))
            break;
          if (!_translucentWidgets.contains(widget))
            break;
          QPainter p(widget);
          p.setClipRegion(static_cast<QPaintEvent *>(event)->region());
          p.fillRect(widget->rect(),
                     QColor(widget->palette().color(QPalette::Window)));

          // separator between the window and decoration
          if (_helper->titleBarColor(true).alphaF() * 100.0 < 100 &&
              !_app.isKonsole) {
            p.setBrush(Qt::NoBrush);
            p.setPen(QColor(0, 0, 0, 40));
            p.drawLine(widget->rect().topLeft(), widget->rect().topRight());
          }
        }
        }
      }
    }

    // update blur region if window is not completely transparent
    if (widget && widget->inherits("QWidget")) {
      // also catch if the alpha channel is set to 255
      if (widget->palette().color(QPalette::Window).alpha() <= 255) {
        if ((qobject_cast<QToolBar *>(widget) ||
             qobject_cast<QMenuBar *>(widget)) ||
            _app.isBarsOpaque || _helper->titleBarColor(true).alphaF() < 1.0) {
          if (event->type() == QEvent::Move || event->type() == QEvent::Show ||
              event->type() == QEvent::Hide) {
            if (_translucentWidgets.contains(widget->window()) &&
                !_app.isKonsole) {
              _blurHelper->forceUpdate(widget->window());
            }
          }
        }
      }
    }
  }

  // fallback
  return ParentStyleClass::eventFilter(object, event);
}

bool Style::eventFilterDialogButtonBox(QDialogButtonBox *widget,
                                       QEvent *event) {
  if (event->type() == QEvent::Paint) {
    QPainter painter(widget);
    auto paintEvent = static_cast<QPaintEvent *>(event);
    painter.setClipRegion(paintEvent->region());

    auto rect(widget->rect());
    rect.setHeight(1);
    const auto &palette(widget->palette());

    const auto color(_helper->separatorColor(palette));
    _helper->renderSeparator(&painter, rect, color, false);
  }

  return false;
}

bool Style::eventFilterPageViewHeader(QWidget *widget, QEvent *event) {
  if (event->type() == QEvent::Paint) {
    QPainter painter(widget);
    const auto &palette(_toolsAreaManager->palette());

    painter.setBrush(palette.color(QPalette::Window));
    painter.setPen(Qt::NoPen);
    painter.drawRect(widget->rect());

    // Add separator next to the search field
    if (widget->objectName() == QLatin1String("KPageView::Search")) {
      auto rect(widget->rect());
      if (widget->layoutDirection() == Qt::RightToLeft) {
        rect.setWidth(1);
      } else {
        rect.setLeft(rect.width() - 1);
      }
      rect.setHeight(rect.height() -
                     Metrics::ToolBar_SeparatorVerticalMargin * 2);
      rect.setY(Metrics::ToolBar_SeparatorVerticalMargin);

      const auto color(_helper->separatorColor(palette));
      _helper->renderSeparator(&painter, rect, color, true);
    }
  }

  return false;
}

bool Style::eventFilterSwitchCheckBox(QCheckBox *checkBox, QEvent *event) {
  if (event->type() == QEvent::Resize || event->type() == QEvent::Show) {
    QObject *ov =
        checkBox->property("blossomui-switch-overlay").value<QObject *>();
    auto *overlay = qobject_cast<BlossomUISwitchWidget *>(ov);
    if (overlay) {
      QStyleOptionButton opt;
      opt.initFrom(checkBox);
      opt.rect = checkBox->rect();
      opt.text = checkBox->text();
      opt.icon = checkBox->icon();
      opt.iconSize = checkBox->iconSize();
      QRect indRect = subElementRect(SE_CheckBoxIndicator, &opt, checkBox);
      overlay->setGeometry(indRect);
    }
  }
  return false;
}

bool Style::eventFilterScrollArea(QWidget *widget, QEvent *event) {
  auto scrollArea = qobject_cast<QAbstractScrollArea *>(widget);
  if (scrollArea)
    updateDolphinCardScrollArea(scrollArea, event->type());

  switch (event->type()) { // TODO: delete
  case QEvent::Paint: {
    if (!scrollArea)
      scrollArea = qobject_cast<QAbstractScrollArea *>(widget);
    QWidget *viewport;
    if (!(scrollArea && (viewport = scrollArea->viewport())))
      break;

    QWidget *child(nullptr);
    QList<QWidget *> children;
    if ((child =
             scrollArea->findChild<QWidget *>("qt_scrollarea_vcontainer"))) {
      if (child->isVisible()) {
        children.append(child);
        if (_app.isDolphin)
          updateDolphinCardScrollAreaScrollbar(scrollArea, true);
      } else {
        if (_app.isDolphin)
          updateDolphinCardScrollAreaScrollbar(scrollArea, false);
      }
    }

    if ((child =
             scrollArea->findChild<QWidget *>("qt_scrollarea_hcontainer")) &&
        child->isVisible()) {
      children.append(child);
    }

    if (children.empty())
      break;
    if (!scrollArea->styleSheet().isEmpty())
      break;

    QPainter painter(scrollArea);
    painter.setClipRegion(static_cast<QPaintEvent *>(event)->region());
    painter.setPen(Qt::NoPen);

    const QPalette::ColorRole role(viewport->backgroundRole());
    QColor background;
    if (role == QPalette::Window && hasAlteredBackground(viewport))
      background = _helper->frameBackgroundColor(viewport->palette());
    else
      background = viewport->palette().color(role);
    if (_app.isDolphin)
      applyDolphinCardBackground(background, scrollArea);
    painter.setBrush(background);

  } break;

  case QEvent::MouseButtonPress:
  case QEvent::MouseButtonRelease:
  case QEvent::MouseMove: {
    QMouseEvent *mouseEvent(static_cast<QMouseEvent *>(event));

    const int frameWidth(pixelMetric(PM_DefaultFrameWidth, nullptr, widget));

    QList<QScrollBar *> scrollBars;
    if (auto scrollArea = qobject_cast<QAbstractScrollArea *>(widget)) {
      if (scrollArea->horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOff)
        scrollBars.append(scrollArea->horizontalScrollBar());
      if (scrollArea->verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff)
        scrollBars.append(scrollArea->verticalScrollBar());

    } else if (widget->inherits("KTextEditor::View")) {
      scrollBars = widget->findChildren<QScrollBar *>();
    }

    foreach (QScrollBar *scrollBar, scrollBars) {
      if (!(scrollBar && scrollBar->isVisible()))
        continue;

      QPoint offset;
      if (scrollBar->orientation() == Qt::Horizontal)
        offset = QPoint(0, frameWidth);
      else
        offset =
            QPoint(QApplication::isLeftToRight() ? frameWidth : -frameWidth, 0);

      QPoint position(scrollBar->mapFrom(widget, mouseEvent->pos() - offset));

      if (!scrollBar->rect().contains(position))
        continue;

      QMouseEvent copy(mouseEvent->type(), position,
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
                       QCursor::pos(),
#endif
                       mouseEvent->button(), mouseEvent->buttons(),
                       mouseEvent->modifiers());

      QCoreApplication::sendEvent(scrollBar, &copy);
      event->setAccepted(true);
      return true;
    }

    break;
  }

  default:
    break;
  }

  return ParentStyleClass::eventFilter(widget, event);
}

bool Style::eventFilterComboBoxContainer(QWidget *widget, QEvent *event) {
  if (event->type() == QEvent::Paint) {
    QPainter painter(widget);
    auto paintEvent = static_cast<QPaintEvent *>(event);
    painter.setClipRegion(paintEvent->region());

    const auto rect(widget->rect());
    const auto &palette(widget->palette());
    const auto background(_helper->frameBackgroundColor(palette));
    const auto outline(_helper->frameOutlineColor(palette));

    const bool hasAlpha(_helper->hasAlphaChannel(widget));
    if (hasAlpha) {
      painter.setCompositionMode(QPainter::CompositionMode_Source);
    }
    _helper->renderMenuFrame(&painter, rect, background, outline, hasAlpha);
  }

  return false;
}

bool Style::eventFilterMdiSubWindow(QMdiSubWindow *subWindow, QEvent *event) {
  if (event->type() == QEvent::Paint) {
    QPainter painter(subWindow);
    QPaintEvent *paintEvent(static_cast<QPaintEvent *>(event));
    painter.setClipRegion(paintEvent->region());

    const auto rect(subWindow->rect());
    const auto background(subWindow->palette().color(QPalette::Window));

    if (subWindow->isMaximized()) {
      painter.setPen(Qt::NoPen);
      painter.setBrush(background);
      painter.drawRect(rect);

    } else {
      _helper->renderMenuFrame(&painter, rect, background, QColor());
    }
  }

  return false;
}

bool Style::eventFilterCommandLinkButton(QCommandLinkButton *button,
                                         QEvent *event) {
  if (event->type() == QEvent::Paint) {
    QPainter painter(button);
    painter.setClipRegion(static_cast<QPaintEvent *>(event)->region());

    const bool isFlat = false;

    QStyleOptionButton option;
    option.initFrom(button);
    option.features |= QStyleOptionButton::CommandLinkButton;
    if (isFlat)
      option.features |= QStyleOptionButton::Flat;
    option.text = QString();
    option.icon = QIcon();

    if (button->isChecked())
      option.state |= State_On;
    if (button->isDown())
      option.state |= State_Sunken;

    drawControl(QStyle::CE_PushButton, &option, &painter, button);

    const int margin(Metrics::Button_MarginWidth + Metrics::Frame_FrameWidth);
    QPoint offset(margin, margin);

    if (button->isDown() && !isFlat)
      painter.translate(1, 1);
    {
      offset += QPoint(1, 1);
    }

    const State &state(option.state);
    const bool enabled(state & State_Enabled);
    bool mouseOver(enabled && (state & State_MouseOver));
    bool hasFocus(enabled && (state & State_HasFocus));

    if (!button->icon().isNull()) {
      const auto pixmapSize(button->icon().actualSize(button->iconSize()));
      const QRect pixmapRect(
          QPoint(offset.x(),
                 button->description().isEmpty()
                     ? qRound((button->height() - pixmapSize.height()) / 2.0)
                     : offset.y()),
          pixmapSize);
      const qreal dpr = painter.device() ? painter.device()->devicePixelRatioF()
                                         : qApp->devicePixelRatio();
      const QPixmap pixmap(
          _helper->coloredIcon(button->icon(), button->palette(), pixmapSize,
                               dpr, enabled ? QIcon::Normal : QIcon::Disabled,
                               button->isChecked() ? QIcon::On : QIcon::Off));
      drawItemPixmap(&painter, pixmapRect, Qt::AlignCenter, pixmap);

      offset.rx() += pixmapSize.width() + Metrics::Button_ItemSpacing;
    }

    QRect textRect(offset, QSize(button->size().width() - offset.x() - margin,
                                 button->size().height() - 2 * margin));
    const QPalette::ColorRole textRole =
        (enabled && hasFocus && !mouseOver && !isFlat)
            ? QPalette::HighlightedText
            : QPalette::ButtonText;
    if (!button->text().isEmpty()) {
      QFont font(button->font());
      font.setBold(true);
      painter.setFont(font);
      if (button->description().isEmpty()) {
        drawItemText(&painter, textRect,
                     Qt::AlignLeft | Qt::AlignVCenter | Qt::TextHideMnemonic,
                     button->palette(), enabled, button->text(), textRole);

      } else {
        drawItemText(&painter, textRect,
                     Qt::AlignLeft | Qt::AlignTop | Qt::TextHideMnemonic,
                     button->palette(), enabled, button->text(), textRole);
        textRect.setTop(textRect.top() + QFontMetrics(font).height());
      }

      painter.setFont(button->font());
    }

    if (!button->description().isEmpty()) {
      drawItemText(&painter, textRect,
                   Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap,
                   button->palette(), enabled, button->description(), textRole);
    }

    return true;
  }

  return false;
}

} // namespace BlossomUI
