/************************************************************************
**
** NAME:        mabalone.c
**
** DESCRIPTION: Abalone
**
** AUTHOR:      Michael Mottmann & Melinda Franco
**
** DATE:        WHEN YOU START/FINISH
**
** UPDATE HIST: RECORD CHANGES YOU HAVE MADE SO THAT TEAMMATES KNOW
**
** 
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "hash.h"
#include <string.h>
#include <math.h>

extern STRING gValueString[];
POSITION gNumberOfPositions  = 924;

POSITION gInitialPosition    = 9;
POSITION gMinimalPosition    = 0;
POSITION kBadPosition        = -1;

STRING   kGameName           = "abalone";
STRING   kDBName             = "Abalone";
BOOLEAN  kPartizan           = TRUE; 
BOOLEAN  kSupportsHeuristic  = FALSE;
BOOLEAN  kSupportsSymmetries = FALSE;
BOOLEAN  kSupportsGraphics   = FALSE;
BOOLEAN  kDebugMenu          = TRUE;
BOOLEAN  kGameSpecificMenu   = TRUE;
BOOLEAN  kTieIsPossible      = FALSE;
BOOLEAN  kLoopy               = TRUE;
BOOLEAN  kDebugDetermineValue = FALSE;

STRING kHelpGraphicInterface =
"Not written yet";

STRING   kHelpTextInterface    =
""; 

STRING   kHelpOnYourTurn =
"";

STRING   kHelpStandardObjective =
"";

STRING   kHelpReverseObjective =
"";

STRING   kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */
"";

STRING   kHelpExample =
"";

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

/*************************************************************************
**
** Below is where you put your #define's and your global variables, structs
**
*************************************************************************/

STRING gGameSpecificMenu = "1.\t Change the value of N, the edge size of the hexagon board\n2.\t Return to the previous menu\n\nSelect Option:  ";

#define NULLSLOT 99
int N = 2;

int BOARDSIZE;
char *gBoard;

struct row {
  int size;
  int start_slot;
};

struct row **rows;

/*************************************************************************
**
** Above is where you put your #define's and your global variables, structs
**
*************************************************************************/

/*
** Function Prototypes:
*/

//Function prototypes here.

// External
extern GENERIC_PTR	SafeMalloc ();
extern void		SafeFree ();

// Internal
int destination (int, int);
int move_hash (int, int, int);
struct row * makerow (int, int);
int m_generic_hash(char *);
void m_generic_unhash(int, char *);
void printrow (int, int);
void printlines (int, int);
void changeBoard ();
/*************************************************************************
**
** Here we declare the global database variables
**
**************************************************************************/

extern VALUE     *gDatabase;

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Initialize the gDatabase, a global variable. and the other
**              local variables.
** 
************************************************************************/
int max;
int init;

void InitializeGame()
{
  /* printf("start initialize game\n");*/
  int count;

  rows = (struct row **) SafeMalloc ((2*N - 1) * sizeof(struct row *));
  
  int rowsize = N, rownum, slot = 0, bsize = 0;
  BOARDSIZE = 0;
  for (rownum = 0; rownum < N - 1; rownum++) {
    rows[rownum] = makerow(rowsize,slot);
    slot += rowsize;
    rowsize++;
  }
  for (rownum = N - 1; rownum <= 2*N - 2; rownum++) {
    rows[rownum] = makerow(rowsize,slot);
    slot += rowsize;
    rowsize--;
  }
  
  printf("BOARDSIZE is %d\n", BOARDSIZE);

  gBoard = (char *) SafeMalloc (BOARDSIZE * sizeof(char));

  if (N == 2) {
    gBoard[0]='x';
    gBoard[1]='x';
    gBoard[2]='*';
    gBoard[3]='*';
    gBoard[4]='*';
    gBoard[5]='o';
    gBoard[6]='o';
  } else if (N == 3) {
    gBoard[0]='x';
    gBoard[1]='*';
    gBoard[2]='x';
    gBoard[3]='x';
    gBoard[4]='x';
    gBoard[5]='x';
    gBoard[6]='x';
    gBoard[7]='*';
    gBoard[8]='*';
    gBoard[9]='*';
    gBoard[10]='*';
    gBoard[11]='*';
    gBoard[12]='o';
    gBoard[13]='o';
    gBoard[14]='o';
    gBoard[15]='o';
    gBoard[16]='o';
    gBoard[17]='*';
    gBoard[18]='o';
  } else {
    for (count = 0; count < BOARDSIZE; count++) {
      if (count < 9)
	gBoard[count] = 'x';
      else if (count >= BOARDSIZE - 9)
	gBoard[count] = 'o';
      else gBoard[count] = '*';
    }
  }

  int init_array[10];
  init_array[0] = 'o';
  init_array[3] = 'x';
  init_array[6] = '*';
  init_array[9] = -1;

  if (N == 2) {
    init_array[1] = 1;
    init_array[2] = 2;
    init_array[4] = 1;
    init_array[5] = 2;
    init_array[7] = 3;
    init_array[8] = 5;
  } else if (N == 3) {
    init_array[1] = 5;
    init_array[2] = 6;
    init_array[4] = 5;
    init_array[5] = 6;
    init_array[7] = 7;
    init_array[8] = 9;
  } else {
    init_array[1] = 8;
    init_array[2] = 9;
    init_array[4] = 8;
    init_array[5] = 9;
    init_array[7] = BOARDSIZE - 18;
    init_array[8] = BOARDSIZE - 16;
  }


  max = generic_hash_init(BOARDSIZE,init_array,NULL);
  printf("%d  # of hash positions!\n",max);
  
  init = generic_hash(gBoard, 1);
  printf("%d  is the initial position\n",init);
  
  generic_unhash(init,gBoard);
  /*printf("testing...\n");
    for (count = 0; count < BOARDSIZE; count++) {
    printf("%c\n", gBoard[count]);
  }*/
  
  /*gDatabase = (VALUE *) SafeMalloc(gNumberOfPositions * sizeof(VALUE));*/
  
  gNumberOfPositions  = max;
  gInitialPosition    = init;
  gMinimalPosition    = init;
 
  /*printf("end initializegame\n");*/
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
** 
************************************************************************/

void DebugMenu()
{
}

/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
** 
************************************************************************/

void GameSpecificMenu() 
{
  int selection;
  printf("%s", gGameSpecificMenu);
  (void) scanf("%d", &selection);
  if (selection == 1)
    changeBoard();
  else if (selection == 2)
    return;
  else {
    printf("\n\n\n Please select a valid option...\n");
    GameSpecificMenu();
  }
}

void changeBoard()
{
  int size;
  printf("Enter the new N:  ");
  (void) scanf("%u", &size);
  if (size < 2) {
    printf("N must be at least 2\n");
    changeBoard();
  }
  else {
    printf("Changing N to %d ...\n", size);
    N = size;
    SafeFree(rows);
    SafeFree(gBoard);
    InitializeGame();
  }
}

  
/************************************************************************
**
** NAME:        SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
**              Ignore if you don't care about Tcl for now.
** 
************************************************************************/

void SetTclCGameSpecificOptions(theOptions)
     int theOptions[];
{
	
}

/************************************************************************
**
** NAME:        DoMove
**
** DESCRIPTION: Apply the move to the position.
** 
** INPUTS:      POSITION thePosition : The old position
**              MOVE     theMove     : The move to apply.
**
** OUTPUTS:     (POSITION) : The position that results after the move.
**
** CALLS:       Hash ()
**              Unhash ()
**	        destination ()
*************************************************************************/
POSITION DoMove(thePosition, theMove)
     POSITION thePosition;
     MOVE theMove;
{
  /*printf("Starting Do Move with input: %d\n", theMove);*/
  int destination(int,int);
  int whoseTurn;
  generic_unhash(thePosition, gBoard);
  int direction;
  int slot1;
  int slot2;
  int dest1;
  int dest2;
  int pushee_dest;
  BOOLEAN twopieces = TRUE;

  direction = theMove % 10;
  theMove = theMove/10;

  if (theMove < 0) {
    theMove = 0 - theMove;
  }

  slot1 = theMove % 100;
  slot2 = theMove/100;
  
  if (slot2 == NULLSLOT) {
    twopieces = FALSE;
  }
  
  if (slot1 == NULLSLOT) {
    twopieces = FALSE;
    slot1 = slot2;
  }
  

  dest1 = destination(slot1, direction);
  if (twopieces) dest2 = destination(slot2, direction);

  /*printf("dest1 = %d, direction = %d, gBoard[dest1] = %c\n", dest1, direction, gBoard[dest1]);*/

  if (gBoard[dest1] != '*') {
    pushee_dest = destination(dest1,direction);
    if (pushee_dest != NULLSLOT) 
      gBoard[pushee_dest] = gBoard[dest1];
  }

 
  gBoard[dest1] = gBoard[slot1];
  gBoard[slot1] = '*';

  if (twopieces) {
    gBoard[dest2] = gBoard[slot2];
    gBoard[slot2] = '*';
  }

  if (whoseMove(thePosition) == 1) {
    whoseTurn = 2;
  }
  else {
    whoseTurn = 1;
  }

  /*printf("finished do move\n");*/
  return (generic_hash(gBoard,whoseTurn));
}

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
** 
** OUTPUTS:     POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition()
{
  
}


/************************************************************************
**
** NAME:        PrintComputersMove*
** DESCRIPTION: Nicely format the computers move.
** 
** INPUTS:      MOVE    computersMove : The computer's move. 
**              STRING  computersName : The computer's name. 
**
************************************************************************/

void PrintComputersMove(computersMove, computersName)
     MOVE computersMove;
     STRING computersName;
{
  printf("%8s's move              : ", computersName);
  PrintMove(computersMove);
  printf("\n");
}


/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with Gobblet. Three in a row for the player
**              whose turn it is a win, otherwise its a loss.
**              Otherwise undecided.
** 
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS:       LIST FUNCTION CALLS
**              
**
************************************************************************/

VALUE Primitive ( POSITION h )
{
  /*printf("prim\n");*/
  BOOLEAN game_over(char[]);

  generic_unhash(h,gBoard);
  
  if (game_over(gBoard)) {
    /*printf("endprim\n");*/
    return (lose);
  }
  /*printf("endprim\n");*/
  return (undecided);
}

BOOLEAN game_over(char theBoard[]){
  int count, x = 0, o = 0, lose;
  
  if (N == 2)
    lose = 2;
  else if (N == 3)
    lose = 5;
  else 
    lose = 8;

  for (count = 0; count < BOARDSIZE; count++) {
    if (theBoard[count] == 'x') {
      x++;
    }
    else if (theBoard[count] == 'o') {
      o++;
    }
    /*printf("position %d is a %c\n", count, theBoard[count]);*/
  }
  /*printf("I counted %d os and %d xs\n", o, x);*/
  return (o<lose || x<lose);
}


/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
**              prediction of the game's outcome.
** 
** INPUTS:      POSITION position   : The position to pretty print.
**              STRING   playerName : The name of the player.
**              BOOLEAN  usersTurn  : TRUE <==> it's a user's turn.
**
** CALLS:       Unhash()
**              GetPrediction()
**              LIST OTHER CALLS HERE
**
************************************************************************/

void PrintPosition(position, playerName, usersTurn)
     POSITION position;
     STRING playerName;
     BOOLEAN usersTurn;
{
  generic_unhash(position, gBoard);
  int r, spacing;

  /* messy centering spacing issues for sizeable first line*/
  printf("\n");
  spacing = (4 * (*rows[N - 1]).size) + 2;
  for (r = 0; r < spacing/2; r++)
    printf (" ");
  printf("Board");
  for (r = 0; r < spacing/2; r++)
    printf (" ");
  if (spacing % 2 == 1)
    printf (" ");

  if (N == 2)
    spacing = (4 * (*rows[N - 1]).size) + 1;
  else
    spacing = (5 * (*rows[N - 1]).size) + 1;
  
  for (r = 0; r < spacing/2; r++)
    printf (" "); 
  printf("Legend");
  for (r = 0; r < spacing/2; r++)
    printf (" ");
  if (spacing % 2 == 1)
    printf (" ");
      
  printf("   Direction Key\n");
  

  printf("/-------------------------------");
  for (r = 0; r < (*rows[N - 1]).size; r++) {
    if (N == 2)
      printf("--------");
    else
      printf("---------");
  }
  printf("\\\n");
    
  /*main board printing here*/
  for (r = (2 * N) - 2; r >= 0; r--) {
    printf("|   ");
    printrow(r, 0);
    printf("   |   ");
    printrow(r, 1);
    printf("   |");

    if (r == N)
	printf("      3   2      |  To move one piece:");
    else if (r == N - 1)
      printf("    -1 -*- 1     |  To move two pieces:");
    else if (r == N - 2)
      printf("     -2  -3      |     (order of pieces doesn't matter)");
    else
      printf("                 |");
    if (r != 0) {
      printf("\n|   ");
      printlines(r, 0);
      printf("   |   ");
      printlines(r, 1);
      printf("   |");

      if (r == N - 1)
	printf("       / \\       |      direction, piece1, piece2");
      else if (r == N)
	printf("       \\ /       |      direction, piece1");
      else
	printf("                 |");
    }
    printf("\n");
  }


printf("\\-------------------------------");
  for (r = 0; r < (*rows[N - 1]).size; r++) {
    if (N == 2)
      printf("--------");
    else
      printf("---------");
  }
  printf("/\n");
}

/************************************************************************
**
** NAME:        GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
**              from this position. Return a pointer to the head of the
**              linked list.
** 
** INPUTS:      POSITION position : The position to branch off of.
**
** OUTPUTS:     (MOVELIST *), a pointer that points to the first item  
**              in the linked list of moves that can be generated.
**
** CALLS:       GENERIC_PTR SafeMalloc(int)
**              Primitive
**
************************************************************************/

MOVELIST *GenerateMoves(position)
         POSITION position;
{
  /*printf("generate\n");*/
  MOVELIST *head = NULL;
  MOVELIST *CreateMovelistNode(); /* In gamesman.c */
  VALUE Primitive();
  int slot, slot2, dest0, dest1, dest2, direction, ssdir;
  char whoseTurn;

  if (whoseMove(position) == 1) {
    whoseTurn = 'x';
  }
  else {
    whoseTurn = 'o';
  }

  generic_unhash(position,gBoard);
  
  if (Primitive(position) == undecided) {
    for (slot = 0; slot < BOARDSIZE ; slot++) {
      if (gBoard[slot] == whoseTurn) {
	for (direction = 1; direction <= 3; direction++) {
	  
	  slot2 = destination(slot, direction);

	  /*Single Piece Moves*/
	  dest0 = destination(slot, (0 - direction));
	  if (gBoard[slot2] == '*') {
	    head = CreateMovelistNode(move_hash(slot,NULLSLOT,direction), head);
	  }
	  if (gBoard[dest0] == '*') {
	    head = CreateMovelistNode(move_hash(slot,NULLSLOT,(0 - direction)), head);
	  }
	  
	  /*Double Piece Moves*/
	  if (gBoard[slot2] == whoseTurn) {
	    /*Test for pushes in both directions*/
	    /*the oppositve direction is represented by the negative of that direction*/
	      
	      dest1 = destination(slot2,direction);
	      
	      if (dest1 != NULLSLOT) {
		dest2 = destination(dest1,direction);
		if ((gBoard[dest1] != whoseTurn) && ((gBoard[dest2]== '*') ||(dest2 == NULLSLOT)||(gBoard[dest1] == '*'))) {
		  head = CreateMovelistNode(move_hash(slot, slot2, direction), head);
		}
	      }
	      direction = 0 - direction; 
	      dest1 = destination(slot,direction);
	      
	      if (dest1 != NULLSLOT) {
		dest2 = destination(dest1,direction);
		if ((gBoard[dest1] != whoseTurn) && ((gBoard[dest2]== '*') || (dest2 == NULLSLOT)||(gBoard[dest1] == '*'))) {
		  head = CreateMovelistNode(move_hash(slot, slot2, direction), head);
		}
	      }
	      direction = 0 - direction;


	    
	    /*Test for possible side steps*/
	    for (ssdir = -3; ssdir <= 3; ssdir++) {
	      if ((ssdir != 0) && (ssdir != direction) && (ssdir != 0 - direction)) {/*skip over nonexistant zero direction as well as push direction*/
		dest1 = destination(slot,ssdir);
		dest2 = destination(slot2,ssdir);

		if ((dest1 != NULLSLOT) && (dest2 != NULLSLOT) && (gBoard[dest1] == '*') && (gBoard[dest2] == '*')) {
		  head = CreateMovelistNode(move_hash(slot,slot2,ssdir), head);
		}


		if (slot == 0) {
		  /*printf("slot is 0, slot2 is %d, dest1 is %d, gBoard[dest1] is %c, dest2 is %d, gBoard[dest2] is %c, in direction %d\n", slot2, dest1, gBoard[dest1], dest2, gBoard[dest2], ssdir);*/
	  
		}

	      }
	    }
	  }
	}   
      }
    }
    /*printf("end gen\n");*/
    return(head);
  }
  /*printf("end gen\n");*/
  return(NULL);
 }

 
/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
**              If so, return Undo or Abort and don't change theMove.
**              Otherwise get the new theMove and fill the pointer up.
** 
** INPUTS:      POSITION *thePosition : The position the user is at. 
**              MOVE *theMove         : The move to fill with user's move. 
**              STRING playerName     : The name of the player whose turn it is
**
** OUTPUTS:     USERINPUT             : Oneof( Undo, Abort, Continue )
**
** CALLS:       ValidMove(MOVE, POSITION)
**              BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(thePosition, theMove, playerName)
     POSITION thePosition;
     MOVE *theMove;
     STRING playerName;
  
{
  USERINPUT ret;
  do {
    printf("for a list of valid moves, press ?\n\n");
    printf("%8s's move :  ", playerName);
  
    ret = HandleDefaultTextInput(thePosition, theMove, playerName);
    if(ret != Continue) 
      return(ret);
   
  }
  while (TRUE);
  return(Continue); /* this is never reached, but lint is now happy */
}



/************************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
**              For example, if the user is allowed to select one slot
**              from the numbers 1-9, and the user chooses 0, it's not
**              valid, but anything from 1-9 IS, regardless if the slot
**              is filled or not. Whether the slot is filled is left up
**              to another routine.
** 
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     BOOLEAN : TRUE if the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(input)
     STRING input;
{
  return TRUE;
}

/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**              No checking if the input is valid is needed as it has
**              already been checked!
** 
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(input)
     STRING input;
{
  /*printf("Starting conversion\n");*/
  int n=0, dir, p1, p2;

  /*skip whitespace*/
  while ((input[n] == ' ') || (input[n] == '(')) {
    n++;
  }
  
  /*get direction*/
  if (input[n] == '*') {
    dir = 0 - (input[n+1] - '0');
    n = n + 3;
  }
  else {
    dir = input[n] - '0';
    n = n + 2;
  }

  /*skip whitespace*/
  while (input[n] == ' ') {
    n++;
  }

  /*get first piece*/
  p1 = input[n] - '0';
  n++;
  if ((input[n] >= '0') && (input[n] <= '9')) {
    p1 = p1 * 10 + (input[n] - '0');
    n++;
  }

  /*skip whitespace and commas*/
  while ((input[n] == ' ') || (input[n] == ',') || (input[n] == ')')) {
    n++;
  }
  
  if (input[n] == 0) {
    p2 = NULLSLOT;
  }
  else {
    /*get 2nd piece*/
    p2 = input[n] - '0';
    n++;
    if ((input[n] >= '0') && (input[n] <= '9')) {
      p2 = p2 * 10 + (input[n] - '0');
    }
  }
  
  /*printf("ending conversion\n");*/
  return move_hash(p1, p2, dir);
}

/************************************************************************
**
** NAME:        PrintMove
**
** DESCRIPTION: Print the move in a nice format.
** 
** INPUTS:      MOVE *theMove         : The move to print. 
**
************************************************************************/

void PrintMove(theMove)
     MOVE theMove;
{
  int direction, slot1, slot2;

  direction = theMove % 10;
  theMove = theMove/10;

  if (theMove < 0) {
    theMove = 0 - theMove;
  }

  slot1 = theMove % 100;
  slot2 = theMove/100;

  if (slot1 == NULLSLOT) {
    printf("(%d,%d)",direction,slot2);
  }
  else if (slot2 == NULLSLOT) {
    printf("(%d,%d)",direction,slot1);
  }
  else {
    printf("(%d,%d,%d)",direction,slot1,slot2);
  }
}

/************************************************************************
**
** NAME:        NumberOfOptions
**
** DESCRIPTION: Calculates and returns the number of option combinations
**				there are with all the game variations you program.
**
** OUTPUTS:     int : the number of option combination there are.
**
************************************************************************/

int NumberOfOptions()
{
  return 1;
}

/************************************************************************
**
** NAME:        getOption
**
** DESCRIPTION: A hash function to keep track of all the game variants.
**				Should return a different number for each set of
**				variants.
**
** OUTPUTS:     int : the number representation of the options.
**
************************************************************************/

int getOption()
{
  return 0;
}

/************************************************************************
**
** NAME:        setOption
**
** DESCRIPTION: The correspondithang unhash for the game variants.
**				Should take the input and set all the appropriate
**				variants.
**
** INPUT:     int : the number representation of the options.
**
************************************************************************/
void setOption(int option)
{

}

/************************************************************************
**
** NAME:        GameSpecificTclInit
**
** DESCRIPTION: NO IDEA, BUT AS FAR AS I CAN TELL IS IN EVERY GAME
**
************************************************************************/
int GameSpecificTclInit(Tcl_Interp* interp,Tk_Window mainWindow) 
{
  return 0;
}

/************************************************************************
*************************************************************************
**         EVERYTHING BELOW THESE LINES IS LOCAL TO THIS FILE
*************************************************************************
************************************************************************/


/************************************************************************
** This is where you can put any helper functions, including your
** hash and unhash functions if you are not using one of the existing
** ones.
************************************************************************/
int destination(int slot, int direction) {
  /*find the row*/
  int r, start, size;
  for (r = 0; r <= 2*N - 2; r++) {
    if (slot < (*rows[r]).start_slot + (*rows[r]).size) {
      break;
    }
  }
  start = (*rows[r]).start_slot;
  size = (*rows[r]).size;

  /*test for nullslot*/

  if ((direction == 1 && slot == start + size - 1) ||
      (direction == 2 && ((r == 2*N -2) || ((slot == start + size -1) && (*rows[r+1]).size < size))) ||
      (direction == 3 && ((r == 2*N -2) || ((slot == start) && (*rows[r+1]).size < size))) ||
      (direction == -1 && slot == start) ||
      (direction == -2 && ((r == 0) || ((slot == start) && (*rows[r-1]).size < size))) ||
      (direction == -3 && ((r == 0) || ((slot == start + size -1) && (*rows[r-1]).size < size)))) {
    return NULLSLOT;
  }

  if (direction == 1 || direction == -1) {
    return (slot + direction);
  }

  if (direction == 2) {
    if (r < N - 1) {
      return ((slot - start + 1) + (*rows[r+1]).start_slot);
    }
    return ((slot - start) + (*rows[r+1]).start_slot);
  }

  if (direction == 3) {
    if (r < N - 1) {
      return ((slot - start) + (*rows[r+1]).start_slot);
    }
    return ((slot - start - 1) + (*rows[r+1]).start_slot);
  }

  if (direction == -2) {
    if (r > N - 1) {
      return ((slot - start) + (*rows[r-1]).start_slot);
    }
    return ((slot - start - 1) + (*rows[r-1]).start_slot);
  }

  if (direction == -3) {
    if (r > N - 1) {
      return ((slot - start + 1) + (*rows[r-1]).start_slot);
    }
    return ((slot - start) + (*rows[r-1]).start_slot);
  }
}


BOOLEAN member(int slot, int places[]) {
  int length = places[0];
  int i = 1;


  while (i<=length) {
    if (places[i] == slot) {
      return TRUE;
    }
  }
  return FALSE;
}
  
int move_hash(int slot1, int slot2, int direction) {
  int bigger, smaller;
  if (slot2 > slot1) {
    bigger = slot2;
    smaller = slot1;
  }
  else {
    bigger = slot1;
    smaller = slot2;
  }

  if (direction > 0) {
    return (direction + (10 * bigger) + (1000 * smaller));
  }
  else {
    return (-1 * ((-1 * direction) + (10 * smaller) + (1000 * bigger)));
  }
}

struct row * makerow (int size, int start) {
  int i;

  struct row *new;
  new = (struct row *) SafeMalloc(1 * sizeof(struct row));
  (*new).size = size;
  (*new).start_slot = start;
  BOARDSIZE += size;
  return new;
}
 
int m_generic_hash(char* board) {
  int size = 8;
  int val;
  int i, hashval= 0;
  for (i=0; i<size;i++) {
    if (board[i] == '*') 
      val = 0;
    if (board[i] == 'x')
      val = 1;
    if (board[i] == 'o')
      val = 2;
    hashval = hashval + val*pow(10,i);
  }
  return hashval;
}

void m_generic_unhash(int val, char* board) {
  int i;
  int size = 8;
  int temp;
  for (i = 0; i<size;i++) {
    temp = val %10;
    if (temp == 0) board[i] = '*';
    if (temp == 1) board[i] = 'x';
    if (temp == 2) board[i] = 'o';
    val = val/10;
  }
}

void printrow (int line, int flag) {
  int s, size, start;

  size = (*rows[line]).size;
  start = (*rows[line]).start_slot;

  for (s = 0; s < abs((N - 1) - line); s++) {
    printf ("  ");
  }

  for (s = 0; s < size; s++) {
    printf ("(");
    
    if (flag == 0)
      if (gBoard[start + s] == '*')
	printf(" ");
      else
	printf ("%c", gBoard[start + s]);
    if (flag == 1) {
      if (N == 2) {
	printf("%d", start + s);
      }
      else {
	if (start +s < 10)
	  printf("0");
	printf("%d",start + s);
      }
    }
    
    if (s != size - 1)
      printf(")-");
    else
      printf(") ");
  }

  for (s = 0; s < abs((N - 1) - line); s++) {
    if (flag == 0)
      printf ("  ");
    if (flag == 1)
      if (N == 2)
	printf("  ");
      else
	printf ("   ");
  }
}
    
void printlines (int line, int flag) {
  int s;
 
  for (s = 0; s < abs((N - 1) - line); s++) {
    printf ("  ");
  }
  
 
  if (line > N - 1) {
    s = 0;
  }
  else {
    s = 1;
    printf("  ");
  }
  for (; s < (*rows[line]).size; s++) {
    if (line > N - 1) {
      if (flag == 0)
	printf("/ \\ ");
      else
	if (N == 2)
	  printf("/ \\ ");
	else
	  printf("/  \\ ");
    }
    else {
      if (flag == 0)
	printf("\\ / ");
      else
	if (N == 2)
	  printf("\\ / ");
	else
	  printf("\\  / ");
    }
  }

  if (line > N - 1)
    s = 0;
  else
    s = -1;

  for (; s < abs((N - 1) - line); s++) {
    if (flag == 0)
      printf ("  ");
    if (flag == 1)
      if (N == 2)
	printf ("  ");
    else
      printf ("   ");
  }

}

