// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuidecoration.h"
#include "blossomuiboxshadowrenderer.h"
#include "blossomui.h"

#include <KDecoration3/DecorationShadow>
#include <QPainter>

namespace
{
struct ShadowParams {
    ShadowParams()
        : offset(QPoint(0, 0))
        , radius(0)
        , opacity(0)
    {
    }

    ShadowParams(const QPoint &offset, int radius, qreal opacity)
        : offset(offset)
        , radius(radius)
        , opacity(opacity)
    {
    }

    QPoint offset;
    int radius;
    qreal opacity;
};

struct CompositeShadowParams {
    CompositeShadowParams() = default;

    CompositeShadowParams(const QPoint &offset, const ShadowParams &shadow1, const ShadowParams &shadow2)
        : offset(offset)
        , shadow1(shadow1)
        , shadow2(shadow2)
    {
    }

    bool isNone() const
    {
        return qMax(shadow1.radius, shadow2.radius) == 0;
    }

    QPoint offset;
    ShadowParams shadow1;
    ShadowParams shadow2;
};

const CompositeShadowParams s_shadowParams[] = {
    // None
    CompositeShadowParams(),
    // Small
    CompositeShadowParams(QPoint(0, 4), ShadowParams(QPoint(0, 0), 16, 1), ShadowParams(QPoint(0, -2), 8, 0.4)),
    // Medium
    CompositeShadowParams(QPoint(0, 8), ShadowParams(QPoint(0, 0), 32, 0.9), ShadowParams(QPoint(0, -4), 16, 0.3)),
    // Large
    CompositeShadowParams(QPoint(0, 12), ShadowParams(QPoint(0, 0), 48, 0.8), ShadowParams(QPoint(0, -6), 24, 0.2)),
    // Very large
    CompositeShadowParams(QPoint(0, 16), ShadowParams(QPoint(0, 0), 64, 0.7), ShadowParams(QPoint(0, -8), 32, 0.1)),
};

inline CompositeShadowParams lookupShadowParams(int size)
{
    switch (size) {
    case BlossomUI::InternalSettings::ShadowNone:
        return s_shadowParams[0];
    case BlossomUI::InternalSettings::ShadowSmall:
        return s_shadowParams[1];
    case BlossomUI::InternalSettings::ShadowMedium:
        return s_shadowParams[2];
    case BlossomUI::InternalSettings::ShadowLarge:
        return s_shadowParams[3];
    case BlossomUI::InternalSettings::ShadowVeryLarge:
        return s_shadowParams[4];
    default:
        return s_shadowParams[3];
    }
}
} // anonymous namespace

namespace BlossomUI
{

int g_sDecoCount = 0;
int g_shadowSizeEnum = InternalSettings::ShadowLarge;
int g_shadowStrength = 255;
QColor g_shadowColor = Qt::black;
std::shared_ptr<KDecoration3::DecorationShadow> g_sShadow;

void Decoration::createShadow()
{
    if (!g_sShadow || g_shadowSizeEnum != m_internalSettings->shadowSize() || g_shadowStrength != m_internalSettings->shadowStrength()
        || g_shadowColor != m_internalSettings->shadowColor()) {
        g_shadowSizeEnum = m_internalSettings->shadowSize();
        g_shadowStrength = m_internalSettings->shadowStrength();
        g_shadowColor = m_internalSettings->shadowColor();

        const CompositeShadowParams params = lookupShadowParams(g_shadowSizeEnum);
        if (params.isNone()) {
            g_sShadow.reset();
            setShadow(g_sShadow);
            return;
        }

        auto withOpacity = [](const QColor &color, qreal opacity) -> QColor {
            QColor c(color);
            c.setAlphaF(opacity);
            return c;
        };

        const QSize boxSize =
            BoxShadowRenderer::calculateMinimumBoxSize(params.shadow1.radius).expandedTo(BoxShadowRenderer::calculateMinimumBoxSize(params.shadow2.radius));

        BoxShadowRenderer shadowRenderer;
        shadowRenderer.setBorderRadius(m_scaledCornerRadius + 0.5);
        shadowRenderer.setBoxSize(boxSize);

        const qreal strength = static_cast<qreal>(g_shadowStrength) / 255.0;
        shadowRenderer.addShadow(params.shadow1.offset, params.shadow1.radius, withOpacity(g_shadowColor, params.shadow1.opacity * strength));
        shadowRenderer.addShadow(params.shadow2.offset, params.shadow2.radius, withOpacity(g_shadowColor, params.shadow2.opacity * strength));

        QImage shadowTexture = shadowRenderer.render();

        QPainter painter(&shadowTexture);
        painter.setRenderHint(QPainter::Antialiasing);

        const QRect outerRect = shadowTexture.rect();

        QRect boxRect(QPoint(0, 0), boxSize);
        boxRect.moveCenter(outerRect.center());

        const QMargins padding = QMargins(boxRect.left() - outerRect.left() - Metrics::Shadow_Overlap - params.offset.x(),
                                          boxRect.top() - outerRect.top() - Metrics::Shadow_Overlap - params.offset.y(),
                                          outerRect.right() - boxRect.right() - Metrics::Shadow_Overlap + params.offset.x(),
                                          outerRect.bottom() - boxRect.bottom() - Metrics::Shadow_Overlap + params.offset.y());
        const QRect innerRect = outerRect - padding;

        painter.setPen(withOpacity(g_shadowColor, 0.4 * strength));
        painter.setBrush(Qt::NoBrush);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawRoundedRect(innerRect, m_scaledCornerRadius - 0.5, m_scaledCornerRadius - 0.5);

        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        painter.drawRoundedRect(innerRect, m_scaledCornerRadius + 0.5, m_scaledCornerRadius + 0.5);

        painter.end();

        g_sShadow = std::make_shared<KDecoration3::DecorationShadow>();
        g_sShadow->setPadding(padding);
        g_sShadow->setInnerShadowRect(QRect(outerRect.center(), QSize(1, 1)));
        g_sShadow->setShadow(shadowTexture);
    }

    setShadow(g_sShadow);
}

} // namespace BlossomUI
