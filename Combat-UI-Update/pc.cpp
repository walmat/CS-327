#include <stdlib.h>
#include <ncurses.h>

#include "string.h"

#include "dungeon.h"
#include "pc.h"
#include "utils.h"
#include "move.h"
#include "path.h"

void delete_pc(character *the_pc)
{
  pc_delete_inv(the_pc);
  pc_delete_eq(the_pc);
  delete static_cast<pc *>(the_pc);
}
uint32_t pc_is_alive(dungeon_t *d)
{
  return ((pc *) d->pc)->alive;
}

void place_pc(dungeon_t *d)
{
  ((pc *) d->pc)->position[dim_y] = rand_range(d->rooms->position[dim_y],
                                               (d->rooms->position[dim_y] +
                                                d->rooms->size[dim_y] - 1));
  ((pc *) d->pc)->position[dim_x] = rand_range(d->rooms->position[dim_x],
                                               (d->rooms->position[dim_x] +
                                                d->rooms->size[dim_x] - 1));

  pc_init_known_terrain(d->pc);
  pc_observe_terrain(d->pc, d);
}

void config_pc(dungeon_t *d)
{
  /* This should be in the PC constructor, now. */
  pc *the_pc;
  static dice pc_dice(0, 1, 4);

  the_pc = new pc;
  d->pc = (character *) the_pc;

  the_pc->symbol = '@';

  place_pc(d);

  the_pc->speed = PC_SPEED;
  the_pc->next_turn = 0;
  the_pc->alive = 1;
  the_pc->sequence_number = 0;
  the_pc->color.push_back(COLOR_WHITE);
  the_pc->damage = &pc_dice;
  the_pc->name = "Isabella Garcia-Shapiro";
  the_pc->hp = rand_range(100,150);
  the_pc->inv_size = 0;
  
  for (int i = 0; i < 12; i++){
    the_pc->equipment[i] = NULL;
  }

  for (int i = 0; i < 10; i++){
    the_pc->inventory[i] = NULL;
  }

  d->charmap[the_pc->position[dim_y]]
            [the_pc->position[dim_x]] = (character *) d->pc;

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
    dir[dim_y] = (mapxy(((pc *) d->pc)->position[dim_x],
                        ((pc *) d->pc)->position[dim_y] - 1) ==
                  ter_wall_immutable) ? 1 : -1;
  } else {
    dir_nearest_wall(d, d->pc, dir);
  }

  return 0;
}

void pc_learn_terrain(character *the_pc, pair_t pos, terrain_type_t ter)
{
  ((pc *) the_pc)->known_terrain[pos[dim_y]][pos[dim_x]] = ter;
  ((pc *) the_pc)->visible[pos[dim_y]][pos[dim_x]] = 1;
}

void pc_see_object(character *the_pc, object *o)
{
  if (o) {
    o->has_been_seen();
  }
}

void pc_reset_visibility(character *the_pc)
{
  uint32_t y, x;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      ((pc *) the_pc)->visible[y][x] = 0;
    }
  }
}

terrain_type_t pc_learned_terrain(character *the_pc, int8_t y, int8_t x)
{
  return ((pc *) the_pc)->known_terrain[y][x];
}

void pc_init_known_terrain(character *the_pc)
{
  uint32_t y, x;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      ((pc *) the_pc)->known_terrain[y][x] = ter_unknown;
      ((pc *) the_pc)->visible[y][x] = 0;
    }
  }
}

void pc_observe_terrain(character *the_pc, dungeon_t *d)
{
  pair_t where;
  pc *p;
  int8_t y_min, y_max, x_min, x_max;

  p = (pc *) the_pc;

  y_min = p->position[dim_y] - PC_VISUAL_RANGE;
  if (y_min < 0) {
    y_min = 0;
  }
  y_max = p->position[dim_y] + PC_VISUAL_RANGE;
  if (y_max > DUNGEON_Y - 1) {
    y_max = DUNGEON_Y - 1;
  }
  x_min = p->position[dim_x] - PC_VISUAL_RANGE;
  if (x_min < 0) {
    x_min = 0;
  }
  x_max = p->position[dim_x] + PC_VISUAL_RANGE;
  if (x_max > DUNGEON_X - 1) {
    x_max = DUNGEON_X - 1;
  }

  for (where[dim_y] = y_min; where[dim_y] <= y_max; where[dim_y]++) {
    where[dim_x] = x_min;
    can_see(d, p->position, where, 1);
    where[dim_x] = x_max;
    can_see(d, p->position, where, 1);
  }
  /* Take one off the x range because we alreay hit the corners above. */
  for (where[dim_x] = x_min - 1; where[dim_x] <= x_max - 1; where[dim_x]++) {
    where[dim_y] = y_min;
    can_see(d, p->position, where, 1);
    where[dim_y] = y_max;
    can_see(d, p->position, where, 1);
  }       
}

int32_t is_illuminated(character *the_pc, int8_t y, int8_t x)
{
  return ((pc *) the_pc)->visible[y][x];
}
 
void pc_pickup_obj(character* the_pc, object* o)
{
  ((pc*) the_pc)->inventory[((pc*) the_pc)->inv_size] = o;
  ((pc*) the_pc)->inv_size++;
}

void pc_delete_inv(character* the_pc)
{
  for(int i = 0; i < ((pc*) the_pc)->inv_size; i++){
    delete(((pc*) the_pc)->inventory[i]);
  }
}

void pc_delete_eq(character* the_pc)
{
  for(int i = 0; i < 12; i++){
    if(((pc*) the_pc)->equipment[i]) {
      delete(((pc*) the_pc)->equipment[i]);
    }
  }
}

void pc_equip(character* the_pc, int32_t i)
{
  pc* p = ((pc*) the_pc);
  int32_t type, index;
  object* o;

  if(!p->inventory[i]) return;

  type = p->inventory[i]->get_type();

  if (type > 0 && type < 11){
    if(p->equipment[type-1]){
      o = p->equipment[type-1];
      p->equipment[type-1] = p->inventory[i];
      p->inventory[i] = o;
    } else {
      p->equipment[type-1] = p->inventory[i];
      for (index = i; index < p->inv_size-1; index++){
	p->inventory[index] = p->inventory[index+1];
      }
      p->inventory[p->inv_size-1] = NULL;
      p->inv_size--;
    }
  } else if (type == 11) {
    //ring equipping
    if (p->equipment[10]){
      if (p->equipment[11]){
	o = p->equipment[11];
	p->equipment[11] = p->inventory[i];
	p->inventory[i] = o;
      } else {
	p->equipment[11] = p->inventory[i];
	for (index = i; index < p->inv_size-1; index++){
	  p->inventory[index] = p->inventory[index+1];
	}
	p->inventory[p->inv_size-1] = NULL;
	p->inv_size--;
      }
    } else {
      p->equipment[10] = p->inventory[i];
      for (index = i; index < p->inv_size-1; index++){
	p->inventory[index] = p->inventory[index+1];
      }
      p->inventory[p->inv_size-1] = NULL;
      p->inv_size--;
    }
  }
  pc_update_speed(the_pc);
}

void pc_unequip(character* the_pc, int32_t i)
{
  pc* p = ((pc*) the_pc);

  if (p->inv_size >= 10) return;

  p->inventory[p->inv_size] = p->equipment[i];
  p->equipment[i] = NULL;
  p->inv_size++;
  pc_update_speed(the_pc);
  
}

void pc_expunge(character* the_pc, int32_t i)
{
  pc* p = ((pc*) the_pc);
  object* o;
  
  if (!(p->inventory[i])) return;

  o = p->inventory[i];
  for (int index = i; index < p->inv_size; index++){
    p->inventory[index] = p->inventory[index+1];
  }
  p->inventory[p->inv_size] = NULL;
  p->inv_size--;
  delete o;
}

void pc_drop(dungeon_t* d, int32_t i)
{
  pc* p = ((pc*) d->pc);

  if (!(p->inventory[i]) || objxy(character_get_x(d->pc), character_get_y(d->pc))) return;
  objxy(character_get_x(d->pc), character_get_y(d->pc)) = p->inventory[i];
      
  for (int index = i; index < p->inv_size; index++){
    p->inventory[index] = p->inventory[index+1];
  }
  p->inventory[p->inv_size] = NULL;
  p->inv_size--;
}

void pc_update_speed(character* the_pc)
{
  pc* p = ((pc*) the_pc);

  p->speed = PC_SPEED;
  for(int i = 0; i < 12; i++){
    if (p->equipment[i]){
      p->speed += p->equipment[i]->get_speed();
    }
  }
  if (p->speed < 1) {
    p->speed = 1;
  }
}

int32_t get_inv_size(character* the_pc)
{
  return ((pc*) the_pc)->inv_size;
}

int32_t get_pc_dmg(character* the_pc)
{
  pc* p = ((pc*) the_pc);
  int32_t dmg = p->damage->roll();

  for (int32_t i = 0; i < 12; i++){
    if (p->equipment[i]) {
      dmg += p->equipment[i]->roll_dice();
    }
  }
  return dmg;
}
