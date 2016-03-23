#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <ncurses.h>

#include "dungeon.h"
#include "path.h"
#include "pc.h"
#include "npc.h"
#include "move.h"

const char *victory =
  "\n                                       o\n"
  "                                      $\"\"$o\n"
  "                                     $\"  $$\n"
  "                                      $$$$\n"
  "                                      o \"$o\n"
  "                                     o\"  \"$\n"
  "                oo\"$$$\"  oo$\"$ooo   o$    \"$    ooo\"$oo  $$$\"o\n"
  "   o o o o    oo\"  o\"      \"o    $$o$\"     o o$\"\"  o$      \"$  "
  "\"oo   o o o o\n"
  "   \"$o   \"\"$$$\"   $$         $      \"   o   \"\"    o\"         $"
  "   \"o$$\"    o$$\n"
  "     \"\"o       o  $          $\"       $$$$$       o          $  ooo"
  "     o\"\"\n"
  "        \"o   $$$$o $o       o$        $$$$$\"       $o        \" $$$$"
  "   o\"\n"
  "         \"\"o $$$$o  oo o  o$\"         $$$$$\"        \"o o o o\"  "
  "\"$$$  $\n"
  "           \"\" \"$\"     \"\"\"\"\"            \"\"$\"            \""
  "\"\"      \"\"\" \"\n"
  "            \"oooooooooooooooooooooooooooooooooooooooooooooooooooooo$\n"
  "             \"$$$$\"$$$$\" $$$$$$$\"$$$$$$ \" \"$$$$$\"$$$$$$\"  $$$\""
  "\"$$$$\n"
  "              $$$oo$$$$   $$$$$$o$$$$$$o\" $$$$$$$$$$$$$$ o$$$$o$$$\"\n"
  "              $\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\""
  "\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"$\n"
  "              $\"                                                 \"$\n"
  "              $\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\""
  "$\"$\"$\"$\"$\"$\"$\"$\n"
  "                                   You win!\n\n";

const char *tombstone =
  "\n\n\n\n                /\"\"\"\"\"/\"\"\"\"\"\"\".\n"
  "               /     /         \\             __\n"
  "              /     /           \\            ||\n"
  "             /____ /  In Loving  \\           ||\n"
  "            |     |    Memory     |          ||\n"
  "            |     |               |          ||\n"
  "            |     |   A. Luser    |          ||\n"
  "            |     |               |          ||\n"
  "            |     |     * *   * * |         _||_\n"
  "            |     |     *\\/* *\\/* |        | TT |\n"
  "            |     |     *_\\_  /   ...\"\"\"\"\"\"| |"
  "| |.\"\"....\"\"\"\"\"\"\"\".\"\"\n"
  "            |     |         \\/..\"\"\"\"\"...\"\"\""
  "\\ || /.\"\"\".......\"\"\"\"...\n"
  "            |     |....\"\"\"\"\"\"\"........\"\"\"\"\""
  "\"^^^^\".......\"\"\"\"\"\"\"\"..\"\n"
  "            |......\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"......"
  "..\"\"\"\"\"....\"\"\"\"\"..\"\"...\"\"\".\n\n"
  "            You're dead.  Better luck in the next life.\n\n\n";



WINDOW* create_win(int x, int y, int width, int height){

  WINDOW *w;
  w = newwin(height,width,y,x);
  
  char top_bot = '-';
  char left_right = '|';
  char corner = '+';
  box(w, 0, 0);
  //mvwprintw(w, 0, 0, "box window");
  wborder(w, left_right, left_right, top_bot, top_bot, corner, corner, corner, corner);
  //mvwprintw(w, 0, 0, "created border");
  refresh();
  wrefresh(w);  
  return w;
}

void get_monsters(dungeon_t *d, character_t **m){

  int num = 0;
  
  for (int i = 0; i < DUNGEON_X; i++){
    for (int j = 0; j < DUNGEON_Y; j++){
      if (d->character[j][i] != NULL && d->character[j][i] != &(d->pc)){
	m[num++] = d->character[j][i];
      }
    }
  }
}

void usage(char *name)
{
  fprintf(stderr,
          "Usage: %s [-r|--rand <seed>] [-l|--load [<file>]]\n"
          "       [-i|--image <pgm>] [-s|--save]\n",
          name);

  exit(-1);
}

int main(int argc, char *argv[])
{
  dungeon_t d;
  time_t seed;
  struct timeval tv;
  uint32_t i;
  uint32_t do_load, do_save, do_seed, do_image;
  uint32_t long_arg;
  char *save_file;
  char *pgm_file;
  int pause = 0;

  memset(&d, 0, sizeof (d));

  /* Default behavior: Seed with the time, generate a new dungeon, *
   * and don't write to disk.                                      */
  do_load = do_save = do_image = 0;
  do_seed = 1;
  save_file = NULL;
  d.max_monsters = 10;
  d.placed_stairs = 0;
  d.level = 0;
  /* The project spec requires '--load' and '--save'.  It's common  *
   * to have short and long forms of most switches (assuming you    *
   * don't run out of letters).  For now, we've got plenty.  Long   *
   * forms use whole words and take two dashes.  Short forms use an *
`   * abbreviation after a single dash.  We'll add '--rand' (to     *
   * specify a random seed), which will take an argument of it's    *
   * own, and we'll add short forms for all three commands, '-l',   *
   * '-s', and '-r', respectively.  We're also going to allow an    *
   * optional argument to load to allow us to load non-default save *
   * files.  No means to save to non-default locations, however.    *
   * And the final switch, '--image', allows me to create a dungeon *
   * from a PGM image, so that I was able to create those more      *
   * interesting test dungeons for you.                             */
 
 if (argc > 1) {
    for (i = 1, long_arg = 0; i < argc; i++, long_arg = 0) {
      if (argv[i][0] == '-') { /* All switches start with a dash */
        if (argv[i][1] == '-') {
          argv[i]++;    /* Make the argument have a single dash so we can */
          long_arg = 1; /* handle long and short args at the same place.  */
        }
        switch (argv[i][1]) {
        case 'r':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-rand")) ||
              argc < ++i + 1 /* No more arguments */ ||
              !sscanf(argv[i], "%lu", &seed) /* Argument is not an integer */) {
            usage(argv[0]);
          }
          do_seed = 0;
          break;
        case 'l':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-load"))) {
            usage(argv[0]);
          }
          do_load = 1;
          if ((argc > i + 1) && argv[i + 1][0] != '-') {
            /* There is another argument, and it's not a switch, so *
             * we'll treat it as a save file and try to load it.    */
            save_file = argv[++i];
          }
          break;
        case 's':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-save"))) {
            usage(argv[0]);
          }
          do_save = 1;
          break;
        case 'i':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-image"))) {
            usage(argv[0]);
          }
          do_image = 1;
          if ((argc > i + 1) && argv[i + 1][0] != '-') {
            /* There is another argument, and it's not a switch, so *
             * we'll treat it as a save file and try to load it.    */
            pgm_file = argv[++i];
          }
          break;
        case 'n':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-nummon")) ||
              argc < ++i + 1 /* No more arguments */ ||
              !sscanf(argv[i], "%hu", &d.max_monsters)) {
            usage(argv[0]);
          }
          break;
        default:
          usage(argv[0]);
        }
      } else { /* No dash */
        usage(argv[0]);
      }
    }
  }

  if (do_seed) {
    /* Allows me to generate more than one dungeon *
     * per second, as opposed to time().           */
    gettimeofday(&tv, NULL);
    seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
  }

  printf("Seed is %ld.\n", seed);
  srand(seed);
  init_dungeon(&d);

  if (do_load) {
    read_dungeon(&d, save_file);
  } else if (do_image) {
    read_pgm(&d, pgm_file);
  } else {
    gen_dungeon(&d);
  }

  config_pc(&d);
  gen_monsters(&d, d.max_monsters, 0);

  initscr();
  raw();
  keypad(stdscr, TRUE);
  define_key("\033Oq", 49);
  define_key("\033Or", 50);
  define_key("\033Os", 51);
  define_key("\033Ot", 52);
  define_key("\033Ou", 53);
  define_key("\033Ov", 54);
  define_key("\033Ow", 55);
  define_key("\033Ox", 56);
  define_key("\033Oy", 57);
  noecho();
  curs_set(0);
  
  while (pc_is_alive(&d) && dungeon_has_npcs(&d)) {
    
    if (pause == 0){
      render_dungeon(&d);
      pause = do_moves(&d);
    }
    if (pause == 1){
      mvprintw(0,0, "Esc to close, arrow keys to move");
      character_t **m = malloc(d.num_monsters * sizeof (character_t*));
      for (int i = 0; i < d.num_monsters; i++){
	m[i] = malloc(sizeof (character_t));
      }
      get_monsters(&d, m);
      WINDOW *wnd = create_win(10, 3, 60, 16);
      int num_lines = d.num_monsters < 14 ? d.num_monsters : 14;
      int start_index = 0;
      int c = 0;
      do {
	if (c == KEY_UP){
	  
	  start_index--;
	  //mvprintw(0,0,"%d", start_index);
	}
	if (c == KEY_DOWN){
	  start_index++;
	  //mvprintw(0,0,"%d", start_index);
	}
	if (start_index > d.num_monsters - num_lines){
	  start_index = d.num_monsters - num_lines;
	}
	if (start_index < 0){
	  start_index = 0;
	}
	//	mvprintw(0,0, "s: %d nm: %d nl: %d", start_index, d.num_monsters, num_lines);
	//clrtoeol();
	for (int i = 1; num_lines > 0; num_lines--, i++){
	  
	  int dist_x = d.pc.position[dim_x] < m[i+start_index-1]->position[dim_x]
	    ? m[i+start_index-1]->position[dim_x] - d.pc.position[dim_x] :
	    d.pc.position[dim_x] - m[i+start_index-1]->position[dim_x];

	  int dist_y = d.pc.position[dim_y] < m[i+start_index-1]->position[dim_y]
	    ? m[i+start_index-1]->position[dim_y] - d.pc.position[dim_y] :
	    d.pc.position[dim_y] - m[i+start_index-1]->position[dim_y];
	  
	  mvwprintw(wnd, i, 1, "%c is %2d %s and %2d %s", m[i+start_index-1]->symbol,
		    dist_x, m[i+start_index-1]->position[dim_x] > d.pc.position[dim_x] ? "East": "West",
		    dist_y, m[i+start_index-1]->position[dim_y] > d.pc.position[dim_y] ? "South": "North");
	}
	num_lines = d.num_monsters < 14 ? d.num_monsters : 14;
	refresh();
	wrefresh(wnd);
	c = getch();
	//mvprintw(0,0, "c: %d, KU: %d, KD: %d", c, KEY_UP, KEY_DOWN);
      } while (c != 27);
      move(0,0);
      clrtoeol();
      free(m);
      pause = 0;
      render_dungeon(&d);
      refresh();
    }
    if (pause == 2){
      //going down
      int level = d.level + 1;
      pc_delete(d.pc.pc);
      delete_dungeon(&d);
      init_dungeon(&d);
      d.level = level;
      gen_dungeon(&d);

      //mvprintw(0,0, "Level: %d", d.level);

      config_pc(&d);

      //place pc on up stairs of new dungeon
      d.character[d.pc.position[dim_y]][d.pc.position[dim_x]] = NULL;
      d.pc.position[dim_x] = d.stair_up_loc[dim_x];
      d.pc.position[dim_y] = d.stair_up_loc[dim_y];
      d.character[d.pc.position[dim_y]][d.pc.position[dim_x]] = &d.pc;

      d.max_monsters += level;
      gen_monsters(&d, d.max_monsters, 0);
      
    }
    else if (pause == 3){
      //going up
      int level = d.level - 1;
      pc_delete(d.pc.pc);
      delete_dungeon(&d);
      init_dungeon(&d);
      d.level = level;
      gen_dungeon(&d);

      config_pc(&d);

      //place pc on down stairs of old dungeon
      d.character[d.pc.position[dim_y]][d.pc.position[dim_x]] = NULL;
      d.pc.position[dim_x] = d.stair_down_loc[dim_x];
      d.pc.position[dim_y] = d.stair_down_loc[dim_y];
      d.character[d.pc.position[dim_y]][d.pc.position[dim_x]] = &d.pc;

      d.max_monsters += level;
      gen_monsters(&d, d.max_monsters, 0);
    }
    pause = 0;
  }
  
  render_dungeon(&d);

  if (do_save) {
    write_dungeon(&d);
  }

  endwin();
  printf(pc_is_alive(&d) ? victory : tombstone);

  pc_delete(d.pc.pc);
  delete_dungeon(&d);

  return 0;
}
