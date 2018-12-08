#ifndef UPDATE_H
#define UPDATE_H

#include <game.h>

void add_console_msg(char *msg, int msg_color, ...);
void update_input(char *level);
void update_events();
void update_camera(SDL_Rect *camera);

// NOTE(Rami): add later
//void update_lighting(char *level, char *fov, player_t *player);

#endif // UPDATE_H