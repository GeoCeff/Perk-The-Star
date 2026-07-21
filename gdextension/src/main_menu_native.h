#pragma once

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/input_event.hpp>

namespace godot {

class Button;
class Label;
class PanelContainer;
class Texture2D;
class VBoxContainer;

class MainMenuNative : public Control {
    GDCLASS(MainMenuNative, Control)

protected:
    static void _bind_methods();

public:
    void _ready() override;
    void _input(const Ref<InputEvent>& event) override;
    void _unhandled_input(const Ref<InputEvent>& event) override;

private:
    Button* btn_play = nullptr;
    Button* btn_tech_tree = nullptr;
    Button* btn_achievements = nullptr;
    Button* btn_codex = nullptr;
    Button* btn_settings = nullptr;
    Button* btn_credits = nullptr;
    Button* btn_exit = nullptr;
    PanelContainer* menu_frame = nullptr;
    Label* title_label = nullptr;
    Label* sub_label = nullptr;
    Label* tagline_label = nullptr;
    Label* description_label = nullptr;
    Label* version_label = nullptr;
    Label* author_label = nullptr;
    Control* mode_overlay = nullptr;
    Control* achievements_overlay = nullptr;
    PanelContainer* mode_info_panel = nullptr;
    Label* mode_info_title = nullptr;
    Label* mode_info_kicker = nullptr;
    Label* mode_info_body = nullptr;
    Label* mode_info_note = nullptr;

    void apply_menu_style();
    void show_mode_overlay();
    void add_mode_button(VBoxContainer* box, const String& mode, bool primary);
    void build_mode_info_panel();
    void show_mode_info(const String& mode);
    void close_mode_overlay();
    void show_tech_tree();
    void show_achievements();
    void close_achievements();
    void start_mode(const String& mode);
    Ref<Texture2D> achievement_icon(int index) const;
    Object* space_theme() const;
};

}
