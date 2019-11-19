internal void
render_tilemap()
{
    SDL_SetRenderTarget(game.renderer, textures[tex_tilemap].tex);
    SDL_RenderClear(game.renderer);
    
    for(u32 x = tile_div(game.camera.x); x <= tile_div(game.camera.x + game.camera.w); ++x)
    {
        for(u32 y = tile_div(game.camera.y); y <= tile_div(game.camera.y + game.camera.h); ++y)
        {
            v4u src = {tile_mul(dungeon.tiles[y][x].value), 0, 32, 32};
            v4u dest = {tile_mul(x), tile_mul(y), 32, 32};
            
            v2u pos = {x, y};
            if(is_seen(pos))
            {
                SDL_SetTextureAlphaMod(textures[tex_game_tileset].tex, 255);
                SDL_RenderCopy(game.renderer, textures[tex_game_tileset].tex, (SDL_Rect *)&src, (SDL_Rect *)&dest);
            }
            else if(has_been_seen(pos))
            {
                SDL_SetTextureAlphaMod(textures[tex_game_tileset].tex, 64);
                SDL_RenderCopy(game.renderer, textures[tex_game_tileset].tex, (SDL_Rect *)&src, (SDL_Rect *)&dest);
            }
        }
    }
    
    SDL_SetRenderTarget(game.renderer, 0);
    
    v4u src = {game.camera.x, game.camera.y, game.camera.w, game.camera.h};
    v4u dest = {0, 0, game.camera.w, game.camera.h};
    SDL_RenderCopy(game.renderer, textures[tex_tilemap].tex, (SDL_Rect *)&src, (SDL_Rect *)&dest);
}

internal void
render_text(char *str, v2u pos, v4f color, font_t *font, ...)
{
    v4u int_color = f32_to_u32_color(color);
    SDL_SetTextureColorMod(font->atlas, int_color.r,int_color.g, int_color.b);
    SDL_SetTextureAlphaMod(font->atlas, int_color.a);
    
    char str_final[128] = {0};
    
    va_list arg_list;
    va_start(arg_list, font);
    vsnprintf(str_final, sizeof(str_final), str, arg_list);
    va_end(arg_list);
    
    u32 origin_x = pos.x;
    char *at = str_final;
    
    while(*at)
    {
        u32 array_index = *at - START_ASCII_CHAR;
        
        if(*at == ' ')
        {
            ++at;
            pos.x += font->space_size;
        }
        
        // TODO(rami): Having += 16 won't do because
        // we have fonts that might not be 16 in size.
        // We should have size variable in every font
        // to be used here instead.
        else if(*at == '\n')
        {
            ++at;
            pos.x = origin_x;
            pos.y += 16;
        }
        
        // TODO(rami): Use this if it's needed in the future.
        /*else if(*at == '\\' && at[1] == 'n')
        {
            at += 2;
            pos.x = origin_x;
            pos.y += 16;
        }*/
        else
        {
            v4u src = {font->metrics[array_index].x, font->metrics[array_index].y, font->metrics[array_index].w, font->metrics[array_index].h};
            v4u dest = {pos.x, pos.y, font->metrics[array_index].w, font->metrics[array_index].h};
            SDL_RenderCopy(game.renderer, font->atlas, (SDL_Rect *)&src, (SDL_Rect *)&dest);
            
            if(font->shared_advance)
            {
                pos.x += font->shared_advance;
            }
            else
            {
                pos.x += font->metrics[array_index].unique_advance_in_px;
            }
            
            ++at;
        }
    }
}