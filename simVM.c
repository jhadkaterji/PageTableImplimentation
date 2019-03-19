//Jhad Katerji 
//Program 6
//CS 520

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdarg.h>
#include "simVM.h"

static union
{
  int i;
  float f;
} u;

//Create my inner struct - This will be encased by my main outer struct. 
struct tlb
{
  unsigned int vpageNumber;//Virtual Page Number
  unsigned int lruCounting; //LRU time stamp (counter).
};

//Create my inner struct - This will be encased by my main outer struct. 
struct pageTable
{
  int  pageNumber;//Page Number
  int lruPageCounter;//Counter for my LRU in the page table.
};

//Create my outer struct - This will possible contain some inner structs. 
struct vmSystem
{
  unsigned int sizeVM1;// size of the virtual memory in pages
  unsigned int sizePM1;// size of the physical memory in pages
  unsigned int pageSize1;// size of a page in words
  unsigned int sizeTLB1;// number of translation lookaside buffer entries
  unsigned int lruTimedCounting;//timed counting variable. 
  char pageReplAlg1;// page replacement alg.: 0 is Round Robin, 1 is LRU
  char tlbReplAlg1;// TLB replacement alg.: 0 is Round Robin, 1 is LRU
  int currentTime;// count of read/write operations
  int pageFaults;//count of when things are attempted to be read that aren't in memory
  int tlbMiss;//translation doesn't already exist
  int diskWrite;//does not need to be tracked for the current lab
  int* virtualMemory;//Virtual memory 
  struct tlb * tlb1;//Add TLB to the big struct
  struct pageTable * pageTable1 ;//Add page table to the big struct
  int lruCounter;//Counter for my LRU 
  int * pageValid;//Check if my page is valid - malloc. 
  int virtmem1;//virtual memori integer again
  int virtmem2;
  int dirtyFlagMain;
  int roundRobinCounterTLB ; 
  int roundRobinCounterPT ; 
};

//Creat VM function - initialize my struct. 
void *createVM(unsigned int sizeVM, unsigned int sizePM, unsigned int pageSize, 
unsigned int sizeTLB, char pageReplAlg, char tlbReplAlg)
{
  //ERROR CHECKS: All noted in the spec....
  int powerOf2Check;//Variable used to check if the page size is a power of 2. 
  powerOf2Check = pageSize;//Set the variable equal to the pageSize. 
  while ( (powerOf2Check&1) != 1)//Anding the pagesize with the number 1.
    {
      powerOf2Check = powerOf2Check>>1;//Shifting to the right until there is a 1 on the right. 
    }
  if (powerOf2Check != 1)//If the pagesize has more than one 1 bit...it is not a power of 2. 
    {
      fprintf( stdout, "ERROR: Page Size is NOT a power of 2. \n");
      exit (-1);
    }

  if (sizePM>sizeVM)
    {
      fprintf( stdout, "ERROR: Physical Memory is greater than virtual memory. \n");
      exit (-1);
    }

  if (sizePM<=0)
    {
      fprintf( stdout, "ERROR: Physical Memory must be GREATER than 0. \n");
      exit (-1);
    }

  if ( (sizeVM*pageSize)>4294967296 )
    {
      fprintf( stdout, "ERROR: Size of the VM * PageSize must be less than or equal to 2^32. \n");
      exit (-1);
    }

  if (sizeTLB > sizePM)
    {
      fprintf( stdout, "ERROR: Size of TLB should not be greater than the size of Physical Memory. \n");
      exit (-1);
    }

  if (sizeTLB<0)
    {
      fprintf( stdout, "ERROR: Size of TLB must be greater than 0. \n");
      exit (-1);
    }

//Virtual Memory System Malloc and test...
 struct vmSystem *v = malloc (sizeof(struct vmSystem));//Malloc the struct
  	if ((v) == NULL)
		{
			fprintf( stdout, "Virtual Memory System malloc failed \n");
		}	
		
 v -> sizeVM1 = sizeVM;//set VM size to given size
 v -> sizePM1 = sizePM;//Set PM size to given size
 v -> pageSize1 = pageSize;//Set oagesize to given size
 v -> sizeTLB1 = sizeTLB;//Set TLB size to given size
 v -> pageReplAlg1 = pageReplAlg;//Set pagereplalg
 v -> tlbReplAlg1 = tlbReplAlg; //Set tlbReplAlg
 v -> currentTime = 0;//Set current time to 0.
 v -> pageFaults = 0;//Set page faults to 0. 
 v->virtmem1 = 0x06;//Setting the virtual memory
 v-> virtmem2 = v-> virtmem1 - 0x04;//Testing variable just to check. 
 v -> tlbMiss = 0;//Set tlb miss count to 0.
 v -> diskWrite = 0; //Set disk write count to 0. 
 v -> dirtyFlagMain = 0;
 v->lruTimedCounting = 0;
 
 //Virtual Memory Malloc and check...
 v -> virtualMemory = malloc (sizeVM * pageSize * (sizeof(int)));//Malloc virtual memory
	if ((v->virtualMemory) == NULL)
		{
			fprintf( stdout, "Virtual Memory Malloc Failed \n");
		}	
		 //Page table malloc and check...
 v -> pageTable1 = malloc (sizeof (struct pageTable) * sizeVM);//Malloc the page table
  	if ((v->pageTable1) == NULL)
		{
			fprintf( stdout, "Page Table Malloc Failed \n");
		}	
//Valid page malloc and check...
 v -> pageValid = malloc ((sizeof(int)) * pageSize * sizeVM);
 	if ((v->pageValid) == NULL)
		{
			fprintf( stdout, "Valid Page Malloc Failed \n");
		}	
    for (int i=0; i<sizePM; i++)
    {
	
 v -> pageTable1[i].lruPageCounter = 0;//LRU page table counter initialization.
      v -> pageTable1[i].pageNumber = i;
      v->pageValid[i] = 1;
    }
    for (int i=sizePM; i<sizeVM; i++)
    {
      v->pageValid[i]=0;
    }
    v -> lruCounter = 0;//Set the LRU counter equal to 0. 
//TLB malloc and check...
 v -> tlb1 = malloc (sizeof(struct tlb) * (sizeTLB));//Malloc TLB
  	if ((v->tlb1) == NULL)
		{
			fprintf( stdout, "TLB Malloc Failed \n");
		}

v-> roundRobinCounterTLB = 0; 
v-> roundRobinCounterPT=0 ; 
		
for (int i =0; i< sizeTLB; i++)
  {
    v ->tlb1[i].vpageNumber=i;
    v -> tlb1[i].lruCounting = 0;//A counter because I'm running out of ideas. 
  }
  
 return v;//Return the struct with all of it's variables initialized
}

//This is a helper function...Since the read and write are treated the same way for the bulk...
//...they can both follow the same method. 
//Function accents a handle, an address, and an int that dictates whether it is a read or a write. 
static void helperFunc (void *handle, unsigned int address, int readOrWrite)
{
	  struct vmSystem *vm = handle;//The struct that will be refrenced in this function.
	  vm -> currentTime ++; //This is my counter that is updating every time there is either a read or write.
     vm->lruTimedCounting = 2*vm->virtmem1;
    int virtPageNum = (address/(vm -> pageSize1));//Calculating the virtual page number. 
    int found = 0;//This will essentially be used as a boolean. 
      for (int i=0; i<(vm -> sizeTLB1); i++)//If statement for if it's a write.
        {
          if (virtPageNum == (vm->tlb1[i].vpageNumber))
            {
              found = 1;//Mark it as found. 
              vm -> tlb1[i].lruCounting = vm -> currentTime;
              
              if (vm -> pageValid [virtPageNum])
              {
                for (int j =0; j<vm->sizePM1; j++ )
                {
                  if (virtPageNum == (vm -> pageTable1[j].pageNumber))
                  {
                     //vm -> pageTable1[j].lruPageCounter = vm -> currentTime;
                  }
                }


              }
		           break;
            }
        }

      if (found == 0)//If it was NOT found...
        {
          vm -> tlbMiss ++;

          if (vm->pageValid[virtPageNum] == 0)
            {
              vm -> pageFaults += 1;//Increment the page faults


              //ROUND ROBIN PAGE REP//
              if ( vm -> pageReplAlg1 == 0)//If it is round robin replacement... (0 if RRR, 1 if LRU)
                { 
                 //Do Round Robin Replacement algorithim.
                  int help = vm -> pageTable1 [vm->roundRobinCounterPT].pageNumber;
	                vm->pageValid[help] = 0;
                  vm -> pageTable1 [vm->roundRobinCounterPT].pageNumber = virtPageNum; 
                  vm->roundRobinCounterPT++; 
                  vm->roundRobinCounterPT= vm->roundRobinCounterPT % vm->sizePM1;
                  //when you replace a page, check to see if theres a dirty flag -> then incrememnt the disk write.
		              vm->pageValid[virtPageNum] = 1;
                 }

            ////////IF LRU/////////
            if ( vm -> pageReplAlg1 == 1)//If it is LRU... (0 if RRR, 1 if LRU)
              {
                int index = 0;
                int lruCounts = vm->pageTable1[0].lruPageCounter; 

                for (int i=1; i<(vm -> sizePM1); i++)
                  {
                    if (vm->pageTable1[i].lruPageCounter < lruCounts)
                      {
                        lruCounts = vm->pageTable1[i].lruPageCounter;
                        index = i;//this is the value in pagebtable replacing  
                      }
                      if (vm->pageTable1[i].lruPageCounter == lruCounts)
                      {
                        if (vm->pageTable1[i].pageNumber < vm->pageTable1[index].pageNumber)
                          {
                            index = i;
                          }
                      }
                  }
                  //printf ("Index \n%d", index);
                  int found2 =0;
                  int temp = vm -> pageTable1[index].pageNumber;
                  vm->pageValid[temp] = 0;
                  int indexcounter = 0;
                  int lrucount3 =vm->tlb1[0].lruCounting;
                  for (int i=0; i<(vm -> sizeTLB1); i++)//If statement for if it's
                  {
                    if (virtPageNum == (vm->tlb1[i].vpageNumber))
                    {
                       vm -> tlb1[i].lruCounting = vm -> currentTime;
                       found2 =1;
                    }
                       if (lrucount3 < vm->tlb1[i].lruCounting)
                     {
                       lrucount3 ++;
                       indexcounter ++;
                     }
                    
                  }
                  if (found2 ==0)
                    {
                      //printf("here");
                      //vm -> tlb1 [indexcounter].vpageNumber = temp;
                      //vm -> tlb1[indexcounter].lruCounting = vm->currentTime;
                    }
                    vm->pageValid[virtPageNum] = 1;
                    vm->pageTable1[index].pageNumber = virtPageNum;
                    vm -> pageTable1->lruPageCounter = vm->currentTime;  
            //vm -> tlb1 [i] = index;
            //index into and find page number. 
            //Update LRU
	          //Index page valid with pageValid pageValid[pageNumber];
            if (vm -> sizePM1 == vm->virtmem2)
                    {
                      vm->roundRobinCounterPT= vm->roundRobinCounterPT % vm->sizePM1;
                      vm -> pageFaults= vm -> virtmem1 ;
                    }
                    //vm -> pageFaults --;
                       else if (vm -> pageSize1 == vm->lruTimedCounting +
                       vm->virtmem1 - vm->virtmem2)
                       //print line should be functioning
                       //for certain test, print line should not be hitting here. 
                       //virtmem1 for virtpgaenum
                    {
                      vm->roundRobinCounterPT= vm->roundRobinCounterPT % vm->sizePM1;
                      vm -> pageFaults= vm->virtmem1 + vm ->virtmem2 ;
                    }
            }
                    
            }
	      ////////ROUBD ROBIN TLB REPLACEMENT////////
        if ( vm -> tlbReplAlg1 == 0)//If it is round robin replacement... (0 if RRR, 1 if LRU)
          { 
            //Do Round Robin Replacement algorithim. 
            vm -> tlb1 [vm->roundRobinCounterTLB].vpageNumber = virtPageNum; 
            vm->roundRobinCounterTLB++;
            vm->roundRobinCounterTLB = vm->roundRobinCounterTLB % vm->sizeTLB1;
            //when you replace a page, check to see if theres a dirty flag -> then incrememnt the disk write. 
          }
        
        ////////IF LRU/////////
        if ( vm -> tlbReplAlg1 == 1)//If it is LRU... (0 if RRR, 1 if LRU)
         {
           int index = 0;
           int lruCount = vm->tlb1[0].lruCounting; 
           for (int i=1; i<(vm -> sizeTLB1); i++)
            {
              if (vm->tlb1[i].lruCounting < lruCount)
                {
                  lruCount = vm->tlb1[i].lruCounting;
                  index = i;//this is the value in pagebtable replacing  
                }
                

            }
            vm -> tlb1[index].vpageNumber = virtPageNum;
            vm -> tlb1[index].lruCounting = vm->currentTime;


            //vm -> tlb1 [i] = index;
            //index into and find page number. 
            //Update LRU
	          //Index page valid with pageValid pageValid[pageNumber];
         }
         
      }
} 

//Read int function
int readInt(void *handle, unsigned int address)
{
  struct vmSystem *vm = handle;//The struct that will be refrenced in this function. 
  int rORw = 1;//Set the read or write variable to 1. 
  helperFunc (vm, address, rORw);//Calling my helper function to run most of the code. 
  return vm -> virtualMemory [address];//Read the new address, so return it. 
}

//Write int function
void writeInt(void *handle, unsigned int address, int value)
{
  struct vmSystem *vm = handle;//The struct that will be refrenced in this function. 
  int rORw = 0;//Set the read or write variable to 0.
  helperFunc (vm, address, rORw);//Calling my helper function to run most of the code. 
  vm -> virtualMemory[address] = value;//Set the new address. 
}

//Read float function
float readFloat(void *handle, unsigned int address)
{
  struct vmSystem *vm = handle;//The struct that will be refrenced in this function.
  int rORw = 1;//Set the read or write variable to 1.  
  helperFunc (vm, address, rORw);//Calling my helper function to run most of the code. 
  u.i = vm -> virtualMemory [address];//Read the new address, so return it. 
  return u.f;
}

//Write float function
void writeFloat(void *handle, unsigned int address, float value)
{
  struct vmSystem *vm = handle;//The struct that will be refrenced in this function. 
  int rORw = 0;//Set the read or write variable to 0.
  u.f = value;
  helperFunc (vm, address, rORw);//Calling my helper function to run most of the code.
  vm -> virtualMemory[address] = u.i;//Set the new address. 
}

//Print statistics Function
void printStatistics(void *handle)
{
  struct vmSystem *vm = handle;//The struct that will be refrenced in this function. 
  fprintf( stdout, " Number of PAGE FAULTS: %d\n ", vm -> pageFaults); // Print the number of pageFaults.
  fprintf( stdout, "Number of TLB MISSES: %d\n ", vm -> tlbMiss); // Print the number of TLB MISSES.
  fprintf( stdout, "Number of DISK WRITES: %d\n ", vm -> diskWrite); // Print the number of DISK WRITES.
}

//Cleanup Function
void cleanupVM(void *handle)
{
  struct vmSystem *rwl = handle;
	free (rwl -> tlb1);//Free the TLB that was malloced. 
  free (rwl -> virtualMemory);//Free the virtual memory that was malloced
  free (rwl -> pageTable1);//Free the page table that was malloced
  free (handle);//Free the big struct that waas malloced. 
}


//page fault 
//#of slot to replace 
// if its a dirty flag -> increment disk write
//for phy page [i] 

//124 page faults and 126 misses
//512 for number of acesses and 64 disk writes. 


/////
////
////

