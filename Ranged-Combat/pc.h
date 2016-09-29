#ifndef PC_H
# define PC_H

# include <stdint.h>

# include "dims.h"
# include "dungeon.h"

# define MAX_INVENTORY 10

typedef struct dungeon dungeon_t;

uint32_t pc_is_alive(dungeon_t *d);
void config_pc(dungeon_t *d);
uint32_t pc_next_pos(dungeon_t *d, pair_t dir);
void place_pc(dungeon_t *d);
void delete_pc(character *the_pc);
void pc_learn_terrain(character *the_pc, pair_t pos, terrain_type_t ter);
terrain_type_t pc_learned_terrain(character *the_pc, int8_t y, int8_t x);
void pc_init_known_terrain(character *the_pc);
void pc_observe_terrain(character *the_pc, dungeon_t *d);
int32_t is_illuminated(character *the_pc, int8_t y, int8_t x);
void pc_reset_visibility(character *the_pc);
void pc_see_object(character *the_pc, object *o);
int32_t get_inv_size(character* the_pc); 
int32_t get_pc_speed(character* the_pc);
int32_t get_pc_dmg(character* the_pc);
int32_t get_pc_hp(character* the_pc);
int32_t get_pc_def(character* the_pc);
int32_t get_pc_speed(character* the_pc);
const char* get_pc_name(character* the_pc);

typedef enum eq_slot {
  eq_slot_weapon,
  eq_slot_offhand,
  eq_slot_ranged,
  eq_slot_light,
  eq_slot_armor,
  eq_slot_helmet,
  eq_slot_cloak,
  eq_slot_gloves,
  eq_slot_boots,
  eq_slot_amulet,
  eq_slot_lring,
  eq_slot_rring,
  num_eq_slots
} eq_slot_t;

extern const char *eq_slot_name[num_eq_slots];

# include "character.h"

class pc : public character {
 private:
  void recalculate_speed();
  uint32_t has_open_inventory_slot();
  int32_t get_first_open_inventory_slot();
  object *from_pile(dungeon_t *d, pair_t pos);

 public:
  terrain_type_t known_terrain[DUNGEON_Y][DUNGEON_X];
  unsigned char visible[DUNGEON_Y][DUNGEON_X];
  object *eq[num_eq_slots];
  object *in[MAX_INVENTORY];
  int32_t inv_size;

  uint32_t wear_in(uint32_t slot);
  uint32_t remove_eq(uint32_t slot);
  uint32_t drop_in(dungeon_t *d, uint32_t slot);
  uint32_t destroy_in(uint32_t slot);
  uint32_t pick_up(dungeon_t *d);
  pc();
  ~pc();
};

#endif
