#include "character.h"
#include <cstdlib>

#include "heap.h"
#include "npc.h"
#include "pc.h"
#include "dungeon.h"


char get_symbol(const character_t *c)
{
  return ((Character*) c)->symbol;
}
void set_symbol(const character_t *c, char s)
{
  ((Character*) c)->symbol = s;
}

int8_t get_char_x(const character_t *c)
{
  return ((Character*) c)->position[dim_x];
}
void set_char_x(const character_t *c, int8_t x)
{
  ((Character*) c)->position[dim_x] = x;
}

int8_t get_char_y(const character_t *c)
{
  return ((Character*) c)->position[dim_y];
}
void set_char_y(const character_t *c, int8_t y)
{
  ((Character*) c)->position[dim_y] = y;
}

int32_t get_speed(const character_t *c)
{
  return ((Character*) c)->speed;
}
void set_speed(const character_t *c, int32_t speed)
{
  ((Character*) c)->speed = speed;
}

uint32_t get_next_turn(const character_t *c)
{
  return ((Character*) c)->next_turn;
}
void set_next_turn(const character_t *c, uint32_t next)
{
  ((Character*) c)->next_turn = next;
}

uint32_t get_alive(const character_t *c)
{
  return ((Character*) c)->alive;
}
void set_alive(const character_t *c, uint32_t a)
{
  ((Character*) c)->alive = a;
}

uint32_t get_seqnum(const character_t *c)
{
  return ((Character*) c)->sequence_number;
}
void set_seqnum(const character_t *c, uint32_t seq)
{
  ((Character*) c)->sequence_number = seq;
}

char *print_character(const void *v)
{
  const character_t *c = (character_t*) v;
    
    static char string[80];
    
    snprintf(string, 80, "%d:%d", get_next_turn(c), get_seqnum(c));
    
    return string;
}

void character_delete(void *v)
{
    /* The PC is never malloc()ed anymore, do don't attempt to free it here. */
  character_t* c;
  
  if (v) {
    c= (character_t*) v;
    free(c);
    //npc_delete((npc_t*) v);
  }
}

int32_t compare_characters_by_next_turn(const void *character1,
                                        const void *character2)
{
    int32_t diff;
    
    diff = get_next_turn(((character_t*) character1)) - get_next_turn(((character_t*) character2));
    
    return diff ? diff : get_seqnum(((character_t *) character1)) - get_seqnum(((character_t *) character2));
    
    /*
     diff = (((character_t *) character1)->next_turn -
     ((character_t *) character2)->next_turn);
     return diff ? diff : (((character_t *) character1)->sequence_number -
     ((character_t *) character2)->sequence_number);
     */
}

uint32_t can_see(dungeon_t *d, pair_t voyeur, pair_t exhibitionist)
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
    
    first[dim_x] = voyeur[dim_x];
    first[dim_y] = voyeur[dim_y];
    second[dim_x] = exhibitionist[dim_x];
    second[dim_y] = exhibitionist[dim_y];
    
    if ((abs(first[dim_x] - second[dim_x]) > VISUAL_RANGE) ||
        (abs(first[dim_y] - second[dim_y]) > VISUAL_RANGE)) {
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


