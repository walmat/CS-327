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

void do_combat(dungeon_t *d, character_t *atk, character_t *def)
{
  def->alive = 0;
  if (def != &d->pc) {
    d->num_monsters--;
  }
}

void move_character(dungeon_t *d, character_t *c, pair_t next)
{
  if (charpair(next) &&
      ((next[dim_y] != c->position[dim_y]) ||
       (next[dim_x] != c->position[dim_x]))) {
    do_combat(d, c, charpair(next));
  } else {
    /* No character in new position. */
    d->character[c->position[dim_y]][c->position[dim_x]] = NULL;
    c->position[dim_y] = next[dim_y];
    c->position[dim_x] = next[dim_x];
    d->character[c->position[dim_y]][c->position[dim_x]] = c;
  }
}

int do_moves(dungeon_t *d)
{
  pair_t next;
  character_t *c;

  /* Remove the PC when it is PC turn.  Replace on next call.  This allows *
   * use to completely uninit the heap when generating a new level without *
   * worrying about deleting the PC.                                       */

  if (pc_is_alive(d)) {
    heap_insert(&d->next_turn, &d->pc);
  }

  while (pc_is_alive(d) && ((c = heap_remove_min(&d->next_turn)) != &d->pc)) {
    if (!c->alive) {
      if (d->character[c->position[dim_y]][c->position[dim_x]] == c) {
        d->character[c->position[dim_y]][c->position[dim_x]] = NULL;
      }
      if (c != &d->pc) {
        character_delete(c);
      }
      continue;
    }
    
    c->next_turn += (1000 / c->speed);
    if (d->npcs_move){
      npc_next_pos(d, c, next);
      move_character(d, c, next);
    }
    heap_insert(&d->next_turn, c);
  }
  if (pc_is_alive(d) && c == &d->pc) {
    move_pc(d, &next);
    if (next[dim_x] == 0 && next[dim_y] == 0){
      //quit the game
      c->alive = 0;
    }
    else if (next[dim_x] == 100 && next[dim_y] == 100) {
      //bring up monster list
      return 1;
    }
    else if (next[dim_x] == 50 && next[dim_y] == 50){
      //generate new dungeon going downstairs
      return 2;
    }
    else if (next[dim_x] == 75 && next[dim_y] == 75){
      //generate new dungeon going upstairs
      return 3;
    }
    if (!(mappair(next) <= ter_floor)) {
      move_character(d, c, next);
    } 
    c->next_turn += (1000 / c->speed);
  }
  return 0;
}

void dir_nearest_wall(dungeon_t *d, character_t *c, pair_t dir)
{
  dir[dim_x] = dir[dim_y] = 0;

  if (c->position[dim_x] != 1 && c->position[dim_x] != DUNGEON_X - 2) {
    dir[dim_x] = (c->position[dim_x] > DUNGEON_X - c->position[dim_x] ? 1 : -1);
  }
  if (c->position[dim_y] != 1 && c->position[dim_y] != DUNGEON_Y - 2) {
    dir[dim_y] = (c->position[dim_y] > DUNGEON_Y - c->position[dim_y] ? 1 : -1);
  }
}

uint32_t in_corner(dungeon_t *d, character_t *c)
{
  uint32_t num_immutable;

  num_immutable = 0;

  num_immutable += (mapxy(c->position[dim_x] - 1,
                          c->position[dim_y]    ) == ter_wall_immutable);
  num_immutable += (mapxy(c->position[dim_x] + 1,
                          c->position[dim_y]    ) == ter_wall_immutable);
  num_immutable += (mapxy(c->position[dim_x]    ,
                          c->position[dim_y] - 1) == ter_wall_immutable);
  num_immutable += (mapxy(c->position[dim_x]    ,
                          c->position[dim_y] + 1) == ter_wall_immutable);

  return num_immutable > 1;
}

void move_pc(dungeon_t *d, pair_t *pt)
{
  char c = getch();
  d->npcs_move = 1;
  (*pt)[dim_x] = (&d->pc)->position[dim_x];
  (*pt)[dim_y] = (&d->pc)->position[dim_y];
  switch(c){
    //move left
  case '4':
  case 'h':
    (*pt)[dim_x]--;
    move(0,0);
    clrtoeol();
    break;
    //move down left
  case '1':
  case 'b':
    (*pt)[dim_x]--;
    (*pt)[dim_y]++;
    move(0,0);
    clrtoeol();
    break;
    //move down
  case '2':
  case 'j':
    (*pt)[dim_y]++;
    move(0,0);
    clrtoeol();
    break;
    //move down right
  case '3':
  case 'n':
    (*pt)[dim_x]++;
    (*pt)[dim_y]++;
    move(0,0);
    clrtoeol();
    break;
    //move right
  case '6':
  case 'l':
    (*pt)[dim_x]++;
    move(0,0);
    clrtoeol();
    break;
    //move up right
  case '9':
  case 'u':
    (*pt)[dim_x]++;
    (*pt)[dim_y]--;
    move(0,0);
    clrtoeol();
    break;
    //move up
  case '8':
  case 'k':
    (*pt)[dim_y]--;
    move(0,0);
    clrtoeol();
    break;
    //move up left
  case '7':
  case 'y':
    (*pt)[dim_x]--;
    (*pt)[dim_y]--;
    move(0,0);
    clrtoeol();
      break;
  case ' ':
    move(0,0);
    clrtoeol();
    //rest for a turn
    break;
  case '>':
    //go downstairs
    //check to see if current spot is '>'
    if (mappair((*pt)) == ter_stair_down){
      (*pt)[dim_x] = 50;
      (*pt)[dim_y] = 50;
    }
    else {
      mvprintw(0,0,"input error: %c", c);
      d->npcs_move = 0;
    }
    break;
  case '<':
    //go upstairs
    //check to see if current spot is '<'
    if (mappair((*pt)) == ter_stair_up){
      (*pt)[dim_x] = 75;
      (*pt)[dim_y] = 75;
    }
    else {
      mvprintw(0,0,"input error: %c", c);
      d->npcs_move = 0;
    }
    break;
  case 'm':
    (*pt)[dim_x] = 100;
    (*pt)[dim_y] = 100;
    break;
  case 'S':
    (*pt)[dim_x] = 0;
    (*pt)[dim_y] = 0;
    break;
  default:
    //default case for any key not recognized
    mvprintw(0,0,"input error: %c", c);
    d->npcs_move = 0;
    break;
  }
}
