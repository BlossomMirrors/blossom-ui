// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuihelper.h"

#include <KConfig>
#include <KConfigGroup>
#include <QApplication>
#include <QEvent>

namespace BlossomUI {

PaletteChangedEventFilter::PaletteChangedEventFilter(Helper *helper)
    : QObject(helper), _helper(helper) {}


bool PaletteChangedEventFilter::eventFilter(QObject *watched, QEvent *event) {
  if (event->type() != QEvent::ApplicationPaletteChange || watched != qApp) {
    return QObject::eventFilter(watched, event);
  }
  if (!qApp->property("KDE_COLOR_SCHEME_PATH").isValid()) {
    return QObject::eventFilter(watched, event);
  }
  const auto path = qApp->property("KDE_COLOR_SCHEME_PATH").toString();
  if (!path.isEmpty()) {
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group(config.group(QStringLiteral("WM")));
    const QPalette palette(QApplication::palette());
    _helper->_activeTitleBarColor =
        group.readEntry("activeBackground",
                        palette.color(QPalette::Active, QPalette::Highlight));
    _helper->_activeTitleBarTextColor = group.readEntry(
        "activeForeground",
        palette.color(QPalette::Active, QPalette::HighlightedText));
    _helper->_inactiveTitleBarColor =
        group.readEntry("inactiveBackground",
                        palette.color(QPalette::Disabled, QPalette::Highlight));
    _helper->_inactiveTitleBarTextColor = group.readEntry(
        "inactiveForeground",
        palette.color(QPalette::Disabled, QPalette::HighlightedText));
  }
  return QObject::eventFilter(watched, event);
}


Helper::Helper(KSharedConfig::Ptr config, QObject *parent)
    : QObject(parent), _config(std::move(config)),
      _eventFilter(new PaletteChangedEventFilter(this)) {}


KSharedConfig::Ptr Helper::config() const { return _config; }


void Helper::loadConfig() {
  _viewFocusBrush =
      KStatefulBrush(KColorScheme::View, KColorScheme::FocusColor);
  _viewHoverBrush =
      KStatefulBrush(KColorScheme::View, KColorScheme::HoverColor);
  _viewNegativeTextBrush =
      KStatefulBrush(KColorScheme::View, KColorScheme::NegativeText);
  _windowAlternateBackgroundBrush =
      KStatefulBrush(KColorScheme::Window, KColorScheme::AlternateBackground);
  _buttonAlternateBackgroundBrush =
      KStatefulBrush(KColorScheme::Button, KColorScheme::AlternateBackground);

  const QPalette palette(QApplication::palette());

  KConfig config(qApp->property("KDE_COLOR_SCHEME_PATH").toString(),
                 KConfig::SimpleConfig);
  KConfigGroup appGroup(config.group("WM"));
  KConfigGroup globalGroup(_config->group("WM"));
  _activeTitleBarColor = appGroup.readEntry(
      "activeBackground",
      globalGroup.readEntry(
          "activeBackground",
          palette.color(QPalette::Active, QPalette::Highlight)));
  _activeTitleBarTextColor = appGroup.readEntry(
      "activeForeground",
      globalGroup.readEntry(
          "activeForeground",
          palette.color(QPalette::Active, QPalette::HighlightedText)));
  _inactiveTitleBarColor = appGroup.readEntry(
      "inactiveBackground",
      globalGroup.readEntry(
          "inactiveBackground",
          palette.color(QPalette::Disabled, QPalette::Highlight)));
  _inactiveTitleBarTextColor = appGroup.readEntry(
      "inactiveForeground",
      globalGroup.readEntry(
          "inactiveForeground",
          palette.color(QPalette::Disabled, QPalette::HighlightedText)));
}

} // namespace BlossomUI
