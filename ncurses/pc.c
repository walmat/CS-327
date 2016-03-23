#include <stdlib.h>
#include "string.h"

#include "dungeon.h"
#include "pc.h"
#include "utils.h"
#include "move.h"
#include "path.h"

void pc_delete(pc_t *pc)
{
  if (pc) {
    free(pc);
  }
}

uint32_t pc_is_alive(dungeon_t *d)
{
  return d->pc.alive;
}

void place_pc(dungeon_t *d)
{
  d->pc.position[dim_y] = rand_range(d->rooms->position[dim_y],
                                     (d->rooms->position[dim_y] +
                                      d->rooms->size[dim_y] - 1));
  d->pc.position[dim_x] = rand_range(d->rooms->position[dim_x],
                                     (d->rooms->position[dim_x] +
                                      d->rooms->size[dim_x] - 1));
}

void config_pc(dungeon_t *d)
{
  memset(&d->pc, 0, sizeof (d->pc));
  d->pc.symbol = '@';
  place_pc(d);
  d->pc.speed = PC_SPEED;
  d->pc.next_turn = 0;
  d->pc.alive = 1;
  d->pc.sequence_number = 0;
  d->pc.pc = calloc(1, sizeof (*d->pc.pc));
  d->pc.npc = NULL;

  d->character[d->pc.position[dim_y]][d->pc.position[dim_x]] = &d->pc;

  dijkstra(d);
  dijkstra_tunnel(d);
}

uint32_t pc_next_pos(dungeon_t *d, pair_t dir)
{
  dir[dim_y] = dir[dim_x] = 0;

  /* Tunnel to the nearest dungeon corner, then move around in hopes *
   * of killing a couple of monsters before we die ourself.          */

  if (in_corner(d, &d->pc)) {
    /* 
       dir[dim_x] = (mapxy(d->pc.position[dim_x] - 1,
       d->pc.position[dim_y]) ==
       ter_wall_immutable) ? 1 : -1;
    */
    dir[dim_y] = (mapxy(d->pc.position[dim_x],
			d->pc.position[dim_y] - 1) ==
		  ter_wall_immutable) ? 1 : -1;
  } else {
    dir_nearest_wall(d, &d->pc, dir);
  }
  
  return 0;
}
