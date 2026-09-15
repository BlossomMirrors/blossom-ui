#ifndef blossomui_core_render_selectionstyle_h
#define blossomui_core_render_selectionstyle_h

#include <QFont>
#include <QIcon>
#include <QPalette>
#include <QPixmap>

namespace BlossomUI {
namespace Render {

class SelectionStyle {
public:
  static constexpr qreal Alpha = 0.18;
  static constexpr qreal HoverAlpha = 0.28;
  static constexpr qreal MinContrast = 4.5;

  static qreal tintAlpha(bool mouseOver) {
    return mouseOver ? HoverAlpha : Alpha;
  }

  static QColor foreground(const QPalette &palette, QPalette::ColorGroup group,
                           QPalette::ColorRole surface, bool mouseOver);

  static QColor tint(const QPalette &palette, QPalette::ColorGroup group,
                     bool mouseOver);

  static QFont font(const QFont &base, bool selected);

  static QPixmap icon(const QIcon &source, const QSize &size, qreal dpr,
                      const QPalette &palette, const QColor &color, bool selected,
                      QIcon::Mode mode = QIcon::Normal,
                      QIcon::State state = QIcon::Off);
};

}
}

#endif
