#include "gamesman.h"
#include <stdio.h>

#define MAX_CELLS 12
#define MAX_ADJACENT 6
#define END_GAME_MOVE -9999
#define INVALID_MOVE -10000
#define DEBUG 0

int numCells = 12;

typedef uint64_t POSITION;

// Function prototypes
POSITION HashBoard(int *board, int player);
void UnhashBoard(POSITION hash, int *board, int *player);
BOOLEAN IsAdjacentToOwnStone(int *board, int cell, int player);
int FindNearestEmptyCell(int *board, int startCell, int direction);
MOVE CreatePlaceMove(int cellIndex);
MOVE CreateMoveStoneMove(int fromCell, int toCell);
MOVE CreateEndGameMove(void);

POSITION gPositionsExplored = 0;
POSITION gInitialPosition = 0;
POSITION gNumberOfPositions = 0;
POSITION kBadPosition = -1;

// kUsePureDraw = TRUE;

CONST_STRING kAuthorName         = "Aditya Tummala (ADI)";
CONST_STRING kGameName           = "Abrobad";
CONST_STRING kDBName             = "abrobad";
STRING kDBNameVolatile           = NULL;

BOOLEAN kPartizan                = TRUE;
BOOLEAN kSupportsHeuristic       = FALSE;
BOOLEAN kSupportsSymmetries      = FALSE;
BOOLEAN kSupportsGraphics        = FALSE;
BOOLEAN kDebugMenu               = FALSE;
BOOLEAN kGameSpecificMenu        = FALSE;
BOOLEAN kTieIsPossible           = TRUE;
BOOLEAN kLoopy                   = TRUE;
BOOLEAN kDebugDetermineValue     = FALSE;
void*    gGameSpecificTclInit     = NULL;

CONST_STRING kHelpGraphicInterface = "";

CONST_STRING kHelpTextInterface = "";

CONST_STRING kHelpOnYourTurn = "";

CONST_STRING kHelpStandardObjective = "";

CONST_STRING kHelpReverseObjective = "";

CONST_STRING kHelpTieOccursWhen = "";

CONST_STRING kHelpExample = "";


void DebugMenu() {
    printf("\nDebug Menu:\n");
    printf("No debugging options are implemented.\n");
    printf("Press Enter to continue.\n");
    getchar();
}

USERINPUT GetAndPrintPlayersMove(POSITION position, MOVE *move, STRING playerName) {
    char input[MAXINPUTLENGTH];
    MOVE tmpMove;
    MOVELIST *legalMoves, *current;
    BOOLEAN isValid;
    
    // Generate the list of legal moves once
    legalMoves = GenerateMoves(position);

    // Check if there are no legal moves
    if (legalMoves == NULL) {
        printf("No legal moves available.\n");
        return Abort; // Or handle appropriately
    }

    // Check if there's only one move and auto-select if desired
    if (gSkipInputOnSingleMove && legalMoves->next == NULL) {
        *move = legalMoves->move;
        MoveToString(*move, input);
        printf("\n---------- SELECTING THE ONLY MOVE ---------> %s\n", input);
        FreeMoveList(legalMoves);
        return Move;
    }

    // Prompt the player for input
    while (TRUE) {
        printf("%8s's move: ", playerName);
        GetMyStr(input, MAXINPUTLENGTH);

        // Handle empty input
        if (input[0] == '\0') {
            printf("Please enter a move.\n");
            continue;
        }

        // Handle special commands
        if (strcmp(input, "q") == 0 || strcmp(input, "Q") == 0) {
            ExitStageRight();
            exit(0);
        } else if (strcmp(input, "h") == 0 || strcmp(input, "H") == 0) {
            HelpMenus();
            continue;
        } else if (strcmp(input, "?") == 0) {
            PrintPossibleMoves(position);
            continue;
        }

        // Validate and convert input to a move
        if (ValidTextInput(input)) {
            tmpMove = ConvertTextInputToMove(input);
            isValid = FALSE;

            // Check if the move is in the list of legal moves
            current = legalMoves;
            while (current != NULL) {
                if (current->move == tmpMove) {
                    isValid = TRUE;
                    break;
                }
                current = current->next;
            }

            if (isValid) {
                *move = tmpMove;
                FreeMoveList(legalMoves);
                return Move;
            } else {
                printf("Invalid move. Please enter a legal move.\n");
                PrintPossibleMoves(position);
            }
        } else {
            printf("Invalid input format. Please try again.\n");

        }
    }

    // Free the list of legal moves
    FreeMoveList(legalMoves);
    return Continue;
}


void MoveToAutoGUIString() {
}

// Create a move for placing a stone in a cell (values from 1 to 12)
MOVE CreatePlaceMove(int cellIndex) {
    return cellIndex;  // Return values 1 to 12
}

MOVE CreateEndGameMove() { return END_GAME_MOVE; }

MOVE CreateMoveStoneMove(int fromCell, int toCell) {
    if (fromCell == toCell) {
        printf("Error: Cannot move stone to the same cell\n");
        return INVALID_MOVE;
    }

    int toCellIndex = toCell - 1;
    if (toCell > fromCell) {
        toCellIndex -= 1; // Adjust index to skip fromCell
    }

    int moveIndex = (fromCell - 1) * 11 + toCellIndex;
    return moveIndex + 13; // Add 13 to avoid conflict with place moves
}

void DecodeMoveStoneMove(MOVE move, int *fromCell, int *toCell) {
    int moveIndex = move - 13;
    *fromCell = (moveIndex / 11) + 1;
    int toCellIndex = moveIndex % 11;

    *toCell = toCellIndex + 1;
    if (*toCell >= *fromCell) {
        *toCell += 1; // Adjust to skip fromCell
    }
}

void MoveToString(MOVE move, char *moveStringBuffer) {
    if (move == 0) {
        // End game move
        strcpy(moveStringBuffer, "End Game");
    } else if (move <= 12) {
        // It's a place move
        sprintf(moveStringBuffer, "Place stone at cell %d", move);
    } else {
        // It's a move stone move
        int moveIndex = move - 13;  // Adjust for the starting point
        int fromCell = (moveIndex / 11) + 1;
        int toCell = (moveIndex % 11) + 1;
        sprintf(moveStringBuffer, "Move stone from cell %d to cell %d", fromCell, toCell);
    }
}

int NumberOfOptions() {
    return 1;
}

void PositionToAutoGUIString() {
}


void PrintComputersMove(MOVE computersMove, STRING computersName) {
    // char moveString[100];
    // MoveToString(computersMove, moveString);
    // printf("%s's move: %s\n", computersName, moveString);
}


void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn) {
    int board[MAX_CELLS];
    int currentPlayer;
    UnhashBoard(position, board, &currentPlayer);

    printf("\nCurrent board position:\n\n");

    // Row 0: Cells 0 and 1
    printf("        ");
    for (int i = 0; i <= 1; i++) {
        int cellValue = board[i];
        if (cellValue == 0) {
            printf(" %2d", 0);
        } else {
            printf("  %d", cellValue);
        }
    }
    printf("\n");

    // Row 1: Cells 2, 3, and 4
    printf("      ");
    for (int i = 2; i <= 4; i++) {
        int cellValue = board[i];
        if (cellValue == 0) {
            printf(" %2d", 0);
        } else {
            printf("  %d", cellValue);
        }
    }
    printf("\n");

    // Row 2: Cells 5, 6, 7, and 8
    printf("    ");
    for (int i = 5; i <= 8; i++) {
        int cellValue = board[i];
        if (cellValue == 0) {
            printf(" %2d", 0);
        } else {
            printf("  %d", cellValue);
        }
    }
    printf("\n");

    // Row 3: Cells 9, 10, and 11
    printf("      ");
    for (int i = 9; i <= 11; i++) {
        int cellValue = board[i];
        if (cellValue == 0) {
            printf(" %2d", 0);
        } else {
            printf("  %d", cellValue);
        }
    }
    printf("\n");

    // Print whose turn it is
    printf("\nIt is %s's turn.\n", playerName);

    // Display input instructions for the user
    if (usersTurn) {
        printf("Enter your move.\n");
    } else {
        printf("Waiting for opponent's move.\n");
    }
}




void PositionToString(POSITION position, char *positionStringBuffer) {
    int board[MAX_CELLS];
    int player;
    UnhashBoard(position, board, &player);

    // Build the board string with row separators
    char boardString[MAX_POSITION_STRING_LENGTH];
    int index = 0;

    // Row 0: Cells 0 and 1
    for (int i = 0; i <= 1; i++) {
        boardString[index++] = '0' + board[i];
        if (i < 1) {
            boardString[index++] = ' ';
        }
    }
    boardString[index++] = '|'; // Row separator

    // Row 1: Cells 2, 3, and 4
    for (int i = 2; i <= 4; i++) {
        boardString[index++] = '0' + board[i];
        if (i < 4) {
            boardString[index++] = ' ';
        }
    }
    boardString[index++] = '|'; // Row separator

    // Row 2: Cells 5, 6, 7, and 8
    for (int i = 5; i <= 8; i++) {
        boardString[index++] = '0' + board[i];
        if (i < 8) {
            boardString[index++] = ' ';
        }
    }
    boardString[index++] = '|'; // Row separator

    // Row 3: Cells 9, 10, and 11
    for (int i = 9; i <= 11; i++) {
        boardString[index++] = '0' + board[i];
        if (i < 11) {
            boardString[index++] = ' ';
        }
    }
    boardString[index] = '\0'; // Null-terminate the string

    // Create the position string: player_boardString
    snprintf(positionStringBuffer, MAX_POSITION_STRING_LENGTH, "%d_%s", player, boardString);
}


POSITION StringToPosition(char *positionString) {
    // Expected format: "player_boardString"
    // where boardString has rows separated by '|', cells separated by spaces
    char *underscorePtr = strchr(positionString, '_');
    if (underscorePtr == NULL) {
        return NULL_POSITION;
    }

    // Parse player turn
    char turnChar = positionString[0];
    int turn = turnChar - '0';
    if (turn != 1 && turn != 2) {
        return NULL_POSITION;
    }

    char *boardString = underscorePtr + 1;
    int board[MAX_CELLS];
    int index = 0;

    // Copy boardString to a temporary buffer for tokenization
    char tempBoardString[MAX_POSITION_STRING_LENGTH];
    strncpy(tempBoardString, boardString, MAX_POSITION_STRING_LENGTH);
    tempBoardString[MAX_POSITION_STRING_LENGTH - 1] = '\0'; // Ensure null-termination

    // Tokenize the board string by rows
    char *rowToken = strtok(tempBoardString, "|");
    while (rowToken != NULL && index < numCells) {
        // Tokenize the row by spaces
        char *cellToken = strtok(rowToken, " ");
        while (cellToken != NULL && index < numCells) {
            if (cellToken[0] >= '0' && cellToken[0] <= '2' && cellToken[1] == '\0') {
                board[index++] = cellToken[0] - '0';
            } else {
                return NULL_POSITION; // Invalid cell value
            }
            cellToken = strtok(NULL, " ");
        }
        rowToken = strtok(NULL, "|");
    }

    if (index != numCells) {
        return NULL_POSITION; // Not enough cell values
    }

    return HashBoard(board, turn);
}


int getOption() {
    return 0;
}

void setOption(int option) {
}


int board[MAX_CELLS];
int adjacency[MAX_CELLS][MAX_ADJACENT];
int directions[MAX_CELLS][MAX_ADJACENT][MAX_CELLS];
int numAdjacent[MAX_CELLS];
int numDirections[MAX_CELLS];
BOOLEAN gameEnded = FALSE;
int currentPlayer = 1;

void GameSpecificMenu() {
    printf("\nGame Specific Options:\n");
    printf("No game-specific options are available.\n");
    printf("Press Enter to continue.\n");
    getchar();
}


void CheckAndPrintPositionsExplored() {
    if (gPositionsExplored % 250 == 0 && gPositionsExplored > 0) {
        printf("Positions explored: " POSITION_FORMAT "\n", gPositionsExplored);
    }
}

void InitializeGame() {
	if (DEBUG) {
		printf("InitializeGame Start \n");
	}
    for (int i = 0; i < numCells; i++) {
        board[i] = 0;
    }
    numAdjacent[0] = 3;
    adjacency[0][0] = 1;
    adjacency[0][1] = 2;
    adjacency[0][2] = 3;
    
    // Directions list for rearrangements.
        numDirections[0] = 3;
        directions[0][0][0] = 1;
        directions[0][1][0] = 3;
        directions[0][1][1] = 7;
        directions[0][1][2] = 11;
        directions[0][2][0] = 2;
        directions[0][2][1] = 5;

    numAdjacent[1] = 3;
    adjacency[1][0] = 0;
    adjacency[1][1] = 3;
    adjacency[1][2] = 4;

        numDirections[1] = 3;
        directions[1][0][0] = 4;
        directions[1][0][1] = 8;
        directions[1][1][0] = 3;
        directions[1][1][1] = 6;
        directions[1][1][2] = 9;
        directions[1][2][0] = 0;
    

    numAdjacent[2] = 4;
    adjacency[2][0] = 0;
    adjacency[2][1] = 3;
    adjacency[2][2] = 5;
    adjacency[2][3] = 6;

        numDirections[2] = 4;
        directions[2][0][0] = 0;
        directions[2][1][0] = 3;
        directions[2][1][1] = 4;
        directions[2][2][0] = 6;
        directions[2][2][1] = 10;
        directions[2][3][0] = 5;

    numAdjacent[3] = 6;
    adjacency[3][0] = 0;
    adjacency[3][1] = 1;
    adjacency[3][2] = 2;
    adjacency[3][3] = 4;
    adjacency[3][4] = 6;
    adjacency[3][5] = 7;

        numDirections[3] = 6;
        directions[3][0][0] = 1;;
        directions[3][1][0] = 4;
        directions[3][2][0] = 7;
        directions[3][2][1] = 11;
        directions[3][3][0] = 6;
        directions[3][3][1] = 9;
        directions[3][4][0] = 2;
        directions[3][5][0] = 0;

    numAdjacent[4] = 4;  // Removed self-reference
    adjacency[4][0] = 1;
    adjacency[4][1] = 3;
    adjacency[4][2] = 7;
    adjacency[4][3] = 8;

        numDirections[4] = 4;
        directions[4][0][0] = 8;
        directions[4][1][0] = 7;
        directions[4][1][1] = 10;
        directions[4][2][0] = 3;
        directions[4][2][1] = 2;
        directions[4][3][0] = 1;

    numAdjacent[5] = 3;
    adjacency[5][0] = 2;
    adjacency[5][1] = 6;
    adjacency[5][2] = 9;

        numDirections[5] = 3;
        directions[5][0][0] = 2;
        directions[5][0][1] = 0;
        directions[5][1][0] = 6;
        directions[5][1][1] = 7;
        directions[5][1][2] = 8;
        directions[5][2][0] = 9;

    numAdjacent[6] = 6;
    adjacency[6][0] = 2;
    adjacency[6][1] = 3;
    adjacency[6][2] = 5;
    adjacency[6][3] = 7;
    adjacency[6][4] = 9;
    adjacency[6][5] = 10;

        numDirections[6] = 6;
        directions[6][0][0] = 3;
        directions[6][0][1] = 1;
        directions[6][1][0] = 7;
        directions[6][1][1] = 8;
        directions[6][2][0] = 10;
        directions[6][3][0] = 9;
        directions[6][4][0] = 5;
        directions[6][5][0] = 2;

    numAdjacent[7] = 6;  // Removed self-reference
    adjacency[7][0] = 3;
    adjacency[7][1] = 4;
    adjacency[7][2] = 6;
    adjacency[7][3] = 8;
    adjacency[7][4] = 10;
    adjacency[7][5] = 11;

        numDirections[7] = 6;
        directions[7][0][0] = 4;
        directions[7][1][0] = 8;
        directions[7][2][0] = 11;
        directions[7][3][0] = 10;
        directions[7][4][0] = 6;
        directions[7][4][1] = 5;
        directions[7][5][0] = 3;
        directions[7][5][1] = 0;

    numAdjacent[8] = 3;  // Adjusted indices and removed out-of-bounds
    adjacency[8][0] = 4;
    adjacency[8][1] = 7;
    adjacency[8][2] = 11;

        numDirections[8] = 3;
        directions[8][0][0] = 11;
        directions[8][1][0] = 7;
        directions[8][1][1] = 6;
        directions[8][1][2] = 5;
        directions[8][2][0] = 4;
        directions[8][2][1] = 1;

    numAdjacent[9] = 3;
    adjacency[9][0] = 5;
    adjacency[9][1] = 6;
    adjacency[9][2] = 10;

        numDirections[9] = 3;
        directions[9][0][0] = 6;
        directions[9][0][1] = 3;
        directions[9][0][2] = 1;
        directions[9][1][0] = 10;
        directions[9][1][1] = 11;
        directions[9][2][0] = 5;

    numAdjacent[10] = 4;
    adjacency[10][0] = 6;
    adjacency[10][1] = 7;
    adjacency[10][2] = 9;
    adjacency[10][2] = 11;

        numDirections[10] = 4;
        directions[10][0][0] = 7;
        directions[10][0][1] = 4;
        directions[10][1][0] = 11;
        directions[10][2][0] = 9;
        directions[10][3][0] = 6;
        directions[10][3][1] = 2;

    numAdjacent[11] = 3;
    adjacency[11][0] = 7;
    adjacency[11][1] = 8;
    adjacency[11][2] = 10;

        numDirections[11] = 3;
        directions[9][0][0] = 8;
        directions[9][1][0] = 10;
        directions[9][1][1] = 9;
        directions[9][2][0] = 7;
        directions[9][2][1] = 3;
        directions[9][2][2] = 0;

    gInitialPosition = HashBoard(board, currentPlayer);
    gNumberOfPositions = pow(3, numCells) * 2;
    // gSupportsMex = TRUE;
    CheckAndPrintPositionsExplored();
    if (DEBUG) {
		printf("END init Game \n");
	}
}

#include <stdint.h>

POSITION HashBoard(int *board, int player) {
    POSITION hash = 0;
    for (int i = 0; i < numCells; i++) {
        hash *= 3; // Since each cell can be 0, 1, or 2
        hash += board[i];
    }
    hash *= 2; // To account for the player turn
    hash += (player - 1);
    return hash;
}

void UnhashBoard(POSITION hash, int *board, int *player) {
    *player = (hash % 2) + 1;
    hash /= 2;
    for (int i = numCells - 1; i >= 0; i--) {
        board[i] = hash % 3;
        hash /= 3;
    }
}


MOVELIST *GenerateMoves(POSITION position) {
    MOVELIST *moves = NULL;
    int board[MAX_CELLS];
    int player;
    UnhashBoard(position, board, &player);

    printf("Generating moves for position: ");
    for (int i = 0; i < numCells; i++) printf("%d ", board[i]);
    printf(" | Player: %d\n", player);

    BOOLEAN canPlace = FALSE;

    // Generate placement moves
    for (int i = 0; i < numCells; i++) {
        if (board[i] == 0 && !IsAdjacentToOwnStone(board, i, player)) {
            canPlace = TRUE;
            MOVE move = CreatePlaceMove(i + 1);
            moves = CreateMovelistNode(move, moves);
        }
    }

    // If no placement moves, generate movement moves and add end game move
    if (!canPlace) {
        // Generate movement moves
        for (int i = 0; i < numCells; i++) {
            if (board[i] == player) {
                int numDirs = numDirections[i];
                for (int dir = 0; dir < numDirs; dir++) {
                    int dest = FindNearestEmptyCell(board, i, dir);
                    if (dest != -1) {
                        MOVE move = CreateMoveStoneMove(i + 1, dest + 1);
                        if (move != INVALID_MOVE) {
                            moves = CreateMovelistNode(move, moves);
                        }
                    }
                }
            }
        }
        // Add the end game move regardless of movement moves
        MOVE move = CreateEndGameMove();
        moves = CreateMovelistNode(move, moves);
    }

    printf("Generated moves: ");
    MOVELIST *currentMove = moves;
    while (currentMove != NULL) {
        printf("%d ", currentMove->move);
        currentMove = currentMove->next;
    }
    printf("\n");
    return moves;
}

/* 
numAdjacent[0] = 3;
adjacency[0][0] = 1;
adjacency[0][1] = 2;
adjacency[0][2] = 3;

// Directions list for rearrangements.
    numDirections[0] = 3;
    directions[0][0][0] = 1;
    directions[0][1][0] = 3;
    directions[0][1][1] = 7;
    directions[0][1][2] = 11;
    directions[0][2][0] = 2;
    directions[0][2][1] = 5;
*/

int FindNearestEmptyCell(int *board, int startCell, int direction) {
    int numCells = sizeof(directions[startCell][direction]);
    for (int i = 0; i < numCells; i++) {
        int nextCell = directions[startCell][direction][i];
        if (board[nextCell] == 0) {
            return nextCell; // Found the nearest empty cell
        }
    }
    return -1; // No empty cell found in this direction
}

BOOLEAN IsAdjacentToOwnStone(int *board, int cell, int player) {
    for (int i = 0; i < numAdjacent[cell]; i++) {
        int adjCell = adjacency[cell][i];
        if (board[adjCell] == player) {
            return TRUE;
        }
    }
    return FALSE;
}


// MOVE CreatePlaceMove(int cellIndex) { return -cellIndex - 1; }

// MOVE CreateMoveStoneMove(int fromCell, int toCell) { return (fromCell * numCells) + toCell; }

// MOVE CreateEndGameMove() { return END_GAME_MOVE; }

void PrintBinary(POSITION value, int numBits) {
    for (int i = numBits - 1; i >= 0; i--) {
        printf("%d", (value >> i) & 1);  // Extract each bit and print it
    }
    printf("\n");
}

POSITION DoMove(POSITION position, MOVE move) {
    gPositionsExplored++;
    CheckAndPrintPositionsExplored();

    int board[MAX_CELLS];
    int player;
    UnhashBoard(position, board, &player);

    if (move == END_GAME_MOVE) {
        gameEnded = TRUE;
        printf("End game move applied.\n");
    } else if (move >= 1 && move <= 12) {
        int cellIndex = move - 1;
        if (board[cellIndex] != 0) {
            printf("Error: Cell %d is already occupied.\n", cellIndex + 1);
            return position;
        }
        board[cellIndex] = player;
    } else if (move >= 13 && move < 145) { // Valid move stone moves
        int fromCell, toCell;
        DecodeMoveStoneMove(move, &fromCell, &toCell);

        if (board[fromCell - 1] != player) {
            printf("Error: No stone of player %d at cell %d\n", player, fromCell);
            return position;
        }
        if (board[toCell - 1] != 0) {
            printf("Error: Destination cell %d is not empty\n", toCell);
            return position;
        }

        board[fromCell - 1] = 0; // Remove stone from fromCell
        board[toCell - 1] = player; // Place stone at toCell
        printf("Moved stone from cell %d to cell %d\n", fromCell, toCell);
    } else {
        printf("Error: Invalid move value: %d\n", move);
        return position; // Return the same position if the move is invalid
    }

    // Switch the player
    player = (player == 1) ? 2 : 1;

    return HashBoard(board, player);
}


void DFS(int player, int cell, int *visited, int *board) {
    if (visited[cell] || board[cell] != player) { return; }
    visited[cell] = 1;
    for (int i = 0; i < numAdjacent[cell]; i++) {
        int nextCell = adjacency[cell][i];
        DFS(player, nextCell, visited, board);
    }
}

int CountPlayerGroups(int player, int *board) {
    int groupCount = 0;
    int visited[MAX_CELLS] = {0};
    for (int i = 0; i < numCells; i++) {
        if (board[i] == player && !visited[i]) {
            DFS(player, i, visited, board);
            groupCount++;
        }
    }
    return groupCount;
}
VALUE Primitive(POSITION position) {
    int board[MAX_CELLS];
    int currentPlayer;
    UnhashBoard(position, board, &currentPlayer);

    if (!gameEnded) {
        return undecided; // The game hasn't ended yet
    }

    // Count the number of groups for both players in the current position
    int playerGroups = CountPlayerGroups(currentPlayer, board);
    int opponent = (currentPlayer == 1) ? 2 : 1;
    int opponentGroups = CountPlayerGroups(opponent, board);

    if (playerGroups <= opponentGroups) {
        return win;
    } else {
        return lose;
    }
}


BOOLEAN ValidTextInput(STRING input) {
    while (isspace(*input)) input++;
    char *endPtr;

    if (strcmp(input, "0") == 0) {
        return TRUE;
    }

    int firstNum = strtol(input, &endPtr, 10);
    if (endPtr == input || firstNum < 1 || firstNum > numCells) {
        return FALSE;
    }

    while (isspace(*endPtr)) endPtr++;

    if (*endPtr == '\0') {
        return TRUE;
    }
    int secondNum = strtol(endPtr, &endPtr, 10);
    if (endPtr == input || secondNum < 1 || secondNum > numCells) {
        return FALSE;
    }
    while (isspace(*endPtr)) endPtr++;

    if (*endPtr == '\0') { return TRUE; }
    return FALSE;
}

MOVE ConvertTextInputToMove(STRING input) {
    while (isspace(*input)) input++;
    char *endPtr;

    if (strcmp(input, "0") == 0) {
        return CreateEndGameMove();  // User entered '0' to end the game
    }

    int firstNum = strtol(input, &endPtr, 10);
    if (endPtr == input || firstNum < 1 || firstNum > 12) {
        return INVALID_MOVE;  // Invalid move
    }
    int firstIndex = firstNum;

    while (isspace(*endPtr)) endPtr++;

    if (*endPtr == '\0') {
        return CreatePlaceMove(firstIndex);  // If only one number, it's a place move
    }

    int secondNum = strtol(endPtr, &endPtr, 10);
    if (endPtr == input || secondNum < 1 || secondNum > 12 || secondNum == firstIndex) {
        return INVALID_MOVE;  // Invalid move
    }
    int secondIndex = secondNum;

    return CreateMoveStoneMove(firstIndex, secondIndex);  // Move stone from firstIndex to secondIndex
}

