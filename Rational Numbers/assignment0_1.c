#include <stdio.h>

void tree ();
int bound = 100000000;
int arry[100000000];

int main () {
  tree();
}

void tree () {
  int i;
  arry[0] = 1;
  for (i=0; i < bound; i++){
    if (2*i+1 < 100000000){
      arry[2*i+1] = arry[i];
    }
    if (2*i+2 < 100000000){
       arry[2*i+2] = arry[i] + arry[i+1];
    }
    if (i+1 >= bound){
      printf("%d/%d\n", arry[i], 1);
    }
    else{
      printf("%d/%d\n", arry[i], arry[i+1]);
    }
  }
}
