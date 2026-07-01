#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPalette>
#include <QWidget>

namespace BlossomUI {

void Style::polishDolphinUrlNavigator(QWidget *) {
  // background/palette sync is handled per-paint by
  // applyDolphinUrlNavigatorStyle(); no one-time setup needed here.
}

void Style::applyDolphinUrlNavigatorStyle(const QWidget *widget,
                                          QColor &background) const {
  if (!widget->inherits("DolphinUrlNavigator"))
    return;

  QComboBox *combo = widget->findChild<QComboBox *>();
  if (!combo)
    return;

  const bool isVisible = combo->isVisible();
  if (isVisible)
    return;

  // Breadcrumb (non-editable) mode: make background translucent or hidden
  if (StyleConfigData::toolBarOpacity() < 100) {
    background.setAlphaF(StyleConfigData::toolBarOpacity() / 100.0);
  } else if (StyleConfigData::disableDolphinUrlNavigatorBackground()) {
    background.setAlphaF(0);
  }

  // Sync the embedded QLineEdit palette so it matches
  QLineEdit *dolphinLineEdit = widget->findChild<QLineEdit *>();
  if (dolphinLineEdit) {
    QPalette pal(dolphinLineEdit->palette());
    pal.setColor(QPalette::Window, background);
    dolphinLineEdit->setPalette(pal);
  }
}

} // namespace BlossomUI
