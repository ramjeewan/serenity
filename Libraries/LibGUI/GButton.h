#pragma once

#include <AK/AKString.h>
#include <AK/Function.h>
#include <LibGUI/GAbstractButton.h>
#include <LibDraw/GraphicsBitmap.h>
#include <LibDraw/StylePainter.h>
#include <LibDraw/TextAlignment.h>

class GAction;

class GButton : public GAbstractButton {
    C_OBJECT(GButton)
public:
    GButton(const StringView& text, GWidget* parent);
    explicit GButton(GWidget* parent);
    virtual ~GButton() override;

    void set_icon(RefPtr<GraphicsBitmap>&&);
    const GraphicsBitmap* icon() const { return m_icon.ptr(); }
    GraphicsBitmap* icon() { return m_icon.ptr(); }

    void set_text_alignment(TextAlignment text_alignment) { m_text_alignment = text_alignment; }
    TextAlignment text_alignment() const { return m_text_alignment; }

    Function<void(GButton&)> on_click;

    void set_button_style(ButtonStyle style) { m_button_style = style; }
    ButtonStyle button_style() const { return m_button_style; }

    virtual void click() override;

    void set_action(GAction&);

    virtual bool accepts_focus() const override { return m_focusable; }
    virtual bool supports_keyboard_activation() const override;
    virtual bool is_uncheckable() const override;

    void set_focusable(bool b) { m_focusable = b; }

protected:
    virtual void paint_event(GPaintEvent&) override;

private:
    RefPtr<GraphicsBitmap> m_icon;
    ButtonStyle m_button_style { ButtonStyle::Normal };
    TextAlignment m_text_alignment { TextAlignment::Center };
    WeakPtr<GAction> m_action;
    bool m_focusable { true };
};
