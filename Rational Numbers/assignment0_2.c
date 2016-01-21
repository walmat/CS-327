#include <stdio.h>
#include <stdlib.h>

int rec_tree(int n);

int main(int argc, char* argv[]) {

  if (argc != 2){
    printf("Error: please provide two arguments.");
    return -1;
  }

  int n = atoi(argv[1]);

  printf("%d/%d\n", rec_tree(n-1), rec_tree(n));
  
  return 0;
}
  
int rec_tree(int n){

  if (n < 2){
    return 1;
  }
  if (n % 2 == 0){
    return rec_tree((n / 2) - 1) + rec_tree(n / 2);
  }
  else {
    return rec_tree(n / 2);
  }  
}
