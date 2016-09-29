#ifndef PATH_H
# define PATH_H

# ifdef __cplusplus
extern "C" {
#endif

typedef struct dungeon dungeon_t;

void dijkstra(dungeon_t *d);
void dijkstra_tunnel(dungeon_t *d);

# ifdef __cplusplus
}
#endif

#endif
