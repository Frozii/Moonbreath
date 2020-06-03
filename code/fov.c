internal void
set_tile_is_seen(dungeon_t *dungeon, v2u pos, b32 value)
{
    dungeon->fov_tiles[(pos.y * dungeon->w) + pos.x].is_seen = value;
}

internal void
set_tile_has_been_seen(dungeon_t *dungeon, v2u pos, b32 value)
{
    dungeon->fov_tiles[(pos.y * dungeon->w) + pos.x].has_been_seen = value;
}

internal b32
tile_is_seen(dungeon_t *dungeon, v2u pos)
{
    b32 result = (dungeon->fov_tiles[(pos.y * dungeon->w) + pos.x].is_seen);
    return(result);
}

internal b32
tile_has_been_seen(dungeon_t *dungeon, v2u pos)
{
    b32 result = (dungeon->fov_tiles[(pos.y * dungeon->w) + pos.x].has_been_seen);
    return(result);
}

internal void
set_tile_is_seen_and_has_been_seen(dungeon_t *dungeon, v2u pos, b32 value)
{
    set_tile_is_seen(dungeon, pos, value);
    set_tile_has_been_seen(dungeon, pos, value);
}

internal void
cast_light(dungeon_t *dungeon,
           v2u start,
           u32 fov_range,
           u32 row,
           f32 start_slope,
           f32 end_slope,
           v2u multiplier_x,
           v2u multiplier_y)
{
    if(start_slope >= end_slope)
    {
        f32 next_start_slope = start_slope;
        
        for(u32 y = row; y <= fov_range; ++y)
        {
            b32 is_current_blocked =  false;
            
            for(s32 dx = -y; dx <= 0; ++dx)
            {
                s32 dy = -y;
                
                // left_slope and right_slope store the
                // extremities of the square.
                f32 left_slope = (dx - 0.5f) / (dy + 0.5f);
                f32 right_slope = (dx + 0.5f) / (dy - 0.5f);
                
                if(start_slope < right_slope)
                {
                    continue;
                }
                else if(end_slope > left_slope)
                {
                    break;
                }
                
                // Get the offset for the current position in the
                // current sector by using the multipliers.
                v2s offset =
                {
                    (dx * multiplier_x.x) + (dy * multiplier_x.y),
                    (dx * multiplier_y.x) + (dy * multiplier_y.y)
                };
                
                v2u current = {start.x + offset.x, start.y + offset.y};
                if(is_pos_in_dungeon(dungeon, current))
                {
                    set_tile_is_seen_and_has_been_seen(dungeon, current, true);
                    
                    if(is_current_blocked)
                    {
                        if(is_tile_traversable(dungeon->tiles, current))
                        {
                            is_current_blocked = false;
                            start_slope = next_start_slope;
                        }
                        else
                        {
                            next_start_slope = right_slope;
                            continue;
                        }
                    }
                    else if(!is_tile_traversable(dungeon->tiles, current))
                    {
                        is_current_blocked = true;
                        next_start_slope = right_slope;
                        
                        // This position is blocking,
                        // start a child scan.
                        cast_light(dungeon,
                                   start,
                                   fov_range,
                                   y + 1,
                                   start_slope,
                                   left_slope,
                                   multiplier_x,
                                   multiplier_y);
                    }
                }
            }
            
            // Scan the next row unless if the current
            // rows last position is blocking.
            if(is_current_blocked)
            {
                break;
            }
        }
    }
}

internal void
update_fov(dungeon_t *dungeon, entity_t *player)
{
#if MOONBREATH_SLOW
    if(debug_fov)
    {
        for(u32 y = 0; y < dungeon->h; ++y)
        {
            for(u32 x = 0; x < dungeon->w; ++x)
            {
                set_tile_is_seen(dungeon, V2u(x, y), true);
            }
        }
    }
    
    else
#endif
    
    {
        // Reset visibility.
        for(u32 y = 0; y < dungeon->h; ++y)
        {
            for(u32 x = 0; x < dungeon->w; ++x)
            {
                set_tile_is_seen(dungeon, V2u(x, y), false);
            }
        }
        
        // Player is visible by default.
        set_tile_is_seen_and_has_been_seen(dungeon, player->pos, true);
        
        // For transforming positions into other sectors.
        s32 multipliers[4][8] =
        {
            {1, 0, 0, -1, -1, 0, 0, 1},
            {0, 1, -1, 0, 0, -1, 1, 0},
            {0, 1, 1, 0, 0, -1, -1, 0},
            {1, 0, 0, 1, -1, 0, 0, -1}
        };
        
        for(u32 sector = 0; sector < 8; ++sector)
        {
            v2u multiplier_x = {multipliers[0][sector], multipliers[1][sector]};
            v2u multiplier_y = {multipliers[2][sector], multipliers[3][sector]};
            
            cast_light(dungeon,
                       player->pos,
                       player->p.fov,
                       1,
                       1.0f,
                       0.0f,
                       multiplier_x,
                       multiplier_y);
        }
    }
}