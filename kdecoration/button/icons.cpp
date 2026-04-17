// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomuibutton.h"
#include "blossomuidecoration.h"
#include "blossomui.h"

#include <KColorUtils>
#include <KDecoration3/DecoratedWindow>
#include <KIconLoader>

#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QtSvg/QSvgRenderer>
#include <zlib.h>

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

    QFile svgFile(iconPath);
    if (!svgFile.open(QIODevice::ReadOnly)) {
        drawIcon(painter);
        return;
    }
    QByteArray rawData = svgFile.readAll();
    svgFile.close();

    QByteArray svgData;
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

    const bool isDarkMode = d->window()->palette().color(QPalette::Window).lightness() < 128;
    const QColor hoverIconColor = (type() == DecorationButtonType::Close && isDarkMode)
        ? d->fontColor()
        : d->titleBarColor();

    const QColor iconColor = KColorUtils::mix(d->fontColor(), hoverIconColor, m_opacity);
    QString svgContent = QString::fromUtf8(svgData);
    svgContent.replace(QStringLiteral("color:#232629"), QStringLiteral("color:") + iconColor.name());
    svgContent.replace(QStringLiteral("color: #232629"), QStringLiteral("color: ") + iconColor.name());

    QSvgRenderer renderer;
    renderer.load(svgContent.toUtf8());
    if (!renderer.isValid()) {
        drawIcon(painter);
        return;
    }

    const int buttonMargin = 1;
    const QRectF drawRect = rect.adjusted(buttonMargin, buttonMargin, -buttonMargin, -buttonMargin);

    qreal iconScale = 1.0;
    int iconWidth = qRound(drawRect.width() * iconScale);
    int iconHeight = qRound(drawRect.height() * iconScale);

    QImage iconImage(QSize(iconWidth, iconHeight), QImage::Format_ARGB32_Premultiplied);
    iconImage.fill(Qt::transparent);
    QPainter iconPainter(&iconImage);
    iconPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    renderer.render(&iconPainter, QRectF(0, 0, iconWidth, iconHeight));
    iconPainter.end();

    QColor bgColor;
    if (type() == DecorationButtonType::Close)
        bgColor = d->window()->color(ColorGroup::Warning, ColorRole::Foreground);
    else
        bgColor = d->fontColor();
    qreal bgOpacity = m_opacity * .6;

    if (bgOpacity > 0 && bgColor.isValid()) {
        painter->save();
        painter->setPen(Qt::NoPen);

        QColor fillColor = bgColor;
        fillColor.setAlphaF(bgOpacity);
        painter->setBrush(fillColor);
        painter->drawRoundedRect(drawRect, 3, 3);

        QColor outlineColor = bgColor;
        outlineColor.setAlphaF(bgOpacity / .6);
        QPen outlinePen(outlineColor);
        outlinePen.setWidthF(1);
        painter->setPen(outlinePen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(drawRect, 3, 3);

        painter->restore();
    }

    const QPoint iconPos(
        qRound(drawRect.x() + (drawRect.width() - iconWidth) / 2.0),
        qRound(drawRect.y() + (drawRect.height() - iconHeight) / 2.0)
    );
    painter->drawImage(iconPos, iconImage);
}

} // namespace BlossomUI
