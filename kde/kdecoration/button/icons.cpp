// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomui.h"
#include "blossomuibutton.h"
#include "blossomuidecoration.h"
#include "config-blossomui.h"

#include <KColorUtils>
#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/ScaleHelpers>
#include <KIconLoader>

#include <QFile>
#include <QHash>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QVariantAnimation>
#include <QtSvg/QSvgRenderer>
#include <zlib.h>

static QHash<QString, QByteArray> s_svgDataCache;
static QHash<QString, QRectF> s_glyphBoxCache;

static QRectF glyphBox(QSvgRenderer &renderer)
{
    constexpr int resolution = 256;
    const QRectF viewBox = renderer.viewBoxF();
    QImage image(resolution, resolution, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    renderer.render(&painter, QRectF(0, 0, resolution, resolution));
    painter.end();

    int left = resolution, right = -1, top = resolution, bottom = -1;
    for (int y = 0; y < resolution; ++y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < resolution; ++x) {
            if (qAlpha(line[x]) <= 8)
                continue;
            left = qMin(left, x);
            right = qMax(right, x);
            top = qMin(top, y);
            bottom = qMax(bottom, y);
        }
    }
    if (right < left)
        return viewBox;

    const auto unitX = [&](int px) {
        return viewBox.x() + px * viewBox.width() / resolution;
    };
    const auto unitY = [&](int py) {
        return viewBox.y() + py * viewBox.height() / resolution;
    };
    return QRectF(QPointF(unitX(left), unitY(top)), QPointF(unitX(right + 1), unitY(bottom + 1)));
}

namespace BlossomUI
{

using KDecoration3::ColorGroup;
using KDecoration3::ColorRole;
using KDecoration3::DecorationButtonType;

void Button::drawIcon(QPainter *painter) const
{
    painter->setRenderHints(QPainter::Antialiasing);

    const QRectF rect = geometry().marginsRemoved(m_padding);
    painter->translate(rect.topLeft());

    const qreal width(rect.width());
    painter->scale(width / 20, width / 20);
    painter->translate(1, 1);

    const QColor backgroundColor(this->backgroundColor());
    if (backgroundColor.isValid()) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(backgroundColor);
        painter->drawEllipse(QRectF(0, 0, 18, 18));

        const QColor foregroundColor(this->foregroundColor());
        QPen borderPen(foregroundColor, 1);
        borderPen.setCosmetic(true);
        painter->setPen(borderPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QRectF(0.5, 0.5, 17, 17));
    }

    const QColor foregroundColor(this->foregroundColor());
    if (foregroundColor.isValid()) {
        QPen pen(foregroundColor);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::MiterJoin);
        pen.setWidthF(PenWidth::Symbol * qMax((qreal)1.0, 20 / width));

        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        switch (type()) {
        case DecorationButtonType::Close:
            painter->drawLine(QPointF(5, 5), QPointF(13, 13));
            painter->drawLine(13, 5, 5, 13);
            break;

        case DecorationButtonType::Maximize:
            if (isChecked()) {
                pen.setJoinStyle(Qt::RoundJoin);
                painter->setPen(pen);
                painter->drawPolygon(QVector<QPointF>{QPointF(4, 9), QPointF(9, 4), QPointF(14, 9), QPointF(9, 14)});
            } else {
                painter->drawPolyline(QVector<QPointF>{QPointF(4, 11), QPointF(9, 6), QPointF(14, 11)});
            }
            break;

        case DecorationButtonType::Minimize:
            painter->drawPolyline(QVector<QPointF>{QPointF(4, 7), QPointF(9, 12), QPointF(14, 7)});
            break;

        case DecorationButtonType::OnAllDesktops:
            painter->setPen(Qt::NoPen);
            painter->setBrush(foregroundColor);

            if (isChecked()) {
                painter->drawEllipse(QRectF(3, 3, 12, 12));

                QColor backgroundColor(this->backgroundColor());
                auto d = qobject_cast<Decoration *>(decoration());
                if (!backgroundColor.isValid() && d) {
                    backgroundColor = d->titleBarColor();
                }

                if (backgroundColor.isValid()) {
                    painter->setBrush(backgroundColor);
                    painter->drawEllipse(QRectF(8, 8, 2, 2));
                }
            } else {
                painter->drawPolygon(QVector<QPointF>{QPointF(6.5, 8.5), QPointF(12, 3), QPointF(15, 6), QPointF(9.5, 11.5)});

                painter->setPen(pen);
                painter->drawLine(QPointF(5.5, 7.5), QPointF(10.5, 12.5));
                painter->drawLine(QPointF(12, 6), QPointF(4.5, 13.5));
            }
            break;

        case DecorationButtonType::Shade:
            if (isChecked()) {
                painter->drawLine(QPointF(4, 5.5), QPointF(14, 5.5));
                painter->drawPolyline(QVector<QPointF>{QPointF(4, 8), QPointF(9, 13), QPointF(14, 8)});
            } else {
                painter->drawLine(QPointF(4, 5.5), QPointF(14, 5.5));
                painter->drawPolyline(QVector<QPointF>{QPointF(4, 13), QPointF(9, 8), QPointF(14, 13)});
            }
            break;

        case DecorationButtonType::KeepBelow:
            painter->drawPolyline(QVector<QPointF>{QPointF(4, 5), QPointF(9, 10), QPointF(14, 5)});
            painter->drawPolyline(QVector<QPointF>{QPointF(4, 9), QPointF(9, 14), QPointF(14, 9)});
            break;

        case DecorationButtonType::KeepAbove:
            painter->drawPolyline(QVector<QPointF>{QPointF(4, 9), QPointF(9, 4), QPointF(14, 9)});
            painter->drawPolyline(QVector<QPointF>{QPointF(4, 13), QPointF(9, 8), QPointF(14, 13)});
            break;

        case DecorationButtonType::ApplicationMenu:
            painter->drawRect(QRectF(3.5, 4.5, 11, 1));
            painter->drawRect(QRectF(3.5, 8.5, 11, 1));
            painter->drawRect(QRectF(3.5, 12.5, 11, 1));
            break;

        case DecorationButtonType::ContextHelp: {
            QPainterPath path;
            path.moveTo(5, 6);
            path.arcTo(QRectF(5, 3.5, 8, 5), 180, -180);
            path.cubicTo(QPointF(12.5, 9.5), QPointF(9, 7.5), QPointF(9, 11.5));
            painter->drawPath(path);
            painter->drawRect(QRectF(9, 15, 0.5, 0.5));
            break;
        }

        default:
            break;
        }
    }
}

void Button::drawIconWithMask(QPainter *painter) const
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const QRectF rect = geometry().marginsRemoved(m_padding);

    auto d = qobject_cast<Decoration *>(decoration());
    if (!d) {
        return;
    }

    QString iconName;
    switch (type()) {
    case DecorationButtonType::Close:
        iconName = QStringLiteral("window-close-symbolic");
        break;
    case DecorationButtonType::Maximize:
        iconName = isChecked() ? QStringLiteral("window-restore-symbolic") : QStringLiteral("window-maximize-symbolic");
        break;
    case DecorationButtonType::Minimize:
        iconName = QStringLiteral("window-minimize-symbolic");
        break;
    default:
        drawIcon(painter);
        return;
    }

    const QString iconPath = KIconLoader::global()->iconPath(iconName, KIconLoader::Small, true);
    if (iconPath.isEmpty()) {
        drawIcon(painter);
        return;
    }

    QByteArray svgData;
    auto it = s_svgDataCache.find(iconPath);
    if (it != s_svgDataCache.end()) {
        svgData = it.value();
    } else {
        QFile svgFile(iconPath);
        if (!svgFile.open(QIODevice::ReadOnly)) {
            drawIcon(painter);
            return;
        }
        QByteArray rawData = svgFile.readAll();
        svgFile.close();

        if (iconPath.endsWith(QLatin1String(".svgz"), Qt::CaseInsensitive)
            || (rawData.size() >= 2 && (quint8)rawData[0] == 0x1f && (quint8)rawData[1] == 0x8b)) {
            z_stream zs{};
            if (inflateInit2(&zs, 15 + 32) == Z_OK) {
                zs.next_in = reinterpret_cast<Bytef *>(rawData.data());
                zs.avail_in = rawData.size();
                char buf[65536];
                int ret;
                do {
                    zs.next_out = reinterpret_cast<Bytef *>(buf);
                    zs.avail_out = sizeof(buf);
                    ret = inflate(&zs, Z_NO_FLUSH);
                    svgData.append(buf, sizeof(buf) - zs.avail_out);
                } while (ret == Z_OK);
                inflateEnd(&zs);
                if (ret != Z_STREAM_END)
                    svgData.clear();
            }
        } else {
            svgData = rawData;
        }

        if (svgData.isEmpty()) {
            drawIcon(painter);
            return;
        }
        s_svgDataCache.insert(iconPath, svgData);
    }

    if (svgData.isEmpty()) {
        drawIcon(painter);
        return;
    }

    const qreal hoverProgress = isPressed() ? 1.0 : (m_animation->state() == QAbstractAnimation::Running) ? m_opacity : (isHovered() ? 1.0 : 0.0);
    const bool isClose = type() == DecorationButtonType::Close;
    const bool lightTitleBar = KColorUtils::luma(d->titleBarColor()) > KColorUtils::luma(d->fontColor());

    // on hover the close icon turns light red on the red fill and in dark mode the other icons take the titlebar color on their light fill
    // in light mode the fill stays light so those icons keep the text color
    const QColor activeIconColor = isClose ? QColor(BLOSSOMUI_RED_50) : (lightTitleBar ? d->fontColor() : d->titleBarColor());
    const QColor iconColor = KColorUtils::mix(d->fontColor(), activeIconColor, hoverProgress);

    QSvgRenderer renderer;
    renderer.load(svgData);
    if (!renderer.isValid()) {
        drawIcon(painter);
        return;
    }

    // fractional scaling puts logical pixels between real ones so the box is snapped to real pixels
    const qreal scale = d->window()->scale();
    const int buttonMargin = 1;
    const qreal hoverInset = 2;
    const QRectF drawRect = rect.adjusted(buttonMargin, buttonMargin, -buttonMargin, -buttonMargin);
    const QRectF hoverRect = KDecoration3::snapToPixelGrid(drawRect.adjusted(hoverInset, hoverInset, -hoverInset, -hoverInset), scale);
    const QSize hoverPixels(qRound(hoverRect.width() * scale), qRound(hoverRect.height() * scale));

    auto glyphIt = s_glyphBoxCache.find(iconPath);
    if (glyphIt == s_glyphBoxCache.end())
        glyphIt = s_glyphBoxCache.insert(iconPath, glyphBox(renderer));
    const QRectF glyph = glyphIt.value();
    renderer.setViewBox(glyph);

    const qreal side = (qMin(drawRect.width(), drawRect.height()) - 2) * Metrics::TitleBar_GlyphFraction
        * (isClose ? 0.9 : 1.0); // X close icon looks bigger so it gets sized down a bit
    const qreal glyphScale = side * scale / qMax(glyph.width(), glyph.height());

    // the icon only sits in the exact middle when its pixel size is odd or even like the box around it
    const auto matchParity = [](qreal size, int box) {
        const int pixels = qMax(1, qRound(size));
        return (box - pixels) % 2 == 0 ? pixels : pixels + 1;
    };
    const QSize glyphPixels(matchParity(glyph.width() * glyphScale, hoverPixels.width()), matchParity(glyph.height() * glyphScale, hoverPixels.height()));

    QImage iconImage(hoverPixels, QImage::Format_ARGB32_Premultiplied);
    iconImage.fill(Qt::transparent);
    QPainter iconPainter(&iconImage);
    iconPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    renderer.render(&iconPainter,
                    QRectF(QPointF((hoverPixels.width() - glyphPixels.width()) / 2, (hoverPixels.height() - glyphPixels.height()) / 2), glyphPixels));
    iconPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    iconPainter.fillRect(iconImage.rect(), iconColor);
    iconPainter.end();
    iconImage.setDevicePixelRatio(scale);

    QColor fillColor;
    QColor borderColor;
    if (isClose) {
        fillColor = QColor(isPressed() ? BLOSSOMUI_RED_700 : BLOSSOMUI_RED_800);
        borderColor = QColor(BLOSSOMUI_RED_700);
    } else {
        if (isPressed()) {
            fillColor = QColor(lightTitleBar ? BLOSSOMUI_WINDOW_BUTTON_LIGHT_PRESSED : BLOSSOMUI_WINDOW_BUTTON_DARK_PRESSED);
        } else {
            fillColor = QColor(lightTitleBar ? BLOSSOMUI_WINDOW_BUTTON_LIGHT_HOVER : BLOSSOMUI_WINDOW_BUTTON_DARK_HOVER);
            borderColor = QColor(lightTitleBar ? BLOSSOMUI_WINDOW_BUTTON_LIGHT_HOVER_BORDER : BLOSSOMUI_WINDOW_BUTTON_DARK_HOVER_BORDER);
        }
    }

    if (hoverProgress > 0) {
        painter->save();
        painter->setPen(Qt::NoPen);
        fillColor.setAlphaF(fillColor.alphaF() * hoverProgress);
        painter->setBrush(fillColor);
        painter->drawRoundedRect(hoverRect, 3, 3);

        if (borderColor.isValid()) {
            // the border sits inside the box so it covers whole pixels at any scale
            const qreal penWidth = qMax(KDecoration3::pixelSize(scale), KDecoration3::snapToPixelGrid(1.0, scale));
            borderColor.setAlphaF(borderColor.alphaF() * hoverProgress);
            QPen borderPen(borderColor);
            borderPen.setWidthF(penWidth);
            painter->setPen(borderPen);
            painter->setBrush(Qt::NoBrush);
            painter->drawRoundedRect(hoverRect.adjusted(penWidth / 2, penWidth / 2, -penWidth / 2, -penWidth / 2), 3, 3);
        }

        painter->restore();
    }

    painter->drawImage(hoverRect.topLeft(), iconImage);
}

} // namespace BlossomUI
