#include "codex_native.h"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/panel_container.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/scroll_container.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable.hpp>

using namespace godot;

namespace {

template <typename T>
T* node_as(Node* owner, const char* path) {
    return Object::cast_to<T>(owner->get_node_or_null(NodePath(path)));
}

Dictionary section(const String& title, const String& body, const String& image_path, const String& caption) {
    Dictionary data;
    data["title"] = title;
    data["body"] = body;
    data["image"] = image_path;
    data["caption"] = caption;
    return data;
}

}

void CodexNative::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_return_scene_path", "path"), &CodexNative::set_return_scene_path);
    ClassDB::bind_method(D_METHOD("get_return_scene_path"), &CodexNative::get_return_scene_path);
    ClassDB::bind_method(D_METHOD("set_close_returns_to_scene", "value"), &CodexNative::set_close_returns_to_scene);
    ClassDB::bind_method(D_METHOD("get_close_returns_to_scene"), &CodexNative::get_close_returns_to_scene);
    ClassDB::bind_method(D_METHOD("set_play_menu_music_on_ready", "value"), &CodexNative::set_play_menu_music_on_ready);
    ClassDB::bind_method(D_METHOD("get_play_menu_music_on_ready"), &CodexNative::get_play_menu_music_on_ready);
    ClassDB::bind_method(D_METHOD("show_standalone_mode"), &CodexNative::show_standalone_mode);
    ClassDB::bind_method(D_METHOD("_on_close_pressed"), &CodexNative::close_pressed);
    ClassDB::bind_method(D_METHOD("_show_section", "section_key"), &CodexNative::show_section);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "return_scene_path", PROPERTY_HINT_FILE, "*.tscn"), "set_return_scene_path", "get_return_scene_path");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "close_returns_to_scene"), "set_close_returns_to_scene", "get_close_returns_to_scene");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "play_menu_music_on_ready"), "set_play_menu_music_on_ready", "get_play_menu_music_on_ready");
}

void CodexNative::_ready() {
    close_button = node_as<Button>(this, "panel/margin/root_box/content_box/nav_box/close_button");
    panel = node_as<PanelContainer>(this, "panel");
    visual_panel = node_as<PanelContainer>(this, "panel/margin/root_box/content_box/article_box/visual_panel");
    visual_image = node_as<TextureRect>(this, "panel/margin/root_box/content_box/article_box/visual_panel/visual_margin/visual_box/visual_image");
    visual_caption = node_as<Label>(this, "panel/margin/root_box/content_box/article_box/visual_panel/visual_margin/visual_box/visual_caption");
    section_title_label = node_as<Label>(this, "panel/margin/root_box/content_box/article_box/section_title_label");
    body_scroll = node_as<ScrollContainer>(this, "panel/margin/root_box/content_box/article_box/body_scroll");
    body_label = node_as<RichTextLabel>(this, "panel/margin/root_box/content_box/article_box/body_scroll/body_label");

    nav_buttons["briefing"] = node_as<Button>(this, "panel/margin/root_box/content_box/nav_box/btn_briefing");
    nav_buttons["controls"] = node_as<Button>(this, "panel/margin/root_box/content_box/nav_box/btn_controls");
    nav_buttons["systems"] = node_as<Button>(this, "panel/margin/root_box/content_box/nav_box/btn_systems");
    nav_buttons["towers"] = node_as<Button>(this, "panel/margin/root_box/content_box/nav_box/btn_towers");
    nav_buttons["astrophage"] = node_as<Button>(this, "panel/margin/root_box/content_box/nav_box/btn_astrophage");
    nav_buttons["rings"] = node_as<Button>(this, "panel/margin/root_box/content_box/nav_box/btn_rings");
    nav_buttons["endings"] = node_as<Button>(this, "panel/margin/root_box/content_box/nav_box/btn_endings");
    build_sections();

    set_visible(true);
    if (play_menu_music_on_ready) {
        if (Node* music = get_node_or_null(NodePath("/root/MusicManager"))) {
            music->call("play_menu_music");
        }
    }
    apply_style();
    if (close_button != nullptr) {
        close_button->connect("pressed", Callable(this, "_on_close_pressed"));
    }
    const Array keys = nav_buttons.keys();
    for (int i = 0; i < keys.size(); ++i) {
        const String section_key = keys[i];
        Button* button = Object::cast_to<Button>(nav_buttons[section_key]);
        if (button == nullptr) {
            continue;
        }
        button->set_toggle_mode(true);
        button->connect("pressed", Callable(this, "_show_section").bind(section_key));
        if (Object* theme = space_theme()) {
            theme->call("apply_secondary_button", button);
        }
        button->add_theme_font_size_override("font_size", 16);
    }
    if (Object* theme = space_theme()) {
        theme->call("apply_secondary_button", close_button, theme->get("ICON_BACK_PATH"));
    }
    show_section("briefing");
}

void CodexNative::set_return_scene_path(const String& path) { return_scene_path = path; }
String CodexNative::get_return_scene_path() const { return return_scene_path; }
void CodexNative::set_close_returns_to_scene(bool value) { close_returns_to_scene = value; }
bool CodexNative::get_close_returns_to_scene() const { return close_returns_to_scene; }
void CodexNative::set_play_menu_music_on_ready(bool value) { play_menu_music_on_ready = value; }
bool CodexNative::get_play_menu_music_on_ready() const { return play_menu_music_on_ready; }

void CodexNative::show_standalone_mode() {
    close_returns_to_scene = false;
    set_visible(true);
    show_section("briefing");
}

void CodexNative::close_pressed() {
    if (close_returns_to_scene) {
        get_tree()->change_scene_to_file(return_scene_path);
    } else {
        queue_free();
    }
}

void CodexNative::show_section(const String& section_key) {
    current_section = section_key;
    Dictionary data = sections.get(section_key, sections["briefing"]);
    if (section_title_label != nullptr) {
        section_title_label->set_text(String(data["title"]));
    }
    if (body_label != nullptr) {
        String body = data["body"];
        if (Object* theme = space_theme()) {
            body_label->set_text(String(theme->call("format_readout_text", body)));
        } else {
            body_label->set_text(body);
        }
    }
    if (visual_image != nullptr) {
        const String image_path = String(data.get("image", ""));
        Ref<Texture2D> texture;
        if (!image_path.is_empty()) {
            texture = ResourceLoader::get_singleton()->load(image_path);
        }
        visual_image->set_texture(texture);
        visual_image->set_visible(texture.is_valid());
    }
    if (visual_caption != nullptr) {
        const String caption = String(data.get("caption", ""));
        visual_caption->set_text(caption);
        visual_caption->set_visible(!caption.is_empty());
    }
    if (body_scroll != nullptr) {
        body_scroll->set_v_scroll(0);
    }
    update_nav_state();
}

void CodexNative::update_nav_state() {
    Object* theme = space_theme();
    const Array keys = nav_buttons.keys();
    for (int i = 0; i < keys.size(); ++i) {
        const String section_key = keys[i];
        Button* button = Object::cast_to<Button>(nav_buttons[section_key]);
        if (button == nullptr) {
            continue;
        }
        const bool pressed = section_key == current_section;
        button->set_pressed_no_signal(pressed);
        if (theme != nullptr) {
            if (pressed) {
                theme->call("apply_primary_button", button);
            } else {
                theme->call("apply_secondary_button", button);
            }
        }
        button->add_theme_font_size_override("font_size", 16);
    }
}

void CodexNative::apply_style() {
    Object* theme = space_theme();
    if (theme == nullptr) {
        return;
    }
    theme->call("apply_cursor");
    theme->call("apply_fonts", this);
    theme->call("apply_deep_panel", panel, theme->get("COLOR_CYAN"));
    theme->call("apply_panel", visual_panel, theme->get("COLOR_GOLD"));
    theme->call("apply_scroll_container", body_scroll);
    theme->call("apply_rich_text_body", body_label, 17);
    if (visual_caption != nullptr) {
        visual_caption->add_theme_font_size_override("font_size", 14);
        visual_caption->add_theme_color_override("font_color", Color(0.82, 0.90, 1.0, 0.94));
    }
}

void CodexNative::build_sections() {
    sections["briefing"] = section(
        "Mission Briefing",
        R"(Situation
The Sun is no longer alone. Astrophage colonies ride the dark lanes between the rings, drinking heat and steering toward the core. The Sol Defense Corps calls the grid Perk: a last orbiting lattice of towers, labs, and flare capacitors.

Your role
You are the ring commander. Build the first satellites, read each wave before it launches, and keep enough light alive for the Sun to answer back.

Campaign orders
- Survive 12 authored waves.
- Preserve luminosity; every breach removes light from the star.
- Stop Burrowers quickly. Once lodged, they keep draining until Bio-Lab excavates them.
- Crack Astrophage Prime in wave 12 by opening its shell with Bio-Lab support.

Mode notes
- Normal Defense is the Prime campaign.
- Endless keeps scaling enemy count, HP, speed, breach damage, rewards, and special wave shapes.
- No-Flare Challenge uses the Prime campaign but disables Solar Flare for bonus Tech XP.

Command doctrine
- Build coverage first, then damage.
- Pair control with finishers: Cryo and Magnetic hold targets for Helios and Tardigrade.
- Mix damage types. Mimics ignore Photon, Farmers absorb Photon and Helios, and Prime resists anything before its shell opens.
- Sell stale placements when pressure shifts. A partial refund beats a silent tower.

Field phrase
Defend me, defend me! - Oa ka Perk!)",
        "res://assets/sprites/backgrounds/battle_nebula_hq.png",
        "Strategic view: the starfield is not decoration. It is the battlefield the orbit grid has to read before every wave."
    );

    sections["controls"] = section(
        "Field Controls",
        R"(Command loop
1. Read Wave Intel.
2. Choose a tower from the bay.
3. Place it on a ring slot that will actually see the threat.
4. Start the wave.
5. Upgrade, sell, or reposition before the next pressure lane forms.

Build controls
- Left click a tower, then click an open orbital slot.
- Click a placed tower to upgrade, sell, or inspect exact stats.
- Number keys 1-6 select towers from the bay.
- T opens the Tech Tree between decisions.

Camera controls
- Mouse wheel zooms around the cursor.
- WASD, screen-edge hover, or right/middle drag pans around the star.
- Home, 0, or Center Sun recenters the view.

Wave controls
- Space or Enter starts the next ready wave.
- Auto Start launches ready waves after a short countdown.
- F fires Solar Flare when charged; No-Flare Challenge disables it.
- Esc opens pause screens or backs out of overlays.

Fast habits
- Upgrade towers that are already firing before buying quiet coverage.
- Fire Solar Flare before a breach, not after luminosity is gone.
- Use pause to check this codex, settings, retry, or return to menu.)",
        "res://assets/ui/icons/icon_play.png",
        "Control icon: Start Defense is the first command, but Wave Intel decides whether the grid is ready."
    );

    sections["systems"] = section(
        "Core Systems",
        R"(The grid
Perk is split between native systems and scene scripts. C++ holds reusable game data, UI widgets, math, audio, and state. GDScript connects the live scene: input, drawing, wave flow, and tactical decisions.

Runtime state
- Luminosity tracks whether the Sun survives.
- Sol Credits pay for tower construction, upgrades, and emergency rebuilds.
- Score, kills, waves, victory, and luminosity create the end-run Tech XP payout.
- Best campaign, no-flare, and endless records save with settings.

Solar Flare
The flare is a manual radial burst charged by cleared waves. It punishes packed lanes and becomes stronger through Helios research. It is not available in No-Flare Challenge.

Tech Tree
Tech XP is permanent. Each tower has a three-step research path and an Apex endpoint. Research upgrades damage, range, fire rate, reward flow, flare cadence, and special tower behavior.

HUD intelligence
- Wave Intel previews enemy mix, warning tags, ring pressure, and rewards.
- Tower panels show cost, role, current stats, upgrade gains, final stats, and refund value.
- End screens show rank, score, kills, waves, luminosity, XP breakdown, best runs, Retry, Tech Tree, and Main Menu.

Commander read
The system is not asking for perfect towers. It is asking for time: more seconds before breach, more shots in range, more control on the right lane.)",
        "res://assets/ui/tech_tiers/bio_lab_tier_4.png",
        "Research visual: Tech XP turns one run's survival data into permanent tower-path upgrades."
    );

    sections["towers"] = section(
        "Tower Dossier",
        R"(Photon Splitter
Fast baseline direct damage. Good first answer on the Corona Belt. Weakness: Mimics ignore it and Farmers feed on it.

Cryo Probe
Control tower that slows threats and stretches engagement windows. Best when placed before heavy damage. Solar storm events can disrupt it.

Bio-Lab Station
Counter-biology platform. It clears lodged Burrowers, improves reward tempo through research, and opens Astrophage Prime's shell.

Magnetic Net
Wide support field. It slows enemies without needing to be the final hit, making it a strong partner for every heavy tower.

Helios Cannon
High-impact finisher. It deletes priority targets when timed well, but Farmers absorb it unless controlled or avoided.

Tardigrade Bomb
Heavy biological ordnance. It is slow, strong, and happiest when Cryo or Magnetic keeps enemies inside the blast window.

Build patterns
- Early wave: Photon plus Cryo covers speed and time.
- Mixed wave: add Magnetic before more damage.
- Burrower wave: keep Bio-Lab funded.
- Prime wave: Bio-Lab opens the shell, then finish with the strongest mixed damage available.

Upgrade rule
Upgrade towers that are already shooting. A level on a tower with targets is worth more than a fresh tower watching empty orbit.)",
        "res://assets/sprites/clean/towers/photon_splitter.png",
        "Tower visual: the Photon Splitter is the first clean answer, but the codex matters because not every enemy accepts Photon damage."
    );

    sections["astrophage"] = section(
        "Astrophage Variants",
        R"(Drifter
The baseline organism. It is not harmless; it teaches timing, reward flow, and how quickly a lane can become crowded.

Bloom
Splits into smaller bodies when defeated. Slow it before it breaks, or the child Drifters arrive as a second wave inside the first.

Burrower
A pressure organism built to reach the Sun. If it lodges, luminosity keeps bleeding until Bio-Lab clears the infection.

Mimic
Carries the MIMIC tag. Photon Splitters waste shots on it, so mixed tower plans are mandatory.

Farmer
Carries the ABSORB tag. Photon and Helios feed it, giving it HP and speed. Control it, route around it, or let non-feeding damage handle it.

Astrophage Prime
The wave 12 command organism. SHELL blocks most damage. Bio-Lab must open the shell before the rest of the grid can finish the target.

Threat reading
- If Wave Intel says MIMIC, do not trust Photon coverage alone.
- If it says ABSORB, stop feeding Farmers.
- If it says SHELL, fund Bio-Lab before buying prettier damage.
- If breaches start chaining, buy control before chasing another finisher.)",
        "res://assets/sprites/clean/enemies_optimized/astrophage-shell_idle_1.png",
        "Enemy visual: Prime's shell is a warning label. Until Bio-Lab opens it, most damage is just noise."
    );

    sections["rings"] = section(
        "Rings + Waves",
        R"(Orbital map
The rings are not just build slots. They are timing machines. A tower's value changes with radius, orbit period, and how long enemies stay inside range.

Ring 1 - Corona Belt
- Radius 80 px, period 6 s, 4 slots.
- Fast orbit, short windows.
- Best for early Photon or Helios intercepts.

Ring 2 - Chromosphere Band
- Radius 140 px, period 11 s, 6 slots.
- Good balance of speed and reach.
- Strong for Cryo Probe and Tardigrade Bomb support.

Ring 3 - Photosphere Arc
- Radius 210 px, period 17 s, 8 slots.
- Slower, broader coverage.
- Strong for Bio-Lab Station and Magnetic Net.

Ring 4 - Outer Veil
- Radius 290 px, period 26 s, 10 slots.
- Longest intercept window.
- Best for early warning, wide control, and lane shaping.

Wave landmarks
- Wave 6 tests your response to solar storm disruption.
- Wave 7 stresses night-side ring pressure.
- Wave 10 rewards Bio-Lab planning.
- Wave 12 deploys Astrophage Prime.
- Endless reshuffles the pressure forever.

Reading the intel
Enemy tags tell you what not to shoot. Ring pressure tells you where to spend. Rewards tell you whether to build now or survive to upgrade after the wave.)",
        "res://assets/sprites/backgrounds/menu_nebula.png",
        "Orbit visual: every ring is a clock. Winning means placing the right weapon where its clock meets the enemy path."
    );

    sections["endings"] = section(
        "Victory + Failure",
        R"(End states
The campaign does not only ask whether the Sun survived. It asks how much light you preserved and how much the grid learned from the run.

Full Shine
Clear 12 waves with luminosity above 80%. This is clean defense: good counters, few breaches, and strong late-wave control.

Dim but Alive
Clear 12 waves with luminosity from 20% to 80%. The Sun survives, but the record says where the grid leaked.

Last Light
Clear 12 waves with luminosity from 1% to 20%. A survival story, not a clean one. Spend the Tech XP and patch the weakness.

Sun Extinguished
Luminosity reaches 0%. The post-mortem still matters: kills, wave reached, and score show what held and what collapsed.

Run records
- Campaign, No-Flare Challenge, and Endless save their best results.
- End screens show score, kills, waves, luminosity, victory bonus, total XP, and best-run notes.
- Retry tests a better opening.
- Tech Tree spends the lesson.
- Main Menu changes the mission mode.

Last order
Do not chase a beautiful orbit. Chase a living Sun.)",
        "res://assets/ui/icons/icon_codex.png",
        "Record visual: the codex turns losses into instructions, then the Tech Tree turns instructions into power."
    );
}

Object* CodexNative::space_theme() const {
    static Ref<RefCounted> resource;
    if (resource.is_null()) {
        resource = Ref<RefCounted>(Object::cast_to<RefCounted>(ClassDB::instantiate("SpaceThemeNative")));
    }
    return resource.ptr();
}
