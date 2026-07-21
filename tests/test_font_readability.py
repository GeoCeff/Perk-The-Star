from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
THEME = (ROOT / "gdextension/src/space_theme_native.cpp").read_text(encoding="utf-8")

assert 'constexpr const char* FONT_BUTTON_PATH = FONT_BODY_PATH;' in THEME
assert 'const bool use_display = label->get_theme_font_size("font_size") >= 24;' in THEME
for font_import in (ROOT / "assets/fonts").glob("*.ttf.import"):
    assert "generate_mipmaps=true" in font_import.read_text(encoding="utf-8")
