all: RLG327

RLG327: dungeon_generation.c binheap.c path_finding.c
	gcc -Wall -Werror path_finding.c binheap.c dungeon_generation.c -o RLG327

clean:
	rm -f RLG327
