#include <LibGUI/GBoxLayout.h>
#include <LibGUI/GPainter.h>
#include <LibGUI/GTabWidget.h>
#include <LibDraw/StylePainter.h>

GTabWidget::GTabWidget(GWidget* parent)
    : GWidget(parent)
{
    set_fill_with_background_color(true);
    set_background_color(Color::WarmGray);
}

GTabWidget::~GTabWidget()
{
}

void GTabWidget::add_widget(const StringView& title, GWidget* widget)
{
    m_tabs.append({ title, widget });
    add_child(*widget);
}

void GTabWidget::set_active_widget(GWidget* widget)
{
    if (widget == m_active_widget)
        return;

    if (m_active_widget)
        m_active_widget->set_visible(false);
    m_active_widget = widget;
    if (m_active_widget) {
        m_active_widget->set_relative_rect(child_rect_for_size(size()));
        m_active_widget->set_visible(true);
    }

    update_bar();
}

void GTabWidget::resize_event(GResizeEvent& event)
{
    if (!m_active_widget)
        return;
    m_active_widget->set_relative_rect(child_rect_for_size(event.size()));
}

Rect GTabWidget::child_rect_for_size(const Size& size) const
{
    switch (m_tab_position) {
    case TabPosition::Top:
        return { { container_padding(), bar_height() + container_padding() }, { size.width() - container_padding() * 2, size.height() - bar_height() - container_padding() * 2 } };
    case TabPosition::Bottom:
        return { { container_padding(), container_padding() }, { size.width() - container_padding() * 2, size.height() - bar_height() - container_padding() * 2 } };
    }
    ASSERT_NOT_REACHED();
}

void GTabWidget::child_event(CChildEvent& event)
{
    if (!event.child() || !is<GWidget>(*event.child()))
        return GWidget::child_event(event);
    auto& child = to<GWidget>(*event.child());
    if (event.type() == GEvent::ChildAdded) {
        if (!m_active_widget)
            set_active_widget(&child);
        else if (m_active_widget != &child)
            child.set_visible(false);
    } else if (event.type() == GEvent::ChildRemoved) {
        if (m_active_widget == &child) {
            GWidget* new_active_widget = nullptr;
            for_each_child_widget([&](auto& new_child) {
                new_active_widget = &new_child;
                return IterationDecision::Break;
            });
            set_active_widget(new_active_widget);
        }
    }
    GWidget::child_event(event);
}

Rect GTabWidget::bar_rect() const
{
    switch (m_tab_position) {
    case TabPosition::Top:
        return { 0, 0, width(), bar_height() };
    case TabPosition::Bottom:
        return { 0, height() - bar_height(), width(), bar_height() };
    }
    ASSERT_NOT_REACHED();
}

Rect GTabWidget::container_rect() const
{
    switch (m_tab_position) {
    case TabPosition::Top:
        return { 0, bar_height(), width(), height() - bar_height() };
    case TabPosition::Bottom:
        return { 0, 0, width(), height() - bar_height() };
    }
    ASSERT_NOT_REACHED();
}

void GTabWidget::paint_event(GPaintEvent& event)
{
    GPainter painter(*this);
    painter.add_clip_rect(event.rect());

    auto container_rect = this->container_rect();
    auto padding_rect = container_rect;
    for (int i = 0; i < container_padding(); ++i) {
        painter.draw_rect(padding_rect, background_color());
        padding_rect.shrink(2, 2);
    }

    StylePainter::paint_frame(painter, container_rect, FrameShape::Container, FrameShadow::Raised, 2);

    for (int i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs[i].widget == m_active_widget)
            continue;
        bool hovered = i == m_hovered_tab_index;
        auto button_rect = this->button_rect(i);
        StylePainter::paint_tab_button(painter, button_rect, false, hovered, m_tabs[i].widget->is_enabled());
        painter.draw_text(button_rect.translated(0, 1), m_tabs[i].title, TextAlignment::Center);
    }

    for (int i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs[i].widget != m_active_widget)
            continue;
        bool hovered = i == m_hovered_tab_index;
        auto button_rect = this->button_rect(i);
        StylePainter::paint_tab_button(painter, button_rect, true, hovered, m_tabs[i].widget->is_enabled());
        painter.draw_text(button_rect.translated(0, 1), m_tabs[i].title, TextAlignment::Center);
        painter.draw_line(button_rect.bottom_left().translated(1, 1), button_rect.bottom_right().translated(-1, 1), background_color());
        break;
    }
}

Rect GTabWidget::button_rect(int index) const
{
    int x_offset = 2;
    for (int i = 0; i < index; ++i)
        x_offset += m_tabs[i].width(font());
    Rect rect { x_offset, 0, m_tabs[index].width(font()), bar_height() };
    if (m_tabs[index].widget != m_active_widget) {
        rect.move_by(0, 2);
        rect.set_height(rect.height() - 2);
    } else {
        rect.move_by(-2, 0);
        rect.set_width(rect.width() + 4);
    }
    rect.move_by(bar_rect().location());
    return rect;
}

int GTabWidget::TabData::width(const Font& font) const
{
    return 16 + font.width(title);
}

void GTabWidget::mousedown_event(GMouseEvent& event)
{
    for (int i = 0; i < m_tabs.size(); ++i) {
        auto button_rect = this->button_rect(i);
        if (!button_rect.contains(event.position()))
            continue;
        set_active_widget(m_tabs[i].widget);
        return;
    }
}

void GTabWidget::mousemove_event(GMouseEvent& event)
{
    int hovered_tab = -1;
    for (int i = 0; i < m_tabs.size(); ++i) {
        auto button_rect = this->button_rect(i);
        if (!button_rect.contains(event.position()))
            continue;
        hovered_tab = i;
        if (m_tabs[i].widget == m_active_widget)
            break;
    }
    if (hovered_tab == m_hovered_tab_index)
        return;
    m_hovered_tab_index = hovered_tab;
    update_bar();
}

void GTabWidget::leave_event(CEvent&)
{
    if (m_hovered_tab_index != -1) {
        m_hovered_tab_index = -1;
        update_bar();
    }
}

void GTabWidget::update_bar()
{
    auto invalidation_rect = bar_rect();
    invalidation_rect.set_height(invalidation_rect.height() + 1);
    update(invalidation_rect);
}

void GTabWidget::set_tab_position(TabPosition tab_position)
{
    if (m_tab_position == tab_position)
        return;
    m_tab_position = tab_position;
    if (m_active_widget)
        m_active_widget->set_relative_rect(child_rect_for_size(size()));
    update();
}
