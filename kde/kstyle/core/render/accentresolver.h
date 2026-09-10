#ifndef blossomui_core_render_accentresolver_h
#define blossomui_core_render_accentresolver_h

#include <QColor>

namespace BlossomUI {
namespace Render {

class AccentResolver {
public:
  static QColor onSurface(const QColor &accent, const QColor &surface,
                          qreal minContrast);
};

}
}

#endif
