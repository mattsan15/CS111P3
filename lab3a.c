#include <unistd.h>
#include <stdint.h>
#include "ext2_fs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const int SUCCESS=0,BADARGMTS=1,ERROR=2;

int main (int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Incorrect number of Arguements!\n");
    fprintf(stderr, "Usage: ./lab3 EXT2_test.img\n");
    exit(1);
  }

  char arg1[14];
  char imageFile[14];
  strcpy(arg1,argv[1]);
  strcpy(imageFile,"EXT2_test.img");
  fprintf(stderr,"%c\n",arg1);
  fprintf(stderr,"%c\n",imageFile);
  
  if(strcpy(imageFile,arg1)!=0){
    fprintf(stderr, "Incorrect Image File!\n");
    fprintf(stderr, "Usage: ./lab3 EXT2_test.img\n");
    exit(1);
  }
  //Project goes here!
}
