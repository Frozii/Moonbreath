#ifndef PLAYER_H
#define PLAYER_H

// NOTE(rami): Needed by item.c
internal i32 heal_player(i32 amount);

typedef struct
{
  sprite_t sprite;
  i32 new_x, new_y;
  i32 x, y, w, h;
  b32 flip;
  char name[16];
  i32 max_hp;
  i32 hp;
  i32 damage;
  i32 armor;
  i32 speed;
  i32 level;
  i32 money;
  i32 fov;
  i32 brightness;
} player_t;

#endif // PLAYER_H