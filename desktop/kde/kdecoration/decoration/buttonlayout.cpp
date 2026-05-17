// SPDX-License-Identifier: GPL-2.0-or-later
#include "blossomui.h"
#include "blossomuidecoration.h"
#include "button/blossomuibutton.h"

#include <KDecoration3/DecorationButtonGroup>
#include <QTimer>

namespace BlossomUI {

void Decoration::createButtons() {
  m_leftButtons = new KDecoration3::DecorationButtonGroup(
      KDecoration3::DecorationButtonGroup::Position::Left, this,
      &Button::create);
  m_rightButtons = new KDecoration3::DecorationButtonGroup(
      KDecoration3::DecorationButtonGroup::Position::Right, this,
      &Button::create);
  updateButtonsGeometry();
}

void Decoration::updateButtonsGeometryDelayed() {
  QTimer::singleShot(0, this, &Decoration::updateButtonsGeometry);
}

void Decoration::updateButtonsGeometry() {
  const auto s = settings();
  const qreal buttonSpacing =
      s->smallSpacing() * Metrics::TitleBar_ButtonSpacing;

  const auto leftButtonList = m_leftButtons->buttons();
  const auto rightButtonList = m_rightButtons->buttons();
  const auto buttonList = leftButtonList + rightButtonList;

  for (auto *b : buttonList) {
    auto btn = static_cast<Button *>(b);

    const int verticalOffset =
        (isTopEdge() ? s->smallSpacing() * Metrics::TitleBar_TopMargin : 0);

    const QSizeF preferredSize = btn->preferredSize();
    const int bHeight = preferredSize.height() + verticalOffset;
    const int bWidth = preferredSize.width();

    const bool isFirstInGroup =
        (leftButtonList.contains(btn) && btn == leftButtonList.first()) ||
        (rightButtonList.contains(btn) && btn == rightButtonList.first());
    const bool isLastInGroup =
        (leftButtonList.contains(btn) && btn == leftButtonList.last()) ||
        (rightButtonList.contains(btn) && btn == rightButtonList.last());

    const qreal leftExpand = isFirstInGroup ? 0 : buttonSpacing;
    const qreal rightExpand = isLastInGroup ? 0 : buttonSpacing;

    btn->setGeometry(QRectF(
        QPointF(0, 0), QSizeF(bWidth + leftExpand + rightExpand, bHeight)));
    btn->setPadding(QMargins(leftExpand, verticalOffset, rightExpand, 0));
    btn->setOffset(QPointF(leftExpand, verticalOffset));
    btn->setIconSize(QSizeF(bWidth, bHeight));
  }

  if (!m_leftButtons->buttons().isEmpty()) {
    m_leftButtons->setSpacing(0);

    const int vPadding =
        isTopEdge() ? 0 : s->smallSpacing() * Metrics::TitleBar_TopMargin;
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

    const int vPadding =
        isTopEdge() ? 0 : s->smallSpacing() * Metrics::TitleBar_TopMargin;
    const int hPadding = s->smallSpacing() * Metrics::TitleBar_SideMargin;
    if (isRightEdge()) {
      auto button = static_cast<Button *>(m_rightButtons->buttons().back());

      QRectF geometry = button->geometry();
      geometry.adjust(0, 0, hPadding, 0);
      button->setGeometry(geometry);
      button->setFlag(Button::FlagFirstInList);
      button->setRightPadding(hPadding);
      button->setIconSize(button->preferredSize());

      m_rightButtons->setPos(QPointF(
          size().width() - m_rightButtons->geometry().width(), vPadding));
    } else {
      m_rightButtons->setPos(QPointF(size().width() -
                                         m_rightButtons->geometry().width() -
                                         hPadding - borderRight(),
                                     vPadding));
    }
  }

  update();
}

} // namespace BlossomUI
