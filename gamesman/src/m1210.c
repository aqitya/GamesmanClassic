/************************************************************************
**
** NAME:        m1210.c
**
** DESCRIPTION: The 1,2,...10 game
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 1995. All rights reserved.
**
** DATE:        08/25/91
**
** UPDATE HIST:  
**
**  9-04-91 1.0a1 : Added <crs> to PrintPosition and added call
**                  to ValidMove() and PrintPossibleMoves()
**  9-06-91 1.0a2 : Added the two extra arguments to PrintPosition
**                  Recoded the way to do "visited" - bitmask
**  9-16-91 1.0a6 : Replaced redundant with GetRawValueFromDatabase
**  5-12-92 1.0a7 : Added Static Evaluation - it's perfect!
** 05-15-95 1.0   : Final release code for M.S.
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include <stdio.h>
#include "gamesman.h"

POSITION gNumberOfPositions  = 11;       /* Every board from 0 to 10 */
POSITION kBadPosition = -1;

POSITION gInitialPosition    = 0;

POSITION gMinimalPosition = 0 ;

STRING   kAuthorName          = "Dan Garcia and his GamesCrafters";
STRING   kGameName            = "1,2,...,10";
BOOLEAN  kPartizan            = FALSE;
BOOLEAN  kDebugMenu           = FALSE;
BOOLEAN  kGameSpecificMenu    = FALSE;
BOOLEAN  kTieIsPossible       = FALSE;
BOOLEAN  kLoopy               = FALSE;
BOOLEAN  kDebugDetermineValue = FALSE;
void*	 gGameSpecificTclInit = NULL;

STRING   kHelpGraphicInterface = "";  /* empty since kSupportsGraphics == FALSE */

STRING   kHelpTextInterface    =
"On your turn, select the number 1 or 2 to raise the total sum. After selecting\n your move, the total is displayed. The play then alternates to the second\n player, who also has the choice of raising the sum by 1 or 2 points. The winner\n is the first person to raise the total sum to 10. If at any point you have made\n a mistake, type u to revert back to your previous position.";

STRING   kHelpOnYourTurn =
"Select the number 1 or 2 to raise the total sum. You may also revert to your\n previous position by typing u.";

STRING   kHelpStandardObjective =
"To be the first player to raise the total to 10.";

STRING   kHelpReverseObjective =
"To force your opponent to raise the total to 10.";

STRING   kHelpTieOccursWhen = ""; /* empty since kTieIsPossible == FALSE */

STRING   kHelpExample =
"TOTAL                        :  0   \n\n\
     Dan's move [(u)ndo/1/2] : { 2 } \n\n\
TOTAL                        :  2    \n\n\
Computer's move              :  2    \n\n\
TOTAL                        :  4    \n\n\
     Dan's move [(u)ndo/1/2] : { 1 } \n\n\
TOTAL                        :  5    \n\n\
Computer's move              :  2    \n\n\
TOTAL                        :  7    \n\n\
     Dan's move [(u)ndo/1/2] : { 2 } \n\n\
TOTAL                        :  9    \n\n\
Computer's move              :  2    \n\n\
TOTAL                        : 11    \n\n\
Computer wins. Nice try, Dan.";

STRING MToS(MOVE);

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

void InitializeGame()
{
  gMoveToStringFunPtr = &MToS;
}

void FreeGame()
{
}

/************************************************************************
**
** NAME:        DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
**              kDebugMenu == FALSE
** 
************************************************************************/

void DebugMenu() { }

/************************************************************************
**
** NAME:        GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
**              the side of the board in an nxn Nim board, etc. Does
**              nothing if kGameSpecificMenu == FALSE
** 
************************************************************************/

void GameSpecificMenu() { }

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
  /* No need to have anything here, we have no extra options */
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
************************************************************************/

POSITION DoMove(thePosition, theMove)
     POSITION thePosition;
     MOVE theMove;
{
  return(thePosition + theMove);
}

/************************************************************************
**
** NAME:        GetInitialPosition
**
** DESCRIPTION: Ask the user for an initial position for testing. Store
**              it in the space pointed to by initialPosition;
** 
** INPUTS:      POSITION initialPosition : The position to fill.
**
************************************************************************/

POSITION GetInitialPosition()
{
  POSITION initialPosition;
  printf("Please input the starting value [1 - 10] : ");
  scanf(POSITION_FORMAT,&initialPosition);
  return initialPosition;
}

/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
** 
** INPUTS:      MOVE   *computersMove : The computer's move. 
**              STRING  computersName : The computer's name. 
**
************************************************************************/

void PrintComputersMove(computersMove,computersName)
     MOVE computersMove;
     STRING computersName;
{
  printf("%8s's move              : %1d\n", computersName, computersMove);
}

/************************************************************************
**
** NAME:        Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
**              'primitive' constraints. Some examples of this is having
**              three-in-a-row with TicTacToe. TicTacToe has two
**              primitives it can immediately check for, when the board
**              is filled but nobody has one = primitive tie. Three in
**              a row is a primitive lose, because the player who faces
**              this board has just lost. I.e. the player before him
**              created the board and won. Otherwise undecided.
** 
** INPUTS:      POSITION position : The position to inspect.
**
** OUTPUTS:     (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
************************************************************************/

VALUE Primitive(position) 
     POSITION position;
{
  if(position == 10) /* If it's your turn, and it's 10, THEY got there! */
    return(gStandardGame ? lose : win); 
  else
    return(undecided);
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
** CALLS:       PositionToBlankOX()
**              GetValueOfPosition()
**              GetPrediction()
**
************************************************************************/

void PrintPosition(position,playerName,usersTurn)
     POSITION position;
     STRING playerName;
     BOOLEAN  usersTurn;
{
    //  VALUE GetValueOfPosition();
  
  printf("\nTOTAL                        : " POSITION_FORMAT " %s \n\n",
         position, GetPrediction(position,playerName,usersTurn));
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
** CALLS:       MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**
************************************************************************/

MOVELIST *GenerateMoves(position)
     POSITION position;
{
  MOVELIST *head = NULL;
  MOVELIST *CreateMovelistNode();
  
  /* If at 9, you can only go 1 to 10. Otherwise you can go 1 or 2 */
  if (position < 9) 
	  head = CreateMovelistNode(2,head);
  if (position < 10)
	  head = CreateMovelistNode(1,head);
  
  return(head);
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
  USERINPUT ret, HandleDefaultTextInput();
  BOOLEAN ValidMove();
  char input[2];

  input[0] = '3';

  do {
    printf("%8s's move [(u)ndo/1/2] : ", playerName);

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
** OUTPUTS:     BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(input)
     STRING input;
{
  return(input[0] <= '2' && input[0] >= '1');
}

/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
** 
** INPUTS:      STRING input : The string input the user typed.
**
** OUTPUTS:     MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(input)
     STRING input;
{
  return((MOVE) input[0] - '0'); /* user input is 1-9, our rep. is 0-8 */
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
  printf("%d", theMove);
}


/************************************************************************
**
** NAME:        MToS
**
** DESCRIPTION: Returns the move as a STRING
** 
** INPUTS:      MOVE *theMove         : The move to put into a string.
**
************************************************************************/

STRING MToS (theMove)
     MOVE theMove;
{
  STRING move = (STRING) SafeMalloc(3);

  sprintf( move, "%d", theMove );

  return move;
}




STRING kDBName = "1210" ;

int NumberOfOptions()
{
	return 2 ;
}

int getOption()
{
	if(gStandardGame) return 1 ;
	return 2 ;
}

void setOption(int option)
{
	if(option == 1)
		gStandardGame = TRUE ;
	else
		gStandardGame = FALSE ;
}


