//
//  dungeon_generation.h
//
//  Created by Matthew Wall on 2/7/16.
//

#ifndef dungeon_generation_h
#define dungeon_generation_h

void connect_rooms();
void generate_rooms(int difficulty);
void generate_rock();
int is_room_placeable(int x, int y, int width, int height);
void place_room(int x, int y, int width, int height);
void swap_rooms(int room1, int room2);
void path_down(int y1, int y2, int x);
void path_right(int x1, int x2, int y);
void path_up(int y1, int y2, int x);

#endif
