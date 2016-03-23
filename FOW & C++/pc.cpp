#include <stdlib.h>

#include "string.h"
#include "dungeon.h"
#include "pc.h"
#include "utils.h"
#include "move.h"
#include "path.h"

void pc_delete(character_t *pc)
{
  //loses a few bytes upon freeing the PC >>not sure how to fix this yet<<
  if (pc) {
    free(pc);
  }
}

uint32_t pc_is_alive(dungeon_t *d)
{
  return get_alive(d->pc);
}

void place_pc(dungeon_t *d)
{
  set_char_y(d->pc, rand_range(d->rooms->position[dim_y],
                                     (d->rooms->position[dim_y] +
                                      d->rooms->size[dim_y] - 1)));
  set_char_x(d->pc, rand_range(d->rooms->position[dim_x],
                                     (d->rooms->position[dim_x] +
                                      d->rooms->size[dim_x] - 1)));
}

void config_pc(dungeon_t *d)
{
  d->pc = (character_t*) malloc(sizeof (PC));
  memset(d->pc, 0, sizeof (*(d->pc)));
  set_symbol(d->pc, '@');

  place_pc(d);

  set_speed(d->pc, PC_SPEED);
  set_next_turn(d->pc, 0);
  set_alive(d->pc, 1);
  set_seqnum(d->pc, 0);
  d->character[get_char_y(d->pc)][get_char_x(d->pc)] = d->pc;
  dijkstra(d);
  dijkstra_tunnel(d);
}

uint32_t pc_next_pos(dungeon_t *d, pair_t dir)
{
  dir[dim_y] = dir[dim_x] = 0;

  /* Tunnel to the nearest dungeon corner, then move around in hopes *
   * of killing a couple of monsters before we die ourself.          */

  if (in_corner(d, d->pc)) {
    /*
    dir[dim_x] = (mapxy(d->pc.position[dim_x] - 1,
                        d->pc.position[dim_y]) ==
                  ter_wall_immutable) ? 1 : -1;
    */
    dir[dim_y] = (mapxy(get_char_x(d->pc),
                        get_char_y(d->pc) - 1) ==
                  ter_wall_immutable) ? 1 : -1;
  } else {
    dir_nearest_wall(d, d->pc, dir);
  }

  return 0;
}

void init_remem_dungeon(dungeon_t *d)
{
  pair_t p;

  for (p[dim_y] = 0; p[dim_y] < DUNGEON_Y; p[dim_y]++){
    for (p[dim_x] = 0; p[dim_x] < DUNGEON_X; p[dim_x]++){
      ((PC*) d->pc)->remembered_dungeon[p[dim_y]][p[dim_x]] = ter_wall;
    }
  }
}

void update_remem_dungeon(dungeon_t *d)
{
  pair_t p, pc;

  pc[dim_x] = get_char_x(d->pc);
  pc[dim_y] = get_char_y(d->pc);

  p[dim_x] = pc[dim_x] - 3;
  p[dim_y] = pc[dim_y] - 3;

  if (p[dim_x] < 0) {
    p[dim_x] = 0;
  }
  if (p[dim_y] < 0) {
    p[dim_y] = 0;
  }

  for (; p[dim_y] <= pc[dim_y] + 3 && p[dim_y] < DUNGEON_Y; p[dim_y]++){
    for (; p[dim_x] <= pc[dim_x] + 3 && p[dim_x] < DUNGEON_X; p[dim_x]++){
      if (can_see(d,pc,p)){
	((PC*) d->pc)->remembered_dungeon[p[dim_y]][p[dim_x]] = mappair(pc);
      }
    }
  }
}

char get_remem_dungeon(dungeon_t* d, uint8_t y, uint8_t x)
{
  x = x < DUNGEON_X ? x : DUNGEON_X - 1;
  y = y < DUNGEON_Y ? y : DUNGEON_Y - 1;

  switch (((PC*) d->pc)->remembered_dungeon[y][x]){

  case ter_wall:
  case ter_wall_immutable:
    return ' ';
    break;
  case ter_floor:
  case ter_floor_room:
    return '.';
    break;
  case ter_floor_hall:
    return '#';
    break;
  case ter_debug:
    return '*';
    break;
  case ter_stairs_up:
    return '<';
    break;
  case ter_stairs_down:
    return '>';
    break;

  default:
    return '0';
  }
}
