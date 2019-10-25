internal void
init_player()
{
    player.w = 32;
    player.h = 32;
    strcpy(player.name, "Zerker");
    player.max_hp = 10;
    player.hp = 8;
    player.strength = 1;
    player.speed = 1;
    player.level = 1;
    player.fov = 5;
    
    player.sprite.idle_frame_count = 1;
    player.sprite.current_frame = player.sprite.idle_start_frame;
    player.sprite.idle_frame_duration = 600;
}

internal void
update_alignment_points(v2u pos)
{
    if(player.sprite.current_frame.x == 0)
    {
        player.head_ap = V2u(pos.x, pos.y - 7);
        player.body_ap = V2u(pos.x - 1, pos.y + 1);
        player.legs_ap = V2u(pos.x, pos.y + 9);
        player.feet_ap = V2u(pos.x, pos.y + 13);
        player.amulet_ap = pos;
        player.first_hand_ap = V2u(pos.x, pos.y + 6);
        
        if(!player.sprite_flip)
        {
            player.second_hand_ap = V2u(pos.x + 5, pos.y + 6);
        }
        else
        {
            player.second_hand_ap = V2u(pos.x - 5, pos.y + 6);
        }
    }
    else if(player.sprite.current_frame.x == 1)
    {
        player.head_ap = V2u(pos.x, pos.y - 9);
        player.body_ap = V2u(pos.x - 1, pos.y - 1);
        player.legs_ap = V2u(pos.x, pos.y + 7);
        player.feet_ap = V2u(pos.x, pos.y + 13);
        player.amulet_ap = V2u(pos.x, pos.y - 2);
        player.first_hand_ap = V2u(pos.x, pos.y + 4);
        
        if(!player.sprite_flip)
        {
            player.second_hand_ap = V2u(pos.x + 5, pos.y + 4);
        }
        else
        {
            player.second_hand_ap = V2u(pos.x - 5, pos.y + 4);
        }
    }
}

internal v2u
get_alignment_point_from_slot(item_slot slot)
{
    v2u result = {0};
    
    switch(slot)
    {
        case slot_head: result = player.head_ap; break;
        case slot_body: result = player.body_ap; break;
        case slot_legs: result = player.legs_ap; break;
        case slot_feet: result = player.feet_ap; break;
        case slot_amulet: result = player.amulet_ap; break;
        case slot_first_hand: result = player.first_hand_ap; break;
        case slot_second_hand: result = player.second_hand_ap; break;
        
        invalid_default_case;
    }
    
    return(result);
}

internal void
render_player()
{
    update_sprite(&player.sprite, state_idle);
    
    v2u pos = get_game_position(player.pos);
    update_alignment_points(pos);
    
    v4u src = V4u(tile_mul(player.sprite.current_frame.x), tile_mul(player.sprite.current_frame.y), player.w, player.h);
    v4u dest = V4u(pos.x, pos.y, player.w, player.h);
    SDL_RenderCopyEx(game.renderer, textures[tex_sprite_sheet].tex, (SDL_Rect *)&src, (SDL_Rect *)&dest, 0, 0, player.sprite_flip);
    
    if(!is_item_slot_occupied(slot_head))
    {
        v4u hair_src = V4u(0, 0, 32, 32);
        v4u hair_dest = V4u(player.head_ap.x, player.head_ap.y, 32, 32);
        SDL_RenderCopyEx(game.renderer, textures[tex_player_parts].tex, (SDL_Rect *)&hair_src, (SDL_Rect *)&hair_dest, 0, 0, player.sprite_flip);
    }
    
    { // Render Player Items
        for(u32 slot_index = 1;
            slot_index < slot_ring;
            ++slot_index)
        {
            for(u32 inventory_index = 0;
                inventory_index < array_count(inventory.slots);
                ++inventory_index)
            {
                if(inventory.slots[inventory_index].id)
                {
                    u32 item_info_index = get_inventory_info_index(inventory_index);
                    if(item_info[item_info_index].slot == slot_index &&
                       inventory.slots[inventory_index].id &&
                       inventory.slots[inventory_index].is_equipped)
                    {
                        v2u item_pos = get_alignment_point_from_slot(item_info[item_info_index].slot);
                        
                        v4u src = V4u(tile_mul(item_info[item_info_index].tile.x), tile_mul(item_info[item_info_index].tile.y), 32, 32);
                        v4u dest = V4u(item_pos.x, item_pos.y, 32, 32);
                        SDL_RenderCopyEx(game.renderer, textures[tex_wearable_item_tileset].tex, (SDL_Rect *)&src, (SDL_Rect *)&dest, 0, 0, player.sprite_flip);
                        
                        break;
                    }
                }
            }
        }
    }
}

internal void
player_keypress(SDL_Scancode key)
{
    // TODO(rami): Switches
    if(key == SDL_SCANCODE_Q)
    {
        game.state = state_quit;
    }
    // TODO(rami): Pop text testing
    else if(key == SDL_SCANCODE_Y)
    {
        add_pop_text("AAAAAAA", player.pos, text_normal_attack);
    }
    else if(key == SDL_SCANCODE_I)
    {
        inventory.is_open = !inventory.is_open;
        inventory.current_slot = V2u(0, 0);
        
        inventory.item_is_being_moved = false;
        inventory.moved_item_src_index = (u32)-1;
        inventory.moved_item_dest_index = (u32)-1;
    }
    else if(inventory.is_open)
    {
        if(key == SDL_SCANCODE_K)
        {
            if(inventory.current_slot.y > 0)
            {
                --inventory.current_slot.y;
            }
            else
            {
                inventory.current_slot.y = INVENTORY_HEIGHT - 1;
            }
        }
        else if(key == SDL_SCANCODE_J)
        {
            if((inventory.current_slot.y + 1) < INVENTORY_HEIGHT)
            {
                ++inventory.current_slot.y;
            }
            else
            {
                inventory.current_slot.y = 0;
            }
        }
        else if(key == SDL_SCANCODE_H)
        {
            if(inventory.current_slot.x > 0)
            {
                --inventory.current_slot.x;
            }
            else
            {
                inventory.current_slot.x = INVENTORY_WIDTH - 1;
            }
        }
        else if(key == SDL_SCANCODE_L)
        {
            if((inventory.current_slot.x + 1) < INVENTORY_WIDTH)
            {
                ++inventory.current_slot.x;
            }
            else
            {
                inventory.current_slot.x = 0;
            }
        }
        else if(key == SDL_SCANCODE_D)
        {
            remove_inventory_item(1);
        }
        else if(key == SDL_SCANCODE_E)
        {
            toggle_equipped_item();
        }
        else if(key == SDL_SCANCODE_C)
        {
            consume_item();
        }
        else if(key == SDL_SCANCODE_M)
        {
            if(inventory.item_is_being_moved)
            {
                inventory.moved_item_dest_index = get_inventory_pos_index();
                if(inventory.moved_item_src_index != inventory.moved_item_dest_index)
                {
                    move_item_in_inventory(inventory.moved_item_src_index, inventory.moved_item_dest_index);
                }
                
                inventory.item_is_being_moved = false;
                inventory.moved_item_src_index = (u32)-1;
                inventory.moved_item_dest_index = (u32)-1;
            }
            else
            {
                u32 index = get_inventory_pos_index();
                if(inventory.slots[index].id)
                {
                    inventory.item_is_being_moved = true;
                    inventory.moved_item_src_index = index;
                }
            }
        }
    }
    else if(!inventory.is_open)
    {
        if(key == SDL_SCANCODE_K)
        {
            player.new_pos = V2u(player.pos.x, player.pos.y - 1);
        }
        else if(key == SDL_SCANCODE_J)
        {
            player.new_pos = V2u(player.pos.x, player.pos.y + 1);
        }
        else if(key == SDL_SCANCODE_H)
        {
            player.new_pos = V2u(player.pos.x - 1, player.pos.y);
            player.sprite_flip = true;
        }
        else if(key == SDL_SCANCODE_L)
        {
            player.new_pos = V2u(player.pos.x + 1, player.pos.y);
            player.sprite_flip = false;
        }
        else if(key == SDL_SCANCODE_COMMA)
        {
            add_inventory_item();
        }
        else if(key == SDL_SCANCODE_D)
        {
            if(is_tile(player.pos, tile_stone_path_down))
            {
                ++dungeon.level;
                add_console_text("You descend further.. Level %u.", color_orange, dungeon.level);
                add_console_text("-----------------------------------------", color_orange);
                
                generate_dungeon();
            }
        }
        else if(key == SDL_SCANCODE_U)
        {
            if(is_tile(player.pos, tile_stone_path_up))
            {
                game.state = state_quit;
            }
        }
    }
    
    if(!inventory.is_open)
    {
        game.turn_changed = true;
    }
}

internal void
get_player_attack_message(char *message)
{
    u32 value = rand_num(1, 4);
    if(value == 1)
    {
        strcpy(message, "bash");
    }
    else if(value == 2)
    {
        strcpy(message, "slash");
    }
    else if(value == 3)
    {
        strcpy(message, "pierce");
    }
    else
    {
        strcpy(message, "smash");
    }
}

internal b32
heal_player(u32 amount)
{
    b32 was_healed = false;
    
    if(player.hp < player.max_hp)
    {
        was_healed = true;
        
        player.hp += amount;
        if(player.hp > player.max_hp)
        {
            player.hp = player.max_hp;
        }
        
        add_pop_text("%u", player.pos, text_heal, amount);
    }
    
    return(was_healed);
}

internal void
player_attack_monster()
{
    for(u32 i = 0; i < array_count(monsters); ++i)
    {
        monster_t *monster = &monsters[i];
        if(monster->type)
        {
            if(V2u_equal(player.new_pos, monster->pos))
            {
                if(monster->state)
                {
                    char attack[64] = {0};
                    get_player_attack_message(attack);
                    add_console_text("You %s the %s for %u damage.", color_white, attack, monster->name, player.strength);
                    add_pop_text("%u", monster->pos, text_normal_attack, player.strength);
                    
                    monster->hp -= player.strength;
                    if((s32)monster->hp <= 0)
                    {
                        add_console_text("You killed the %s!", color_red, monster->name);
                        set_monster_sprite_state(monster, state_dead);
                    }
                    else
                    {
                        monster->in_combat = true;
                    }
                }
                
                return;
            }
        }
    }
}

internal void
update_player()
{
    if(is_inside_dungeon(player.new_pos))
    {
#if 1
        set_occupied(player.pos, false);
        player.pos = player.new_pos;
        set_occupied(player.pos, true);
        
        return;
#endif
        
        if(is_traversable(player.new_pos))
        {
            if(is_occupied(player.new_pos))
            {
                // TODO(rami): If we have other entity types than monsters,
                // we'll have to know who we're trying to interact with here.
                player_attack_monster();
            }
            else
            {
                set_occupied(player.pos, false);
                player.pos = player.new_pos;
                set_occupied(player.pos, true);
            }
        }
        else
        {
            if(is_tile(player.new_pos, tile_stone_door_closed))
            {
                add_console_text("You open the door.", color_white);
                set_tile(player.new_pos, tile_stone_door_open);
            }
            else if(is_tile(player.new_pos, tile_stone_path_up))
            {
                add_console_text("A path to the surface, [A]scend to flee the mountain.", color_white);
            }
            else if(is_tile(player.new_pos, tile_stone_path_down))
            {
                add_console_text("A path that leads further downwards.. [D]escend?", color_white);
            }
        }
        
        ++game.turn;
    }
}
