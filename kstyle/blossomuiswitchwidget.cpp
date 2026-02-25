/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "blossomuiswitchwidget.h"
#include "blossomuihelper.h"

#include <QCheckBox>
#include <QMouseEvent>
#include <QPainter>

using namespace BlossomUI;

BlossomUISwitchWidget::BlossomUISwitchWidget(Helper *helper, QCheckBox *parent)
    : QWidget(parent)
    , _helper(helper)
    , _checkBox(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_NoMousePropagation, false);
    setFocusPolicy(Qt::NoFocus);
    _checked = parent->isChecked();
    setCursor(Qt::PointingHandCursor);
    connect(parent, &QCheckBox::toggled, this, &BlossomUISwitchWidget::updateFromParent);
}

void BlossomUISwitchWidget::setChecked(bool on)
{
    if (_checked != on) {
        _checked = on;
        if (_checkBox && _checkBox->isChecked() != on)
            _checkBox->setChecked(on);
        update();
        emit toggled(on);
    }
}

void BlossomUISwitchWidget::updateFromParent()
{
    if (_checkBox && _checked != _checkBox->isChecked()) {
        _checked = _checkBox->isChecked();
        update();
    }
}

bool BlossomUISwitchWidget::hitTrack(const QPoint &pos) const
{
    return rect().contains(pos);
}

void BlossomUISwitchWidget::paintEvent(QPaintEvent *)
{
    if (!_helper || !_checkBox)
        return;
    QPainter p(this);
    const QPalette &palette = _checkBox->palette();
    const bool sunken = _pressed && hitTrack(mapFromGlobal(QCursor::pos()));
    CheckBoxState state = _checked ? CheckOn : CheckOff;
    _helper->renderSwitch(&p, rect(), palette, sunken, _hover, state, _checked ? 1.0 : 0.0);
}

void BlossomUISwitchWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;
    e->accept();
    _pressed = true;
    _pressPos = e->pos();
    _dragging = false;
    update();
}

void BlossomUISwitchWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (!(e->buttons() & Qt::LeftButton)) {
        _pressed = false;
        _dragging = false;
        update();
        return;
    }
    e->accept();
    if (!_dragging) {
        if ((e->pos() - _pressPos).manhattanLength() >= _dragThreshold)
            _dragging = true;
        else {
            update();
            return;
        }
    }
    const int cx = width() / 2;
    const bool on = e->pos().x() >= cx;
    if (_checkBox->layoutDirection() == Qt::RightToLeft) {
        if (_checked != !on)
            setChecked(!on);
    } else {
        if (_checked != on)
            setChecked(on);
    }
    update();
}

void BlossomUISwitchWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;
    e->accept();
    if (!_dragging && hitTrack(e->pos()))
        setChecked(!_checked);
    _pressed = false;
    _dragging = false;
    update();
}

bool BlossomUISwitchWidget::event(QEvent *e)
{
    if (e->type() == QEvent::Enter) {
        _hover = true;
        update();
        return true;
    }
    if (e->type() == QEvent::Leave) {
        _hover = false;
        _pressed = false;
        _dragging = false;
        update();
        return true;
    }
    return QWidget::event(e);
}
