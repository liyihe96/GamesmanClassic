#include "hash.h"


/**
 ** generic_hash_context_init() : generates the req'd arrays for a game
 **                    returns an int that refers to this context.
 ** generic_hash_context_switch(int context) : switch to the indicated context
 **
 ** generic_hash_cur_context() : returns the current context
 **
 ** generic_hash_max_pos() : returns the current context's gNumberOfPositions.
 **      --possibly--
 ** use only one global array of a new struct that we are going to make to keep
 ** track of all of local arrays for each context.
 **
 ** things in struct: offset, memoized combinations, pieces, bookeeping for
 **  things such as "has g_h_init been called?"
 ** everything else can be local or not depending on speed.
 **
 ** get rid of global pointers.
 **
 **/



/*********************************************************************************
*** A *PERFECT* hash function -
*** - Efficency, Readability, and Multiple Contexts by Scott Lindeneau
*** - User-specified variables version designed and implemented by Michel D'Sa
*** - Original (3-variable only) version designed by Dan Garcia
***   and implemented by Attila Gyulassy
*********************************************************************************/


/* Global Variables */
struct hashContext **contextList = NULL;
int hash_tot_context = 0;


/* DDG Tried to fix build by externing in hash.h, declaring in hash.c */
/* 2004-10-20 */
POSITION *hash_hashOffset = NULL;
POSITION *hash_NCR = NULL;
POSITION *hash_miniOffset = NULL;
char *hash_pieces = NULL;
int *hash_localMins = NULL;
int *hash_thisCount = NULL;
int *hash_miniIndices = NULL;
int hash_numPieces = 0;
int hash_boardSize = 0;
int hash_usefulSpace = 0;
int hash_maxPos = 0;



/*******************************
**
**  Generic Hash Initialization
**
*******************************/

/* initializes hash tables and critical values */
POSITION generic_hash_init(int boardsize, int *pieces_array, int (*fn)(int *))
{
  int i = -1, k;
  POSITION sofar,temp;
  if(NULL == cCon)
    ExitStageRightErrorString("Attempting to Initialize invalid hash context");

  if(cCon->init)
    freeHashContext();

  cCon->init = TRUE;
  cCon->numPieces = hash_countPieces(pieces_array);
  cCon->gfn = fn;

  cCon->pieces = (char*) SafeMalloc (sizeof(char) * cCon->numPieces);
  cCon->mins = (int*) SafeMalloc (sizeof(int) * cCon->numPieces);
  cCon->maxs = (int*) SafeMalloc (sizeof(int) * cCon->numPieces);
  cCon->nums = (int*) SafeMalloc (sizeof(int) * cCon->numPieces);
  cCon->thisCount = (int*) SafeMalloc (sizeof(int) * cCon->numPieces);
  cCon->localMins = (int*) SafeMalloc (sizeof(int) * cCon->numPieces);
  cCon->gpdStore = (int*) SafeMalloc (sizeof(int) * cCon->numPieces);
  cCon->miniOffset = (POSITION*) SafeMalloc (sizeof(POSITION) * (cCon->numPieces+2));
  cCon->miniIndices = (int*) SafeMalloc (sizeof(int) * (cCon->numPieces+2));
  
  getPieceParams(pieces_array, cCon->pieces, cCon->mins,cCon->maxs);
  for (i = 0; i < cCon->numPieces;i++){
    cCon->nums[i] = cCon->maxs[i] - cCon->mins[i] + 1;
  }
  
  temp = 0;
  cCon->numCfgs = 1;
  for (i = 0; i < cCon->numPieces;i++)
    {
      if(cCon->numCfgs < temp)
	ExitStageRightErrorString("Generic Hash: Number of Cfg's wrap on current POSITION type");
      cCon->numCfgs *= cCon->nums[i];
      temp = cCon->numCfgs;
    }
  cCon->pieceIndices = (int*) SafeMalloc (sizeof(int) * cCon->numCfgs);
  
 
  nCr_init(boardsize);
  //hash_combiCalc();  
  cCon->hashOffset = (POSITION*) malloc(sizeof(POSITION) * (cCon->usefulSpace+2));
  cCon->offsetIndices = (int*) malloc(sizeof(int) * (cCon->usefulSpace+2));
  for(i = 0; i < cCon->usefulSpace; i++)
    {
      cCon->hashOffset[i] = -1;
      cCon->offsetIndices[i] = -1;
    }
  cCon->hashOffset[0] = 0;
  cCon->offsetIndices[0] = 0;
  for (k = 1;k <= cCon->usefulSpace;k++)
    {
      cCon->hashOffset[k] = combiCount(gPieceDist(k-1));
      cCon->offsetIndices[k] = gpi(k-1);
    }
  cCon->hashOffset[k] = -1;
  temp = 0;
  sofar = 0;
  //printf("\n");
  for (i = 1; i <= cCon->usefulSpace; i++)
    {
      //printf("block: %llu Tot:%llu\n",cCon->hashOffset[i],sofar);
      //fflush(NULL);
      sofar += cCon->hashOffset[i];
      if(sofar<temp){
	ExitStageRightErrorString("To many positions to represent in current position format");
      }
      cCon->hashOffset[i] = sofar;
      cCon->offsetIndices[i]++;
      temp = sofar;
    }	
  cCon->hashOffset[cCon->usefulSpace + 1] = -1;
  cCon->maxPos = sofar;
  generic_hash_context_switch(cCon->contextNumber);
  return sofar*2;
}


/* initializes pascal's triangle and piece configuration tables */
void nCr_init(int boardsize)
{
  int i, j, k, sum, ctr;
  int *temp = (int*) SafeMalloc (sizeof(int) * cCon->numPieces);
  hash_NCR = cCon->NCR = (POSITION*) SafeMalloc(sizeof(POSITION) * (boardsize + 1) * (boardsize + 1));
  hash_boardSize = cCon->boardSize = boardsize;
  
  /*store the left and right wings of pascal's triangle*/
  for(i = 0; i<= boardsize; i++){
    cCon->NCR[i*(boardsize+1)] = 1;
    cCon->NCR[i*(boardsize+1) + i] = 1;
  }
  
  /*compute and store the middle of pascal's triangle*/
  for(i = 1; i<= boardsize; i++){
    for(j = 1; j < i ; j++){
      cCon->NCR[i*(boardsize+1) + j] = cCon->NCR[(i-1)*(boardsize+1) + j-1] + cCon->NCR[(i-1)*(boardsize+1) + j];
    }
  }
  
  /* compute the gPieceDist[i][j]'s with a method similar to Horner's method of
     evaluating a polynomial p using only deg(p) multiplications
  */
  ctr = 0;
  for(i=0;i<cCon->numCfgs;i++){
    cCon->pieceIndices[i] = -1;
  }

  for (i = 0;i < cCon->numCfgs;i++){
    sum = 0;
    k = i;
    for (j = 0;j < cCon->numPieces;j++){
      temp[j] = cCon->mins[j] + (k % (cCon->nums[j]));
      sum += temp[j];
      k = k/(cCon->nums[j]);
    }
    if (sum == boardsize && validConfig(temp)){
      cCon->pieceIndices[ctr] = i;
      ctr++;
    }
  }
  cCon->usefulSpace = ctr;
  SafeFree(temp);
}


/* memoizes combinations not used right now */
void hash_combiCalc(){
  int i,sums;
  int *thPieces = (int *) SafeMalloc(sizeof(int) * cCon->numPieces);
  
  for(i=0;i<cCon->numPieces;i++)
    thPieces[i] = cCon->mins[i];
  sums = 1;
  for(i=0;i<cCon->numPieces;i++){
    if(cCon->nums[i] != 0)
      sums *= cCon->nums[i];
  }
  cCon->combiArray = (POSITION *) SafeMalloc(sizeof(POSITION) * sums);
  while(thPieces[cCon->numPieces-1] <= cCon->maxs[cCon->numPieces-1]){
    
    sums = 0;
    for(i=0;i<cCon->numPieces;i++){
      sums = combiHash(sums,cCon->nums[i],thPieces[i]);
    }
    cCon->combiArray[sums] = combiCount(thPieces);
    thPieces[0]++;
    i=0;
    while(i<(cCon->numPieces-1) && thPieces[i] > cCon->maxs[i]){
      thPieces[i]=cCon->mins[i];
      i++;
      thPieces[i]++;
    }
  }

}

/***************************************
**
**      Generic Hashing
**
****************************************/

/* hashes *board to an int */
POSITION generic_hash(char* board, int player) //accomodates whoseMove
{
  int temp, i, j, sum;
  int boardSize = hash_boardSize;
  
  for (i = 0;i < cCon->numPieces;i++){
    hash_thisCount[i] = 0;
    hash_localMins[i] = cCon->mins[i];
  }
  
  for (i = 0; i < boardSize; i++){
    for (j = 0; j < hash_numPieces; j++){
      if (board[i] == hash_pieces[j]){
	hash_thisCount[j]++;
      }
    }
  }
  sum = 0;
  for (i = hash_numPieces-1;i >= 0;i--){
    sum += (hash_thisCount[i] - cCon->mins[i]);
    if (i > 0){
      sum *= cCon->nums[i-1];
    }
  }
  temp = hash_hashOffset[searchIndices(sum)];
  temp += hash_cruncher(board);
  return temp + (player-1)*(hash_maxPos); //accomodates whoseMove
}

/* helper func from generic_unhash() computes lexicographic rank of *board
   among boards with the same configuration argument *thiscount */
POSITION hash_cruncher (char* board)
{
  POSITION sum = 0;
  int i = 0, k = 0, max1 = 0;
  int boardSize = hash_boardSize;
   
  for(;boardSize>1;boardSize--){
    i = 0;
    
    while (board[boardSize - 1] != hash_pieces[i]) i++;
    for (k = 0;k < i;k++)
      {
	max1 = 1;
	if (hash_localMins[k] > 1) max1 = hash_localMins[k];
	
	if (hash_thisCount[k] >= max1)
	  {
	    hash_thisCount[k]--;
	    sum += combiCount(hash_thisCount);
	    hash_thisCount[k]++;
	  }
      }
    hash_thisCount[i]--;
    hash_localMins[i]--;
  }
  
  return sum;
}

/* tells whose move it is, given a board's hash number */
int whoseMove (POSITION hashed)
{
  return hashed >= cCon->maxPos ? 2 : 1;
}


/*************************************
**
**      Generic UnHashing
**
*************************************/

/* unhashes hashed to a board */
char* generic_unhash(POSITION hashed, char* dest)
{
  POSITION offst;
  int i, j, boardSize;
  hashed %= hash_maxPos; //accomodates whoseMove
  
  boardSize = hash_boardSize;
  j = searchOffset(hashed);
  offst = hash_hashOffset[j];
  hashed -= offst;
  for (i = 0;i < cCon->numPieces;i++){
    hash_localMins[i] = cCon->mins[i];
    hash_thisCount[i] = gpd(cCon->offsetIndices[j + 1] - 1)[i];
  }
  hash_uncruncher(hashed, dest);
  return dest;
}

/* helper func from generic_hash() computes a board, given its lexicographic rank hashed
   among boards with the same configuration argument *thiscount*/
void hash_uncruncher (POSITION hashed, char *dest)
{
  int i = 0, j = 0;
  int max1 = 0;
  int boardSize = hash_boardSize;

  for(;boardSize>0;boardSize--){
    if (boardSize == 1){
      i = 0;
      while (hash_thisCount[i] == 0) i++;
      dest[0] = hash_pieces[i];
    }else{
      hash_miniOffset[0] = 0;
      hash_miniIndices[0] = 0;
      j = 1;
      for (i = 0; (i < hash_numPieces) && (hash_miniOffset[j-1] <= hashed);i++){
	max1 = 1;
	if (hash_localMins[i] > 1) max1 = hash_localMins[i];
	if (hash_thisCount[i] >= max1)
	  {
	    hash_thisCount[i]--;
	    hash_miniOffset[j] = hash_miniOffset[j-1] + combiCount(hash_thisCount);
	    hash_thisCount[i]++;
	    hash_miniIndices[j] = i;
	    j++;
	  }
      }
      i = j-1;
      hash_thisCount[hash_miniIndices[i]]--;
      hash_localMins[hash_miniIndices[i]]--;
      dest[boardSize-1] = hash_pieces[hash_miniIndices[i]];
      hashed = hashed - hash_miniOffset[i-1];
    }
    
  }
  
}



/************************************
*************************************
**
**  Hash Context Functions
**
*************************************
************************************/

/************************************
**
** generic_hash_context_init()
**
**  Generates a new uninitialized
**  hash context with a unique hash 
**  context identifier
**
************************************/
int generic_hash_context_init() {

  int i;
  struct hashContext **temp;
  struct hashContext *newHashC;
  hash_tot_context++;
  temp = (struct hashContext **) SafeMalloc (sizeof(struct hashContext*)*(hash_tot_context+1));
  if(contextList != NULL){
    for(i=0;i<hash_tot_context;i++) temp[i] = contextList[i];
    SafeFree(contextList);
    contextList = temp;
  }else{
    contextList = temp;
  }
  newHashC = (struct hashContext *) SafeMalloc (sizeof(struct hashContext));
  *contextList  = *(contextList+hash_tot_context) = newHashC;
  cCon->hashOffset = NULL;
  cCon->maxPos = 0;
  cCon->NCR = NULL;
  cCon->gpdStore = NULL;
  cCon->offsetIndices = NULL;
  cCon->pieceIndices = NULL;
  cCon->boardSize = 0;
  cCon->numCfgs = 0;
  cCon->usefulSpace = 0;
  cCon->numPieces = 0;
  cCon->pieces = NULL;
  cCon->nums = NULL;
  cCon->mins = NULL;
  cCon->maxs = NULL;
  cCon->thisCount = NULL;
  cCon->localMins = NULL;
  cCon->gfn = NULL;
  cCon->init = FALSE;
  cCon->contextNumber = hash_tot_context;
  return hash_tot_context;
}

/******************************
**
** generic_hash_context_switch(int context)
**
**  Switches the current hash context 
**  to the indicated hash context. Set's global
**  variables for speed improvements during solving
**
******************************/

void generic_hash_context_switch(int context){
  if(context > hash_tot_context)
    ExitStageRightErrorString("Attempting to switch to non-existant hash context");
  cCon = *(contextList + context);
  hash_hashOffset = cCon->hashOffset;
  hash_NCR = cCon->NCR;
  hash_miniOffset = cCon->miniOffset;
  hash_pieces = cCon->pieces;
  hash_thisCount = cCon->thisCount;
  hash_localMins = cCon->localMins;
  hash_miniOffset = cCon->miniOffset;
  hash_miniIndices = cCon->miniIndices;
  hash_numPieces = cCon->numPieces;
  hash_boardSize = cCon->boardSize;
  hash_usefulSpace = cCon->usefulSpace;
  hash_maxPos = cCon->maxPos;
}



/*******************************
**
**      Various Helper functions
**          Not all are used
**
********************************/

/* frees up memory for current context */
void freeHashContext(){
  if(NULL == cCon)
    ExitStageRightErrorString("Attempting to free an invalid hash context");

  SafeFree(cCon->hashOffset);
  SafeFree(cCon->NCR);
  SafeFree(cCon->gpdStore);
  SafeFree(cCon->offsetIndices);
  SafeFree(cCon->pieceIndices);
  SafeFree(cCon->pieces);
  SafeFree(cCon->nums);
  SafeFree(cCon->mins);
  SafeFree(cCon->maxs);
  SafeFree(cCon->thisCount);
  SafeFree(cCon->localMins);
  
}

/* determines if a piece distribution is valid or not.
   t[i] denotes the number of occurrences of the ith piece in this cofiguration
   */
int validConfig(int *t)
{
	int cond = (cCon->gfn == NULL)? 1 : cCon->gfn(t);
	return cond;
}


/* function used to map a member of gHashOffset into gOffsetIndices */
int searchIndices(int s)
{
	int i = cCon->usefulSpace;
	while(cCon->offsetIndices[i] > s) i--;
	return i;
}

/* function used to map a member of gOffsetIndices into gHashOffset */
int searchOffset(int h)
{
  int i = hash_usefulSpace;
  while(hash_hashOffset[i] > h) i--;
  return i;
}

/* helper function used to find n choose (t1,t2,t3,...,tn) where the ti's are
   the members of *tc */
POSITION combiCount(int* tc)
{
  POSITION sum = 0, prod = 1, ind = 0,old=1,hold=0;
  for (ind = 0;ind < cCon->numPieces - 1;ind++){
    sum += tc[ind];
    hold = nCr(sum+tc[ind+1], sum);
    prod *= hold;
    if(prod/hold !=  old)
      ExitStageRightErrorString("Combination Calculation Wraps");
    old = prod;
  }
  return prod;
}




/* parses the char* input to generic_hash_init() and assigns corresponding values to
   the pieces, minima, and maxima arrays*/
int getPieceParams (int *pa, char *pi, int *mi, int *ma)
{
  int i = 0;
  while (pa[i*3] != -1){
    pi[i] = (char)pa[i*3];
    mi[i] = pa[i*3 + 1];
    ma[i] = pa[i*3 + 2];
    i++;
  }
  return i;
}
int hash_countPieces(int *pA){
  int i = 0;
  
  while(pA[i*3] != -1) i++;
  
  return i;
}

/* finds the piece distribution whose index is n */
int* gpd (int n)
{
  int k = n, j;
  for (j = 0;j < cCon->numPieces;j++){
    cCon->gpdStore[j] = cCon->mins[j] + (k % (cCon->nums[j]));
    k = k/(cCon->nums[j]);
  }
  return cCon->gpdStore;
}

/* shorthand for gpd() */
int *gPieceDist(int i)
{
  return gpd(gpi(i));
}

/* shorthand for gPieceIndices */
int gpi (int n)
{
  return (cCon->pieceIndices)[n];
}

/* shorthand for n choose r */
int nCr(int n, int r)
{
  return hash_NCR[n*(hash_boardSize+1) + r];
}