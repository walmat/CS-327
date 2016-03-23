#ifndef PC_H
# define PC_H

# include <stdint.h>

# include "dims.h"

#if __cplusplus
extern "C" {
#endif

#include "path.h"
#include "move.h"
#include "dungeon.h"
  
typedef struct dungeon dungeon_t;

typedef struct pc {
} pc_t;

void pc_delete(character_t *pc);
uint32_t pc_is_alive(dungeon_t *d);
void config_pc(dungeon_t *d);
uint32_t pc_next_pos(dungeon_t *d, pair_t dir);
void place_pc(dungeon_t *d);

void init_remem_dungeon(dungeon_t *d);
void update_remem_dungeon(dungeon_t *d);
char get_remem_dungeon(dungeon_t* d, uint8_t y, uint8_t x);
  
#if __cplusplus
}
class PC : public Character {
 public:
  terrain_type_t remembered_dungeon[DUNGEON_Y][DUNGEON_X];
  
};
#endif
  
#endif
