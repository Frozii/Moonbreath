internal b32
is_pos_in_shadow(f32 pos_slope, shadow_data_t *data)
{
    b32 result = false;
    
    for(u32 i = 0; i < data->shadow_count; ++i)
    {
        shadow_t shadow = data->shadows[i];
        if(shadow.start <= pos_slope && shadow.end >= pos_slope)
        {
            result = true;
            break;
        }
    }
    
    return(result);
}

internal v2u
get_tile_pos_from_local_pos(u32 sector, v2u player, v2u local_pos)
{
    v2u result = {0};
    
    switch(sector)
    {
        case 0: result = V2u(player.x + local_pos.x, player.y - local_pos.y); break;
        case 1: result = V2u(player.x + local_pos.y, player.y - local_pos.x); break;
        case 2: result = V2u(player.x + local_pos.y, player.y + local_pos.x); break;
        case 3: result = V2u(player.x + local_pos.x, player.y + local_pos.y); break;
        case 4: result = V2u(player.x - local_pos.x, player.y + local_pos.y); break;
        case 5: result = V2u(player.x - local_pos.y, player.y + local_pos.x); break;
        case 6: result = V2u(player.x - local_pos.y, player.y - local_pos.x); break;
        case 7: result = V2u(player.x - local_pos.x, player.y - local_pos.y); break;
        
        invalid_default_case;
    }
    
    return(result);
}

internal void
add_shadow(shadow_t shadow, shadow_data_t *data)
{
    assert(data->shadow_count < array_count(data->shadows));
    data->shadows[data->shadow_count++] = shadow;
}

internal void
set_as_visible(v2u pos)
{
    dungeon.fov_tiles[(pos.y * dungeon.w) + pos.x].is_seen = true;
    dungeon.fov_tiles[(pos.y * dungeon.w) + pos.x].value = 1;
}

internal void
update_fov()
{
#if MOONBREATH_SLOW
    if(debug_fov)
    {
        for(u32 y = 0; y < dungeon.h; ++y)
        {
            for(u32 x = 0; x < dungeon.w; ++x)
            {
                dungeon.fov_tiles[(y * dungeon.w) + x].value = 1;
            }
        }
    }
    
    else
#endif
    
    {
        for(u32 y = 0; y < dungeon.h; ++y)
        {
            for(u32 x = 0; x < dungeon.w; ++x)
            {
                dungeon.fov_tiles[(y * dungeon.w) + x].value = 0;
            }
        }
        
        set_as_visible(player->pos);
        
        for(u32 sector = 0; sector < 8; ++sector)
        {
            f32 shadow_start = 0.0f;
            f32 shadow_end = 0.0f;
            shadow_data_t shadow_data = {0};
            
            v2u pos = {0};
            for(pos.y = 0; pos.y < player->fov; ++pos.y)
            {
                b32 is_previous_traversable = true;
                
                for(pos.x = 0; pos.x <= pos.y; ++pos.x)
                {
                    v2u tile_pos = get_tile_pos_from_local_pos(sector, player->pos, pos);
                    if(is_inside_dungeon(tile_pos))
                    {
                        f32 pos_slope = slope(0, 0, pos.x, pos.y);
                        if(!is_pos_in_shadow(pos_slope, &shadow_data))
                        {
                            set_as_visible(tile_pos);
                            
                            if(is_dungeon_traversable(tile_pos))
                            {
                                if(!is_previous_traversable)
                                {
                                    shadow_end = slope(0.0f, 0.0f, pos.x + 0.5f, pos.y);
                                    shadow_t shadow = {shadow_start, shadow_end};
                                    add_shadow(shadow, &shadow_data);
                                }
                            }
                            else
                            {
                                if(is_previous_traversable)
                                {
                                    shadow_start = slope(0, 0, pos.x, pos.y);
                                    is_previous_traversable = false;
                                }
                            }
                        }
                    }
                }
                
                if(!is_previous_traversable)
                {
                    shadow_end = slope(0.0f, 0.0f, pos.y + 0.5f, pos.y);
                    shadow_t shadow = {shadow_start, shadow_end};
                    add_shadow(shadow, &shadow_data);
                }
            }
        }
    }
}

internal void
set_seen(v2u pos, u32 value)
{
    dungeon.fov_tiles[(pos.y * dungeon.w) + pos.x].is_seen = value;
}

internal b32
is_seen(v2u pos)
{
    b32 result = (dungeon.fov_tiles[(pos.y * dungeon.w) + pos.x].value);
    return(result);
}

internal b32
has_been_seen(v2u pos)
{
    b32 result = (dungeon.fov_tiles[(pos.y * dungeon.w) + pos.x].is_seen);
    return(result);
}
