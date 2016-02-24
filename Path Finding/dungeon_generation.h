//
//  dungeon_generation.h
//
//
//  Created by Matthew Wall on 2/7/16.
//
//

#ifndef dungeon_generation_h
#define dungeon_generation_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

//consts
#define TRUE 1
#define FALSE 0
#define ROOM '.'
#define ROCK ' '
#define TUNNEL '#'

int num_rooms;
int room[6][2];
int rooms[60][4];
typedef struct {
  char type;
  int hardness;
  int nonTun;
  int tun;
  int x, y;
}tile_t;
tile_t dungeon[21][80];

//functions
void connect_rooms();
int rand_range(int min, int max);
void generate_rooms(int difficulty);
void generate_rock();
int is_room_placeable(int x, int y, int width, int height);
void place_room(int x, int y, int width, int height);
void print_screen();
void swap_rooms(int room1, int room2);
void selection_sort();
void path_down(int y1, int y2, int x);
void path_right(int x1, int x2, int y);
void path_up(int y1, int y2, int x);
int save_dungeon(char* path, int version);
int load_dungeon(char* path, int* version);
int run_this(int argc, char* argv[]);

#endif
