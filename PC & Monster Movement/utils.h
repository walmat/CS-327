#ifndef utils_h
#define utils_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define TRUE 1
#define FALSE 0
#define ROOM '.'
#define ROCK ' '
#define TUNNEL '#'

int num_rooms;
int num_monsters;
int room[6][2];
int rooms[60][4];

typedef struct {
  char current;
  int attributes;
  int isNPC;
  int x,y;
  int speed;
  int isDead;
  int next_turn;
  int age;
} character;

character player;
character monsters[50];

typedef struct {
  char type;
  int hardness;
  int nonTun;
  int tun;
  int x,y;
  character* occupant;
} tile_t;

tile_t dungeon[21][80];

//functions
int rand_range(int min, int max);
void print_screen();
void selection_sort();
int save_dungeon(char* path, int version);
int load_dungeon(char* path, int* version);
int run_this(int argc, char* argv[]);

#endif /* utils_h */
