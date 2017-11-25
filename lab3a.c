#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

const int SUCCESS=0,BADARGMTS=1,ERROR=2;
int image = -1; //Image file descriptor, initially invalid

/*      Global Offsets                   */
const int numGroups=1; //Only one single group in the file system
int superblockOffset = 1024;  //The superblock is always located at byte offset 1024 from the beginning of the file
int groupOffset = 1024+1024; //Offset of Superblock + SuperBlock(len)
uint8_t buf8;
uint32_t buf32;
uint16_t buf16;

/*  Globals Filesystem Values     */
int blockSize;// block size (in bytes, decimal)
int blocksPerGroup;// blocks per group (decimal)
int inodesPerGroup;// i-nodes per group (decimal)
int numBmap;   // block number of free block bitmap for this group (decimal)
int numImap;   // block number of free i-node bitmap for this group (decimal)
int inodeTable;   // block number of first block of i-nodes in this group (decimal)
int inodeSize;// i-node size (in bytes, decimal)

void superblock_summary(){

 int totBlocks;// total number of blocks (decimal)                                                                                                                                                          
 int totInodes;// total number of i-nodes (decimal)                                                                                                                                                           
 int non; // first non-reserved i-node (decimal)                                                                                                                                                              

  // total number of blocks (decimal)                                                                                                                                                                       
 pread(image, &buf32, 4, superblockOffset + 4);
 totBlocks = buf32;

 // total number of i-nodes (decimal)                                                                                                                                                                       
 pread(image, &buf32, 4, superblockOffset + 0);
 totInodes = buf32;

 // block size (in bytes, decimal)                                                                                                                                                                          
 pread(image, &buf32, 4, superblockOffset + 24);
 blockSize = 1024 << buf32;

 // i-node size (in bytes, decimal)                                                                                                                                                                         
 pread(image, &buf16, 2, superblockOffset + 88);
 inodeSize = buf16;

 // blocks per group (decimal)                                                                                                                                                                              
 pread(image, &buf32, 4, superblockOffset + 32);
 blocksPerGroup = buf32;

 // i-nodes per group (decimal)                                                                                                                                                                             
 pread(image, &buf32, 4, superblockOffset + 40);
 inodesPerGroup = buf32;

 // first non-reserved i-node (decimal)                                                                                                                                                                     
 pread(image, &buf32, 4, superblockOffset + 84);
 non = buf32;
 // Print out for CSV                                                                                                                                                                                       
 fprintf(stdout, "%s,%d,%d,%d,%d,%d,%d,%d\n", "SUPERBLOCK", totBlocks, totInodes, blockSize, inodeSize,
         blocksPerGroup, inodesPerGroup, non);
}

void group_summary(){
  
  int groupNumber=0;   // group number (decimal, starting from zero)
  int blocksInGroup=0; // total number of blocks in this group (decimal)
  int inodesInGroup=inodesPerGroup; // total number of i-nodes in this group (decimal)
  int numFreeBlocks=0; // number of free blocks (decimal)
  int numFreeInodes=0; // number of free i-nodes (decimal)

  groupNumber = 0; //only single group in the file system, always 0th index group

  pread(image, &buf32, 4, superblockOffset + 4);
  blocksInGroup = buf32;
  
  pread(image, &buf16, 2, groupOffset + 12);
  numFreeBlocks = buf16;
  
  pread(image, &buf16, 2, groupOffset + 14);
  numFreeInodes = buf16;
  
  pread(image, &buf32, 4, groupOffset);
  numBmap = buf32;
  
  pread(image, &buf32, 4, groupOffset + 4);
  numImap = buf32;

  pread(image, &buf32, 4, groupOffset + 8);
  inodeTable = buf32;
  
  fprintf(stdout, "%s,%d,%d,%d,%d,%d,%d,%d,%d\n","GROUP",groupNumber,blocksInGroup,inodesInGroup,numFreeBlocks,
          numFreeInodes,numBmap,numImap,inodeTable);
}

void freeblock_summary(){
  // Go through the block bitmap byte by byte                                                                                                                                                               
  for(int i = 0; i < blockSize; i++){
    // Get the byte
    pread(image, &buf8, 1, (numBmap * blockSize) + i);
    int8_t getBit = 1;
    // Go through the byte bit by bit
    for(int j = 1; j <= 8; j++){
      // See if the bit is set.                                                                                                                                                                         
      int checkAllocation = buf8 & getBit;
      // If it's not, then it's free and print it out.                                                                                                                                                  
      if (checkAllocation == 0)
        fprintf(stdout, "%s,%d\n", "BFREE", i * 8 + j);
      // Move on to the next bit.
      getBit = getBit << 1;
    }
  }
}

void freeinode_summary(){
  // Go through the inode bitmap byte by byte                                                                                                                                                               
  for(int i = 0; i < blockSize; i++){
    // Get the byte                                                                                                                                                                                       
    pread(image, &buf8, 1, (numImap * blockSize) + i);
    int8_t getBit = 1;
    // Go through the byte bit by bit                                                                                                                                                                     
    for(int j = 1; j <= 8; j++){
      // See if the bit is set.                                                                                                                                                                         
      int checkAllocation = buf8 & getBit;
      // If it's not, then it's free and print it out.                                                                                                                                                  
      if (checkAllocation == 0)
        fprintf(stdout, "%s,%d\n", "IFREE", i * 8 + j);
      // Move on to the next bit.                                                                                                                                                                       
      getBit = getBit << 1;
    }
  }
}

void inode_summary(){
  int inodeTableOffset = (inodeTable*blockSize);
  
  //for each Inode structure in the Inode table
  for (int i = 0; i <= inodesPerGroup; ++i) {
    //Offset of the beggining of the Inode structure
    int offset = inodeTableOffset + (inodeSize*i);
    
    int link_count;
    pread(image, &buf16, 2, offset + 26);
    link_count = buf16;
    
    int mode;
    pread(image, &buf16, 2, offset);
    mode = buf16;
    
    //Validate Inode structure
    if (mode!=0 && link_count!=0) {

      int owner;
      pread(image, &buf16, 2, offset + 2);
      owner = buf16;
      
      int group;
      pread(image, &buf16, 2, offset + 24);
      group = buf16;
      
      char fileType;
      int file = mode & 0xF000;
      switch (file){
        case 0x8000:
          fileType='f';
          break;
        case 0x4000:
          fileType='d';
          break;
        case 0xA000:
        fileType='s';
        break;
        default:
        fileType='?';
        break;
      }

      time_t     ctime,atime,mtime;
      struct tm  *cts,*ats,*mts;
      char       cbuf[80],abuf[80],mbuf[80];

      //time of last I-node change
      pread(image, &buf32, 4, offset + 12);
      ctime = buf32;
      cts = gmtime(&ctime);
      strftime(cbuf,sizeof(cbuf),"%D %H:%M:%S",cts);

      //modification time      
      pread(image, &buf32, 4, offset + 16);
      mtime = buf32;
      mts = gmtime(&mtime);
      strftime(mbuf,sizeof(mbuf),"%D %H:%M:%S",mts);

      //time of last access
      pread(image, &buf32, 4, offset + 8);
      atime = buf32;
      ats = gmtime(&atime);
      strftime(abuf,sizeof(abuf),"%D %H:%M:%S",ats);

      int numBlocks;
      pread(image, &buf32, 4, offset + 28);
      numBlocks = buf32;

      int fileSize;
      pread(image, &buf32, 4, offset + 4);
      fileSize = buf32;

      uint32_t blockAddress[15];
      for (int i = 0,j=0; i < 60; i+=4,j++) {
        pread(image, &buf32, 4, offset + 40 + i);
        blockAddress[j] = buf32;
      }

      mode = mode & 0x0FFF;

      fprintf(stdout, "%s,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n","INODE",(i+1),fileType,mode,owner,group,link_count,
              cbuf,mbuf,abuf,fileSize,numBlocks,blockAddress[0],blockAddress[1],blockAddress[2],blockAddress[3],blockAddress[4],
              blockAddress[5],blockAddress[6],blockAddress[7],blockAddress[8],blockAddress[9],blockAddress[10],blockAddress[11],
              blockAddress[12],blockAddress[13],blockAddress[14]);

      if (fileType == 'd'){
	     int logicalByteOffset = 0;
	     int inodeNumOfRefFile;
       int entryLen;
       int nameLen;

	     // 12 direct blocks
    	 for (int k = 0; k < 12; k++) {
	       int dataBlockNum = blockAddress[k];
	       // Make sure that the data block address is valid (not zero)
	       if (dataBlockNum != 0) {
	         // The current data block offset will be at the dataBlockNum times the size of a block in the file system.
	         int currOffset = blockSize * dataBlockNum;
	         // Go through the loop until we reach the end of the block.
	         while (currOffset < (blockSize * dataBlockNum) + blockSize) {
	           // Get the inode number of the referenced file.
	           pread(image, &buf32, 4, currOffset);
                   inodeNumOfRefFile = buf32;

    	           // Get the entry length.
	           pread(image, &buf16, 2, currOffset + 4);
                   entryLen = buf16;

	           // Get the name length.
	           pread(image, &buf8, 1, currOffset + 6);
	           nameLen = buf8;

	           // If the I-node number is 0, then it is not valid, we do not print anything
	           // and we restart the loop, moving the current offset to the next entry length.
	           if (inodeNumOfRefFile == 0) {
		          currOffset = currOffset + entryLen;
		          continue;
	           }

	           // Get the name of the file/directory
	           char name[100] = {0};
	           char bufForName;
	           for (int l = 0; l < nameLen; l++) {
		          pread(image, &bufForName, 1, currOffset + 8 + l);
		          name[l] = bufForName;
	           }

	           fprintf(stdout, "%s,%d,%d,%d,%d,%d,'%s'\n", "DIRENT", (i+1), logicalByteOffset, inodeNumOfRefFile, entryLen, nameLen, name);
	           // Move both the current offset and the logical byte offset by the entry length.
	           // The difference between the two is that the logical byte offset is equal to 0
	           // when currOffset is equal to blockSize * dataBlockNum.
	           currOffset = currOffset + entryLen;
	           logicalByteOffset = logicalByteOffset + entryLen;
	          }
	        }
	      }
      }
      
      int logicalBlockOffset = 0;

      // Single indirect block is valid
      if (blockAddress[12] != 0) {
	     // Beginning offset
	     logicalBlockOffset = 12;
	     // Get the indirect block being scanned.
       int indirectBlockNum = blockAddress[12];
	     // Go through the indirect block, for every 4 bytes
	     for(int k = 0; k < blockSize; k += 4){
	       // Get the inner data block number
	       pread(image, &buf32, 4, (indirectBlockNum * blockSize) + k);
	       int innerDataBlockNum = buf32;
	       // If the inner data block number is valid (not zero)
	       if(innerDataBlockNum != 0){
	         fprintf(stdout, "%s,%d,%d,%d,%d,%d\n","INDIRECT",(i+1), 1, logicalBlockOffset, indirectBlockNum, innerDataBlockNum);
	       }
	       // Increase the logical block offset by 1.
	       logicalBlockOffset++;
	     }
      }
      
      // Double indirect block is valid
      if (blockAddress[13] != 0) {
	     // Get the indirect block being scanned.
        int indirectBlockNum = blockAddress[13];
	     // Go through the indirect block, for every 4 bytes
        for(int k = 0; k < blockSize; k += 4){
	       // Get the inner indirect block
          pread(image, &buf32, 4, (indirectBlockNum * blockSize) + k);
          int innerIndirectBlockNum = buf32;
	       // If the inner indirect block number is valid (not zero)
          if(innerIndirectBlockNum != 0){
            fprintf(stdout, "%s,%d,%d,%d,%d,%d\n","INDIRECT",(i+1), 2, logicalBlockOffset, indirectBlockNum, innerIndirectBlockNum);
	         // Go through the inner indirect block, for every 4 bytes
            for (int l = 0; l < blockSize; l += 4){
	           // Get the inner data block number
              pread(image, &buf32, 4, (innerIndirectBlockNum * blockSize) + l);
              int innerDataBlockNum = buf32;
	           // If the inner data block number is valid (not zero)
              if (innerDataBlockNum != 0){
                fprintf(stdout, "%s,%d,%d,%d,%d,%d\n","INDIRECT",(i+1), 1, logicalBlockOffset, innerIndirectBlockNum, innerDataBlockNum);
              }
	           // Increase the logical block offset by 1.
	             logicalBlockOffset++;
            }
          }
	       // Increase the logical block offset by 255.
	       logicalBlockOffset += 255;
        }
      }
      
      // Triple indirect block is valid
      if (blockAddress[14] != 0) {
	     // Get the indirect block being scanned.
        int indirectBlockNum = blockAddress[14];
	     // Go through the indirect block, for every 4 bytes
        for(int k = 0; k < blockSize; k += 4){
	       // Get the inner indirect block
          pread(image, &buf32, 4, (indirectBlockNum * blockSize) + k);
          int innerIndirectBlockNum = buf32;
	       // If the inner data block number is valid (not zero) 
          if(innerIndirectBlockNum != 0){
            fprintf(stdout, "%s,%d,%d,%d,%d,%d\n","INDIRECT",(i+1), 3, logicalBlockOffset, indirectBlockNum, innerIndirectBlockNum);
	         // Go through the inner indirect block, for every 4 bytes
            for (int l = 0; l < blockSize; l += 4){
	           // Get the 2nd inner indirect block
              pread(image, &buf32, 4, (innerIndirectBlockNum * blockSize) + l);
              int innerIndirectBlockNum2 = buf32;
	           // If the 2nd inner data block number is valid (not zero)
              if (innerIndirectBlockNum2 != 0){
                fprintf(stdout, "%s,%d,%d,%d,%d,%d\n","INDIRECT",(i+1), 2, logicalBlockOffset, innerIndirectBlockNum, innerIndirectBlockNum2);
		            // Go through the 2nd inner indirect block, for every 4 bytes
		            for (int m = 0; m < blockSize; m += 4) {
		              // Get the inner data block number
                  pread(image, &buf32, 4, (innerIndirectBlockNum2 * blockSize) + m);
                  int innerDataBlockNum = buf32;
		              // If the inner data block number is valid (not zero)
                  if (innerDataBlockNum != 0){
                    fprintf(stdout, "%s,%d,%d,%d,%d,%d\n","INDIRECT",(i+1), 1, logicalBlockOffset, innerIndirectBlockNum2, innerDataBlockNum);
                  }
		              // Increase the logical block offset by 1.
		              logicalBlockOffset++;
                }
              }
	           // Increase the logical block offset by 255.
	           logicalBlockOffset += 255;
            }
          }
          logicalBlockOffset += 16777215;
        }	
      }
    }
  }
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

  group_summary();

  freeblock_summary();

  freeinode_summary();

  //also does directory entries and indirect block refrences
  inode_summary();

  return(SUCCESS);
}
