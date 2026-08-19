#ifndef blossomui_core_render_contentrenderer_h
#define blossomui_core_render_contentrenderer_h

// SPDX-License-Identifier: GPL-2.0-or-later

#include "contenttraits.h"
#include "widgetstate.h"

#include <QIcon>
#include <QRectF>
#include <QString>

class QPainter;
class QFontMetrics;

namespace BlossomUI {
class Helper;
class Mnemonics;

namespace Render {

enum class ContentArrangement { IconOnly, TextOnly, TextBesideIcon, TextUnderIcon };

struct ContentLayout {
  ContentArrangement arrangement = ContentArrangement::TextBesideIcon;
  Qt::Alignment alignment = Qt::AlignCenter;
  int itemSpacing = 4;
  int leftMargin = 0;
  QSize iconSize;
};

//* what a content part draws: an icon, a text, or both
struct Content {
  QIcon icon;
  QString text;
  QFont font;
  QIcon::Mode iconMode = QIcon::Normal;
  QIcon::State iconState = QIcon::Off;
  QPalette::ColorRole textRole = QPalette::ButtonText;
  int textFlags = 0;
};

struct ContentRects {
  QRectF icon;
  QRectF text;
};

class ContentRenderer {
public:
  explicit ContentRenderer(Helper *helper) : _helper(helper) {}

  ContentRects layout(const QRectF &bounds, const ContentLayout &spec,
                      const Content &content, const QFontMetrics &metrics) const;

  void paint(QPainter *painter, const QRectF &bounds, const ContentLayout &spec,
             const Content &content, const WidgetInteractionState &state) const;

private:
  Helper *_helper;
};

} // namespace Render
} // namespace BlossomUI

#endif
