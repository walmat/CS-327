#ifndef PC_H
# define PC_H

# include <stdint.h>

# include "dims.h"

# ifdef __cplusplus
extern "C" {
# endif

typedef struct dungeon dungeon_t;
# include "dungeon.h"

typedef struct {
} pc_t;

uint32_t pc_is_alive(dungeon_t *d);
void config_pc(dungeon_t *d);
uint32_t pc_next_pos(dungeon_t *d, pair_t dir);
void place_pc(dungeon_t *d);
void delete_pc(character_t *the_pc);
void pc_learn_terrain(character_t *the_pc, pair_t pos, terrain_type_t ter);
terrain_type_t pc_learned_terrain(character_t *the_pc, int8_t y, int8_t x);
void pc_init_known_terrain(character_t *the_pc);
void pc_observe_terrain(character_t *the_pc, dungeon_t *d);
int32_t is_illuminated(character_t *the_pc, int8_t y, int8_t x);
void pc_reset_visibility(character_t *the_pc);

# ifdef __cplusplus
}

# include "character.h"

class pc : public character {
 public:
  terrain_type_t known_terrain[DUNGEON_Y][DUNGEON_X];
  unsigned char visible[DUNGEON_Y][DUNGEON_X];
};

# endif
#endif
