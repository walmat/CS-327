#include "dungeon_generation.h"

int run_this(int argc, char* argv[]){

  num_rooms = 0;
  
  room[0][0] = 4;
  room[0][1] = 4;
  room[1][0] = 4;
  room[1][1] = 2;
  room[2][0] = 4;
  room[2][1] = 6;
  room[3][0] = 5;
  room[3][1] = 8;
  room[4][0] = 8;
  room[4][1] = 2;
  room[5][0] = 10;
  room[5][1] = 10;
  
  generate_rock();
  int difficulty = rand_range(1,8);
  int version = 0;
  if (argc == 1){
    generate_rooms(difficulty);
    connect_rooms();
    print_screen();
  }
  else if (argc == 2){

    char* path;
    char input[100];
    printf("Please enter your file name: ");
    scanf("%s", input);

    path = (char*) malloc ((strlen(getenv("HOME")) + strlen("/.rlg327/") +
			    + strlen(input) * sizeof(char*)));

    strcat(path, getenv("HOME"));
    strcat(path, "/.rlg327/");
    strcat(path, input);

    if (strcmp(argv[1], "--load") == 0){
      //load dungeon
      printf("Reading from %s\n", path);
      load_dungeon(path, &version);
    }

    else if (strcmp(argv[1], "--save") == 0){
      //save dungeon
      printf("Writing to %s\n", path);
      generate_rooms(difficulty);
      connect_rooms();
      print_screen();
      save_dungeon(path, version);
    }

    else {
      printf("Invalid argument, arguments are --save & --load\n");
      return -1;
    }
    free(path);
  }
  else if (argc == 3){

    char* path;

    char input[100];

    if (((strcmp(argv[1], "--load") == 0) && (strcmp(argv[2], "--save") == 0)) ||
	((strcmp(argv[1], "--save") == 0) && (strcmp(argv[2], "--load") == 0))){

      printf("Please enter your file name: ");
      scanf("%s", input);

      path = (char*) malloc ((strlen(getenv("HOME"))
			      + strlen("/.rlg327/") + strlen(input)) * sizeof(char*));

      strcat(path, getenv("HOME"));
      strcat(path, "/.rlg327/");
      strcat(path, input);

      load_dungeon(path, &version);
      save_dungeon(path, version+1);
    }
    else {
      printf("Invalid argument, arguments are --save & --load\n");
      return -1;
    }
    free(path);
  }
  return 0;
}

/*
Fills the dungeon with rock later to be replaced by rooms.
 */
void generate_rock(){  
  for (int i = 0; i < 21; i++){
    for (int j = 0; j < 80; j++){
      if ((i+1 > 21) || (i-1 < 0) || (j+1 > 80) || (j-1 < 0)){
          dungeon[i][j].hardness = 255;
      }
      else {
          dungeon[i][j].hardness = rand_range(1,254);
      }
      dungeon[i][j].type = ROCK;
      dungeon[i][j].nonTun = 255;
      dungeon[i][j].tun = 255;
      dungeon[i][j].x = i;
      dungeon[i][j].y = j;
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
  int x, y = 0;
   
  //avoided integer division because infinite loop
  while ((((periodCount/grid)*100) < percentage) || (num_rooms < 5)){

    int width, height;
    int roomChoice = rand_range(1,6);

    switch (roomChoice){
    case 1  : //room 1
      width = room[0][0];
      height = room[0][1];
      break;
    case 2  : //room 2
      width = room[1][0];
      height = room[1][1];
      break;
    case 3  : //room 3
      width = room[2][0];
      height = room[2][1];
      break;
    case 4  : //room 4
      width = room[3][0];
      height = room[3][1];
      break;
    case 5  : //room 5
      width = room[4][0];
      height = room[4][1];
      break;
    default : //room 6
      width = room[5][0];
      height = room[5][1];
      break;
    }
    
    x = rand_range(1, 78-width);
    y = rand_range(1, 19-height);
        
    if (is_room_placeable(x, y, width, height) == 1){
      place_room(x, y, width, height); //place the room in the location
      periodCount += (width*height);
      rooms[num_rooms][0] = x;
      rooms[num_rooms][1] = y;
      rooms[num_rooms][2] = width;
      rooms[num_rooms][3] = height;
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
int is_room_placeable(int x, int y, int width, int height){ //1 = true 0 = false
  //check to see if any of the spaces the room will occupy contains a ROOM
  //make sure the rooms are at least 2 spaces from each other in all directions.
  for (int i = x; i < (width + x); i++){
    for (int j = y; j < (height + y); j++){
      if (dungeon[j][i].type == ROOM){
	return FALSE;
      }
      else if (dungeon[j-2][i].type == ROOM){
	return FALSE;
      }
      else if (dungeon[j+2][i].type == ROOM){
	return FALSE;
      }
      else if (dungeon[j][i-2].type == ROOM){
	return FALSE;
      }
      else if (dungeon[j][i+2].type == ROOM){
	return FALSE;
      }
      else if (dungeon[j-1][i-1].type == ROOM){
	return FALSE;
      }
      else if (dungeon[j+1][i-1].type == ROOM){
	return FALSE;
      }
      else if (dungeon[j+1][i+1].type == ROOM){
        return FALSE;
      }
      else if (dungeon[j-1][i+1].type == ROOM){
	return FALSE;
      }
    }
  }
  return TRUE;
}


/*
Places the room based on random coordinates width & height as well as the x (width) and y (height) of the room.
 */
void place_room (int x, int y, int width, int height){
  for (int i = x; i < (width + x); i++){
    for (int j = y; j < (height + y); j++){
      dungeon[j][i].type = ROOM;
      dungeon[j][i].hardness = 0;
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
      printf("%c", dungeon[i][j].type);
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


/*
Used in the selection_sort() method. Swaps the values of room1 with room2 (values = {x coord, y coord, # of periods}).
 */
void swap_rooms(int room1, int room2){

  int temp[] =
  {rooms[room1][0], rooms[room1][1], rooms[room1][2], rooms[room1][3]};

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
    if (dungeon[y][i].type != ROOM){
      dungeon[y][i].type = TUNNEL;
      dungeon[y][i].hardness = 0;
    }
  }
}

void path_up(int y1, int y2, int x){

  for (int i = y1; i >= y2; i--){
    if (dungeon[i][x].type != ROOM){
      dungeon[i][x].type = TUNNEL;
      dungeon[i][x].hardness = 0;
    }
  }
}

void path_down(int y1, int y2, int x){

  for (int i = y1; i < y2; i++){
    if (dungeon[i][x].type != ROOM){
      dungeon[i][x].type = TUNNEL;
      dungeon[i][x].hardness = 0;
    }
  }
}

int load_dungeon(char* path, int* version){
    
    char file_type[7];
    int file_version = *version;
    int file_size;
    FILE *f;
    
    f = fopen(path, "r");
    if (f == NULL){
        printf("No file at %s\n", path);
	return -1;
    }
    
    fread(file_type, 6, 1, f);
    fread(&file_version, 4, 1, f);
    fread(&file_size, 4, 1, f);

    
    file_version = be32toh(file_version);
    file_size = be32toh(file_size);
    
    for (int i = 1; i < 20; i++){
        for(int j = 1; j < 79; j++){
            fread(&dungeon[i][j].hardness, 1, 1, f);
        }
    }
    int c = 1496; //has to be 1496 at this point
    int r;
    for (r = 0; c < file_size; r++, c+=4){
      fread(&rooms[r][0],1,1,f);
      fread(&rooms[r][1],1,1,f);
      fread(&rooms[r][2],1,1,f);
      fread(&rooms[r][3],1,1,f);
    }
    num_rooms = r;
    fclose(f);
    
    //assign rooms from first room (x,y) to last room (x,y).
    for (int k = 0; k < r; k++){
        for (int y = rooms[k][0]; y < rooms[k][0] + rooms[k][2]; y++) {
            for (int x = rooms[k][1]; x < rooms[k][1] + rooms[k][3]; x++) {
	      if (dungeon[x][y].hardness == 0) {
                    dungeon[x][y].type = ROOM;
		    printf("wrote room\n");
                }
            }
        }
    }
    
    //assign tunnels
    for (int i = 0; i < 21; i++){
        for (int j = 0; j < 80; j++){
            if ((dungeon[i][j].hardness == 0) && (dungeon[i][j].type != ROOM)){
                dungeon[i][j].type = TUNNEL;
            }
	    //this should be done, but checking anyway
            else if (dungeon[i][j].hardness != 0){
                dungeon[i][j].type = ROCK;
            }
        }
    }    
    print_screen();
    return 0;
}
         
int save_dungeon(char* path, int version){

  char file_type[7] = "RLG327";
  uint32_t file_version = version;
  uint32_t file_size = 1496 + (num_rooms * 4);
  FILE *f;
             
  f = fopen(path, "w");
  if (f == NULL){
     printf("Couldn't save file at %s", path);
     return -1;
  }
  
  file_version = htobe32(file_version);
  file_size = htobe32(file_size);

  fwrite(file_type, 6, 1, f);
  fwrite(&file_version, 4, 1, f);
  fwrite(&file_size, 4, 1, f);

  //write hardness
  for (int i = 1; i < 20; i++){
    for (int j = 1; j < 79; j++){
      fwrite(&dungeon[i][j].hardness, 1, 1, f);
    }
  }
  //room information
  for (int i = 0; i < num_rooms; i++){
    fwrite(&rooms[i][0], 1, 1, f);
    fwrite(&rooms[i][1], 1, 1, f);
    fwrite(&rooms[i][2], 1, 1, f);
    fwrite(&rooms[i][3], 1, 1, f);
  }
  fclose(f);
  return 0;
}
