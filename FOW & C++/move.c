#include "move.h"

#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "dungeon.h"
#include "heap.h"
#include "move.h"
#include "npc.h"
#include "pc.h"
#include "character.h"
#include "utils.h"
#include "path.h"
#include "io.h"

void do_combat(dungeon_t *d, character_t *atk, character_t *def)
{
  set_alive(def, 0);
  if (def != d->pc) {
    d->num_monsters--;
  }
}

void move_character(dungeon_t *d, character_t *c, pair_t next)
{
  if (charpair(next) &&
      ((next[dim_y] != get_char_y(c)) ||
       (next[dim_x] != get_char_x(c)))) {
    do_combat(d, c, charpair(next));
  }
  /* No character in new position. */

  d->character[get_char_y(c)][get_char_x(c)] = NULL;
  set_char_y(c, next[dim_y]);
  set_char_x(c, next[dim_x]);
  d->character[get_char_y(c)][get_char_x(c)] = c;
}

void do_moves(dungeon_t *d)
{
  pair_t next;
  character_t *c;

  /* Remove the PC when it is PC turn.  Replace on next call.  This allows *
   * use to completely uninit the heap when generating a new level without *
   * worrying about deleting the PC.                                       */

  if (pc_is_alive(d)) {
    heap_insert(&d->next_turn, d->pc);
  }

  while (pc_is_alive(d) && ((c = heap_remove_min(&d->next_turn)) != d->pc)) {
    if (!get_alive(c)) {
      if (d->character[get_char_y(c)][get_char_x(c)] == c) {
        d->character[get_char_y(c)][get_char_x(c)] = NULL;
      }
      if (c != d->pc) {
        character_delete(c);
      }
      continue;
    }

    set_next_turn(c, get_next_turn(c) + 1000/get_speed(c));

    npc_next_pos(d, c, next);
    move_character(d, c, next);

    heap_insert(&d->next_turn, c);
  }

  io_display_remembered(d);
  
  if (pc_is_alive(d) && c == d->pc) {
    set_next_turn(c, get_next_turn(c) + 1000/get_speed(c));
    io_handle_input(d);
  }
}

void dir_nearest_wall(dungeon_t *d, character_t *c, pair_t dir)
{
  dir[dim_x] = dir[dim_y] = 0;

  if (get_char_x(c) != 1 && get_char_x(c) != DUNGEON_X - 2) {
    dir[dim_x] = (get_char_x(c) > DUNGEON_X - get_char_x(c) ? 1 : -1);
  }
  if (get_char_y(c) != 1 && get_char_y(c) != DUNGEON_Y - 2) {
    dir[dim_y] = (get_char_y(c) > DUNGEON_Y - get_char_y(c) ? 1 : -1);
  }
}

uint32_t in_corner(dungeon_t *d, character_t *c)
{
  uint32_t num_immutable;

  num_immutable = 0;

  num_immutable += (mapxy(get_char_x(c) - 1,
                          get_char_y(c)    ) == ter_wall_immutable);
  num_immutable += (mapxy(get_char_x(c) + 1,
                          get_char_y(c)    ) == ter_wall_immutable);
  num_immutable += (mapxy(get_char_x(c)    ,
                          get_char_y(c) - 1) == ter_wall_immutable);
  num_immutable += (mapxy(get_char_x(c)    ,
                          get_char_y(c) + 1) == ter_wall_immutable);

  return num_immutable > 1;
}

static void new_dungeon_level(dungeon_t *d, uint32_t dir)
{
  /* Eventually up and down will be independantly meaningful. *
   * For now, simply generate a new dungeon.                  */

  switch (dir) {
  case '<':
  case '>':
    new_dungeon(d);
    init_remem_dungeon(d);
    update_remem_dungeon(d);
    break;
  default:
    break;
  }
}

uint32_t move_pc(dungeon_t *d, uint32_t dir)
{
  pair_t next;
  uint32_t was_stairs = 0;

  next[dim_y] = get_char_y(d->pc);
  next[dim_x] = get_char_x(d->pc);

  switch (dir) {
  case 1:
  case 2:
  case 3:
    next[dim_y]++;
    break;
  case 4:
  case 5:
  case 6:
    break;
  case 7:
  case 8:
  case 9:
    next[dim_y]--;
    break;
  }
  switch (dir) {
  case 1:
  case 4:
  case 7:
    next[dim_x]--;
    break;
  case 2:
  case 5:
  case 8:
    break;
  case 3:
  case 6:
  case 9:
    next[dim_x]++;
    break;
  case '<':
    if (d->map[get_char_y(d->pc)][get_char_x(d->pc)] == ter_stairs_up) {
      was_stairs = 1;
      new_dungeon_level(d, '<');
    }
    break;
  case '>':
    if (d->map[get_char_y(d->pc)][get_char_x(d->pc)] == ter_stairs_down) {
      was_stairs = 1;
      new_dungeon_level(d, '>');
    }
    break;
  }
  if (was_stairs) {
    return 0;
  }
  if ((dir != '>') && (dir != '<') && (mappair(next) >= ter_floor)) {
    move_character(d, d->pc, next);
    dijkstra(d);
    dijkstra_tunnel(d);

    return 0;
  }
  return 1;
}
