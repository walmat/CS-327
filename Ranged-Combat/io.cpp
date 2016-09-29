#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <string>
#include <sstream>

#include "io.h"
#include "move.h"
#include "path.h"
#include "pc.h"
#include "utils.h"
#include "dungeon.h"
#include "npc.h"

/* Same ugly hack we did in path.c */
static dungeon_t *dungeon;

typedef struct io_message {
  /* Will print " --more-- " at end of line when another message follows. *
   * Leave 10 extra spaces for that.                                      */
  char msg[71];
  struct io_message *next;
} io_message_t;

static io_message_t *io_head, *io_tail;

static void sigalrm_handler(int unused)
{
  io_display(dungeon);
}

static void mask_alarm(void)
{
  sigset_t s;

  sigemptyset(&s);
  sigaddset(&s, SIGALRM);
  sigprocmask(SIG_BLOCK, &s, NULL);
}

static void unmask_alarm(void)
{
  sigset_t s;

  sigemptyset(&s);
  sigaddset(&s, SIGALRM);
  sigprocmask(SIG_UNBLOCK, &s, NULL);
}

void io_init_terminal(dungeon_t *d)
{
  struct itimerval itv;

  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
  init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
  init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);

  itv.it_interval.tv_sec = 0;
  itv.it_interval.tv_usec = 200000; /* 1/5 seconds */
  itv.it_value.tv_sec = 0;
  itv.it_value.tv_usec = 200000;
  setitimer(ITIMER_REAL, &itv, NULL);
  dungeon = d;
  signal(SIGALRM, sigalrm_handler);
}

void io_reset_terminal(void)
{
  endwin();

  while (io_head) {
    io_tail = io_head;
    io_head = io_head->next;
    free(io_tail);
  }
  io_tail = NULL;
}

void io_queue_message(const char *format, ...)
{
  io_message_t *tmp;
  va_list ap;

  if (!(tmp = (io_message_t *) malloc(sizeof (*tmp)))) {
    perror("malloc");
    exit(1);
  }

  tmp->next = NULL;

  va_start(ap, format);

  vsnprintf(tmp->msg, sizeof (tmp->msg), format, ap);

  va_end(ap);

  if (!io_head) {
    io_head = io_tail = tmp;
  } else {
    io_tail->next = tmp;
    io_tail = tmp;
  }
}

static void io_print_message_queue(uint32_t y, uint32_t x)
{
  mask_alarm();
  while (io_head) {
    io_tail = io_head;
    attron(COLOR_PAIR(COLOR_CYAN));
    mvprintw(y, x, "%-80s", io_head->msg);
    attroff(COLOR_PAIR(COLOR_CYAN));
    io_head = io_head->next;
    /* Removed the test for head that prevents "more" prompt on final *
     * message in order to prevent redraw from smashing it.           */
    attron(COLOR_PAIR(COLOR_CYAN));
    mvprintw(y, x + 70, "%10s", " --more-- ");
    attroff(COLOR_PAIR(COLOR_CYAN));
    refresh();
    getch();
    free(io_tail);
  }
  io_tail = NULL;
  unmask_alarm();
}

static char distance_to_char[] = {
  '0',
  '1',
  '2',
  '3',
  '4',
  '5',
  '6',
  '7',
  '8',
  '9',
  'a',
  'b',
  'c',
  'd',
  'e',
  'f',
  'g',
  'h',
  'i',
  'j',
  'k',
  'l',
  'm',
  'n',
  'o',
  'p',
  'q',
  'r',
  's',
  't',
  'u',
  'v',
  'w',
  'x',
  'y',
  'z',
  'A',
  'B',
  'C',
  'D',
  'E',
  'F',
  'G',
  'H',
  'I',
  'J',
  'K',
  'L',
  'M',
  'N',
  'O',
  'P',
  'Q',
  'R',
  'S',
  'T',
  'U',
  'V',
  'W',
  'X',
  'Y',
  'Z'
};

void io_display_ch(dungeon_t *d)
{
  mask_alarm();
  mvprintw(11, 33, " HP:    %5d ", d->the_pc->hp);
  mvprintw(12, 33, " Speed: %5d ", d->the_pc->speed);
  mvprintw(14, 27, " Hit any key to continue. ");
  refresh();
  getch();
  unmask_alarm();
}

void io_display_tunnel(dungeon_t *d)
{
  uint32_t y, x;
  mask_alarm();
  clear();
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      mvaddch(y + 1, x, (d->pc_tunnel[y][x] < 62              ?
                         distance_to_char[d->pc_tunnel[y][x]] :
                         '*'));
    }
  }
  refresh();
  while (getch() != 27 /* ESC */)
    ;
  unmask_alarm();
}

void io_display_distance(dungeon_t *d)
{
  uint32_t y, x;
  mask_alarm();
  clear();
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      mvaddch(y + 1, x, (d->pc_distance[y][x] < 62              ?
                         distance_to_char[d->pc_distance[y][x]] :
                         '*'));
    }
  }
  refresh();
  while (getch() != 27 /* ESC */)
    ;
  unmask_alarm();
}

void io_display_hardness(dungeon_t *d)
{
  uint32_t y, x;
  mask_alarm();
  clear();
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      /* Maximum hardness is 255.  We have 62 values to display it, but *
       * we only want one zero value, so we need to cover [1,255] with  *
       * 61 values, which gives us a divisor of 254 / 61 = 4.164.       *
       * Generally, we want to avoid floating point math, but this is   *
       * not gameplay, so we'll make an exception here to get maximal   *
       * hardness display resolution.                                   */
      mvaddch(y + 1, x, (d->hardness[y][x]                             ?
                         distance_to_char[1 + (d->hardness[y][x] / 5)] :
                         '0'));
    }
  }
  refresh();
  while (getch() != 27 /* ESC */)
    ;
  unmask_alarm();
}

void io_display_all(dungeon_t *d)
{
  uint32_t y, x;

  mask_alarm();
  clear();
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      if (d->charmap[y][x]) {
        attron(COLOR_PAIR(d->charmap[y][x]->get_color()));
        mvaddch(y + 1, x, d->charmap[y][x]->get_symbol());
        attroff(COLOR_PAIR(d->charmap[y][x]->get_color()));
      } else if (d->objmap[y][x] && d->objmap[y][x]->have_seen()) {
        attron(COLOR_PAIR(d->objmap[y][x]->get_color()));
        mvaddch(y + 1, x, d->objmap[y][x]->get_symbol());
        attroff(COLOR_PAIR(d->objmap[y][x]->get_color()));
      } else {
        switch (mapxy(x, y)) {
        case ter_wall:
        case ter_wall_immutable:
          mvaddch(y + 1, x, ' ');
          break;
        case ter_floor:
        case ter_floor_room:
          mvaddch(y + 1, x, '.');
          break;
        case ter_floor_hall:
          mvaddch(y + 1, x, '#');
          break;
        case ter_debug:
          mvaddch(y + 1, x, '*');
          break;
        case ter_stairs_up:
          mvaddch(y + 1, x, '<');
          break;
        case ter_stairs_down:
          mvaddch(y + 1, x, '>');
          break;
        default:
 /* Use zero as an error symbol, since it stands out somewhat, and it's *
  * not otherwise used.                                                 */
          mvaddch(y + 1, x, '0');
        }
      }
    }
  }

  io_print_message_queue(0, 0);

  refresh();
  getch();
  unmask_alarm();
}

void io_display(dungeon_t *d)
{
  uint32_t y, x;
  uint32_t illuminated;

  mask_alarm();
  clear();
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      if ((illuminated = is_illuminated(d->the_pc, y, x))) {
        attron(A_BOLD);
      }
      if (d->charmap[y][x] &&
          can_see(d,
                  character_get_pos(d->the_pc),
                  character_get_pos(d->charmap[y][x]),
                  1)) {
        attron(COLOR_PAIR(d->charmap[y][x]->get_color()));
        mvaddch(y + 1, x, d->charmap[y][x]->get_symbol());
        attroff(COLOR_PAIR(d->charmap[y][x]->get_color()));
      } else if (d->objmap[y][x] && d->objmap[y][x]->have_seen()) {
        attron(COLOR_PAIR(d->objmap[y][x]->get_color()));
        mvaddch(y + 1, x, d->objmap[y][x]->get_symbol());
        attroff(COLOR_PAIR(d->objmap[y][x]->get_color()));
      } else {
        switch (pc_learned_terrain(d->the_pc, y, x)) {
        case ter_wall:
        case ter_wall_immutable:
        case ter_unknown:
          mvaddch(y + 1, x, ' ');
          break;
        case ter_floor:
        case ter_floor_room:
          mvaddch(y + 1, x, '.');
          break;
        case ter_floor_hall:
          mvaddch(y + 1, x, '#');
          break;
        case ter_debug:
          mvaddch(y + 1, x, '*');
          break;
        case ter_stairs_up:
          mvaddch(y + 1, x, '<');
          break;
        case ter_stairs_down:
          mvaddch(y + 1, x, '>');
          break;
        default:
 /* Use zero as an error symbol, since it stands out somewhat, and it's *
  * not otherwise used.                                                 */
          mvaddch(y + 1, x, '0');
        }
      }
      if (illuminated) {
        attroff(A_BOLD);
      }
    }
  }

  io_print_message_queue(0, 0);

  refresh();
  unmask_alarm();
}

void io_display_monster_list(dungeon_t *d)
{
  mvprintw(11, 33, " HP:    XXXXX ");
  mvprintw(12, 33, " Speed: XXXXX ");
  mvprintw(14, 27, " Hit any key to continue. ");
  refresh();
  getch();
}



static void io_list_item_description_display(dungeon_t *d, int32_t count)
{
  std::string word;
  std::string line;
  std::string desc = ((pc*) d->the_pc)->in[count]->get_description();
  std::stringstream ss(desc);
  int32_t i = 0;
  char s[41]; /* pointer to array of 41 characters */

  mvprintw(3, 19, " %-40s ", "");
  snprintf(s, 41, "%s", ((pc*) d->the_pc)->in[count]->get_name());
  mvprintw(4, 19, " %-40s ", s);
  mvprintw(5, 19, " %-40s ", "");

  line.clear();
  word.clear();
  while (ss >> word){
    if (line.size() + word.size() + 1 <= 40) {
      line += " ";
      line += word;
      word.clear();
    } else {
      snprintf(s, 41, "%s", line.c_str());
      mvprintw(i+6, 19, " %-41s ", s);
      i++;
      line.clear();
      line += word;
      word.clear();
    }
  }
  if (line.size() > 0) {
    snprintf(s, 41, "%s", line.c_str());
    mvprintw(i+6, 19, " %-41s ", s);
    i++;
  }
  
  mvprintw(i+8, 19, " %-40s ", "");
  mvprintw(i+9, 19, " %-40s ", "Hit escape to continue.");
  refresh();
}

uint32_t io_teleport_pc(dungeon_t *d)
{
  /* Just for fun. */
  pair_t dest;

  do {
    dest[dim_x] = rand_range(1, DUNGEON_X - 2);
    dest[dim_y] = rand_range(1, DUNGEON_Y - 2);
  } while (charpair(dest));

  d->charmap[character_get_y(d->the_pc)][character_get_x(d->the_pc)] = NULL;
  d->charmap[dest[dim_y]][dest[dim_x]] = d->the_pc;

  character_set_y(d->the_pc, dest[dim_y]);
  character_set_x(d->the_pc, dest[dim_x]);

  if (mappair(dest) < ter_floor) {
    mappair(dest) = ter_floor;
  }

  pc_observe_terrain(d->the_pc, d);

  dijkstra(d);
  dijkstra_tunnel(d);

  return 0;
}

/* Adjectives to describe our monsters */
static const char *adjectives[] = {
  "A menacing ",
  "A threatening ",
  "A horrifying ",
  "An intimidating ",
  "An aggressive ",
  "A frightening ",
  "A terrifying ",
  "A terrorizing ",
  "An alarming ",
  "A frightening ",
  "A dangerous ",
  "A glowering ",
  "A glaring ",
  "A scowling ",
  "A chilling ",
  "A scary ",
  "A creepy ",
  "An eerie ",
  "A spooky ",
  "A slobbering ",
  "A drooling ",
  " A horrendous ",
  "An unnerving ",
  "A cute little ",  /* Even though they're trying to kill you, */
  "A teeny-weenie ", /* they can still be cute!                 */
  "A fuzzy ",
  "A fluffy white ",
  "A kawaii ",       /* For our otaku */
  "Hao ke ai de "    /* And for our Chinese */
  /* And there's one special case (see below) */
};

static void io_scroll_monster_list(char (*s)[40], uint32_t count)
{
  uint32_t offset;
  uint32_t i;

  offset = 0;

  while (1) {
    for (i = 0; i < 13; i++) {
      mvprintw(i + 6, 19, " %-40s ", s[i + offset]);
    }
    switch (getch()) {
    case KEY_UP:
      if (offset) {
        offset--;
      }
      break;
    case KEY_DOWN:
      if (offset < (count - 13)) {
        offset++;
      }
      break;
    case 27:
      return;
    }

  }
}

static bool is_vowel(const char c)
{
  return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
          c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U');
}

static void io_list_monsters_display(dungeon_t *d,
                                     character **c,
                                     uint32_t count)
{
  uint32_t i;
  char (*s)[40]; /* pointer to array of 40 char */

  (void) adjectives;

  s = (char (*)[40]) malloc((count ? count : 1) * sizeof (*s));

  mvprintw(3, 19, " %-40s ", "");
  /* Borrow the first element of our array for this string: */
  snprintf(s[0], 40, "You know of %d monsters:", count);
  mvprintw(4, 19, " %-40s ", s);
  mvprintw(5, 19, " %-40s ", "");

  for (i = 0; i < count; i++) {
    snprintf(s[i], 40, "%3s%s (%c): %2d %s by %2d %s",
             (is_vowel(character_get_name(c[i])[0]) ? "An " : "A "),
             character_get_name(c[i]),
             character_get_symbol(c[i]),
             abs(character_get_y(c[i]) - character_get_y(d->the_pc)),
             ((character_get_y(c[i]) - character_get_y(d->the_pc)) <= 0 ?
              "North" : "South"),
             abs(character_get_x(c[i]) - character_get_x(d->the_pc)),
             ((character_get_x(c[i]) - character_get_x(d->the_pc)) <= 0 ?
              "East" : "West"));
    if (count <= 13) {
      /* Handle the non-scrolling case right here. *
       * Scrolling in another function.            */
      mvprintw(i + 6, 19, " %-40s ", s[i]);
    }
  }

  if (count <= 13) {
    mvprintw(count + 6, 19, " %-40s ", "");
    mvprintw(count + 7, 19, " %-40s ", "Hit escape to continue.");
    while (getch() != 27 /* escape */)
      ;
  } else {
    mvprintw(19, 19, " %-40s ", "");
    mvprintw(20, 19, " %-40s ",
             "Arrows to scroll, escape to continue.");
    io_scroll_monster_list(s, count);
  }

  free(s);
}

static int compare_monster_distance(const void *v1, const void *v2)
{
  const character *const *c1 = (const character * const *) v1;
  const character *const *c2 = (const character * const *) v2;

  return (dungeon->pc_distance[character_get_y(*c1)][character_get_x(*c1)] -
          dungeon->pc_distance[character_get_y(*c2)][character_get_x(*c2)]);
}

static void io_list_monsters(dungeon_t *d)
{
  character **c;
  uint32_t x, y, count;

  mask_alarm();
  c = (character **) malloc(d->num_monsters * sizeof (*c));

  /* Get a linear list of monsters */
  for (count = 0, y = 1; y < DUNGEON_Y - 1; y++) {
    for (x = 1; x < DUNGEON_X - 1; x++) {
      if (d->charmap[y][x] &&
          d->charmap[y][x] != d->the_pc &&
          can_see(d, d->the_pc->position, character_get_pos(d->charmap[y][x]), 1)) {
        c[count++] = d->charmap[y][x];
      }
    }
  }

  /* Sort it by distance from PC */
  dungeon = d;
  qsort(c, count, sizeof (*c), compare_monster_distance);

  /* Display it */
  io_list_monsters_display(d, c, count);
  free(c);

  unmask_alarm();

  /* And redraw the dungeon */
  io_display(d);
}

static void io_list_player_stats(dungeon_t *d, int32_t count)
{
  int32_t i;
  char (*s)[40]; /* pointer to array of 40 char */

  s = (char (*)[40]) malloc(count * sizeof (*s));
  mvprintw(3, 19, " %-40s ", "");
  mvprintw(4, 19, "Name: %s", get_pc_name(d->the_pc));
  mvprintw(5, 19, " %-40s ", "");

  for (i = 0; i < count; i++){
    switch(i){
    case 0:
      snprintf(s[i], 40, "Hitpoints: %d", get_pc_hp(d->the_pc));
      break;
    case 1:
      snprintf(s[i], 40, "Next Atk: %d", get_pc_dmg(d->the_pc));
      break;
    case 2:
      snprintf(s[i], 40, "Defense: %d", get_pc_def(d->the_pc));
      break;
      /*    case 3:                                                                                  
      //snprintf(s[i], 40, "Dodge: %d", get_pc_dodge(d->the_pc));                                        
      break;*/
    default:
      snprintf(s[i], 40, "Speed: %d", get_pc_speed(d->the_pc));
      break;
    }
    mvprintw(i + 6, 19, " %-40s ", s[i]);
  }
  mvprintw(count + 6, 19, " %-40s ", "");
  mvprintw(count + 7, 19, " %-40s ", "Hit escape to continue.");
  refresh();
  free(s);
}

void io_object_to_string(object *o, char *s, uint32_t size)
{
  if (o) {
    snprintf(s, size, "%s (sp: %d, dmg: %d+%dd%d, rng: %d)",
             o->get_name(), o->get_speed(), o->get_damage_base(),
             o->get_damage_number(), o->get_damage_sides(), o->get_range());
  } else {
    *s = '\0';
  }
}

uint32_t io_wear_eq(dungeon_t *d)
{
  uint32_t i, key;
  char s[61];

  mask_alarm();
  for (i = 0; i < MAX_INVENTORY; i++) {
    /* We'll write 12 lines, 10 of inventory, 1 blank, and 1 prompt. *
     * We'll limit width to 60 characters, so very long object names *
     * will be truncated.  In an 80x24 terminal, this gives offsets  *
     * at 10 x and 6 y to start printing things.  Same principal in  *
     * other functions, below.                                       */
    io_object_to_string(d->the_pc->in[i], s, 61);
    mvprintw(i + 6, 10, " %c) %-55s ", '0' + i, s);
  }
  mvprintw(16, 10, " %-58s ", "");
  mvprintw(17, 10, " %-58s ", "Wear which item (ESC to cancel)?");
  refresh();

  while (1) {
    if ((key = getch()) == 27 /* ESC */) {
      io_display(d);
      unmask_alarm();
      return 1;
    }

    if (key < '0' || key > '9') {
      if (isprint(key)) {
        snprintf(s, 61, "Invalid input: '%c'.  Enter 0-9 or ESC to cancel.",
                 key);
        mvprintw(18, 10, " %-58s ", s);
      } else {
        mvprintw(18, 10, " %-58s ",
                 "Invalid input.  Enter 0-9 or ESC to cancel.");
      }
      refresh();
      continue;
    }

    if (!d->the_pc->in[key - '0']) {
      mvprintw(18, 10, " %-58s ", "Empty inventory slot.  Try again.");
      continue;
    }

    if (!d->the_pc->wear_in(key - '0')) {
      unmask_alarm();
      return 0;
    }

    snprintf(s, 61, "Can't wear %s.  Try again.",
             d->the_pc->in[key - '0']->get_name());
    mvprintw(18, 10, " %-58s ", s);
    refresh();
  }

  unmask_alarm();
  return 1;
}

void io_display_in(dungeon_t *d)
{
  uint32_t i;
  char s[61];

  mask_alarm();
  for (i = 0; i < MAX_INVENTORY; i++) {
    io_object_to_string(d->the_pc->in[i], s, 61);
    mvprintw(i + 7, 10, " %c) %-55s ", '0' + i, s);
  }

  mvprintw(17, 10, " %-58s ", "");
  mvprintw(18, 10, " %-58s ", "Hit any key to continue.");

  refresh();

  getch();
  unmask_alarm();

  io_display(d);
}

static void io_list_inventory_display(dungeon_t *d, int32_t count, int8_t sel)
{
  uint32_t i;
  char (*s)[40]; /* pointer to array of 40 char */

  s = (char (*)[40]) malloc((count == 0 ? 1 : count) * sizeof (*s));

  mvprintw(3, 19, " %-40s ", "");
  /* Borrow the first element of our array for this string: */
  switch (sel)
    {
    case 0:
      snprintf(s[0], 40, "You are carrying %d items", count);
      break;
    case 1:
      snprintf(s[0], 40, "Choose an item from your inventory");
      break;
    default:
      snprintf(s[0], 40, "Invalid option. Please try again.");
      break;
    }
  mvprintw(4, 19, " %-40s ", s);
  mvprintw(5, 19, " %-40s ", "");

  for (i = 0; i < count; i++) {
    snprintf(s[i], 40, "%2d: %s",
	     i,
	     ((pc*) d->the_pc)->in[i]->get_name()
	     );
    mvprintw(i + 6, 19, " %-40s ", s[i]);
  }
  mvprintw(count + 6, 19, " %-40s ", "");
  mvprintw(count + 7, 19, " %-40s ", "Hit escape to continue.");
  refresh();
  free(s);
}

uint32_t io_range_attack_display(dungeon_t *d)
{
  uint32_t i = 0, j = 0, index = 0, key;
  char s[61];
  character* m[10];

  mask_alarm();
  for(i = character_get_x(d->the_pc) - 2; i < character_get_x(d->the_pc) + 3; i++){
    for(j = character_get_y(d->the_pc) - 2; j < character_get_y(d->the_pc) + 3; j++){
      if (charxy(i, j) && charxy(i,j) != d->the_pc){
	m[index++] = charxy(i,j);
	mvprintw(6+index, 10, " %d: %s ", index - 1, character_get_name(m[index-1]));
      }
    }
  }
  mvprintw(17, 10, " %-58s ", "");
  mvprintw(18, 10, " %-58s ", "Choose a monster to attack (ESC to cancel)?");
  refresh();

  while (1) {
    if ((key = getch()) == 27 /* ESC */) {
      io_display(d);
      return 1;
    }

    if (key < '0' || key > '9') {
      if (isprint(key)) {
	snprintf(s, 61, "Invalid input: '%c'.  Enter 0-9 or ESC to cancel.",
		 key);
	mvprintw(18, 10, " %-58s ", s);
      } else {
	mvprintw(18, 10, " %-58s ",
		 "Invalid input.  Enter 0-9 or ESC to cancel.");
      }
      refresh();
      continue;
    }
    key -= '0';
    mvprintw(0,0,"Key: %d", key);
    if (d->the_pc->eq[2] && m[key]){
      do_combat(d, d->the_pc, m[key]);
      return 0;
    }
    
    snprintf(s, 61, "Can't attack that monster, try again.");
    mvprintw(19, 10, " %-58s ", s);
  }

  unmask_alarm();
  return 1;
}

uint32_t io_remove_eq(dungeon_t *d)
{
  uint32_t i, key;
  char s[61], t[61];

  mask_alarm();
  for (i = 0; i < num_eq_slots; i++) {
    sprintf(s, "[%s]", eq_slot_name[i]);
    io_object_to_string(d->the_pc->eq[i], t, 61);
    mvprintw(i + 5, 10, " %c %-9s) %-45s ", 'a' + i, s, t);
  }
  mvprintw(17, 10, " %-58s ", "");
  mvprintw(18, 10, " %-58s ", "Take off which item (ESC to cancel)?");
  refresh();

  while (1) {
    if ((key = getch()) == 27 /* ESC */) {
      io_display(d);
      return 1;
    }

    if (key < 'a' || key > 'l') {
      if (isprint(key)) {
        snprintf(s, 61, "Invalid input: '%c'.  Enter a-l or ESC to cancel.",
                 key);
        mvprintw(18, 10, " %-58s ", s);
      } else {
        mvprintw(18, 10, " %-58s ",
                 "Invalid input.  Enter a-l or ESC to cancel.");
      }
      refresh();
      continue;
    }

    if (!d->the_pc->eq[key - 'a']) {
      mvprintw(18, 10, " %-58s ", "Empty equipment slot.  Try again.");
      continue;
    }

    if (!d->the_pc->remove_eq(key - 'a')) {
      unmask_alarm();
      return 0;
    }

    snprintf(s, 61, "Can't take off %s.  Try again.",
             d->the_pc->eq[key - 'a']->get_name());
    mvprintw(19, 10, " %-58s ", s);
  }

  unmask_alarm();
  return 1;
}

void io_display_eq(dungeon_t *d)
{
  uint32_t i;
  char s[61], t[61];

  mask_alarm();
  for (i = 0; i < num_eq_slots; i++) {
    sprintf(s, "[%s]", eq_slot_name[i]);
    io_object_to_string(d->the_pc->eq[i], t, 61);
    mvprintw(i + 5, 10, " %c %-9s) %-45s ", 'a' + i, s, t);
  }
  mvprintw(17, 10, " %-58s ", "");
  mvprintw(18, 10, " %-58s ", "Hit any key to continue.");

  refresh();

  getch();

  io_display(d);
  unmask_alarm();
}

uint32_t io_drop_in(dungeon_t *d)
{
  uint32_t i, key;
  char s[61];

  mask_alarm();
  for (i = 0; i < MAX_INVENTORY; i++) {
    /* We'll write 12 lines, 10 of inventory, 1 blank, and 1 prompt. *
     * We'll limit width to 60 characters, so very long object names *
     * will be truncated.  In an 80x24 terminal, this gives offsets  *
     * at 10 x and 6 y to start printing things.                     */
      mvprintw(i + 6, 10, " %c) %-55s ", '0' + i,
               d->the_pc->in[i] ? d->the_pc->in[i]->get_name() : "");
  }
  mvprintw(16, 10, " %-58s ", "");
  mvprintw(17, 10, " %-58s ", "Drop which item (ESC to cancel)?");
  refresh();

  while (1) {
    if ((key = getch()) == 27 /* ESC */) {
      io_display(d);
      unmask_alarm();
      return 1;
    }

    if (key < '0' || key > '9') {
      if (isprint(key)) {
        snprintf(s, 61, "Invalid input: '%c'.  Enter 0-9 or ESC to cancel.",
                 key);
        mvprintw(18, 10, " %-58s ", s);
      } else {
        mvprintw(18, 10, " %-58s ",
                 "Invalid input.  Enter 0-9 or ESC to cancel.");
      }
      refresh();
      continue;
    }

    if (!d->the_pc->in[key - '0']) {
      mvprintw(18, 10, " %-58s ", "Empty inventory slot.  Try again.");
      continue;
    }

    if (!d->the_pc->drop_in(d, key - '0')) {
      unmask_alarm();
      return 0;
    }

    snprintf(s, 61, "Can't drop %s.  Try again.",
             d->the_pc->in[key - '0']->get_name());
    mvprintw(18, 10, " %-58s ", s);
    refresh();
  }

  unmask_alarm();
  return 1;
}

uint32_t io_expunge_in(dungeon_t *d)
{
  uint32_t i, key;
  char s[61];

  mask_alarm();
  for (i = 0; i < MAX_INVENTORY; i++) {
    /* We'll write 12 lines, 10 of inventory, 1 blank, and 1 prompt. *
     * We'll limit width to 60 characters, so very long object names *
     * will be truncated.  In an 80x24 terminal, this gives offsets  *
     * at 10 x and 6 y to start printing things.                     */
      mvprintw(i + 6, 10, " %c) %-55s ", '0' + i,
               d->the_pc->in[i] ? d->the_pc->in[i]->get_name() : "");
  }
  mvprintw(16, 10, " %-58s ", "");
  mvprintw(17, 10, " %-58s ", "Destroy which item (ESC to cancel)?");
  refresh();

  while (1) {
    if ((key = getch()) == 27 /* ESC */) {
      io_display(d);
      unmask_alarm();
      return 1;
    }

    if (key < '0' || key > '9') {
      if (isprint(key)) {
        snprintf(s, 61, "Invalid input: '%c'.  Enter 0-9 or ESC to cancel.",
                 key);
        mvprintw(18, 10, " %-58s ", s);
      } else {
        mvprintw(18, 10, " %-58s ",
                 "Invalid input.  Enter 0-9 or ESC to cancel.");
      }
      refresh();
      continue;
    }

    if (!d->the_pc->in[key - '0']) {
      mvprintw(18, 10, " %-58s ", "Empty inventory slot.  Try again.");
      continue;
    }

    if (!d->the_pc->destroy_in(key - '0')) {
      io_display(d);

      unmask_alarm();
      return 1;
    }

    snprintf(s, 61, "Can't destroy %s.  Try again.",
             d->the_pc->in[key - '0']->get_name());
    mvprintw(18, 10, " %-58s ", s);
    refresh();
  }

  unmask_alarm();
  return 1;
}

void io_handle_input(dungeon_t *d)
{
  uint32_t fail_code;
  int key, break_loop, c;

  do {
    switch (key = getch()) {
    case '7':
    case 'y':
    case KEY_HOME:
      fail_code = move_pc(d, 7);
      break;
    case '8':
    case 'k':
    case KEY_UP:
      fail_code = move_pc(d, 8);
      break;
    case '9':
    case 'u':
    case KEY_PPAGE:
      fail_code = move_pc(d, 9);
      break;
    case '6':
    case 'l':
    case KEY_RIGHT:
      fail_code = move_pc(d, 6);
      break;
    case '3':
    case 'n':
    case KEY_NPAGE:
      fail_code = move_pc(d, 3);
      break;
    case '2':
    case 'j':
    case KEY_DOWN:
      fail_code = move_pc(d, 2);
      break;
    case '1':
    case 'b':
    case KEY_END:
      fail_code = move_pc(d, 1);
      break;
    case '4':
    case 'h':
    case KEY_LEFT:
      fail_code = move_pc(d, 4);
      break;
    case '5':
    case ' ':
    case KEY_B2:
      fail_code = 0;
      break;
    case '>':
      if (dungeon_has_npcs(d)){
	fail_code = 1;
	io_queue_message("None shall pass (before clearing this floor)...");
	break;
      }
      fail_code = move_pc(d, '>');
      break;
    case '<':
      fail_code = move_pc(d, '<');
      break;
    case 'S':
      d->save_and_exit = 1;
      character_reset_turn(d->the_pc);
      fail_code = 0;
      break;
    case 'Q':
      /* Extra command, not in the spec.  Quit without saving.          */
      d->quit_no_save = 1;
      fail_code = 0;
      break;
    case 'T':
      /* New command.  Display the distances for tunnelers.             */
      io_display_tunnel(d);
      fail_code = 1;
      break;
    case 'D':
      /* New command.  Display the distances for non-tunnelers.         */
      io_display_distance(d);
      fail_code = 1;
      break;
    case 'H':
      /* New command.  Display the hardnesses.                          */
      io_display_hardness(d);
      fail_code = 1;
      break;
    case 's':
      /* New command.  Return to normal display after displaying some   *
       * special screen.                                                */
      io_display(d);
      fail_code = 1;
      break;
    case 'g':
      /* Teleport the PC to a random place in the dungeon.              */
      io_teleport_pc(d);
      fail_code = 0;
      break;
    case 'm':
      io_list_monsters(d);
      fail_code = 1;
      break;
    case 'p':
      mask_alarm();
      io_list_player_stats(d, 4);
      while(getch() != 27);
      unmask_alarm();
      fail_code = 1;
      break;
    case 'a':
      io_display_all(d);
      fail_code = 1;
      break;
    case 'w':
      fail_code = io_wear_eq(d);
      break;
    case 't':
      fail_code = io_remove_eq(d);
      break;
    case 'd':
      fail_code = io_drop_in(d);
      break;
    case 'x':
      fail_code = io_expunge_in(d);
      break;
    case 'i':
      io_display_in(d);
      fail_code = 1;
      break;
    case 'e':
      io_display_eq(d);
      fail_code = 1;
      break;
    case 'r':
      fail_code = io_range_attack_display(d);
      break;
    case 'I':
      break_loop = 0;
      mask_alarm();
      io_list_inventory_display(d, get_inv_size(d->the_pc), 1);
      c = getch();
      if (c == 27){
	unmask_alarm();
	fail_code = 1;
	break;
      }
      c -= 48;
      while(!(c >= 0 && ((pc *)(d->the_pc))->in[c])){
	io_list_inventory_display(d, get_inv_size(d->the_pc), 2);
	c = getch();
	if (c == 27){
	  break_loop = 1;
	  break;
	}
	c -= 48;
      }
      if (!break_loop){
	unmask_alarm();
	io_display(d);
	mask_alarm();
	io_list_item_description_display(d, c);
	while (getch() != 27);
      }
      unmask_alarm();
      fail_code = 1;
      break;
    case 'q':
      /* Demonstrate use of the message queue.  You can use this for *
       * printf()-style debugging (though gdb is probably a better   *
       * option.  Not that it matterrs, but using this command will  *
       * waste a turn.  Set fail_code to 1 and you should be able to *
       * figure out why I did it that way.                           */
      io_queue_message("This is the first message.");
      io_queue_message("Since there are multiple messages, "
                       "you will see \"more\" prompts.");
      io_queue_message("You can use any key to advance through messages.");
      io_queue_message("Normal gameplay will not resume until the queue "
                       "is empty.");
      io_queue_message("Long lines will be truncated, not wrapped.");
      io_queue_message("io_queue_message() is variadic and handles "
                       "all printf() conversion specifiers.");
      io_queue_message("Did you see %s?", "what I did there");
      io_queue_message("When the last message is displayed, there will "
                       "be no \"more\" prompt.");
      io_queue_message("Have fun!  And happy printing!");
      fail_code = 0;
      break;
    default:
      /* Also not in the spec.  It's not always easy to figure out what *
       * key code corresponds with a given keystroke.  Print out any    *
       * unhandled key here.  Not only does it give a visual error      *
       * indicator, but it also gives an integer value that can be used *
       * for that key in this (or other) switch statements.  Printed in *
       * octal, with the leading zero, because ncurses.h lists codes in *
       * octal, thus allowing us to do reverse lookups.  If a key has a *
       * name defined in the header, you can use the name here, else    *
       * you can directly use the octal value.                          */
      mvprintw(0, 0, "Unbound key: %#o ", key);
      fail_code = 1;
    }
  } while (fail_code);
}
