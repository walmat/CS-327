#include <stdio.h>
#include <stdint.h>
#include <endian.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>

#include "dungeon.h"
#include "utils.h"
#include "heap.h"
#include "pc.h"
#include "npc.h"

typedef struct corridor_path {
  heap_node_t *hn;
  uint8_t pos[2];
  uint8_t from[2];
  /* Because paths can meander about the dungeon, they can be *
   * significantly longer than DUNGEON_X.                     */
  int32_t cost;
} corridor_path_t;

static int32_t corridor_path_cmp(const void *key, const void *with) {
  return ((corridor_path_t *) key)->cost - ((corridor_path_t *) with)->cost;
}

/* This is the same basic algorithm as in move.c, but *
 * subtle differences make it difficult to reuse.     */
static void dijkstra_corridor(dungeon_t *d, pair_t from, pair_t to)
{
  static corridor_path_t path[DUNGEON_Y][DUNGEON_X], *p;
  static uint32_t initialized = 0;
  heap_t h;
  uint32_t x, y;

  if (!initialized) {
    for (y = 0; y < DUNGEON_Y; y++) {
      for (x = 0; x < DUNGEON_X; x++) {
        path[y][x].pos[dim_y] = y;
        path[y][x].pos[dim_x] = x;
      }
    }
    initialized = 1;
  }
  
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      path[y][x].cost = INT_MAX;
    }
  }

  path[from[dim_y]][from[dim_x]].cost = 0;

  heap_init(&h, corridor_path_cmp, NULL);

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      if (mapxy(x, y) != ter_wall_immutable) {
        path[y][x].hn = heap_insert(&h, &path[y][x]);
      } else {
        path[y][x].hn = NULL;
      }
    }
  }

  while ((p = heap_remove_min(&h))) {
    p->hn = NULL;

    if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x]) {
      for (x = to[dim_x], y = to[dim_y];
           (x != from[dim_x]) || (y != from[dim_y]);
           p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y]) {
        if (mapxy(x, y) != ter_floor_room) {
          mapxy(x, y) = ter_floor_hall;
          hardnessxy(x, y) = 0;
        }
      }
      heap_delete(&h);
      return;
    }
    if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost >
         p->cost + (hardnesspair(p->pos) ? hardnesspair(p->pos) : 8) +
         ((p->pos[dim_x] != from[dim_x]) ? 48  : 0))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost =
        p->cost + (hardnesspair(p->pos) ? hardnesspair(p->pos) : 8) +
         ((p->pos[dim_x] != from[dim_x]) ? 48  : 0);
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1]
                                           [p->pos[dim_x]    ].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost >
         p->cost + (hardnesspair(p->pos) ? hardnesspair(p->pos) : 8) +
         ((p->pos[dim_y] != from[dim_y]) ? 48  : 0))) {
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost =
        p->cost + (hardnesspair(p->pos) ? hardnesspair(p->pos) : 8) +
         ((p->pos[dim_y] != from[dim_y]) ? 48  : 0);
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] - 1].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost >
         p->cost + (hardnesspair(p->pos) ? hardnesspair(p->pos) : 8) +
         ((p->pos[dim_y] != from[dim_y]) ? 48  : 0))) {
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost =
        p->cost + (hardnesspair(p->pos) ? hardnesspair(p->pos) : 8) +
         ((p->pos[dim_y] != from[dim_y]) ? 48  : 0);
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] + 1].hn);
    }
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost >
         p->cost + (hardnesspair(p->pos) ? hardnesspair(p->pos) : 8) +
         ((p->pos[dim_x] != from[dim_x]) ? 48  : 0))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost =
        p->cost + (hardnesspair(p->pos) ? hardnesspair(p->pos) : 8) +
         ((p->pos[dim_x] != from[dim_x]) ? 48  : 0);
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1]
                                           [p->pos[dim_x]    ].hn);
    }
  }
}

/* Chooses a random point inside each room and connects them with a *
 * corridor.  Random internal points prevent corridors from exiting *
 * rooms in predictable locations.                                  */
static int connect_two_rooms(dungeon_t *d, room_t *r1, room_t *r2)
{
  pair_t e1, e2;

  e1[dim_y] = rand_range(r1->position[dim_y],
                         r1->position[dim_y] + r1->size[dim_y] - 1);
  e1[dim_x] = rand_range(r1->position[dim_x],
                         r1->position[dim_x] + r1->size[dim_x] - 1);
  e2[dim_y] = rand_range(r2->position[dim_y],
                         r2->position[dim_y] + r2->size[dim_y] - 1);
  e2[dim_x] = rand_range(r2->position[dim_x],
                         r2->position[dim_x] + r2->size[dim_x] - 1);

  /*  return connect_two_points_recursive(d, e1, e2);*/
  dijkstra_corridor(d, e1, e2);
  return 0;
}

static int connect_rooms(dungeon_t *d)
{
  uint32_t i;

  for (i = 1; i < d->num_rooms; i++) {
    connect_two_rooms(d, d->rooms + i - 1, d->rooms + i);
  }

  return 0;
}

static int empty_dungeon(dungeon_t *d)
{
  uint8_t x, y;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      mapxy(x, y) = ter_wall;
      hardnessxy(x, y) = rand_range(1, 254);
      if (y == 0 || y == DUNGEON_Y - 1 ||
          x == 0 || x == DUNGEON_X - 1) {
        mapxy(x, y) = ter_wall_immutable;
        hardnessxy(x, y) = 255;
      }
      charxy(x, y) = NULL;
    }
  }

  return 0;
}

static int place_rooms(dungeon_t *d)
{
  pair_t p;
  uint32_t i;
  int success;
  room_t *r;

  for (success = 0; !success; ) {
    success = 1;
    for (i = 0; success && i < d->num_rooms; i++) {
      r = d->rooms + i;
      r->position[dim_x] = 1 + rand() % (DUNGEON_X - 2 - r->size[dim_x]);
      r->position[dim_y] = 1 + rand() % (DUNGEON_Y - 2 - r->size[dim_y]);
      for (p[dim_y] = r->position[dim_y] - 1;
           success && p[dim_y] < r->position[dim_y] + r->size[dim_y] + 1;
           p[dim_y]++) {
        for (p[dim_x] = r->position[dim_x] - 1;
             success && p[dim_x] < r->position[dim_x] + r->size[dim_x] + 1;
             p[dim_x]++) {
          if (mappair(p) >= ter_floor) {
            success = 0;
            empty_dungeon(d);
          } else if ((p[dim_y] != r->position[dim_y] - 1)              &&
                     (p[dim_y] != r->position[dim_y] + r->size[dim_y]) &&
                     (p[dim_x] != r->position[dim_x] - 1)              &&
                     (p[dim_x] != r->position[dim_x] + r->size[dim_x])) {
            mappair(p) = ter_floor_room;
            hardnesspair(p) = 0;
          }
        }
      }
    }
  }

  return 0;
}

static int make_rooms(dungeon_t *d)
{
  uint32_t i;

  for (i = MIN_ROOMS; i < MAX_ROOMS && rand_under(6, 8); i++)
    ;

  d->num_rooms = i;
  d->rooms = malloc(sizeof (*d->rooms) * d->num_rooms);

  for (i = 0; i < d->num_rooms; i++) {
    d->rooms[i].size[dim_x] = ROOM_MIN_X;
    d->rooms[i].size[dim_y] = ROOM_MIN_Y;
    while (rand_under(3, 4) && d->rooms[i].size[dim_x] < ROOM_MAX_X) {
      d->rooms[i].size[dim_x]++;
    }
    while (rand_under(3, 4) && d->rooms[i].size[dim_y] < ROOM_MAX_Y) {
      d->rooms[i].size[dim_y]++;
    }
    /* Initially, every room is connected only to itself. */
    d->rooms[i].connected = i;
  }

  return 0;
}

static void place_stairs(dungeon_t *d)
{
  pair_t p;
  do {
    while ((p[dim_y] = rand_range(1, DUNGEON_Y - 2)) &&
           (p[dim_x] = rand_range(1, DUNGEON_X - 2)) &&
           ((mappair(p) < ter_floor)                 ||
            (mappair(p) > ter_stairs)))
      ;
    mappair(p) = ter_stairs_down;
  } while (rand_under(1, 3));
  do {
    while ((p[dim_y] = rand_range(1, DUNGEON_Y - 2)) &&
           (p[dim_x] = rand_range(1, DUNGEON_X - 2)) &&
           ((mappair(p) < ter_floor)                 ||
            (mappair(p) > ter_stairs)))
      
      ;
    mappair(p) = ter_stairs_up;
  } while (rand_under(1, 4));
}

int gen_dungeon(dungeon_t *d)
{
  /*
  pair_t p1, p2;

  p1[dim_x] = rand_range(1, 158);
  p1[dim_y] = rand_range(1, 94);
  p2[dim_x] = rand_range(1, 158);
  p2[dim_y] = rand_range(1, 94);
  */

  empty_dungeon(d);

  /*
  connect_two_points_recursive(d, p1, p2);
  return 0;
  */

  do {
    make_rooms(d);
  } while (place_rooms(d));
  connect_rooms(d);
  place_stairs(d);

  return 0;
}

void delete_dungeon(dungeon_t *d)
{
  free(d->rooms);
  heap_delete(&d->next_turn);
  memset(d->character, 0, sizeof (d->character));
}

void init_dungeon(dungeon_t *d)
{
  empty_dungeon(d);

  memset(&d->next_turn, 0, sizeof (d->next_turn));
  heap_init(&d->next_turn, compare_characters_by_next_turn, character_delete);
}

static int write_dungeon_map(dungeon_t *d, FILE *f)
{
  uint32_t x, y;

  for (y = 1; y < DUNGEON_Y - 1; y++) {
    for (x = 1; x < DUNGEON_X - 1; x++) {
      fwrite(&d->hardness[y][x], sizeof (unsigned char), 1, f);
    }
  }

  return 0;
}

static int write_rooms(dungeon_t *d, FILE *f)
{
  uint32_t i;

  for (i = 0; i < d->num_rooms; i++) {
    /* write order is xpos, ypos, width, height */
    fwrite(&d->rooms[i].position[dim_x], 1, 1, f);
    fwrite(&d->rooms[i].position[dim_y], 1, 1, f);
    fwrite(&d->rooms[i].size[dim_x], 1, 1, f);
    fwrite(&d->rooms[i].size[dim_y], 1, 1, f);
  }

  return 0;
}

static uint32_t calculate_dungeon_size(dungeon_t *d)
{
  return (14 /* The semantic, version, and size */     +
          ((DUNGEON_X - 2) * (DUNGEON_Y - 2)) /* The hardnesses */ +
          (d->num_rooms * 4) /* Four bytes per room */);
}

int write_dungeon(dungeon_t *d)
{
  char *home;
  char *filename;
  FILE *f;
  size_t len;
  uint32_t be32;

  if (!(home = getenv("HOME"))) {
    fprintf(stderr, "\"HOME\" is undefined.  Using working directory.\n");
    home = ".";
  }

  len = (strlen(home) + strlen(SAVE_DIR) + strlen(DUNGEON_SAVE_FILE) +
         1 /* The NULL terminator */                                 +
         2 /* The slashes */);

  filename = malloc(len * sizeof (*filename));
  sprintf(filename, "%s/%s/", home, SAVE_DIR);
  makedirectory(filename);
  strcat(filename, DUNGEON_SAVE_FILE);

  if (!(f = fopen(filename, "w"))) {
    perror(filename);
    free(filename);

    return 1;
  }
  free(filename);

  /* The semantic, which is 6 bytes, 0-5 */
  fwrite(DUNGEON_SAVE_SEMANTIC, 1, strlen(DUNGEON_SAVE_SEMANTIC), f);

  /* The version, 4 bytes, 6-9 */
  be32 = htobe32(DUNGEON_SAVE_VERSION);
  fwrite(&be32, sizeof (be32), 1, f);

  /* The size of the file, 4 bytes, 10-13 */
  be32 = htobe32(calculate_dungeon_size(d));
  fwrite(&be32, sizeof (be32), 1, f);

  /* The dungeon map, 61440 bytes, 14-1495 */
  write_dungeon_map(d, f);

  /* And the rooms, num_rooms * 4 bytes, 1496-end */
  write_rooms(d, f);

  fclose(f);

  return 0;
}

static int read_dungeon_map(dungeon_t *d, FILE *f)
{
  uint32_t x, y;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      if (y == 0 || y == DUNGEON_Y - 1 ||
          x == 0 || x == DUNGEON_X - 1) {
        d->hardness[y][x] = 255;
        d->map[y][x] = ter_wall_immutable;
      } else {
        fread(&d->hardness[y][x], sizeof (d->hardness[y][x]), 1, f);
        if (d->hardness[y][x] == 0) {
          /* Mark it as a corridor.  We can't recognize room cells until *
           * after we've read the room array, which we haven't done yet. */
          d->map[y][x] = ter_floor_hall;
        } else {
          d->map[y][x] = ter_wall;
        }
      }
    }
  }

  return 0;
}

static int read_rooms(dungeon_t *d, FILE *f)
{
  uint32_t i;
  uint32_t x, y;

  for (i = 0; i < d->num_rooms; i++) {
    fread(&d->rooms[i].position[dim_x], 1, 1, f);
    fread(&d->rooms[i].position[dim_y], 1, 1, f);
    fread(&d->rooms[i].size[dim_x], 1, 1, f);
    fread(&d->rooms[i].size[dim_y], 1, 1, f);

    /* After reading each room, we need to reconstruct them in the dungeon. */
    for (y = d->rooms[i].position[dim_y];
         y < d->rooms[i].position[dim_y] + d->rooms[i].size[dim_y];
         y++) {
      for (x = d->rooms[i].position[dim_x];
           x < d->rooms[i].position[dim_x] + d->rooms[i].size[dim_x];
           x++) {
        mapxy(x, y) = ter_floor_room;
      }
    }
  }

  return 0;
}

static int calculate_num_rooms(uint32_t dungeon_bytes)
{
  return ((dungeon_bytes -
          (14 /* The semantic, version, and size */       +
           ((DUNGEON_X - 2) * (DUNGEON_Y - 2)) /* The hardnesses */)) /
          4 /* Four bytes per room */);
}

int read_dungeon(dungeon_t *d, char *file)
{
  char semantic[6];
  uint32_t be32;
  FILE *f;
  char *home;
  size_t len;
  char *filename;
  struct stat buf;

  if (!file) {
    if (!(home = getenv("HOME"))) {
      fprintf(stderr, "\"HOME\" is undefined.  Using working directory.\n");
      home = ".";
    }

    len = (strlen(home) + strlen(SAVE_DIR) + strlen(DUNGEON_SAVE_FILE) +
           1 /* The NULL terminator */                                 +
           2 /* The slashes */);

    filename = malloc(len * sizeof (*filename));
    sprintf(filename, "%s/%s/%s", home, SAVE_DIR, DUNGEON_SAVE_FILE);

    if (!(f = fopen(filename, "r"))) {
      perror(filename);
      free(filename);
      exit(-1);
    }

    if (stat(filename, &buf)) {
      perror(filename);
      exit(-1);
    }

    free(filename);
  } else {
    if (!(f = fopen(file, "r"))) {
      perror(file);
      exit(-1);
    }
    if (stat(file, &buf)) {
      perror(file);
      exit(-1);
    }

  }

  d->num_rooms = 0;

  fread(semantic, sizeof(semantic), 1, f);
  semantic[6] = '\0';
  if (strncmp(semantic, DUNGEON_SAVE_SEMANTIC, 6)) {
    fprintf(stderr, "Not an RLG327 save file.\n");
    exit(-1);
  }
  fread(&be32, sizeof (be32), 1, f);
  if (be32toh(be32) != 0) { /* Since we expect zero, be32toh() is a no-op. */
    fprintf(stderr, "File version mismatch.\n");
    exit(-1);
  }
  fread(&be32, sizeof (be32), 1, f);
  if (buf.st_size != be32toh(be32)) {
    fprintf(stderr, "File size mismatch.\n");
    exit(-1);
  }
  read_dungeon_map(d, f);
  d->num_rooms = calculate_num_rooms(buf.st_size);
  d->rooms = malloc(sizeof (*d->rooms) * d->num_rooms);
  read_rooms(d, f);

  fclose(f);

  return 0;
}

int read_pgm(dungeon_t *d, char *pgm)
{
  FILE *f;
  char s[80];
  uint8_t gm[19][78];
  uint32_t x, y;
  uint32_t i;

  if (!(f = fopen(pgm, "r"))) {
    perror(pgm);
    exit(-1);
  }

  if (!fgets(s, 80, f) || strncmp(s, "P5", 2)) {
    fprintf(stderr, "Expected P5\n");
    exit(-1);
  }
  if (!fgets(s, 80, f)) {
    fprintf(stderr, "Expected comment\n");
    exit(-1);
  }
  if (!fgets(s, 80, f) || strncmp(s, "78 19", 5)) {
    fprintf(stderr, "Expected 78\n");
    exit(-1);
  }
  if (!fgets(s, 80, f) || strncmp(s, "255", 2)) {
    fprintf(stderr, "Expected 255\n");
    exit(-1);
  }

  fread(gm, 1, 19 * 78, f);

  fclose(f);

  /* In our gray map, treat black (0) as corridor, white (255) as room, *
   * all other values as a hardness.  For simplicity, treat every white *
   * cell as its own room, so we have to count white after reading the  *
   * image in order to allocate the room array.                         */
  for (d->num_rooms = 0, y = 0; y < 19; y++) {
    for (x = 0; x < 78; x++) {
      if (!gm[y][x]) {
        d->num_rooms++;
      }
    }
  }
  d->rooms = malloc(sizeof (*d->rooms) * d->num_rooms);

  for (i = 0, y = 0; y < 19; y++) {
    for (x = 0; x < 78; x++) {
      if (!gm[y][x]) {
        d->rooms[i].position[dim_x] = x + 1;
        d->rooms[i].position[dim_y] = y + 1;
        d->rooms[i].size[dim_x] = 1;
        d->rooms[i].size[dim_y] = 1;
        i++;
        d->map[y + 1][x + 1] = ter_floor_room;
        d->hardness[y + 1][x + 1] = 0;
      } else if (gm[y][x] == 255) {
        d->map[y + 1][x + 1] = ter_floor_hall;
        d->hardness[y + 1][x + 1] = 0;
      } else {
        d->map[y + 1][x + 1] = ter_wall;
        d->hardness[y + 1][x + 1] = gm[y][x];
      }
    }
  }

  for (x = 0; x < 80; x++) {
    d->map[0][x] = ter_wall_immutable;
    d->hardness[0][x] = 255;
    d->map[20][x] = ter_wall_immutable;
    d->hardness[20][x] = 255;
  }
  for (y = 1; y < 20; y++) {
    d->map[y][0] = ter_wall_immutable;
    d->hardness[y][0] = 255;
    d->map[y][79] = ter_wall_immutable;
    d->hardness[y][79] = 255;
  }

  return 0;
}

void new_dungeon(dungeon_t *d)
{
  uint32_t sequence_number;

  sequence_number = d->character_sequence_number;

  delete_dungeon(d);

  init_dungeon(d);
  gen_dungeon(d);
  d->character_sequence_number = sequence_number;

  place_pc(d);
  d->character[get_char_y(d->pc)][get_char_x(d->pc)] = d->pc;

  /* Need to add a mechanism to decide on a number of monsters.  --nummon  *
   * is just for testing, and if we're generating new dungeon levels, then *
   * presumably we've already done that testing.  We'll just put 10 in for *
   * now.                                                                  */
  gen_monsters(d, 10, get_next_turn(d->pc));
}
