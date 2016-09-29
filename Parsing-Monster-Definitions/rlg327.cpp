#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "dungeon.h"
#include "path.h"
#include "pc.h"
#include "npc.h"
#include "move.h"
#include "io.h"

using namespace std;

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
  "             /____ /   Rest in   \\           ||\n"
  "            |     |    Pieces     |          ||\n"
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



void usage(char *name)
{
  fprintf(stderr,
          "Usage: %s [-r|--rand <seed>] [-l|--load [<file>]]\n"
          "       [-i|--image <pgm>] [-s|--save]\n",
          name);

  exit(-1);
}

string trim(string& str)
{
  size_t first = str.find_first_not_of(' ');
  size_t last = str.find_last_not_of(' ');
  return str.substr(first, (last-first+1));
}

int main(int argc, char *argv[])
{
  dungeon_t d;
  time_t seed;
  struct timeval tv;
  int32_t i;
  int32_t do_load, do_save, do_seed, do_image;
  int32_t long_arg;
  char *save_file;
  char *pgm_file;

  memset(&d, 0, sizeof (d));

  /* Default behavior: Seed with the time, generate a new dungeon, *
   * and don't write to disk.                                      */
  do_load = do_save = do_image = 0;
  do_seed = 1;
  save_file = NULL;
  d.max_monsters = 10;

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


  ifstream monster_file;
  string out;
  string name;
  string sym;
  string color;
  string desc;
  string speed;
  string dmg;
  string hp;
  string abil;
  string keyword;

  string path;
  path += getenv("HOME");
  path += "/.rlg327/monster_desc.txt";
  
  int metadata = 0;
  
  //handle monster type and generation
  monster_file.open(path.c_str());

  if (!monster_file.is_open()){
    cout<<"file not opened."<< endl;
    return -1;
  }
  while (1){

    out.erase();
    name.erase();
    sym.erase();
    color.erase();
    desc.erase();
    speed.erase();
    dmg.erase();
    hp.erase();
    abil.erase();

    if (!metadata){
      getline(monster_file,out);
      if (out.compare("RLG327 MONSTER DESCRIPTION 1") != 0){
	cout<< "Metadata structure invalid" <<endl;
	monster_file.close();
	return -1;
      }
      cout<< "metadata matched" << endl << endl;
      metadata = 1;
    }
    else {
      while (getline(monster_file, out) && out.compare("BEGIN MONSTER") != 0);

      if (out.compare("BEGIN MONSTER")) break;

      while (getline(monster_file, out) && out.compare("END") != 0){

	if (out.empty()){
	  continue;
	}
	
	istringstream iss(out);
	string key;
	
	iss >> keyword;
	
	if (keyword.compare("NAME") == 0){
	  //	  cout << "name" << endl;
	  while (iss >> key){
	    name += key;
	    name += " ";
	  }
	  name = trim(name); //erases trailing character (space)
	}
	else if (keyword.compare("DESC") == 0){
	  //cout << "desc" << endl;
	  while (getline(monster_file,out) && out.compare(".") != 0){
	    if(out.empty()){
	      continue;
	    }
	    desc += out;
	    desc += "\n";
	  }
	  desc = desc.substr(0, desc.size() - 1);
	}
	else if (keyword.compare("SYMB") == 0){
	  //cout << "symbol" << endl;
	  iss >> sym;
	}
	else if (keyword.compare("COLOR") == 0){
	  //cout << "color" << endl;
	  iss >> color;
	}
	else if (keyword.compare("SPEED") == 0){
	  //cout << "speed" << endl;
	  iss >> speed;
	}
	else if (keyword.compare("ABIL") == 0){
	  //cout << "ability" << endl;
	  while(iss >> key){
	    abil += key;
	    abil += " ";
	  }
	  abil = trim(abil); //deletes trailing space
	}
	else if (keyword.compare("HP") == 0){
	  //cout << "hp" << endl;
	  iss >> hp;
	}
	else if (keyword.compare("DAM") == 0){
	  //cout << "damage" << endl;
	  iss >> dmg;
	}
      }
      
      if (out.compare("END")) break;
      
      if (!(name.empty() || sym.empty() || desc.empty() || color.empty() ||
      	    speed.empty() || abil.empty() || hp.empty() || dmg.empty()))
      {
	  cout << name << endl << desc << endl << sym << endl << color << endl
	       << speed << endl << abil << endl << hp << endl << dmg << endl << endl;
      }
    }
  }
  /* commented out for this assignment
     io_init_terminal();
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

  io_display(&d);
  while (pc_is_alive(&d) && dungeon_has_npcs(&d) && !d.save_and_exit) {
    do_moves(&d);
    if (!pc_is_alive(&d)) {
      break;
    }
  }
  io_display(&d);
  if (!d.save_and_exit) {
    sleep(2);
  }

  io_reset_terminal();

  if (do_save) {
    write_dungeon(&d);
  }

  printf(pc_is_alive(&d) ? victory : tombstone);
  */
  /* PC can't be deleted with the dungeon, else *
   * it disappears when we use the stairs.      */
  //if (pc_is_alive(&d)) {
    /* If the PC is dead, it's in the move heap and will get automatically *
     * deleted when the heap destructs.  In that case, we can't call       *
     * delete_pc(), because it will lead to a double delete.               */
    //delete_pc(d.pc);
  //}
  //delete_dungeon(&d);
  
  return 0;
 
}
