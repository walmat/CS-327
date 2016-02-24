#include "dungeon_generation.h"
#include "binheap.h"

#define WIDTH 21
#define HEIGHT 80

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

  for(int i = 0; i < WIDTH; i++){
    for(int j = 0; j < HEIGHT; j++){
      if(dungeon[i][j].hardness == 0){
	dungeon[i][j].type = '@';
	dungeon[i][j].tun = 0;
	dungeon[i][j].nonTun = 0;
	goto start;
      }
    }
  }

 start:
  
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

int main(int argc, char* argv[]){
  srand(time(NULL));
  run_this(argc, argv);
  calc_dist();
  print_screen();
  print_arry(0);
  print_arry(1);
}
