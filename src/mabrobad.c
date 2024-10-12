#include "gamesman.h"

#define MAX_CELLS 12
#define MAX_ADJACENT 6
#define END_GAME_MOVE -9999
#define INVALID_MOVE -10000

CONST_STRING kAuthorName = "Aditya Tummala (ADI)";
CONST_STRING kGameName = "Abrobad";
CONST_STRING kDBName = "abrobad";
BOOLEAN kLoopy = TRUE;
BOOLEAN kTieIsPossible = TRUE;

int numCells = 12;

int board[MAX_CELLS];
int adjacency[MAX_CELLS][MAX_ADJACENT];
int numAdjacent[MAX_CELLS];
BOOLEAN gameEnded = FALSE;
int currentPlayer = 1;

void GameSpecificMenu() {
  char GetMyChar();

  
}

void InitializeGame() {
    for (int i = 0; i < numCells; i++) {
        board[i] = 0;
    }
    numAdjacent[0] = 3;
    adjacency[0][0] = 1;
    adjacency[0][1] = 2;
    adjacency[0][2] = 3;

    numAdjacent[1] = 3;
    adjacency[1][0] = 0;
    adjacency[1][1] = 3;
    adjacency[1][2] = 4;

    numAdjacent[2] = 3;
    adjacency[2][0] = 0;
    adjacency[2][1] = 4;
    adjacency[2][2] = 5;

    numAdjacent[3] = 4;
    adjacency[3][0] = 1;
    adjacency[3][1] = 6;
    adjacency[3][2] = 7;
    adjacency[3][3] = 4;

    numAdjacent[4] = 6;
    adjacency[4][0] = 1;
    adjacency[4][1] = 2;
    adjacency[4][2] = 3;
    adjacency[4][3] = 5;
    adjacency[4][4] = 7;
    adjacency[4][5] = 8;

    numAdjacent[5] = 4;
    adjacency[5][0] = 2;
    adjacency[5][1] = 4;
    adjacency[5][2] = 8;
    adjacency[5][3] = 9;

    numAdjacent[6] = 3;
    adjacency[6][0] = 3;
    adjacency[6][1] = 7;
    adjacency[6][2] = 10;

    numAdjacent[7] = 6;
    adjacency[7][0] = 3;
    adjacency[7][1] = 4;
    adjacency[7][2] = 6;
    adjacency[7][3] = 8;
    adjacency[7][4] = 10;
    adjacency[7][5] = 11;

    numAdjacent[8] = 6;
    adjacency[8][0] = 4;
    adjacency[8][1] = 5;
    adjacency[8][2] = 7;
    adjacency[8][3] = 9;
    adjacency[8][4] = 11;
    adjacency[8][5] = 12;

    numAdjacent[9] = 3;
    adjacency[9][0] = 5;
    adjacency[9][1] = 8;
    adjacency[9][2] = 11;

    numAdjacent[10] = 3;
    adjacency[10][0] = 6;
    adjacency[10][1] = 7;
    adjacency[10][2] = 11;

    numAdjacent[11] = 4;
    adjacency[11][0] = 10;
    adjacency[11][1] = 8;
    adjacency[11][2] = 9;
    adjacency[11][3] = 12;

    gInitialPosition = HashBoard(board, currentPlayer);
    gNumberOfPositions = pow(3, numCells) * 2;
    gSupportsMex = TRUE;
}

POSITION HashBoard(int *board, int player) {
    POSITION hash = 0;
    POSITION power = 1;
    for (int i = 0; i < numCells; i++) {
        hash += board[i] * power;
        power *= 3;
    }
    if (player == 2) {
        hash += power;
    }
    return hash;
}

void UnhashBoard(POSITION hash, int *board, int *player) {
    POSITION power = 1;
    POSITION totalPower = 1;
    for (int i = 0; i < numCells; i++) { totalPower *= 3; }
    if (hash >= totalPower) {
        *player = 2;
        hash -= totalPower;
    } else { *player = 1; }
    for (int i = 0; i < numCells; i++) {
        board[i] = hash % 3;
        hash /= 3;
    }
}

MOVELIST *GenerateMoves(POSITION position) {
    MOVELIST *moves = NULL;
    int board[MAX_CELLS];
    int player;
    UnhashBoard(position, board, &player);
    BOOLEAN canPlace = FALSE;
    for (int i = 0; i < numCells; i++) {
        if (board[i] == 0 && !IsAdjacentToOwnStone(board, i, player)) {
            canPlace = TRUE;
            MOVE move = CreatePlaceMove(i);
            moves = CreateMovelistNode(move, moves);
        }
    }
    if (!canPlace) {
        for (int i = 0; i < numCells; i++) {
            if (board[i] == player) {
                for (int dir = 0; dir < numAdjacent[i]; dir++) {
                    int dest = FindNearestEmptyCell(board, i, dir);
                    if (dest != -1) {
                        MOVE move = CreateMoveStoneMove(i, dest);
                        moves = CreateMovelistNode(move, moves);
                    }
                }
            }
        }
        MOVE move = CreateEndGameMove();
        moves = CreateMovelistNode(move, moves);
    }
    return moves;
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

int FindNearestEmptyCell(int *board, int startCell, int direction) {
    int currentCell = startCell;
    while (TRUE) {
        int nextCell = adjacency[currentCell][direction];
        if (nextCell == -1) { return -1; }
        if (board[nextCell] == 0) { return nextCell; }
        currentCell = nextCell;
    }
}

MOVE CreatePlaceMove(int cellIndex) { return -cellIndex - 1; }

MOVE CreateMoveStoneMove(int fromCell, int toCell) { return (fromCell * numCells) + toCell; }

MOVE CreateEndGameMove() { return END_GAME_MOVE; }

POSITION DoMove(POSITION position, MOVE move) {
    int board[MAX_CELLS];
    int player;
    UnhashBoard(position, board, &player);
    if (move == END_GAME_MOVE) {
        gameEnded = TRUE;
    } else if (move < 0) {
        int cellIndex = -move - 1;
        board[cellIndex] = player;
    } else {
        int fromCell = move / numCells;
        int toCell = move % numCells;
        board[fromCell] = 0;
        board[toCell] = player;
    }
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
    if (!gameEnded) { return undecided; }
    int playerGroups = CountPlayerGroups(currentPlayer, board);
    int filledBoard[MAX_CELLS];
    int opponent = (currentPlayer == 1) ? 2 : 1;
    for (int i = 0; i < numCells; i++) {
        if (board[i] == 0) { filledBoard[i] = opponent;} 
        else { filledBoard[i] = board[i]; }
    }
    int opponentGroups = CountPlayerGroups(opponent, filledBoard);

    if (playerGroups <= opponentGroups) { return (currentPlayer == 1) ? win : lose;
    } else { return (currentPlayer == 1) ? lose : win; }
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

    if (strcmp(input, "0") == 0) { return CreateEndGameMove(); }

    int firstNum = strtol(input, &endPtr, 10);
    if (endPtr == input) { return INVALID_MOVE; }
    int firstIndex = firstNum - 1;

    while (isspace(*endPtr)) endPtr++;

    if (*endPtr == '\0') { return CreatePlaceMove(firstIndex); }

    int secondNum = strtol(endPtr, &endPtr, 10);
    if (endPtr == input) {
        return INVALID_MOVE;
    }
    int secondIndex = secondNum - 1;
    while (isspace(*endPtr)) endPtr++;
    if (*endPtr == '\0') { return CreateMoveStoneMove(firstIndex, secondIndex); }
    return INVALID_MOVE;
}

