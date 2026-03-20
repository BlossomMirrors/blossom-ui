/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
 * Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * Copyright 2018  Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "blossomuidecoration.h"

#include "blossomui.h"
#include "blossomuisettingsprovider.h"
#include "config-blossomui.h"

#include "blossomuibutton.h"

#include "blossomuiboxshadowrenderer.h"

#include <KDecoration3/DecorationButtonGroup>
#include <KDecoration3/DecorationShadow>
#include <KDecoration3/ScaleHelpers>

#include <KColorScheme>
#include <KColorUtils>
#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>

#include <QPainter>
#include <QTextStream>
#include <QTimer>
#include <QVariantAnimation>

#if BLOSSOMUI_HAVE_X11
#include <QX11Info>
#endif

K_PLUGIN_FACTORY_WITH_JSON(BlossomUIDecoFactory, "blossomui.json", registerPlugin<BlossomUI::Decoration>(); registerPlugin<BlossomUI::Button>();)

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
        // Fallback to the Large size.
        return s_shadowParams[3];
    }
}
}

namespace BlossomUI
{

using KDecoration3::ColorGroup;
using KDecoration3::ColorRole;

//________________________________________________________________
static int g_sDecoCount = 0;
static int g_shadowSizeEnum = InternalSettings::ShadowLarge;
static int g_shadowStrength = 255;
static QColor g_shadowColor = Qt::black;
static std::shared_ptr<KDecoration3::DecorationShadow> g_sShadow;

//________________________________________________________________
Decoration::Decoration(QObject *parent, const QVariantList &args)
    : KDecoration3::Decoration(parent, args)
    , m_animation(new QVariantAnimation(this))
{
    g_sDecoCount++;
}

//________________________________________________________________
Decoration::~Decoration()
{
    g_sDecoCount--;
    if (g_sDecoCount == 0) {
        // last deco destroyed, clean up shadow
        g_sShadow.reset();
    }
}

//________________________________________________________________
void Decoration::setOpacity(qreal value)
{
    if (m_opacity == value)
        return;
    m_opacity = value;
    update();
}

//________________________________________________________________
QColor Decoration::titleBarColor() const
{
    auto c = window();
    if (hideTitleBar())
        return c->color(ColorGroup::Inactive, ColorRole::TitleBar);
    else if (m_animation->state() == QAbstractAnimation::Running) {
        return KColorUtils::mix(c->color(ColorGroup::Inactive, ColorRole::TitleBar), c->color(ColorGroup::Active, ColorRole::TitleBar), m_opacity);
    } else
        return c->color(c->isActive() ? ColorGroup::Active : ColorGroup::Inactive, ColorRole::TitleBar);
}
//________________________________________________________________
QColor Decoration::outlineColor() const
{
    auto c(window());
    if (!m_internalSettings->drawTitleBarSeparator())
        return QColor();
    if (m_animation->state() == QAbstractAnimation::Running) {
        QColor color(c->palette().color(QPalette::Highlight));
        color.setAlpha(color.alpha() * m_opacity);
        return color;
    } else if (c->isActive())
        return c->palette().color(QPalette::Highlight);
    else
        return QColor();
}

//________________________________________________________________
QColor Decoration::fontColor() const
{
    auto c = window();
    return c->color(c->isActive() ? ColorGroup::Active : ColorGroup::Inactive, ColorRole::Foreground);
}

//________________________________________________________________
bool Decoration::init()
{
    auto c = window();

    // active state change animation
    // It is important start and end value are of the same type, hence 0.0 and not just 0
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        setOpacity(value.toReal());
    });

    reconfigure();
    updateTitleBar();
    updateBlur();
    auto s = settings();
    connect(s.get(), &KDecoration3::DecorationSettings::borderSizeChanged, this, &Decoration::recalculateBorders);

    // a change in font might cause the borders to change
    connect(s.get(), &KDecoration3::DecorationSettings::fontChanged, this, &Decoration::recalculateBorders);
    connect(s.get(), &KDecoration3::DecorationSettings::spacingChanged, this, &Decoration::recalculateBorders);

    // buttons
    connect(s.get(), &KDecoration3::DecorationSettings::spacingChanged, this, &Decoration::updateButtonsGeometryDelayed);
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsLeftChanged, this, &Decoration::updateButtonsGeometryDelayed);
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsRightChanged, this, &Decoration::updateButtonsGeometryDelayed);

    // full reconfiguration
    connect(s.get(), &KDecoration3::DecorationSettings::reconfigured, this, &Decoration::reconfigure);

    connect(s.get(), &KDecoration3::DecorationSettings::reconfigured, this, &Decoration::updateButtonsGeometryDelayed);

    connect(window(), &KDecoration3::DecoratedWindow::activeChanged, this, &Decoration::recalculateBorders);
    connect(c, &KDecoration3::DecoratedWindow::adjacentScreenEdgesChanged, this, &Decoration::recalculateBorders);
    connect(c, &KDecoration3::DecoratedWindow::maximizedHorizontallyChanged, this, &Decoration::recalculateBorders);
    connect(c, &KDecoration3::DecoratedWindow::maximizedVerticallyChanged, this, &Decoration::recalculateBorders);

    connect(c, &KDecoration3::DecoratedWindow::captionChanged, this, [this]() {
        // update the caption area
        update(titleBar());
    });

    connect(c, &KDecoration3::DecoratedWindow::activeChanged, this, &Decoration::updateAnimationState);
    connect(c, &KDecoration3::DecoratedWindow::adjacentScreenEdgesChanged, this, &Decoration::updateTitleBar);
    connect(this, &KDecoration3::Decoration::bordersChanged, this, &Decoration::updateTitleBar);

    connect(c, &KDecoration3::DecoratedWindow::activeChanged, this, &Decoration::updateBlur);
    connect(c, &KDecoration3::DecoratedWindow::widthChanged, this, &Decoration::updateTitleBar);
    connect(c, &KDecoration3::DecoratedWindow::maximizedChanged, this, &Decoration::updateTitleBar);

    connect(c, &KDecoration3::DecoratedWindow::sizeChanged, this, &Decoration::updateBlur); // recalculate blur region on resize

    connect(c, &KDecoration3::DecoratedWindow::widthChanged, this, &Decoration::updateButtonsGeometry);
    connect(c, &KDecoration3::DecoratedWindow::maximizedChanged, this, &Decoration::updateButtonsGeometry);
    connect(c, &KDecoration3::DecoratedWindow::adjacentScreenEdgesChanged, this, &Decoration::updateButtonsGeometry);

    // shade button doesn't resize properly so this is now required
    connect(this, &KDecoration3::Decoration::bordersChanged, this, &Decoration::updateButtonsGeometryDelayed);

    // update borders and colors when color scheme changes
    connect(c, &KDecoration3::DecoratedWindow::paletteChanged, this, [this]() {
        recalculateBorders();
        updateTitleBar();
        updateBlur();
        update();
    });

    createButtons();
    createShadow();
    return true;
}

//________________________________________________________________
void Decoration::updateBlur()
{
    auto c = window();
    const QColor titleBarColor = c->color(c->isActive() ? ColorGroup::Active : ColorGroup::Inactive, ColorRole::TitleBar);

    // set opaque to false when non-maximized, regardless of color (prevents kornerbug)
    if (titleBarColor.alpha() == 255) {
        this->setOpaque(c->isMaximized());
    } else {
        this->setOpaque(false);
    }

    calculateWindowAndTitleBarShapes(true);
    this->setBlurRegion(QRegion(m_windowPath->toFillPolygon().toPolygon()));
}

//________________________________________________________________
void Decoration::calculateWindowAndTitleBarShapes(const bool windowShapeOnly)
{
    auto s = settings();

    if (!windowShapeOnly) {
        // set titleBar geometry and path
        m_titleRect = QRect(QPoint(0, 0), QSize(size().width(), borderTop()));
        m_titleBarPath->clear();
        if (isMaximized() || !s->isAlphaChannelSupported()) {
            m_titleBarPath->addRect(m_titleRect);

        } else {
            QPainterPath path;
            const qreal radius = m_scaledCornerRadius;

            // expand path 1px outward on all sides to prevent AA-induced semi-transparent edge
            // QRect::bottom() = top+height-1, keeping the boundary pixel outside the fill so
            // CompositionMode_Source cant produce a fractional-alpha gap between titlebar and window during animations
            const qreal W = m_titleRect.width();
            const qreal bottom = m_titleRect.bottom();
            path.moveTo(-1.0, bottom);
            path.lineTo(-1.0, radius - 1.0);
            path.arcTo(QRectF(-1.0, -1.0, radius * 2, radius * 2), 180, -90);
            path.lineTo(W + 1.0 - radius, -1.0);
            path.arcTo(QRectF(W + 1.0 - radius * 2, -1.0, radius * 2, radius * 2), 90, -90);
            path.lineTo(W + 1.0, bottom);
            path.closeSubpath();

            *m_titleBarPath = path;
        }
    }

    // set windowPath
    m_windowPath->clear(); // clear the path for subsequent calls to this function
    if (s->isAlphaChannelSupported() && !isMaximized()) {
        // adjust by 1.0 pixel to prevent antialiasing inset and ensure smooth edges
        QRectF adjustedRect = rect().adjusted(-1.0, -1.0, 1.0, 1.0);
        m_windowPath->addRoundedRect(adjustedRect, m_scaledCornerRadius, m_scaledCornerRadius);
    } else
        m_windowPath->addRect(rect());
}

//________________________________________________________________
void Decoration::updateTitleBar()
{
    // The titlebar rect has margins around it so the window can be resized by dragging a decoration edge.
    auto s = settings();
    const bool maximized = isMaximized();
    const qreal width = maximized ? window()->width() : window()->width() - 2 * s->smallSpacing() * Metrics::TitleBar_SideMargin;
    const qreal height = (maximized || isTopEdge()) ? borderTop() : borderTop() - s->smallSpacing() * Metrics::TitleBar_TopMargin;
    const qreal x = maximized ? 0 : s->smallSpacing() * Metrics::TitleBar_SideMargin;
    const qreal y = (maximized || isTopEdge()) ? 0 : s->smallSpacing() * Metrics::TitleBar_TopMargin;
    setTitleBar(QRectF(x, y, width, height));
}

//________________________________________________________________
void Decoration::updateAnimationState()
{
    if (m_internalSettings->animationsEnabled()) {
        auto c = window();
        m_animation->setDirection(c->isActive() ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
        if (m_animation->state() != QAbstractAnimation::Running)
            m_animation->start();

    } else {
        update();
    }
}

//________________________________________________________________
qreal Decoration::borderSize(bool bottom, qreal scale) const
{
    const qreal pixelSize = KDecoration3::pixelSize(scale);
    const qreal baseSize = std::max<qreal>(pixelSize, KDecoration3::snapToPixelGrid(settings()->smallSpacing(), scale));

    if (m_internalSettings && (m_internalSettings->mask() & BorderSize)) {
        switch (m_internalSettings->borderSize()) {
        case InternalSettings::BorderNone:
            return 0;
        case InternalSettings::BorderNoSides:
            if (bottom) {
                return KDecoration3::snapToPixelGrid(std::max(4.0, baseSize + Metrics::Frame_FrameRadius), scale);
            } else {
                return 0;
            }
        default:
        case InternalSettings::BorderTiny:
            if (bottom) {
                return KDecoration3::snapToPixelGrid(std::max(4.0, baseSize), scale);
            } else {
                return baseSize;
            }
        case InternalSettings::BorderNormal:
            return baseSize * 2;
        case InternalSettings::BorderLarge:
            return baseSize * 3;
        case InternalSettings::BorderVeryLarge:
            return baseSize * 4;
        case InternalSettings::BorderHuge:
            return baseSize * 5;
        case InternalSettings::BorderVeryHuge:
            return baseSize * 6;
        case InternalSettings::BorderOversized:
            return baseSize * 10;
        }

    } else {
        switch (settings()->borderSize()) {
        case KDecoration3::BorderSize::None:
            return 0;
        case KDecoration3::BorderSize::NoSides:
            if (bottom) {
                return KDecoration3::snapToPixelGrid(std::max(4.0, baseSize + Metrics::Frame_FrameRadius), scale);
            } else {
                return 0;
            }
        default:
        case KDecoration3::BorderSize::Tiny:
            if (bottom) {
                return KDecoration3::snapToPixelGrid(std::max(4.0, baseSize), scale);
            } else {
                return baseSize;
            }
        case KDecoration3::BorderSize::Normal:
            return baseSize * 2;
        case KDecoration3::BorderSize::Large:
            return baseSize * 3;
        case KDecoration3::BorderSize::VeryLarge:
            return baseSize * 4;
        case KDecoration3::BorderSize::Huge:
            return baseSize * 5;
        case KDecoration3::BorderSize::VeryHuge:
            return baseSize * 6;
        case KDecoration3::BorderSize::Oversized:
            return baseSize * 10;
        }
    }
}

//________________________________________________________________
void Decoration::reconfigure()
{
    SettingsProvider::self()->reconfigure();

    m_internalSettings = SettingsProvider::self()->internalSettings(this);

    setScaledCornerRadius();

    // animation
    m_animation->setDuration(m_internalSettings->animationsDuration());

    // borders
    recalculateBorders();

    // shadow
    createShadow();

    updateBlur();
}

//________________________________________________________________
void Decoration::recalculateBorders()
{
    setBorders(bordersFor(window()->nextScale()));

    // extended sizes
    const qreal extSize = KDecoration3::snapToPixelGrid(settings()->largeSpacing(), window()->nextScale());
    qreal extSides = 0;
    qreal extBottom = 0;
    if (hasNoBorders()) {
        if (!isMaximizedHorizontally()) {
            extSides = extSize;
        }
        if (!isMaximizedVertically()) {
            extBottom = extSize;
        }

    } else if (hasNoSideBorders() && !isMaximizedHorizontally()) {
        extSides = extSize;
    }

    setResizeOnlyBorders(QMarginsF(extSides, 0, extSides, extBottom));

    qreal bottomLeftRadius = 0;
    qreal bottomRightRadius = 0;
    if (hasNoBorders() && m_internalSettings->roundedCorners()) {
        if (!isBottomEdge()) {
            if (!isLeftEdge()) {
                bottomLeftRadius = m_scaledCornerRadius;
            }
            if (!isRightEdge()) {
                bottomRightRadius = m_scaledCornerRadius;
            }
        }
    }
    setBorderRadius(KDecoration3::BorderRadius(0, 0, bottomRightRadius, bottomLeftRadius));

    if (window()->isMaximized() || !outlinesEnabled()) {
        setBorderOutline(KDecoration3::BorderOutline());
    } else {
        auto c = window();
        const auto frameColor = c->color(c->isActive() ? ColorGroup::Active : ColorGroup::Inactive, ColorRole::Frame);
        const auto textColor = c->palette().text().color();

        const bool isLightTheme = textColor.lightness() < 128;
        const bool isOLEDTheme = frameColor.lightness() < 5;
        const qreal mixRatio = isLightTheme ? 0.30 : (isOLEDTheme ? 0.25 : 0.12);
        const auto borderColor = KColorUtils::mix(frameColor, textColor, mixRatio);

        const qreal thickness = std::max(KDecoration3::pixelSize(window()->scale()), KDecoration3::snapToPixelGrid(1, window()->scale()));

        qreal bottomLeftRadius = 0;
        qreal bottomRightRadius = 0;
        if (!hasNoBorders() || m_internalSettings->roundedCorners()) {
            bottomLeftRadius = m_scaledCornerRadius;
            bottomRightRadius = m_scaledCornerRadius;
        }

        const auto radius = KDecoration3::BorderRadius(m_scaledCornerRadius, m_scaledCornerRadius, bottomRightRadius, bottomLeftRadius);
        setBorderOutline(KDecoration3::BorderOutline(thickness, borderColor, radius));
    }
}

//________________________________________________________________
void Decoration::createButtons()
{
    m_leftButtons = new KDecoration3::DecorationButtonGroup(KDecoration3::DecorationButtonGroup::Position::Left, this, &Button::create);
    m_rightButtons = new KDecoration3::DecorationButtonGroup(KDecoration3::DecorationButtonGroup::Position::Right, this, &Button::create);
    updateButtonsGeometry();
}

//________________________________________________________________
void Decoration::updateButtonsGeometryDelayed()
{
    QTimer::singleShot(0, this, &Decoration::updateButtonsGeometry);
}

//________________________________________________________________
void Decoration::updateButtonsGeometry()
{
    const auto s = settings();
    const qreal buttonSpacing = s->smallSpacing() * Metrics::TitleBar_ButtonSpacing;

    const auto leftButtonList = m_leftButtons->buttons();
    const auto rightButtonList = m_rightButtons->buttons();
    const auto buttonList = leftButtonList + rightButtonList;

    for (auto *b : buttonList) {
        auto btn = static_cast<Button *>(b);

        const int verticalOffset = (isTopEdge() ? s->smallSpacing() * Metrics::TitleBar_TopMargin : 0);

        const QSizeF preferredSize = btn->preferredSize();
        const int bHeight = preferredSize.height() + verticalOffset;
        const int bWidth = preferredSize.width();

        const bool isFirstInGroup =
            (leftButtonList.contains(btn) && btn == leftButtonList.first()) || (rightButtonList.contains(btn) && btn == rightButtonList.first());
        const bool isLastInGroup =
            (leftButtonList.contains(btn) && btn == leftButtonList.last()) || (rightButtonList.contains(btn) && btn == rightButtonList.last());

        const qreal leftExpand = isFirstInGroup ? 0 : buttonSpacing;
        const qreal rightExpand = isLastInGroup ? 0 : buttonSpacing;

        btn->setGeometry(QRectF(QPointF(0, 0), QSizeF(bWidth + leftExpand + rightExpand, bHeight)));
        btn->setPadding(QMargins(leftExpand, verticalOffset, rightExpand, 0));
        btn->setOffset(QPointF(leftExpand, verticalOffset));
        btn->setIconSize(QSizeF(bWidth, bHeight));
    }

    if (!m_leftButtons->buttons().isEmpty()) {
        m_leftButtons->setSpacing(0);

        const int vPadding = isTopEdge() ? 0 : s->smallSpacing() * Metrics::TitleBar_TopMargin;
        const int hPadding = s->smallSpacing() * Metrics::TitleBar_SideMargin;
        if (isLeftEdge()) {
            auto button = static_cast<Button *>(m_leftButtons->buttons().front());

            QRectF geometry = button->geometry();
            geometry.adjust(-hPadding, 0, 0, 0);
            button->setGeometry(geometry);
            button->setFlag(Button::FlagFirstInList);
            button->setLeftPadding(hPadding);
            button->setIconSize(button->preferredSize());

            m_leftButtons->setPos(QPointF(0, vPadding));

        } else {
            m_leftButtons->setPos(QPointF(hPadding + borderLeft(), vPadding));
        }
    }

    if (!m_rightButtons->buttons().isEmpty()) {
        m_rightButtons->setSpacing(0);

        const int vPadding = isTopEdge() ? 0 : s->smallSpacing() * Metrics::TitleBar_TopMargin;
        const int hPadding = s->smallSpacing() * Metrics::TitleBar_SideMargin;
        if (isRightEdge()) {
            auto button = static_cast<Button *>(m_rightButtons->buttons().back());

            QRectF geometry = button->geometry();
            geometry.adjust(0, 0, hPadding, 0);
            button->setGeometry(geometry);
            button->setFlag(Button::FlagFirstInList);
            button->setRightPadding(hPadding);
            button->setIconSize(button->preferredSize());

            m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width(), vPadding));

        } else {
            m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width() - hPadding - borderRight(), vPadding));
        }
    }

    update();
}

//________________________________________________________________
void Decoration::paint(QPainter *painter, const QRectF &repaintRegion)
{
    // TODO: optimize based on repaintRegion
    auto c = window();
    auto s = settings();

    calculateWindowAndTitleBarShapes();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setPen(Qt::NoPen);

    if (s->isAlphaChannelSupported() && !isMaximized()) {
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(rect(), Qt::transparent);
        QPainterPath roundedClip;
        roundedClip.addRoundedRect(QRectF(rect()), m_scaledCornerRadius, m_scaledCornerRadius);
        painter->setClipPath(roundedClip);
        painter->setBrush(c->color(c->isActive() ? ColorGroup::Active : ColorGroup::Inactive, ColorRole::Frame));
        painter->drawRect(QRectF(rect()));
        painter->setClipping(false);
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->setClipRect(rect());
    } else {
        painter->fillRect(rect(), Qt::transparent);
        painter->setBrush(c->color(c->isActive() ? ColorGroup::Active : ColorGroup::Inactive, ColorRole::Frame));
        painter->drawRect(rect());
    }

    if (!hideTitleBar())
        paintTitleBar(painter, repaintRegion);

    painter->restore();
}

//________________________________________________________________
void Decoration::paintTitleBar(QPainter *painter, const QRectF &repaintRegion)
{
    const auto c = window();

    const QRectF fullTitleRect(QPointF(0, 0), QSizeF(size().width(), borderTop()));
    if (!fullTitleRect.intersects(repaintRegion))
        return;

    if (!m_internalSettings)
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setPen(Qt::NoPen);

    if (c->isActive() && m_internalSettings->drawBackgroundGradient()) {
        const QColor titleBarColor(this->titleBarColor());
        QLinearGradient gradient(0, 0, 0, m_titleRect.height());
        gradient.setColorAt(0.0, titleBarColor.lighter(120));
        gradient.setColorAt(0.8, titleBarColor);
        painter->setBrush(gradient);
    } else {
        painter->setBrush(titleBarColor());
    }

    painter->drawPath(*m_titleBarPath);
    painter->restore();

    // guard against being called before init() assigns these
    if (!m_leftButtons || !m_rightButtons)
        return;

    auto s = settings();
    painter->setFont(s->font());
    painter->setPen(fontColor());
    const auto cR = captionRect();
    const QString caption = painter->fontMetrics().elidedText(c->caption(), Qt::ElideMiddle, cR.first.width());
    painter->drawText(cR.first, cR.second | Qt::TextSingleLine, caption);

    m_leftButtons->paint(painter, repaintRegion);
    m_rightButtons->paint(painter, repaintRegion);
}

//________________________________________________________________
int Decoration::buttonSize() const
{
    const int baseSize = settings()->gridUnit();
    switch (m_internalSettings->buttonSize()) {
    case InternalSettings::ButtonTiny:
        return baseSize * 1.2;
    case InternalSettings::ButtonSmall:
        return baseSize * 1.8;
    default:
    case InternalSettings::ButtonDefault:
        return baseSize * 2.4;
    case InternalSettings::ButtonLarge:
        return baseSize * 3.0;
    case InternalSettings::ButtonVeryLarge:
        return baseSize * 4.0;
    }
}

//________________________________________________________________
int Decoration::captionHeight() const
{
    return hideTitleBar() ? borderTop() : borderTop() - settings()->smallSpacing() * (Metrics::TitleBar_BottomMargin + Metrics::TitleBar_TopMargin) - 1;
}

//________________________________________________________________
QPair<QRectF, Qt::Alignment> Decoration::captionRect() const
{
    if (hideTitleBar())
        return qMakePair(QRect(), Qt::AlignCenter);
    else {
        auto c = window();
        const qreal leftOffset =
            KDecoration3::snapToPixelGrid(m_leftButtons->buttons().isEmpty() ? Metrics::TitleBar_SideMargin * settings()->smallSpacing()
                                                                             : m_leftButtons->geometry().x() + m_leftButtons->geometry().width()
                                                  + Metrics::TitleBar_SideMargin * settings()->smallSpacing(),
                                          window()->scale());

        const qreal rightOffset = KDecoration3::snapToPixelGrid(m_rightButtons->buttons().isEmpty() ? Metrics::TitleBar_SideMargin * settings()->smallSpacing()
                                                                                                    : size().width() - m_rightButtons->geometry().x()
                                                                        + Metrics::TitleBar_SideMargin * settings()->smallSpacing(),
                                                                window()->scale());

        const qreal yOffset = KDecoration3::snapToPixelGrid(settings()->smallSpacing() * Metrics::TitleBar_TopMargin, window()->scale());
        const QRectF maxRect(leftOffset, yOffset, size().width() - leftOffset - rightOffset, captionHeight());

        switch (m_internalSettings->titleAlignment()) {
        case InternalSettings::AlignLeft:
            return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignLeft);

        case InternalSettings::AlignRight:
            return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignRight);

        case InternalSettings::AlignCenter:
            return qMakePair(maxRect, Qt::AlignCenter);

        default:
        case InternalSettings::AlignCenterFullWidth: {
            // full caption rect
            const QRectF fullRect = QRect(0, yOffset, size().width(), captionHeight());
            QRectF boundingRect(settings()->fontMetrics().boundingRect(c->caption()));

            // text bounding rect
            boundingRect.setTop(yOffset);
            boundingRect.setHeight(captionHeight());
            boundingRect.moveLeft((size().width() - boundingRect.width()) / 2.0);

            if (boundingRect.left() < leftOffset)
                return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignLeft);
            else if (boundingRect.right() > size().width() - rightOffset)
                return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignRight);
            else
                return qMakePair(fullRect, Qt::AlignCenter);
        }
        }
    }
}

//________________________________________________________________
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

        // Draw outline.
        painter.setPen(withOpacity(g_shadowColor, 0.4 * strength));
        painter.setBrush(Qt::NoBrush);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawRoundedRect(innerRect, m_scaledCornerRadius - 0.5, m_scaledCornerRadius - 0.5);

        // Mask out inner rect.
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

QMarginsF Decoration::bordersFor(qreal scale) const
{
    qreal left = isLeftEdge() ? 0 : borderSize(false, scale);
    qreal right = isRightEdge() ? 0 : borderSize(false, scale);
    qreal bottom = isBottomEdge() ? 0 : borderSize(true, scale);

    qreal top = 0;
    if (hideTitleBar()) {
        top = bottom;
    } else {
        QFontMetrics fm(settings()->font());
        top += KDecoration3::snapToPixelGrid(std::max(fm.height(), buttonSize()), scale);

        // padding below
        const int baseSize = settings()->smallSpacing();
        top += KDecoration3::snapToPixelGrid(baseSize * Metrics::TitleBar_BottomMargin, scale);

        // padding above
        top += KDecoration3::snapToPixelGrid(baseSize * Metrics::TitleBar_TopMargin, scale);
    }
    return QMarginsF(left, top, right, bottom);
}

void Decoration::setScaledCornerRadius()
{
    m_scaledCornerRadius = m_internalSettings->cornerRadius() * window()->nextScale();
}

void Decoration::updateScale()
{
    setScaledCornerRadius();
    recalculateBorders();
}

} // namespace

#include "blossomuidecoration.moc"
