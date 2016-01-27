dungeon_generation.c:
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

Want to change the global array rooms[60][4] to malloc at some later date because as of now, it's worst case senario.
