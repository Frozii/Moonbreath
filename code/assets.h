#ifndef ASSETS_H
#define ASSETS_H

#define FONT_ATLAS_WIDTH 1376
#define FONT_ATLAS_HEIGHT 32

#define START_ASCII_CHAR 33

// NOTE(rami): If you need to test a font
//render_text("!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'abcdefghijklmnopqrstuvwxyz{|}~", v2i(100, 100), color_white, font[font_classic]);

typedef enum
{
    font_classic,
    font_cursive,
    font_misc,
    
    font_total
} game_font;

typedef enum
{
    tex_tilemap,
    tex_game_tileset,
    tex_item_tileset,
    tex_wearable_item_tileset,
    tex_sprite_sheet,
    tex_inventory_win,
    tex_inventory_item_win,
    tex_inventory_selected_item,
    tex_interface_bottom_win,
    tex_health_bar_outside,
    tex_health_bar_inside,
    tex_player_parts,
    
    tex_total
} game_texture;

typedef struct
{
    u32 x, y, w, h;
    u32 unique_advance_in_px;
} glyph_metrics_t;

typedef struct
{
    b32 success;
    SDL_Texture *atlas;
    glyph_metrics_t metrics[94];
    u32 space_size;
    u32 shared_advance;
} font_t;

#endif // ASSETS_H