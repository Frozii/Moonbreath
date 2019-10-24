#include <SDL2/include/SDL.h>
#include <SDL2/include/SDL_image.h>
#include <SDL2/include/SDL_ttf.h>

#include <time.h>
#include <stdint.h>

#include "types.h"
#include "util.c"
#include "dungeon.c"
#include "fov.c"
#include "render.c"
#include "item.c"
#include "ui.c"
#include "pathfind.c"
// #include "conf.c" // TODO(rami): Work on conf when we need it again
#include "assets.c"
#include "pop_text.c"
#include "monster.c"
#include "player.c"

// NOTE(rami): Two Steps
// Write the fastest, simpliest way what you need, make it actually work.
// Can you clean it? Simplify it? Pull things into reusable functions? (Compression Oriented)

// TODO(rami): Better health potion art
// TODO(rami): Ring of Protection needs to stand out more from the background

// TODO(rami): When a monster or monsters come into view have a message saying something like
// "You see a Slime."
// With multiple enemies:
// "You see a Slime and a Skeleton" on the same line to save message space.
// Or have them in the single version for every enemy.

// TODO(rami): When you see an item, "You found a weapon.", "You found a scroll."

// TODO(rami): After the ground work for the dungeon level layouts is done
// we can focus more on adding monsters, items, gold etc. to the levels.
// We also need to think about how we want to do our animation,
// we could animate each item on the player that's being worn but
// that's a lot of work, we could instead just have a sprite for the
// player and animate that so that worn items wouldn't be seen outside of
// the inventory.

// TODO(rami): Message when you get a level up, what it actually changes?

internal void
resize_window(u32 w, u32 h)
{
    SDL_SetWindowSize(game.window, w, h);
    game.window_size = V2u(w, h);
    game.console_size.w = game.window_size.w;
    game.camera = V4s(0, 0, game.window_size.w, game.window_size.h - game.console_size.h);
}

internal void
toggle_fullscreen()
{
    u32 flags = SDL_GetWindowFlags(game.window);
    if(flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
    {
        SDL_SetWindowFullscreen(game.window, 0);
        resize_window(1280, 720);
        SDL_SetWindowPosition(game.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
    else
    {
        SDL_SetWindowFullscreen(game.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        
        v2u window_size = {0};
        SDL_GetWindowSize(game.window, (s32 *)&window_size.x, (s32 *)&window_size.y);
        resize_window(window_size.x, window_size.y);
    }
}

internal void
update_camera()
{
#if 0
    printf("camera.x1: %d\n", game.camera.x);
    printf("camera.y1: %d\n", game.camera.y);
    printf("camera.x2: %d\n", game.camera.x + game.camera.w);
    printf("camera.y2: %d\n\n", game.camera.y + game.camera.h);
#endif
    
    game.camera.x = tile_mul(player.pos.x) - (game.camera.w * 0.5);
    
    // NOTE(rami): This gives us 24 pixels from the top and bottom
    // initially when the camera is not locked to an edge which seems to be
    // the closest we can get to 32 pixels.
    game.camera.y = tile_mul(player.pos.y) - (game.camera.h * 0.5) + (player.h * 0.5);
    
    if(game.camera.x < 0)
    {
        game.camera.x = 0;
    }
    
    if(game.camera.y < 0)
    {
        game.camera.y = 0;
    }
    
    if(game.camera.x >= (s32)(tile_mul(MAX_DUNGEON_WIDTH) - game.camera.w))
    {
        game.camera.x = tile_mul(MAX_DUNGEON_WIDTH) - game.camera.w;
    }
    
    if(game.camera.y >= (s32)(tile_mul(MAX_DUNGEON_HEIGHT) - game.camera.h))
    {
        game.camera.y = tile_mul(MAX_DUNGEON_HEIGHT) - game.camera.h;
    }
}

internal void
update_events()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_QUIT)
        {
            game.state = state_quit;
        }
        
        else if(event.type == SDL_KEYDOWN)
        {
#if 1
            if(1)
#else
                if(!event.key.repeat)
#endif
            {
                SDL_Scancode key = event.key.keysym.scancode;
                
                b32 alt_key_was_down = (event.key.keysym.mod & KMOD_ALT);
                if((key == SDL_SCANCODE_F4) && alt_key_was_down)
                {
                    game.state = state_quit;
                }
                else if((key == SDL_SCANCODE_RETURN) && alt_key_was_down)
                {
                    SDL_Window *window = SDL_GetWindowFromID(event.window.windowID);
                    if(window)
                    {
                        toggle_fullscreen();
                    }
                }
                else
                {
                    player_keypress(key);
                }
            }
        }
    }
}

internal b32
set_window_icon()
{
    b32 result = false;
    
    SDL_Surface *icon = IMG_Load("data/images/icon.png");
    if(icon)
    {
        SDL_SetWindowIcon(game.window, icon);
        SDL_FreeSurface(icon);
        result = true;
    }
    else
    {
        printf("ERROR: Failed to load window icon\n");
    }
    
    return(result);
}

internal b32
set_fonts()
{
    b32 result = true;
    
    fonts[font_classic] = create_bmp_font("data/fonts/classic16x16.png", 16, 16, 14, 8, 12);
    fonts[font_classic_outlined] = create_bmp_font("data/fonts/pop_up16x16.png", 16, 16, 14, 8, 12);
    fonts[font_cursive] = create_ttf_font("data/fonts/alkhemikal.ttf", 16, 4);
    fonts[font_misc] = create_ttf_font("data/fonts/monaco.ttf", 16, 4);
    
    for(u32 i = 0; i < font_total; ++i)
    {
        if(!fonts[i]->success)
        {
            result = false;
            printf("ERROR: Font atlas %u could not be created\n", i);
        }
    }
    
    return(result);
}

internal b32
set_textures()
{
    b32 result = true;
    
    textures[tex_tilemap].tex = SDL_CreateTexture(game.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, tile_mul(MAX_DUNGEON_WIDTH), tile_mul(MAX_DUNGEON_HEIGHT));
    textures[tex_tilemap].w = tile_mul(MAX_DUNGEON_WIDTH);
    textures[tex_tilemap].h = tile_mul(MAX_DUNGEON_HEIGHT);
    
    textures[tex_game_tileset] = load_texture("data/images/game_tileset.png", 0);
    textures[tex_item_tileset] = load_texture("data/images/item_tileset.png", 0);
    textures[tex_wearable_item_tileset] = load_texture("data/images/wearable_item_tileset.png", 0);
    textures[tex_sprite_sheet] = load_texture("data/images/sprite_sheet.png", 0);
    textures[tex_inventory_win] = load_texture("data/images/inventory_win.png", 0);
    textures[tex_inventory_item_win] = load_texture("data/images/inventory_item_win.png", 0);
    textures[tex_inventory_selected_item] = load_texture("data/images/inventory_selected_item.png", 0);
    textures[tex_interface_bottom_win] = load_texture("data/images/interface_bottom_win.png", 0);
    textures[tex_health_bar_outside] = load_texture("data/images/health_bar_outside.png", 0);
    textures[tex_health_bar_inside] = load_texture("data/images/health_bar_inside.png", 0);
    textures[tex_player_parts] = load_texture("data/images/player_parts.png", 0);
    
    for(u32 i = 0; i < tex_total; ++i)
    {
        if(!textures[i].tex)
        {
            result = false;
            printf("ERROR: Texture %u could not be created\n", i);
        }
    }
    
    return(result);
}

internal void
set_game_data()
{
    // TODO(rami): Debug
    u64 seed = 16674218;
    
    //u64 seed = time(0);
    srand(seed);
    printf("Random Seed: %lu\n\n", seed);
    
    game.state = state_running;
    game.window_size = V2u(1280, 720);
    game.console_size = V2u(game.window_size.w, 160);
    game.camera = V4s(0, 0, game.window_size.w, game.window_size.h - game.console_size.h);
    game.turn_changed = false;
    
    dungeon.level = 1;
    dungeon.w = 64;
    dungeon.h = 64;
    
    for(u32 i = 0; i < array_count(items); ++i)
    {
        items[i].unique_id = i + 1;
    }
    
    set_monster_spawn_chances();
    set_item_info_data();
}

internal u32
init_game()
{
    b32 result = false;
    
    set_game_data();
    
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        u32 window_flags = 0;
        game.window = SDL_CreateWindow("Moonbreath", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                       game.window_size.w, game.window_size.h,
                                       window_flags);
        if(game.window)
        {
            printf("Monitor refresh rate: %dHz\n\n", get_window_refresh_rate());
            
            u32 renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
            game.renderer = SDL_CreateRenderer(game.window, -1, renderer_flags);
            if(game.renderer)
            {
                u32 img_flags = IMG_INIT_PNG;
                if(IMG_Init(img_flags) & img_flags)
                {
                    if(!TTF_Init())
                    {
                        if(set_window_icon())
                        {
                            if(set_fonts())
                            {
                                if(set_textures())
                                {
                                    result = true;
                                }
                                else
                                {
                                    // NOTE(rami): Texture failed
                                }
                            }
                            else
                            {
                                // NOTE(rami): Font failed
                            }
                        }
                        else
                        {
                            // NOTE(rami): Window icon failed
                        }
                    }
                    else
                    {
                        printf("ERROR: SDL TTF library could not initialize: %s\n", SDL_GetError());
                    }
                }
                else
                {
                    printf("ERROR: SLD image library could not initialize: %s\n", SDL_GetError());
                }
            }
            else
            {
                printf("ERROR: SDL could not create a renderer: %s\n", SDL_GetError());
            }
        }
        else
        {
            printf("ERROR: SDL could not create window: %s\n", SDL_GetError());
        }
    }
    else
    {
        printf("ERROR: SDL could not initialize: %s\n", SDL_GetError());
    }
    
    return(result);
}

internal void
array_debug()
{
    // NOTE(rami): Pop up text
#if 0
    for(s32 i = array_count(pop_texts) - 1; i > -1; --i)
    {
        pop_text_t *pop_text = &pop_texts[i];
        if(pop_texts[i].active)
        {
            printf("\npop_up_text[%u]\n", i);
            printf("str: %s\n", pop_text->str);
            printf("x: %u, y: %u\n", pop_text->pos.x, pop_text->pos.y);
            printf("change: %.02f\n", pop_text->change);
            printf("speed: %.02f\n", pop_text->speed);
            printf("duration_time: %ums\n", pop_text->duration_time);
            printf("start_time: %ums\n", pop_text->start_time);
        }
    }
#endif
    
    // NOTE(rami): Inventory
#if 0
    for(s32 i = array_count(inventory.slots) - 1; i > -1; --i)
    {
        item_t *inv_slot = &inventory.slots[i];
        if(inv_slot->id)
        {
            printf("\ninventory.slots[%u]\n", i);
            printf("id %u\n", inv_slot->id);
            printf("unique_id %u\n", inv_slot->unique_id);
            printf("x: %u, y: %u\n", inv_slot->pos.x, inv_slot->pos.y);
            printf("in_inventory %u\n", inv_slot->in_inventory);
            printf("is_equipped %u\n", inv_slot->is_equipped);
        }
    }
#endif
    
#if 0
    // NOTE(rami): Item Info
    for(s32 i = array_count(item_info) - 1; i > -1; --i)
    {
        item_info_t *info = &item_info[i];
        if(info->id)
        {
            printf("\nitem_info[%u]\n", i);
            printf("id: %u\n", info->id);
            printf("name: %s\n", info->name);
            printf("slot: %u\n", info->slot);
            printf("DESCRIPTION SKIPPED\n");
            printf("tile: %u, %u\n", info->tile.x, info->tile.y);
            
            if(info->type == type_weapon || info->type == type_armor)
            {
                printf("strength: %u\n", info->general.strength);
                printf("defence: %u\n", info->general.defence);
                printf("hp: %u\n", info->general.hp);
            }
            else if(item_info[i].type == type_consumable)
            {
                printf("effect: %u\n", info->consumable.effect);
                printf("effect_text: %s\n", info->consumable.effect_text);
                printf("effect_amount: %u\n", info->consumable.effect_amount);
            }
        }
    }
#endif
    
    // NOTE(rami): Item
#if 0
    for(s32 i = array_count(items) - 1; i > -1; --i)
    {
        item_t *item = &items[i];
        if(item->id)
        {
            printf("\nitems[%u]\n", i);
            printf("id %u\n", item->id);
            printf("unique_id %u\n", item->unique_id);
            printf("x: %u, y: %u\n", item->pos.x, item->pos.y);
            printf("in_inventory %u\n", item->in_inventory);
            printf("is_equipped %u\n", item->is_equipped);
        }
    }
#endif
    
    // NOTE(rami): Player
#if 0
    printf("\nPlayer\n");
    printf("new_x, new_y: %u, %u\n", player.new_pos.x, player.new_pos.y);
    printf("x, y: %u, %u\n", player.pos.x, player.pos.y);
    printf("w, h: %u, %u\n", player.w, player.h);
    printf("name: %s\n", player.name);
    printf("max_hp: %u\n", player.max_hp);
    printf("hp: %u\n", player.hp);
    printf("strength: %u\n", player.strength);
    printf("defence: %u\n", player.defence);
    printf("speed: %u\n", player.speed);
    printf("level: %u\n", player.level);
    printf("money: %u\n", player.money);
    printf("fov: %u\n", player.fov);
    printf("sprite_flip: %u\n", player.sprite_flip);
#endif
    
    // NOTE(rami): Monster
#if 0
    for(s32 i = array_count(monsters) - 1; i > -1; --i)
    {
        monster_t *monster = &monsters[i];
        if(monster->type)
        {
            printf("\nmonster[%u]\n", i);
            printf("type: %u\n", monster->type);
            printf("ai: %u\n", monster->ai);
            
            printf("x, y: %u, %u\n", monster->pos.x, monster->pos.y);
            printf("w, h: %u, %u\n", monster->size.w, monster->size.h);
            printf("in_combat: %u\n", monster->in_combat);
            printf("max_hp: %u\n", monster->max_hp);
            printf("hp: %u\n", monster->hp);
            printf("damage: %u\n", monster->damage);
            printf("armor: %u\n", monster->armor);
            printf("speed: %u\n", monster->speed);
            printf("level: %u\n", monster->level);
        }
    }
#endif
}

internal void
run_game()
{
    init_player();
    generate_dungeon();
    update_fov(); // NOTE(rami): This is so that we can see without moving initially.
    
#if 0
    add_monster(monster_slime, V2u(0, 0));
#endif
    
    /*add_item(id_iron_sword, V2u(player.pos.x, player.pos.y));
    add_item(id_lesser_health_potion, V2u(player.pos.x + 1, player.pos.y));
    add_item(id_knight_greaves, V2u(player.pos.x + 2, player.pos.y));
    add_item(id_ring_of_protection, V2u(player.pos.x + 3, player.pos.y));*/
    
    add_item(id_iron_sword_new, V2u(player.pos.x, player.pos.y + 1));
    add_item(id_iron_sword_old, V2u(player.pos.x, player.pos.y + 2));
    //add_item(id_ring_of_protection, V2u(player.pos.x, player.pos.y + 3));
    //add_item(id_lesser_health_potion, V2u(player.pos.x, player.pos.y + 4));
    
#if 0
    add_item(id_rune_helmet, V2u(player.pos.x, player.pos.y));
    add_item(id_rune_amulet, V2u(player.pos.x + 1, player.pos.y));
    add_item(id_rune_chestplate, V2u(player.pos.x + 2, player.pos.y));
    add_item(id_rune_platelegs, V2u(player.pos.x + 3, player.pos.y));
    add_item(id_rune_boots, V2u(player.pos.x + 4, player.pos.y));
    add_item(id_iron_sword, V2u(player.pos.x + 5, player.pos.y));
    add_item(id_iron_sword, V2u(player.pos.x + 6, player.pos.y));
    add_item(id_rune_shield, V2u(player.pos.x + 7, player.pos.y));
    add_item(id_rune_ring, V2u(player.pos.x + 8, player.pos.y));
#endif
    
    u32 frames_per_second = 60;
    f32 target_seconds_per_frame = 1.0f / (f32)frames_per_second;
    
    u64 old_counter = SDL_GetPerformanceCounter();
    f32 old_dt = SDL_GetPerformanceCounter();
    f32 perf_count_frequency = (f32)SDL_GetPerformanceFrequency();
    
    while(game.state)
    {
        f32 new_dt = SDL_GetPerformanceCounter();
        game.dt = (f32)(new_dt - old_dt) / perf_count_frequency;
        old_dt = new_dt;
        
        // TODO(rami): Debug
        array_debug();
        
        SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
        SDL_RenderClear(game.renderer);
        
        update_events();
        
        if(game.turn_changed)
        {
            update_player();
            update_monsters();
            update_fov();
            
            game.turn_changed = false;
        }
        
        update_camera();
        update_pop_text();
        
        render_tilemap();
        render_items();
        render_monsters();
        render_player();
        render_ui();
        render_pop_text();
        
        u64 work_counter_elapsed = SDL_GetPerformanceCounter() - old_counter;
        f32 ms_for_work = (1000.0f * (f32)work_counter_elapsed) / perf_count_frequency;
        
        if(get_seconds_elapsed(old_counter, SDL_GetPerformanceCounter(), perf_count_frequency) < target_seconds_per_frame)
        {
            u32 time_to_delay =
                ((target_seconds_per_frame - get_seconds_elapsed(old_counter, SDL_GetPerformanceCounter(), perf_count_frequency)) * 1000) - 1;
            SDL_Delay(time_to_delay);
            
            while(get_seconds_elapsed(old_counter, SDL_GetPerformanceCounter(), perf_count_frequency)
                  < target_seconds_per_frame)
            {
            }
        }
        else
        {
            // NOTE(rami): Valgrind will trigger this!
            //assert(0, "Missed frate rate");
        }
        
        u64 new_counter = SDL_GetPerformanceCounter();
        u64 elapsed_counter = new_counter - old_counter;
        
        f32 ms_per_frame = (1000.0f * (f32)elapsed_counter) / perf_count_frequency;
        f32 frames_per_second = perf_count_frequency / (f32)elapsed_counter;
        old_counter = new_counter;
        
#if MOONBREATH_SLOW
        render_text("FPS: %.02f", V2u(25, 25), color_white, fonts[font_classic_outlined], frames_per_second);
        render_text("Frame: %.02fms", V2u(25, 50), color_white, fonts[font_classic_outlined], ms_per_frame);
        render_text("Update and Render: %.02fms", V2u(25, 75), color_white, fonts[font_classic_outlined], ms_for_work);
        render_text("DT Per Frame: %.02f", V2u(25, 100), color_white, fonts[font_classic_outlined], game.dt);
        
        render_text("Player Pos: %u, %u", V2u(25, 150), color_white, fonts[font_classic_outlined], player.pos.x, player.pos.y);
        //render_text("Player Pos x 32: %u, %u", V2u(25, 175), color_white, fonts[font_classic_outlined], tile_mul(player.pos.x), tile_mul(player.pos.y));
#endif
        
        SDL_RenderPresent(game.renderer);
    }
}

internal void
exit_game()
{
    free_assets();
    
    if(game.renderer)
    {
        SDL_DestroyRenderer(game.renderer);
        game.renderer = 0;
    }
    
    if(game.window)
    {
        SDL_DestroyWindow(game.window);
        game.window = 0;
    }
    
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

int
main(int argc, char *argv[])
{
    if(init_game())
    {
        run_game();
        exit_game();
        return(EXIT_SUCCESS);
    }
    
    exit_game();
    return(EXIT_FAILURE);
}