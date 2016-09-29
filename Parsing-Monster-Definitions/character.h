#ifndef CHARACTER_H
# define CHARACTER_H

# include <stdint.h>

# include "dims.h"

# ifdef __cplusplus
extern "C" {
# endif

typedef struct dungeon dungeon_t;

typedef struct {
} character_t;

int32_t compare_characters_by_next_turn(const void *character1,
                                        const void *character2);
uint32_t can_see(dungeon_t *d, pair_t voyeur, pair_t exhibitionist, int is_pc);
void character_delete(void *c);
int8_t *character_get_pos(const character_t *c);
int8_t character_get_y(const character_t *c);
void character_set_y(character_t *c, int8_t y);
int8_t character_get_x(const character_t *c);
void character_set_x(character_t *c, int8_t x);
uint32_t character_get_next_turn(const character_t *c);
void character_die(character_t *c);
int character_is_alive(const character_t *c);
void character_next_turn(character_t *c);
void character_reset_turn(character_t *c);
char character_get_symbol(const character_t *c);
  

# ifdef __cplusplus
}

class character {
 public:
  char symbol;
  pair_t position;
  int32_t speed;
  uint32_t next_turn;
  uint32_t alive;
  /* The priority queue is not stable.  It's nice to have a record of *
   * how many monsters have been created, and this sequence number    *
   * serves that purpose, but more importantly, prioritizing lower    *
   * sequence numbers ahead of higher ones guarantees that turn order *
   * is fair.  PC gets sequence number zero, and a global sequence,   *
   * stored in the dungeon, is incremented each time a NPC is         *
   * created and copied here then.                                    */
  uint32_t sequence_number;
};

# endif
#endif
