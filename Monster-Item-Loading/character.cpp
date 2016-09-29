#include <stdlib.h>

#include "character.h"
#include "heap.h"
#include "npc.h"
#include "pc.h"
#include "dungeon.h"

void character_delete(void *c)
{
  delete (character *) c;
}

static char *print_character(const void *v)
{
  (void) print_character;

  const character *c = (const character *)v;

  static char string[80];

  snprintf(string, 80, "%d:%d", c->next_turn, c->sequence_number);

  return string;
}

int32_t compare_characters_by_next_turn(const void *character1,
                                        const void *character2)
{
  int32_t diff;

  diff = (((character *) character1)->next_turn -
          ((character *) character2)->next_turn);
  return diff ? diff : (((character *) character1)->sequence_number -
                        ((character *) character2)->sequence_number);
}

uint32_t can_see(dungeon_t *d, pair_t voyeur, pair_t exhibitionist, int is_pc)
{
  /* Application of Bresenham's Line Drawing Algorithm.  If we can draw *
   * a line from v to e without intersecting any walls, then v can see  *
   * e.  Unfortunately, Bresenham isn't symmetric, so line-of-sight     *
   * based on this approach is not reciprocal (Helmholtz Reciprocity).  *
   * This is a very real problem in roguelike games, and one we're      *
   * going to ignore for now.  Algorithms that are symmetrical are far  *
   * more expensive.                                                    */

  pair_t first, second;
  pair_t del, f;
  int16_t a, b, c, i;
  int16_t visual_range;

  visual_range = is_pc ? PC_VISUAL_RANGE : NPC_VISUAL_RANGE;

  first[dim_x] = voyeur[dim_x];
  first[dim_y] = voyeur[dim_y];
  second[dim_x] = exhibitionist[dim_x];
  second[dim_y] = exhibitionist[dim_y];

  /* Monsters only use this to see the PC, so we can *
   * short circuit the tests when they are far away. */
  if ((abs(first[dim_x] - second[dim_x]) > visual_range) ||
      (abs(first[dim_y] - second[dim_y]) > visual_range)) {
    return 0;
  }

  /*
  mappair(first) = ter_debug;
  mappair(second) = ter_debug;
  */

  if (second[dim_x] > first[dim_x]) {
    del[dim_x] = second[dim_x] - first[dim_x];
    f[dim_x] = 1;
  } else {
    del[dim_x] = first[dim_x] - second[dim_x];
    f[dim_x] = -1;
  }

  if (second[dim_y] > first[dim_y]) {
    del[dim_y] = second[dim_y] - first[dim_y];
    f[dim_y] = 1;
  } else {
    del[dim_y] = first[dim_y] - second[dim_y];
    f[dim_y] = -1;
  }

  if (del[dim_x] > del[dim_y]) {
    a = del[dim_y] + del[dim_y];
    c = a - del[dim_x];
    b = c - del[dim_x];
    for (i = 0; i <= del[dim_x]; i++) {
      if (is_pc) {
        pc_learn_terrain(d->pc, first, mappair(first));
        pc_see_object(d->pc, objpair(first));
      }
      if ((mappair(first) < ter_floor) && i && (i != del[dim_x])) {
        return 0;
      }
      /*      mappair(first) = ter_debug;*/
      first[dim_x] += f[dim_x];
      if (c < 0) {
        c += a;
      } else {
        c += b;
        first[dim_y] += f[dim_y];
      }
    }
    return 1;
  } else {
    a = del[dim_x] + del[dim_x];
    c = a - del[dim_y];
    b = c - del[dim_y];
    for (i = 0; i <= del[dim_y]; i++) {
      if (is_pc) {
        pc_learn_terrain(d->pc, first, mappair(first));
        pc_see_object(d->pc, objpair(first));
      }
      if ((mappair(first) < ter_floor) && i && (i != del[dim_y])) {
        return 0;
      }
      /*      mappair(first) = ter_debug;*/
      first[dim_y] += f[dim_y];
      if (c < 0) {
        c += a;
      } else {
        c += b;
        first[dim_x] += f[dim_x];
      }
    }
    return 1;
  }

  return 1;
}

int8_t *character_get_pos(const character *c)
{
  return ((character *) c)->position;
}

int8_t character_get_y(const character *c)
{
  return ((character *) c)->position[dim_y];
}

void character_set_y(character *c, int8_t y)
{
  ((character *) c)->position[dim_y] = y;
}

int8_t character_get_x(const character *c)
{
  return ((character *) c)->position[dim_x];
}

void character_set_x(character *c, int8_t x)
{
  ((character *) c)->position[dim_x] = x;
}

uint32_t character_get_next_turn(const character *c)
{
  return ((character *) c)->next_turn;
}

void character_die(character *c)
{
  ((character *) c)->alive = 0;
}

int character_is_alive(const character *c)
{
  return ((character *) c)->alive;
}

void character_next_turn(character *c)
{
  character *the_character = (character *) c;

  the_character->next_turn += (1000 / the_character->speed);
}

void character_reset_turn(character *c)
{
  character *the_character = (character *) c;

  the_character->next_turn -= (1000 / the_character->speed);
}

char character_get_symbol(const character *c)
{
  return ((character *) c)->symbol;
}

const char *character_get_name(const character *c)
{
  return c->name;
}
