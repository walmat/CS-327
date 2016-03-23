#ifndef CHARACTER_H
# define CHARACTER_H

# include <stdint.h>

# include "dims.h"

#if __cplusplus
extern "C"{
#endif

typedef struct dungeon dungeon_t;
typedef struct npc npc_t;
typedef struct pc pc_t;
typedef struct dice_t dice_t;

typedef struct character {
 
} character_t;

int32_t compare_characters_by_next_turn(const void *character1,
                                        const void *character2);
uint32_t can_see(dungeon_t *d, pair_t voyeur, pair_t exhibitionist);
void character_delete(void *c);

char get_symbol(const character_t *c);
void set_symbol(const character_t *c, char s);

int8_t get_char_x(const character_t *c);
void set_char_x(const character_t *c, int8_t x);

int8_t get_char_y(const character_t *c);
void set_char_y(const character_t *c, int8_t y);

int32_t get_speed(const character_t *c);
void set_speed(const character_t *c, int32_t speed);

uint32_t get_next_turn(const character_t *c);
void set_next_turn(const character_t *c, uint32_t next);

uint32_t get_alive(const character_t *c);
void set_alive(const character_t *c, uint32_t a);

uint32_t get_seqnum(const character_t *c);
void set_seqnum(const character_t *c, uint32_t seq);
    
  
#if __cplusplus
}
class Character {
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

#endif
  
#endif
