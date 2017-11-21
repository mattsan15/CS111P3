#include "ext2_fs.h"
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const int SUCCESS=0,BADARGMTS=1,ERROR=2;
int image = -1; //Image file descriptor, initially invalid

void superblock_summary(){
  //The superblock is always located at byte offset 1024 from the beginning of the file
  int offset = 1024;

int totBlocks;// total number of blocks (decimal)
int totInodes;// total number of i-nodes (decimal)
int blockSize;// block size (in bytes, decimal)
int inodeSize;// i-node size (in bytes, decimal)
int blocksPerGroup;// blocks per group (decimal)
int inodesPerGroup;// i-nodes per group (decimal)
 int non; // first non-reserved i-node (decimal)

  uint32_t buf;
  
  pread(image, &buf, 4, offset + 4);
 totBlocks = buf;
 // This prints out stuff!!!
 fprintf(stdout, "%d", totBlocks);
 //fprintf(stdout, "%d,%d,%d,%d,%d,%d,%d,%d\n", );
}

void group_summary(){
}

void freeblock_summary(){
}

void freeinode_summary(){
}

void inode_summary(){
}

void directory_entry(){
  /*
  For each directory I-node, scan every data block. For each valid (non-zero I-node number) directory entry, produce a new-line terminated line, with seven comma-separated fields (no white space).

  DIRENT
  parent inode number (decimal) ... the I-node number of the directory that contains this entry
  logical byte offset (decimal) of this entry within the directory
  inode number of the referenced file (decimal)
  entry length (decimal)
  name length (decimal)
  name (string, surrounded by single-quotes). Don't worry about escaping, we promise there will be no single-quotes or commas in any of the file names.
  */  
}

void indirectblock_references(){
  /*The I-node summary contains a list of all 12 blocks, and the primary single, double, and triple indirect blocks.
    We also need to know about the blocks that are pointed to by those indirect blocks. For each file or directory I-node, scan the single indirect blocks and (recursively) the double and triple indirect blocks.
    For each non-zero block pointer you find, produce a new-line terminated line with six comma-separated fields (no white space).

  INDIRECT
  I-node number of the owning file (decimal)
  (decimal) level of indirection for the block being scanned ... 1 single indirect, 2 double indirect, 3 triple
  logical block offset (decimal) represented by the referenced block. If the referenced block is a data block, this is the logical block offset of that block within the file. If the referenced block is a single- or double-indirect block, this is the same as the logical offset of the first data block to which it refers.
  block number of the (1,2,3) indirect block being scanned (decimal) ... not the highest level block (in the recursive scan), but the lower level block that contains the block reference reported by this entry.
  block number of the referenced block (decimal)
  Logical block is a commonly used term. It ignores physical file structure (where data is actulally stored, indirect blocks, sparseness, etc) and views the data in the file as a (logical) stream of bytes. If the block size was 1K (1024 bytes):

  bytes 0-1023 would be in logical block 0
  bytes 1024-2047 would be in logical block 1
  bytes 2048-3071 would be in logical block 2
  ...
  You can confirm your understanding of logical block numbers by looking at the INDIRECT entries in the sample output.
  If an I-node contains a triple indirect block:

  the triple indirect block number would be included in the INODE summary.
  INDIRECT entries (with level 3) would be produced for each double indirect block pointed to by that triple indirect block.
  INDIRECT entries (with level 2) would be produced for each indirect block pointed to by one of those double indirect blocks.
  INDIRECT entries (with level 1) would be produced for each data block pointed to by one of those indirect blocks.
  */  
}


int main (int argc, char *argv[]) {
  if (argc != 2) { //Single image file arguement
    fprintf(stderr,"Incorrect number of Arguements!\nUsage: ./lab3 EXT2_test.img\n");
    exit(BADARGMTS);
  }

  image = open(argv[1],O_RDONLY); //attempt to open image file
  if (image == -1) {
    perror("image file error");
    exit(BADARGMTS);
  }


superblock_summary();
/*
group_summary();

freeblock_summary();

freeinode_summary();

inode_summary();

directory_entry();

indirectblock_references();
*/
return(SUCCESS);
}
