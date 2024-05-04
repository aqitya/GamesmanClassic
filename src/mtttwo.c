/************************************************************************
**
** NAME:        mtttwo.c
**
** DESCRIPTION: Tic-Tac-Two
**
** AUTHOR:      Stella Wan, Nala Chen, Cameron Cheung
**
** DATE:        2024-01-01
**
************************************************************************/

#include "gamesman.h"

POSITION gNumberOfPositions = 0;
POSITION kBadPosition = -1;
POSITION gInitialPosition = 0;

CONST_STRING kAuthorName = "Stella Wan, Nala Chen, and Cameron Cheung";
CONST_STRING kGameName = "Tic-Tac-Two";
CONST_STRING kDBName = "tttwo";
BOOLEAN kPartizan = TRUE;
BOOLEAN kTieIsPossible = TRUE;
BOOLEAN kLoopy = TRUE;
BOOLEAN kGameSpecificMenu = FALSE;
BOOLEAN kDebugDetermineValue = FALSE;
BOOLEAN kDebugMenu = FALSE;

CONST_STRING kHelpGraphicInterface = "";
CONST_STRING kHelpTextInterface = "";
CONST_STRING kHelpOnYourTurn = "";
CONST_STRING kHelpStandardObjective = "";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpTieOccursWhen = "";
CONST_STRING kHelpExample = "";

/**
 * @brief Tcl-related stuff. Do not change if you do not plan to make a Tcl interface.
 */
void *gGameSpecificTclInit = NULL;
void SetTclCGameSpecificOptions(int theOptions[]) { (void)theOptions; }

void InitializeGame();
void DebugMenu();
POSITION hash(char *board, int xPlaced, int oPlaced, int gridPos, char turn);
void unhash(POSITION position, char *board, int *xPlaced, int *oPlaced, int *gridPos, char *turn);
MOVE hashMove(BOOLEAN isGridMove, int from, int to);
void unhashMove(MOVE move, BOOLEAN *isGridMove, int *from, int *to);
void GameSpecificMenu();
POSITION DoMove(POSITION position, MOVE move);
void PrintComputersMove(MOVE computersMove, STRING computersName);
VALUE Primitive(POSITION position);
void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn);
MOVELIST *GenerateMoves(POSITION position);
POSITION GetCanonicalPosition(POSITION position);
void unhashTier(TIER tier, int *xPlaced, int *oPlaced, char *turn);
TIER hashTier(int xPlaced, int oPlaced);
TIERLIST* getTierChildren(TIER tier);
TIERPOSITION NumberOfTierPositions(TIER tier);
STRING tierToString(TIER tier);
POSITION UnDoMove(POSITION position, UNDOMOVE undoMove);
UNDOMOVELIST *GenerateUndoMovesToTier(POSITION position, TIER tier);
BOOLEAN ValidTextInput(STRING input);
MOVE ConvertTextInputToMove(STRING input);
void PrintMove(MOVE theMove);
int NumberOfOptions();
int getOption();
void setOption(int option);
MULTIPARTEDGELIST* GenerateMultipartMoveEdges(POSITION position, MOVELIST *moveList, POSITIONLIST *positionList);

POSITION GetCanonicalPositionTest(POSITION position, POSITION *symmetricTo);

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

#define X 'X'
#define O 'O'
#define BLANK '-'
int sideLength = 5;
int boardSize = 25;
int numPiecesPerPlayer = 4; // Number of pieces each player has.
int numGridPlacements = 9; // Number of ways you can place the grid.

int centerMapping[25] = {-1,-1,-1,-1,-1,-1,0,1,2,-1,-1,3,4,5,-1,-1,6,7,8,-1,-1,-1,-1,-1,-1};
int revCenterMapping[9] = {6,7,8,11,12,13,16,17,18};
int gridSlots[9][9] = {
  {0,1,2,5,6,7,10,11,12},
  {1,2,3,6,7,8,11,12,13},
  {2,3,4,7,8,9,12,13,14},
  {5,6,7,10,11,12,15,16,17},
  {6,7,8,11,12,13,16,17,18},
  {7,8,9,12,13,14,17,18,19},
  {10,11,12,15,16,17,20,21,22},
  {11,12,13,16,17,18,21,22,23},
  {12,13,14,17,18,19,22,23,24}
};
int allTheRows[9][8][3] = {
  {
    {0,1,2},
    {5,6,7},
    {10,11,12},
    {0,5,10},
    {1,6,11},
    {2,7,12},
    {0,6,12},
    {2,6,10}
  },
  {
    {1,2,3},
    {6,7,8},
    {11,12,13},
    {1,6,11},
    {2,7,12},
    {3,8,13},
    {1,7,13},
    {3,7,11}
  },
  {
    {2,3,4},
    {7,8,9},
    {12,13,14},
    {2,7,12},
    {3,8,13},
    {4,9,14},
    {2,8,14},
    {4,8,12}
  },
  {
    {5,6,7},
    {10,11,12},
    {15,16,17},
    {5,10,15},
    {6,11,16},
    {7,12,17},
    {5,11,17},
    {7,11,15}
  },
  {
    {6,7,8},
    {11,12,13},
    {16,17,18},
    {6,11,16},
    {7,12,17},
    {8,13,18},
    {6,12,18},
    {8,12,16}
  },
  {
    {7,8,9},
    {12,13,14},
    {17,18,19},
    {7,12,17},
    {8,13,18},
    {9,14,19},
    {7,13,19},
    {9,13,17}
  },
  {
    {10,11,12},
    {15,16,17},
    {20,21,22},
    {10,15,20},
    {11,16,21},
    {12,17,22},
    {10,16,22},
    {12,16,20}
  },
  {
    {11,12,13},
    {16,17,18},
    {21,22,23},
    {11,16,21},
    {12,17,22},
    {13,18,23},
    {11,17,23},
    {13,17,21}
  },
  {
    {12,13,14},
    {17,18,19},
    {22,23,24},
    {12,17,22},
    {13,18,23},
    {14,19,24},
    {12,18,24},
    {14,18,22}
  }
};
int numGridAdjacencies[9] = {3,5,3,5,8,5,3,5,3};
int gridAdjacencies[9][8] = {
  {7,11,12,-1,-1,-1,-1,-1},
  {6,8,11,12,13,-1,-1,-1},
  {7,12,13,-1,-1,-1,-1,-1},
  {6,7,12,16,17,-1,-1,-1},
  {6,7,8,11,13,16,17,18},
  {7,8,12,17,18,-1,-1,-1},
  {11,12,17,-1,-1,-1,-1,-1},
  {11,12,13,16,18,-1,-1,-1},
  {12,13,17,-1,-1,-1,-1,-1}
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

POSITION combinations[26][5][5] = {{{1,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},{{1,1,0,0,0},{1,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},{{1,2,1,0,0},{2,2,0,0,0},{1,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},{{1,3,3,1,0},{3,6,3,0,0},{3,3,0,0,0},{1,0,0,0,0},{0,0,0,0,0}},{{1,4,6,4,1},{4,12,12,4,0},{6,12,6,0,0},{4,4,0,0,0},{1,0,0,0,0}},{{1,5,10,10,5},{5,20,30,20,5},{10,30,30,10,0},{10,20,10,0,0},{5,5,0,0,0}},{{1,6,15,20,15},{6,30,60,60,30},{15,60,90,60,15},{20,60,60,20,0},{15,30,15,0,0}},{{1,7,21,35,35},{7,42,105,140,105},{21,105,210,210,105},{35,140,210,140,35},{35,105,105,35,0}},{{1,8,28,56,70},{8,56,168,280,280},{28,168,420,560,420},{56,280,560,560,280},{70,280,420,280,70}},{{1,9,36,84,126},{9,72,252,504,630},{36,252,756,1260,1260},{84,504,1260,1680,1260},{126,630,1260,1260,630}},{{1,10,45,120,210},{10,90,360,840,1260},{45,360,1260,2520,3150},{120,840,2520,4200,4200},{210,1260,3150,4200,3150}},{{1,11,55,165,330},{11,110,495,1320,2310},{55,495,1980,4620,6930},{165,1320,4620,9240,11550},{330,2310,6930,11550,11550}},{{1,12,66,220,495},{12,132,660,1980,3960},{66,660,2970,7920,13860},{220,1980,7920,18480,27720},{495,3960,13860,27720,34650}},{{1,13,78,286,715},{13,156,858,2860,6435},{78,858,4290,12870,25740},{286,2860,12870,34320,60060},{715,6435,25740,60060,90090}},{{1,14,91,364,1001},{14,182,1092,4004,10010},{91,1092,6006,20020,45045},{364,4004,20020,60060,120120},{1001,10010,45045,120120,210210}},{{1,15,105,455,1365},{15,210,1365,5460,15015},{105,1365,8190,30030,75075},{455,5460,30030,100100,225225},{1365,15015,75075,225225,450450}},{{1,16,120,560,1820},{16,240,1680,7280,21840},{120,1680,10920,43680,120120},{560,7280,43680,160160,400400},{1820,21840,120120,400400,900900}},{{1,17,136,680,2380},{17,272,2040,9520,30940},{136,2040,14280,61880,185640},{680,9520,61880,247520,680680},{2380,30940,185640,680680,1701700}},{{1,18,153,816,3060},{18,306,2448,12240,42840},{153,2448,18360,85680,278460},{816,12240,85680,371280,1113840},{3060,42840,278460,1113840,3063060}},{{1,19,171,969,3876},{19,342,2907,15504,58140},{171,2907,23256,116280,406980},{969,15504,116280,542640,1763580},{3876,58140,406980,1763580,5290740}},{{1,20,190,1140,4845},{20,380,3420,19380,77520},{190,3420,29070,155040,581400},{1140,19380,155040,775200,2713200},{4845,77520,581400,2713200,8817900}},{{1,21,210,1330,5985},{21,420,3990,23940,101745},{210,3990,35910,203490,813960},{1330,23940,203490,1085280,4069800},{5985,101745,813960,4069800,14244300}},{{1,22,231,1540,7315},{22,462,4620,29260,131670},{231,4620,43890,263340,1119195},{1540,29260,263340,1492260,5969040},{7315,131670,1119195,5969040,22383900}},{{1,23,253,1771,8855},{23,506,5313,35420,168245},{253,5313,53130,336490,1514205},{1771,35420,336490,2018940,8580495},{8855,168245,1514205,8580495,34321980}},{{1,24,276,2024,10626},{24,552,6072,42504,212520},{276,6072,63756,425040,2018940},{2024,42504,425040,2691920,12113640},{10626,212520,2018940,12113640,51482970}},{{1,25,300,2300,12650},{25,600,6900,50600,265650},{300,6900,75900,531300,2656500},{2300,50600,531300,3542000,16824500},{12650,265650,2656500,16824500,75710250}}};

BOOLEAN kSupportsSymmetries = TRUE; /* Whether we support symmetries */

/**
 * @brief Initialize global variables.
 */
void InitializeGame() {

  /* FOR THE PURPOSES OF INTERACT. FEEL FREE TO CHANGE IF SOLVING. */ 
	if (gIsInteract) {
		gLoadTierdbArray = FALSE; // SET TO TRUE IF SOLVING
	}
	/********************************/

	gCanonicalPosition = GetCanonicalPosition;

	kSupportsTierGamesman = TRUE;
	kExclusivelyTierGamesman = TRUE;

	gTierChildrenFunPtr = &getTierChildren;
	gNumberOfTierPositionsFunPtr = &NumberOfTierPositions;
	gTierToStringFunPtr = &tierToString;

  gUnDoMoveFunPtr = &UnDoMove;
	gGenerateUndoMovesToTierFunPtr = &GenerateUndoMovesToTier;
  gGenerateMultipartMoveEdgesFunPtr = &GenerateMultipartMoveEdges;

  gInitialTier = 0;
  gInitialTierPosition = 4; // Hash of the initial position in the initial tier
  //gInitialPosition = gInitialTierPosition;
}

POSITION hash(char *board, int xPlaced, int oPlaced, int gridPos, char turn) {
  POSITION sum = 0;
	int numX = xPlaced;
	int numO = oPlaced;
  for (int i = boardSize - 1; i > 0; i--) { // no need to calculate i == 0
    switch (board[i]) {
      case X:
        numX--;
        break;
      case O:
        if (numX > 0) sum += combinations[i][numO][numX-1];
        numO--;
        break;
      case BLANK:
        if (numX > 0) sum += combinations[i][numO][numX-1];
        if (numO > 0) sum += combinations[i][numO-1][numX];
        break;
    }
  }
	TIER tier = hashTier(xPlaced, oPlaced);
	TIERPOSITION tierPosition = sum * numGridPlacements + centerMapping[gridPos];
	if (oPlaced >= 2) {
    tierPosition <<= 1;
    if (turn == O) {
      tierPosition |= 1;
    }
	}
  return gHashToWindowPosition(tierPosition, tier);
}

void unhash(POSITION position, char *board, int *xPlaced, int *oPlaced, int *gridPos, char *turn) {
  TIER tier; TIERPOSITION tierPosition;
  gUnhashToTierPosition(position, &tierPosition, &tier);
  
  unhashTier(tier, xPlaced, oPlaced, turn);
  // If we are not in the first four tiers, then `turn` at this point
  // is undefined. We must look at the tierPosition to determine `turn`.
  if ((*oPlaced) >= 2) { 
    (*turn) = (tierPosition & 1) ? O : X;
    tierPosition >>= 1;
	}

  (*gridPos) = revCenterMapping[tierPosition % numGridPlacements];
  tierPosition /= numGridPlacements;

	POSITION o1, o2;
  int numX = (*xPlaced);
	int numO = (*oPlaced);
  for (int i = boardSize - 1; i >= 0; i--) {
    o1 = (numX > 0) ? combinations[i][numO][numX-1] : 0;
    o2 = o1 + ((numO > 0) ? combinations[i][numO-1][numX] : 0);
    if (tierPosition >= o2) {
      board[i] = BLANK;
      tierPosition -= o2;
    } else if (tierPosition >= o1) {
      if (numO > 0) {
        board[i] = O;
        numO--;
      } else {
        board[i] = BLANK;
      }
      tierPosition -= o1;
    } else {
      if (numX > 0) {
        board[i] = X;
        numX--;
      } else if (numO > 0) {
        board[i] = O;
        numO--;
      } else {
        board[i] = BLANK;
      }
    }
  }
}

MOVE hashMove(BOOLEAN isGridMove, int from, int to) {
  return (((isGridMove) ? 1 : 0) << 10) | (from << 5) | to;
}

void unhashMove(MOVE move, BOOLEAN *isGridMove, int *from, int *to) {
  (*isGridMove) = ((move >> 10) > 0) ? TRUE : FALSE;
  (*from) = (move >> 5) & 0x1F;
  (*to) = move & 0x1F;
}

/**
 * @brief Return the child position reached when the input move
 * is made from the input position.
 * 
 * @param position : The old position
 * @param move     : The move to apply.
 * 
 * @return The child position, encoded as a 64-bit integer. See
 * the POSITION typedef in src/core/types.h.
 */
POSITION DoMove(POSITION position, MOVE move) {
  char turn;
  int xPlaced, oPlaced, gridPos;
  char board[boardSize];
  unhash(position, board, &xPlaced, &oPlaced, &gridPos, &turn);
  BOOLEAN isGridMove;
  int from, to;
  unhashMove(move, &isGridMove, &from, &to);

  if (isGridMove) { // Moving the grid
    gridPos = to;
  } else if (from == to) { // Placement of X or O
    if (turn == X) {
      xPlaced++;
    } else {
      oPlaced++;
    }
    board[to] = turn;
  } else { // Sliding an X or O
    board[from] = BLANK;
    board[to] = turn;
  }

  return hash(board, xPlaced, oPlaced, gridPos, (turn == X) ? O : X);
}

/**
 * @brief Return win, lose, or tie if the input position
 * is primitive; otherwise return undecided.
 * 
 * @param position : The position to inspect.
 * 
 * @return an enum; one of (win, lose, tie, undecided). See 
 * src/core/types.h for the value enum definition.
 */
VALUE Primitive(POSITION position) {
  char turn;
  int xPlaced, oPlaced, gridPos;
  char board[boardSize];
  unhash(position, board, &xPlaced, &oPlaced, &gridPos, &turn);

  BOOLEAN x3inARow = FALSE;
  BOOLEAN o3inARow = FALSE;
  int cmIdx = centerMapping[gridPos];
  for (int i = 0; i < 8 && !(x3inARow && o3inARow); i++) {
    char first = board[allTheRows[cmIdx][i][0]];
    if (first == board[allTheRows[cmIdx][i][1]] && first == board[allTheRows[cmIdx][i][2]]) {
      if (first == X) {
        x3inARow = TRUE;
      } else if (first == O) {
        o3inARow = TRUE;
      }
    }
  }
  if (x3inARow && o3inARow) {
    return tie;
  } else if (x3inARow) {
    return (turn == X) ? win : lose;
  } else if (o3inARow) {
    return (turn == O) ? win : lose;
  } else {
    return undecided;
  }
}

/**
 * @brief Return a list of the legal moves from the input position.
 * 
 * @param position The position to branch off of.
 * 
 * @return The head of a linked list of the legal moves from the input position.
 * 
 * @note 
 * If there has been fewer than 4 pieces:
 *     We can 1) place pieces in empty locations within the grid
 * If there has been at least 4 pieces:
 *     We can 1) place pieces in empty locations within the grid
 *            2) move the grid
 *            3) place new pieces on empty locations within the grid
 */
MOVELIST *GenerateMoves(POSITION position) {
  MOVELIST *moves = NULL;
  if (Primitive(position) == undecided) {
    char turn;
    int xPlaced, oPlaced, gridPos;
    char board[boardSize];
    unhash(position, board, &xPlaced, &oPlaced, &gridPos, &turn);
    int cmIdx = centerMapping[gridPos];
    
    if ((turn == X && (numPiecesPerPlayer - xPlaced)) || (turn == O && (numPiecesPerPlayer - oPlaced))) {
      for (int i = 0; i < 9; i++) { // Placement moves
        int to = gridSlots[cmIdx][i];
        if (board[to] == BLANK) {
          moves = CreateMovelistNode(hashMove(FALSE, to, to), moves);
        }
      }
    }

    if (oPlaced >= 2) { // Grid and sliding moves
      for (int from = 0; from < boardSize; from++) {
        if (board[from] == turn) {
          for (int j = 0; j < 9; j++) { // Sliding Moves
            int to = gridSlots[cmIdx][j];
            if (board[to] == BLANK) {
              moves = CreateMovelistNode(hashMove(FALSE, from, to), moves);
            }
          }
        }
      }
      for (int i = 0; i < numGridAdjacencies[cmIdx]; i++) { // Grid Moves
        moves = CreateMovelistNode(hashMove(TRUE, gridPos, gridAdjacencies[cmIdx][i]), moves);
      }
    }
  }

  return moves;
}

POSITION DoSymmetry(int symmetry, char *originalBoard, int xPlaced, int oPlaced, int gridPos, char turn) {

	char symBoard[boardSize];
  int symGridPos;

  for (int i = 0; i < boardSize; i++) {
    symBoard[gSymmetryMatrix[symmetry][i]] = originalBoard[i];
  }
  symGridPos = gSymmetryMatrix[symmetry][gridPos];

  return hash(symBoard, xPlaced, oPlaced, symGridPos, turn);
}

POSITION GetCanonicalPositionTest(POSITION position, POSITION *symmetricTo) {
  char turn;
	int xPlaced, oPlaced, gridPos;
  char originalBoard[boardSize];
	unhash(position, originalBoard, &xPlaced, &oPlaced, &gridPos, &turn);
  POSITION canonPos = position;
  for (int i = 0; i < 8; i++) {
    POSITION symPos = DoSymmetry(i, originalBoard, xPlaced, oPlaced, gridPos, turn);
    symmetricTo[i] = symPos;
    if (symPos < canonPos) canonPos = symPos;
  }
  if (xPlaced >= 2 && xPlaced == oPlaced) {
    for (int i = 0; i < 25; i++) {
      if (originalBoard[i] != BLANK) {
        originalBoard[i] = (originalBoard[i] == X) ? O : X;
      }
    }
    turn = (turn == X) ? O : X;
    for (int i = 0; i < 8; i++) {
      POSITION symPos = DoSymmetry(i, originalBoard, xPlaced, oPlaced, gridPos, turn);
      symmetricTo[i + 8] = symPos;
      if (symPos < canonPos) canonPos = symPos;
    }
  }
  return canonPos;
}

/**
 * @brief Given a position P, GetCanonicalPosition(P) shall return a 
 * position Q such that P is symmetric to Q and 
 * GetCanonicalPosition(Q) also returns Q.
 * 
 * @note This Q is the "canonical position". For the choice of the 
 * canonical position, we use the lowest-hash-value position 
 * symmetric to the input position.
 * 
 * Ideally, we maximize the number of positions P such that
 * GetCanonicalPosition(P) != P.
 * 
 * @param position : The position to inspect.
 */
POSITION GetCanonicalPosition(POSITION position) {
  char turn;
	int xPlaced, oPlaced, gridPos;
  char originalBoard[boardSize];
	unhash(position, originalBoard, &xPlaced, &oPlaced, &gridPos, &turn);
  POSITION canonPos = position;
  for (int i = 0; i < 8; i++) {
    POSITION symPos = DoSymmetry(i, originalBoard, xPlaced, oPlaced, gridPos, turn);
    if (symPos < canonPos) canonPos = symPos;
  }
  if (xPlaced >= 2 && xPlaced == oPlaced) {
    for (int i = 0; i < 25; i++) {
      if (originalBoard[i] != BLANK) {
        originalBoard[i] = (originalBoard[i] == X) ? O : X;
      }
    }
    turn = (turn == X) ? O : X;
    for (int i = 0; i < 8; i++) {
      POSITION symPos = DoSymmetry(i, originalBoard, xPlaced, oPlaced, gridPos, turn);
      if (symPos < canonPos) canonPos = symPos;
    }
  }
  return canonPos;
}

/*********** BEGIN TIER/UNDOMOVE FUNCTIONS ***********/

/**
 * @brief Given the tier, indicate how many X's and O's
 * have been placed and store them in xPlaced and oPlaced.
 * Also, if fewer than two O's have been placed so far, i.e.,
 * we are in the first four tiers, where each player must place
 * at least two of their own pieces, we can determine whose turn
 * it is from the tier alone. Otherwise, if we are not in the
 * first four tiers, then we will need more information.
 */
void unhashTier(TIER tier, int *xPlaced, int *oPlaced, char *turn) {
	(*xPlaced) = (tier / 10) % 10;
	(*oPlaced) = tier % 10;
  (*turn) = X;
  if ((*xPlaced) <= 2 && (*oPlaced) == (*xPlaced) - 1) {
    (*turn) = O;
  }
}

TIER hashTier(int xPlaced, int oPlaced) {
	return xPlaced * 10 + oPlaced;
}

/**
 * @brief Return the head of a linked list of child tiers 
 * of this tier.
 * If making a move from some position in this tier can
 * lead to another position in this tier, then include
 * this tier in the linked list as well.
 */
TIERLIST *getTierChildren(TIER tier) {
  TIERLIST *tierChildren = NULL;
  int xPlaced = tier / 10;
  int oPlaced = tier % 10;
  if (oPlaced < 2) {
    if (xPlaced == oPlaced) {
      tierChildren = CreateTierlistNode(hashTier(xPlaced + 1, oPlaced), tierChildren);
    } else {
      tierChildren = CreateTierlistNode(hashTier(xPlaced, oPlaced + 1), tierChildren);
    }
  } else {
    tierChildren = CreateTierlistNode(tier, tierChildren);
    if (xPlaced < 4) {
      tierChildren = CreateTierlistNode(hashTier(xPlaced + 1, oPlaced), tierChildren);
    }
    if (oPlaced < 4) {
      tierChildren = CreateTierlistNode(hashTier(xPlaced, oPlaced + 1), tierChildren);
    }
  }
  return tierChildren;
}

/**
 * @brief An upper bound on the number of positions in the
 * input tier.
 * 
 * @note Every position within this tier must
 * hash to a value (tierposition) smaller than the number
 * returned by this function.
 */
TIERPOSITION NumberOfTierPositions(TIER tier) {
	int xPlaced, oPlaced;
  char turn;
	unhashTier(tier, &xPlaced, &oPlaced, &turn);
  return (numGridPlacements * combinations[boardSize][oPlaced][xPlaced]) << (oPlaced >= 2 ? 1 : 0);
}

/**
 * @brief Return the string representation of the input
 * tier. This allocates heap space, so be sure to
 * free the output of this function.
 */
STRING tierToString(TIER tier) {
	STRING tierStr = (STRING) SafeMalloc(sizeof(char) * 40);
	int xPlaced, oPlaced;
	char turn;
	unhashTier(tier, &xPlaced, &oPlaced, &turn);
  sprintf(tierStr, "%d X on board, %d O on board", xPlaced, oPlaced);
	return tierStr;
}

/**
 * @brief Return a linked list of all possible moves from parent
 * positions in the input tier that could have been made in order to arrive
 * at the input position.
 * 
 * @note This allocates heap space, so be sure to free the returned
 * linked list.
 */
UNDOMOVELIST *GenerateUndoMovesToTier(POSITION position, TIER tier) {
	UNDOMOVELIST *undoMoves = NULL;
	int xPlaced, oPlaced, gridPos;
	char turn;
  char board[boardSize];
	unhash(position, board, &xPlaced, &oPlaced, &gridPos, &turn);
	char oppTurn = (turn == X) ? O : X;
  int cmIdx = centerMapping[gridPos];

	int toXPlaced, toOPlaced;
	char toTurn;
	unhashTier(tier, &toXPlaced, &toOPlaced, &toTurn);

  if (xPlaced == toXPlaced && oPlaced == toOPlaced) { // Slide or gridmove
    for (int from = 0; from < boardSize; from++) {
      if (board[from] == BLANK) {
        for (int j = 0; j < 9; j++) { // Sliding Moves
          int to = gridSlots[cmIdx][j];
          if (board[to] == oppTurn)
            undoMoves = CreateUndoMovelistNode(hashMove(FALSE, from, to), undoMoves);
        }
      }
    }
    for (int i = 0; i < numGridAdjacencies[cmIdx]; i++) { // Grid Moves
      undoMoves = CreateUndoMovelistNode(hashMove(TRUE, gridAdjacencies[cmIdx][i], gridPos), undoMoves);
    }
  } else if ((toXPlaced == xPlaced - 1 && oppTurn == X) || (toOPlaced == oPlaced - 1 && oppTurn == O)) { // xPlaced
    for (int i = 0; i < 9; i++) { // Placement moves
      int to = gridSlots[cmIdx][i];
      if (board[to] == oppTurn)
        undoMoves = CreateUndoMovelistNode(hashMove(FALSE, to, to), undoMoves);
    }
  }

	return undoMoves;
}

/**
 * @brief Return the parent position given the undoMove.
 */
POSITION UnDoMove(POSITION position, UNDOMOVE undoMove) {
	int xPlaced, oPlaced, gridPos;
	char turn;
  char board[boardSize];
	unhash(position, board, &xPlaced, &oPlaced, &gridPos, &turn);
	char oppTurn = (turn == X) ? O : X;
	int from, to;
  BOOLEAN isGridMove;
	unhashMove((MOVE) undoMove, &isGridMove, &from, &to);

	if (isGridMove) {
    gridPos = from;
  } else {
    board[to] = BLANK;
    if (from == to) {
      if (oppTurn == X) {
        xPlaced--;
      } else {
        oPlaced--;
      }
    } else {
      board[from] = oppTurn;
    }
  }

	return hash(board, xPlaced, oPlaced, gridPos, oppTurn);
}

/*********** END TIER/UNDOMOVE FUNCTIONS ***********/

/*********** BEGIN TEXTUI FUNCTIONS ***********/

/**
 * @brief Print the position in a pretty format, including the
 * prediction of the game's outcome.
 * 
 * @param position   : The position to pretty-print.
 * @param playerName : The name of the player.
 * @param usersTurn  : TRUE <==> it's a user's turn.
 * 
 * @note See GetPrediction() in src/core/gameplay.h to see how
 * to print the prediction of the game's outcome.
 */
void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
  int xPlaced, oPlaced, gridPos;
  char turn;
  char board[boardSize];
  unhash(position, board, &xPlaced, &oPlaced, &gridPos, &turn);

  // gridX and gridY tell you where the index of the center of the grid is
  int gridx = gridPos % sideLength;
  int gridy = gridPos / sideLength;
  printf("\n");
  for (int i = 0; i < sideLength; i++) {
    // Print two rows for each row in the original board to encode for 
    // pieces (row 1) and inner grid (row 2) 
    printf("  |");
    for (int j = 0; j < sideLength; j++) {
      printf("  "); 
      int index = j + i * sideLength;
      if (board[index] == X) {
        printf("X |");
      } else if (board[index] == O) {
        printf("O |");
      } else {
        printf("  |"); 
      }
    }
    if (i == 1) {
      printf("   %d X Left to Place", numPiecesPerPlayer - xPlaced);
    } else if (i == 2) {
      printf("   Turn: %c", turn);
    } else if (i == 3) {
      printf("   %s", GetPrediction(position, playerName, usersTurn));
    }
    printf("\n");
    printf("  |");
    for (int j = 0; j < sideLength; j++) {
      printf("__"); 
      if ((i >= gridy - 1 && i <= gridy + 1) && (j >= gridx - 1 && j <= gridx + 1)) {
        printf("#_|");
      } else {
        printf("__|");
      }
    }
    if (i == 1) {
      printf("   %d O Left to Place", numPiecesPerPlayer - oPlaced);
    }
    printf("\n");
  }
  printf("\n");
}

/**
 * @brief Find out if the player wants to undo, abort, or neither.
 * If so, return Undo or Abort and don't change `move`.
 * Otherwise, get the new `move` and fill the pointer up.
 * 
 * @param position The position the user is at.
 * @param move The move to fill with user's move.
 * @param playerName The name of the player whose turn it is
 * 
 * @return One of (Undo, Abort, Continue)
 */
USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE *theMove, STRING playerName) {
  USERINPUT input;
	for (;;) {
		printf("%8s's move:  ", playerName);
		input = HandleDefaultTextInput(thePosition, theMove, playerName);
		if (input != Continue) return input;
	}
  return Continue;
}

/**
 * @brief Given a move that the user typed while playing a game in the
 * TextUI, return TRUE if the input move string is of the right "form"
 * and can be converted to a move hash.
 * 
 * @param input : The string input the user typed.
 * 
 * @return TRUE iff the input is a valid text input.
 */
BOOLEAN ValidTextInput(STRING input) {
  return (input[0] == 'A' || input[0] == 'G' || input[0] == 'M') && input[1] == '-';
}

/**
 * @brief Convert the string input to the internal move representation.
 * 
 * @param input This is a user-inputted move string already validated
 * by ValidTextInput().
 * 
 * @return The hash of the move specified by the text input.
 */
MOVE ConvertTextInputToMove(STRING input) {
  char type = input[0]; 
  int source; 
  int dest; 
  MOVE ret = 0;
  if (type == 'A') {
    if (input[3] != 0) {
      dest = (input[2] - '0') * 10 + (input[3] - '0');
    } else {
      dest = input[2] - '0'; 
    }
    ret = hashMove(FALSE, dest - 1, dest - 1);
  } else if (type == 'M' || type == 'G') {
    // Get source
    if (input[3] != '-') {
      source = (input[2] - '0') * 10 + (input[3] - '0');
      // Get dest 
      if (input[6] != 0) {
        dest = (input[5] - '0') * 10 + (input[6] - '0');
      } else {
        dest = input[5] - '0'; 
      }
    } else {
      source = input[2] - '0'; 
      // Get dest 
      if (input[5] != 0) {
        dest = (input[4] - '0') * 10 + (input[5] - '0');
      } else {
        dest = input[4] - '0'; 
      }
    }
    if (type == 'M') {
      ret = hashMove(FALSE, source - 1, dest - 1);
    } else {
      ret = hashMove(TRUE, source - 1, dest - 1);
    }
  } else {
    printf("You should not be here. Something went wrong."); 
  }
  return ret; 
}

/**
 * @brief Write a short human-readable string representation
 * of the move to the input buffer.
 * 
 * @param move The move hash to convert to a move string.
 * 
 * @param moveStringBuffer The buffer to write the move string
 * to.
 * 
 * @note The space available in `moveStringBuffer` is MAX_MOVE_STRING_LENGTH 
 * (see src/core/autoguistrings.h). Do not write past this limit and ensure
 * that the move string written to `moveStringBuffer` is properly 
 * null-terminated.
 * 
 * @note Moves in TTTWO are formatted as A-DEST, M-SRC-DEST, or G-SRC-DEST
 */
void MoveToString(MOVE move, char *moveStringBuffer) {
  BOOLEAN isGridMove;
  int from, to;
  unhashMove(move & 0xFFFF, &isGridMove, &from, &to);
  // 0: placing new pieces
  // message: "A-DEST"
  if (move & 0x80000) { // Move is "choose to move grid" partmove
    sprintf(moveStringBuffer, "G-%d", from + 1);
  } else if (move & 0x40000) { // Move is "select piece to move" partmove
    sprintf(moveStringBuffer, "M-%d", from + 1);
  } else if (move & 0x30000) { 
    // Move is partmove: either select where to move grid or
    // select where to move piece
    sprintf(moveStringBuffer, "%d", to + 1);
  } else { // fullmove
    if (from == to) {
      sprintf(moveStringBuffer, "A-%d", to + 1);
    } else if (!isGridMove) {
      // 1: moving existing pieces
      // message: "M-SRC-DEST"
      sprintf(moveStringBuffer, "M-%d-%d", from + 1, to + 1);
    } else {
      // 2: moving the grid
      // message: "G-SRC-DEST"
      sprintf(moveStringBuffer, "G-%d-%d", from + 1, to + 1); 
    }
  }
}

/**
 * @brief Nicely format the computers move.
 * 
 * @param computersMove : The computer's move.
 * @param computersName : The computer's name.
 */
void PrintComputersMove(MOVE computersMove, STRING computersName) {
  char moveStringBuffer[32];
  MoveToString(computersMove, moveStringBuffer);
  printf("%s's move: %s\n", computersName, moveStringBuffer);
}

/**
 * @brief Menu used to debug internal problems. 
 * Does nothing if kDebugMenu == FALSE.
 */
void DebugMenu(void) {}

/*********** END TEXTUI FUNCTIONS ***********/

/*********** BEGIN VARIANT FUNCTIONS ***********/
int NumberOfOptions() { return 1; }
int getOption() { return 0; }
void setOption(int option) { (void) option; }
void GameSpecificMenu() {}
/*********** END VARIANT-RELATED FUNCTIONS ***********/

POSITION encodeIntermediatePosition(POSITION position, BOOLEAN isGridMove, int from) {
	// 0b1 1 00000 0; intermediate marker (1), isGridMove (1), from (5)
	return position | (1LL << 63) | (((isGridMove) ? 1LL : 0LL) << 62) | (((POSITION) from) << 57); 
}

BOOLEAN decodeIntermediatePosition(POSITION interPos, POSITION *origPos, BOOLEAN *isGridMove, int *from) {
	(*origPos) = interPos & 0xFFFFFFFFFFFFFF;
	(*isGridMove) = ((interPos >> 62) & 1) ? TRUE : FALSE;
	(*from) = (interPos >> 57) & 0x1F;
	return (interPos >> 63) ? TRUE : FALSE;
}

POSITION StringToPosition(char *positionString) {
	int turnInt;
	char *board;
	if (ParseStandardOnelinePositionString(positionString, &turnInt, &board)) {
		char turn = (turnInt == 1) ? X : O;
    int xPlaced = 0, oPlaced = 0;
    int i;
    for (i = 0; i < boardSize; i++) {
      switch (board[i]) {
        case 'X':
          xPlaced++;
          break;
        case 'O':
          oPlaced++;
          break;
        default:
          break;
      }
    }
    int gridPos = 12;
    for (; i < boardSize + numGridPlacements; i++) {
      if (board[i] == 'G') {
        gridPos = revCenterMapping[i - boardSize];
        break;
      }
    }
    gInitializeHashWindow(hashTier(xPlaced, oPlaced), FALSE);
    return hash(board, xPlaced, oPlaced, gridPos, turn);
	}
	return NULL_POSITION;
}

/* boardSize (pieces) + numGridPlacements (where grid is) + 1 ("select grid center" sign + also for multipart) + 2 (multipart) */
void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	POSITION pos;
  BOOLEAN isGridMove;
  int from;
  BOOLEAN isIntermediate = decodeIntermediatePosition(position, &pos, &isGridMove, &from);

  int xPlaced, oPlaced, gridPos;
  char turn;
  char board[boardSize];
  unhash(pos, board, &xPlaced, &oPlaced, &gridPos, &turn);

  int turnInt = (turn == X) ? 1 : 2;

  int entityStringSize = boardSize + numGridPlacements + 2;
  char finalBoard[entityStringSize];
  memset(finalBoard, '-', entityStringSize * sizeof(char));
  memcpy(finalBoard, board, boardSize * sizeof(char));

  int cmIdx = centerMapping[gridPos];
  finalBoard[boardSize + cmIdx] = 'G';

  if (isIntermediate) {
    if (isGridMove) {
      // Show "Choose a grid center" SVG message at bottom
      finalBoard[entityStringSize - 2] = 'T';
    } else {
      // For making the different intermediate positions have
      // different autogui position strings
      // This is needed to distinguish the intermediate positions because
      // We need to show a different set of part-moves from each
      finalBoard[entityStringSize - 2] = 'a' + from; // will not be shown
    }
  }
  finalBoard[entityStringSize - 1] = '\0';
  AutoGUIMakePositionString(turnInt, finalBoard, autoguiPositionStringBuffer);
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
  (void) position;
  int isGridMove, from, to;
  unhashMove(move & 0xFFFF, &isGridMove, &from, &to);
  if (move & 0x80000) { // Move is "choose to move grid";
    AutoGUIMakeMoveButtonStringA('Z', boardSize + numGridPlacements, 'x', autoguiMoveStringBuffer);
  } else if (move & 0x40000) { // Move is "select piece to move";
    AutoGUIMakeMoveButtonStringA('h', from, 'y', autoguiMoveStringBuffer);
  } else if (move & 0x20000) { // Move is "select where to move grid";
    AutoGUIMakeMoveButtonStringA('h', to, 'z', autoguiMoveStringBuffer);
  } else if (move & 0x10000) { // Move is "select where to move piece"
    AutoGUIMakeMoveButtonStringM(from, to, 'z', autoguiMoveStringBuffer);
  } else {
    if (from == to) { // Single-part move
      AutoGUIMakeMoveButtonStringA('h', to, 'x', autoguiMoveStringBuffer);
    } else { // A full-move that is multipart
      AutoGUIWriteEmptyString(autoguiMoveStringBuffer);
    }
  }
}

MULTIPARTEDGELIST* GenerateMultipartMoveEdges(POSITION position, MOVELIST *moveList, POSITIONLIST *positionList) {
	(void) positionList;
  MULTIPARTEDGELIST *mpel = NULL;
  int edgeFromAdded = 0;
  BOOLEAN gridMoveAdded = FALSE;
  POSITION gridMoveInterPos = encodeIntermediatePosition(position, TRUE, 0);
	while (moveList != NULL) {
    BOOLEAN isGridMove;
    int from, to;
    unhashMove(moveList->move, &isGridMove, &from, &to);

    if (isGridMove) {
      if (!gridMoveAdded) {
        // Add "choose to move grid" partMove
        mpel = CreateMultipartEdgeListNode(NULL_POSITION, gridMoveInterPos, 0x80000 | moveList->move, 0, mpel);
        gridMoveAdded = TRUE;
      }
      // Add "choose where to move grid" partMove
      mpel = CreateMultipartEdgeListNode(gridMoveInterPos, NULL_POSITION, 0x20000 | moveList->move, moveList->move, mpel);
    } else if (from != to) {
      POSITION slideMoveInterPos = encodeIntermediatePosition(position, FALSE, from);
      if (!(edgeFromAdded & (1 << from))) {
        // Add "select piece to move" partMove
        mpel = CreateMultipartEdgeListNode(NULL_POSITION, slideMoveInterPos, 0x40000 | moveList->move, 0, mpel);
        edgeFromAdded ^= (1 << from);
      }
      // Add "select where to move the piece" partMove
      mpel = CreateMultipartEdgeListNode(slideMoveInterPos, NULL_POSITION, 0x10000 | moveList->move, moveList->move, mpel);
    }
    // Ignore placement moves, they're single-part

		moveList = moveList->next;
	}
	return mpel;
}