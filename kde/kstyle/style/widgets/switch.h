#pragma once

#include <QWidget>

class QCheckBox;

namespace BlossomUI {
class Helper;
}

/** Pill+thumb toggle overlay that sits on top of a QCheckBox indicator. Syncs
 * state with the checkbox and supports dragging. */
class BlossomUISwitchWidget : public QWidget {
  Q_OBJECT
public:
  explicit BlossomUISwitchWidget(BlossomUI::Helper *helper, QCheckBox *parent);

  bool isChecked() const { return _checked; }
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

  // visual thumb position (0 = off, 1 = on), animated with overshoot on
  // toggles and following the pointer during drags
  void animateTo(qreal target);
  qreal _visualPos = 0.0;
  class QVariantAnimation *_anim = nullptr;
};
