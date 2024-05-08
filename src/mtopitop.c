/************************************************************************
**
** NAME:        mtopitop.c
**
** DESCRIPTION: Topitop
**
** AUTHOR:      (Version 1) Mike Hamada and Alex Choy
**              (Version 2) Matthew Yu and Cameron Cheung
**
** DATE:        February 2022
**
**************************************************************************/

// For instructions on how to play Topitop, see the following link:
// https://github.com/GamesCrafters/Explainers/blob/master/instructions/en/games/topitop.xml

#include "gamesman.h"

/* Defines */
#define BLUE 'B'
#define RED 'R'

#define BLUEBUCKETPIECE 'B'
#define REDBUCKETPIECE 'R'
#define SMALLPIECE 'S'
#define LARGEPIECE 'L'
#define BLUESMALLPIECE 'X'
#define REDSMALLPIECE 'O'
#define CASTLEPIECE 'C'
#define BLUECASTLEPIECE 'P'
#define REDCASTLEPIECE 'Q'
#define BLANKPIECE '-'

#define BLUEDISPLAY "  B  "
#define REDDISPLAY "  R  "
#define SMALLDISPLAY " /_\\ "
#define LARGEDISPLAY "/___\\"
#define BLANKDISPLAY "     "

#define NULLMOVE 0b1111111111

POSITION gNumberOfPositions = 0;
POSITION kBadPosition = -1;
POSITION gInitialPosition = 0;

int gSymmetryMatrix[8][9] = {
    {0,1,2,3,4,5,6,7,8},
    {6,3,0,7,4,1,8,5,2},
    {8,7,6,5,4,3,2,1,0},
    {2,5,8,1,4,7,0,3,6},
    {2,1,0,5,4,3,8,7,6},
    {0,3,6,1,4,7,2,5,8},
    {6,7,8,3,4,5,0,1,2},
    {8,5,2,7,4,1,6,3,0}
};

int numAdjacencies[9] = {3, 5, 3, 5, 8, 5, 3, 5, 3};

int adjacencyMatrix[9][8] = {
	{1, 3, 4, 9, 9, 9, 9, 9},
	{0, 2, 3, 4, 5, 9, 9, 9},
	{1, 4, 5, 9, 9, 9, 9, 9},
	{0, 1, 4, 6, 7, 9, 9, 9},
	{0, 1, 2, 3, 5, 6, 7, 8},
	{1, 2, 4, 7, 8, 9, 9, 9},
	{3, 4, 7, 9, 9, 9, 9, 9},
	{3, 4, 5, 6, 8, 9, 9, 9},
	{4, 5, 7, 9, 9, 9, 9, 9}
};

int movesToIds[9][9] = {
	{0, 1, 100, 2, 3, 100, 100, 100, 100},
	{4, 0, 5, 6, 7, 8, 100, 100, 100},
	{100, 9, 0, 100, 10, 11, 100, 100, 100},
	{12, 13, 100, 0, 14, 100, 15, 16, 100},
	{17, 18, 19, 20, 0, 21, 22, 23, 24},
	{100, 25, 26, 100, 27, 0, 100, 28, 29},
	{100, 100, 100, 30, 31, 100, 0, 32, 100},
	{100, 100, 100, 33, 34, 35, 36, 0, 37},
	{100, 100, 100, 100, 38, 39, 100, 40, 0}
};

int idsToMoves[2][41] = {
	{0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8},
	{0, 1, 3, 4, 0, 2, 3, 4, 5, 1, 4, 5, 0, 1, 4, 6, 7, 0, 1, 2, 3, 5, 6, 7, 8, 1, 2, 4, 7, 8, 3, 4, 7, 3, 4, 5, 6, 8, 4, 5, 7}
};

CONST_STRING kAuthorName = "(V1) Mike Hamada, Alex Choy; (V2) Matthew Yu, Cameron Cheung";
CONST_STRING kGameName = "Topitop";
CONST_STRING kDBName = "topitop";   /* The name to store the database under */
BOOLEAN kPartizan = TRUE;
BOOLEAN kDebugMenu = FALSE;
BOOLEAN kGameSpecificMenu = TRUE;
BOOLEAN kTieIsPossible = FALSE;
BOOLEAN kLoopy = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
BOOLEAN kSupportsSymmetries = TRUE; /* Whether we support symmetries */
void* gGameSpecificTclInit = NULL;

BOOLEAN tyingRule = FALSE;
VALUE ifNoLegalMoves = undecided;
BOOLEAN isMisere = FALSE;

CONST_STRING kHelpOnYourTurn = "";
CONST_STRING kHelpStandardObjective = "";
CONST_STRING kHelpGraphicInterface = "";
CONST_STRING kHelpTextInterface = "";
CONST_STRING kHelpReverseObjective = "";
CONST_STRING kHelpTieOccursWhen = "A tie never occurs.";
CONST_STRING kHelpExample = "";

void unhashPosition(POSITION position, char *board, char *turn, int *disallowedMove, int *blueLeft, int *redLeft, int *smallLeft, int *largeLeft);
POSITION hashPosition(char* board, char turn, int disallowedMove);
void unhashMove(MOVE move, char *piece, int *from, int *to, BOOLEAN *p2Turn);
MOVE hashMove(char piece, int from, int to, BOOLEAN p2Turn);
char* PrintHelper(char piece, int level);
void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn);
POSITION GetCanonicalPosition(POSITION position);
void PositionToString(POSITION position, char *positionStringBuffer);

void countPiecesOnBoard(char *board, int *bb, int *rb, int *bs, int *rs, int *bc, int *rc, int *s, int *l, int *c);
void unhashTier(TIER tier, int *bb, int *rb, int *bs, int *rs, int *bc, int *rc, int *s, int *l, int *c);
TIER hashTier(TIER bb, TIER rb, TIER bs, TIER rs, TIER bc, TIER rc, TIER s, TIER l, TIER c);
TIERLIST* gTierChildren(TIER tier);
TIERPOSITION gNumberOfTierPositions(TIER tier);
STRING TierToString(TIER tier);
POSITION fact(int n);
TIERPOSITION numRearrangements(int length, int numBlanks, int bb, int rb, int bs, int rs, int bc, int rc, int s, int l, int c);

void InitializeGame() {

	/* FOR THE PURPOSES OF INTERACT. FEEL FREE TO CHANGE IF SOLVING. */ 
	if (gIsInteract) {
		gLoadTierdbArray = FALSE; // SET TO TRUE IF SOLVING
	}
	/********************************/
	
	gCanonicalPosition = GetCanonicalPosition;
	gPositionToStringFunPtr = &PositionToString;
	kSupportsTierGamesman = TRUE;
	kExclusivelyTierGamesman = TRUE;
	gInitialTier = 0;
	gInitialTierPosition = 0;
	gTierChildrenFunPtr = &gTierChildren;
	gNumberOfTierPositionsFunPtr = &gNumberOfTierPositions;
	gTierToStringFunPtr = &TierToString;
}

/////

void DebugMenu() {}

POSITION fact(int n) {
    if (n <= 1) return 1;
	POSITION prod = 1;
	for (int i = 2; i <= n; i++)
		prod *= i;
    return prod;
}

TIERPOSITION numRearrangements(int length, int numBlanks, int bb, int rb, int bs, int rs, int bc, int rc, int s, int l, int c) {
	if (numBlanks < 0 || bb < 0 || rb < 0 || bs < 0 || rs < 0 || bc < 0 || rc < 0 || s < 0 || l < 0 || c < 0) return 0;
	if (length <= 1) return 1;
	return fact(length) / (fact(numBlanks) * fact(bb) * fact(rb) * fact(bs) * fact(rs) * fact(bc) * fact(rc) * fact(s) * fact(l) * fact(c));
}

void unhashPosition(POSITION position, char *board, char *turn, int *disallowedMove, int *blueLeft, int *redLeft, int *smallLeft, int *largeLeft) {
	TIER tier; TIERPOSITION tierPosition;
	gUnhashToTierPosition(position, &tierPosition, &tier);

	int bb, rb, bs, rs, bc, rc, s, l, c;
	unhashTier(tier, &bb, &rb, &bs, &rs, &bc, &rc, &s, &l, &c);
	int numBlanks = 9 - bb - rb - bs - rs - bc - rc - s - l - c;

	(*disallowedMove) = tierPosition % 41;
	TIERPOSITION half = gNumberOfTierPositions(tier) >> 1;
	(*turn) = (tierPosition >= half) ? RED : BLUE;
	(*blueLeft) = 2 - bb - bs - bc;
	(*redLeft) = 2 - rb - rs - rc;
	(*smallLeft) = 4 - bs - rs - bc - rc - s - c;
	(*largeLeft) = 4 - bc - rc - l - c;

	tierPosition -= (tierPosition % 41);
	if (tierPosition >= half) {
		tierPosition -= half;
	}
	tierPosition /= 41;

	TIERPOSITION idxOfFirstBLUEBUCKETPIECE, idxOfFirstREDBUCKETPIECE, idxOfFirstSMALLPIECE, idxOfFirstLARGEPIECE, idxOfFirstBLUESMALLPIECE, idxOfFirstREDSMALLPIECE, idxOfFirstCASTLEPIECE, idxOfFirstBLUECASTLEPIECE, idxOfFirstREDCASTLEPIECE;
	for (int i = 0; i < 9; i++) {
		TIERPOSITION lastIdx = 0;
		
		if (numBlanks > 0) {
			lastIdx += numRearrangements(8 - i, numBlanks - 1, bb, rb, bs, rs, bc, rc, s, l, c);
		}

		if (bb > 0) {
			idxOfFirstBLUEBUCKETPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb - 1, rb, bs, rs, bc, rc, s, l, c);
		} else {
			idxOfFirstBLUEBUCKETPIECE = -1;
		}

		if (rb > 0) {
			idxOfFirstREDBUCKETPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb, rb - 1, bs, rs, bc, rc, s, l, c);
		} else {
			idxOfFirstREDBUCKETPIECE = -1;
		}

		if (s > 0) {
			idxOfFirstSMALLPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc, rc, s - 1, l, c);
		} else {
			idxOfFirstSMALLPIECE = -1;
		}

		if (l > 0) {
			idxOfFirstLARGEPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc, rc, s, l - 1, c);
		} else {
			idxOfFirstLARGEPIECE = -1;
		}

		if (bs > 0) {
			idxOfFirstBLUESMALLPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb, rb, bs - 1, rs, bc, rc, s, l, c);
		} else {
			idxOfFirstBLUESMALLPIECE = -1;
		}

		if (rs > 0) {
			idxOfFirstREDSMALLPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs - 1, bc, rc, s, l, c);
		} else {
			idxOfFirstREDSMALLPIECE = -1;
		}

		if (c > 0) {
			idxOfFirstCASTLEPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc, rc, s, l, c - 1);
		} else {
			idxOfFirstCASTLEPIECE = -1;
		}

		if (bc > 0) {
			idxOfFirstBLUECASTLEPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc - 1, rc, s, l, c);
		} else {
			idxOfFirstBLUECASTLEPIECE = -1;
		}

		if (rc > 0) {
			idxOfFirstREDCASTLEPIECE = lastIdx;
			lastIdx += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc, rc - 1, s, l, c);
		} else {
			idxOfFirstREDCASTLEPIECE = -1;
		}

		if (tierPosition >= idxOfFirstREDCASTLEPIECE) {
			board[i] = REDCASTLEPIECE;
			rc--;
			tierPosition -= idxOfFirstREDCASTLEPIECE;
		} else if (tierPosition >= idxOfFirstBLUECASTLEPIECE) {
			board[i] = BLUECASTLEPIECE;
			bc--;
			tierPosition -= idxOfFirstBLUECASTLEPIECE;
		} else if (tierPosition >= idxOfFirstCASTLEPIECE) {
			board[i] = CASTLEPIECE;
			c--;
			tierPosition -= idxOfFirstCASTLEPIECE;
		} else if (tierPosition >= idxOfFirstREDSMALLPIECE) {
			board[i] = REDSMALLPIECE;
			rs--;
			tierPosition -= idxOfFirstREDSMALLPIECE;
		} else if (tierPosition >= idxOfFirstBLUESMALLPIECE) {
			board[i] = BLUESMALLPIECE;
			bs--;
			tierPosition -= idxOfFirstBLUESMALLPIECE;
		} else if (tierPosition >= idxOfFirstLARGEPIECE) {
			board[i] = LARGEPIECE;
			l--;
			tierPosition -= idxOfFirstLARGEPIECE;
		} else if (tierPosition >= idxOfFirstSMALLPIECE) {
			board[i] = SMALLPIECE;
			s--;
			tierPosition -= idxOfFirstSMALLPIECE;
		} else if (tierPosition >= idxOfFirstREDBUCKETPIECE) {
			board[i] = REDBUCKETPIECE;
			rb--;
			tierPosition -= idxOfFirstREDBUCKETPIECE;
		} else if (tierPosition >= idxOfFirstBLUEBUCKETPIECE) {
			board[i] = BLUEBUCKETPIECE;
			bb--;
			tierPosition -= idxOfFirstBLUEBUCKETPIECE;
		} else {
			board[i] = BLANKPIECE;
			numBlanks--;
		}
	}	
}

POSITION hashPosition(char* board, char turn, int disallowedMove) {
	int bb, rb, bs, rs, bc, rc, s, l, c;
	countPiecesOnBoard(board, &bb, &rb, &bs, &rs, &bc, &rc, &s, &l, &c);
	TIER tier = hashTier(bb, rb, bs, rs, bc, rc, s, l, c);
	TIERPOSITION tierPosition = 0;
	int numBlanks = 9 - bb - rb - bs - rs - bc - rc - s - l - c;
	for (int i = 0; i < 9; i++) {
		switch(board[i]) {
			case REDCASTLEPIECE:
				tierPosition += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc - 1, rc, s, l, c);
  				// fall through
			case BLUECASTLEPIECE:
				tierPosition += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc, rc, s, l, c - 1);
  				// fall through
			case CASTLEPIECE:
				tierPosition += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs - 1, bc, rc, s, l, c);
  				// fall through
			case REDSMALLPIECE:
				tierPosition += numRearrangements(8 - i, numBlanks, bb, rb, bs - 1, rs, bc, rc, s, l, c);
  				// fall through
			case BLUESMALLPIECE:
				tierPosition += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc, rc, s, l - 1, c);
  				// fall through
			case LARGEPIECE:
				tierPosition += numRearrangements(8 - i, numBlanks, bb, rb, bs, rs, bc, rc, s - 1, l, c);
  				// fall through
			case SMALLPIECE:
				tierPosition += numRearrangements(8 - i, numBlanks, bb, rb - 1, bs, rs, bc, rc, s, l, c);
  				// fall through
			case REDBUCKETPIECE:
				tierPosition += numRearrangements(8 - i, numBlanks, bb - 1, rb, bs, rs, bc, rc, s, l, c);
  				// fall through
			case BLUEBUCKETPIECE:
				tierPosition += numRearrangements(8 - i, numBlanks - 1, bb, rb, bs, rs, bc, rc, s, l, c);
  				// fall through
			default:
				break; 
		}
		switch(board[i]) {
			case REDCASTLEPIECE:
				rc--;
				break;
			case BLUECASTLEPIECE:
				bc--;
				break;
			case CASTLEPIECE:
				c--;
				break;
			case REDSMALLPIECE:
				rs--;
				break;
			case BLUESMALLPIECE:
				bs--;
				break;
			case LARGEPIECE:
				l--;
				break;
			case SMALLPIECE:
				s--;
				break;
			case REDBUCKETPIECE:
				rb--;
				break;
			case BLUEBUCKETPIECE:
				bb--;
				break;
			default:
				numBlanks--;
				break; 
		}
	}

	tierPosition *= 41;
	tierPosition += ((turn == RED) ? (gNumberOfTierPositions(tier) >> 1) : 0);
	tierPosition += disallowedMove;
	return gHashToWindowPosition(tierPosition, tier);
}

void countPiecesOnBoard(char *board, int *bb, int *rb, int *bs, int *rs, int *bc, int *rc, int *s, int *l, int *c) {
	*bb = *rb = *bs = *rs = *bc = *rc = *s = *l = *c = 0;
	for (int i = 0; i < 9; i++) {
		switch(board[i]) {
			case BLUEBUCKETPIECE:
				(*bb)++;
				break;
			case REDBUCKETPIECE:
				(*rb)++;
				break;
			case BLUESMALLPIECE:
				(*bs)++;
				break;
			case REDSMALLPIECE:
				(*rs)++;
				break;
			case BLUECASTLEPIECE:
				(*bc)++;
				break;
			case REDCASTLEPIECE:
				(*rc)++;
				break;
			case SMALLPIECE:
				(*s)++;
				break;
			case LARGEPIECE:
				(*l)++;
				break;
			case CASTLEPIECE:
				(*c)++;
				break;
			default:
				break;
		}
	}
}

void unhashTier(TIER tier, int *bb, int *rb, int *bs, int *rs, int *bc, int *rc, int *s, int *l, int *c) {
	int mask = 0b1111;
	*bb = tier & mask;
	tier >>= 4;
	*rb = tier & mask;
	tier >>= 4;
	*bs = tier & mask;
	tier >>= 4;
	*rs = tier & mask;
	tier >>= 4;
	*bc = tier & mask;
	tier >>= 4;
	*rc = tier & mask;
	tier >>= 4;
	*s = tier & mask;
	tier >>= 4;
	*l = tier & mask;
	tier >>= 4;
	*c = tier & mask;
}

TIER hashTier(TIER bb, TIER rb, TIER bs, TIER rs, TIER bc, TIER rc, TIER s, TIER l, TIER c) {
	return bb | (rb << 4) | (bs << 8) | (rs << 12) | (bc << 16) | (rc << 20) | (s << 24) | (l << 28) | (c << 32);
}

TIERLIST* gTierChildren(TIER tier) {
	int bb, rb, bs, rs, bc, rc, s, l, c;
	unhashTier(tier, &bb, &rb, &bs, &rs, &bc, &rc, &s, &l, &c);
	int total = bb + rb + bs + rs + bc + rc + s + l + c;

	TIERLIST* list = NULL;
	if (bc == 2 || rc == 2) return list; // Primitive tiers contain two castles of the same color
	if (tier != gInitialTier) list = CreateTierlistNode(tier, list);
	
	// Placement
	if (total < 9) {
		if (bb + bs + bc < 2) list = CreateTierlistNode(hashTier(bb + 1, rb, bs, rs, bc, rc, s, l, c), list);
		if (rb + rs + rc < 2) list = CreateTierlistNode(hashTier(bb, rb + 1, bs, rs, bc, rc, s, l, c), list); // If there are red buckets to place
		if (s + c + rc + bc + rs + bs < 4) list = CreateTierlistNode(hashTier(bb, rb, bs, rs, bc, rc, s + 1, l, c), list); // If there are smalls to place
		if (l + c + rc + bc < 4) list = CreateTierlistNode(hashTier(bb, rb, bs, rs, bc, rc, s, l + 1, c), list); // If there are larges to place
	}

	// Sliding
	if (bb > 0 && s > 0) list = CreateTierlistNode(hashTier(bb - 1, rb, bs + 1, rs, bc, rc, s - 1, l, c), list); // Blue + Small = BlueSmall
	if (rb > 0 && s > 0) list = CreateTierlistNode(hashTier(bb, rb - 1, bs, rs + 1, bc, rc, s - 1, l, c), list); // Red + Small = RedSmall
	if (bs > 0 && l > 0) list = CreateTierlistNode(hashTier(bb, rb, bs - 1, rs, bc + 1, rc, s, l - 1, c), list); // BlueSmall + Large = BlueCastle
	if (rs > 0 && l > 0) list = CreateTierlistNode(hashTier(bb, rb, bs, rs - 1, bc, rc + 1, s, l - 1, c), list); // RedSmall + Large = RedCastle
	if (s > 0 && l > 0) list = CreateTierlistNode(hashTier(bb, rb, bs, rs, bc, rc, s - 1, l - 1, c + 1), list); // Small + Large = Castle
	if (bb > 0 && c > 0) list = CreateTierlistNode(hashTier(bb - 1, rb, bs, rs, bc + 1, rc, s, l, c - 1), list); // Blue + Castle = BlueCastle
	if (rb > 0 && c > 0) list = CreateTierlistNode(hashTier(bb, rb - 1, bs, rs, bc, rc + 1, s, l, c - 1), list); // Red + Castle = RedCastle
	return list;
}

TIERPOSITION gNumberOfTierPositions(TIER tier) {
	int bb, rb, bs, rs, bc, rc, s, l, c;
	unhashTier(tier, &bb, &rb, &bs, &rs, &bc, &rc, &s, &l, &c);
	int numBlanks = 9 - bb - rb - bs - rs - bc - rc - s - l - c;
	// 9! * 41 (Upper bound on # of possible disallowedMoves) * 2 (to account for turn) = 29756160 
	return 29756160 / (fact(numBlanks) * fact(bb) * fact(rb) * fact(bs) * fact(rs) * fact(bc) * fact(rc) * fact(s) * fact(l) * fact(c));
}

STRING TierToString(TIER tier) {
	int bb, rb, bs, rs, bc, rc, s, l, c;
	unhashTier(tier, &bb, &rb, &bs, &rs, &bc, &rc, &s, &l, &c);
	STRING str = (STRING) SafeMalloc(sizeof(char) * 100);
	sprintf(str, "BB: %d, RB: %d, BS: %d, RS: %d, BC: %d, RC: %d, S: %d, L: %d, C: %d", bb, rb, bs, rs, bc, rc, s, l, c);
	return str;
}

void unhashMove(MOVE move, char *piece, int *from, int *to, BOOLEAN *p2Turn) {
	*to = move & 0b1111;
	move >>= 4;
	*from = move & 0b1111;
	move >>= 4;
	switch (move & 0b1111) {
		case 1:
			*piece = REDBUCKETPIECE;
			break;
		case 2:
			*piece = SMALLPIECE;
			break;
		case 3:
			*piece = LARGEPIECE;
			break;
		default:
			*piece = BLUEBUCKETPIECE;
			break;
	}
	move >>= 4;
	*p2Turn = move ? TRUE : FALSE;
}

MOVE hashMove(char piece, int from, int to, BOOLEAN p2Turn) {
	MOVE theHash = 0;
	theHash |= to;
	theHash |= from << 4;
	switch (piece) {
		case REDBUCKETPIECE:
			theHash |= 1 << 8;
			break;
		case SMALLPIECE:
			theHash |= 2 << 8;
			break;
		case LARGEPIECE:
			theHash |= 3 << 8;
			break;
		default:
			break;
	}
	if (p2Turn) {
		theHash |= 1 << 12;
	}
	return theHash;
}

void GameSpecificMenu() {
	do {
		printf("\n\t----- Game-specific options for %s -----\n\n\n", kGameName);

		if (tyingRule == FALSE && ifNoLegalMoves == undecided && isMisere == FALSE) {
			printf("\tCurrently using default (Variant 0) rules.\n");
		} else {
			printf("\tCurrently using rules for Variant %d.\n", getOption());
		}

		printf("\n\ts)\tIf no legal moves: change from %s to %s.\n", (ifNoLegalMoves == undecided) ? "PASS TURN" : "IMMEDIATE LOSS", (ifNoLegalMoves == undecided) ? "IMMEDIATE LOSS" : "PASS TURN");
		printf("\n\tt)\tTwo-tower tying rule: change from %s to %s.\n", (tyingRule) ? "ON" : "OFF", (tyingRule) ? "OFF" : "ON");
		printf("\n\tm)\tChange from %sMISERE to %sMISERE.\n", (isMisere) ? "" : "NON-", (isMisere) ? "NON-" : "");
		printf("\n\tb)\tBack = Return to previous activity.\n");
		printf("\n\nSelect an option: ");

		switch(GetMyChar()) {
		case 'S': case 's':
			ifNoLegalMoves = (ifNoLegalMoves == undecided) ? lose : undecided;
			break;
		case 'T': case 't':
			tyingRule = !tyingRule;
			break;
		case 'M': case 'm':
			isMisere = !isMisere;
			break;
		case 'B': case 'b': case 'Q': case 'q':
			return;
		default:
			printf("\nSorry, I don't know that option. Try another.\n");
			HitAnyKeyToContinue();
			break;
		}
	} while(TRUE);
}

int NumberOfOptions() { return 8; }

int getOption() {
	return (isMisere << 2) | (tyingRule << 1) | (ifNoLegalMoves == lose);
}

void setOption(int option) {
	ifNoLegalMoves = (option & 0b1) ? lose : undecided;
	tyingRule = (option & 0b10) ? TRUE : FALSE;
	isMisere = (option & 0b100) ? TRUE : FALSE;
}

void SetTclCGameSpecificOptions(int theOptions[]) { (void) theOptions; }

POSITION DoMove(POSITION position, MOVE move) {
	char turn;
	int disallowedMove, blueLeft, redLeft, smallLeft, largeLeft;
	char board[9];
	unhashPosition(position, board, &turn, &disallowedMove, &blueLeft, &redLeft, &smallLeft, &largeLeft);

	if (move == NULLMOVE) {
		return hashPosition(board, (turn == BLUE) ? RED : BLUE, 0);
	}
	
	char piece;
	int from, to;
	BOOLEAN p2Turn;
	unhashMove(move, &piece, &from, &to, &p2Turn);
	if (from == to) { // Placement
		board[to] = piece;
		switch (piece) {
			case BLUEBUCKETPIECE:
				blueLeft--;
				break;
			case REDBUCKETPIECE:
				redLeft--;
				break;
			case SMALLPIECE:
				smallLeft--;
				break;
			case LARGEPIECE:
				largeLeft--;
				break;
			default:
				break;
		}
		disallowedMove = 0;
	} else { // SLIDING
		switch (board[to]) {
			case BLANKPIECE:
				if (board[from] == SMALLPIECE || board[from] == LARGEPIECE || board[from] == CASTLEPIECE) {
					disallowedMove = movesToIds[to][from];  // A reversal of this slide is forbidden.
				} else {
					disallowedMove = 0;
				}
				board[to] = board[from];
				break;
			case SMALLPIECE:
				if (board[from] == BLUEBUCKETPIECE) {
					board[to] = BLUESMALLPIECE;
				} else {
					board[to] = REDSMALLPIECE;
				}
				disallowedMove = 0;
				break;
			case LARGEPIECE:
				if (board[from] == SMALLPIECE) {
					board[to] = CASTLEPIECE;
				} else if (board[from] == BLUESMALLPIECE) {
					board[to] = BLUECASTLEPIECE;
				} else {
					board[to] = REDCASTLEPIECE;
				}
				disallowedMove = 0;
				break;
			case CASTLEPIECE:
				if (board[from] == BLUEBUCKETPIECE) {
					board[to] = BLUECASTLEPIECE;
				} else {
					board[to] = REDCASTLEPIECE;
				}
				disallowedMove = 0;
				break;
			default:
				break;
		}
		board[from] = BLANKPIECE;
	}
	return hashPosition(board, (turn == BLUE) ? RED : BLUE, disallowedMove);
}

VALUE Primitive(POSITION position) {
	char turn;
	int disallowedMove, blueLeft, redLeft, smallLeft, largeLeft;
	char board[9];
	unhashPosition(position, board, &turn, &disallowedMove, &blueLeft, &redLeft, &smallLeft, &largeLeft);
	int bb, rb, bs, rs, bc, rc, s, l, c;
	countPiecesOnBoard(board, &bb, &rb, &bs, &rs, &bc, &rc, &s, &l, &c);
	
	if (bc == 2 || rc == 2) {
		if (!tyingRule) {
			return (isMisere) ? win : lose;
		} else {
			if (bc == 2 && rc == 1) {
				for (int i = 0; i < 9; i++) {
					if (board[i] == REDBUCKETPIECE) { // Check if red bucket piece is next to castle piece.
						for (int j = 0; j < numAdjacencies[i]; j++) {
							if (board[adjacencyMatrix[i][j]] == CASTLEPIECE) {
								return tie;
							}
						}
					} else if (board[i] == REDSMALLPIECE) { // Check if red small piece is next to large piece.
						for (int j = 0; j < numAdjacencies[i]; j++) {
							if (board[adjacencyMatrix[i][j]] == LARGEPIECE) {
								return tie;
							}
						}
					}
				}
				return (isMisere) ? win : lose;

			}
			return (isMisere) ? win : lose;
		}
	}
	if (ifNoLegalMoves == lose) {
		MOVELIST *moveList = GenerateMoves(position);
		if (moveList->move == NULLMOVE) {
			FreeMoveList(moveList);
			return (isMisere) ? win : lose;
		}
		FreeMoveList(moveList);
	}
	return undecided;
}

/************************************************************************
**
** NAME: PrintPosition
**
**     *-------*-------*-------*
**     |   B   |       |       |  Turn: Blue
**     |  /_\  |       |       |  Disallowed Move: [4-3]
**     | /___\ |  /_\  |   R   |
**     *-------+-------+-------*  Remaining Pieces to Place:
**     |       |       |       |  Blue Bucket (0)
**     |       |       |       |  Red Bucket (0)
**     |       | /___\ | /___\ |  Small Sand Pile (0)
**     *-------+-------+-------*  Large Sand Pile (0)
**     |       |       |       |
**     |  /_\  |   R   |       |  <Prediction>
**     | /___\ |  /_\  |   B   |
**     *-------*-------*-------*
**
************************************************************************/

char* PrintHelper(char piece, int level) {
	if (level == 0) {
		switch (piece) {
			case BLUECASTLEPIECE:
			case REDCASTLEPIECE:
			case CASTLEPIECE:
			case LARGEPIECE:
				return LARGEDISPLAY;
			case BLUESMALLPIECE:
			case REDSMALLPIECE:
			case SMALLPIECE:
				return SMALLDISPLAY;
			case BLUEBUCKETPIECE:
				return BLUEDISPLAY;
			case REDBUCKETPIECE:
				return REDDISPLAY;
			default:
				return BLANKDISPLAY;
		}
	} else if (level == 1) {
		switch (piece) {
			case BLUECASTLEPIECE:
			case REDCASTLEPIECE:
			case CASTLEPIECE:
				return SMALLDISPLAY;
			case BLUESMALLPIECE:
				return BLUEDISPLAY;
			case REDSMALLPIECE:
				return REDDISPLAY;
			default:
				return BLANKDISPLAY;
		}
	} else {
		if (piece == BLUECASTLEPIECE) return BLUEDISPLAY;
		if (piece == REDCASTLEPIECE) return REDDISPLAY;
		return BLANKDISPLAY;
	}
}

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
	char turn;
	int disallowedMove, blueLeft, redLeft, smallLeft, largeLeft;
	char board[9];
	unhashPosition(position, board, &turn, &disallowedMove, &blueLeft, &redLeft, &smallLeft, &largeLeft);
	
	char strDisallowedMove[7];
	if (disallowedMove == 0) {
		sprintf(strDisallowedMove, "None");
	} else {
		sprintf(strDisallowedMove, "%d-%d", idsToMoves[0][disallowedMove] + 1, idsToMoves[1][disallowedMove] + 1);
	}

	printf("\n");
	printf("        *-------*-------*-------*\n");
	printf("        | %s | %s | %s |  Turn: %s\n", PrintHelper(board[0], 2), PrintHelper(board[1], 2), PrintHelper(board[2], 2), (turn == BLUE) ? "Blue" : "Red");
	printf("        | %s | %s | %s |  Disallowed Move: %s\n", PrintHelper(board[0], 1), PrintHelper(board[1], 1), PrintHelper(board[2], 1), strDisallowedMove);
	printf("        | %s | %s | %s |\n", PrintHelper(board[0], 0), PrintHelper(board[1], 0), PrintHelper(board[2], 0));
	printf("        *-------*-------*-------*  Remaining Pieces to Place:\n");
	printf("        | %s | %s | %s |  Blue Bucket (%d)\n", PrintHelper(board[3], 2), PrintHelper(board[4], 2), PrintHelper(board[5], 2), blueLeft);
	printf("        | %s | %s | %s |  Red Bucket (%d)\n", PrintHelper(board[3], 1), PrintHelper(board[4], 1), PrintHelper(board[5], 1), redLeft);
	printf("        | %s | %s | %s |  Small Sand Pile (%d)\n", PrintHelper(board[3], 0), PrintHelper(board[4], 0), PrintHelper(board[5], 0), smallLeft);
	printf("        *-------*-------*-------*  Large Sand Pile (%d)\n", largeLeft);
	printf("        | %s | %s | %s |\n", PrintHelper(board[6], 2), PrintHelper(board[7], 2), PrintHelper(board[8], 2));
	printf("        | %s | %s | %s |  %s\n", PrintHelper(board[6], 1), PrintHelper(board[7], 1), PrintHelper(board[8], 1), GetPrediction(position, playerName, usersTurn));
	printf("        | %s | %s | %s |\n", PrintHelper(board[6], 0), PrintHelper(board[7], 0), PrintHelper(board[8], 0));
	printf("        *-------*-------*-------*\n\n");
}

MOVELIST *GenerateMoves(POSITION position) {
	MOVELIST *moveList = NULL;

	char turn;
	int disallowedMove, blueLeft, redLeft, smallLeft, largeLeft;
	char board[9];
	unhashPosition(position, board, &turn, &disallowedMove, &blueLeft, &redLeft, &smallLeft, &largeLeft);

	for (int to = 0; to < 9; to++) {
		if (board[to] == BLANKPIECE) {
	
			// PLACEMENT //
			if (blueLeft > 0 && turn == BLUE)
				moveList = CreateMovelistNode(hashMove(BLUEBUCKETPIECE, to, to, FALSE), moveList);
			else if (redLeft > 0 && turn == RED)
				moveList = CreateMovelistNode(hashMove(REDBUCKETPIECE, to, to, TRUE), moveList);

			if (smallLeft > 0) moveList = CreateMovelistNode(hashMove(SMALLPIECE, to, to, FALSE), moveList);
      		if (largeLeft > 0) moveList = CreateMovelistNode(hashMove(LARGEPIECE, to, to, FALSE), moveList);

			// SLIDE TO BLANK //
			for (int j = 0; j < numAdjacencies[to]; j++) {
				int from = adjacencyMatrix[to][j];
				if (!(from == idsToMoves[0][disallowedMove] && to == idsToMoves[1][disallowedMove])) { // Don't undo previous person's slide
					if ((turn == BLUE && board[from] != BLANKPIECE && board[from] != REDBUCKETPIECE && board[from] != REDSMALLPIECE && board[from] != REDCASTLEPIECE) || (turn == RED && board[from] != BLANKPIECE && board[from] != BLUEBUCKETPIECE && board[from] != BLUESMALLPIECE && board[from] != BLUECASTLEPIECE))
						moveList = CreateMovelistNode(hashMove(BLUEBUCKETPIECE, from, to, FALSE), moveList);
				}
			}
		} else if (board[to] == SMALLPIECE || board[to] == CASTLEPIECE) { // SLIDE ONTO SMALL PIECE OR CASTLE PIECE //
			for (int j = 0; j < numAdjacencies[to]; j++) {
				int from = adjacencyMatrix[to][j];
				if ((turn == BLUE && board[from] == BLUEBUCKETPIECE) || (turn == RED && board[from] == REDBUCKETPIECE))
					moveList = CreateMovelistNode(hashMove(BLUEBUCKETPIECE, from, to, FALSE), moveList);
			}
		} else if (board[to] == LARGEPIECE) { // SLIDE ONTO LARGE PIECE //
			for (int j = 0; j < numAdjacencies[to]; j++) {
				int from = adjacencyMatrix[to][j];
				if (board[from] == SMALLPIECE || (turn == BLUE && board[from] == BLUESMALLPIECE) || (turn == RED && board[from] == REDSMALLPIECE))
					moveList = CreateMovelistNode(hashMove(BLUEBUCKETPIECE, from, to, FALSE), moveList);
			}
		}
	}

	// Add passing move if no legal moves.
	if (moveList == NULL) {
		moveList = CreateMovelistNode(NULLMOVE, moveList);
	}

	return moveList;
}

POSITION GetCanonicalPosition(POSITION position) {
	char turn;
	int disallowedMove, blueLeft, redLeft, smallLeft, largeLeft;
	char originalBoard[9];
	unhashPosition(position, originalBoard, &turn, &disallowedMove, &blueLeft, &redLeft, &smallLeft, &largeLeft);

	char canonBoard[9];
	int canonDisallowedMove = disallowedMove;
    int bestSymmetryNum = 0;

    for (int symmetryNum = 1; symmetryNum < 8; symmetryNum++)
        for (int i = 0; i < 9; i++) {
            char pieceInSymmetry = originalBoard[gSymmetryMatrix[symmetryNum][i]];
            char pieceInBest = originalBoard[gSymmetryMatrix[bestSymmetryNum][i]];
            if (pieceInSymmetry > pieceInBest) {
                bestSymmetryNum = symmetryNum;
                break;
            }
        };

    if (bestSymmetryNum == 0) {
        return position;
	}
    
    for (int i = 0; i < 9; i++) { // Transform the rest of the board.
        canonBoard[i] = originalBoard[gSymmetryMatrix[bestSymmetryNum][i]];
	}
	
	if (disallowedMove != 0) { // Transform disallowed move.
		canonDisallowedMove = movesToIds[gSymmetryMatrix[bestSymmetryNum][idsToMoves[0][disallowedMove]]][gSymmetryMatrix[bestSymmetryNum][idsToMoves[1][disallowedMove]]];
	}

    return hashPosition(canonBoard, turn, canonDisallowedMove);
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
	USERINPUT input;
	do {
		printf("%8s's move (u)ndo: ", playerName);
		input = HandleDefaultTextInput(position, move, playerName);
		if (input != Continue)
			return input;
	} while (TRUE);

	/* NOT REACHED */
	return Continue;
}

BOOLEAN ValidTextInput(STRING input) {
	return (input[0] >= 49 && input[0] <= 57 && input[1] == '-' && input[2] >= 49 && input[2] <= 57 && input[3] == '\0') || // Movement
			((input[0] == 'B' || input[0] == 'R' || input[0] == 'S' || input[0] == 'L' || input[0] == 'b' || input[0] == 'r' || input[0] == 's' || input[0] == 'l') && input[1] >= 49 && input[1] <= 57 && input[2] == '\0') || // Placement
			strcmp(input, "None") == 0; // Pass Turn
}

MOVE ConvertTextInputToMove(STRING input) {
	char piece = BLUEBUCKETPIECE;
	if (input[0] >= 49 && input[0] <= 57) { // Movement
		return hashMove(piece, input[0] - '1', input[2] - '1', FALSE);
	} else if (input[0] == 'N' || input[0] == 'n') {
		return NULLMOVE;
	} else {
		BOOLEAN isP2Turn = FALSE;
		switch(input[0]) {
			case 'B': case 'b':
				piece = BLUEBUCKETPIECE;
				break;
			case 'R': case 'r':
				isP2Turn = TRUE;
				piece = REDBUCKETPIECE;
				break;
			case 'S': case 's':
				piece = SMALLPIECE;
				break;
			case 'L': case 'l':
				piece = LARGEPIECE;
				break;
			default:
				break;
		}
		return hashMove(piece, input[1] - '1', input[1] - '1', isP2Turn);
	}
}

void MoveToString(MOVE move, char *moveStringBuffer) {
	char piece;
	int from, to;
	BOOLEAN p2Turn;
	if (move == NULLMOVE) {
		sprintf(moveStringBuffer, "None");
	} else {
		unhashMove(move, &piece, &from, &to, &p2Turn);
		if (from == to) { // Placement
			sprintf(moveStringBuffer, "%c%d", piece, from + 1);
		} else { // Sliding
			sprintf(moveStringBuffer, "%d-%d", from + 1, to + 1);
		}
	}
}

void PrintComputersMove(MOVE computersMove, STRING computersName) {
	char str[10];
	MoveToString(computersMove, str);
	printf("%8s's move: %s\n", computersName, str);
}

// 27 bucket centers
// 18 small piece centers
// 9 large piece centers
// 1 disallowedFromIdx [54]
// 1 disallowedToIdx [55]
// 20*2 arrow centers [56-95]
// 27 placepiece move button centers [96-122]
// 1 pass turn center [123]

// BUCKET (3) top mid bottom SMALL mid bottom (2) LARGE bottom (1) 

int uwapiArrowCoords[8][9] = {
	{ 9,  0,  9,  2,  4,  9,  9,  9,  9},
	{ 9,  9,  6,  8, 10, 12,  9,  9,  9},
	{ 9,  9,  9,  9, 14, 16,  9,  9,  9},
	{ 9,  9,  9,  9, 18,  9, 20, 22,  9},
	{ 9,  9,  9,  9,  9, 24, 26, 28, 30},
	{ 9,  9,  9,  9,  9,  9,  9, 32, 34},
	{ 9,  9,  9,  9,  9,  9,  9, 36,  9},
	{ 9,  9,  9,  9,  9,  9,  9,  9, 38}
};

void PositionToString(POSITION position, char *positionStringBuffer) {
	char turn;
	int disallowedMove, blueLeft, redLeft, smallLeft, largeLeft;
	char board[9];
	unhashPosition(position, board, &turn, &disallowedMove, &blueLeft, &redLeft, &smallLeft, &largeLeft);

	positionStringBuffer[0] = turn == BLUE ? '1' : '2';
	positionStringBuffer[1] = '_';
	memcpy(positionStringBuffer + 2, board, 9);
	if (disallowedMove) {
		positionStringBuffer[11] = (disallowedMove == 0) ? '-' : idsToMoves[0][disallowedMove] + '1';
		positionStringBuffer[12] = (disallowedMove == 0) ? '-' : idsToMoves[1][disallowedMove] + '1';
		positionStringBuffer[13] = '\0';
	} else {
		positionStringBuffer[11] = '\0';
	}
}

POSITION StringToPosition(char *positionString) {
	int turn;
	char *board;
	if (ParseStandardOnelinePositionString(positionString, &turn, &board)) {
		char turnChar = (turn == 1) ? BLUE : RED;
		int disallowedMove = (board[9] == '\0') ? 0 : movesToIds[board[9] - '1'][board[10] - '1'];
		int bb, rb, bs, rs, bc, rc, s, l, c;
		countPiecesOnBoard(board, &bb, &rb, &bs, &rs, &bc, &rc, &s, &l, &c);
		gInitializeHashWindow(hashTier(bb, rb, bs, rs, bc, rc, s, l, c), FALSE);
		return hashPosition(board, turnChar, disallowedMove);
	}
	return NULL_POSITION;
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	char entityString[57];
	memset(entityString, '-', 57 * sizeof(char));

	char turn;
	int disallowedMove, blueLeft, redLeft, smallLeft, largeLeft;
	char board[9];
	unhashPosition(position, board, &turn, &disallowedMove, &blueLeft, &redLeft, &smallLeft, &largeLeft);
	int i, j;

	for (i = 0, j = 0; i < 9; i++, j += 6) {
		if (board[i] == BLUECASTLEPIECE) {
			entityString[j] = BLUEBUCKETPIECE;
			entityString[j + 3] = SMALLPIECE;
			entityString[j + 5] = LARGEPIECE;
		} else if (board[i] == REDCASTLEPIECE) {
			entityString[j] = REDBUCKETPIECE;
			entityString[j + 3] = SMALLPIECE;
			entityString[j + 5] = LARGEPIECE;
		} else if (board[i] == BLUESMALLPIECE) {
			entityString[j + 1] = BLUEBUCKETPIECE;
			entityString[j + 4] = SMALLPIECE;
		} else if (board[i] == REDSMALLPIECE) {
			entityString[j + 1] = REDBUCKETPIECE;
			entityString[j + 4] = SMALLPIECE;
		} else if (board[i] == BLUEBUCKETPIECE) {
			entityString[j + 2] = BLUEBUCKETPIECE;
		} else if (board[i] == REDBUCKETPIECE) {
			entityString[j + 2] = REDBUCKETPIECE;
		} else if (board[i] == CASTLEPIECE) {
			entityString[j + 3] = SMALLPIECE;
			entityString[j + 5] = LARGEPIECE;
		} else if (board[i] == SMALLPIECE) {
			entityString[j + 4] = SMALLPIECE;
		} else if (board[i] == LARGEPIECE) {
			entityString[j + 5] = LARGEPIECE;
		}
	}

	int uturn = (turn == BLUE) ? 1 : 2;
	entityString[54] = (disallowedMove == 0) ? '-' : idsToMoves[0][disallowedMove] + '1';
	entityString[55] = (disallowedMove == 0) ? '-' : idsToMoves[1][disallowedMove] + '1';
	entityString[56] = '\0';
	AutoGUIMakePositionString(uturn, entityString, autoguiPositionStringBuffer);
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
	(void) position;
  	if (move == NULLMOVE) {
		AutoGUIMakeMoveButtonStringA('P', 123, 'v', autoguiMoveStringBuffer);
		return;
	}
	
	char piece;
	int from, to;
	BOOLEAN p2Turn;
	unhashMove(move, &piece, &from, &to, &p2Turn);

	if (from == to) {// Placement
		if (piece == SMALLPIECE) {
			AutoGUIMakeMoveButtonStringA('v', 105 + to, 'w', autoguiMoveStringBuffer);
		} else if (piece == LARGEPIECE) {
			AutoGUIMakeMoveButtonStringA('w', 114 + to, 'x', autoguiMoveStringBuffer);
		} else {
			AutoGUIMakeMoveButtonStringA(p2Turn ? 'u' : 't', 96 + to, 'y', autoguiMoveStringBuffer);
		}
	} else {
		if (to > from) {
			int d = uwapiArrowCoords[from][to] + 56;
			AutoGUIMakeMoveButtonStringM(d, d + 1, 'z', autoguiMoveStringBuffer);
		} else {
			int d = uwapiArrowCoords[to][from] + 56;
			AutoGUIMakeMoveButtonStringM(d + 1, d, 'z', autoguiMoveStringBuffer);
		}
	}
}