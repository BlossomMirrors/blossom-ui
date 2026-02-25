/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef BLOSSOMUI_SWITCH_WIDGET_H
#define BLOSSOMUI_SWITCH_WIDGET_H

#include <QWidget>

class QCheckBox;

namespace BlossomUI
{
class Helper;
}

/** Widget that draws and behaves as a switch (pill + thumb, draggable). Overlays the indicator area of a QCheckBox and syncs state. */
class BlossomUISwitchWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BlossomUISwitchWidget(BlossomUI::Helper *helper, QCheckBox *parent);

    bool isChecked() const
    {
        return _checked;
    }
    void setChecked(bool on);

Q_SIGNALS:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    bool event(QEvent *e) override;

private:
    void updateFromParent();
    bool hitTrack(const QPoint &pos) const;

    BlossomUI::Helper *_helper = nullptr;
    QCheckBox *_checkBox = nullptr;
    bool _checked = false;
    bool _hover = false;
    bool _pressed = false;
    bool _dragging = false;
    QPoint _pressPos;
    static const int _dragThreshold = 4;
};

#endif
