/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
 * Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
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
#include "blossomuibutton.h"

#include <KColorUtils>
#include <KDecoration3/DecoratedWindow>
#include <KIconLoader>

#include <QDir>
#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QStandardPaths>
#include <QVariantAnimation>
#include <QtSvg/QSvgRenderer>
#include <kdecoration3/decorationdefines.h>

namespace BlossomUI
{

using KDecoration3::ColorGroup;
using KDecoration3::ColorRole;
using KDecoration3::DecorationButtonType;

Button::Button(DecorationButtonType type, Decoration *decoration, QObject *parent)
    : DecorationButton(type, decoration, parent)
    , m_animation(new QVariantAnimation(this))
{
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        setOpacity(value.toReal());
    });

    connect(decoration->window(), SIGNAL(iconChanged(QIcon)), this, SLOT(update()));
    connect(decoration->settings().get(), &KDecoration3::DecorationSettings::reconfigured, this, &Button::reconfigure);
    connect(this, &KDecoration3::DecorationButton::hoveredChanged, this, &Button::updateAnimationState);

    reconfigure();
}

Button::Button(QObject *parent, const QVariantList &args)
    : Button(args.at(0).value<DecorationButtonType>(), args.at(1).value<Decoration *>(), parent)
{
    setGeometry(QRectF(QPointF(0, 0), preferredSize()));
}

Button *Button::create(DecorationButtonType type, KDecoration3::Decoration *decoration, QObject *parent)
{
    if (auto d = qobject_cast<Decoration *>(decoration)) {
        Button *b = new Button(type, d, parent);
        const auto c = d->window();
        switch (type) {
        case DecorationButtonType::Close:
            b->setVisible(c->isCloseable());
            QObject::connect(c, &KDecoration3::DecoratedWindow::closeableChanged, b, &BlossomUI::Button::setVisible);
            break;

        case DecorationButtonType::Maximize:
            b->setVisible(c->isMaximizeable());
            QObject::connect(c, &KDecoration3::DecoratedWindow::maximizeableChanged, b, &BlossomUI::Button::setVisible);
            break;

        case DecorationButtonType::Minimize:
            b->setVisible(c->isMinimizeable());
            QObject::connect(c, &KDecoration3::DecoratedWindow::minimizeableChanged, b, &BlossomUI::Button::setVisible);
            break;

        case DecorationButtonType::ContextHelp:
            b->setVisible(c->providesContextHelp());
            QObject::connect(c, &KDecoration3::DecoratedWindow::providesContextHelpChanged, b, &BlossomUI::Button::setVisible);
            break;

        case DecorationButtonType::Shade:
            b->setVisible(c->isShadeable());
            QObject::connect(c, &KDecoration3::DecoratedWindow::shadeableChanged, b, &BlossomUI::Button::setVisible);
            break;

        case DecorationButtonType::Menu:
            QObject::connect(c, &KDecoration3::DecoratedWindow::iconChanged, b, [b]() {
                b->update();
            });
            break;

        default:
            break;
        }

        return b;
    }

    return nullptr;
}

void Button::paint(QPainter *painter, const QRectF &repaintRegion)
{
    Q_UNUSED(repaintRegion)

    if (!decoration())
        return;

    switch (type()) {
    case KDecoration3::DecorationButtonType::Menu: {
        const QRectF iconRect = geometry().marginsRemoved(m_padding);
        const auto c = decoration()->window();
        if (auto deco = qobject_cast<Decoration *>(decoration())) {
            const QPalette activePalette = KIconLoader::global()->customPalette();
            QPalette palette = c->palette();
            palette.setColor(QPalette::WindowText, deco->fontColor());
            KIconLoader::global()->setCustomPalette(palette);
            c->icon().paint(painter, iconRect.toRect());
            if (activePalette == QPalette()) {
                KIconLoader::global()->resetPalette();
            } else {
                KIconLoader::global()->setCustomPalette(palette);
            }
        } else {
            c->icon().paint(painter, iconRect.toRect());
        }
        break;
    }
    case KDecoration3::DecorationButtonType::Spacer:
        break;
    default:
        painter->save();
        drawIconWithMask(painter);
        painter->restore();
        break;
    }
}

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
        case DecorationButtonType::Close: {
            painter->drawLine(QPointF(5, 5), QPointF(13, 13));
            painter->drawLine(13, 5, 5, 13);
            break;
        }

        case DecorationButtonType::Maximize: {
            if (isChecked()) {
                pen.setJoinStyle(Qt::RoundJoin);
                painter->setPen(pen);

                painter->drawPolygon(QVector<QPointF>{QPointF(4, 9), QPointF(9, 4), QPointF(14, 9), QPointF(9, 14)});

            } else {
                painter->drawPolyline(QVector<QPointF>{QPointF(4, 11), QPointF(9, 6), QPointF(14, 11)});
            }
            break;
        }

        case DecorationButtonType::Minimize: {
            painter->drawPolyline(QVector<QPointF>{QPointF(4, 7), QPointF(9, 12), QPointF(14, 7)});
            break;
        }

        case DecorationButtonType::OnAllDesktops: {
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
        }

        case DecorationButtonType::Shade: {
            if (isChecked()) {
                painter->drawLine(QPointF(4, 5.5), QPointF(14, 5.5));
                painter->drawPolyline(QVector<QPointF>{QPointF(4, 8), QPointF(9, 13), QPointF(14, 8)});

            } else {
                painter->drawLine(QPointF(4, 5.5), QPointF(14, 5.5));
                painter->drawPolyline(QVector<QPointF>{QPointF(4, 13), QPointF(9, 8), QPointF(14, 13)});
            }

            break;
        }

        case DecorationButtonType::KeepBelow: {
            painter->drawPolyline(QVector<QPointF>{QPointF(4, 5), QPointF(9, 10), QPointF(14, 5)});

            painter->drawPolyline(QVector<QPointF>{QPointF(4, 9), QPointF(9, 14), QPointF(14, 9)});
            break;
        }

        case DecorationButtonType::KeepAbove: {
            painter->drawPolyline(QVector<QPointF>{QPointF(4, 9), QPointF(9, 4), QPointF(14, 9)});

            painter->drawPolyline(QVector<QPointF>{QPointF(4, 13), QPointF(9, 8), QPointF(14, 13)});
            break;
        }

        case DecorationButtonType::ApplicationMenu: {
            painter->drawRect(QRectF(3.5, 4.5, 11, 1));
            painter->drawRect(QRectF(3.5, 8.5, 11, 1));
            painter->drawRect(QRectF(3.5, 12.5, 11, 1));
            break;
        }

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

    const int buttonMargin = 2;
    const QRectF drawRect = rect.adjusted(buttonMargin, buttonMargin, -buttonMargin, -buttonMargin);

    QString iconFileName;
    switch (type()) {
    case DecorationButtonType::Close:
        iconFileName = QStringLiteral("close.svg");
        break;
    case DecorationButtonType::Maximize:
        if (isChecked()) {
            iconFileName = QStringLiteral("normalize.svg");
        } else {
            iconFileName = QStringLiteral("maximize.svg");
        }
        break;
    case DecorationButtonType::Minimize:
        iconFileName = QStringLiteral("minimize.svg");
        break;
    default:
        drawIcon(painter);
        return;
    }

    QStringList searchPaths = {QStringLiteral("/usr/share/blossomui/icons/"),
                               QStringLiteral("/usr/local/share/blossomui/icons/"),
                               QDir::homePath() + QStringLiteral("/.local/share/blossomui/icons/")};

    QString iconPath;
    for (const QString &basePath : searchPaths) {
        QString testPath = basePath + iconFileName;
        if (QFile::exists(testPath)) {
            iconPath = testPath;
            break;
        }
    }

    if (iconPath.isEmpty()) {
        drawIcon(painter);
        return;
    }

    QFile svgFile(iconPath);
    if (!svgFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        drawIcon(painter);
        return;
    }
    const bool isDarkMode = d->window()->palette().color(QPalette::Window).lightness() < 128;
    const QColor hoverIconColor = (type() == DecorationButtonType::Close && isDarkMode)
        ? d->fontColor()
        : d->titleBarColor();

    const QColor iconColor = KColorUtils::mix(d->fontColor(), hoverIconColor, m_opacity);
    QString svgContent = QString::fromUtf8(svgFile.readAll());
    svgFile.close();

    svgContent.replace(QStringLiteral("color:#232629"), QStringLiteral("color:") + iconColor.name());

    QSvgRenderer renderer;
    renderer.load(svgContent.toUtf8());
    if (!renderer.isValid()) {
        drawIcon(painter);
        return;
    }

    qreal iconScale = 0.85;
    int iconWidth = qRound(drawRect.width() * iconScale);
    int iconHeight = qRound(drawRect.height() * iconScale);
    QSize iconSize(iconWidth, iconHeight);

    QImage iconImage(iconSize, QImage::Format_ARGB32_Premultiplied);
    iconImage.fill(Qt::transparent);

    QPainter iconPainter(&iconImage);
    iconPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    renderer.render(&iconPainter, QRectF(0, 0, iconWidth, iconHeight));
    iconPainter.end();

    QColor bgColor;
    if (type() == DecorationButtonType::Close) {
        bgColor = d->window()->color(ColorGroup::Warning, ColorRole::Foreground);
    } else {
        bgColor = d->fontColor();
    }
    qreal bgOpacity = m_opacity * 0.6;

    QRect pixelRect = drawRect.toAlignedRect();

    if (bgOpacity > 0.0 && bgColor.isValid()) {
        painter->save();
        painter->setPen(Qt::NoPen);

        QColor fillColor = bgColor;
        fillColor.setAlphaF(bgOpacity);
        painter->setBrush(fillColor);
        painter->drawRoundedRect(pixelRect, 3, 3);

        QColor outlineColor = bgColor;
        outlineColor.setAlphaF(bgOpacity / 0.6);
        QPen outlinePen(outlineColor);
        outlinePen.setWidthF(1.0);
        painter->setPen(outlinePen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(pixelRect.adjusted(0, 0, -1, -1), 3, 3);

        painter->restore();
    }

    QPoint iconPos(pixelRect.x() + (pixelRect.width() - iconWidth) / 2, pixelRect.y() + (pixelRect.height() - iconHeight) / 2);
    painter->drawImage(iconPos, iconImage);
}

QColor Button::foregroundColor() const
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d) {
        return QColor();

    } else if (isPressed()) {
        if (type() == DecorationButtonType::Close) {
            return d->titleBarColor();
        }
        return d->titleBarColor();

    } else if (type() == DecorationButtonType::Close && d->internalSettings()->outlineCloseButton()) {
        return d->titleBarColor();

    } else if ((type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade)
               && isChecked()) {
        return d->titleBarColor();

    } else if (m_animation->state() == QAbstractAnimation::Running) {
        return KColorUtils::mix(d->fontColor(), d->titleBarColor(), m_opacity);

    } else if (isHovered()) {
        return d->titleBarColor();

    } else {
        return d->fontColor();
    }
}

QColor Button::backgroundColor() const
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d) {
        return QColor();
    }

    auto c = d->window();
    if (isPressed()) {
        if (type() == DecorationButtonType::Close)
            return c->color(ColorGroup::Warning, ColorRole::Foreground);
        else
            return KColorUtils::mix(d->titleBarColor(), d->fontColor(), 0.3);

    } else if ((type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade)
               && isChecked()) {
        return d->fontColor();

    } else if (m_animation->state() == QAbstractAnimation::Running) {
        if (type() == DecorationButtonType::Close) {
            if (d->internalSettings()->outlineCloseButton()) {
                return KColorUtils::mix(d->fontColor(), c->color(ColorGroup::Warning, ColorRole::Foreground).lighter(), m_opacity);

            } else {
                QColor color(c->color(ColorGroup::Warning, ColorRole::Foreground).lighter());
                color.setAlpha(color.alpha() * m_opacity);
                return color;
            }

        } else {
            QColor color(d->fontColor());
            color.setAlpha(color.alpha() * m_opacity);
            return color;
        }

    } else if (isHovered()) {
        if (type() == DecorationButtonType::Close)
            return c->color(ColorGroup::Warning, ColorRole::Foreground).lighter();
        else
            return d->fontColor();

    } else if (type() == DecorationButtonType::Close && d->internalSettings()->outlineCloseButton()) {
        return d->fontColor();

    } else {
        return QColor();
    }
}

void Button::reconfigure()
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d) {
        return;
    }

    switch (type()) {
    case KDecoration3::DecorationButtonType::Spacer:
        setPreferredSize(QSizeF(d->buttonSize() * 0.5, d->buttonSize()));
        break;
    default:
        setPreferredSize(QSizeF(d->buttonSize(), d->buttonSize()));
        break;
    }

    m_animation->setDuration(d->internalSettings()->animationsDuration());
}

void Button::updateAnimationState(bool hovered)
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!(d && d->internalSettings()->animationsEnabled()))
        return;

    m_animation->setDirection(hovered ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    if (m_animation->state() != QAbstractAnimation::Running)
        m_animation->start();
}

} // namespace
