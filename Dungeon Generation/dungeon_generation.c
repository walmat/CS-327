#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
//consts
#define TRUE 1
#define FALSE 0

//members
char ROOM = '.';
char ROCK = ' ';
char TUNNEL = '#';
int num_rooms = 0;
char dungeon[21][80];
int room[5][2] = {{4,4}, {4,2}, {4,6}, {5,8}, {8,2}};
int rooms[60][4];

//functions
void connect_rooms();
int rand_range(int min, int max);
void generate_rooms(int difficulty);
void generate_rock();
int is_room_placeable(int width, int height, int x, int y);
void place_room(int width, int height, int x, int y);
void choose_room();
void print_screen();
void swap_rooms(int room1, int room2);
void selection_sort();
void path_down(int y1, int y2, int x);
void path_right(int x1, int x2, int y);
void path_up(int y1, int y2, int x);

/*
Run this. Enter a difficulty like so: ./dungeon_generation 5
 */
int main (int argc, char* argv[]){

  if (argc != 2){
    printf("Please enter a difficulty: (1-8)\n");
    return -1;
  }
  
  int difficulty = atoi(argv[1]);

  if (difficulty > 8 || difficulty < 1){
    printf("Please enter a difficulty between 1 and 8\n");
    return -1;
  }
  srand(time(NULL));
  generate_rock();
  generate_rooms(difficulty);
  connect_rooms();
  print_screen();
  /*
  for(int i = 0; i < 60; i++){
    if (rooms[i][0] != 0){
      printf("location: (%d, %d)\t width: %d height: %d\n", rooms[i][0], rooms[i][1], rooms[i][2], rooms[i][3]);
    }
  }*/
  return 0;
  
}


/*
Fills the dungeon with rock later to be replaced by rooms.
 */
void generate_rock(){
  
  for (int i = 0; i < 21; i++){
    for (int j = 0; j < 80; j++){
      dungeon[i][j] = ROCK;
    }
  }
}

/*
Generates a random room from room[5][2] array. Attempts to place it at random (x,y) coordinates
and iterates until the dungeon is generated based on the difficulty.
 */
void generate_rooms(int difficulty){
 
  //percentage of the dungeon to fill
  //base case of 7% with max case of 19%
  int percentage = 4 + (3 * difficulty);
  float grid = 1482.0f; //grid width
  float periodCount = 0.0f;
  int width, height = 0;
   
  //avoided integer division because infinite loop
  while (((periodCount/grid)*100) < percentage){

    int x, y;
    int roomChoice = rand_range(1,5);
    
    switch (roomChoice){
    case 1  : //room 1
      x = room[0][0];
      y = room[0][1];
      break;
    case 2  : //room 2
      x = room[1][0];
      y = room[1][1];
      break;
    case 3  : //room 3
      x = room[2][0];
      y = room[2][1];
      break;
    case 4  : //room 4
      x = room[3][0];
      y = room[3][1];
      break;
    default : //room 5
      x = room[4][0];
      y = room[4][1];
      break;
    }
    width = rand_range(1, 78-x);
    height = rand_range(1, 19-y);
        
    if (is_room_placeable(width, height, x, y) == 1){
      place_room(width, height, x, y); //place the room in the location
      periodCount += (x*y);
      rooms[num_rooms][0] = width;
      rooms[num_rooms][1] = height;
      rooms[num_rooms][2] = x;
      rooms[num_rooms][3] = y;
      num_rooms++;
    }
  }
  selection_sort();
}

/*
Simple function used for pseudorandom room placement as well as room coordinates.
 */
int rand_range(int min, int max){
  return(rand() % (max-min + 1)) + min;
}


/*
Checks to see if the room in process of being placed is at least 2 spaces in (+- xy) as
well as 1 space in all diagonals from another room.
 */
int is_room_placeable(int width, int height, int x, int y){ //1 = true 0 = false
  //check to see if any of the spaces the room will occupy contains a ROOM
  //make sure the rooms are at least 2 spaces from each other in all directions.
  for (int i = width; i < (width + x); i++){
    for (int j = height; j < (height + y); j++){
      if (dungeon[j][i] == ROOM){
	return FALSE;
      }
      else if (dungeon[j-2][i] == ROOM){
	return FALSE;
      }
      else if (dungeon[j+2][i] == ROOM){
	return FALSE;
      }
      else if (dungeon[j][i-2] == ROOM){
	return FALSE;
      }
      else if (dungeon[j][i+2] == ROOM){
	return FALSE;
      }
      else if (dungeon[j-1][i-1] == ROOM){
	return FALSE;
      }
      else if (dungeon[j+1][i-1] == ROOM){
	return FALSE;
      }
      else if (dungeon[j+1][i+1] == ROOM){
        return FALSE;
      }
      else if (dungeon[j-1][i+1] == ROOM){
	return FALSE;
      }
    }
  }
  return TRUE;
}


/*
Places the room based on random coordinates width & height as well as the x (width) and y (height) of the room.
 */
void place_room (int width, int height, int x, int y){
  for (int i = width; i < (width + x); i++){
    for (int j = height; j < (height + y); j++){
      dungeon[j][i] = ROOM;      
    }
  }
}


/*
Updates the screen every call print_screen(). Useful for when we have movement.
 */
void print_screen(){

  system("clear");
  for (int i = 0; i < 21; i++){
    for (int j = 0; j < 80; j++){
      printf("%c", dungeon[i][j]);
    }
    printf("\n");
  }
}

/*
Loops through rooms_unsorted after selection sort is called and draws cooridors to the nearest room
based on a radius around it.
 */
void connect_rooms(){

  for (int i = 0; i < num_rooms-1; i++){
    if (i + 1 > num_rooms){
      //connect rooms_unsorted[i] with rooms_unsorted[num_rooms]
      
      }
    else {
      //connect rooms_unsorted[i] with rooms_unsorted[i+1].
      if (rooms[i][1]+ rooms[i][3]/2 < rooms[i+1][1] + rooms[i+1][3]/2){
	//path down then right
	path_down(rooms[i][1] + rooms[i][3]/2,rooms[i+1][1] + rooms[i+1][3]/2, rooms[i][0] + rooms[i][2]/2);
	path_right(rooms[i][0] + rooms[i][2]/2, rooms[i+1][0] + rooms[i+1][2]/2, rooms[i+1][1] + rooms[i+1][3]/2);
      }
      else if (rooms[i][1] + rooms[i][3]/2 == rooms[i+1][1] + rooms[i+1][3]/2){
	//path right
	path_right(rooms[i][0] + rooms[i][2]/2, rooms[i+1][0] + rooms[i+1][2]/2, rooms[i+1][1] + rooms[i+1][3]/2);
      }
      else {
	//path up then right
	path_up(rooms[i][1] + rooms[i][3]/2, rooms[i+1][1] + rooms[i+1][3]/2, rooms[i][0] + rooms[i][2]/2);
	path_right(rooms[i][0] + rooms[i][2]/2, rooms[i+1][0] + rooms[i+1][2]/2, rooms[i+1][1] + rooms[i+1][3]/2);
	}
    }
  }
}


/*
Used in the selection_sort() method. Swaps the values of room1 with room2 (values = {x coord, y coord, # of periods}).
 */
void swap_rooms(int room1, int room2){

  int temp[] = {rooms[room1][0], rooms[room1][1], rooms[room1][2], rooms[room1][3]};

  rooms[room1][0] = rooms[room2][0];
  rooms[room1][1] = rooms[room2][1];
  rooms[room1][2] = rooms[room2][2];
  rooms[room1][3] = rooms[room2][3];
  
  rooms[room2][0] = temp[0];
  rooms[room2][1] = temp[1];
  rooms[room2][2] = temp[2];
  rooms[room2][3] = temp[3];
  
}


/*
Need to sort the unsorted array based on ascending x coordinates and in the event of a tie,
sort based on ascending y coordinates. Useful for drawing cooridors to the nearest room.
 */
void selection_sort(){

  for (int i = 0; i < num_rooms; i++){

    int min[] = {rooms[i][0], rooms[i][1], rooms[i][2], rooms[i][3]};
    int index = i;
    for (int j = i+1; j < num_rooms; j++){

      if ((rooms[j][0] > min[0]) ||
	  (rooms[j][0] == min[0] && rooms[j][1] >= min[1])){
	continue;
      }

      min[0] = rooms[j][0];
      min[1] = rooms[j][1];
      min[2] = rooms[j][2];
      min[3] = rooms[j][3];
      index = j;
    }
    if (index != i){
      swap_rooms(i, index);
    }
  }
}

void path_right(int x1, int x2, int y){

  for (int i = x1; i < x2; i++){
    if (dungeon[y][i] != ROOM){
      dungeon[y][i] = TUNNEL;
    }
  }
}

void path_up(int y1, int y2, int x){

  for (int i = y1; i >= y2; i--){
    if (dungeon[i][x] != ROOM){
      dungeon[i][x] = TUNNEL;
    }
  }
}

void path_down(int y1, int y2, int x){

  for (int i = y1; i < y2; i++){
    if (dungeon[i][x] != ROOM){
      dungeon[i][x] = TUNNEL;
    }
  }
}
