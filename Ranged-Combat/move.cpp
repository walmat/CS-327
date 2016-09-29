#include "move.h"

#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <ncurses.h>

#include "dungeon.h"
#include "heap.h"
#include "move.h"
#include "npc.h"
#include "pc.h"
#include "character.h"
#include "utils.h"
#include "path.h"
#include "io.h"

int32_t display_message = 0;

void do_combat(dungeon_t *d, character *atk, character *def)
{
  uint32_t damage, i;
  
  if (atk != d->the_pc) {
    damage = atk->damage->roll();
    io_queue_message("The %s hits you for %d.", atk->name, damage);
  } else {
    for (i = damage = 0; i < num_eq_slots; i++) {
      if (i == eq_slot_weapon && !d->the_pc->eq[i]) {
        damage += atk->damage->roll();
      } else if (d->the_pc->eq[i]) {
        damage += d->the_pc->eq[i]->roll_dice();
      }
    }
    io_queue_message("You hit the %s for %d.", def->name, damage);
  }

  if (damage >= def->hp) {
    if (atk != d->the_pc) {
      io_queue_message("You die.");
      io_queue_message(""); /* Extra message to force pause on "more" prompt */
    } else {
      io_queue_message("The %s dies.", def->name);
    }
    def->hp = 0;
    def->alive = 0;
    if (def != d->the_pc) {
      d->num_monsters--;
    }
    charpair(def->position) = NULL;
  } else {
    def->hp -= damage;
  }
}

void move_character(dungeon_t *d, character *c, pair_t next)
{
  pair_t displacement;
  uint32_t found_cell;
  pair_t order[9] = {
    { -1, -1 },
    { -1,  0 },
    { -1,  1 },
    {  0, -1 },
    {  0,  0 },
    {  0,  1 },
    {  1, -1 },
    {  1,  0 },
    {  1,  1 },
  };
  uint32_t s, i;

  if (charpair(next) &&
      ((next[dim_y] != c->position[dim_y]) ||
       (next[dim_x] != c->position[dim_x]))) {
    if ((charpair(next) == d->the_pc) ||
        c == d->the_pc) {
      do_combat(d, c, charpair(next));
    } else {
      /* Easiest way for a monster to displace another monster is *
       * to swap them.  This could lead to some strangeness where *
       * two monsters of the exact same speed continually         *
       * displace each other and never make progress, but I don't *
       * have any real problem with that.  When we have better    *
       * game balance, weaker monsters should not be able to      *
       * displace stronger monsters.                              */
      /* Turns out I don't like swapping them after all.  We'll   *
       * instead select a random square from the 8 surrounding    *
       * the target cell.  Keep doing it until either we swap or  *
       * find an empty one for the displacement.                  */
      for (s = rand() % 9, found_cell = i = 0;
           i < 9 && !found_cell; i++) {
        displacement[dim_y] = next[dim_y] + order[s % 9][dim_y];
        displacement[dim_x] = next[dim_x] + order[s % 9][dim_x];
        if (((npc *) charpair(next))->characteristics & NPC_PASS_WALL) {
          if (!charpair(displacement) ||
              (charpair(displacement) == c)) {
            found_cell = 1;
          }
        } else {
          if ((!charpair(displacement) &&
               (mappair(displacement) >= ter_floor)) ||
              (charpair(displacement) == c)) {
            found_cell = 1;
          }
        }
      }

      if (!found_cell) {
        return;
      }

      assert(charpair(next));

      charpair(c->position) = NULL;
      charpair(displacement) = charpair(next);
      charpair(next) = c;
      charpair(displacement)->position[dim_y] = displacement[dim_y];
      charpair(displacement)->position[dim_x] = displacement[dim_x];
      c->position[dim_y] = next[dim_y];
      c->position[dim_x] = next[dim_x];
    }
  } else {
    /* No character in new position. */

    d->charmap[c->position[dim_y]][c->position[dim_x]] = NULL;
    c->position[dim_y] = next[dim_y];
    c->position[dim_x] = next[dim_x];
    d->charmap[c->position[dim_y]][c->position[dim_x]] = c;
  }

  if (c == d->the_pc) {
    pc_reset_visibility(c);
    pc_observe_terrain(c, d);
  }
}

void do_moves(dungeon_t *d)
{
  pair_t next;
  character *c;

  /* Remove the PC when it is PC turn.  Replace on next call.  This allows *
   * use to completely uninit the heap when generating a new level without *
   * worrying about deleting the PC.                                       */

  if (pc_is_alive(d)) {
    heap_insert(&d->next_turn, d->the_pc);
  }

  while (pc_is_alive(d) && ((c = ((character *)
                                  heap_remove_min(&d->next_turn))) != d->the_pc)) {
    if (!character_is_alive(c)) {
      if (d->charmap[character_get_y(c)][character_get_x(c)] == c) {
        d->charmap[character_get_y(c)][character_get_x(c)] = NULL;
      }
      if (c != d->the_pc) {
        character_delete(c);
      }
      continue;
    }

    character_next_turn(c);

    npc_next_pos(d, c, next);
    move_character(d, c, next);

    heap_insert(&d->next_turn, c);
  }

  if (!dungeon_has_npcs(d) && !display_message){
    io_queue_message("Proceed to the next floor...");
    display_message = 1;
  }
  io_display(d);

  if (pc_is_alive(d) && c == d->the_pc) {
    character_next_turn(c);
    io_handle_input(d);
  }
}

void dir_nearest_wall(dungeon_t *d, character *c, pair_t dir)
{
  dir[dim_x] = dir[dim_y] = 0;

  if (character_get_x(c) != 1 && character_get_x(c) != DUNGEON_X - 2) {
    dir[dim_x] = (character_get_x(c) > DUNGEON_X - character_get_x(c) ? 1 : -1);
  }
  if (character_get_y(c) != 1 && character_get_y(c) != DUNGEON_Y - 2) {
    dir[dim_y] = (character_get_y(c) > DUNGEON_Y - character_get_y(c) ? 1 : -1);
  }
}

uint32_t in_corner(dungeon_t *d, character *c)
{
  uint32_t num_immutable;

  num_immutable = 0;

  num_immutable += (mapxy(character_get_x(c) - 1,
                          character_get_y(c)    ) == ter_wall_immutable);
  num_immutable += (mapxy(character_get_x(c) + 1,
                          character_get_y(c)    ) == ter_wall_immutable);
  num_immutable += (mapxy(character_get_x(c)    ,
                          character_get_y(c) - 1) == ter_wall_immutable);
  num_immutable += (mapxy(character_get_x(c)    ,
                          character_get_y(c) + 1) == ter_wall_immutable);

  return num_immutable > 1;
}

static void new_dungeon_level(dungeon_t *d, uint32_t dir)
{
  /* Eventually up and down will be independantly meaningful. *
   * For now, simply generate a new dungeon.                  */

  switch (dir) {
  case '<':
    new_dungeon(d);
    d->level--;
    display_message = 0;
    io_queue_message("level: %d", d->level);
    break;
  case '>':
    new_dungeon(d);
    d->level++;
    display_message = 0;
    io_queue_message("level: %d", d->level);
    break;
  default:
    break;
  }
}

uint32_t move_pc(dungeon_t *d, uint32_t dir)
{
  pair_t next;
  uint32_t was_stairs = 0;

  next[dim_y] = character_get_y(d->the_pc);
  next[dim_x] = character_get_x(d->the_pc);

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
    if (mappair(character_get_pos(d->the_pc)) == ter_stairs_up) {
      was_stairs = 1;
      new_dungeon_level(d, '<');
      //      return 10; //going up
    }
    break;
  case '>':
    if (mappair(character_get_pos(d->the_pc)) == ter_stairs_down) {
      was_stairs = 1;
      new_dungeon_level(d, '>');
      //return 15; //going down
    }
    break;
  }

  if (was_stairs) {
    return 0;
  }

  if ((dir != '>') && (dir != '<') && (mappair(next) >= ter_floor)) {
    move_character(d, d->the_pc, next);
    dijkstra(d);
    dijkstra_tunnel(d);
    d->the_pc->pick_up(d);

    return 0;
  }

  return 1;
}
