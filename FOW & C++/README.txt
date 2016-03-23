dungeon_generation.c:

**1.01** VERSION 0.1-0.6

Thus far the program accomplishes the following in order:
     1. generates a 80x21 dungeon filled with rock
     2. based on the difficulty provided, randomly generates and places the
        rooms on random (x,y) coordinates in the 80x21 grid.
     3. draws corridors from room 1 to room n sequentially. (1-2 2-3..(n-1)-n)
     4. prints the screen at the end.

A couple notable functions are generate_rooms() & connect_rooms();

   generate_rooms():
	So this function has a couple special things happening at once. Firstly
	it generates a percentage of the room to be filled based on the diffic-
	ulty provided. It selects a random room to be placed inside the switch
	statement. It then loops through the array dungeon[21][80] and
	attempts to place these rooms through the is_room_placeable() function.

	Next, it performs a selection sort on the array of rooms once all the
	rooms have been placed properly.


   connect_rooms():
	This one was what took the most time for me. It starts by identifying
	whether the room next to the one being checked is either: 1- above it,
	2- below it, or 3- directly parallel to it. In case 1, it attempts to
	path up to the second rooms y coordinate + height/2, and then path
	right to the second rooms x coordinate at height/2. Similarly, case 2
	paths down and then right. Case 3 just paths right.

Want to change the global array rooms[60][4] to malloc at some later date because as of now, it's worst case senario (max difficulty * percentage/ 4x4 room).

**1.02** VERSION 0.7

I've added two main functions to the program: save_dungeon() and load_dungeon(). These operate just as they sound, and they perform based off of command line arguments. If the user prompts for a --save & --load it will always load it first and then save it. File format is as follows:
     BYTES     	    



Both functions save to the location: $HOME/User/.rlg327/

     save_dungeon():
	The gist of this is to read in a specified file format and convert it
	to a generated dungeon. The dungeon then can be loaded at a later time
	by knowing the name you saved it under.

     load_dungeon():
	The basis of this function is to load a dungeon based on a specified
	file in which the user passes in. It can either be a new dungeon, or a
	previously saved one.

Example run arguments:

1	./dungeon_generation
	
2	./dungeon_generation --save
	> name_of_file

3	./dungeon_generation --load
	> name_of_file
	
4	./dungeon_generation --load --save
	> name_of_file

**1.03** VERSION 0.8

I started this weeks portion of the project by splitting my one file into many files and naming them according to what they do. I have the following so far.

  1. dungeon_generation.c
  2. dungeon_generation.h
  3. main.c (unused)
  4. path_finding.c

There are other files that were provided for us.

  1. binheap.c
  2. binheap.h

I bridge all these together into a binary named RLG327 (Roguelike Game 327).

What I've done after that is I basically had to write a way to implement Djikstras Algorithm with path finding. On top of this I had to check the hardness of each cell and make the monster's movement be based on that and the distane to the PC. Also, since some monsters were able to tunnel and some weren't, I had to have that check in there as well. After that, my priority queue allowed the stack "decrease_key" in order to get the shortest distance to the PC.

I had to change the way I was generating my dungeon around a little bit as well. Previously I was using a global array, but now I'm using a struct which holds all the useful information I need to check in path_finding.c like hardness, tunnel ability/not, x, y, etc..

**1.04** VERSION 0.9

I started out by implementing the player characters movement. I have a function that places the player character, and one that takes care of all 8 directions of movement. Suffered quite a bit with this, and it took a lot longer than I wanted to get the struct updated to figure out the current and next player character's location. Monster's placement is handled by a function called place_monsters() and for now I have a static number of 10 monsters being placed (will implement command line later). I then do a 50% chance to each monster of having each ability covered in get_attributes() and finally assign the monsters represented character in pick_monster(). Then I place the monsters similarly to the placement of the player character. I put the player character as the min element in binheap, and add the monsters underneath that, and each move is based on that. Unfortunately still a lot of work to be done on this week's assignment. Was a lot more hefty than I imagined, but I did put a lot of time into this.

**1.05** VERSION 1.0

This weeks assignment I started by taking Jeremy's 1.04 code.

Were were asked to implement ncurses into the working code. The only thing I had to do was change the printf() statements inside the render_dungeon() function to mvaddch(). Likewise, in the main method inside rlg327.c, I had to do the following calls:

     initscr(); -> initializes the screen
     raw(); -> no need to press enter
     keypad(stdscr, TRUE); -> enables f-keys, arrows, etc.
     define_keys(); -> defines the different key values on the num pad
     noecho(); -> won't print the char input to stdscr
     curs_set(0); -> sets cursor to invisible

The dungeon alone is controlled by a variable I named 'pause' which determines which state of the game I'm in.

pause == 0:
      game is rendered normally and character movement is enabled
pause == 1:
      game is paused and the monster list is enabled, movement is disabled
pause == 2:
      PC is entering an up staircase and a new dungeon is generated
pause == 3:
      PC is entering a down staircase and a new (harder) dungeon is generated

The rest of the code is basically identical to 1.04 other than a few more struct variables added to dungeon_t

Most challenging part of this assignment was managing a pointer to an array of pointers, in this case, monsters (which can be found in pause == 1).

**1.06** VERSION 1.1

To start of this week, I first had to convert all the 1.05 code we had to c++, which was the trickiest part. I had to wrap the header files for pc,npc,& character in __cplusplus ifs and then convert the structs I had for character and npc to classes. I added in getters and setters for the variables that were inside the structs for both classes. Other than that, creating the .cpp files was a matter of creating the getters and setters and adding in the functions from the corresponding .c file.

After this, I implemented a couple functions to take care of the fog of war. All of which do exactly what they sound like they do.

      init_remem_dungeon():
	fills the first dungeon will a layer full of walls (spaces).
      get_remem_dungeon():
	fills the remembered 3x3 grid and placed the correct characters.
      update_remem_dungeon():
	find the placed PC and construct a 3x3 vision map around it.

Other than this, nothing much else was changed from the previous week.
