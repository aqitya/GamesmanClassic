/************************************************************************
**
** NAME:	solveretrograde.c
**
** DESCRIPTION:	The Retrograde Solver.
**
** AUTHOR:	Max Delgadillo
**		GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2006-10-11
**
**
** LICENSE:	This file is part of GAMESMAN,
**		The Finite, Two-person Perfect-Information Game Generator
**		Released under the GPL:
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program, in COPYING; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**************************************************************************/

#include "gamesman.h"
#include "solveretrograde.h"
#include "tierdb.h"
#include "dirent.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

// TIER VARIABLES
TIERLIST* tierSolveList; // the total list for the game, w/initial at start
TIERLIST* solveList; // only the tiers we have yet to solve, REVERSED
TIERLIST* solvedList; // the tiers we have already solved
TIER numTiers; // the total number of tiers for this game
TIER tiersSolved; // the number of tiers we've solved so far
// PRE-SOLVER VARIABLES:
int variant; // The variant of this game, used so getOption() is only called once
char filename[80]; // a global filename variable for re-use.
STRING tierStr; // a global tier string variable for re-use.
BOOLEAN tierNames; // Whether or not to display names of tiers.
BOOLEAN checkLegality; // Whether or not to check legality of tierpositions.
BOOLEAN useUndo; // Whether or not to use undomove functions.
BOOLEAN forceLoopy; // Whether or not to force the loopy solver on non-loopy tiers.
BOOLEAN checkCorrectness; // Whether or not to check correctness after the solve.

// Solver procs
void CheckRequiredTierGamesmanAPIFunctions();
void checkExistingDB();
void AutoSolveAllTiers();
BOOLEAN gotoNextTier();
void solveFirst(TIER);
void PrepareToSolveNextTier();
void changeTierSolveList();
POSITION GetMyPosition();
// Solver Heart
void SolveTier();
void SolveWithNonLoopyAlgorithm();
void SolveWithLoopyAlgorithm();
void LoopyParentsHelper(IPOSITIONLIST*, VALUE, REMOTENESS);
// Solver ChildCounter and Hashtable functions
void rInitFRStuff();
void rFreeFRStuff();
IPOSITIONLIST* rRemoveFRList(VALUE, REMOTENESS);
void rInsertFR(VALUE, POSITION, REMOTENESS);
// Sanity Checkers
void checkForCorrectness();
TIERLIST* checkAndDefineTierTree();
BOOLEAN checkTierTree();

/************************************************************************
**
** NAME:        DetermineRetrogradeValue
**
** DESCRIPTION: Called by Gamesman, the solver goes into a menu
**				rather than just solving.
**
************************************************************************/

VALUE DetermineRetrogradeValue(POSITION position) {
	if (position == -1ULL) return undecided;
	gDontLoadTierDB = FALSE;

	variant = getOption();
	tierNames = TRUE;
	checkLegality = useUndo = forceLoopy = checkCorrectness = FALSE;
	// initialize local variables
	BOOLEAN cont = TRUE, isLegalGiven = TRUE, undoGiven = TRUE;

	ifprintf(gTierSolvePrint, "\n\n===== Welcome to the TIER-GAMESMAN Retrograde Solver! =====\n");
	ifprintf(gTierSolvePrint, "Currently solving game (%s) with variant (%d)\n", kGameName, variant);
	ifprintf(gTierSolvePrint, "Initial Position: %llu, in Initial Tier: %llu\n", gInitialTierPosition, gInitialTier);

	CheckRequiredTierGamesmanAPIFunctions();

	ifprintf(gTierSolvePrint, "\n----- Checking the OPTIONAL API Functions: -----\n\n");
	if (gIsLegalFunPtr == NULL) {
		ifprintf(gTierSolvePrint, "-IsLegal NOT GIVEN\nLegality Checking Disabled\n");
		isLegalGiven = FALSE;
	} else checkLegality = TRUE;
	if (gGenerateUndoMovesToTierFunPtr == NULL) {
		ifprintf(gTierSolvePrint, "-GenerateUndoMovesToTier NOT GIVEN\nUndoMove Use Disabled\n");
		undoGiven = FALSE;
	}
	if (gUnDoMoveFunPtr == NULL) {
		ifprintf(gTierSolvePrint, "-UnDoMove NOT GIVEN\nUndoMove Use Disabled\n");
		undoGiven = FALSE;
	}
	if (gTierToStringFunPtr == NULL) {
		ifprintf(gTierSolvePrint, "-TierToString NOT GIVEN\nTier Name Printing Disabled\n");
		tierNames = FALSE;
	}
	if (isLegalGiven && undoGiven && tierNames)
		ifprintf(gTierSolvePrint, "API Optional Functions Confirmed.\n");

	ifprintf(gTierSolvePrint, "\n----- Checking and Generating the Tier Tree: -----\n\n");

	if (!gIsInteract) {
		solveList = checkAndDefineTierTree();
		if (solveList == NULL) {
			printf("\nPlease fix gTierChildren before attempting to solve!\n"
				"Exiting Retrograde Solver (WITHOUT Solving)...\n");
			ExitStageRight();
		} else ifprintf(gTierSolvePrint, "Tier Tree generated successfully.\n");
	}

	tierSolveList = CopyTierlist(solveList);
	solvedList = NULL;
	tiersSolved = 0;

	ifprintf(gTierSolvePrint, "-Tier Solve Order:\n");
	TIERLIST* ptr = solveList;
	numTiers = 0;
	for (; ptr != NULL; ptr = ptr->next) {
		ifprintf(gTierSolvePrint, "%llu ", ptr->tier);
		if (tierNames) {
			tierStr = gTierToStringFunPtr(ptr->tier);
			ifprintf(gTierSolvePrint, ": %s\n", tierStr);
			if (tierStr != NULL) SafeFree(tierStr);
		}
		numTiers++;
	}
	ifprintf(gTierSolvePrint, "\n   %llu Tiers are confirmed to be solved.\n", numTiers);
	gTotalTiers = numTiers; // for the GUI load percentage printing

	if (!gIsInteract) {
		ifprintf(gTierSolvePrint, "\n----- Checking for existing Tier DBs: -----\n\n");
		checkExistingDB();
	}

	TIERLIST *tltl;
	TIERLIST *tltl2 = NULL;
	TIERLIST *tlhead;
	TIERLIST *tl2head;
	TIERPOSITION posi;
	MOVELIST *movel;
	time_t start, end;

	if (solveList == NULL) {
		ifprintf(gTierSolvePrint, "\nLooks like the game is already fully solved! Enjoy the game!\n");
	} else {
		if (tiersSolved == 0) // No DBs loaded, a fresh solve
			ifprintf(gTierSolvePrint, "No DBs Found! Starting a fresh solve...\n");

		if (gTierSolverMenu) {
			PrepareToSolveNextTier();

			char c;
			while(cont) {
				printf("\n\n\t----- RETROGRADE SOLVER MENU for game: %s -----\n", kGameName);
				printf("\tReady to solve %sLOOPY tier %llu", (gCurrentTierIsLoopy ? "" : "NON-"), gCurrentTier);
				if (tierNames) {
					tierStr = gTierToStringFunPtr(gCurrentTier);
					printf(" (%s)", tierStr);
					if (tierStr != NULL) SafeFree(tierStr);
				}
				printf("\n\tThe tier hash contains (%lld) positions.", gCurrentTierSize);
				printf("\n\tTiers left: %llu (%.1f%c Solved)\n\n", numTiers-tiersSolved, 100*(double)tiersSolved/numTiers, '%');
				if (isLegalGiven)
					printf("\tl)\tCheck (L)egality using IsLegal? Currently: %s\n", (checkLegality ? "YES" : "NO"));
				else printf("\t\t(Legality Checking using IsLegal DISABLED)\n");
				if (undoGiven)
					printf("\tu)\t(U)se UndoMove functions for Loopy Solve? Currently: %s\n", (useUndo ? "YES" : "NO"));
				else printf("\t\t(Undomove functions for Loopy Solve DISABLED)\n");
				printf("\tc)\tCheck (C)orrectness after solve? Currently: %s\n"
				       "\tf)\t(F)orce Loopy solve for Non-Loopy tiers? Currently: %s\n\n"
				       "\ts)\t(S)olve the next tier.\n\n"
                       "\ta)\t(A)utomate the solving for all the tiers left.\n\n"
				       "\tt)\tChange the (T)ier Solve Order.\n\n"
				       "\tb)\t(B)egin the Game before fully solving!\n\n"
				       "\tq)\t(Q)uit the Retrograde Solver.\n"
				       "\nSelect an option:  ", (checkCorrectness ? "YES" : "NO"), (forceLoopy ? "YES" : "NO"));
				c = GetMyChar();
				switch(c) {
				case 'l': case 'L':
					if (isLegalGiven) //IsLegal is given
						checkLegality = !checkLegality;
					else printf("IsLegal isn't written! Thus, you can't check legality!\n");
					break;
				case 'u': case 'U':
					if (undoGiven) //Undo stuff is given
						useUndo = !useUndo;
					else printf("UndoMove function(s) not written! Thus, you can't use the UndoMove Algorithm!\n");
					break;
				case 'c': case 'C':
					checkCorrectness = !checkCorrectness;
					break;
				case 'f': case 'F':
					forceLoopy = !forceLoopy;
					break;
				case 's': case 'S':
					SolveTier();
					if (!gotoNextTier()) {
						printf("\n%s is now fully solved!\n", kGameName);
						cont = FALSE;
					} else PrepareToSolveNextTier();
					break;
				case 'a': case 'A':
					printf("Fully Solving starting from Tier %llu...\n\n",gCurrentTier);
					BOOLEAN loop = TRUE;

					while (loop) {
						PrepareToSolveNextTier();
						SolveTier();
						loop = gotoNextTier();
						printf("\n\n---Tiers left: %llu (%.1f%c Solved)", numTiers-tiersSolved, 100*(double)tiersSolved/numTiers, '%');
					}
					printf("\n%s is now fully solved!\n", kGameName);
					cont = FALSE;
					break;
				case 't': case 'T':
					changeTierSolveList();
					break;
				case 'q': case 'Q':
					printf("Exiting Retrograde Solver (WITHOUT Fully Solving)...\n"
					       "Keep in mind that next time you start the solve, you will\n"
					       "continue from this exact point, if all the databases are there.\n"
					       "To ensure correct solving, make sure that the already-written\n"
					       "databases (in your data/m%s_%d_tierdb directory) are not altered,\n"
					       "and the API functions are unchanged from their current state.\n", kDBName, variant);
					ExitStageRight();
					break;
				case 'k': case 'K':
					tltl = checkAndDefineTierTree();
					tlhead = tltl;
					while (tltl) {
						printf("%llu\n", tltl->tier);
						tltl = tltl->next;
					}
					FreeTierList(tlhead);
					break;
				case 'z': case 'Z':
					tltl = checkAndDefineTierTree();
					tlhead = tltl;
					while (tltl) {
						tltl2 = CreateTierlistNode(tltl->tier, tltl2);
						tltl = tltl->next;
					}
					FreeTierList(tlhead);
					tl2head = tltl2;
					while (tltl2) {
						printf("Gonna Process Tier %llu\n", tltl2->tier);
						start = clock();
						gInitializeHashWindow(tltl2->tier, FALSE);
						for (posi = 0; posi < gNumberOfTierPositionsFunPtr(tltl2->tier); posi++) {
							movel = GenerateMoves(posi);
							FreeMoveList(movel);
						}
						end = clock();
						printf("Total time taken by CPU: %f\n", (double)(end - start) / CLOCKS_PER_SEC);
						tltl2 = tltl2->next;
					}
					FreeTierList(tl2head);
					break;
				default:
					printf("Invalid option!\n");
				}
			}
		} else {
			// if no menu, go straight to auto solve!
			if (gSolveOnlyTier) {
				gInitializeHashWindow(gTierToOnlySolve, TRUE);
				SolveTier();
			} else {
				AutoSolveAllTiers();
			}
		}
	}
	ifprintf(gTierSolvePrint, "Exiting Retrograde Solver...\n\n");
	FreeTierList(tierSolveList);
	FreeTierList(solveList);
	FreeTierList(solvedList);
	return undecided; //just bitter at the fact that this is ignored
}


void AutoSolveAllTiers() {
    ifprintf(gTierSolvePrint, "Fully Solving the game...\n\n");
    BOOLEAN loop = TRUE;

    while (loop) {
        PrepareToSolveNextTier();
        SolveTier();
        loop = gotoNextTier();
        ifprintf(gTierSolvePrint, "\n\n---Tiers left: %llu (%.1f%c Solved)", numTiers-tiersSolved, 100*(double)tiersSolved/numTiers, '%');
    }
    ifprintf(gTierSolvePrint, "\n%s is now fully solved!\n", kGameName);
}

// Inits the hash window/database and prepares to solve tier
void PrepareToSolveNextTier() {
	ifprintf(gTierSolvePrint, "\n------Preparing to solve tier: %llu\n", solveList->tier);
	gInitializeHashWindow(solveList->tier, TRUE);
	PercentDone(Clean); //reset percentage bar
	ifprintf(gTierSolvePrint, "  Done! Hash Window initialized and Database loaded and prepared!\n");
}

// we just solved the current tier, now go to the next
// returns TRUE if there's more tiers to solve, false otherwise
BOOLEAN gotoNextTier() {
	TIERLIST* temp = solveList;
	solveList = solveList->next;
	solvedList = CreateTierlistNode(temp->tier, solvedList);
	SafeFree(temp);
	tiersSolved++;
	PercentDone(AdvanceTier);
	return (solveList != NULL);
}

// Alters solveList so that tier goes on Front of list
void solveFirst(TIER tier) {
	solveList = MoveToFrontOfTierlist(tier, solveList);
}

// weed existing DBs out of the solveList
void checkExistingDB() {
	TIERLIST* ptr = solveList, *tmp;
	int result;

	while(ptr != NULL) {
		result = CheckTierDB(ptr->tier, variant); // check the tier's DB
		if(result == 0) {
			ptr = ptr->next;
			continue;
		} else if (result == -1) {
			ifprintf(gTierSolvePrint, "--%llu's Tier DB appears incorrect/corrupted. Re-solving...\n", ptr->tier);
			ptr = ptr->next;
			continue;
		} else if (result == 1) {
			ifprintf(gTierSolvePrint, "  %llu's Tier DB Found!\n", ptr->tier);
			if (ptr->tier != solveList->tier) { // if this isn't next to solve!
				tmp = ptr->next;
				solveFirst(ptr->tier);
				gotoNextTier();
				ptr = tmp;
			} else { //this is first on the list
				gotoNextTier();
				ptr = solveList;
			}
		}
	}
}

// A helper which tells what tiers CAN be set to be solved next
TIERLIST* possibleNextTiers() {
	TIERLIST* nexts = NULL, *ptr = solveList, *children, *childptr;
	TIER t, ct;
	for (; ptr != NULL; ptr = ptr->next) {
		t = ptr->tier;
		children = childptr = gTierChildrenFunPtr(t);
		for (; childptr != NULL; childptr = childptr->next) {
			ct = childptr->tier;
			if (TierInList(ct, solveList) && ct != t)
				break;
		}
		if (childptr == NULL) //all children solved!
			nexts = CreateTierlistNode(t, nexts);
		FreeTierList(children);
	}
	return nexts;
}

// A menu with which to change TierSolveList
void changeTierSolveList() {
	TIERLIST* nexts = possibleNextTiers();
	if (nexts->next == NULL) {
		printf("There's only that one tier to choose from!\n");
		FreeTierList(nexts);
		return;
	}
	while (TRUE) {
		TIERLIST* ptr = nexts;
		printf("You can choose from these tiers which to solve next:\n");
		for (; ptr != NULL; ptr = ptr->next) {
			printf("  %llu", ptr->tier);
			if (tierNames) {
				tierStr = gTierToStringFunPtr(ptr->tier);
				printf(" (%s)\n", tierStr);
				if (tierStr != NULL) SafeFree(tierStr);
			}
		}
		printf("\nEnter a TIER number to solve next, or non-number to go back:\n> ");
		POSITION p = GetMyPosition();
		if (p == kBadPosition) break;
		TIER t = (TIER)p;
		if (TierInList(t, nexts)) {
			solveFirst(t);
			PrepareToSolveNextTier();
			break;
		} else printf("That's not a tier to be solved next! Try again...\n\n");
	}
	FreeTierList(nexts);
}

POSITION GetMyPosition() {
	char inString[MAXINPUTLENGTH];
	POSITION p; int i;
	while(TRUE) {
		GetMyStr(inString, MAXINPUTLENGTH);
		if (inString[0] == '\0') continue;
		p = 0;
		i = 0;
		while (inString[i] >= '0' && inString[i] <='9') {
			p = (p*10) + (inString[i]-'0');
			i++;
		}
		if (inString[i] != '\0' && inString[i] != '\n') {
			return kBadPosition;
		}
		return p;
	}
}

/************************************************************************
**
** NAME:        InitTierGamesman
**
** DESCRIPTION: Called by textui.c when NOT solving, just playing
**
************************************************************************/

void CheckRequiredTierGamesmanAPIFunctions() {
	BOOLEAN cont;
	ifprintf(gTierSolvePrint, "\n-----Checking the REQUIRED TIER GAMESMAN API Functions:-----\n\n");
	if (gInitialTier == -1ULL) {
		ifprintf(gTierSolvePrint, "-gInitialTier NOT GIVEN\n"); cont = FALSE;
	}
	if (gInitialTierPosition == -1ULL) {
		ifprintf(gTierSolvePrint, "-gInitialTierPosition NOT GIVEN\n"); cont = FALSE;
	}
	if (gTierChildrenFunPtr == NULL) {
		ifprintf(gTierSolvePrint, "-TierChildren NOT GIVEN\n"); cont = FALSE;
	}
	if (gNumberOfTierPositionsFunPtr == NULL) {
		ifprintf(gTierSolvePrint, "-NumberOfTierPositions NOT GIVEN\n"); cont = FALSE;
	}
	if (cont == FALSE) {
		printf("\nOne or more required parts of the API not given...\n");
		ExitStageRight();
	} else {
		ifprintf(gTierSolvePrint, "API Required Functions Confirmed.\n");
	}
}

// ONLY check tier tree. Called by textui.c
POSITION InitTierGamesman() {
	gDontLoadTierDB = TRUE;
	CheckRequiredTierGamesmanAPIFunctions();

	ifprintf(gTierSolvePrint, "\n-----Checking the Tier Tree for correctness:-----\n\n");
	if (!checkTierTree()) {
		printf("\nPlease fix gTierChildren before attempting to play!\n");
		ExitStageRight();
	} else ifprintf(gTierSolvePrint, "No Errors Found! Tier Tree correctness confirmed.\n");
	gInitializeHashWindow(gInitialTier, TRUE);
	return gHashToWindowPosition(gInitialTierPosition, gInitialTier);
}

/************************************************************************
**
** SOLVER CHILDCOUNTER AND HASHTABLE FUNCTIONS
**
************************************************************************/

// This will be my "children counter" until this goes to file
// or I think of something better.
// Oh, and I use chars as a hack way to get 8-bits
typedef unsigned char CHILDCOUNT;
CHILDCOUNT* childCounts;

// The Parent Pointers
POSITIONLIST** rParents;

/* Rather than a Frontier Queue, this uses a sort of hashtable,
   with a POSITIONLIST for every REMOTENESS from 0 to REMOTENESS_MAX-1.
   It's constant time insert and remove, so it works just fine. */
IFRnode**        rWinFR = NULL;  // The FRontier Win Hashtable
IFRnode**        rLoseFR = NULL; // The FRontier Lose Hashtable
IFRnode**        rTieFR = NULL;  // The FRontier Tie Hashtable

/*
   PROOFS OF CORRECTNESS.
   I realized the following things about the Queue:

   Processing WIN frontier, rem. R adds to LOSE, R+1.
    ""     LOSE   ""      ""  "" ""  "" WIN, R+1.
    ""     TIE    ""      ""  "" ""  "" TIE, R+1.
   LOSES: When processing list R, will only be added to WINS list R+1.
   WINS: The most complicated... when processing list R, it adds to
      LOSES list R+1. So if we process that LOSE list IMMEDIATELY,
      we'll be adding to the WINS list R+2.
   TIES: When processing list R, will only be added to self list R+1.

   So, have a list for every remoteness; order inside that list doesn't matter.
   -Start by going through List Remoteness 0, then 1, then 2, etc...
   As we go through frontier, things are only added ABOVE THE REMOTENESS
   WE'RE CURRENTLY EXPLORING (the reason this all works). This is because
   we always add to the "queue" as currentChild'sRemoteness+1. Therefore,
   we can just iterate through a list one by one really fast and it all
   works out.
   -So, first go through the LOSE/WIN lists alternately, then TIE lists.
   LOSE and TIE are basically the same; WIN is more interesting. Whenever
   we get a win generating LOSEs, note that they're ALL the same remoteness.
   So instead of putting them back in into the queue (since we're going to
   see all of them next ANYWAY, and it doesn't matter the order) just have
   a local "LOSE" list that you go through immediately!
   -So it's proven that on an INSERT, you're NEVER going to be inserting
   equal to lowestList or currentList.
 */

void rInitFRStuff() {
	POSITION i;
	childCounts = (CHILDCOUNT*) SafeMalloc (gCurrentTierSize * sizeof(CHILDCOUNT));
	for (i = 0; i < gCurrentTierSize; i++)
		childCounts[i] = 0;
	if (!useUndo) {
		rParents = (POSITIONLIST**) SafeMalloc (gNumberOfPositions * sizeof(POSITIONLIST*));
		for (i = 0; i < gNumberOfPositions; i++)
			rParents[i] = NULL;
	}
	// 255 * 4 bytes = 1,020 bytes = ~1 KB
	rWinFR = (IFRnode**) SafeMalloc (REMOTENESS_MAX * sizeof(IFRnode*));
	rLoseFR = (IFRnode**) SafeMalloc (REMOTENESS_MAX * sizeof(IFRnode*)); // ~1 KB
	rTieFR = (IFRnode**) SafeMalloc (REMOTENESS_MAX * sizeof(IFRnode*)); // ~1 KB
	for (i = 0; i < REMOTENESS_MAX; i++)
		rWinFR[i] = rLoseFR[i] = rTieFR[i] = NULL;
}

void rFreeFRStuff() {
	if (childCounts != NULL) SafeFree(childCounts);
	if (!useUndo) {
		// Free the Position Lists
		POSITION i;
		for (i = 0; i < gNumberOfPositions; i++)
			FreePositionList(rParents[i]);
		if (rParents != NULL) SafeFree(rParents);
	}
	// Free the Position Lists
	if (rWinFR != NULL) SafeFree(rWinFR);
	if (rLoseFR != NULL) SafeFree(rLoseFR);
	if (rTieFR != NULL) SafeFree(rTieFR);
}

IPOSITIONLIST* rRemoveFRList(VALUE value, REMOTENESS r) {
	if (value == win)
		return rWinFR[r];
	else if (value == lose)
		return rLoseFR[r];
	else if (value == tie)
		return rTieFR[r];
	return NULL;
}

void rInsertFR(VALUE value, POSITION position, REMOTENESS r) {
	// this is probably the best place to put this:
	assert(r >= 0 && r < REMOTENESS_MAX);
	if(value == win)
		rWinFR[r] = StorePositionInIList(position, rWinFR[r]);
	else if (value == lose)
		rLoseFR[r] = StorePositionInIList(position, rLoseFR[r]);
	else if (value == tie)
		rTieFR[r] = StorePositionInIList(position, rTieFR[r]);
}



/************************************************************************
**
** NAME:        SolveTier
**
** DESCRIPTION: The heart of the solver. This uses the current tier number,
**				then calls the "loopy solver" algorithm
**				to solve the current tier.
**				If non loopy, then it calls the non-loopy solver.
**              Expects gInitializeHashWindow to have already been
**              called with the correct tier, and the databases loaded
**              properly.
**
************************************************************************/

POSITION numSolved, trueSizeOfTier;

void SolveTier() {
	numSolved = trueSizeOfTier = 0;

	// header
	ifprintf(gTierSolvePrint, "\n----- Solving Tier %llu", gCurrentTier);
	if (tierNames) {
		tierStr = gTierToStringFunPtr(gCurrentTier);
		ifprintf(gTierSolvePrint, " (%s)", tierStr);
		SafeFree(tierStr);
	}
	ifprintf(gTierSolvePrint, "-----\n");
	// print flags information
	ifprintf(gTierSolvePrint, "Size of current hash window: %llu\n",gNumberOfPositions);
	ifprintf(gTierSolvePrint, "Size of tier %llu's hash: %llu\n",gCurrentTier,gCurrentTierSize);
	ifprintf(gTierSolvePrint, "\nSolver Type: %sLOOPY\n",((forceLoopy||gCurrentTierIsLoopy) ? "" : "NON-"));
	ifprintf(gTierSolvePrint, "Using Symmetries: %s\n",(gSymmetries ? "YES" : "NO"));
	ifprintf(gTierSolvePrint, "Checking Legality (using IsLegal): %s\n",(checkLegality ? "YES" : "NO"));
	// now actually SOLVE depending on which solver to use
	if (forceLoopy || gCurrentTierIsLoopy) { // LOOPY SOLVER
		ifprintf(gTierSolvePrint, "Using UndoMove Functions: %s\n",(useUndo ? "YES" : "NO"));
		SolveWithLoopyAlgorithm();
		ifprintf(gTierSolvePrint, "--Freeing Child Counters and Frontier Hashtables...\n");
		rFreeFRStuff();
	} else SolveWithNonLoopyAlgorithm(); // NON-LOOPY SOLVER
	// successfully finished solving!
	ifprintf(gTierSolvePrint, "\nTier fully solved!\n");
	if (checkCorrectness) {
		ifprintf(gTierSolvePrint, "--Checking Correctness...\n");
		checkForCorrectness();
	}
	// now save to database
	ifprintf(gTierSolvePrint, "--Saving the Tier Database to File...\n");
	if (SaveDatabase()) {
		ifprintf (gTierSolvePrint, "Database successfully saved!\n");
	} else {
		printf("Couldn't save tierDB!\n");
		ExitStageRight();
	}
}

void SolveWithNonLoopyAlgorithm() {
	ifprintf(gTierSolvePrint, "\n-----PREPARING NON-LOOPY SOLVER-----\n");
	POSITION pos, child;
	MOVELIST *moves, *movesptr;
	VALUE value;
	REMOTENESS remoteness;
	REMOTENESS maxWinRem, minLoseRem, minTieRem;
	BOOLEAN seenLose, seenTie;

	ifprintf(gTierSolvePrint, "Doing a sweep of the tier, and solving it in one go...\n");
	for (pos = 0; pos < gCurrentTierSize; pos++) { // Solve only parents
		if ((!checkLegality || gIsLegalFunPtr(pos)) && (!gSymmetries || pos == gCanonicalPosition(pos))) {
			value = Primitive(pos);
			if (value != undecided) { // check for primitive-ness
				SetRemoteness(pos, 0);
				StoreValueOfPosition(pos, value);
			} else {
				moves = movesptr = GenerateMoves(pos);
				maxWinRem = -1;
				minLoseRem = minTieRem = REMOTENESS_MAX;
				seenLose = seenTie = FALSE;
				for (; movesptr != NULL; movesptr = movesptr->next) {
					child = DoMove(pos, movesptr->move);
					if (gSymmetries) {
						child = gCanonicalPosition(child);
					}
					value = GetValueOfPosition(child);
					if (value != undecided) {
						remoteness = Remoteness(child);
						if (value == tie) {
							seenTie = TRUE;
							if (remoteness < minTieRem)
								minTieRem = remoteness;
							continue;
						} else if (value == lose) {
							seenLose = TRUE;
							if (remoteness < minLoseRem)
								minLoseRem = remoteness;
							continue;
						} else if (remoteness > maxWinRem) //win
							maxWinRem = remoteness;
					} else {
						printf("ERROR: GenerateMoves on %llu found undecided child, %llu!\n", pos, child);
						ExitStageRight();
					}
					FreeMoveList(moves);
					if (seenLose) {
						SetRemoteness(pos ,minLoseRem + 1);
						StoreValueOfPosition(pos, win);
					} else if (seenTie) {
						if (minTieRem == REMOTENESS_MAX) {
							SetRemoteness(pos, REMOTENESS_MAX); // a draw
						} else {
							SetRemoteness(pos, minTieRem + 1); // else a tie
						}
						StoreValueOfPosition(pos, tie);
					} else {
						SetRemoteness(pos, maxWinRem + 1);
						StoreValueOfPosition(pos, lose);
					}
				}
			}
		}
	}
}

unsigned long long dedupHashSize = 0ULL; // Number of slots in dedup hash
unsigned long long dedupHashElem = 0ULL; // Number of entries added to dedup hash
unsigned long long dedupHashMask = 0LL;
unsigned long long dedupHashBytes = 0LL;
unsigned long long DEDUP_HASH_SIZE_INITIAL = 32LL; // Initial size of Dedup Hash
int DEDUP_HASH_RATIO = 2; // Keep dedup hash size > (dedupHashElem << DEDUP_HASH_RATIO)
POSITION *dedupHash = NULL; // Dedup hashtable

void dedupHashExpand() {
    if (dedupHash == NULL) {
        dedupHashSize = DEDUP_HASH_SIZE_INITIAL;
        dedupHash = (POSITION *) calloc(dedupHashSize, sizeof(POSITION));
		if (dedupHash == NULL) {
			fprintf(stderr, "Error: calloc failed to allocate");
			ExitStageRight();
			exit(0);
		}
	} else {
        unsigned long long oldDedupHashSize = dedupHashSize;
		dedupHashSize *= 2;
        printf("Expanding Dedup Hash to %lld\n", dedupHashSize);
		POSITION *oldDedupHash = dedupHash;
        dedupHash = (POSITION *) calloc(dedupHashSize, sizeof(POSITION));
		if (dedupHash == NULL) {
			fprintf(stderr, "Error: calloc failed to allocate");
			ExitStageRight();
			exit(0);
		}
		for (POSITION i = 0; i < oldDedupHashSize; i++) {
			if (oldDedupHash[i] != 0) {
				dedupHashAdd(oldDedupHash[i] - 1);
			}
		}
		SafeFree(oldDedupHash);
    }
	dedupHashMask = dedupHashSize - 1;
	dedupHashBytes = dedupHashSize * sizeof(POSITION);
}

// if the element exists already, return FALSE
// if the element doesn't exist, add it, return TRUE
BOOLEAN dedupHashAdd(POSITION pos) {
    if (dedupHashSize <= (dedupHashElem << DEDUP_HASH_RATIO)) {
        dedupHashExpand();
    }
    POSITION posAdjusted = pos + 1;
	POSITION slot = posAdjusted & dedupHashMask;
	while (TRUE) {
		if (dedupHash[slot] == 0) {
			dedupHash[slot] = posAdjusted;
			dedupHashElem++;
			return TRUE;
		} else if (dedupHash[slot] == posAdjusted) {
			return FALSE;
		} else {
			if (slot == 0) {
				slot = dedupHashSize - 1;
			} else {
				slot -= 1;
			}
		}
	}
}

/************************************************************************
**
** NAME:        SolveWithLoopyAlgorithm
**
** DESCRIPTION: A Retrograde implementation of the loopy solver algorithm
**				(which is MUCH better/more efficient than the normal
**				loopy solver, dare I say).
**
************************************************************************/

void SolveWithLoopyAlgorithm() {
	ifprintf(gTierSolvePrint, "\n-----PREPARING LOOPY SOLVER-----\n");
	POSITION pos, canonPos, child;
	MOVELIST *moves, *movesptr;
	VALUE value;
	REMOTENESS remoteness;

	ifprintf(gTierSolvePrint, "--Setting up Child Counters and Frontier Hashtables...\n");
	rInitFRStuff();
	ifprintf(gTierSolvePrint, "--Doing a sweep of the tier, and setting up the frontier...\n");
	for (pos = 0; pos < gCurrentTierSize; pos++) { // SET UP PARENTS
		if (childCounts[pos] == 0) { // else, ignore this child, it was already solved
			if (!gSymmetries || pos == gCanonicalPosition(pos)) {
				trueSizeOfTier++;
				value = Primitive(pos);
				if (value != undecided) { // check for primitive-ness
					SetRemoteness(pos,0);
					StoreValueOfPosition(pos,value);
					numSolved++;
					rInsertFR(value, pos, 0);
				} else {
					moves = movesptr = GenerateMoves(pos);
					if (dedupHash != NULL) {
						dedupHashElem = 0LL;
						memset(dedupHash, 0, dedupHashBytes);
					}
					if (moves == NULL) { // no chillins
						printf("ERROR: GenerateMoves on %llu returned NULL\n", pos);
						ExitStageRight();
					} else {
						//otherwise, make a Child Counter for it
						movesptr = moves;
						for (; movesptr != NULL; movesptr = movesptr->next) {
							child = gSymmetries ? gCanonicalPosition(DoMove(pos, movesptr->move)) : DoMove(pos, movesptr->move);
							if (gSymmetries && useUndo && !dedupHashAdd(child)) continue;
							childCounts[pos]++;

							if (!useUndo) { // if parent pointers, add to parent pointer list
								rParents[child] = StorePositionInList(pos, rParents[child]);
							}
						}
						FreeMoveList(moves);
					}
				}
			}
		}
	}
	if (checkLegality) {
		ifprintf(gTierSolvePrint, "True size of tier: %lld\n",trueSizeOfTier);
		ifprintf(gTierSolvePrint, "Tier %llu's hash efficiency: %.1f%c\n",gCurrentTier, 100*(double)trueSizeOfTier/gCurrentTierSize, '%');
	}
	ifprintf(gTierSolvePrint, "Amount now solved (primitives): %lld (%.1f%c)\n",numSolved, 100*(double)numSolved/trueSizeOfTier, '%');
	if (numSolved == trueSizeOfTier) {
		ifprintf(gTierSolvePrint, "Tier is all primitives! No loopy algorithm needed!\n");
		return;
	}
	// SET UP FRONTIER!
	ifprintf(gTierSolvePrint, "--Doing a sweep of child tiers, and setting up the frontier...\n");
	for (pos = gCurrentTierSize; pos < gNumberOfPositions; pos++) {
		if (!useUndo && rParents[pos] == NULL) // if we didn't even see this child, don't put it on frontier!
			continue;
		if (gSymmetries) {// use the canonical position's values
			canonPos = gCanonicalPosition(pos);
			if (useUndo && pos != canonPos)
				continue;
		} else {
			canonPos = pos; // else ignore
		}
		value = GetValueOfPosition(canonPos);
		remoteness = Remoteness(canonPos);
		if (!((value == tie && remoteness == REMOTENESS_MAX)
		      || value == undecided))
			rInsertFR(value, pos, remoteness);
	}
	tierdb_free_childpositions();
	ifprintf(gTierSolvePrint, "\n--Beginning the loopy algorithm...\n");
	REMOTENESS r; IPOSITIONLIST* list;
	ifprintf(gTierSolvePrint, "--Processing Lose/Win Frontiers!\n");
	for (r = 0; r <= REMOTENESS_MAX; r++) {
		if (r!=REMOTENESS_MAX) {
			list = rRemoveFRList(lose,r);
			if (list != NULL)
				LoopyParentsHelper(list, win, r);
		}
		if (r!=0) {
			list = rRemoveFRList(win,r-1);
			if (list != NULL)
				LoopyParentsHelper(list, lose, r-1);
		}
	}
	ifprintf(gTierSolvePrint, "Amount now solved: %lld (%.1f%c)\n",numSolved, 100*(double)numSolved/trueSizeOfTier, '%');
	if (numSolved == trueSizeOfTier)
		return; // Else, we must process ties!
	ifprintf(gTierSolvePrint, "--Processing Tie Frontier!\n");
	for (r = 0; r < REMOTENESS_MAX; r++) {
		list = rRemoveFRList(tie,r);
		if (list != NULL)
			LoopyParentsHelper(list, tie, r);
	}

	ifprintf(gTierSolvePrint, "Amount now solved: %lld (%.1f%c)\n",numSolved, 100*(double)numSolved/trueSizeOfTier, '%');
	if (numSolved == trueSizeOfTier)
		return; // Else, we have undecideds... must make them DRAWs
	ifprintf(gTierSolvePrint, "--Setting undecided to DRAWs...\n");
	for(pos = 0; pos < gCurrentTierSize; pos++) {
		if (childCounts[pos] > 0) { // no lose/tie children, no/some wins = draw
			SetRemoteness(pos,REMOTENESS_MAX); // a draw
			StoreValueOfPosition(pos, tie);
			numSolved++;
		}
	}
	assert(numSolved == trueSizeOfTier);
}

void LoopyParentsHelper(IPOSITIONLIST* list, VALUE valueParents, REMOTENESS remotenessChild) {
	POSITION child, parent;
	IFRnode *miniLoseFR = NULL;
	UNDOMOVELIST *parents, *parentsPtr;
	POSITIONLIST *parentList;

	unsigned long long idx = 0;
	IPOSITIONSUBLIST *currISL = list->head;
	while (idx < list->size) {
		child = currISL->positions[idx & 1023];
		idx++;
		if ((idx & 1023) == 0)
			currISL = currISL->next;
		if (useUndo) { // use the UndoMove lists
			parents = parentsPtr = gGenerateUndoMovesToTierFunPtr(child, gCurrentTier);
			if (dedupHash != NULL) {
				dedupHashElem = 0LL;
				memset(dedupHash, 0, dedupHashBytes);
			}
			for (; parentsPtr != NULL; parentsPtr = parentsPtr->next) {
				parent = gSymmetries ? gCanonicalPosition(gUnDoMoveFunPtr(child, parentsPtr->undomove)) : gUnDoMoveFunPtr(child, parentsPtr->undomove);
				if (gSymmetries && !dedupHashAdd(parent)) continue;
				
				if (parent >= gCurrentTierSize) {
					TIERPOSITION tp; TIER t;
					
					gUnhashToTierPosition(parent, &tp, &t);
					printf("ERROR: %llu generated undo-parent %llu (Tier: %llu, TierPosition: %llu),\n"
					       "which is not in the current tier being solved!\n", child, parent, t, tp);
					ExitStageRight();
				}
				// if childCounts is already 0, we don't mess with this parent
				// (already dealt with OR illegal)
				if (childCounts[parent] != 0) {
					// With losing children, every parent is winning, so we just go through
					// all the parents and declare them winning.
					// Same with tie children.
					if (valueParents == win || valueParents == tie) {
						childCounts[parent] = 0; // reset child counter
						rInsertFR(valueParents, parent, remotenessChild + 1);
						// With winning children, first decrement the child counter by one. If
						// child counter reaches 0, put the parent not in the FR but in the miniFR.
					} else if (valueParents == lose) {
						childCounts[parent] -= 1;
						if (childCounts[parent] != 0) continue;
						miniLoseFR = StorePositionInIList(parent, miniLoseFR);
					}
					SetRemoteness(parent, remotenessChild+1);
					StoreValueOfPosition(parent, valueParents);
					numSolved++;
				}
			}
			FreeUndoMoveList(parents);
		} else { // use the parents pointers
			parentList = rParents[child];
			for (; parentList != NULL; parentList = parentList->next) {
				parent = parentList->position;
				if (childCounts[parent] != 0) {
					if (valueParents == win || valueParents == tie) {
						childCounts[parent] = 0;
						rInsertFR(valueParents, parent, remotenessChild+1);
					} else if (valueParents == lose) {
						childCounts[parent] -= 1;
						if (childCounts[parent] != 0) continue;
						miniLoseFR = StorePositionInIList(parent, miniLoseFR);
					}
					SetRemoteness(parent, remotenessChild+1);
					StoreValueOfPosition(parent, valueParents);
					numSolved++;
				}
			}
		}
		// if we inserted into LOSE, deal with them now
		if (valueParents == lose && miniLoseFR != NULL) {
			LoopyParentsHelper(miniLoseFR, win, remotenessChild+1); // will be freed here too
			miniLoseFR = NULL;
		}
	}
	FreeIPositionList(list); // no longer need it!
}


/************************************************************************
**
** SANITY CHECKERS
**
************************************************************************/

// correctness checker
void checkForCorrectness() {
	BOOLEAN check = TRUE;
	POSITION pos, child;
	REMOTENESS maxWinRem, minLoseRem, minTieRem;
	BOOLEAN seenLose, seenTie, okay;
	MOVELIST *moves, *children;
	VALUE value, valueP, valueC; REMOTENESS remoteness, remotenessC;
	for (pos = 0; pos < gCurrentTierSize; pos++) {
		value = GetValueOfPosition(pos);
		if (value == undecided) {
			if ((checkLegality && !gIsLegalFunPtr(pos)) ||
			    (gSymmetries && pos != gCanonicalPosition(pos)))
				continue; // correct to be undecided
			else {
				printf("CORRUPTION: (%llu) is UNDECIDED, but shouldn't be!\n", pos);
				check = FALSE;
			}
		}
		remoteness = Remoteness(pos);
		valueP = Primitive(pos);

		if (remoteness == 0) { // better be a primitive!
			if (valueP == undecided) {
				printf("CORRUPTION: (%llu) is a non-Primitive with Remoteness 0!\n", pos);
				check = FALSE;
			} else if (value != valueP) {
				printf("CORRUPTION: (%llu) is Primitive with value %s, but db says %s!\n",
				       pos, gValueString[(int)valueP], gValueString[(int)value]);
				check = FALSE;
			}
		} else {
			if (valueP != undecided) {
				printf("CORRUPTION: (%llu) is a Primitive with Remoteness %d, not 0!\n",
				       pos, remoteness);
				check = FALSE;
			} else {
				moves = children = GenerateMoves(pos);
				if (moves == NULL) { // no children!
					printf("CORRUPTION: (%llu) has no GenerateMoves, yet is a %s in %d!\n",
					       pos, gValueString[(int)value], remoteness);
					check = FALSE;
				} else { //the REALLY annoying part, actually checking the children:
					minLoseRem = minTieRem = REMOTENESS_MAX;
					maxWinRem = 0;
					seenLose = seenTie = FALSE; okay = TRUE;
					for (; children != NULL; children = children->next) {
						child = DoMove(pos, children->move);
						if (gSymmetries)
							child = gCanonicalPosition(child);
						valueC = GetValueOfPosition(child);
						if (valueC != undecided) {
							remotenessC = Remoteness(child);
							if (valueC == tie) { //this COULD be tie OR draw
								seenTie = TRUE;
								if (remotenessC < minTieRem)
									minTieRem = remotenessC;
							} else if (valueC == lose) {
								seenLose = TRUE;
								if (remotenessC < minLoseRem)
									minLoseRem = remotenessC;
							} else if (valueC == win) {
								if (remotenessC > maxWinRem)
									maxWinRem = remotenessC;
							}
						} else {
							printf("CORRUPTION: (%llu) has UNDECIDED child, (%llu)!\n", pos, child);
							check = okay = FALSE;
						}
					}
					FreeMoveList(moves);
					if (okay) { // No undecided children
						if (seenLose) { // better be WIN!
							if (value != win || remoteness != minLoseRem+1) {
								printf("CORRUPTION: (%llu) SHOULD be a %s in %d, but it is a %s in %d!\n",
								       pos, gValueString[(int)win], minLoseRem+1, gValueString[(int)value], remoteness);
								check = FALSE;
							}
						} else if (seenTie) {
							if (minTieRem == REMOTENESS_MAX) { // a draw
								if (value != tie || remoteness != minTieRem) {
									printf("CORRUPTION: (%llu) SHOULD be a Draw, but it is a %s in %d!\n",
									       pos, gValueString[(int)value], remoteness);
									check = FALSE;
								}
							} else { // a tie
								if (value != tie || remoteness != minTieRem+1) {
									printf("CORRUPTION: (%llu) SHOULD be a %s in %d, but it is a %s in %d!\n",
									       pos, gValueString[(int)tie], minTieRem+1, gValueString[(int)value], remoteness);
									check = FALSE;
								}
							}
						} else { // better be LOSE!
							if (value != lose || remoteness != maxWinRem+1) {
								printf("CORRUPTION: (%llu) SHOULD be a %s in %d, but it is a %s in %d!\n",
								       pos, gValueString[(int)lose], maxWinRem+1, gValueString[(int)value], remoteness);
								check = FALSE;
							}
						}
					}
				}
			}
		}
	}
	if (check)
		ifprintf(gTierSolvePrint, "Congratulations! No Corruption found!\n");
	else printf("There appears to be some corruption...\n");
}

/* TIER LIST GENERATION SECTION */

// a hashtable datastructure of tiernodes
typedef struct tiernode_list
{
	TIER tier;
	BOOLEAN solved;
	struct tiernode_list* next;
}
TIERNODELIST;

void FreeTierNodeList(TIERNODELIST* ptr){
	TIERNODELIST *last;
	while (ptr != NULL) {
		last = ptr;
		ptr = ptr->next;
		SafeFree((GENERIC_PTR)last);
	}
}

TIERNODELIST *CreateTierNodelistNode(TIER theTier, TIERNODELIST* theNextTier){
	TIERNODELIST *theHead = (TIERNODELIST *) SafeMalloc (sizeof(TIERNODELIST));
	theHead->tier = theTier;
	theHead->solved = FALSE;
	theHead->next = theNextTier;
	return theHead;
}

TIERLIST *list = NULL; // the list to be generated
TIERNODELIST** nodesHashTable = NULL; // a hashtable to keep track of nodes

void initDFSStuff() {
	list = NULL;
	nodesHashTable = (TIERNODELIST**) SafeMalloc(HASHTABLE_BUCKETS * sizeof(TIERNODELIST*));
	unsigned int i;
	for (i = 0; i < HASHTABLE_BUCKETS; i++)
		nodesHashTable[i] = NULL;
}

void freeDFSStuff() {
	FreeTierList(list);
	unsigned int i;
	for (i = 0; i < HASHTABLE_BUCKETS; i++)
		FreeTierNodeList(nodesHashTable[i]);
	if (nodesHashTable != NULL)
		SafeFree(nodesHashTable);
}

// puts a node into the hashtable, returns the node pointer
// invariant: node NOT in hashtable already
TIERNODELIST* putNode(TIER tier) {
	int tierBucket = tier % HASHTABLE_BUCKETS;
	nodesHashTable[tierBucket] =
	        CreateTierNodelistNode(tier, nodesHashTable[tierBucket]);
	return nodesHashTable[tierBucket];
}

// gets node from a hashtable
// if solved, return 1, else return 0
// if the node isn't there, return -1
int getNode(TIER tier) {
	TIERNODELIST* bucket = nodesHashTable[tier % HASHTABLE_BUCKETS];
	for (; bucket != NULL; bucket = bucket->next) {
		if (tier == bucket->tier) {
			if (bucket->solved)
				return 1;
			else return 0;
		}
	}
	return -1;
}

// the actual DFS traverser
BOOLEAN tierDFS(TIER tier, BOOLEAN defineList) {
	TIERLIST *children, *cptr;
	TIER child;
	int childResult;

	TIERNODELIST* node = putNode(tier);

	children = cptr = gTierChildrenFunPtr(tier);
	for (; cptr != NULL; cptr = cptr->next) {
		child = cptr->tier;
		if (tier == child) continue;

		childResult = getNode(child);
		// if child not visited, DFS
		if (childResult == -1) {
			if (!tierDFS(child, defineList)) {
				FreeTierList(children);
				return FALSE;
			}
			// else check if there's a cycle
		} else if (childResult == 0) {
			printf("ERROR! Tier %llu leads back to higher Tier %llu!\n", child, tier);
			FreeTierList(children);
			return FALSE;
		}
	}
	FreeTierList(children);
	node->solved = TRUE;
	if (defineList)
		list = CreateTierlistNode(tier, list);
	return TRUE;
}

// NOW, the actual API functions for checking/generating the tier solve list

// Check tier hierarchy AND define tierSolveList
TIERLIST* checkAndDefineTierTree() {
	initDFSStuff();
	BOOLEAN check = tierDFS(gInitialTier, TRUE);
	TIERLIST* toReturn = CopyTierlist(list);
	freeDFSStuff();
	if (check)
		return toReturn;
	else return NULL;
}

// ONLY check tier hierarchy
BOOLEAN checkTierTree() {
	initDFSStuff();
	BOOLEAN check = tierDFS(gInitialTier, FALSE);
	freeDFSStuff();
	return check;
}