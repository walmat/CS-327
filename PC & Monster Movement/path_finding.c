#include "binheap.h"
#include "utils.h"

#define WIDTH 21
#define HEIGHT 80

void redraw_PC(int x2, int y2);
void get_attributes(character *m);
void pick_monster(character *m);

int32_t dist_nonTun(const void *key, const void *with){
  return ((tile_t*) key)->nonTun - ((tile_t*) with)->nonTun;
}

int32_t dist_tun(const void *key, const void *with){
  return ((tile_t*) key)->tun - ((tile_t*) with)->tun;
}

void init_binheap(binheap_node_t *n[WIDTH][HEIGHT], binheap_t *h){
  int i, j;
  for (i = 0; i < WIDTH; i++){
    for (j = 0; j < HEIGHT; j++){
      n[i][j] = binheap_insert(h, &dungeon[i][j]);
    }
  }
}

void place_PC(){
  for(int i = 0; i < WIDTH; i++){
    for(int j = 0; j < HEIGHT; j++){
     //place player character
      if(dungeon[i][j].hardness == 0){
	dungeon[i][j].tun = 0;
	dungeon[i][j].nonTun = 0;
	dungeon[i][j].occupant = &player;
	player.isNPC = 0;
	player.x = i;
	player.y = j;
	player.age = 0;
	player.isDead = 0;
	player.next_turn = 0;
	player.current = '@';
	return;
      }
    }
  }
}

void place_monsters(character* m){

  while(1){
    int x = rand_range(1,20);
    int y = rand_range(1,79);
    //place player character
    if(dungeon[x][y].hardness == 0 && dungeon[x][y].occupant == NULL){
      dungeon[x][y].tun = 0;
      dungeon[x][y].nonTun = 0;
      dungeon[x][y].occupant = m;
      m->isNPC = 1;
      m->isDead = 0;
      m->age = 0;
      m->next_turn = 0;
      m->x = x;
      m->y = y;
      get_attributes(m);
      pick_monster(m);
      return;
    }
  }
}

void get_attributes(character *m){

  if (rand() % 2){
    //intelligence
    m->attributes |= 0x01;
  }
  if (rand() % 2){
    //telepathy
    m->attributes |= 0x02;
  }
  if (rand() % 2){
    //tunneling
    m->attributes |= 0x04;
  }
  if (rand() % 2){
    //erratic
    m->attributes |= 0x08;
  }
}

void pick_monster(character *m){

  switch(m->attributes){

  case 0:
    //basic bitch
    m->current = '0';
    m->speed = rand_range(5,20);
    return;
  case 1:
    //intelligence
    m->current = '1';
    m->speed = rand_range(5,20);
    return;
  case 2:
    //telepathy
    m->current = '2';
    m->speed = rand_range(5,20);
    return;
  case 3:
    //intelligence + telepathy
    m->current = '3';
    m->speed = rand_range(5,20);
    return;
  case 4:
    //tunneling
    m->current = '4';
    m->speed = rand_range(5,20);
    return;
  case 5:
    //tunneling + intelligence
    m->current = '5';
    m->speed = rand_range(5,20);
    return;
  case 6:
    //tunneling + telepathy
    m->current = '6';
    m->speed = rand_range(5,20);
    return;
  case 7:
    //tunneling + intelligence + telepathy
    m->current = '7';
    m->speed = rand_range(5,20);
    return;
  case 8:
    //erratic
    m->current = '8';
    m->speed = rand_range(5,20);
    return;
  case 9:
    //erratic + intelligence
    m->current = '9';
    m->speed = rand_range(5,20);
    return;
  case 10:
    //erratic + telepathy
    m->current = 'a';
    m->speed = rand_range(5,20);
    return;
  case 11:
    //erratic + intelligence + telepathy
    m->current = 'b';
    m->speed = rand_range(5,20);
    return;
  case 12:
    //erratic + tunneling
    m->current = 'c';
    m->speed = rand_range(5,20);
    return;
  case 13:
    //erratic + tunneling + intelligence
    m->current = 'd';
    m->speed = rand_range(5,20);
    return;
  case 14:
    //erratic + tunneling + telepathy
    m->current = 'e';
    m->speed = rand_range(5,20);
    return;
  case 15:
    //erratic + tunneling + telepathy + intelligence
    m->current = 'f';
    m->speed = rand_range(5,20);
    return;
  }
}  

int is_visible(character *m){

  for (int i = 0; i < num_rooms; i++){

    //room check
    if ((m->x >= rooms[i][0] && m->x <= rooms[i][0] + rooms[i][2]) && (player.x >= rooms[i][0] && player.x <= rooms[i][0] + rooms[i][2])){
      if ((m-> y >= rooms[i][1] && m->y <= rooms[i][1] + rooms[i][3]) && (player.y >= rooms[i][1] && player.y <= rooms[i][1] + rooms[i][3])){
	return TRUE;
      }
    }

    //in different rooms
    if (m->x == player.x){
      if (m->y > player.y){
	for (int i = player.y; i < m->y; i++){
	  if (dungeon[i][player.x].type == ROCK){
	    return FALSE;
	  }
	}
	return TRUE;
      } else {
	for (int i = m->y; i < player.y; i++){
	  if (dungeon[i][m->x].type == ROCK){
	    return FALSE;
	  }
	}
      }
      return TRUE;
    }
    if (m->y == player.y){
      if (m->x > player.x){
	for(int i = player.x; i < m->x; i++){
	  if (dungeon[player.y][i].type == ROCK){
	    return FALSE;
	  }
	}
	return TRUE;
      } else {
	for (int i = m->x; i < player.x; i++){
	  if (dungeon[m->y][i].type == ROCK){
	    return FALSE;
	  }
	}
      }
      return TRUE;
    }
  }
  return FALSE;
}



void move_monsters(character *m){

  switch(m->attributes){

  case 0:
    //basic bitch
    if (is_visible(m)) {
      //move up
      if (m->y > player.y){
	m->y -= 1;
      }
      //move up to the right
      else if (m->y > player.y && m->x < player.x){
	m->x += 1;
	m->y -= 1;
      }
      //move right
      else if (m->y == player.y && m->x < player.x){
	m->x += 1;
      }
      //move down and to the right
      else if (m->y < player.y && m->x < player.x){
	m->x += 1;
	m->y += 1;
      }
      //move down
      else if (m->y < player.y && m->x == player.x){
	m->y += 1;
      }
      //move down and to the left
      else if (m->y < player.y && m->x > player.x){
	m->x -= 1;
	m->y += 1;
      }
      //move left
      else if (m->y == player.y && m->x > player.x){
	m->x -= 1;
      }
      //move up and left
      else {
	m->x -= 1;
	m->y -= 1;
      }
    }
    return;
  case 1:
    //intelligence
    return;
  case 2:
    //telepathy
    return;
  case 3:
    //intelligence + telepathy
    return;
  case 4:
    //tunneling
    return;
  case 5:
    //tunneling + intelligence
    return;
  case 6:
    //tunneling + telepathy
    return;
  case 7:
    //tunneling + intelligence + telepathy
    return;
  case 8:
    //erratic
    return;
  case 9:
    //erratic + intelligence
    return;
  case 10:
    //erratic + telepathy
    return;
  case 11:
    //erratic + intelligence + telepathy
    return;
  case 12:
    //erratic + tunneling
    return;
  case 13:
    //erratic + tunneling + intelligence
    return;
  case 14:
    //erratic + tunneling + telepathy
    return;
  case 15:
    //erratic + tunneling + telepathy + intelligence
    return;
  }
}

void reset_distances(){
  for (int i = 0; i < WIDTH; i++){
    for (int j = 0; j < HEIGHT; j++){
      dungeon[i][j].nonTun = 255;
      dungeon[i][j].tun = 255;
    }
  }
}


void print_arry(int ability){
  printf("\n");
  int i, j;

  for(i = 0; i < WIDTH; i++){
    for (j = 0; j < HEIGHT; j++){

      int dist = (ability) ? dungeon[i][j].tun : dungeon[i][j].nonTun;
      
      printf("%c", dist < 10 ? 48 + dist : dist < 36 ? 97 + (dist-10) : dist < 62 ? 65 + (dist-36) : dist < 255 ? 'Z' : ' ');
      
    }
    printf("\n");
  }
}

void  calc_dist(){

  binheap_t h;
  binheap_node_t *n[WIDTH][HEIGHT];

  reset_distances();

  dungeon[player.x][player.y].nonTun = 0;
  dungeon[player.x][player.y].tun = 0;
  
  binheap_init(&h, dist_nonTun, NULL);
  init_binheap(n, &h);
  //non tunneling ability
  while(!binheap_is_empty(&h)){
    tile_t* min = (tile_t*)binheap_peek_min(&h);
    int dist = ((min)-> nonTun) + 1;
    int x = (min)->x;
    int y = (min)->y;

    if (x - 1 >= 0) {
      if (dungeon[x-1][y].hardness == 0){
	if (dungeon[x-1][y].nonTun > dist){
	  dungeon[x-1][y].nonTun = dist;
	  binheap_decrease_key(&h, n[x-1][y]);
	}
      }
    }
    if (x + 1 < WIDTH) {
      if (dungeon[x+1][y].hardness == 0){
	if (dungeon[x+1][y].nonTun > dist){
	  dungeon[x+1][y].nonTun = dist;
	  binheap_decrease_key(&h, n[x+1][y]);
	}
      }
    }
    if (y - 1 >= 0) {
      if (dungeon[x][y-1].hardness == 0){
	if (dungeon[x][y-1].nonTun > dist){
       	  dungeon[x][y-1].nonTun = dist;
	  binheap_decrease_key(&h, n[x][y-1]);
	}
      }
    }
    if (y + 1 < HEIGHT) {
      if (dungeon[x][y+1].hardness == 0){
	if (dungeon[x][y+1].nonTun > dist){
	  dungeon[x][y+1].nonTun = dist;
	  binheap_decrease_key(&h, n[x][y+1]);
	}
      }
    }
    if (x + 1 < WIDTH && y + 1 < HEIGHT) {
      if (dungeon[x+1][y+1].hardness == 0){
	if (dungeon[x+1][y+1].nonTun > dist){
	  dungeon[x+1][y+1].nonTun = dist;
	  binheap_decrease_key(&h, n[x+1][y+1]);
	}
      }
    }
    if (x + 1 < WIDTH && y - 1 >= 0) {
      if (dungeon[x+1][y-1].hardness == 0){
	if (dungeon[x+1][y-1].nonTun > dist){
	  dungeon[x+1][y-1].nonTun = dist;
	  binheap_decrease_key(&h, n[x+1][y-1]);
	}
      }
    }
    if (x - 1 >= 0 && y + 1 < HEIGHT) {
      if (dungeon[x-1][y+1].hardness == 0){
	if (dungeon[x-1][y+1].nonTun > dist){
	  dungeon[x-1][y+1].nonTun = dist;
	  binheap_decrease_key(&h, n[x-1][y+1]);
	}
      }
    }
    if (x - 1 >= 0 && y - 1 >= 0) {
      if (dungeon[x-1][y-1].hardness == 0){
	if (dungeon[x-1][y-1].nonTun > dist){
	  dungeon[x-1][y-1].nonTun = dist;
	  binheap_decrease_key(&h, n[x-1][y-1]);
	}
      }
    }
    binheap_remove_min(&h);
  }

  binheap_init(&h, dist_tun, NULL);
  init_binheap(n, &h);
  
  //tunnel ability
  while(!binheap_is_empty(&h)){
    tile_t* min = (tile_t*)binheap_peek_min(&h);
    int dist = (min)-> tun;
    int weight_dist;
    int x = (min)->x;
    int y = (min)->y;

    if (x - 1 >= 0) {
      if (dungeon[x-1][y].hardness < 255){
	weight_dist =
	  (dungeon[x-1][y].hardness >= 0 && dungeon[x-1][y].hardness <= 84) ? (dist + 1):
	  (dungeon[x-1][y].hardness > 84 && dungeon[x-1][y].hardness <= 170) ? (dist + 2):
	  (dist + 3);
	if (dungeon[x-1][y].tun > weight_dist){
	  dungeon[x-1][y].tun = weight_dist;
	  binheap_decrease_key(&h, n[x-1][y]);
	}
      }
    }
    if (x + 1 < WIDTH) {
      if (dungeon[x+1][y].hardness < 255){
	weight_dist =
	(dungeon[x+1][y].hardness >= 0 && dungeon[x+1][y].hardness <= 84) ? (dist + 1):
      (dungeon[x+1][y].hardness > 84 && dungeon[x+1][y].hardness <= 170) ? (dist + 2):
	  (dist + 3);
	if (dungeon[x+1][y].tun > weight_dist){
	  dungeon[x+1][y].tun = weight_dist;
	  binheap_decrease_key(&h, n[x+1][y]);
	}
      }
    }
    if (y - 1 >= 0) {
      if (dungeon[x][y-1].hardness < 255){
	weight_dist = 
	(dungeon[x][y-1].hardness >= 0 && dungeon[x][y-1].hardness <= 84) ? (dist + 1):
      (dungeon[x][y-1].hardness > 84 && dungeon[x][y-1].hardness <= 170) ? (dist + 2):
	  (dist + 3);
	if (dungeon[x][y-1].tun > weight_dist){
       	  dungeon[x][y-1].tun = weight_dist;
	  binheap_decrease_key(&h, n[x][y-1]);
	}
      }
    }
    if (y + 1 < HEIGHT) {
      if (dungeon[x][y+1].hardness < 255){
	weight_dist =
        (dungeon[x][y+1].hardness >= 0 && dungeon[x][y+1].hardness <= 84) ? (dist + 1):
      (dungeon[x][y+1].hardness > 84 && dungeon[x][y+1].hardness <= 170) ? (dist + 2):
	  (dist + 3);
	if (dungeon[x][y+1].tun > weight_dist){
	  dungeon[x][y+1].tun = weight_dist;
	  binheap_decrease_key(&h, n[x][y+1]);
	}
      }
    }
    if (x + 1 < WIDTH && y + 1 < HEIGHT) {
      if (dungeon[x+1][y+1].hardness < 255){
	weight_dist =
    (dungeon[x+1][y+1].hardness >= 0 && dungeon[x+1][y+1].hardness <= 84) ? (dist + 1):
  (dungeon[x+1][y+1].hardness > 84 && dungeon[x+1][y+1].hardness <= 170) ? (dist + 2):
	  (dist + 3);
	if (dungeon[x+1][y+1].tun > weight_dist){
	  dungeon[x+1][y+1].tun = weight_dist;
	  binheap_decrease_key(&h, n[x+1][y+1]);
	}
      }
    }
    if (x + 1 < WIDTH && y - 1 >= 0) {
      if (dungeon[x+1][y-1].hardness < 255){
	weight_dist =
    (dungeon[x+1][y-1].hardness >= 0 && dungeon[x+1][y-1].hardness <= 84) ? (dist + 1):
  (dungeon[x+1][y-1].hardness > 84 && dungeon[x+1][y-1].hardness <= 170) ? (dist + 2):
	  (dist + 3);
	if (dungeon[x+1][y-1].tun > weight_dist){
	  dungeon[x+1][y-1].tun = weight_dist;
	  binheap_decrease_key(&h, n[x+1][y-1]);
	}
      }
    }
    if (x - 1 >= 0 && y + 1 < HEIGHT) {
      if (dungeon[x-1][y+1].hardness < 255){
	weight_dist =
    (dungeon[x-1][y+1].hardness >= 0 && dungeon[x-1][y+1].hardness <= 84) ? (dist + 1):
  (dungeon[x-1][y+1].hardness > 84 && dungeon[x-1][y+1].hardness <= 170) ? (dist + 2):
	  (dist + 3);
	if (dungeon[x-1][y+1].tun > weight_dist){
	  dungeon[x-1][y+1].tun = weight_dist;
	  binheap_decrease_key(&h, n[x-1][y+1]);
	}
      }
    }
    if (x - 1 >= 0 && y - 1 >= 0) {
      if (dungeon[x-1][y-1].hardness < 255){
	weight_dist =
    (dungeon[x-1][y-1].hardness >= 0 && dungeon[x-1][y-1].hardness <= 84) ? (dist + 1):
  (dungeon[x-1][y-1].hardness > 84 && dungeon[x-1][y-1].hardness <= 170) ? (dist + 2):
	  (dist + 3);
	if (dungeon[x-1][y-1].tun > weight_dist){
	  dungeon[x-1][y-1].tun = weight_dist;
	  binheap_decrease_key(&h, n[x-1][y-1]);
	}
      }
    }
    binheap_remove_min(&h);
  }

  binheap_delete(&h);
}

void move_PC(){

  for (int i = 0; i < WIDTH; i++){
    for (int j = 0; j < HEIGHT; j++){
      if (dungeon[i][j].occupant != NULL && (dungeon[i][j].occupant)->current == '@'){
	//choose direction
	//dungeon[i][j].occupant = NULL;
	int x = rand_range(1,8);
	switch(x){
	  //move up
	case 1:
	  if(dungeon[i][j-1].hardness < 255){
	    redraw_PC(i,j-1);
	  }
	  return;
	  //move up-right
	case 2:
	  if (dungeon[i+1][j-1].hardness < 255){
	    redraw_PC(i+1,j-1);
	  }
	  return;
	  //move right
	case 3:
	  if (dungeon[i+1][j].hardness < 255){
	    redraw_PC(i+1,j);
	  }
	  return;
	  //move down-right
	case 4:
	  if (dungeon[i+1][j+1].hardness < 255){
	    redraw_PC(i+1,j+1);
	  }
	  return;
	  //move down
	case 5:
	  if (dungeon[i][j+1].hardness < 255){
	    redraw_PC(i,j+1);
	  }
	  return;
	  //move down-left
	case 6:
	  if (dungeon[i-1][j+1].hardness < 255){
	    redraw_PC(i-1,j+1);
	  }
	  return;
	  //move left
	case 7:
	  if (dungeon[i-1][j].hardness < 255){
	    redraw_PC(i-1,j);
	  }
	  return;
	  //move up-left
	case 8:
	  if (dungeon[i-1][j-1].hardness < 255){
	    redraw_PC(i-1,j-1);
	  }
	  return;
	}
      }
    }
  }
}

void redraw_PC(int x2, int y2){

  /*
    (x2, y2) = new
   */

  dungeon[player.x][player.y].occupant = NULL;
  dungeon[x2][y2].occupant = &player;
  player.x = x2;
  player.y = y2;
  if (dungeon[x2][y2].hardness > 0 && dungeon[x2][y2].hardness < 255){
    dungeon[x2][y2].type = TUNNEL;
    dungeon[x2][y2].hardness = 0;
  }
}


int32_t sort_queue(const void* key, const void* with){
  if (((character*) key)->next_turn != ((character*) with)->next_turn){
    return ((character*)key)->next_turn - ((character*)with)->next_turn;
  }
  return ((character*) key)->age - ((character*) with)->age;
}
  


int main(int argc, char* argv[]){
  srand(time(NULL));
  binheap_t h;
  binheap_init(&h, sort_queue, NULL);
  run_this(argc, argv);
  place_PC();
  binheap_insert(&h, &player);
  for (int i = 0; i < num_monsters; i++){
    place_monsters(&monsters[i]);
    binheap_insert(&h, &monsters[i]);
  }
  while (!binheap_is_empty(&h) && h.size > 1 && !player.isDead){
    fflush(stdout);
    system("clear");
    character* min = (character*) binheap_peek_min(&h);
    if (min->isNPC){
      move_monsters(min);
      binheap_remove_min(&h);
      binheap_insert(&h, min);
    }
    else {
      move_PC();
      binheap_remove_min(&h);
      binheap_insert(&h, min);
    }
    calc_dist();
    print_screen();
    print_arry(0);
    print_arry(1);
    printf("turn: %c, isNPC: %d\n", min->current, min->isNPC);
    usleep(125000);
  }
}
