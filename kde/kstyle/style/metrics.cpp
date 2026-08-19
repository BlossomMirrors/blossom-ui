#include "blossomuianimations.h"
#include "blossomuipropertynames.h"
#include "blossomuistyle.h"
#include "blossomuistyleconfigdata.h"
#include "button.h"
#include "checkbox.h"
#include "comboboxcontrol.h"
#include "frame.h"
#include "header.h"
#include "menucontrol.h"
#include "misccontrol.h"
#include "private.h"
#include "progressbar.h"
#include "slider.h"
#include "spinbox.h"
#include "switch.h"
#include "tabbar.h"
#include "titlebarcontrol.h"
#include "toolbarcontrol.h"
#include "tooltipcontrol.h"

#include <QAbstractScrollArea>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGraphicsView>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QPushButton>
#include <QScrollBar>
#include <QSlider>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStyleOptionButton>
#include <QStyleOptionComboBox>
#include <QStyleOptionHeader>
#include <QStyleOptionMenuItem>
#include <QStyleOptionProgressBar>
#include <QStyleOptionSlider>
#include <QStyleOptionSpinBox>
#include <QStyleOptionTab>
#include <QStyleOptionTabWidgetFrame>
#include <QStyleOptionToolButton>
#include <QTabBar>
#include <QTableView>
#include <QTextEdit>
#include <QToolBar>
#include <QToolBox>
#include <QToolButton>
#include <QTreeView>

namespace BlossomUI {

// generic top-level layout fallbacks, in logical pixels - not tied to any
// specific widget, only used by the pixelMetric()/sizeFromContents()
// dispatchers below
static constexpr int Layout_TopLevelMarginWidth = 10;
static constexpr int Layout_ChildMarginWidth = 6;
static constexpr int Layout_DefaultSpacing = 12;

int Style::pixelMetric(PixelMetric metric, const QStyleOption *option,
                       const QWidget *widget) const {
  // handle special cases
  switch (metric) {
  // frame width
  case PM_DefaultFrameWidth: {
    const auto isControl = isQtQuickControl(option, widget);

    if (!widget && !isControl) {
      return 0;
    }

    if (qobject_cast<const QMenu *>(widget))
      return StyleConfigData::cornerRadius() > 1 ? 4 : 0;
    if (qobject_cast<const QLineEdit *>(widget))
      return Render::LineEdit_FrameWidth;
    else if (isControl) {
      const QString &elementType =
          option->styleObject->property("elementType").toString();
      if (elementType == QLatin1String("edit") ||
          elementType == QLatin1String("spinbox")) {
        return Render::LineEdit_FrameWidth;

      } else if (elementType == QLatin1String("combobox")) {
        return Render::ComboBox_FrameWidth;
      }

      return Render::Frame_FrameWidth;
    }

    const auto forceFrame = widget->property(PropertyNames::forceFrame);

    if (forceFrame.isValid() && !forceFrame.toBool()) {
      return 0;
    }
    if ((forceFrame.isValid() && forceFrame.toBool()) ||
        widget->property(PropertyNames::bordersSides).isValid()) {
      return Render::Frame_FrameWidth;
    }

    if (widget && widget->inherits("KTextEditor::View") &&
        !StyleConfigData::kTextEditDrawFrame() && !_app.isKdevelop)
      return 0;

    // fallback
    return Render::Frame_FrameWidth;
  }

  case PM_ComboBoxFrameWidth: {
    const auto comboBoxOption(
        qstyleoption_cast<const QStyleOptionComboBox *>(option));
    return comboBoxOption && comboBoxOption->editable
               ? Render::LineEdit_FrameWidth
               : Render::ComboBox_FrameWidth;
  }

  case PM_SpinBoxFrameWidth:
    return Render::SpinBox_FrameWidth;
  case PM_ToolBarFrameWidth:
    return Render::ToolBar_FrameWidth;
  case PM_ToolTipLabelFrameWidth:
    return Render::ToolTip_FrameWidth;

  // layout
  case PM_LayoutLeftMargin:
  case PM_LayoutTopMargin:
  case PM_LayoutRightMargin:
  case PM_LayoutBottomMargin: {
    /*
     * use either Child margin or TopLevel margin,
     * depending on  widget type
     */
    if ((option && (option->state & QStyle::State_Window)) ||
        (widget && widget->isWindow())) {
      return Layout_TopLevelMarginWidth;

    } else if (widget && widget->inherits("KPageView")) {
      return 0;

    } else {
      return Layout_ChildMarginWidth;
    }
  }

  case PM_LayoutHorizontalSpacing:
    return Layout_DefaultSpacing;
  case PM_LayoutVerticalSpacing:
    return Layout_DefaultSpacing;

  // buttons
  case PM_ButtonMargin: {
    // needs special case for kcalc buttons, to prevent the application to set
    // too small margins
    if (widget && widget->inherits("KCalcButton"))
      return Render::Button_MarginWidth + 4;
    else
      return Render::Button_MarginWidth;
  }

  case PM_ButtonDefaultIndicator:
    return 0;
  case PM_ButtonShiftHorizontal:
    return 0;
  case PM_ButtonShiftVertical:
    return 0;

  // menubars
  case PM_MenuBarPanelWidth:
    return 0;
  case PM_MenuBarHMargin:
    return 0;
  case PM_MenuBarVMargin:
    return 0;
  case PM_MenuBarItemSpacing:
    return 0;
  case PM_MenuDesktopFrameWidth:
    return 0;

  // menu buttons
  case PM_MenuButtonIndicator:
    return Render::MenuButton_IndicatorWidth;

  // toolbars
  case PM_ToolBarHandleExtent:
    return Render::ToolBar_HandleExtent;
  case PM_ToolBarSeparatorExtent:
    return Render::ToolBar_SeparatorWidth;
  case PM_ToolBarExtensionExtent:
    return pixelMetric(PM_SmallIconSize, option, widget) +
           2 * Render::ToolButton_MarginWidth;

  case PM_ToolBarItemMargin:
    return 2;
  case PM_ToolBarItemSpacing:
    return Render::ToolBar_ItemSpacing;

  // tabbars
  case PM_TabBarIconSize:
    return 20;
  case PM_TabBarTabShiftVertical:
    return 0;
  case PM_TabBarTabShiftHorizontal:
    return 0;
  case PM_TabBarTabOverlap:
    return Render::TabBar_TabOverlap;
  // case PM_TabBarTabOverlap: return StyleConigData::cornerRadius() + 2;
  case PM_TabBarBaseOverlap:
    return Render::TabBar_BaseOverlap;
  case PM_TabBarTabHSpace:
    return 2 * Render::TabBar_TabMarginWidth;
  case PM_TabBarTabVSpace:
    return 2 * Render::TabBar_TabMarginHeight;
  case PM_TabCloseIndicatorWidth:
  case PM_TabCloseIndicatorHeight:
    return pixelMetric(PM_SmallIconSize, option, widget);
  case PM_TabBarScrollButtonWidth:
    return 28;

  // scrollbars
  case PM_ScrollBarExtent:
    return Render::ScrollBar_Extend;
  case PM_ScrollBarSliderMin:
    return Render::ScrollBar_MinSliderHeight;

  // title bar
  case PM_TitleBarHeight:
    return 2 * Render::TitleBar_MarginWidth +
           pixelMetric(PM_SmallIconSize, option, widget);

  // sliders
  case PM_SliderThickness:
    return Render::Slider_ControlThickness + 2 * Render::Slider_HoverMargin;
  case PM_SliderControlThickness:
    return Render::Slider_ControlThickness + 2 * Render::Slider_HoverMargin;
  case PM_SliderLength:
    return Render::Slider_ControlThickness;

  // checkboxes and radio buttons
  case PM_IndicatorWidth:
    return isSwitchCheckBox(option, widget) ? Render::Switch_Width
                                            : Render::CheckBox_Size;
  case PM_IndicatorHeight:
    return isSwitchCheckBox(option, widget) ? Render::Switch_Height
                                            : int(Render::CheckBox_Size);
  case PM_ExclusiveIndicatorWidth:
    return Render::CheckBox_Size;
  case PM_ExclusiveIndicatorHeight:
    return Render::CheckBox_Size;

  // list heaaders
  case PM_HeaderMarkSize:
    return Render::Header_ArrowSize;
  case PM_HeaderMargin:
    return Render::Header_MarginWidth;

  // dock widget
  // return 0 here, since frame is handled directly in polish
  case PM_DockWidgetFrameWidth:
    return 0;
  case PM_DockWidgetTitleMargin:
    return Render::Frame_FrameWidth;
  case PM_DockWidgetTitleBarButtonMargin:
    return Render::ToolButton_MarginWidth;

  case PM_SplitterWidth:
    return Render::Splitter_SplitterWidth;
  case PM_DockWidgetSeparatorExtent:
    return Render::Splitter_SplitterWidth;

  // fallback
  default:
    return ParentStyleClass::pixelMetric(metric, option, widget);
  }
}

int Style::styleHint(StyleHint hint, const QStyleOption *option,
                     const QWidget *widget,
                     QStyleHintReturn *returnData) const {
  setSurfaceFormat(
      widget); /* FIXME Why here and nowhere else?
                  Perhaps because of its use in qapplication.cpp. */
  switch (hint) {
  case SH_RubberBand_Mask: {
    if (auto mask = qstyleoption_cast<QStyleHintReturnMask *>(returnData)) {
      mask->region = option->rect;

      /*
       * need to check on widget before removing inner region
       * in order to still preserve rubberband in MainWindow and QGraphicsView
       * in QMainWindow because it looks better
       * in QGraphicsView because the painting fails completely otherwise
       */
      if (widget &&
          (qobject_cast<const QAbstractItemView *>(widget->parent()) ||
           qobject_cast<const QGraphicsView *>(widget->parent()) ||
           qobject_cast<const QMainWindow *>(widget->parent()))) {
        return true;
      }

      // also check if widget's parent is some itemView viewport
      if (widget && widget->parent() &&
          qobject_cast<const QAbstractItemView *>(widget->parent()->parent()) &&
          static_cast<const QAbstractItemView *>(widget->parent()->parent())
                  ->viewport() == widget->parent()) {
        return true;
      }

      // mask out center
      mask->region -= insideMargin(option->rect, 1);

      return true;
    }
    return false;
  }

  case SH_ComboBox_ListMouseTracking:
    return true;
  case SH_ComboBox_Popup:
    // Use menu-style popup (PE_PanelMenu + CE_MenuItem) so Slint Qt backend and
    // others get styled dropdown
    return true;
  case SH_MenuBar_MouseTracking:
    return true;
  case SH_Menu_MouseTracking:
    return true;
  case SH_Menu_Scrollable: {
    if (StyleConfigData::scrollableMenu()) {
      return true;
    } else {
      return false;
    }
  }

  case SH_Menu_SubMenuPopupDelay:
    return 150;
  case SH_Menu_SloppySubMenus:
    return true;

  // TODO Qt6: drop deprecated SH_Widget_Animate
  case SH_Widget_Animate:
    return true;
  case SH_Menu_SupportsSections:
    return true;
  case SH_Widget_Animation_Duration: {
    const KConfigGroup g(
        KSharedConfig::openConfig(QStringLiteral("kdeglobals")),
        QStringLiteral("KDE"));
    const qreal factor = g.readEntry("AnimationDurationFactor", 1.0);
    return qRound(Animation_BaseDuration * qBound(0.0, factor, 10.0));
  }

  case SH_DialogButtonBox_ButtonsHaveIcons:
    return true;

  case SH_GroupBox_TextLabelVerticalAlignment:
    return Qt::AlignVCenter;
  case SH_TabBar_Alignment:
    return StyleConfigData::tabBarDrawCenteredTabs() ? Qt::AlignCenter
                                                     : Qt::AlignLeft;
  case SH_ToolBox_SelectedPageTitleBold:
    return false;
  case SH_ScrollBar_MiddleClickAbsolutePosition:
    return true;
  case SH_ScrollView_FrameOnlyAroundContents:
    return false;
  case SH_FormLayoutFormAlignment:
    return Qt::AlignLeft | Qt::AlignTop;
  case SH_FormLayoutLabelAlignment:
    return Qt::AlignRight;
  case SH_FormLayoutFieldGrowthPolicy:
    return QFormLayout::ExpandingFieldsGrow;
  case SH_FormLayoutWrapPolicy:
    return QFormLayout::DontWrapRows;
  case SH_MessageBox_TextInteractionFlags:
    return Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse;
  case SH_ProgressDialog_CenterCancelButton:
    return false;
  case SH_MessageBox_CenterButtons:
    return false;

  case SH_RequestSoftwareInputPanel:
    return RSIP_OnMouseClick;
  case SH_TitleBar_NoBorder:
    return true;
  case SH_DockWidget_ButtonsHaveFrame:
    return false;
  default:
    return ParentStyleClass::styleHint(hint, option, widget, returnData);
  }
}

QSize Style::sizeFromContents(ContentsType element, const QStyleOption *option,
                              const QSize &size, const QWidget *widget) const {
  switch (element) {
  case CT_CheckBox:
    return checkBoxSizeFromContents(option, size, widget);
  case CT_RadioButton:
    return checkBoxSizeFromContents(option, size, widget);
  case CT_LineEdit:
    return lineEditSizeFromContents(option, size, widget);
  case CT_ComboBox:
    return comboBoxSizeFromContents(option, size, widget);
  case CT_SpinBox:
    return spinBoxSizeFromContents(option, size, widget);
  case CT_Slider:
    return sliderSizeFromContents(option, size, widget);
  case CT_PushButton:
    return pushButtonSizeFromContents(option, size, widget);
  case CT_ToolButton:
    return toolButtonSizeFromContents(option, size, widget);
  case CT_MenuBar:
    return defaultSizeFromContents(option, size, widget);
  case CT_MenuBarItem:
    return menuBarItemSizeFromContents(option, size, widget);
  case CT_MenuItem:
    return menuItemSizeFromContents(option, size, widget);
  case CT_ProgressBar:
    return progressBarSizeFromContents(option, size, widget);
  case CT_TabWidget:
    return tabWidgetSizeFromContents(option, size, widget);
  case CT_TabBarTab:
    return tabBarTabSizeFromContents(option, size, widget);
  case CT_HeaderSection:
    return headerSectionSizeFromContents(option, size, widget);
  case CT_ItemViewItem:
    return itemViewItemSizeFromContents(option, size, widget);

  // fallback
  default:
    return ParentStyleClass::sizeFromContents(element, option, size, widget);
  }
}

QStyle::SubControl
Style::hitTestComplexControl(ComplexControl control,
                             const QStyleOptionComplex *option,
                             const QPoint &point, const QWidget *widget) const {
  switch (control) {
  case CC_ScrollBar: {
    auto grooveRect =
        subControlRect(CC_ScrollBar, option, SC_ScrollBarGroove, widget);
    if (grooveRect.contains(point)) {
      // Must be either page up/page down, or just click on the slider.
      auto sliderRect =
          subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget);

      if (sliderRect.contains(point))
        return SC_ScrollBarSlider;
      else if (preceeds(point, sliderRect, option))
        return SC_ScrollBarSubPage;
      else
        return SC_ScrollBarAddPage;
    }

    // This is one of the up/down buttons. First, decide which one it is.
    if (preceeds(point, grooveRect, option)) {
      if (_subLineButtons == DoubleButton) {
        auto buttonRect =
            scrollBarInternalSubControlRect(option, SC_ScrollBarSubLine);
        return scrollBarHitTest(buttonRect, point, option);

      } else
        return SC_ScrollBarSubLine;
    }

    if (_addLineButtons == DoubleButton) {
      auto buttonRect =
          scrollBarInternalSubControlRect(option, SC_ScrollBarAddLine);
      return scrollBarHitTest(buttonRect, point, option);

    } else
      return SC_ScrollBarAddLine;
  }

  // fallback
  default:
    return ParentStyleClass::hitTestComplexControl(control, option, point,
                                                   widget);
  }
}

QSize Style::checkBoxSizeFromContents(const QStyleOption *option,
                                      const QSize &contentsSize,
                                      const QWidget *widget) const {
  // get contents size
  QSize size(contentsSize);

  const bool isSwitch = isSwitchCheckBox(option, widget);
  const int indicatorW =
      isSwitch ? Render::Switch_Width : Render::CheckBox_Size;
  const int indicatorH =
      isSwitch ? Render::Switch_Height : int(Render::CheckBox_Size);

  // add focus height
  size = expandSize(size, 0, Render::CheckBox_FocusMarginWidth);

  // make sure there is enough height for indicator
  size.setHeight(qMax(size.height(), indicatorH));

  // Add space for the indicator and the icon
  size.rwidth() += indicatorW + Render::CheckBox_ItemSpacing;

  // also add extra space, to leave room to the right of the label
  size.rwidth() += Render::CheckBox_ItemSpacing;

  return size;
}

QSize Style::lineEditSizeFromContents(const QStyleOption *option,
                                      const QSize &contentsSize,
                                      const QWidget *widget) const {
  // cast option and check
  const auto frameOption(qstyleoption_cast<const QStyleOptionFrame *>(option));
  if (!frameOption)
    return contentsSize;

  const bool flat(frameOption->lineWidth == 0);
  const int frameWidth(pixelMetric(PM_DefaultFrameWidth, option, widget));
  return flat ? contentsSize
              : expandSize(contentsSize,
                           frameWidth + Render::LineEdit_HPadding, frameWidth);
}

QSize Style::comboBoxSizeFromContents(const QStyleOption *option,
                                      const QSize &contentsSize,
                                      const QWidget *widget) const {
  // cast option and check
  const auto comboBoxOption(
      qstyleoption_cast<const QStyleOptionComboBox *>(option));
  if (!comboBoxOption) {
    return contentsSize;
  }

  // copy size
  QSize size(contentsSize);

  // make sure there is enough height for the button
  size.setHeight(qMax(size.height(), int(Render::MenuButton_IndicatorWidth)));

  // add relevant margin
  const int frameWidth(pixelMetric(PM_ComboBoxFrameWidth, option, widget));
  size = expandSize(size, frameWidth);

  // add button width and spacing
  size.rwidth() += Render::MenuButton_IndicatorWidth + 2;
  size.rwidth() += Render::Button_ItemSpacing;

  return size;
}

QSize Style::spinBoxSizeFromContents(const QStyleOption *option,
                                     const QSize &contentsSize,
                                     const QWidget *widget) const {
  // cast option and check
  const auto spinBoxOption(
      qstyleoption_cast<const QStyleOptionSpinBox *>(option));
  if (!spinBoxOption)
    return contentsSize;

  const bool flat(!spinBoxOption->frame);

  // copy size
  QSize size(contentsSize);

  // add editor margins
  const int frameWidth(pixelMetric(PM_SpinBoxFrameWidth, option, widget));
  if (!flat)
    size = expandSize(size, frameWidth);

  // make sure there is enough height for the button
  size.setHeight(qMax(size.height() + Render::Frame_FrameWidth,
                      int(Render::SpinBox_ArrowButtonWidth)));

  // add button width and spacing
  size.rwidth() += Render::SpinBox_ArrowButtonWidth;

  return size;
}

QSize Style::sliderSizeFromContents(const QStyleOption *option,
                                    const QSize &contentsSize,
                                    const QWidget *) const {
  // cast option and check
  const auto sliderOption(
      qstyleoption_cast<const QStyleOptionSlider *>(option));
  if (!sliderOption)
    return contentsSize;

  // store tick position and orientation
  const QSlider::TickPosition tickPosition(sliderOption->tickPosition);
  const bool horizontal(sliderOption->orientation == Qt::Horizontal);

  // do nothing if no ticks are requested
  if (tickPosition == QSlider::NoTicks)
    return contentsSize;

  /*
   * Qt adds its own tick length directly inside QSlider.
   * Take it out and replace by ours, if needed
   */
  const int tickLength(
      Render::Slider_TickLength + Render::Slider_TickMarginWidth +
      (Render::Slider_GrooveThickness - Render::Slider_ControlThickness) / 2);

  const int builtInTickLength(5);

  QSize size(contentsSize);
  if (horizontal) {
    if (tickPosition & QSlider::TicksAbove)
      size.rheight() += tickLength - builtInTickLength;
    if (tickPosition & QSlider::TicksBelow)
      size.rheight() += tickLength - builtInTickLength;

  } else {
    if (tickPosition & QSlider::TicksAbove)
      size.rwidth() += tickLength - builtInTickLength;
    if (tickPosition & QSlider::TicksBelow)
      size.rwidth() += tickLength - builtInTickLength;
  }

  return size;
}

QSize Style::pushButtonSizeFromContents(const QStyleOption *option,
                                        const QSize &contentsSize,
                                        const QWidget *widget) const {
  // cast option and check
  const auto buttonOption(
      qstyleoption_cast<const QStyleOptionButton *>(option));
  if (!buttonOption)
    return contentsSize;

  // output
  QSize size;

  // check text and icon
  const bool hasText(!buttonOption->text.isEmpty());
  const bool flat(buttonOption->features & QStyleOptionButton::Flat);
  bool hasIcon(!buttonOption->icon.isNull());

  if (!(hasText || hasIcon)) {
    /*
    no text nor icon is passed.
    assume custom button and use contentsSize as a starting point
    */
    size = contentsSize;

  } else {
    /*
    rather than trying to guess what Qt puts into its contents size calculation,
    we recompute the button size entirely, based on button option
    this ensures consistency with the rendering stage
    */

    // update has icon to honour showIconsOnPushButtons, when possible
    hasIcon &= (showIconsOnPushButtons() || flat || !hasText);

    // text
    if (hasText)
      size = buttonOption->fontMetrics.size(Qt::TextShowMnemonic,
                                            buttonOption->text);

    // icon
    if (hasIcon) {
      QSize iconSize = buttonOption->iconSize;
      if (!iconSize.isValid())
        iconSize = QSize(pixelMetric(PM_SmallIconSize, option, widget),
                         pixelMetric(PM_SmallIconSize, option, widget));

      size.setHeight(qMax(size.height(), iconSize.height()));
      size.rwidth() += iconSize.width();

      if (hasText)
        size.rwidth() += Render::Button_ItemSpacing;
    }
  }

  // menu
  const bool hasMenu(buttonOption->features & QStyleOptionButton::HasMenu);
  if (hasMenu) {
    size.rwidth() += Render::MenuButton_IndicatorWidth;
    if (hasText || hasIcon)
      size.rwidth() += Render::Button_ItemSpacing;
  }

  // expand with buttons margin
  size = expandSize(size, Render::Button_MarginWidth);

  // make sure buttons have a minimum width
  if (hasText) {
    size.setWidth(qMax(size.width(), int(Render::Button_MinWidth)));
  }

  // adjust the size add on the button size from StyleConfigData

  size.rwidth() += StyleConfigData::buttonWidth();
  size.rheight() += StyleConfigData::buttonHeight();

  // finally add frame margins
  return expandSize(size, Render::Frame_FrameWidth);
}

QSize Style::toolButtonSizeFromContents(const QStyleOption *option,
                                        const QSize &contentsSize,
                                        const QWidget *) const {
  // cast option and check
  const auto toolButtonOption =
      qstyleoption_cast<const QStyleOptionToolButton *>(option);
  if (!toolButtonOption)
    return contentsSize;

  // copy size
  QSize size = contentsSize;

  // get relevant state flags
  const State &state(option->state);
  const bool autoRaise(state & State_AutoRaise);

  const int marginWidth(autoRaise ? Render::ToolButton_MarginWidth
                                  : Render::Button_MarginWidth +
                                        Render::Frame_FrameWidth);

  size = expandSize(size, marginWidth);

  return size;
}

QSize Style::menuBarItemSizeFromContents(const QStyleOption *,
                                         const QSize &contentsSize,
                                         const QWidget *) const {
  return expandSize(contentsSize, Render::MenuBarItem_MarginWidth,
                    Render::MenuBarItem_MarginHeight);
}

QSize Style::menuItemSizeFromContents(const QStyleOption *option,
                                      const QSize &contentsSize,
                                      const QWidget *widget) const {
  // cast option and check
  const auto menuItemOption =
      qstyleoption_cast<const QStyleOptionMenuItem *>(option);
  if (!menuItemOption)
    return contentsSize;

  /*
   * First calculate the intrinsic size of the item.
   * this must be kept consistent with what's in drawMenuItemControl
   */
  switch (menuItemOption->menuItemType) {
  case QStyleOptionMenuItem::Normal:
  case QStyleOptionMenuItem::DefaultItem:
  case QStyleOptionMenuItem::SubMenu: {
    QString text = menuItemOption->text;
    qsizetype acceleratorSeparatorPos = text.indexOf(QLatin1Char('\t'));
    const bool hasAccelerator = acceleratorSeparatorPos >= 0;
    if (hasAccelerator) {
      text = text.left(acceleratorSeparatorPos);
    }

    QFontMetrics fm(menuItemOption->font);
    QSize size = fm.boundingRect({}, Qt::TextHideMnemonic, text).size();

    int iconWidth = 0;
    if (showIconsInMenuItems()) {
      iconWidth = isQtQuickControl(option, widget)
                      ? qMax(pixelMetric(PM_SmallIconSize, option, widget),
                             menuItemOption->maxIconWidth)
                      : menuItemOption->maxIconWidth;
    }

    int leftColumnWidth = 0;

    // add icon width
    if (iconWidth > 0) {
      leftColumnWidth += iconWidth + Render::MenuItem_ItemSpacing;
    }

    // add checkbox indicator width
    if (menuItemOption->menuHasCheckableItems) {
      leftColumnWidth += Render::CheckBox_Size + Render::MenuItem_ItemSpacing;
    }

    // add spacing for accelerator
    /*
     * Note:
     * The width of the accelerator itself is not included here since
     * Qt will add that on separately after obtaining the
     * sizeFromContents() for each menu item in the menu to be shown
     * ( see QMenuPrivate::calcActionRects() )
     */
    if (hasAccelerator) {
      size.rwidth() += Render::MenuItem_AcceleratorSpace;
    }

    // right column
    const int rightColumnWidth =
        Render::MenuButton_IndicatorWidth + Render::MenuItem_ItemSpacing;
    size.rwidth() += leftColumnWidth + rightColumnWidth;

    // make sure height is large enough for icon and arrow
    size.setHeight(
        qMax(size.height(), int(Render::MenuButton_IndicatorWidth)));
    size.setHeight(qMax(size.height(), int(Render::CheckBox_Size)));
    size.setHeight(qMax(size.height(), iconWidth));
    return expandSize(
        size, Render::MenuItem_MarginWidth,
        (Render::MenuItem_MarginHeight + StyleConfigData::menuItemHeight()));
  }

  case QStyleOptionMenuItem::Separator: {
    // contentsSize for separators in QMenuPrivate::updateActionRects() is {2,2}
    // We choose to override that.
    // Have at least 1px for separator line.
    int w = 1;
    int h = 1;

    // If the menu item is a section, add width for text
    // and make height the same as other menu items, plus extra top padding.
    if (!menuItemOption->text.isEmpty()) {
      auto font = menuItemOption->font;
      font.setBold(true);
      QFontMetrics fm(font);
      QRect textRect = fm.boundingRect(
          {}, Qt::TextSingleLine | Qt::TextHideMnemonic, menuItemOption->text);
      w = qMax(w, textRect.width());
      h = qMax(h, fm.height());

      if (showIconsInMenuItems()) {
        int iconWidth = menuItemOption->maxIconWidth;
        if (isQtQuickControl(option, widget)) {
          iconWidth =
              qMax(pixelMetric(PM_SmallIconSize, option, widget), iconWidth);
        }
        h = qMax(h, iconWidth);
      }

      if (menuItemOption->menuHasCheckableItems) {
        h = qMax(h, int(Render::CheckBox_Size));
      }

      h = qMax(h, int(Render::MenuButton_IndicatorWidth));
      h += (Render::MenuItem_MarginHeight +
            StyleConfigData::menuItemHeight()); // extra top padding
    }

    return {w + Render::MenuItem_MarginWidth * 2,
            h + (Render::MenuItem_MarginHeight +
                 StyleConfigData::menuItemHeight()) *
                    2};
  }

  // for all other cases, return input
  default:
    return contentsSize;
  }
}

QSize Style::progressBarSizeFromContents(const QStyleOption *option,
                                         const QSize &contentsSize,
                                         const QWidget *) const {
  // cast option
  const auto progressBarOption(
      qstyleoption_cast<const QStyleOptionProgressBar *>(option));
  if (!progressBarOption)
    return contentsSize;

  const bool horizontal(
      BlossomUIPrivate::isProgressBarHorizontal(progressBarOption));

  // make local copy
  QSize size(contentsSize);

  if (horizontal) {
    // check text visibility
    const bool textVisible(progressBarOption->textVisible);

    size.setWidth(qMax(size.width(), int(Render::ProgressBar_Thickness)));
    size.setHeight(qMax(size.height(), int(Render::ProgressBar_Thickness)));
    if (textVisible)
      size.setHeight(qMax(size.height(), option->fontMetrics.height()));

  } else {
    size.setHeight(qMax(size.height(), int(Render::ProgressBar_Thickness)));
    size.setWidth(qMax(size.width(), int(Render::ProgressBar_Thickness)));
  }

  return size;
}

QSize Style::tabWidgetSizeFromContents(const QStyleOption *option,
                                       const QSize &contentsSize,
                                       const QWidget *widget) const {
  // cast option and check
  const auto tabOption =
      qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option);
  if (!tabOption)
    return expandSize(contentsSize, Render::TabWidget_MarginWidth);

  // try find direct children of type QTabBar and QStackedWidget
  // this is needed in order to add TabWidget margins only if they are necessary
  // around tabWidget content, not the tabbar
  if (!widget)
    return expandSize(contentsSize, Render::TabWidget_MarginWidth);
  QTabBar *tabBar = nullptr;
  QStackedWidget *stack = nullptr;
  auto children(widget->children());
  foreach (auto child, children) {
    if (!tabBar)
      tabBar = qobject_cast<QTabBar *>(child);
    if (!stack)
      stack = qobject_cast<QStackedWidget *>(child);
    if (tabBar && stack)
      break;
  }

  if (!(tabBar && stack))
    return expandSize(contentsSize, Render::TabWidget_MarginWidth);

  // tab orientation
  const bool verticalTabs(tabOption && isVerticalTab(tabOption->shape));
  if (verticalTabs) {
    const int tabBarHeight = tabBar->minimumSizeHint().height();
    const int stackHeight = stack->minimumSizeHint().height();
    if (contentsSize.height() == tabBarHeight &&
        tabBarHeight + 2 * (Render::Frame_FrameWidth - 1) >=
            stackHeight + 2 * Render::TabWidget_MarginWidth)
      return QSize(contentsSize.width() + 2 * Render::TabWidget_MarginWidth,
                   contentsSize.height() + 2 * (Render::Frame_FrameWidth - 1));
    else
      return expandSize(contentsSize, Render::TabWidget_MarginWidth);

  } else {
    const int tabBarWidth = tabBar->minimumSizeHint().width();
    const int stackWidth = stack->minimumSizeHint().width();
    if (contentsSize.width() == tabBarWidth &&
        tabBarWidth + 2 * (Render::Frame_FrameWidth - 1) >=
            stackWidth + 2 * Render::TabWidget_MarginWidth)
      return QSize(contentsSize.width() + 2 * (Render::Frame_FrameWidth - 1),
                   contentsSize.height() + 2 * Render::TabWidget_MarginWidth);
    else
      return expandSize(contentsSize, Render::TabWidget_MarginWidth);
  }
}

QSize Style::tabBarTabSizeFromContents(const QStyleOption *option,
                                       const QSize &contentsSize,
                                       const QWidget *) const {
  const auto tabOption(qstyleoption_cast<const QStyleOptionTab *>(option));
  const bool hasText(tabOption && !tabOption->text.isEmpty());
  const bool hasIcon(tabOption && !tabOption->icon.isNull());
  const bool hasLeftButton(tabOption && !tabOption->leftButtonSize.isEmpty());
  const bool hasRightButton(tabOption && !tabOption->leftButtonSize.isEmpty());

  // calculate width increment for horizontal tabs
  int widthIncrement = 8;
  if (hasIcon && !(hasText || hasLeftButton || hasRightButton))
    widthIncrement -= 4;
  if (hasText && hasIcon)
    widthIncrement += Render::TabBar_TabItemSpacing;
  if (hasLeftButton && (hasText || hasIcon))
    widthIncrement += Render::TabBar_TabItemSpacing;
  if (hasRightButton && (hasText || hasIcon || hasLeftButton))
    widthIncrement += Render::TabBar_TabItemSpacing;
  const bool documentMode(tabOption && tabOption->documentMode);

  int extra;
  if (StyleConfigData::documentModeTabs())
    extra = documentMode ? 0 : 8;
  else
    extra = documentMode ? 0 : 2;

  // add margins
  QSize size(contentsSize);

  // compare to minimum size
  const bool verticalTabs(tabOption && isVerticalTab(tabOption));
  if (verticalTabs) {
    size.rheight() += widthIncrement;
    if (hasIcon && !hasText)
      size = size.expandedTo(QSize(
          Render::TabBar_TabMinHeight + StyleConfigData::tabsHeight(), 0));
    else
      size = size.expandedTo(
          QSize(Render::TabBar_TabMinHeight + StyleConfigData::tabsHeight(),
                Render::TabBar_TabMinWidth));

  } else {
    size.rwidth() += widthIncrement;
    if (hasIcon && !hasText)
      size = size.expandedTo(QSize(0, Render::TabBar_TabMinHeight +
                                          StyleConfigData::tabsHeight()));
    else
      size = size.expandedTo(QSize(Render::TabBar_TabMinWidth,
                                   Render::TabBar_TabMinHeight +
                                       StyleConfigData::tabsHeight() + extra));
  }

  return size;
}

QSize Style::headerSectionSizeFromContents(const QStyleOption *option,
                                           const QSize &contentsSize,
                                           const QWidget *) const {
  // cast option and check
  const auto headerOption(
      qstyleoption_cast<const QStyleOptionHeader *>(option));
  if (!headerOption)
    return contentsSize;

  // get text size
  const bool horizontal(headerOption->orientation == Qt::Horizontal);
  const bool hasText(!headerOption->text.isEmpty());
  const bool hasIcon(!headerOption->icon.isNull());

  const QSize textSize(
      hasText ? headerOption->fontMetrics.size(0, headerOption->text)
              : QSize());
  const QSize iconSize(hasIcon ? QSize(22, 22) : QSize());

  // contents width
  int contentsWidth(0);
  if (hasText)
    contentsWidth += textSize.width();
  if (hasIcon) {
    contentsWidth += iconSize.width();
    if (hasText)
      contentsWidth += Render::Header_ItemSpacing;
  }

  // contents height
  int contentsHeight(headerOption->fontMetrics.height());
  if (hasIcon)
    contentsHeight = qMax(contentsHeight, iconSize.height());

  if (horizontal && headerOption->sortIndicator != QStyleOptionHeader::None) {
    // also add space for sort indicator
    contentsWidth += Render::Header_ArrowSize + Render::Header_ItemSpacing;
    contentsHeight = qMax(contentsHeight, int(Render::Header_ArrowSize));
  }

  // update contents size, add margins and return
  const QSize size(
      contentsSize.expandedTo(QSize(contentsWidth, contentsHeight)));
  return expandSize(size, Render::Header_MarginWidth);
}

QSize Style::itemViewItemSizeFromContents(const QStyleOption *option,
                                          const QSize &contentsSize,
                                          const QWidget *widget) const {
  // call base class
  const QSize size(ParentStyleClass::sizeFromContents(CT_ItemViewItem, option,
                                                      contentsSize, widget));
  if (widget && widget->inherits("KFilePlacesView"))
    return size;
  if (!qobject_cast<const QTableView *>(widget)) {
    const QMargins margins = _helper->itemViewItemMargins(
        qstyleoption_cast<const QStyleOptionViewItem *>(option));
    return size + QSize(margins.left() + margins.right() +
                            Render::ItemView_ItemPaddingWidth * 2,
                        margins.top() + margins.bottom() +
                            Render::ItemView_ItemPaddingHeight * 2);
  }
  return expandSize(
      size,
      Render::ItemView_ItemMarginLeft + Render::ItemView_ItemMarginRight,
      Render::ItemView_ItemMarginBottom + Render::ItemView_ItemMarginTop);
}

} // namespace BlossomUI
