internal u32
get_pathfind_value(u32 *map, u32 width, v2u pos)
{
    u32 result = map[(pos.y * width) + pos.x];
    return(result);
}

internal void
set_pathfind_value(u32 *map, u32 width, v2u pos, u32 value)
{
    map[(pos.y * width) + pos.x] = value;
}

#if 0
internal void
print_map(u32 *map, u32 width, u32 height)
{
    for(u32 y = 0; y < height; ++y)
    {
        for(u32 x = 0; x < width; ++x)
        {
            v2u current = {x, y};
            if(get_pathfind_value(map, width, current) != 1024)
            {
                printf("%u ", get_pathfind_value(map, width, current));
            }
        }
        
        printf("\n");
    }
}
#endif

internal void
update_pathfind_map(u32 *map, u32 width, u32 height)
{
    assert(width != 0 && height != 0);
    
    if(is_dungeon_traversable(player->pos))
    {
        u32 map_default_value = 1024;
        
        // NOTE(rami): Initialize to a high value.
        for(u32 y = 0; y < height; ++y)
        {
            for(u32 x = 0; x < width; ++x)
            {
                set_pathfind_value(map, width, V2u(x, y), map_default_value);
            }
        }
        
        // NOTE(rami): This is the lowest number, the goal.
        set_pathfind_value(map, width, player->pos, 0);
        
        for(;;)
        {
            next_iteration:
            
            for(u32 y = 0; y < height; ++y)
            {
                for(u32 x = 0; x < width; ++x)
                {
                    v2u current = {x, y};
                    
                    // TODO(rami): We need to be able to go through closed doors
                    // with this so we don't infinite loop. If we were to have
                    // different doors in the future, we would need something like
                    // a is_door() function to be used here instead.
                    if(is_dungeon_traversable(current) ||
                       is_dungeon_tile(current, tile_stone_door_closed))
                    {
                        u32 lowest_neighbour = get_pathfind_value(map, width, current);
                        
                        // NOTE(rami): Up
                        if(y >= 1)
                        {
                            v2u up = {x, y - 1};
                            if(get_pathfind_value(map, width, up) < lowest_neighbour)
                            {
                                lowest_neighbour = get_pathfind_value(map, width, up);
                            }
                        }
                        
                        // NOTE(rami): Down
                        if(y < (height - 1))
                        {
                            v2u down = {x, y + 1};
                            if(get_pathfind_value(map, width, down) < lowest_neighbour)
                            {
                                lowest_neighbour = get_pathfind_value(map, width, down);
                            }
                        }
                        
                        // NOTE(rami): Left
                        if(x >= 1)
                        {
                            v2u left = {x - 1, y};
                            if(get_pathfind_value(map, width, left) < lowest_neighbour)
                            {
                                lowest_neighbour = get_pathfind_value(map, width, left);
                            }
                        }
                        
                        // NOTE(rami): Right
                        if(x < (width - 1))
                        {
                            v2u right = {x + 1, y};
                            if(get_pathfind_value(map, width, right) < lowest_neighbour)
                            {
                                lowest_neighbour = get_pathfind_value(map, width, right);
                            }
                        }
                        
                        if(lowest_neighbour < map[(y * width) + x])
                        {
                            set_pathfind_value(map, width, current, lowest_neighbour + 1);
                        }
                    }
                }
            }
            
#if 0
            printf("\n\nResult Map\n");
            print_map(map, width, height);
#endif
            
            for(u32 y = 0; y < height; ++y)
            {
                for(u32 x = 0; x < width; ++x)
                {
                    v2u current = {x, y};
                    if(is_dungeon_traversable(current) && get_pathfind_value(map, width, current) == map_default_value)
                    {
                        goto next_iteration;
                    }
                }
            }
            
            break;
        }
    }
}

internal v2u
next_pathfind_pos(u32 *map, u32 width, entity_t *enemy)
{
    v2u result = {0};
    
    u32 lowest_distance = get_pathfind_value(map, width, enemy->pos);
    v2u pos = {0, 0};
    
    for(u32 dir = dir_up; dir <= dir_down_right; ++dir)
    {
        switch(dir)
        {
            case dir_up: pos = V2u(enemy->pos.x, enemy->pos.y - 1); break;
            case dir_down: pos = V2u(enemy->pos.x, enemy->pos.y + 1); break;
            case dir_left: pos = V2u(enemy->pos.x - 1, enemy->pos.y); break;
            case dir_right: pos = V2u(enemy->pos.x + 1, enemy->pos.y); break;
            
            case dir_up_left: pos = V2u(enemy->pos.x - 1, enemy->pos.y - 1); break;
            case dir_up_right: pos = V2u(enemy->pos.x + 1, enemy->pos.y - 1); break;
            case dir_down_left: pos = V2u(enemy->pos.x - 1, enemy->pos.y + 1); break;
            case dir_down_right: pos = V2u(enemy->pos.x + 1, enemy->pos.y + 1); break;
            
            invalid_default_case;
        }
        
        u32 pos_distance = get_pathfind_value(map, width, pos);
        if(pos_distance <= lowest_distance && (!is_dungeon_occupied(pos) || V2u_equal(pos, player->pos)))
        {
            lowest_distance = pos_distance;
            result = pos;
        }
    }
    
    return(result);
}

// TODO(rami): (Can we) Make the node count even lower?
#define NODE_COUNT 128

typedef enum
{
    pathfind_cardinal,
    pathfind_cardinal_and_ordinal
} pathfind_type;

typedef enum
{
    cardinal_cost = 10,
    cardinal_and_ordinal_cost = 14
} pathfind_cost;

typedef struct
{
    b32 active;
    v2u parent_pos;
    v2u pos;
    u32 g;
    u32 h;
    u32 f;
} node_t;

typedef struct
{
    node_t open[NODE_COUNT];
    node_t closed[NODE_COUNT];
} lists_t;

typedef struct
{
    b32 was_found;
    u32 list_length;
    v2u list[NODE_COUNT];
} path_t;

internal void
move_open_node_to_closed(lists_t *lists, v2u pos)
{
    node_t node_to_move = {0};
    
    for(u32 i = 0; i < NODE_COUNT; ++i)
    {
        if(lists->open[i].active && V2u_equal(lists->open[i].pos, pos))
        {
            node_to_move = lists->open[i];
            memset(&lists->open[i], 0, sizeof(node_t));
            break;
        }
    }
    
    for(u32 i = 0; i < NODE_COUNT; ++i)
    {
        if(!lists->closed[i].active)
        {
            lists->closed[i] = node_to_move;
            break;
        }
    }
}

internal void
add_open_node(node_t *open_list, v2u pos, v2u parent_pos, u32 g, v2u end, pathfind_type type)
{
    for(u32 i = 0; i < NODE_COUNT; ++i)
    {
        if(!open_list[i].active)
        {
            open_list[i].active = true;
            open_list[i].parent_pos = parent_pos;
            open_list[i].pos = pos;
            open_list[i].g = g;
            open_list[i].h = tile_dist_cardinal_and_ordinal(pos, end) * cardinal_and_ordinal_cost;
            open_list[i].f = open_list[i].g + open_list[i].h;
            return;
        }
    }
}

internal b32
in_list(node_t *closed_list, v2u pos)
{
    b32 result = false;
    
    for(u32 i = 0; i < NODE_COUNT; ++i)
    {
        if(V2u_equal(closed_list[i].pos, pos))
        {
            result = true;
            break;
        }
    }
    
    return(result);
}

internal node_t
find_node(node_t *list, v2u pos)
{
    node_t result = {0};
    
    for(u32 i = 0; i < NODE_COUNT; ++i)
    {
        if(list[i].active && V2u_equal(list[i].pos, pos))
        {
            result = list[i];
            break;
        }
    }
    
    return(result);
}

internal node_t
find_best_node(node_t *list)
{
    node_t result = {0};
    
    for(u32 i = 0; i < NODE_COUNT; ++i)
    {
        if(list[i].active)
        {
            if(result.f == 0)
            {
                result = list[i];
            }
            else if(list[i].f < result.f)
            {
                result = list[i];
            }
        }
    }
    
    return(result);
}

internal void
check_adjacent_nodes(lists_t *lists, v2u pos, v2u end, pathfind_type type)
{
    node_t current_node = find_node(lists->closed, pos);
    
    u32 direction_limit = dir_right;
    if(type == pathfind_cardinal_and_ordinal)
    {
        direction_limit = dir_down_right;
    }
    
    for(u32 i = dir_up; i <= direction_limit; ++i)
    {
        v2u new_pos = {0};
        u32 cost = 0;
        
        if(i == dir_up)
        {
            new_pos = V2u(pos.x, pos.y - 1);
            cost = cardinal_cost;
        }
        else if(i == dir_down)
        {
            new_pos = V2u(pos.x, pos.y + 1);
            cost = cardinal_cost;
        }
        else if(i == dir_left)
        {
            new_pos = V2u(pos.x - 1, pos.y);
            cost = cardinal_cost;
        }
        else if(i == dir_right)
        {
            new_pos = V2u(pos.x + 1, pos.y);
            cost = cardinal_cost;
        }
        
        if(type == pathfind_cardinal_and_ordinal)
        {
            if(i == dir_up_left)
            {
                new_pos = V2u(pos.x - 1, pos.y - 1);
                cost = cardinal_and_ordinal_cost;
            }
            else if(i == dir_up_right)
            {
                new_pos = V2u(pos.x + 1, pos.y - 1);
                cost = cardinal_and_ordinal_cost;
            }
            else if(i == dir_down_left)
            {
                new_pos = V2u(pos.x - 1, pos.y + 1);
                cost = cardinal_and_ordinal_cost;
            }
            else if(i == dir_down_right)
            {
                new_pos = V2u(pos.x + 1, pos.y + 1);
                cost = cardinal_and_ordinal_cost;
            }
        }
        
        if(is_dungeon_traversable(new_pos) && !in_list(lists->closed, new_pos))
        {
            if(in_list(lists->open, new_pos))
            {
                node_t dir_node = find_node(lists->open, new_pos);
                
                if(current_node.g + cost < dir_node.g)
                {
                    dir_node.parent_pos = current_node.pos;
                    dir_node.g = current_node.g + cost;
                    dir_node.f = dir_node.g + dir_node.h;
                }
            }
            else
            {
                add_open_node(lists->open, new_pos, pos, cost, end, type);
            }
        }
    }
}

internal void
set_path_list(path_t *path, node_t *closed_list, v2u start, v2u end)
{
    u32 length = 0;
    
    node_t current = find_node(closed_list, end);
    for(u32 i = 0; i < NODE_COUNT; ++i)
    {
        if(V2u_equal(current.pos, start))
        {
            break;
        }
        
        ++length;
        current = find_node(closed_list, current.parent_pos);
    }
    
    path->list_length = length;
    
    current = find_node(closed_list, end);
    for(s32 i = length - 1; i >= 0; --i)
    {
        if(V2u_equal(current.pos, start))
        {
            break;
        }
        else
        {
            path->list[i] = current.pos;
            current = find_node(closed_list, current.parent_pos);
        }
    }
}

internal path_t *
pathfind(v2u start, v2u end, pathfind_type type)
{
    path_t *path = calloc(1, sizeof(path_t));
    lists_t *lists = calloc(1, sizeof(lists_t));
    
    if(path && lists)
    {
        add_open_node(lists->open, start, start, 0, end, type);
        
        for(u32 i = 0; i < NODE_COUNT; ++i)
        {
            node_t current_node = find_best_node(lists->open);
            if(current_node.active)
            {
                move_open_node_to_closed(lists, current_node.pos);
                check_adjacent_nodes(lists, current_node.pos, end, type);
                
                if(in_list(lists->closed, end))
                {
                    path->was_found = true;
                    set_path_list(path, lists->closed, start, end);
                    break;
                }
            }
            else
            {
                // NOTE(rami): We can't find any new nodes so break early.
                break;
            }
        }
    }
    
    free(lists);
    return(path);
}

#if 0
printf("\n-OPEN LIST-\n\n");
for(u32 i = 0; i < NODE_COUNT; ++i)
{
    if(lists->open[i].active)
    {
        printf("Node %u\n", i);
        printf("parent.x: %u\n", lists->open[i].parent_pos.x);
        printf("parent.y: %u\n", lists->open[i].parent_pos.y);
        printf("pos.x: %u\n", lists->open[i].pos.x);
        printf("pos.y: %u\n", lists->open[i].pos.y);
        printf("g: %u\n", lists->open[i].g);
        printf("h: %u\n", lists->open[i].h);
        printf("f: %u\n\n", lists->open[i].f);
    }
}

printf("\n-CLOSED LIST-\n\n");
for(u32 i = 0; i < NODE_COUNT; ++i)
{
    if(lists->closed[i].active)
    {
        printf("Node %u\n", i);
        printf("parent.x: %u\n", lists->closed[i].parent_pos.x);
        printf("parent.y: %u\n", lists->closed[i].parent_pos.y);
        printf("pos.x: %u\n", lists->closed[i].pos.x);
        printf("pos.y: %u\n", lists->closed[i].pos.y);
        printf("g: %u\n", lists->closed[i].g);
        printf("h: %u\n", lists->closed[i].h);
        printf("f: %u\n\n", lists->closed[i].f);
    }
}
#endif