#include "blossomuimnemonics.h"
#include "blossomuipropertynames.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "private.h"

#include <QAbstractScrollArea>
#include <QAccessible>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QStyleOptionComboBox>
#include <QStyleOptionFrame>
#include <QStyleOptionGroupBox>
#include <QStyleOptionHeader>
#include <QStyleOptionProgressBar>
#include <QStyleOptionSlider>
#include <QStyleOptionSpinBox>
#include <QStyleOptionTab>
#include <QStyleOptionTabWidgetFrame>
#include <QStyleOptionToolButton>
#include <QTabBar>
#include <QTabWidget>
#include <QToolBox>

namespace BlossomUI {

QRect Style::subElementRect(SubElement element, const QStyleOption *option,
                            const QWidget *widget) const {
  switch (element) {
  case SE_PushButtonContents:
    return pushButtonContentsRect(option, widget);
  case SE_CheckBoxContents:
    return checkBoxContentsRect(option, widget);
  case SE_RadioButtonContents:
    return checkBoxContentsRect(option, widget);
  case SE_LineEditContents:
    return lineEditContentsRect(option, widget);
  case SE_ProgressBarGroove:
    return progressBarGrooveRect(option, widget);
  case SE_ProgressBarContents:
    return progressBarContentsRect(option, widget);
  case SE_ProgressBarLabel:
    return progressBarLabelRect(option, widget);
  case SE_FrameContents:
    return frameContentsRect(option, widget);
  case SE_HeaderArrow:
    return headerArrowRect(option, widget);
  case SE_HeaderLabel:
    return headerLabelRect(option, widget);
  case SE_TabBarTabLeftButton:
    return tabBarTabLeftButtonRect(option, widget);
  case SE_TabBarTabRightButton:
    return tabBarTabRightButtonRect(option, widget);
  case SE_TabWidgetTabBar:
    return tabWidgetTabBarRect(option, widget);
  case SE_TabWidgetTabContents:
    return tabWidgetTabContentsRect(option, widget);
  case SE_TabWidgetTabPane:
    return tabWidgetTabPaneRect(option, widget);
  case SE_TabWidgetLeftCorner:
    return tabWidgetCornerRect(SE_TabWidgetLeftCorner, option, widget);
  case SE_TabWidgetRightCorner:
    return tabWidgetCornerRect(SE_TabWidgetRightCorner, option, widget);
  case SE_ToolBoxTabContents:
    return toolBoxTabContentsRect(option, widget);

  case SE_ItemViewItemCheckIndicator:
  case SE_ItemViewItemDecoration: {
    QRect baseRect = ParentStyleClass::subElementRect(element, option, widget);
    const auto viewOption =
        qstyleoption_cast<const QStyleOptionViewItem *>(option);
    const QMargins margins = _helper->itemViewItemMargins(viewOption);

    int marginAdjust = 0;
    const auto frame =
        viewOption ? qobject_cast<const QFrame *>(viewOption->widget) : nullptr;
    if (frame && frame->frameShape() == QFrame::StyledPanel) {
      marginAdjust = 1;
    }

    if (viewOption &&
        (viewOption->decorationPosition == QStyleOptionViewItem::Left ||
         viewOption->decorationPosition == QStyleOptionViewItem::Right)) {
      if ((option->direction == Qt::RightToLeft) !=
          (viewOption->decorationPosition == QStyleOptionViewItem::Right)) {
        const auto adjustment = baseRect.right() - margins.right() -
                                Metrics::ItemView_ItemPaddingWidth +
                                marginAdjust;
        if (viewOption->rect.width() > adjustment) {
          baseRect.moveLeft(adjustment);
        }
      } else {
        const auto adjustment = baseRect.left() + margins.left() +
                                Metrics::ItemView_ItemPaddingWidth -
                                marginAdjust;
        if (viewOption->rect.width() > adjustment) {
          baseRect.moveLeft(adjustment);
        }
      }
    }

    baseRect.moveTop(baseRect.top() + margins.top() - margins.bottom());
    return baseRect;
  }

  case SE_ItemViewItemText: {
    auto viewItem = qstyleoption_cast<const QStyleOptionViewItem *>(option);
    QRect rect = ParentStyleClass::subElementRect(element, option, widget);
    if (viewItem) {
      const QMargins margins = _helper->itemViewItemMargins(viewItem);
      rect.setRight(rect.right() - margins.right() -
                    Metrics::ItemView_ItemPaddingWidth);
      rect.setLeft(rect.left() + margins.left() +
                   Metrics::ItemView_ItemPaddingWidth +
                   Metrics::ItemView_IconTextSpacing);
      rect.moveTop(rect.top() + margins.top() - margins.bottom());
    }
    return rect;
  }

  // fallback
  default:
    return ParentStyleClass::subElementRect(element, option, widget);
  }
}

QRect Style::subControlRect(ComplexControl element,
                            const QStyleOptionComplex *option,
                            SubControl subControl,
                            const QWidget *widget) const {
  switch (element) {
  case CC_GroupBox:
    return groupBoxSubControlRect(option, subControl, widget);
  case CC_ToolButton:
    return toolButtonSubControlRect(option, subControl, widget);
  case CC_ComboBox:
    return comboBoxSubControlRect(option, subControl, widget);
  case CC_SpinBox:
    return spinBoxSubControlRect(option, subControl, widget);
  case CC_ScrollBar:
    return scrollBarSubControlRect(option, subControl, widget);
  case CC_Dial:
    return dialSubControlRect(option, subControl, widget);
  case CC_Slider:
    return sliderSubControlRect(option, subControl, widget);

  // fallback
  default:
    return ParentStyleClass::subControlRect(element, option, subControl,
                                            widget);
  }
}

QRect Style::pushButtonContentsRect(const QStyleOption *option,
                                    const QWidget *) const {
  return insideMargin(option->rect, Metrics::Frame_FrameWidth);
}

bool Style::isSwitchWidget(const QWidget *widget) const {
  if (!widget)
    return false;
#if QT_CONFIG(accessibility)
  if (QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(
          const_cast<QWidget *>(widget))) {
    return static_cast<int>(iface->role()) == 0x00000087;
  }
#endif
  return false;
}

bool Style::isSwitchCheckBox(const QStyleOption *option,
                             const QWidget *widget) const {
  if (widget && isSwitchWidget(widget))
    return true;
  if (const QStyleOptionButton *buttonOption =
          qstyleoption_cast<const QStyleOptionButton *>(option))
    return buttonOption->text.contains(
        QLatin1String("Switch"),
        Qt::CaseInsensitive); // fallback for Slint Qt backend
  return false;
}

QRect Style::checkBoxContentsRect(const QStyleOption *option,
                                  const QWidget *widget) const {
  const int indicatorWidth = isSwitchCheckBox(option, widget)
                                 ? Metrics::Switch_Width
                                 : Metrics::CheckBox_Size;
  return visualRect(
      option, option->rect.adjusted(
                  indicatorWidth + Metrics::CheckBox_ItemSpacing, 0, 0, 0));
}

QRect Style::lineEditContentsRect(const QStyleOption *option,
                                  const QWidget *widget) const {
  // cast option and check
  const auto frameOption(qstyleoption_cast<const QStyleOptionFrame *>(option));
  if (!frameOption)
    return option->rect;

  // check flatness
  const bool flat(frameOption->lineWidth == 0);
  if (flat)
    return option->rect;

  // copy rect and take out margins
  auto rect(option->rect);

  // take out margins if there is enough room; add horizontal padding for inputs
  const int frameWidth(pixelMetric(PM_DefaultFrameWidth, option, widget));
  if (rect.height() >= option->fontMetrics.height() + 2 * frameWidth)
    return insideMargin(rect, frameWidth + Metrics::LineEdit_HPadding,
                        frameWidth);
  else
    return rect;
}

QRect Style::progressBarGrooveRect(const QStyleOption *option,
                                   const QWidget *widget) const {
  // cast option and check
  const auto progressBarOption(
      qstyleoption_cast<const QStyleOptionProgressBar *>(option));
  if (!progressBarOption)
    return option->rect;

  // get flags and orientation
  const bool textVisible(progressBarOption->textVisible);
  const bool busy(progressBarOption->minimum == 0 &&
                  progressBarOption->maximum == 0);
  const bool horizontal(
      BlossomUIPrivate::isProgressBarHorizontal(progressBarOption));

  // copy rectangle and adjust
  auto rect(option->rect);
  const int frameWidth(pixelMetric(PM_DefaultFrameWidth, option, widget));
  if (horizontal)
    rect = insideMargin(rect, frameWidth, 0);
  else
    rect = insideMargin(rect, 0, frameWidth);

  if (textVisible && !busy && horizontal) {
    auto textRect(subElementRect(SE_ProgressBarLabel, option, widget));
    textRect = visualRect(option, textRect);
    rect.setRight(textRect.left() - Metrics::ProgressBar_ItemSpacing - 1);
    rect = visualRect(option, rect);
    rect = centerRect(rect, rect.width(), Metrics::ProgressBar_Thickness);

  } else if (horizontal) {
    rect = centerRect(rect, rect.width(), Metrics::ProgressBar_Thickness);

  } else {
    rect = centerRect(rect, Metrics::ProgressBar_Thickness, rect.height());
  }

  return rect;
}

QRect Style::progressBarContentsRect(const QStyleOption *option,
                                     const QWidget *widget) const {
  // cast option and check
  const auto progressBarOption(
      qstyleoption_cast<const QStyleOptionProgressBar *>(option));
  if (!progressBarOption)
    return QRect();

  // get groove rect
  const auto rect(progressBarGrooveRect(option, widget));

  // in busy mode, grooveRect is used
  const bool busy(progressBarOption->minimum == 0 &&
                  progressBarOption->maximum == 0);
  if (busy)
    return rect;

  // get orientation
  const bool horizontal(
      BlossomUIPrivate::isProgressBarHorizontal(progressBarOption));

  // check inverted appearance
  const bool inverted(progressBarOption->invertedAppearance);

  // get progress and steps
  const qreal progress(progressBarOption->progress -
                       progressBarOption->minimum);
  const int steps(
      qMax(progressBarOption->maximum - progressBarOption->minimum, 1));

  // Calculate width fraction
  const qreal widthFrac = qMin(qreal(1), progress / steps);

  // convert the pixel width
  const int indicatorSize(widthFrac *
                          (horizontal ? rect.width() : rect.height()));

  QRect indicatorRect;
  if (horizontal) {
    indicatorRect =
        QRect(inverted ? (rect.right() - indicatorSize + 1) : rect.left(),
              rect.y(), indicatorSize, rect.height());
    indicatorRect = visualRect(option->direction, rect, indicatorRect);

  } else
    indicatorRect = QRect(
        rect.x(), inverted ? rect.top() : (rect.bottom() - indicatorSize + 1),
        rect.width(), indicatorSize);

  return indicatorRect;
}

QRect Style::frameContentsRect(const QStyleOption *option,
                               const QWidget *widget) const {
  if (!StyleConfigData::sidePanelDrawFrame() &&
      qobject_cast<const QAbstractScrollArea *>(widget) &&
      widget->property(PropertyNames::sidePanelView).toBool()) {
    // adjust margins for sidepanel widgets
    return option->rect.adjusted(4, 4, -4, 4);

  } else {
    // base class implementation
    return ParentStyleClass::subElementRect(SE_FrameContents, option, widget);
  }
}

QRect Style::progressBarLabelRect(const QStyleOption *option,
                                  const QWidget *) const {
  // cast option and check
  const auto progressBarOption(
      qstyleoption_cast<const QStyleOptionProgressBar *>(option));
  if (!progressBarOption)
    return QRect();

  // get flags and check
  const bool textVisible(progressBarOption->textVisible);
  const bool busy(progressBarOption->minimum == 0 &&
                  progressBarOption->maximum == 0);
  if (!textVisible || busy)
    return QRect();

  // get direction and check
  const bool horizontal(
      BlossomUIPrivate::isProgressBarHorizontal(progressBarOption));
  if (!horizontal)
    return QRect();

  int textWidth = qMax(
      option->fontMetrics.size(_mnemonics->textFlags(), progressBarOption->text)
          .width(),
      option->fontMetrics.size(_mnemonics->textFlags(), QStringLiteral("100%"))
          .width());

  auto rect(insideMargin(option->rect, Metrics::Frame_FrameWidth, 0));
  rect.setLeft(rect.right() - textWidth + 1);
  rect = visualRect(option, rect);

  return rect;
}

QRect Style::headerArrowRect(const QStyleOption *option,
                             const QWidget *) const {
  // cast option and check
  const auto headerOption(
      qstyleoption_cast<const QStyleOptionHeader *>(option));
  if (!headerOption)
    return option->rect;

  // check if arrow is necessary
  if (headerOption->sortIndicator == QStyleOptionHeader::None)
    return QRect();

  auto arrowRect(insideMargin(option->rect, Metrics::Header_MarginWidth));
  arrowRect.setLeft(arrowRect.right() - Metrics::Header_ArrowSize + 1);

  return visualRect(option, arrowRect);
}

QRect Style::headerLabelRect(const QStyleOption *option,
                             const QWidget *) const {
  // cast option and check
  const auto headerOption(
      qstyleoption_cast<const QStyleOptionHeader *>(option));
  if (!headerOption)
    return option->rect;

  // check if arrow is necessary
  auto labelRect(insideMargin(option->rect, Metrics::Header_MarginWidth, 0));
  if (headerOption->sortIndicator == QStyleOptionHeader::None)
    return labelRect;

  labelRect.adjust(0, 0,
                   -Metrics::Header_ArrowSize - Metrics::Header_ItemSpacing, 0);
  return visualRect(option, labelRect);
}

QRect Style::tabBarTabLeftButtonRect(const QStyleOption *option,
                                     const QWidget *) const {
  // cast option and check
  const auto tabOption(qstyleoption_cast<const QStyleOptionTab *>(option));
  if (!tabOption || tabOption->leftButtonSize.isEmpty())
    return QRect();

  const auto rect(option->rect);
  const QSize size(tabOption->leftButtonSize);
  QRect buttonRect(QPoint(0, 0), size);

  // vertical positioning
  switch (tabOption->shape) {
  case QTabBar::RoundedNorth:
  case QTabBar::TriangularNorth:

  case QTabBar::RoundedSouth:
  case QTabBar::TriangularSouth:
    buttonRect.moveLeft(rect.left() + Metrics::TabBar_TabMarginWidth);
    buttonRect.moveTop((rect.height() - buttonRect.height()) / 2.0);
    buttonRect = visualRect(option, buttonRect);
    break;

  case QTabBar::RoundedWest:
  case QTabBar::TriangularWest:
    buttonRect.moveBottom(rect.bottom() - Metrics::TabBar_TabMarginWidth);
    buttonRect.moveLeft((rect.width() - buttonRect.width()) / 2.0);
    break;

  case QTabBar::RoundedEast:
  case QTabBar::TriangularEast:
    buttonRect.moveTop(rect.top() + Metrics::TabBar_TabMarginWidth);
    buttonRect.moveLeft((rect.width() - buttonRect.width()) / 2.0);
    break;

  default:
    break;
  }

  return buttonRect;
}

QRect Style::tabBarTabRightButtonRect(const QStyleOption *option,
                                      const QWidget *) const {
  // cast option and check
  const auto tabOption(qstyleoption_cast<const QStyleOptionTab *>(option));
  if (!tabOption || tabOption->rightButtonSize.isEmpty())
    return QRect();

  const auto rect(option->rect);
  const auto size(tabOption->rightButtonSize);
  QRect buttonRect(QPoint(0, 0), size);

  // vertical positioning
  switch (tabOption->shape) {
  case QTabBar::RoundedNorth:
  case QTabBar::TriangularNorth:

  case QTabBar::RoundedSouth:
  case QTabBar::TriangularSouth:
    buttonRect.moveRight(rect.right() - Metrics::TabBar_TabMarginWidth);
    buttonRect.moveTop((rect.height() - buttonRect.height()) / 2.0);
    buttonRect = visualRect(option, buttonRect);
    break;

  case QTabBar::RoundedWest:
  case QTabBar::TriangularWest:
    buttonRect.moveTop(rect.top() + Metrics::TabBar_TabMarginWidth);
    buttonRect.moveLeft((rect.width() - buttonRect.width()) / 2.0);
    break;

  case QTabBar::RoundedEast:
  case QTabBar::TriangularEast:
    buttonRect.moveBottom(rect.bottom() - Metrics::TabBar_TabMarginWidth);
    buttonRect.moveLeft((rect.width() - buttonRect.width()) / 2.0);
    break;

  default:
    break;
  }

  return buttonRect;
}

QRect Style::tabWidgetTabBarRect(const QStyleOption *option,
                                 const QWidget *widget) const {
  // cast option and check
  const auto tabOption =
      qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option);
  if (!tabOption)
    return ParentStyleClass::subElementRect(SE_TabWidgetTabBar, option, widget);

  // do nothing if tabbar is hidden
  const QSize tabBarSize(tabOption->tabBarSize);

  auto rect(option->rect);
  QRect tabBarRect(QPoint(0, 0), tabBarSize);

  Qt::Alignment tabBarAlignment(styleHint(SH_TabBar_Alignment, option, widget));

  const bool documentMode(tabOption->lineWidth == 0);

  // horizontal positioning
  const bool verticalTabs(isVerticalTab(tabOption->shape));
  if (verticalTabs) {
    tabBarRect.setHeight(qMin(tabBarRect.height(), rect.height() - 2));
    if (tabBarAlignment == Qt::AlignCenter)
      tabBarRect.moveTop(rect.top() +
                         (rect.height() - tabBarRect.height()) / 2.0);
    else
      tabBarRect.moveTop(rect.top() + 1);

  } else {
    // account for corner rects and tab widget frame shadow
    // need to re-run visualRect to remove right-to-left handling, since it is
    // re-added on tabBarRect at the end
    const auto leftButtonRect(visualRect(
        option, subElementRect(SE_TabWidgetLeftCorner, option, widget)));
    const auto rightButtonRect(visualRect(
        option, subElementRect(SE_TabWidgetRightCorner, option, widget)));

    rect.setLeft(leftButtonRect.width() +
                 (documentMode ? 0 : Metrics::Frame_FrameWidth));
    rect.setRight(rightButtonRect.left() +
                  (documentMode ? 0 : Metrics::Frame_FrameWidth));
    const int sizeCorrection =
        -1; // HACK: for some reason, the rect size is 1px larger than expected,
            // so it needs to be reduced
    if (StyleConfigData::tabBarTabExpandFullWidth() &&
        /*StyleConfigData::tabBarOpacity() == 100 ||*/ StyleConfigData::
            documentModeTabs()) {
      tabBarRect.setWidth(rect.width() - 2 * Metrics::Frame_FrameWidth -
                          sizeCorrection); // adwaita qt style tab
    } else {
      tabBarRect.setWidth(
          qMin(tabBarRect.width(), rect.width() - 2)); // fixed width tabs
    }
    if (tabBarAlignment == Qt::AlignCenter) {
      tabBarRect.moveLeft(rect.left() +
                          (rect.width() - tabBarRect.width()) / 2.0);
    } else if (tabOption->lineWidth == 0) {
      tabBarRect.moveLeft(rect.left());
    } else {
      tabBarRect.moveLeft(rect.left() - 1);
    }

    tabBarRect = visualRect(option, tabBarRect);
  }

  // vertical positioning
  switch (tabOption->shape) {
  case QTabBar::RoundedNorth:
  case QTabBar::TriangularNorth:
    if (!documentMode)
      tabBarRect.moveTop(rect.top() + Metrics::Frame_FrameWidth - 1);
    break;

  case QTabBar::RoundedSouth:
  case QTabBar::TriangularSouth:
    tabBarRect.moveBottom(rect.bottom() - 1);
    break;

  case QTabBar::RoundedWest:
  case QTabBar::TriangularWest:
    tabBarRect.moveLeft(rect.left() + 1);
    break;

  case QTabBar::RoundedEast:
  case QTabBar::TriangularEast:
    tabBarRect.moveRight(rect.right() - 1);
    break;

  default:
    break;
  }

  return tabBarRect;
}

QRect Style::tabWidgetTabContentsRect(const QStyleOption *option,
                                      const QWidget *widget) const {
  // cast option and check
  const auto tabOption =
      qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option);
  if (!tabOption)
    return option->rect;

  // do nothing if tabbar is hidden
  if (tabOption->tabBarSize.isEmpty())
    return option->rect;
  const auto rect = tabWidgetTabPaneRect(option, widget);

  // include margin and shadow size
  const bool documentMode(tabOption->lineWidth == 0);
  if (documentMode || !StyleConfigData::documentModeTabs()) {
    // add margin only to the relevant side
    switch (tabOption->shape) {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
      return rect.adjusted(
          0, Metrics::TabWidget_MarginWidth /*+ Metrics::Frame_FrameWidth*/, 0,
          0);

    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
      return rect.adjusted(
          0, 0, 0,
          -Metrics::TabWidget_MarginWidth /*- Metrics::Frame_FrameWidth*/);

    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
      return rect.adjusted(
          Metrics::TabWidget_MarginWidth /*+ Metrics::Frame_FrameWidth*/, 0, 0,
          0);

    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
      return rect.adjusted(
          0, 0, -Metrics::TabWidget_MarginWidth /*- Metrics::Frame_FrameWidth*/,
          0);

    default:
      return rect;
    }

    // include tabbar and margins
  } else {
    QRect r = insideMargin(
        rect, Metrics::TabWidget_MarginWidth /*+ Metrics::Frame_FrameWidth*/);

    // add margin only to the relevant side
    switch (tabOption->shape) {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
      return r.adjusted(0, tabOption->tabBarSize.height() + 2 * 2, 0, 0);

    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
      return r.adjusted(0, 0, 0, -tabOption->tabBarSize.height());

    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
      return r.adjusted(tabOption->tabBarSize.width(), 0, 0, 0);

    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
      return r.adjusted(0, 0, -tabOption->tabBarSize.width(), 0);

    default:
      return r;
    }
  }
}

QRect Style::tabWidgetTabPaneRect(const QStyleOption *option,
                                  const QWidget *widget) const {
  Q_UNUSED(widget)
  const auto tabOption =
      qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option);

  // return here if tab is a qml widget or is not in document mode
  // we will not subtract the tab size from the tab pane for an unified look
  // with immutable tabs
  if (!tabOption || tabOption->tabBarSize.isEmpty() ||
      (StyleConfigData::documentModeTabs() && !(tabOption->lineWidth == 0)))
    return option->rect;

  const int overlap = Metrics::TabBar_BaseOverlap - 1;
  const QSize tabBarSize(tabOption->tabBarSize - QSize(overlap, overlap));

  // adjust
  auto rect(option->rect);
  switch (tabOption->shape) {
  case QTabBar::RoundedNorth:
  case QTabBar::TriangularNorth:
    if (!(tabOption->lineWidth == 0))
      rect.adjust(0, tabBarSize.height() + 4, 0, 0);
    else
      rect.adjust(0, tabBarSize.height(), 0, 0);
    break;

  case QTabBar::RoundedSouth:
  case QTabBar::TriangularSouth:
    rect.adjust(0, 0, 0, -tabBarSize.height());
    break;

  case QTabBar::RoundedWest:
  case QTabBar::TriangularWest:
    rect.adjust(tabBarSize.width(), 0, 0, 0);
    break;

  case QTabBar::RoundedEast:
  case QTabBar::TriangularEast:
    rect.adjust(0, 0, -tabBarSize.width(), 0);
    break;

  default:
    return QRect();
  }

  return rect;
}

QRect Style::tabWidgetCornerRect(SubElement element, const QStyleOption *option,
                                 const QWidget *) const {
  // cast option and check
  const auto tabOption =
      qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option);
  if (!tabOption)
    return option->rect;

  // do nothing if tabbar is hidden
  const QSize tabBarSize(tabOption->tabBarSize);
  if (tabBarSize.isEmpty())
    return QRect();

  // do nothing for vertical tabs
  const bool verticalTabs(isVerticalTab(tabOption->shape));
  if (verticalTabs)
    return QRect();

  const auto rect(option->rect);
  QRect cornerRect;
  switch (element) {
  case SE_TabWidgetLeftCorner:
    cornerRect = QRect(QPoint(0, 0), tabOption->leftCornerWidgetSize);
    cornerRect.moveLeft(rect.left());
    break;

  case SE_TabWidgetRightCorner:
    cornerRect = QRect(QPoint(0, 0), tabOption->rightCornerWidgetSize);
    cornerRect.moveRight(rect.right());
    break;

  default:
    break;
  }

  // expend height to tabBarSize, if needed, to make sure base is properly
  // rendered
  cornerRect.setHeight(qMax(cornerRect.height(), tabBarSize.height() + 1));

  switch (tabOption->shape) {
  case QTabBar::RoundedNorth:
  case QTabBar::TriangularNorth:
    cornerRect.moveTop(rect.top());
    break;

  case QTabBar::RoundedSouth:
  case QTabBar::TriangularSouth:
    cornerRect.moveBottom(rect.bottom());
    break;

  default:
    break;
  }

  // return cornerRect;
  cornerRect = visualRect(option, cornerRect);
  return cornerRect;
}

QRect Style::toolBoxTabContentsRect(const QStyleOption *option,
                                    const QWidget *widget) const {
  // cast option and check
  const auto toolBoxOption(
      qstyleoption_cast<const QStyleOptionToolBox *>(option));
  if (!toolBoxOption)
    return option->rect;

  // copy rect
  const auto &rect(option->rect);

  int contentsWidth(0);
  if (!toolBoxOption->icon.isNull()) {
    const int iconSize(pixelMetric(QStyle::PM_SmallIconSize, option, widget));
    contentsWidth += iconSize;

    if (!toolBoxOption->text.isEmpty())
      contentsWidth += Metrics::ToolBox_TabItemSpacing;
  }

  if (!toolBoxOption->text.isEmpty()) {
    const int textWidth =
        toolBoxOption->fontMetrics
            .size(_mnemonics->textFlags(), toolBoxOption->text)
            .width();
    contentsWidth += textWidth;
  }

  contentsWidth += 2 * Metrics::ToolBox_TabMarginWidth;
  contentsWidth = qMin(contentsWidth, rect.width());
  contentsWidth = qMax(contentsWidth, int(Metrics::ToolBox_TabMinWidth));
  return centerRect(rect, contentsWidth, rect.height());
}

QRect Style::genericLayoutItemRect(const QStyleOption *option,
                                   const QWidget *) const {
  return insideMargin(option->rect, -Metrics::Frame_FrameWidth);
}

QRect Style::groupBoxSubControlRect(const QStyleOptionComplex *option,
                                    SubControl subControl,
                                    const QWidget *widget) const {
  QRect rect = option->rect;
  switch (subControl) {
  case SC_GroupBoxFrame:
    return rect;

  case SC_GroupBoxContents: {
    // cast option and check
    const auto groupBoxOption =
        qstyleoption_cast<const QStyleOptionGroupBox *>(option);
    if (!groupBoxOption)
      break;

    // take out frame width
    rect = insideMargin(rect, Metrics::Frame_FrameWidth);

    // get state
    const bool checkable(groupBoxOption->subControls &
                         QStyle::SC_GroupBoxCheckBox);
    const bool emptyText(groupBoxOption->text.isEmpty());

    // calculate title height
    int titleHeight(0);
    if (!emptyText)
      titleHeight = groupBoxOption->fontMetrics.height();
    if (checkable)
      titleHeight = qMax(titleHeight, int(Metrics::CheckBox_Size));

    // add margin
    if (titleHeight > 0)
      titleHeight += 2 * Metrics::GroupBox_TitleMarginWidth;

    rect.adjust(0, titleHeight, 0, 0);
    return rect;
  }

  case SC_GroupBoxCheckBox:
  case SC_GroupBoxLabel: {
    // cast option and check
    const auto groupBoxOption =
        qstyleoption_cast<const QStyleOptionGroupBox *>(option);
    if (!groupBoxOption)
      break;

    // take out frame width
    rect = insideMargin(rect, Metrics::Frame_FrameWidth);

    const bool emptyText(groupBoxOption->text.isEmpty());
    const bool checkable(groupBoxOption->subControls &
                         QStyle::SC_GroupBoxCheckBox);

    // calculate title height
    int titleHeight(0);
    int titleWidth(0);
    if (!emptyText) {
      const QFontMetrics fontMetrics = option->fontMetrics;
      titleHeight = qMax(titleHeight, fontMetrics.height());
      titleWidth +=
          fontMetrics.size(_mnemonics->textFlags(), groupBoxOption->text)
              .width();
    }

    if (checkable) {
      titleHeight = qMax(titleHeight, int(Metrics::CheckBox_Size));
      titleWidth += Metrics::CheckBox_Size;
      if (!emptyText)
        titleWidth += Metrics::CheckBox_ItemSpacing;
    }

    // adjust height
    auto titleRect(rect);
    titleRect.setHeight(titleHeight);
    titleRect.translate(0, Metrics::GroupBox_TitleMarginWidth);

    // center
    titleRect = centerRect(titleRect, titleWidth, titleHeight);

    if (subControl == SC_GroupBoxCheckBox) {
      // vertical centering
      titleRect = centerRect(titleRect, titleWidth, Metrics::CheckBox_Size);

      // horizontal positioning
      const QRect subRect(titleRect.topLeft(),
                          QSize(Metrics::CheckBox_Size, titleRect.height()));
      return visualRect(option->direction, titleRect, subRect);

    } else {
      // vertical centering
      QFontMetrics fontMetrics = option->fontMetrics;
      titleRect = centerRect(titleRect, titleWidth, fontMetrics.height());

      // horizontal positioning
      auto subRect(titleRect);
      if (checkable)
        subRect.adjust(Metrics::CheckBox_Size + Metrics::CheckBox_ItemSpacing,
                       0, 0, 0);
      return visualRect(option->direction, titleRect, subRect);
    }
  }

  default:
    break;
  }

  return ParentStyleClass::subControlRect(CC_GroupBox, option, subControl,
                                          widget);
}

QRect Style::toolButtonSubControlRect(const QStyleOptionComplex *option,
                                      SubControl subControl,
                                      const QWidget *widget) const {
  // cast option and check
  const auto toolButtonOption =
      qstyleoption_cast<const QStyleOptionToolButton *>(option);
  if (!toolButtonOption)
    return ParentStyleClass::subControlRect(CC_ToolButton, option, subControl,
                                            widget);

  const bool hasPopupMenu(toolButtonOption->features &
                          QStyleOptionToolButton::MenuButtonPopup);
  const bool hasInlineIndicator(
      toolButtonOption->features & QStyleOptionToolButton::HasMenu &&
      toolButtonOption->features & QStyleOptionToolButton::PopupDelay &&
      !hasPopupMenu);

  // store rect
  const auto &rect(option->rect);
  const int menuButtonWidth(Metrics::MenuButton_IndicatorWidth);
  switch (subControl) {
  case SC_ToolButtonMenu: {
    // check features
    if (!(hasPopupMenu || hasInlineIndicator))
      return QRect();

    // check features
    auto menuRect(rect);
    menuRect.setLeft(rect.right() - menuButtonWidth + 1);
    if (hasInlineIndicator) {
      menuRect.setTop(menuRect.bottom() - menuButtonWidth + 1);
    }

    return visualRect(option, menuRect);
  }

  case SC_ToolButton: {
    if (hasPopupMenu) {
      auto contentsRect(rect);
      contentsRect.setRight(rect.right() - menuButtonWidth);
      return visualRect(option, contentsRect);

    } else
      return rect;
  }

  default:
    return QRect();
  }
}

QRect Style::comboBoxSubControlRect(const QStyleOptionComplex *option,
                                    SubControl subControl,
                                    const QWidget *widget) const {
  // cast option and check
  const auto comboBoxOption(
      qstyleoption_cast<const QStyleOptionComboBox *>(option));
  if (!comboBoxOption)
    return ParentStyleClass::subControlRect(CC_ComboBox, option, subControl,
                                            widget);

  const bool editable(comboBoxOption->editable);
  const bool flat(editable && !comboBoxOption->frame);

  // copy rect
  auto rect(option->rect);

  switch (subControl) {
  case SC_ComboBoxFrame:
    return flat ? rect : QRect();
  case SC_ComboBoxListBoxPopup:
    return rect;

  case SC_ComboBoxArrow: {
    // take out frame width
    if (!flat)
      rect = insideMargin(rect, Metrics::Frame_FrameWidth);

    QRect arrowRect(rect.right() - Metrics::MenuButton_IndicatorWidth + 1,
                    rect.top(), Metrics::MenuButton_IndicatorWidth,
                    rect.height());

    arrowRect = centerRect(arrowRect, Metrics::MenuButton_IndicatorWidth,
                           Metrics::MenuButton_IndicatorWidth);
    return visualRect(option, arrowRect);
  }

  case SC_ComboBoxEditField: {
    QRect labelRect;
    const int frameWidth(pixelMetric(PM_ComboBoxFrameWidth, option, widget));
    labelRect =
        QRect(rect.left(), rect.top(),
              rect.width() - Metrics::MenuButton_IndicatorWidth, rect.height());

    // remove margins
    if (!flat &&
        rect.height() >= option->fontMetrics.height() + 2 * frameWidth) {
      labelRect.adjust(frameWidth, frameWidth, 0, -frameWidth);
    }

    return visualRect(option, labelRect);
  }

  default:
    break;
  }

  return ParentStyleClass::subControlRect(CC_ComboBox, option, subControl,
                                          widget);
}

QRect Style::spinBoxSubControlRect(const QStyleOptionComplex *option,
                                   SubControl subControl,
                                   const QWidget *widget) const {
  // cast option and check
  const auto spinBoxOption(
      qstyleoption_cast<const QStyleOptionSpinBox *>(option));
  if (!spinBoxOption)
    return ParentStyleClass::subControlRect(CC_SpinBox, option, subControl,
                                            widget);
  const bool flat(!spinBoxOption->frame);

  // copy rect
  auto rect(option->rect);

  switch (subControl) {
  case SC_SpinBoxFrame:
    return flat ? QRect() : rect;

  case SC_SpinBoxUp:
  case SC_SpinBoxDown: {
    // take out frame width
    if (!flat && rect.height() >= 2 * Metrics::Frame_FrameWidth +
                                      Metrics::SpinBox_ArrowButtonWidth)
      rect = insideMargin(rect, Metrics::Frame_FrameWidth);

    QRect arrowRect;
    arrowRect =
        QRect(rect.right() - Metrics::SpinBox_ArrowButtonWidth + 1, rect.top(),
              Metrics::SpinBox_ArrowButtonWidth, rect.height());

    const int arrowHeight(
        qMin(rect.height(), int(Metrics::SpinBox_ArrowButtonWidth)));
    arrowRect =
        centerRect(arrowRect, Metrics::SpinBox_ArrowButtonWidth, arrowHeight);
    arrowRect.setHeight(arrowHeight / 2);
    if (subControl == SC_SpinBoxDown)
      arrowRect.translate(0, arrowHeight / 2);

    return visualRect(option, arrowRect);
  }

  case SC_SpinBoxEditField: {
    QRect labelRect;
    labelRect =
        QRect(rect.left(), rect.top(),
              rect.width() - Metrics::SpinBox_ArrowButtonWidth, rect.height());

    // remove right side line editor margins
    const int frameWidth(pixelMetric(PM_SpinBoxFrameWidth, option, widget));
    if (!flat &&
        labelRect.height() >= option->fontMetrics.height() + 2 * frameWidth) {
      labelRect.adjust(frameWidth, frameWidth, 0, -frameWidth);
    }

    return visualRect(option, labelRect);
  }

  default:
    break;
  }

  return ParentStyleClass::subControlRect(CC_SpinBox, option, subControl,
                                          widget);
}

QRect Style::scrollBarInternalSubControlRect(const QStyleOptionComplex *option,
                                             SubControl subControl) const {
  const auto &rect = option->rect;
  const State &state(option->state);
  const bool horizontal(state & State_Horizontal);

  switch (subControl) {
  case SC_ScrollBarSubLine: {
    int majorSize(scrollBarButtonHeight(_subLineButtons));
    if (horizontal)
      return visualRect(
          option, QRect(rect.left(), rect.top(), majorSize, rect.height()));
    else
      return visualRect(
          option, QRect(rect.left(), rect.top(), rect.width(), majorSize));
  }

  case SC_ScrollBarAddLine: {
    int majorSize(scrollBarButtonHeight(_addLineButtons));
    if (horizontal)
      return visualRect(option, QRect(rect.right() - majorSize + 1, rect.top(),
                                      majorSize, rect.height()));
    else
      return visualRect(option,
                        QRect(rect.left(), rect.bottom() - majorSize + 1,
                              rect.width(), majorSize));
  }

  default:
    return QRect();
  }
}

QRect Style::scrollBarSubControlRect(const QStyleOptionComplex *option,
                                     SubControl subControl,
                                     const QWidget *widget) const {
  // cast option and check
  const auto sliderOption(
      qstyleoption_cast<const QStyleOptionSlider *>(option));
  if (!sliderOption)
    return ParentStyleClass::subControlRect(CC_ScrollBar, option, subControl,
                                            widget);

  // get relevant state
  const State &state(option->state);
  const bool horizontal(state & State_Horizontal);

  switch (subControl) {
  case SC_ScrollBarSubLine:
  case SC_ScrollBarAddLine:
    return scrollBarInternalSubControlRect(option, subControl);

  case SC_ScrollBarGroove: {
    auto topRect = visualRect(
        option, scrollBarInternalSubControlRect(option, SC_ScrollBarSubLine));
    auto bottomRect = visualRect(
        option, scrollBarInternalSubControlRect(option, SC_ScrollBarAddLine));

    QPoint topLeftCorner;
    QPoint botRightCorner;

    if (horizontal) {
      topLeftCorner = QPoint(topRect.right() + 1, topRect.top());
      botRightCorner = QPoint(bottomRect.left() - 1, topRect.bottom());

    } else {
      topLeftCorner = QPoint(topRect.left(), topRect.bottom() + 1);
      botRightCorner = QPoint(topRect.right(), bottomRect.top() - 1);
    }

    // define rect
    return visualRect(option, QRect(topLeftCorner, botRightCorner));
  }

  case SC_ScrollBarSlider: {
    // handle RTL here to unreflect things if need be
    auto groove =
        visualRect(option, subControlRect(CC_ScrollBar, option,
                                          SC_ScrollBarGroove, widget));

    if (sliderOption->minimum == sliderOption->maximum)
      return groove;

    // Figure out how much room there is
    int space(horizontal ? groove.width() : groove.height());

    // Calculate the portion of this space that the slider should occupy
    int sliderSize = space * qreal(sliderOption->pageStep) /
                     (sliderOption->maximum - sliderOption->minimum +
                      sliderOption->pageStep);
    sliderSize =
        qMax(sliderSize, static_cast<int>(Metrics::ScrollBar_MinSliderHeight));
    sliderSize = qMin(sliderSize, space);

    space -= sliderSize;
    if (space <= 0)
      return groove;

    int pos =
        qRound(qreal(sliderOption->sliderPosition - sliderOption->minimum) /
               (sliderOption->maximum - sliderOption->minimum) * space);
    if (sliderOption->upsideDown)
      pos = space - pos;
    if (horizontal)
      return visualRect(option, QRect(groove.left() + pos, groove.top(),
                                      sliderSize, groove.height()));
    else
      return visualRect(option, QRect(groove.left(), groove.top() + pos,
                                      groove.width(), sliderSize));
  }

  case SC_ScrollBarSubPage: {
    // handle RTL here to unreflect things if need be
    auto slider =
        visualRect(option, subControlRect(CC_ScrollBar, option,
                                          SC_ScrollBarSlider, widget));
    auto groove =
        visualRect(option, subControlRect(CC_ScrollBar, option,
                                          SC_ScrollBarGroove, widget));

    if (horizontal)
      return visualRect(option,
                        QRect(groove.left(), groove.top(),
                              slider.left() - groove.left(), groove.height()));
    else
      return visualRect(option,
                        QRect(groove.left(), groove.top(), groove.width(),
                              slider.top() - groove.top()));
  }

  case SC_ScrollBarAddPage: {
    // handle RTL here to unreflect things if need be
    auto slider =
        visualRect(option, subControlRect(CC_ScrollBar, option,
                                          SC_ScrollBarSlider, widget));
    auto groove =
        visualRect(option, subControlRect(CC_ScrollBar, option,
                                          SC_ScrollBarGroove, widget));

    if (horizontal)
      return visualRect(option, QRect(slider.right() + 1, groove.top(),
                                      groove.right() - slider.right(),
                                      groove.height()));
    else
      return visualRect(option, QRect(groove.left(), slider.bottom() + 1,
                                      groove.width(),
                                      groove.bottom() - slider.bottom()));
  }

  default:
    return ParentStyleClass::subControlRect(CC_ScrollBar, option, subControl,
                                            widget);
    ;
  }
}

QRect Style::dialSubControlRect(const QStyleOptionComplex *option,
                                SubControl subControl,
                                const QWidget *widget) const {
  // cast option and check
  const auto sliderOption(
      qstyleoption_cast<const QStyleOptionSlider *>(option));
  if (!sliderOption)
    return ParentStyleClass::subControlRect(CC_Dial, option, subControl,
                                            widget);

  // adjust rect to be square, and centered
  auto rect(option->rect);
  const int dimension(qMin(rect.width(), rect.height()));
  rect = centerRect(rect, dimension, dimension);

  switch (subControl) {
  case QStyle::SC_DialGroove:
    return insideMargin(rect,
                        static_cast<int>(Metrics::Slider_ControlThickness -
                                         Metrics::Slider_GrooveThickness) /
                            2.0);
  case QStyle::SC_DialHandle: {
    // calculate angle at which handle needs to be drawn
    const qreal angle(dialAngle(sliderOption, sliderOption->sliderPosition));

    // groove rect
    const QRectF grooveRect(insideMargin(
        rect, static_cast<int>(Metrics::Slider_ControlThickness) / 2.0));
    qreal radius(grooveRect.width() / 2.0);

    // slider center
    QPointF center(grooveRect.center() + QPointF(radius * std::cos(angle),
                                                 -radius * std::sin(angle)));

    // slider rect
    QRect handleRect(0, 0, Metrics::Slider_ControlThickness,
                     Metrics::Slider_ControlThickness);
    handleRect.moveCenter(center.toPoint());
    return handleRect;
  }

  default:
    return ParentStyleClass::subControlRect(CC_Dial, option, subControl,
                                            widget);
    ;
  }
}

int Style::sliderTickMarksLength() {
  const int tickLength =
      Metrics::Slider_TickLength + Metrics::Slider_TickMarginWidth +
      (Metrics::Slider_GrooveThickness - Metrics::Slider_ControlThickness) / 2;
  constexpr int builtInTickLength(5);
  return tickLength - builtInTickLength;
}

QRect Style::sliderRectWithoutTickMarks(const QStyleOptionSlider *option) {
  // store tick position and orientation
  const QSlider::TickPosition tickPosition(option->tickPosition);
  const bool horizontal(option->orientation == Qt::Horizontal);
  const int tick = sliderTickMarksLength();

  auto rect(option->rect);

  if (horizontal) {
    if (tickPosition & QSlider::TicksAbove) {
      rect.setTop(-tick);
    }
    if (tickPosition & QSlider::TicksBelow) {
      rect.setBottom(rect.bottom() + tick);
    }
  } else {
    if (tickPosition & QSlider::TicksAbove) {
      rect.setLeft(-tick);
    }
    if (tickPosition & QSlider::TicksBelow) {
      rect.setRight(rect.right() + tick);
    }
  }

  return rect;
}

QRect Style::sliderSubControlRect(const QStyleOptionComplex *option,
                                  SubControl subControl,
                                  const QWidget *widget) const {
  // cast option and check
  const auto sliderOption(
      qstyleoption_cast<const QStyleOptionSlider *>(option));
  if (!sliderOption)
    return ParentStyleClass::subControlRect(CC_Slider, option, subControl,
                                            widget);

  // direction
  const bool horizontal(sliderOption->orientation == Qt::Horizontal);

  auto rect(sliderRectWithoutTickMarks(sliderOption));

  switch (subControl) {
  case SC_SliderHandle: {
    QRect ret(centerRect(rect, Metrics::Slider_ControlThickness,
                         Metrics::Slider_ControlThickness));

    constexpr int len = Metrics::Slider_ControlThickness + 2 * Metrics::Slider_HoverMargin;
    const int sliderPos = sliderPositionFromValue(
        sliderOption->minimum, sliderOption->maximum,
        sliderOption->sliderPosition,
        (horizontal ? rect.width() : rect.height()) - len,
        sliderOption->upsideDown);
    if (horizontal) {
      ret.moveLeft(rect.x() + Metrics::Slider_HoverMargin + sliderPos);
    } else {
      ret.moveTop(rect.y() + Metrics::Slider_HoverMargin + sliderPos);
    }
    ret = visualRect(option->direction, rect, ret);
    return ret;
  }
  case SC_SliderGroove: {
    auto grooveRect =
        insideMargin(rect, pixelMetric(PM_DefaultFrameWidth, option, widget));

    if (horizontal) {
      grooveRect = centerRect(rect, grooveRect.width(), Metrics::Slider_GrooveThickness);
      grooveRect.adjust(Metrics::Slider_HoverMargin, 0, -Metrics::Slider_HoverMargin, 0);
    } else {
      grooveRect = centerRect(rect, Metrics::Slider_GrooveThickness, grooveRect.height());
      grooveRect.adjust(0, Metrics::Slider_HoverMargin, 0, -Metrics::Slider_HoverMargin);
    }
    return grooveRect;
  }

  default:
    return ParentStyleClass::subControlRect(CC_Slider, option, subControl,
                                            widget);
  }
}

} // namespace BlossomUI
