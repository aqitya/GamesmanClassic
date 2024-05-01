/************************************************************************
**
** NAME:        mbaghchal.c
**
** DESCRIPTION: Bagh Chal (Tigers Move)
**
** AUTHOR:      Damian Hites
**              Max Delgadillo
**              Deepa Mahajan
**              Cameron Cheung
**
** DATE:        2006.9.27
**
**************************************************************************/

#include "gamesman.h"

CONST_STRING kGameName   = "Bagh-Chal (Tigers Move)";
CONST_STRING kAuthorName = "Damian Hites, Max Delgadillo, Deepa Mahajan, Cameron Cheung";
CONST_STRING kDBName     = "baghchal";

BOOLEAN kPartizan            = TRUE;
BOOLEAN kGameSpecificMenu    = FALSE;
BOOLEAN kTieIsPossible       = FALSE;
BOOLEAN kLoopy               = TRUE;
BOOLEAN kSupportsSymmetries = TRUE;
POSITION gNumberOfPositions   =  0;
POSITION gInitialPosition     =  0;
POSITION kBadPosition         = -1;

BOOLEAN kDebugMenu           = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;

CONST_STRING kHelpGraphicInterface =
        "Not written yet";

CONST_STRING kHelpTextInterface    =
        "On your move, if you are placing a goat, you enter the\n"
        "coordinates of the space where you would like to place\n"
        "your goat (ex: a3).  If you are moving a goat or a tiger,\n"
        "you enter the coordinates of the piece you would like to\n"
        "move and the coordinates of the space you would like to\n"
        "move the piece to (ex: a3 b3).";

CONST_STRING kHelpOnYourTurn =
        "FOR TIGER:\n"
        "Move one tiger one space in any of the directions\n"
        "indicated by the lines.  You may also jump one goat\n"
        "if it is in a space next to you and the space behind\n"
        "it is empty.  Jumping a goat removes it from the board."
        "FOR GOAT, PHASE 1:\n"
        "Place a goat in a space anywhere on the board that is\n"
        "not already occupied."
        "FOR GOAT, PHASE 2:\n"
        "Move one goat one space in any of the directions\n"
        "indicated by the lines.";

CONST_STRING kHelpStandardObjective =
        "The objective of the goats is to try and trap the tigers\n"
        "so that they are unable to move.  The tigers are trying to\n"
        "remove every goat from the board.";

CONST_STRING kHelpReverseObjective = "This is not implemented.";

CONST_STRING kHelpTieOccursWhen = "A tie cannot occur.";

CONST_STRING kHelpExample = "";

#define MOVE_ENCODE(from, to, remove) ((from << 10) | (to << 5) | remove)
#define GOAT    'G'
#define TIGER   'T'
#define SPACE   '-'
#define INVALID -1

// At each point, how many adjacent points are there?
// numAdjacencies[i] gives you the number of points adjacent to point i.
int numAdjacencies[25] = {3,3,5,3,3,3,8,4,8,3,5,4,8,4,5,3,8,4,8,3,3,3,5,3,3};

// At each point, what are the adjacent points and what are the jump points?
// adjacencies[i][0] gives you the points adjacent to point i.
// adjacencies[i][1] gives you the points you can jump to from point i.
// From point i, you can jump over adjacencies[i][0][j] to reach 
// adjacencies[i][1][j]. If adjacencies[i][0][j] != -1 but 
// adjacencies[i][1][j] == -1, then from point i, you're not able to jump over
// point adjacencies[i][0][j].
int adjacencies[25][2][8] = {
	{
		{1,5,6,-1,-1,-1,-1,-1},
		{2,10,12,-1,-1,-1,-1,-1}
	},
	{
		{0,2,6,-1,-1,-1,-1,-1},
		{-1,3,11,-1,-1,-1,-1,-1}
	},
	{
		{1,3,6,7,8,-1,-1,-1},
		{0,4,10,12,14,-1,-1,-1}
	},
	{
		{2,4,8,-1,-1,-1,-1,-1},
		{1,-1,13,-1,-1,-1,-1,-1}
	},
	{
		{3,8,9,-1,-1,-1,-1,-1},
		{2,12,14,-1,-1,-1,-1,-1}
	},
	{
		{0,6,10,-1,-1,-1,-1,-1},
		{-1,7,15,-1,-1,-1,-1,-1}
	},
	{
		{0,1,2,5,7,10,11,12},
		{-1,-1,-1,-1,8,-1,16,18}
	},
	{
		{2,6,8,12,-1,-1,-1,-1},
		{-1,5,9,17,-1,-1,-1,-1}
	},
	{
		{2,3,4,7,9,12,13,14},
		{-1,-1,-1,6,-1,16,18,-1}
	},
	{
		{4,8,14,-1,-1,-1,-1,-1},
		{-1,7,19,-1,-1,-1,-1,-1}
	},
	{
		{5,6,11,15,16,-1,-1,-1},
		{0,2,12,20,22,-1,-1,-1}
	},
	{
		{6,10,12,16,-1,-1,-1,-1},
		{1,-1,13,21,-1,-1,-1,-1}
	},
	{
		{6,7,8,11,13,16,17,18},
		{0,2,4,10,14,20,22,24}
	},
	{
		{8,12,14,18,-1,-1,-1,-1},
		{3,11,-1,23,-1,-1,-1,-1}
	},
	{
		{8,9,13,18,19,-1,-1,-1},
		{2,4,12,22,24,-1,-1,-1}
	},
	{
		{10,16,20,-1,-1,-1,-1,-1},
		{5,17,-1,-1,-1,-1,-1,-1}
	},
	{
		{10,11,12,15,17,20,21,22},
		{-1,6,8,-1,18,-1,-1,-1}
	},
	{
		{12,16,18,22,-1,-1,-1,-1},
		{7,15,19,-1,-1,-1,-1,-1}
	},
	{
		{12,13,14,17,19,22,23,24},
		{6,8,-1,16,-1,-1,-1,-1}
	},
	{
		{14,18,24,-1,-1,-1,-1,-1},
		{9,17,-1,-1,-1,-1,-1,-1}
	},
	{
		{15,16,21,-1,-1,-1,-1,-1},
		{10,12,22,-1,-1,-1,-1,-1}
	},
	{
		{16,20,22,-1,-1,-1,-1,-1},
		{11,-1,23,-1,-1,-1,-1,-1}
	},
	{
		{16,17,18,21,23,-1,-1,-1},
		{10,12,14,20,24,-1,-1,-1}
	},
	{
		{18,22,24,-1,-1,-1,-1,-1},
		{13,21,-1,-1,-1,-1,-1,-1}
	},
	{
		{18,19,23,-1,-1,-1,-1,-1},
		{12,14,22,-1,-1,-1,-1,-1}
	}
};

int gSymmetryMatrix[8][25] = {
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24},
	{4,3,2,1,0,9,8,7,6,5,14,13,12,11,10,19,18,17,16,15,24,23,22,21,20},
	{20,15,10,5,0,21,16,11,6,1,22,17,12,7,2,23,18,13,8,3,24,19,14,9,4},
	{0,5,10,15,20,1,6,11,16,21,2,7,12,17,22,3,8,13,18,23,4,9,14,19,24},
	{24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
	{20,21,22,23,24,15,16,17,18,19,10,11,12,13,14,5,6,7,8,9,0,1,2,3,4},
	{4,9,14,19,24,3,8,13,18,23,2,7,12,17,22,1,6,11,16,21,0,5,10,15,20},
	{24,19,14,9,4,23,18,13,8,3,22,17,12,7,2,21,16,11,6,1,20,15,10,5,0}
};

// combinations[i][j]][k] gives i!/(j!k!(i-j-k)!)
POSITION combinations[26][5][21];

/*************************************************************************
**
** Global Variables
**
*************************************************************************/
int sideLength = 5;
int boardSize = 25;
int numInitialTigers = 4;
int numInitialGoats = 20;
int goatCaptureGoal = 5;

/*************************************************************************
**
** Function Prototypes
**
*************************************************************************/
int translate(int x, int y);
int getX(int index);
int getY(int index);
POSITION hash(char*, int, int, char);
void unhash(POSITION, char* board, int*, int*,char*);
POSITION GetCanonicalPosition(POSITION position);

void SetupTierStuff();
TIERLIST* TierChildren(TIER);
TIERPOSITION NumberOfTierPositions(TIER);
UNDOMOVELIST* GenerateUndoMovesToTier(POSITION, TIER);
POSITION UndoMove(POSITION, UNDOMOVE);
STRING TierToString(TIER);

void unhashTier(TIER, int*, int*, char*);
TIER hashTier(int, int, char);
void unhashMove(MOVE, int*, int*, int*);

/************************************************************************
**
** NAME:        InitializeGame
**
** DESCRIPTION: Prepares the game for execution.
**              Initializes required variables.
**              Sets up gDatabase (if necessary).
**
************************************************************************/

POSITION fact(int n) {
	POSITION prod = 1;
	for (int i = 2; i <= n; i++) {
		prod *= i;
	}
	return prod;
}

// calculates the number of combinations
POSITION getNumPos(int size, int numTigers, int numGoats) {
    int numBlanks = size - numTigers - numGoats;

    // Do fact(size) / fact(numTigers) / fact(numGoats) / fact(numBlanks)
	// without overflowing
	POSITION n1, n2, n3;
	if (numBlanks >= numTigers) {
		n2 = numTigers;
		if (numBlanks >= numGoats) {
			n1 = numBlanks;
			n3 = numGoats;
		} else {
			n1 = numGoats;
			n3 = numBlanks;
		}
	} else {
		n2 = numBlanks;
		if (numTigers >= numGoats) {
			n1 = numTigers;
			n3 = numGoats;
		} else {
			n1 = numGoats;
			n3 = numTigers;
		}
	}

	POSITION temp = 1;
	for (int i = n1 + 1; i <= size; i++) {
		temp *= i;
	}
	return temp / fact(n2) / fact(n3);
}

// Pre-calculate the number of combinations and store in combinations array
// Combinations can be retrieved using combinations[boardsize][numx][numo]
void combinationsInit() {
	int m1, m2;
    for (int size = 0; size <= 25; size++) {
		m1 = size > 4 ? 4 : size;
        for (int numTigers = 0; numTigers <= m1; numTigers++) {
			m2 = (size - numTigers > 20) ? 20 : size - numTigers;
            for (int numGoats = 0; numGoats <= m2; numGoats++) {
                combinations[size][numTigers][numGoats] = getNumPos(size, numTigers, numGoats);
			}
		}
	}
}

void InitializeGame() {

	/* FOR THE PURPOSES OF INTERACT. FEEL FREE TO CHANGE IF SOLVING. */ 
	if (gIsInteract) {
		gLoadTierdbArray = FALSE; // SET TO TRUE IF SOLVING
	}
	/********************************/

	combinationsInit();

	gSymmetries = TRUE;
	gCanonicalPosition = GetCanonicalPosition;

	kSupportsTierGamesman = TRUE;
	kExclusivelyTierGamesman = TRUE;

	gTierChildrenFunPtr = &TierChildren;
	gNumberOfTierPositionsFunPtr = &NumberOfTierPositions;
	gTierToStringFunPtr = &TierToString;

	gUnDoMoveFunPtr = &UndoMove;
	gGenerateUndoMovesToTierFunPtr = &GenerateUndoMovesToTier;

	gInitialTier = hashTier(20, 0, GOAT);
	char initialBoard[25] = {
		TIGER, SPACE, SPACE, SPACE, TIGER,
		SPACE, SPACE, SPACE, SPACE, SPACE,
		SPACE, SPACE, SPACE, SPACE, SPACE,
		SPACE, SPACE, SPACE, SPACE, SPACE,
		TIGER, SPACE, SPACE, SPACE, TIGER
	};
	gInitialTierPosition = hash(initialBoard, 20, 0, GOAT);
	gInitialPosition = gInitialTierPosition;
	gNumberOfPositions = NumberOfTierPositions(gInitialTier);
}


/************************************************************************
**
** NAME:        GenerateMoves
**
************************************************************************/

MOVELIST *GenerateMoves(POSITION position) {
	int goatsLeft, goatsCaptured;
	char board[boardSize];
	char turn;
	unhash(position, board, &goatsLeft, &goatsCaptured, &turn);
	MOVELIST *moves = NULL;
	int adjIndex, jumpIndex, i, j;
	if (goatsCaptured < goatCaptureGoal) {
		for (i = 0; i < boardSize; i++) {
			if (turn == GOAT && goatsLeft && board[i] == SPACE) { 
				// Goat Placement
				moves = CreateMovelistNode(MOVE_ENCODE(i, i, i), moves);
			} else if (board[i] == turn && (turn == TIGER || !goatsLeft)) { 
				for (j = 0; j < numAdjacencies[i]; j++) {
					adjIndex = adjacencies[i][0][j];
					jumpIndex =  adjacencies[i][1][j];
					if (board[adjIndex] == SPACE) {
						// Goat or Tiger Sliding
						moves = CreateMovelistNode(MOVE_ENCODE(i, adjIndex, adjIndex), moves);
					} else if (turn == TIGER && jumpIndex != -1 && board[adjIndex] == GOAT && board[jumpIndex] == SPACE) {
						// Tiger Jumping
						moves = CreateMovelistNode(MOVE_ENCODE(i, jumpIndex, adjIndex), moves);
					}
				}
			}
		}
	}
	return moves;
}


/************************************************************************
**
** NAME:        DoMove
**
*************************************************************************/

POSITION DoMove(POSITION position, MOVE move) {
	int goatsLeft, goatsCaptured;
	char board[boardSize];
	char turn;
	unhash(position, board, &goatsLeft, &goatsCaptured, &turn);
	int from, to, remove;
	unhashMove(move, &from, &to, &remove);

	board[to] = turn;
	if (from == to) {
		goatsLeft--;
	} else {
		board[from] = SPACE;
	}

	if (remove != to) {
		board[remove] = SPACE;
		goatsCaptured++;
	}

	return hash(board, goatsLeft, goatsCaptured, turn == GOAT ? TIGER : GOAT);
}

/************************************************************************
**
** NAME:        Primitive
**
************************************************************************/

VALUE Primitive(POSITION position) {
	MOVELIST* moves = GenerateMoves(position);
	if (moves == NULL) return lose;
	FreeMoveList(moves);
	return undecided;
}


/************************************************************************
**
** NAME:        PrintPosition
**
** DESCRIPTION: Prints the position in a pretty format, including the
**              prediction of the game's outcome.
**
** INPUTS:      POSITION position    : The position to pretty print.
**              STRING   playersName : The name of the player.
**              BOOLEAN  usersTurn   : TRUE <==> it's a user's turn.
**
** CALLS:       GetPrediction()      : Returns the prediction of the game
**
************************************************************************/

void PrintPosition(POSITION position, STRING playersName, BOOLEAN usersTurn) {
	int goatsLeft, goatsCaptured;
	char board[boardSize];
	char turn;
	unhash(position, board, &goatsLeft, &goatsCaptured, &turn);
	printf("\t%s's Turn (%s):\n  ", playersName, (turn == GOAT ? "Goat" : "Tiger"));
	int i, j;
	for (i = 1; i <= sideLength + 1; i++) { // print the rows one by one
		if (i <= sideLength) {
			printf("\t%d ", sideLength - i + 1); // first, print the row with the pieces
			for (j = 1; j <= sideLength; j++) {
				char theChar = board[translate(i, j)];
				printf("%c", (theChar == SPACE) ? '+' : theChar);
				if (j < sideLength)
					printf("-");
			}
			printf("\n\t  ");
			if (i < sideLength) { // then, print the row with the lines (diagonals and such)
				for (j = 1; j < sideLength; j++) {
					if(j % 2 && i % 2)
						printf("|\\");
					else if(j % 2 && ((i % 2) == 0))
						printf("|/");
					else if(i % 2)
						printf("|/");
					else
						printf("|\\");

					if (j == sideLength - 1)
						printf("|");
				}
			}
			if (i == 1) {
				if (goatsLeft != 0) printf("  <STAGE 1> Goats To Place: %d, Goats Captured: %d", goatsLeft, goatsCaptured);
				else printf("  <STAGE 2> Goats Captured: %d", goatsCaptured);
			} else if (i == 2)
				printf("    %s", GetPrediction(position, playersName, usersTurn));
			if(i<sideLength) printf("\n");
		} else if (i > sideLength) {
			for (j = 1; j <= sideLength; j++) // print the column numbers
				printf("%c ", j + 'a' - 1);
		}
	}
	printf("\n\n");
}

/************************************************************************
**
** NAME:        GetAndPrintPlayersMove
**
** DESCRIPTION: Finds out if the player wishes to undo, abort, or use
**              some other gamesman option. The gamesman core does
**              most of the work here.
**
** INPUTS:      POSITION position    : Current position
**              MOVE     *move       : The move to fill with user's move.
**              STRING   playersName : Current Player's Name
**
** OUTPUTS:     USERINPUT          : One of
**                                   (Undo, Abort, Continue)
**
** CALLS:       USERINPUT HandleDefaultTextInput(POSITION, MOVE*, STRING)
**                                 : Gamesman Core Input Handling
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playersName) {
	USERINPUT input;
	int goatsLeft, goatsCaptured;
	char board[boardSize];
	char turn;
	unhash(position, board, &goatsLeft, &goatsCaptured, &turn);

	for (;;) {
		/***********************************************************
		* CHANGE THE LINE BELOW TO MATCH YOUR MOVE FORMAT
		***********************************************************/
		if(turn == GOAT && goatsLeft > 0) {//stage1
			printf("%8s's move [(undo)/([%c-%c][%d-%d])] : ", playersName, 'a', sideLength + 'a' - 1, 1, sideLength);
		} else {
			printf("%8s's move [(undo)/([%c-%c][%d-%d] [%c-%c][%d-%d])] : ", playersName, 'a', sideLength + 'a' - 1, 1, sideLength, 'a', sideLength+'a' - 1, 1, sideLength);
		}
		input = HandleDefaultTextInput(position, move, playersName);

		if (input != Continue)
			return input;
	}

	/* NOTREACHED */
	return Continue;
}


/***v*********************************************************************
**
** NAME:        ValidTextInput
**
** DESCRIPTION: Rudimentary check to check if input is in the move form
**              you are expecting. Does not check if it is a valid move.
**              Only checks if it fits the move form.
**
************************************************************************/

BOOLEAN ValidTextInput(STRING input) {
	int size = strlen(input);
	if(size != 2 && size != 4)
		return FALSE;
	if (!isalpha(input[0]) || !isdigit(input[1]) ||
	    (size == 4 && (!isalpha(input[2]) || !isdigit(input[3]))))
		return FALSE;
	return TRUE;
}


/************************************************************************
**
** NAME:        ConvertTextInputToMove
**
** DESCRIPTION: Converts the string input your internal move representation.
**              Gamesman already checked the move with ValidTextInput
**              and ValidMove.
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input) {
	int y = input[0] - 'a' + 1;
	int x = sideLength - input[1] + '1';
	if (input[2] == '\0') {
		int idx = translate(x,y);
		return MOVE_ENCODE(idx, idx, idx);
	}
	int y1 = input[2] - 'a' + 1;
	int x1 = sideLength - input[3] + '1';
	
	int removeX, removeY;
	if (x1 > x) {
		removeX = (x1 > x + 1) ? x1 - 1 : x1;
	} else {
		removeX = (x1 < x - 1) ? x1 + 1 : x1;
	}
	if (y1 > y) {
		removeY = (y1 > y + 1) ? y1 - 1 : y1;
	} else {
		removeY = (y1 < y - 1) ? y1 + 1 : y1;
	}

	return MOVE_ENCODE(translate(x, y), translate(x1, y1), translate(removeX, removeY));
}

void GameSpecificMenu() {}
void SetTclCGameSpecificOptions (int options[]) { (void)options; }
void DebugMenu() {}
void setOption(int option) { (void)option; }


int NumberOfOptions() {
	return 1;
}

int getOption() {
	return 0;
}

int translate(int x, int y) {
	return (x - 1) * sideLength + (y - 1);
}

int getX(int index) {
	return index / sideLength + 1;
}

int getY(int index) {
	return index % sideLength + 1;
}

POSITION hash(char* board, int goatsLeft, int goatsCaptured, char turn) {
	POSITION sum = 0;
	int numGoats = numInitialGoats - goatsLeft - goatsCaptured;
	int numTigers = numInitialTigers;
    for (int i = boardSize - 1; i > 0; i--) { // No need to calculate i == 0
        switch (board[i]) {
            case GOAT:
                numGoats--;
                break;
            case TIGER:
                if (numGoats > 0) sum += combinations[i][numTigers][numGoats - 1];
                numTigers--;
                break;
            case SPACE:
                if (numGoats > 0) sum += combinations[i][numTigers][numGoats - 1];
                if (numTigers > 0) sum += combinations[i][numTigers - 1][numGoats];
                break;
        }
    }
	TIER tier = hashTier(goatsLeft, goatsCaptured, turn);
	TIERPOSITION tierPosition = sum;
	if (goatsLeft == 0) { // If in Phase 2
		tierPosition <<= 1;
		if (turn == TIGER) {
			tierPosition |= 1;
		}
	}
	if (gHashWindowInitialized) {
		return gHashToWindowPosition(tierPosition, tier);
	} else {
		return tierPosition;
	}
}

void unhash(POSITION position, char *board, int *goatsLeft, int *goatsCaptured, char *turn) {
	TIER tier; TIERPOSITION tierPosition;
	gUnhashToTierPosition(position, &tierPosition, &tier);

	char turnIfInPhase1;
	unhashTier(tier, goatsLeft, goatsCaptured, &turnIfInPhase1);
	if ((*goatsLeft) > 0) { // If in Phase 1
		(*turn) = turnIfInPhase1;
	} else { // If in Phase 2
		(*turn) = (tierPosition & 1) ? TIGER : GOAT;
		tierPosition >>= 1;
	}
	POSITION o1, o2;
	int numGoats = numInitialGoats - (*goatsLeft) - (*goatsCaptured);
	int numTigers = numInitialTigers;
    for (int i = boardSize - 1; i >= 0; i--) {
        o1 = (numGoats > 0) ? combinations[i][numTigers][numGoats - 1] : 0;
        o2 = o1 + ((numTigers > 0) ? combinations[i][numTigers - 1][numGoats] : 0);
        if (tierPosition >= o2) {
            board[i] = SPACE;
            tierPosition -= o2;
        } else if (tierPosition >= o1) {
            if (numTigers > 0) {
                board[i] = TIGER;
                numTigers--;
            } else
                board[i] = SPACE;
            tierPosition -= o1;
        } else {
            if (numGoats > 0) {
                board[i] = GOAT;
                numGoats--;
            } else if (numTigers > 0) {
                board[i] = TIGER;
                numTigers--;
            } else
                board[i] = SPACE;
        }
    }
}

void unhashMove(MOVE move, int *from, int *to, int *remove) {
	(*from) = move >> 10;
	(*to) = (move >> 5) & 0x1F;
	(*remove) = move & 0x1F;
}

void MoveToString(MOVE move, char *moveStringBuffer) {
	int from, to, remove;
	unhashMove(move, &from, &to, &remove);
	int fromX = getX(from);
	int fromY = getY(from);
	int toX = getX(to);
	int toY = getY(to);
	if (from == to) { // Placement
		snprintf(
			moveStringBuffer, 8, 
			"%c%c", toY + 'a' - 1, '0' + sideLength - toX + 1);
	} else { // Movement
		snprintf(
			moveStringBuffer, 8, "%c%c%c%c", 
			fromY + 'a' - 1, '0' + sideLength - fromX + 1, 
			toY + 'a' - 1, '0' + sideLength - toX + 1);
	}
}

/************************************************************************
**
** NAME:        PrintComputersMove
**
** DESCRIPTION: Nicely formats the computers move.
**
** INPUTS:      MOVE    computersMove : The computer's move.
**              STRING  computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove (MOVE computersMove, STRING computersName) {
	printf("%8s's move : ", computersName);
	char moveStringBuffer[8];
	MoveToString(computersMove, moveStringBuffer);
	printf("%s\n\n", moveStringBuffer);
}

/************************************************************************
**
** NAME:        GetCanonicalPosition
**
** DESCRIPTION: Go through all of the positions that are symmetrically
**              equivalent and return the SMALLEST, which will be used
**              as the canonical element for the equivalence set.
**
** INPUTS:      POSITION position : The position return the canonical elt. of.
**
** OUTPUTS:     POSITION          : The canonical element of the set.
**
************************************************************************/

POSITION GetCanonicalPosition(POSITION position) {
	int goatsLeft, goatsPlaced;
	char originalBoard[boardSize];
	char turn;
	unhash(position, originalBoard, &goatsLeft, &goatsPlaced, &turn);
	char canonBoard[boardSize];
    int bestSymmetryNum = 0;

    for (int symmetryNum = 1; symmetryNum < 8; symmetryNum++)
        for (int i = boardSize - 1; i >= 0; i--) {
            char pieceInSymmetry = originalBoard[gSymmetryMatrix[symmetryNum][i]];
            char pieceInBest = originalBoard[gSymmetryMatrix[bestSymmetryNum][i]];
            if (pieceInSymmetry != pieceInBest) {
                if (pieceInSymmetry > pieceInBest) // If new smallest hash.
                    bestSymmetryNum = symmetryNum;
                break;
            }
        };

    if (bestSymmetryNum == 0) {
        return position;
	}
    
    for (int i = 0; i < boardSize; i++) // Transform the rest of the board.
        canonBoard[i] = originalBoard[gSymmetryMatrix[bestSymmetryNum][i]];

    return hash(canonBoard, goatsLeft, goatsPlaced, turn);
}

void unhashTier(TIER tier, int *goatsLeft, int *goatsCaptured, char *turnIfInPhase1) {
	(*turnIfInPhase1) = (tier >> 10) ? TIGER : GOAT;
	(*goatsLeft) = (tier >> 5) & 0x1F;
	(*goatsCaptured) = tier & 0x1F;
}

TIER hashTier(int goatsLeft, int goatsCaptured, char turnIfInPhase1) {
	// Assumes goatsCaptured is fewer than 5 bits
	if (goatsLeft == 0) {
		turnIfInPhase1 = TIGER;
	}
	return (((turnIfInPhase1 == TIGER) ? 1 : 0) << 10) | (goatsLeft << 5) | goatsCaptured;
}

TIERLIST* TierChildren(TIER tier) {
	TIERLIST* list = NULL;
	int goatsLeft, goatsCaptured;
	char turnIfInPhase1;
	unhashTier(tier, &goatsLeft, &goatsCaptured, &turnIfInPhase1);
	if (goatsCaptured < goatCaptureGoal) {
		if (goatsLeft) { // Phase 1
			if (turnIfInPhase1 == GOAT) {
				list = CreateTierlistNode(hashTier(goatsLeft - 1, goatsCaptured, TIGER), list);
			} else {
				list = CreateTierlistNode(hashTier(goatsLeft, goatsCaptured, GOAT), list);
				list = CreateTierlistNode(hashTier(goatsLeft, goatsCaptured + 1, GOAT), list);
			}
		} else { // Phase 2
			list = CreateTierlistNode(tier, list);
			if (goatsLeft + goatsCaptured < numInitialGoats) {
				// if there are goats on the board
				list = CreateTierlistNode(hashTier(goatsLeft, goatsCaptured + 1, TIGER), list);
			}
		}
	}
	return list;
}

TIERPOSITION NumberOfTierPositions(TIER tier) {
	int goatsLeft, goatsCaptured;
	char turnIfInPhase1;
	unhashTier(tier, &goatsLeft, &goatsCaptured, &turnIfInPhase1);
	int numGoats = numInitialGoats - goatsLeft - goatsCaptured;
	return combinations[boardSize][numInitialTigers][numGoats] << ((goatsLeft == 0) ? 1 : 0);
}

POSITION UndoMove(POSITION position, UNDOMOVE undoMove) {
	int goatsLeft, goatsCaptured;
	char board[boardSize];
	char turn;
	unhash(position, board, &goatsLeft, &goatsCaptured, &turn);
	char oppTurn = (turn == GOAT) ? TIGER : GOAT;
	int from, to, remove;
	unhashMove(undoMove, &from, &to, &remove);

	board[to] = SPACE;
	if (to == from) {
		goatsLeft++;
	} else {
		board[from] = oppTurn;
	}
	if (to != remove) {
		board[remove] = GOAT;
		goatsCaptured--;
	}
	turn = oppTurn;

	return hash(board, goatsLeft, goatsCaptured, turn);
}

UNDOMOVELIST *GenerateUndoMovesToTier(POSITION position, TIER tier) {
	UNDOMOVELIST *undoMoves = NULL;
	int goatsLeft, goatsCaptured;
	char board[boardSize];
	char turn;
	unhash(position, board, &goatsLeft, &goatsCaptured, &turn);
	char oppTurn = (turn == GOAT) ? TIGER : GOAT;

	int toGoatsLeft, toGoatsCaptured;
	char toTurn;
	unhashTier(tier, &toGoatsLeft, &toGoatsCaptured, &toTurn);
	if (toGoatsLeft == goatsLeft && toGoatsCaptured == goatsCaptured && (oppTurn == TIGER || !goatsLeft)) { // Reverse a slide
		for (int i = 0; i < boardSize; i++) {
			if (board[i] == oppTurn) {
				for (int j = 0; j < numAdjacencies[i]; j++) {
					int adjIndex = adjacencies[i][0][j];
					if (board[adjIndex] == SPACE) {
						undoMoves = CreateUndoMovelistNode(MOVE_ENCODE(adjIndex, i, i), undoMoves);
					}
				}
			}
		}
	} else if (toGoatsLeft == goatsLeft + 1 && oppTurn == GOAT) { // Reverse a goat placement
		for (int i = 0; i < boardSize; i++) {
			if (board[i] == GOAT) {
				undoMoves = CreateUndoMovelistNode(MOVE_ENCODE(i, i, i), undoMoves);
			}
		}
	} else if (toGoatsCaptured == goatsCaptured - 1 && oppTurn == TIGER) { // Reverse a tiger capture
		for (int i = 0; i < boardSize; i++) {
			if (board[i] == TIGER) {
				for (int j = 0; j < numAdjacencies[i]; j++) {
					int adjIndex = adjacencies[i][0][j];
					int jumpIndex =  adjacencies[i][1][j];
					if (board[adjIndex] == SPACE && jumpIndex != -1 && board[jumpIndex] == SPACE) {
						undoMoves = CreateUndoMovelistNode(MOVE_ENCODE(jumpIndex, i, adjIndex), undoMoves);
					}
				}
			}
		}
	}

	return undoMoves;
}

STRING TierToString(TIER tier) {
	STRING tierStr = (STRING) SafeMalloc(sizeof(char) * 64);
	int goatsLeft, goatsCaptured;
	char turn;
	unhashTier(tier, &goatsLeft, &goatsCaptured, &turn);
	int goatsOnBoard = numInitialGoats - goatsLeft - goatsCaptured;

	if (goatsLeft) {
		sprintf(tierStr, "Stage 1: %d Goats On Board, %d To Place, %d Goats Removed, %s's Turn", goatsOnBoard, goatsLeft, goatsCaptured, (turn == GOAT) ? "Goat" : "Tiger");
	} else {
		sprintf(tierStr, "Stage 2: %d Goats On Board, %d Goats Removed", goatsOnBoard, goatsCaptured);
	}
	return tierStr;
}

POSITION StringToPosition(char *positionString) {
	int turn;
	char *board;
	if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
		char turnChar = (turn == 1) ? GOAT : TIGER;
		int goatsLeft = 10 * (board[boardSize] - '0') + (board[boardSize + 1] - '0');
		int goatsCaptured = 10 * (board[boardSize + 2] - '0') + (board[boardSize + 3] - '0');

		POSITION position = hash(board, goatsLeft, goatsCaptured, turnChar);
		gInitializeHashWindow(hashTier(goatsLeft, goatsCaptured, turnChar), FALSE);
		return position;
	}
	return NULL_POSITION;
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	char board[boardSize + 5];
	int goatsLeft, goatsCaptured;
	char turn;
	unhash(position, board, &goatsLeft, &goatsCaptured, &turn);
	board[boardSize] = (goatsLeft / 10) + '0';
	board[boardSize + 1] = (goatsLeft % 10) + '0';
	board[boardSize + 2] = (goatsCaptured / 10) + '0';
	board[boardSize + 3] = (goatsCaptured % 10) + '0';
	board[boardSize + 4] = '\0';
	AutoGUIMakePositionString(turn == GOAT ? 1 : 2, board, autoguiPositionStringBuffer);
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
	TIER tier; TIERPOSITION tierposition;
	gUnhashToTierPosition(position, &tierposition, &tier);
	int goatsLeft, goatsCaptured;
	char turn;
	unhashTier(tier, &goatsLeft, &goatsCaptured, &turn);
	if (goatsLeft == 0) {
		turn = (position & 1) ? TIGER : GOAT;
	}

	char sound = (turn == GOAT) ? 'g' : 't';
	int from, to, remove;
	unhashMove(move, &from, &to, &remove);
	if (from == to) {
		AutoGUIMakeMoveButtonStringA('-', to, sound, autoguiMoveStringBuffer);
	} else {
		AutoGUIMakeMoveButtonStringM(from, to, sound, autoguiMoveStringBuffer);
	}
}
