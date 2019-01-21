#include <level_generation.h>

void fill_rect_with_tile(uint8 *level, int32 level_pitch, SDL_Rect rect, int32 fill_tile)
{
    for(int32 x = rect.x; x < rect.x + rect.w; x++)
    {
        for(int32 y = rect.y; y < rect.y + rect.h; y++)
        {
            level[(y * level_pitch) + x] = fill_tile;
        }
    }
}

int32 check_rect_for_tile(uint8 *level, int32 level_pitch, SDL_Rect rect, int32 check_tile)
{
    for(int32 x = rect.x; x < rect.x + rect.w; x++)
    {
        for(int32 y = rect.y; y < rect.y + rect.h; y++)
        {
            if(level[(y * level_pitch) + x] == check_tile)
            {
                return 1;
            }
        }
    }

    return 0;
}
 
int32 check_rect_is_only_tile(uint8 *level, int32 level_pitch, SDL_Rect rect, int32 check_tile)
{
    for(int32 x = rect.x; x < rect.x + rect.w; x++)
    {
        for(int32 y = rect.y; y < rect.y + rect.h; y++)
        {
            if(level[(y * level_pitch) + x] != check_tile)
            {
                return 0;
            }
        }
    }

    return 1;
}

// NOTE(Rami): room_count should be changed to feature count, since we add a certain amount of features.
// NOTE(Rami): We could generate rooms and bring them close enough to an existing room, then add a door and keep doing that until the level is finished.
void generate_level(uint8 *level, int32 level_pitch, int32 level_width, int32 level_height, int32 feature_count)
{
    // NOTE(Rami): 
    feature_count = 2;

    for(int32 i = 0; i < level_width * level_height; i++)
    {
        level[i] = TILE_WALL_STONE;
    }

    printf("\nFeature 0 is always a room and is set in the middle\n");
    SDL_Rect room = {level_width / 2, level_height / 2, int_between(4, 10), int_between(4, 10)};
    fill_rect_with_tile(level, level_pitch, room, TILE_FLOOR_STONE);
    place_player(room.x, room.y);

    for(int32 i = 1; i < feature_count; i++)
    {
        printf("\nFeature %d\n", i);

        for(;;)
        {
            int32 rand_feature_x = int_between(1, level_width - 2);
            int32 rand_feature_y = int_between(1, level_height - 2);
            int32 feature_direction = 0;
            if(level[(rand_feature_y * level_pitch) + rand_feature_x] == TILE_WALL_STONE)
            {
                if(level[(rand_feature_y * level_pitch) + (rand_feature_x - 1)] == TILE_FLOOR_STONE)
                {
                    feature_direction = 1;
                }
                else if(level[(rand_feature_y * level_pitch) + (rand_feature_x + 1)] == TILE_FLOOR_STONE)
                {
                    feature_direction = 2;
                }
                else if(level[((rand_feature_y - 1) * level_pitch) + rand_feature_x] == TILE_FLOOR_STONE)
                {
                    feature_direction = 3;
                }
                else if(level[((rand_feature_y + 1) * level_pitch) + rand_feature_x] == TILE_FLOOR_STONE)
                {
                    feature_direction = 4;
                }
            }

            if(!feature_direction)
            // if(!feature_direction || feature_direction == 2 || feature_direction == 3 || feature_direction == 4)
            {
                continue;
            }

            // int32 feature = int_between(2, 2);
            int32 feature = 2;
            if(feature == 1)
            {
                printf("Random feature chosen, Room\n");
            }
            else if(feature == 2)
            {
                printf("Random feature chosen, Corridor\n");
            }

            if(feature_direction == 1)
            {
                printf("Feature direction is Right\n");
            }
            else if(feature_direction == 2)
            {
                printf("Feature direction is Left\n");
            }
            else if(feature_direction == 3)
            {
                printf("Feature direction is Down\n");
            }
            else if(feature_direction == 4)
            {
                printf("Feature direction is Up\n");
            }

            if(feature == 1)
            {
                SDL_Rect room;
                room.w = int_between(ROOM_MIN_WIDTH, ROOM_MAX_WIDTH);
                room.h = int_between(ROOM_MIN_HEIGHT, ROOM_MAX_HEIGHT);

                // room.w = 5;
                // room.h = 5;

                if(feature_direction == 1)
                {
                    room.x = rand_feature_x + 1;
                    room.y = rand_feature_y - (room.h / 2);
                }
                else if(feature_direction == 2)
                {
                    room.x = rand_feature_x - room.w;
                    room.y = rand_feature_y - (room.h / 2);
                }
                else if(feature_direction == 3)
                {
                    room.x = rand_feature_x - (room.w / 2);
                    room.y = rand_feature_y + 1;
                }
                else if(feature_direction == 4)
                {
                    room.x = rand_feature_x - (room.w / 2);
                    room.y = rand_feature_y - room.h;
                }

                if(room.x > 0 && room.y > 0 && room.x + room.w < level_width && room.y + room.h < level_height &&
                check_rect_is_only_tile(level, level_pitch, (SDL_Rect){room.x - 1, room.y - 1, room.w + 2, room.h + 2}, TILE_WALL_STONE))
                {
                    printf("Room did fit\n");
                    level[(rand_feature_y * level_pitch) + rand_feature_x] = TILE_DOOR_CLOSED;
                    fill_rect_with_tile(level, level_pitch, room, TILE_FLOOR_STONE);
                    break;
                }

                printf("Room did not fit\n");
                continue;
            }
            else if(feature == 2)
            {
                SDL_Rect corridor;
                // NOTE(Rami): Have the corridor width and height be rand depending on direction

                if(feature_direction == 1)
                {
                    corridor.w = 4;
                    corridor.h = 1;
                    corridor.x = rand_feature_x + 1;
                    corridor.y = rand_feature_y;
                }
                else if(feature_direction == 2)
                {
                    corridor.w = 4;
                    corridor.h = 1;
                    corridor.x = rand_feature_x - corridor.w;
                    corridor.y = rand_feature_y;
                }
                else if(feature_direction == 3)
                {
                    corridor.w = 1;
                    corridor.h = 4;
                    corridor.x = rand_feature_x;
                    corridor.y = rand_feature_y + 1;
                }
                else if(feature_direction == 4)
                {
                    corridor.w = 1;
                    corridor.h = 4;
                    corridor.x = rand_feature_x;
                    corridor.y = rand_feature_y - corridor.h;
                }

                // NOTE(Rami): Does the corridor need any extended checking for check_rect_is_only_tile??
                if(corridor.x > 0 && corridor.y > 0 && corridor.x + corridor.w < level_width && corridor.y + corridor.h < level_height &&
                check_rect_is_only_tile(level, level_pitch, (SDL_Rect){corridor.x, corridor.y, corridor.w, corridor.h}, TILE_WALL_STONE))
                {
                    printf("Corridor did fit\n");
                    level[(rand_feature_y * level_pitch) + rand_feature_x] = TILE_DOOR_CLOSED;
                    fill_rect_with_tile(level, level_pitch, corridor, TILE_FLOOR_STONE);
                    break;
                }

                printf("Corridor did not fit\n");
                continue;
            }
        }
    }

    // init_and_place_rooms(level, level_pitch, level_width, level_height, room_count, rooms);
    // connect_rooms(level, level_pitch, room_count, rooms);

    // int32 spawn_room;
    // SDL_Rect room;

    // for(;;)
    // {
    //     spawn_room = int_between(0, room_count - 1);
    //     room = rooms[spawn_room];

    //     int32 rand_room_x = int_between(room.x + 1, room.x + (room.w - 2));
    //     int32 rand_room_y = int_between(room.y + 1, room.y + (room.h - 2));

    //     printf("rand_room_x: %d\n", rand_room_x);
    //     printf("rand_room_y: %d\n\n", rand_room_y);

    //     place_tile(level, level_pitch, rand_room_x, rand_room_y, TILE_PATH_UP);

    //     place_player(rand_room_x, rand_room_y);

    //     int32 offset = int_between(0, 3);

    //     if(offset == 0)
    //     {
    //         place_player(rand_room_x, rand_room_y - 1);
    //     }
    //     else if(offset == 1)
    //     {
    //         place_player(rand_room_x, rand_room_y + 1);
    //     }
    //     else if(offset == 2)
    //     {
    //         place_player(rand_room_x - 1, rand_room_y);
    //     }
    //     else
    //     {
    //         place_player(rand_room_x + 1, rand_room_y);
    //     }
        
    //     break;
    // }

    // for(;;)
    // {
    //     int32 next_level_room = int_between(0, room_count - 1);
    //     room = rooms[next_level_room];

    //     if(next_level_room != spawn_room)
    //     {
    //         int32 rand_room_x = int_between(room.x + 1, room.x + (room.w - 2));
    //         int32 rand_room_y = int_between(room.y + 1, room.y + (room.h - 2));

    //         place_tile(level, level_pitch, rand_room_x, rand_room_y, TILE_PATH_DOWN);
    //         break;
    //     }
    // }
}

void init_and_place_rooms(uint8 *level, int32 level_pitch, int32 level_width, int32 level_height, int32 room_count, SDL_Rect *rooms)
{
    // clear level data
    for(int32 i = 0; i < level_width * level_height; i++)
    {
        level[i] = TILE_WALL_STONE;
    }

    // iterate over the rooms
    SDL_Rect temp;
    for(int32 i = 0; i < room_count; i++)
    {
        // keep attempting to generate room data until it's valid
        for(;;)
        {
            // generate new room data
            temp.w = int_between(3, 10);
            temp.h = int_between(3, 10);
            temp.x = int_between(1, level_width - temp.w - 1);
            temp.y = int_between(1, level_height - temp.h - 1);

            // test if the new data is valid
            if(is_room_valid(level, level_pitch, temp))
            {
                // copy the valid data to the room
                // handle next room
                rooms[i] = temp;
                break;
            }
        }
    }
}

void connect_rooms(uint8 *level, int32 level_pitch, int32 room_count, SDL_Rect *rooms)
{
    // connect rooms
    for(int32 i = 1; i < room_count; i++)
    {
        pos_t room_a_cell = rand_pos_in_rect(rooms[i - 1]);
        pos_t room_b_cell = rand_pos_in_rect(rooms[i]);

        // get the direction towards the next room
        int32 sx = room_a_cell.x < room_b_cell.x ? 1 : -1;
        int32 sy = room_a_cell.y < room_b_cell.y ? 1 : -1;

        // create x-axis part of the corridor
        for (int32 x = room_a_cell.x; x != room_b_cell.x; x += sx)
        {
            level[(room_a_cell.y * level_pitch) + x] = TILE_FLOOR_STONE;
        }

        // create y-axis part of the corridor
        for (int32 y = room_a_cell.y; y != room_b_cell.y; y += sy)
        {
            level[(y * level_pitch) + room_b_cell.x] = TILE_FLOOR_STONE;
        }
    }
}

int32 is_room_valid(uint8 *level, int32 level_pitch, SDL_Rect room)
{
    // check if any of the cells for the room are already occupied
    // the first offset is so that we can't generate rooms that are directly next to eachother
    for(int32 temp_y = room.y - 1; temp_y < room.y + room.h + 1; temp_y++)
    {
        for(int32 temp_x = room.x - 1; temp_x < room.x + room.w + 1; temp_x++)
        {
            // if the cell is not a wall then the cell is occupied
            if(level[(temp_y * level_pitch) + temp_x] != TILE_WALL_STONE)
            {
                // room was not valid so return 0
                return 0;
            }
        }
    }

    // if we get this far then the cells for the room weren't occupied
    for(int32 temp_y = (room.y - 1); temp_y < (room.y + room.h + 1); temp_y++)
    {
        for(int32 temp_x = (room.x - 1); temp_x < (room.x + room.w + 1); temp_x++)
        {
            if(temp_y == (room.y - 1) || temp_y == (room.y + room.h) || temp_x == (room.x - 1) || temp_x == (room.x + room.w))
            {
                level[(temp_y * level_pitch) + temp_x] = TILE_WALL_STONE;
            }
            else
            {
                level[(temp_y * level_pitch) + temp_x] = TILE_FLOOR_STONE;
            }
        }
    }

    // room was valid so return 1
    return 1;
}

pos_t rand_pos_in_rect(SDL_Rect room)
{
    pos_t temp;

    // decrement by one so the chosen cell is not outside the room
    temp.x = int_between(room.x, room.x + (room.w - 1));
    temp.y = int_between(room.y, room.y + (room.h - 1));
    return temp;
}

void place_tile(uint8 *level, int32 level_pitch, int32 x, int32 y, int32 tile)
{
    level[(y * level_pitch) + x] = tile;
}