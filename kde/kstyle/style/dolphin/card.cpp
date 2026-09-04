#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "card.h"
#include "dolphin/delegate.h"

#include <KColorUtils>

#include <QAbstractScrollArea>
#include <QApplication>
#include <QEvent>
#include <QLayout>
#include <QListView>
#include <QPalette>
#include <QTableView>
#include <QTimer>
#include <QTreeView>
#include <QWidget>

namespace BlossomUI {

void Style::polishDolphinScrollArea(QAbstractScrollArea *scrollArea) {
  if (scrollArea->inherits("KItemListContainer")) {
    if (QWidget *parent = scrollArea->parentWidget();
        parent && parent->inherits("DolphinView")) {
      // Card margins: left aligns with toolbar item inset (ToolBar_FrameWidth=2
      // + PM_ToolBarItemMargin=2 = 4)
      const int margin = 8;
      const int leftMargin = 4;
      parent->setContentsMargins(leftMargin, margin, margin, margin);
      if (auto viewport = scrollArea->viewport())
        viewport->setContentsMargins(0, 0, 0, 0);

      const int radius = Render::cardRadius();
      scrollArea->setProperty("_blossomui_viewport_radius", radius);
      scrollArea->setProperty("_blossomui_card", true);
      scrollArea->installEventFilter(this);

      auto applyCardColor = [scrollArea]() {
        auto *vp = scrollArea->viewport();
        if (!vp)
          return;
        QPalette pal = vp->palette();
        vp->setPalette(pal);
        vp->setAutoFillBackground(true);
        vp->update();
      };
      QTimer::singleShot(0, scrollArea, applyCardColor);
      QTimer::singleShot(100, scrollArea, applyCardColor);

      auto *cardOverlay = new QWidget(scrollArea);
      cardOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
      cardOverlay->setAttribute(Qt::WA_TranslucentBackground);
      cardOverlay->setAttribute(Qt::WA_NoSystemBackground);
      cardOverlay->setGeometry(scrollArea->rect());
      cardOverlay->raise();
      cardOverlay->setProperty("_blossomui_card_overlay", true);
      cardOverlay->installEventFilter(this);
      scrollArea->setProperty(
          "_blossomui_card_overlay_widget",
          QVariant::fromValue(static_cast<QObject *>(cardOverlay)));

      // Apply margins to the entire Dolphin window
      QWidget *mainWindow = parent;
      while (mainWindow && !mainWindow->isWindow())
        mainWindow = mainWindow->parentWidget();
      if (mainWindow)
        mainWindow->setContentsMargins(8, 8, 8, 8);
    }
  }

  // Detail view (table/tree): inset viewport on right and bottom for symmetric
  // card corners
  if (qobject_cast<QTableView *>(scrollArea) ||
      qobject_cast<QTreeView *>(scrollArea)) {
    const int radius = StyleConfigData::itemViewRadius();
    const int inset = qMax(1, radius);
    scrollArea->setContentsMargins(0, 0, inset, inset);
  }
}

void Style::eventFilterDolphinListView(QListView *listView, QEvent *event) {
  const QEvent::Type t = event->type();
  if (t == QEvent::Show || t == QEvent::Polish || t == QEvent::LayoutRequest ||
      t == QEvent::Resize) {
    listView->setSpacing(2);
    listView->setUniformItemSizes(false);
    if (listView->inherits("KFilePlacesView") && listView->itemDelegate() &&
        !listView->property("_blossomui_places_delegate").toBool()) {
      listView->setProperty("_blossomui_places_delegate", true);
      listView->setItemDelegateForColumn(
          0, new BlossomUIPrivate::DolphinPlacesDelegate(
                 listView, listView->itemDelegate()));
    }
  }
}

void Style::updateDolphinCardScrollArea(QAbstractScrollArea *scrollArea,
                                        QEvent::Type t) {
  if (t == QEvent::ApplicationPaletteChange &&
      scrollArea->property("_blossomui_card").toBool()) {
    QTimer::singleShot(0, scrollArea, [scrollArea]() {
      auto *vp = scrollArea->viewport();
      if (!vp)
        return;
      const QColor win = QApplication::palette().color(QPalette::Window);
      const QColor cardColor = Render::cardBackgroundFill(win).brush.color();
      QPalette pal = vp->palette();
      pal.setColor(QPalette::Window, cardColor);
      pal.setColor(QPalette::Base, cardColor);
      vp->setPalette(pal);
      vp->setAutoFillBackground(true);
      vp->update();
    });
  }

  if ((t == QEvent::Resize || t == QEvent::Show || t == QEvent::Paint ||
       t == QEvent::Hide || t == QEvent::FocusIn || t == QEvent::FocusOut ||
       t == QEvent::WindowActivate || t == QEvent::WindowDeactivate) &&
      scrollArea->property("_blossomui_viewport_radius").isValid()) {
    QTimer::singleShot(0, scrollArea, [this, scrollArea]() {
      const int radius =
          scrollArea->property("_blossomui_viewport_radius").toInt();
      if (scrollArea->inherits("KItemListContainer") && scrollArea->parentWidget() && scrollArea->parentWidget()->inherits("DolphinView")) {
        if (auto *ov = qobject_cast<QWidget *>(
                scrollArea->property("_blossomui_card_overlay_widget")
                    .value<QObject *>())) {
          ov->setGeometry(scrollArea->rect());
          ov->raise();
          ov->show(); // ensure overlay is visible after dialog interactions
        }
        // reapply margins to parent DolphinView if needed
        if (QWidget *parent = scrollArea->parentWidget();
            parent && parent->inherits("DolphinView")) {
          const int margin = 8;
          const int leftMargin = 4;
          parent->setContentsMargins(leftMargin, margin, margin, margin);
        }
        // reapply margins to the entire Dolphin window
        QWidget *mainWindow = scrollArea->window();
        if (mainWindow) {
          mainWindow->setContentsMargins(8, 8, 8, 8);
        }
        // ensure viewport has autoFillBackground enabled
        if (auto *vp = scrollArea->viewport()) {
          vp->setAutoFillBackground(true);
        }
      } else if (scrollArea->property("_blossomui_viewport_inset").isValid()) {
        const int inset =
            scrollArea->property("_blossomui_viewport_inset").toInt();
        QWidget *vp = scrollArea->viewport();
        if (vp) {
          const qreal dpr = vp->devicePixelRatioF();
          const int w = vp->width();
          const int h = vp->height();
          if (w > inset && h > inset)
            vp->setMask(_helper->roundedRectRegionBottomCorners(
                w - inset, h - inset, radius, dpr));
          else
            vp->clearMask();
        }
      }
      // ensure viewport has autoFillBackground enabled for both card and detail views
      if (auto *vp = scrollArea->viewport()) {
        vp->setAutoFillBackground(true);
      }
    });
  }
}

void Style::applyDolphinCardBackground(
    QColor &background, const QAbstractScrollArea *scrollArea) const {
  if (!scrollArea->inherits("KItemListContainer"))
    return;
  const QColor win = QApplication::palette().color(QPalette::Window);
  background = Render::cardBackgroundFill(win).brush.color();
}

void Style::updateDolphinCardScrollAreaScrollbar(
    QAbstractScrollArea *scrollArea, bool scrollbarVisible) {
  if (!scrollArea->inherits("KItemListContainer"))
    return;
  QWidget *parent = scrollArea->parentWidget();
  if (!parent || !parent->inherits("DolphinView"))
    return;
  const bool current = scrollArea->property("VISIBLE-SEPARATORS").toBool();
  if (current != scrollbarVisible) {
    scrollArea->setProperty("VISIBLE-SEPARATORS", scrollbarVisible);
    scrollArea->update();
  }
}

//* Dolphin's DolphinView: zero content margins
void Style::polishDolphinView(QWidget *widget) {
  if (!(_app.isDolphin && widget->inherits("DolphinView")))
    return;
  widget->setContentsMargins(0, 0, 0, 0);
  if (auto layout = widget->layout())
    layout->setContentsMargins(0, 0, 0, 0);
}

//* Dolphin's view: disable autofill on scroll area grandchildren
void Style::polishDolphinViewAutofill(QWidget *widget) {
  if (!(_app.isDolphin &&
        qobject_cast<QAbstractScrollArea *>(getParent(widget, 2)) &&
        !qobject_cast<QAbstractScrollArea *>(getParent(widget, 3))))
    return;
  if (widget->autoFillBackground())
    widget->setAutoFillBackground(false);
}

} // namespace BlossomUI
