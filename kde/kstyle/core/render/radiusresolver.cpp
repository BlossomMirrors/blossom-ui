// SPDX-License-Identifier: GPL-2.0-or-later
#include "radiusresolver.h"

#include "blossomuihelper.h"

namespace BlossomUI {
namespace Render {

qreal RadiusResolver::resolve(const Geometry &geometry) const {
  if (geometry.radiusOverride >= 0.0)
    return geometry.radiusOverride - geometry.inset;

  qreal radius = 0.0;
  switch (geometry.radiusRole) {
  case RadiusRole::Frame:
    radius = _helper->frameRadius(PenWidth::NoPen);
    break;
  case RadiusRole::Input:
    radius = _helper->inputFrameRadius(PenWidth::NoPen);
    break;
  case RadiusRole::Tab:
    radius = _helper->tabFrameRadius(PenWidth::NoPen);
    break;
  case RadiusRole::Button:
  default:
    radius = _helper->buttonFrameRadius(PenWidth::NoPen);
    break;
  }
  return radius - geometry.inset;
}

} // namespace Render
} // namespace BlossomUI
