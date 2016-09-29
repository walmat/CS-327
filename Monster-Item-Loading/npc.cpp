#include <stdlib.h>

#include "utils.h"
#include "npc.h"
#include "pc.h"
#include "dungeon.h"
#include "character.h"
#include "move.h"
#include "path.h"
#include "ncurses.h"

void gen_monsters(dungeon_t *d, uint32_t nummon, uint32_t game_turn)
{
  uint32_t i;

  d->num_monsters = nummon;
  for (i = 0; i < nummon; i++) {
    monster_description::generate_monster(d);
  }
}

void npc_next_pos_rand_tunnel(dungeon_t *d, character *c, pair_t next)
{
  pair_t n;
  union {
    uint32_t i;
    uint8_t a[4];
  } r;

  do {
    n[dim_y] = next[dim_y];
    n[dim_x] = next[dim_x];
    r.i = rand();
    if (r.a[0] > 85 /* 255 / 3 */) {
      if (r.a[0] & 1) {
        n[dim_y]--;
      } else {
        n[dim_y]++;
      }
    }
    if (r.a[1] > 85 /* 255 / 3 */) {
      if (r.a[1] & 1) {
        n[dim_x]--;
      } else {
        n[dim_x]++;
      }
    }
  } while (mappair(n) == ter_wall_immutable);

  if (hardnesspair(n) <= 60) {
    if (hardnesspair(n)) {
      hardnesspair(n) = 0;
      mappair(n) = ter_floor_hall;

      /* Update distance maps because map has changed. */
      dijkstra(d);
      dijkstra_tunnel(d);
    }

    next[dim_x] = n[dim_x];
    next[dim_y] = n[dim_y];
  } else {
    hardnesspair(n) -= 60;
  }
}

void npc_next_pos_rand(dungeon_t *d, character *c, pair_t next)
{
  pair_t n;
  union {
    uint32_t i;
    uint8_t a[4];
  } r;

  do {
    n[dim_y] = next[dim_y];
    n[dim_x] = next[dim_x];
    r.i = rand();
    if (r.a[0] > 85 /* 255 / 3 */) {
      if (r.a[0] & 1) {
        n[dim_y]--;
      } else {
        n[dim_y]++;
      }
    }
    if (r.a[1] > 85 /* 255 / 3 */) {
      if (r.a[1] & 1) {
        n[dim_x]--;
      } else {
        n[dim_x]++;
      }
    }
  } while (mappair(n) < ter_floor);

  next[dim_y] = n[dim_y];
  next[dim_x] = n[dim_x];
}

void npc_next_pos_line_of_sight(dungeon_t *d, character *c, pair_t next)
{
  pair_t dir;
  pc *the_pc;
  npc *the_npc;

  the_pc = (pc *) d->pc;
  the_npc = (npc *) c;

  dir[dim_y] = the_pc->position[dim_y] - the_npc->position[dim_y];
  dir[dim_x] = the_pc->position[dim_x] - the_npc->position[dim_x];
  if (dir[dim_y]) {
    dir[dim_y] /= abs(dir[dim_y]);
  }
  if (dir[dim_x]) {
    dir[dim_x] /= abs(dir[dim_x]);
  }

  if (mapxy(next[dim_x] + dir[dim_x],
            next[dim_y] + dir[dim_y]) >= ter_floor) {
    next[dim_x] += dir[dim_x];
    next[dim_y] += dir[dim_y];
  } else if (mapxy(next[dim_x] + dir[dim_x], next[dim_y]) >= ter_floor) {
    next[dim_x] += dir[dim_x];
  } else if (mapxy(next[dim_x], next[dim_y] + dir[dim_y]) >= ter_floor) {
    next[dim_y] += dir[dim_y];
  }
}

void npc_next_pos_line_of_sight_tunnel(dungeon_t *d,
                                       character *c,
                                       pair_t next)
{
  pair_t dir;
  pc *the_pc;
  npc *the_npc;

  the_pc = (pc *) d->pc;
  the_npc = (npc *) c;

  dir[dim_y] = the_pc->position[dim_y] - the_npc->position[dim_y];
  dir[dim_x] = the_pc->position[dim_x] - the_npc->position[dim_x];
  if (dir[dim_y]) {
    dir[dim_y] /= abs(dir[dim_y]);
  }
  if (dir[dim_x]) {
    dir[dim_x] /= abs(dir[dim_x]);
  }

  dir[dim_x] += next[dim_x];
  dir[dim_y] += next[dim_y];

  if (hardnesspair(dir) <= 60) {
    if (hardnesspair(dir)) {
      hardnesspair(dir) = 0;
      mappair(dir) = ter_floor_hall;

      /* Update distance maps because map has changed. */
      dijkstra(d);
      dijkstra_tunnel(d);
    }

    next[dim_x] = dir[dim_x];
    next[dim_y] = dir[dim_y];
  } else {
    hardnesspair(dir) -= 60;
  }
}

void npc_next_pos_gradient(dungeon_t *d, character *c, pair_t next)
{
  npc *the_npc;

  the_npc = (npc *) c;

  /* Handles both tunneling and non-tunneling versions */
  pair_t min_next;
  uint16_t min_cost;
  if (the_npc->characteristics & NPC_TUNNEL) {
    min_cost = (d->pc_tunnel[next[dim_y] - 1][next[dim_x]] +
                (d->hardness[next[dim_y] - 1][next[dim_x]] / 60));
    min_next[dim_x] = next[dim_x];
    min_next[dim_y] = next[dim_y] - 1;
    if ((d->pc_tunnel[next[dim_y] + 1][next[dim_x]    ] +
         (d->hardness[next[dim_y] + 1][next[dim_x]] / 60)) < min_cost) {
      min_cost = (d->pc_tunnel[next[dim_y] + 1][next[dim_x]] +
                  (d->hardness[next[dim_y] + 1][next[dim_x]] / 60));
      min_next[dim_x] = next[dim_x];
      min_next[dim_y] = next[dim_y] + 1;
    }
    if ((d->pc_tunnel[next[dim_y]    ][next[dim_x] + 1] +
         (d->hardness[next[dim_y]    ][next[dim_x] + 1] / 60)) < min_cost) {
      min_cost = (d->pc_tunnel[next[dim_y]][next[dim_x] + 1] +
                  (d->hardness[next[dim_y]][next[dim_x] + 1] / 60));
      min_next[dim_x] = next[dim_x] + 1;
      min_next[dim_y] = next[dim_y];
    }
    if ((d->pc_tunnel[next[dim_y]    ][next[dim_x] - 1] +
         (d->hardness[next[dim_y]    ][next[dim_x] - 1] / 60)) < min_cost) {
      min_cost = (d->pc_tunnel[next[dim_y]][next[dim_x] - 1] +
                  (d->hardness[next[dim_y]][next[dim_x] - 1] / 60));
      min_next[dim_x] = next[dim_x] - 1;
      min_next[dim_y] = next[dim_y];
    }
    if ((d->pc_tunnel[next[dim_y] - 1][next[dim_x] + 1] +
         (d->hardness[next[dim_y] - 1][next[dim_x] + 1] / 60)) < min_cost) {
      min_cost = (d->pc_tunnel[next[dim_y] - 1][next[dim_x] + 1] +
                  (d->hardness[next[dim_y] - 1][next[dim_x] + 1] / 60));
      min_next[dim_x] = next[dim_x] + 1;
      min_next[dim_y] = next[dim_y] - 1;
    }
    if ((d->pc_tunnel[next[dim_y] + 1][next[dim_x] + 1] +
         (d->hardness[next[dim_y] + 1][next[dim_x] + 1] / 60)) < min_cost) {
      min_cost = (d->pc_tunnel[next[dim_y] + 1][next[dim_x] + 1] +
                  (d->hardness[next[dim_y] + 1][next[dim_x] + 1] / 60));
      min_next[dim_x] = next[dim_x] + 1;
      min_next[dim_y] = next[dim_y] + 1;
    }
    if ((d->pc_tunnel[next[dim_y] - 1][next[dim_x] - 1] +
         (d->hardness[next[dim_y] - 1][next[dim_x] - 1] / 60)) < min_cost) {
      min_cost = (d->pc_tunnel[next[dim_y] - 1][next[dim_x] - 1] +
                  (d->hardness[next[dim_y] - 1][next[dim_x] - 1] / 60));
      min_next[dim_x] = next[dim_x] - 1;
      min_next[dim_y] = next[dim_y] - 1;
    }
    if ((d->pc_tunnel[next[dim_y] + 1][next[dim_x] - 1] +
         (d->hardness[next[dim_y] + 1][next[dim_x] - 1] / 60)) < min_cost) {
      min_cost = (d->pc_tunnel[next[dim_y] + 1][next[dim_x] - 1] +
                  (d->hardness[next[dim_y] + 1][next[dim_x] - 1] / 60));
      min_next[dim_x] = next[dim_x] - 1;
      min_next[dim_y] = next[dim_y] + 1;
    }
    if (hardnesspair(min_next) <= 60) {
      if (hardnesspair(min_next)) {
        hardnesspair(min_next) = 0;
        mappair(min_next) = ter_floor_hall;

        /* Update distance maps because map has changed. */
        dijkstra(d);
        dijkstra_tunnel(d);
      }

      next[dim_x] = min_next[dim_x];
      next[dim_y] = min_next[dim_y];
    } else {
      hardnesspair(min_next) -= 60;
    }
  } else {
    /* Make monsters prefer cardinal directions */
    if (d->pc_distance[next[dim_y] - 1][next[dim_x]    ] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]--;
      return;
    }
    if (d->pc_distance[next[dim_y] + 1][next[dim_x]    ] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]++;
      return;
    }
    if (d->pc_distance[next[dim_y]    ][next[dim_x] + 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_x]++;
      return;
    }
    if (d->pc_distance[next[dim_y]    ][next[dim_x] - 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_x]--;
      return;
    }
    if (d->pc_distance[next[dim_y] - 1][next[dim_x] + 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]--;
      next[dim_x]++;
      return;
    }
    if (d->pc_distance[next[dim_y] + 1][next[dim_x] + 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]++;
      next[dim_x]++;
      return;
    }
    if (d->pc_distance[next[dim_y] - 1][next[dim_x] - 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]--;
      next[dim_x]--;
      return;
    }
    if (d->pc_distance[next[dim_y] + 1][next[dim_x] - 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]++;
      next[dim_x]--;
      return;
    }
  }
}

static void npc_next_pos_00(dungeon_t *d, character *c, pair_t next)
{
  pc *the_pc;
  npc *the_npc;

  the_pc = (pc *) d->pc;
  the_npc = (npc *) c;

  /* not smart; not telepathic; not tunneling; not erratic */
  if (can_see(d, character_get_pos(c), character_get_pos(d->pc), 0)) {
    the_npc->pc_last_known_position[dim_y] = the_pc->position[dim_y];
    the_npc->pc_last_known_position[dim_x] = the_pc->position[dim_x];
    npc_next_pos_line_of_sight(d, c, next);
  } else {
    npc_next_pos_rand(d, c, next);
  }
}

static void npc_next_pos_01(dungeon_t *d, character *c, pair_t next)
{
  pc *the_pc;
  npc *the_npc;

  the_pc = (pc *) d->pc;
  the_npc = (npc *) c;

  /*     smart; not telepathic; not tunneling; not erratic */
  if (can_see(d, character_get_pos(c), character_get_pos(d->pc), 0)) {
    the_npc->pc_last_known_position[dim_y] = the_pc->position[dim_y];
    the_npc->pc_last_known_position[dim_x] = the_pc->position[dim_x];
    the_npc->have_seen_pc = 1;
    npc_next_pos_line_of_sight(d, c, next);
  } else if (the_npc->have_seen_pc) {
    npc_next_pos_line_of_sight(d, c, next);
  }

  if ((next[dim_x] == the_npc->pc_last_known_position[dim_x]) &&
      (next[dim_y] == the_npc->pc_last_known_position[dim_y])) {
    the_npc->have_seen_pc = 0;
  }
}

static void npc_next_pos_02(dungeon_t *d, character *c, pair_t next)
{
  pc *the_pc;
  npc *the_npc;

  the_pc = (pc *) d->pc;
  the_npc = (npc *) c;

  /* not smart;     telepathic; not tunneling; not erratic */
  the_npc->pc_last_known_position[dim_y] = the_pc->position[dim_y];
  the_npc->pc_last_known_position[dim_x] = the_pc->position[dim_x];
  npc_next_pos_line_of_sight(d, c, next);
}

static void npc_next_pos_03(dungeon_t *d, character *c, pair_t next)
{
  /*     smart;     telepathic; not tunneling; not erratic */
  npc_next_pos_gradient(d, c, next);
}

static void npc_next_pos_04(dungeon_t *d, character *c, pair_t next)
{
  pc *the_pc;
  npc *the_npc;

  the_pc = (pc *) d->pc;
  the_npc = (npc *) c;

  /* not smart; not telepathic;     tunneling; not erratic */
  if (can_see(d, character_get_pos(c), character_get_pos(d->pc), 0)) {
    the_npc->pc_last_known_position[dim_y] = the_pc->position[dim_y];
    the_npc->pc_last_known_position[dim_x] = the_pc->position[dim_x];
    npc_next_pos_line_of_sight(d, c, next);
  } else {
    npc_next_pos_rand_tunnel(d, c, next);
  }
}

static void npc_next_pos_05(dungeon_t *d, character *c, pair_t next)
{
  pc *the_pc;
  npc *the_npc;

  the_pc = (pc *) d->pc;
  the_npc = (npc *) c;

  /*     smart; not telepathic;     tunneling; not erratic */
  if (can_see(d, character_get_pos(c), character_get_pos(d->pc), 0)) {
    the_npc->pc_last_known_position[dim_y] = the_pc->position[dim_y];
    the_npc->pc_last_known_position[dim_x] = the_pc->position[dim_x];
    the_npc->have_seen_pc = 1;
    npc_next_pos_line_of_sight(d, c, next);
  } else if (the_npc->have_seen_pc) {
    npc_next_pos_line_of_sight_tunnel(d, c, next);
  }

  if ((next[dim_x] == the_npc->pc_last_known_position[dim_x]) &&
      (next[dim_y] == the_npc->pc_last_known_position[dim_y])) {
    the_npc->have_seen_pc = 0;
  }
}

static void npc_next_pos_06(dungeon_t *d, character *c, pair_t next)
{
  pc *the_pc;
  npc *the_npc;

  the_pc = (pc *) d->pc;
  the_npc = (npc *) c;

  /* not smart;     telepathic;     tunneling; not erratic */
  the_npc->pc_last_known_position[dim_y] = the_pc->position[dim_y];
  the_npc->pc_last_known_position[dim_x] = the_pc->position[dim_x];
  npc_next_pos_line_of_sight_tunnel(d, c, next);
}

static void npc_next_pos_07(dungeon_t *d, character *c, pair_t next)
{
  /*     smart;     telepathic;     tunneling; not erratic */
  npc_next_pos_gradient(d, c, next);
}

static void npc_next_pos_08(dungeon_t *d, character *c, pair_t next)
{
  /* not smart; not telepathic; not tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand(d, c, next);
  } else {
    npc_next_pos_00(d, c, next);
  }
}

static void npc_next_pos_09(dungeon_t *d, character *c, pair_t next)
{
  /*     smart; not telepathic; not tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand(d, c, next);
  } else {
    npc_next_pos_01(d, c, next);
  }
}

static void npc_next_pos_0a(dungeon_t *d, character *c, pair_t next)
{
  /* not smart;     telepathic; not tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand(d, c, next);
  } else {
        npc_next_pos_02(d, c, next);
  }
}

static void npc_next_pos_0b(dungeon_t *d, character *c, pair_t next)
{
  /*     smart;     telepathic; not tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand(d, c, next);
  } else {
    npc_next_pos_03(d, c, next);
  }
}

static void npc_next_pos_0c(dungeon_t *d, character *c, pair_t next)
{
  /* not smart; not telepathic;     tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand_tunnel(d, c, next);
  } else {
    npc_next_pos_04(d, c, next);
  }
}

static void npc_next_pos_0d(dungeon_t *d, character *c, pair_t next)
{
  /*     smart; not telepathic;     tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand_tunnel(d, c, next);
  } else {
    npc_next_pos_05(d, c, next);
  }
}

static void npc_next_pos_0e(dungeon_t *d, character *c, pair_t next)
{
  /* not smart;     telepathic;     tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand_tunnel(d, c, next);
  } else {
    npc_next_pos_06(d, c, next);
  }
}

static void npc_next_pos_0f(dungeon_t *d, character *c, pair_t next)
{
  /*     smart;     telepathic;     tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand_tunnel(d, c, next);
  } else {
    npc_next_pos_07(d, c, next);
  }
}

void (*npc_move_func[])(dungeon_t *d, character *c, pair_t next) = {
  /* We'll have one function for each combination of bits, so the *
   * order is based on binary counting through the NPC_* bits.    *
   * It could be very easy to mess this up, so be careful.  We'll *
   * name them according to their hex value.                      */
  npc_next_pos_00,
  npc_next_pos_01,
  npc_next_pos_02,
  npc_next_pos_03,
  npc_next_pos_04,
  npc_next_pos_05,
  npc_next_pos_06,
  npc_next_pos_07,
  npc_next_pos_08,
  npc_next_pos_09,
  npc_next_pos_0a,
  npc_next_pos_0b,
  npc_next_pos_0c,
  npc_next_pos_0d,
  npc_next_pos_0e,
  npc_next_pos_0f,
};

void npc_next_pos(dungeon_t *d, character *c, pair_t next)
{
  npc *the_npc = (npc *) c;

  next[dim_y] = the_npc->position[dim_y];
  next[dim_x] = the_npc->position[dim_x];

  npc_move_func[the_npc->characteristics & 0x0000000f](d, c, next);
}

uint32_t dungeon_has_npcs(dungeon_t *d)
{
  return d->num_monsters;
}

npc::npc(dungeon_t *d, const monster_description &m)
{
  pair_t p;
  uint32_t room;

  symbol = m.symbol;
  color = m.color;
  room = rand_range(1, d->num_rooms - 1);
  do {
    p[dim_y] = rand_range(d->rooms[room].position[dim_y],
                          (d->rooms[room].position[dim_y] +
                           d->rooms[room].size[dim_y] - 1));
    p[dim_x] = rand_range(d->rooms[room].position[dim_x],
                          (d->rooms[room].position[dim_x] +
                           d->rooms[room].size[dim_x] - 1));
  } while (d->charmap[p[dim_y]][p[dim_x]]);
  pc_last_known_position[dim_y] = p[dim_y];
  pc_last_known_position[dim_x] = p[dim_x];
  position[dim_y] = p[dim_y];
  position[dim_x] = p[dim_x];
  d->charmap[p[dim_y]][p[dim_x]] = this;
  speed = m.speed.roll();
  hp = m.hitpoints.roll();
  damage = &m.damage;
  next_turn = d->pc->next_turn;
  alive = 1;
  sequence_number = ++d->character_sequence_number;
  characteristics = m.abilities;
  have_seen_pc = 0;
  name = m.name.c_str();
  description = (const char *) m.description.c_str();
}
