/************************************************************************
**
** NAME:        mfox.c
**
** DESCRIPTION: Forest Fox
**
** AUTHOR:      Jiachun Li, Yifan Zhou
**
** DATE:        2023-04-22
**
************************************************************************/

#include <stdio.h>
#include "gamesman.h"

/* IMPORTANT GLOBAL VARIABLES */
STRING kAuthorName = "Jiachun Li, Yifan Zhou";
STRING kGameName = "Fox"; //  use this spacing and case
STRING kDBName = "fox"; // use this spacing and case
POSITION gNumberOfPositions = 0; // TODO: Put your number of positions upper bound here.
POSITION gInitialPosition = 0; // TODO: Put the hash value of the initial position.
BOOLEAN kPartizan = TRUE; // TODO: Is the game PARTIZAN i.e. given a board does each player have a different set of moves available to them?
BOOLEAN kTieIsPossible = TRUE; // TODO: Is a tie or draw possible?
BOOLEAN kLoopy = TRUE; // TODO: Is this game loopy?
BOOLEAN kSupportsSymmetries = TRUE; // TODO: Whether symmetries are supported (i.e. whether the GetCanonicalPosition is implemented)
int BOARDSIZE = 31;
char init_board[]={'D','D','D','D','D',
                   'D','D','D','D','D',                 
	                 'D','D','J','D','D',                 
	                 '-','-','-','-','-',
	                 '-','-','-','-','-',
	                     '-','-','- ',
                   '-',    '-',    '-'};

/* Likely you do not have to change these. */
POSITION GetCanonicalPosition(POSITION);
STRING MoveToString(MOVE);
POSITION kBadPosition = -1;
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;

/* You do not have to change these for now. */
BOOLEAN kGameSpecificMenu = FALSE;
BOOLEAN kDebugMenu = FALSE;

/* These variables are not needed for solving but if you have time 
after you're done solving the game you should initialize them 
with something helpful. */
STRING kHelpGraphicInterface = "";
STRING kHelpTextInterface = "";
STRING kHelpOnYourTurn = "";
STRING kHelpStandardObjective = "";
STRING kHelpReverseObjective = "";
STRING kHelpTieOccursWhen = /* Should follow 'A Tie occurs when... */ "";
STRING kHelpExample = "";

/* You don't have to change this. */
void DebugMenu() {}
/* Ignore this function. */
void SetTclCGameSpecificOptions(int theOptions[]) {}
/* Do not worry about this yet because you will only be supporting 1 variant for now. */
void GameSpecificMenu() {}






/*********** BEGIN SOLVING FUNCIONS ***********/

/* TODO: Add a hashing function and unhashing function, if needed. */

/* Initialize any global variables or data structures needed before
solving or playing the game. */
void InitializeGame() {
  int piece_array[] = {'J', 1, 1,
		                   'D', 9, 14, 
		                   '-', 16, 21,
		                   -1};
 	gNumberOfPositions = generic_hash_init(BOARDSIZE, piece_array, NULL, 0);

	int init;
	int boardStats[3];
	boardPieceStats(start_standard_board,boardStats);
	hash_data[5] = numFoxes(boardStats);
	hash_data[8] = numGeese(boardStats);
	hash_data[7] = GEESE_MIN - 1;



	mergePositionSetRequiredBits(numFoxes(boardStats));

	init = mergePositionGoAgain(generic_hash_hash(start_standard_board,GEESE_PLAYER), 0);

	gInitialPosition = init;
	gNumberOfPositions = max * intpow(2,NUM_GOAGAIN_BITS);





  gCanonicalPosition = GetCanonicalPosition;

	gBoard = (char *) SafeMalloc (BOARDSIZE * sizeof(char));

	


  for (int i = 0; i < BOARDSIZE; i++)
			gBoard[i] = '-';

	gInitialPosition = generic_hash_hash(gBoard, 1);

	gMinimalPosition = gInitialPosition;


	gMoveToStringFunPtr = &MoveToString;
  
}

/* Return the hash value of the initial position. */
POSITION GetInitialPosition() {
  /* YOUR CODE HERE */
  return 0;
}

/* Return a linked list of moves. */
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;
  /* YOUR CODE HERE 
     
     To add to the linked list, do 
     moves = CreateMovelistNode(<the move you're adding>, moves);
     See the function CreateMovelistNode in src/core/misc.c
  */
  return moves;
}

/* Return the position that results from making the 
input move on the input position. */
POSITION DoMove(POSITION position, MOVE move) {
  /* YOUR CODE HERE */
  return 0;
}

/*****************************************************************
**  Determine whether the position is a primitive win,
**  primitive tie, primitive lose, or not primitive.
**  OUTPUTS: (VALUE) an enum; one of: (win, lose, tie, undecided)
**  See src/core/types.h for the value enum definition.
******************************************************************/
VALUE Primitive(POSITION position) {
  /* YOUR CODE HERE */
  return undecided;
}

/* Symmetry Handling: Return the canonical position. */
POSITION GetCanonicalPosition(POSITION position) {
  /* YOUR CODE HERE */
  return position;
}

/*********** END SOLVING FUNCTIONS ***********/







/*********** BEGIN TEXTUI FUNCTIONS ***********/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  /* THIS ONE IS MOST IMPORTANT FOR YOUR DEBUGGING */
  /* YOUR CODE HERE */
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  /* YOUR CODE HERE */
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
  /* YOUR CODE HERE */
  return Continue;
}

/* Return whether the input text signifies a valid move. */
BOOLEAN ValidTextInput(STRING input) {
  /* YOUR CODE HERE */
  return TRUE;
}

/* Assume the text input signifies a valid move. Return
the move hash corresponding to the move. */
MOVE ConvertTextInputToMove(STRING input) {
  /* YOUR CODE HERE */
  return 0;
}

/* Return the string representation of the move. 
Ideally this matches with what the user is supposed to
type when they specify moves. */
STRING MoveToString(MOVE move) {
  /* YOUR CODE HERE */
  return NULL;
}

/* Basically just print the move. */
void PrintMove(MOVE move) {
  /* YOUR CODE HERE */
}

/*********** END TEXTUI FUNCTIONS ***********/









/*********** BEGIN VARIANT FUNCTIONS ***********/

/* How many variants are you supporting? */
int NumberOfOptions() {
  /* YOUR CODE HERE */
  return 1;
}

/* Return the current variant id. */
int getOption() {
  /* YOUR CODE HERE */
  return 0;
}

/* The input is a variant id. This function sets any global variables
or data structures according to the variant specified by the variant id. */
void setOption(int option) {
  /* YOUR CODE HERE  */
}

/*********** END VARIANT-RELATED FUNCTIONS ***********/







/* Don't worry about these Interact functions below yet.
They are used for the AutoGUI which eventually we would
want to implement, but they are not needed for solving. */
POSITION InteractStringToPosition(STRING board) {
  /* YOUR CODE HERE */
  return 0;
}

STRING InteractPositionToString(POSITION position) {
  /* YOUR CODE HERE */
  return NULL;
}

/* Optional. */
STRING InteractPositionToEndData(POSITION position) {
  return NULL;
}

STRING InteractMoveToString(POSITION position, MOVE move) {
  /* YOUR CODE HERE */
  return MoveToString(move);
}