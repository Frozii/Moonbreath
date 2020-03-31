internal void
render_tilemap()
{
    SDL_SetRenderTarget(game.renderer, textures.tilemap.tex);
    SDL_RenderClear(game.renderer);
    
    v4u render_area =
    {
        tile_div(game.camera.x),
        tile_div(game.camera.y),
        tile_div(game.camera.x + game.camera.w),
        tile_div(game.camera.y + game.camera.h)
    };
    
    // NOTE(rami): If the dungeon w/h is less than
    // the w/h of the camera we can clamp the render area
    // to the w/h of the dungeon.
    if(tile_mul(dungeon.w) < game.camera.w)
    {
        render_area.w = dungeon.w - 1;
    }
    
    if(tile_mul(dungeon.h) < game.camera.h)
    {
        render_area.h = dungeon.h - 1;
    }
    
#if 0
    printf("render_area.x: %d\n", render_area.x);
    printf("render_area.y: %d\n", render_area.y);
    printf("render_area.w: %d\n", render_area.w);
    printf("render_area.h: %d\n\n", render_area.h);
#endif
    
    u32 tileset_tile_width = tile_div(textures.tileset.w);
    
    for(u32 y = render_area.y; y <= render_area.h; ++y)
    {
        for(u32 x = render_area.x; x <= render_area.w; ++x)
        {
            v2u tilemap = {x, y};
            v2u tile = v2u_from_index(get_dungeon_tile(tilemap), tileset_tile_width);
            
            v4u src =
            {
                tile_mul(tile.x),
                tile_mul(tile.y),
                32,
                32
            };
            
            v4u dest =
            {
                tile_mul(tilemap.x),
                tile_mul(tilemap.y),
                32,
                32
            };
            
            if(is_seen(tilemap))
            {
                SDL_SetTextureColorMod(textures.tileset.tex, 255, 255, 255);
                SDL_RenderCopy(game.renderer, textures.tileset.tex, (SDL_Rect *)&src, (SDL_Rect *)&dest);
                
                u32 blood_value = get_dungeon_tile_blood(tilemap);
                if(blood_value)
                {
                    v2u blood_tile = v2u_from_index(blood_value, tileset_tile_width);
                    
                    v4u blood_src =
                    {
                        tile_mul(blood_tile.x),
                        tile_mul(blood_tile.y),
                        32,
                        32
                    };
                    
                    SDL_RenderCopy(game.renderer, textures.tileset.tex, (SDL_Rect *)&blood_src, (SDL_Rect *)&dest);
                }
            }
            else if(has_been_seen(tilemap))
            {
                SDL_SetTextureColorMod(textures.tileset.tex, 85, 85, 85);
                SDL_RenderCopy(game.renderer, textures.tileset.tex, (SDL_Rect *)&src, (SDL_Rect *)&dest);
            }
        }
    }
    
    SDL_SetRenderTarget(game.renderer, 0);
    
    v4u src = {game.camera.x, game.camera.y, game.camera.w, game.camera.h};
    v4u dest = {0, 0, game.camera.w, game.camera.h};
    SDL_RenderCopy(game.renderer, textures.tilemap.tex, (SDL_Rect *)&src, (SDL_Rect *)&dest);
}

internal void
render_text(char *text, u32 x, u32 y, v4f color, font_t *font, u32 wrap_width, ...)
{
    v4u int_color = f32_to_u32_color(color);
    SDL_SetTextureColorMod(font->atlas, int_color.r,int_color.g, int_color.b);
    SDL_SetTextureAlphaMod(font->atlas, int_color.a);
    
    u32 origin_x = x;
    u32 last_space_x = 0;
    char text_final[128] = {0};
    
    va_list arg_list;
    va_start(arg_list, wrap_width);
    vsnprintf(text_final, sizeof(text_final), text, arg_list);
    va_end(arg_list);
    
    char *at = text_final;
    while(*at)
    {
        u32 metric_index = *at - START_ASCII_GLYPH;
        
        v4u src = {font->metrics[metric_index].x, font->metrics[metric_index].y, font->metrics[metric_index].w, font->metrics[metric_index].h};
        v4u dest = {x, y, font->metrics[metric_index].w, font->metrics[metric_index].h};
        SDL_RenderCopy(game.renderer, font->atlas, (SDL_Rect *)&src, (SDL_Rect *)&dest);
        
        if(font->type == font_bmp)
        {
            x += font->shared_glyph_advance;
        }
        else
        {
            x += font->metrics[metric_index].glyph_advance;
        }
        
        if(wrap_width && *at == ' ')
        {
            last_space_x = x;
            if(last_space_x > wrap_width)
            {
                x = origin_x;
                y += font->size;
            }
        }
        
        ++at;
    }
}