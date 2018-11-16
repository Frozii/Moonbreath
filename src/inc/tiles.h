#ifndef TILES_H
#define TILES_H

// NOTE(Rami): The amount of tiles in one row could be something like 10, and after that we move on to the next row.

// tile info
#define TILE_SIZE 32

typedef enum
{
  TILE_WALL_STONE = 0,
  TILE_FLOOR_GRASS,
  TILE_FLOOR_STONE,
  TILE_DOOR_CLOSED,
  TILE_DOOR_OPEN,
  TILE_PATH_UP,
  TILE_PATH_DOWN
} level_tiles_e;

#endif // TILES_H