#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace config {

    constexpr int CELL_SIZE = 100;

    // One selection slot per player/color. Bump this and extend playerIndexOf
    // (below, near colorOf/pieceOf) when the game grows past two colors.
    constexpr int NUM_PLAYERS = 2;

    struct PieceStats {
        //2 זמן המנוחה קבוע לכל כלי בלי תלות בפעולה 
        //1. כרגע התחשבנו במהירות תזוזה לכל כלי לפי מהירות תזוזה למשבצת לאותו הכלי 
        double speedCellsPerSec; 
        long   restMs;            
    };

    //נחליף את המהירויות ברגע שנקבל אותן
    inline PieceStats statsFor(char piece) {
        switch (piece) {
            case 'Q': return {4.0, 0};
            case 'R': return {3.0, 0};
            case 'B': return {3.0, 0};
            case 'N': return {3.5, 0};
            case 'K': return {3.0, 0};
            case 'P': return {2.0, 0};
            default:  return {0.0, 0}; 
        }
    }
}

struct Board {
    std::vector<std::vector<std::string>> grid;
    int rows() const { return (int)grid.size(); }
    int cols() const { return grid.empty() ? 0 : (int)grid[0].size(); }
};

struct Selection {
    bool active = false;
    int  row = 0, col = 0;
    long selectedAtMs = 0;   // tie-break: which pending selection an ambiguous click completes
};

struct PieceMove {
    int         fromRow, fromCol;
    int         toRow, toCol;
    long        startMs;
    long        durationMs;
    std::string piece;          // identity travels inside the move, not on the board
};

struct GameState {
    Board                  board;
    long                   elapsedMs = 0;
    std::vector<Selection> selections;   // one per player/color, indexed by playerIndexOf
    std::vector<PieceMove> activeMoves;
};

class BoardError : public std::runtime_error {
public:
    explicit BoardError(const std::string& code)
        : std::runtime_error(code), code_(code) {}
    const std::string& code() const { return code_; }
private:
    std::string code_;
};

std::string trim(const std::string& v) {
    size_t a = 0, b = v.size();
    while (a < b && std::isspace((unsigned char)v[a])) ++a;
    while (b > a && std::isspace((unsigned char)v[b - 1])) --b;
    return v.substr(a, b - a);
}

std::vector<std::string> splitWords(const std::string& line) {
    std::vector<std::string> out;
    std::istringstream ss(line);
    std::string tok;
    while (ss >> tok) out.push_back(tok);
    return out;
}

struct Sections {
    std::vector<std::string> boardLines;
    std::vector<std::string> commandLines;
};

Sections parseSections(const std::string& text) {
    Sections s;
    std::istringstream stream(text);
    std::string line;
    enum { NONE, BOARD, COMMANDS } where = NONE;

    while (std::getline(stream, line)) {
        std::string t = trim(line);
        if (t == "Board:")    { where = BOARD;    continue; }
        if (t == "Commands:") { where = COMMANDS; continue; }
        if (t.empty())        { continue; }
        if (where == BOARD)         s.boardLines.push_back(t);
        else if (where == COMMANDS) s.commandLines.push_back(t);
    }
    return s;
}

Board parseBoard(const std::vector<std::string>& boardLines) {
    Board b;
    for (const auto& line : boardLines) b.grid.push_back(splitWords(line));
    return b;
}

bool isValidToken(const std::string& t) {
    if (t == ".") return true;
    if (t.size() != 2) return false;
    if (t[0] != 'w' && t[0] != 'b') return false;
    switch (t[1]) {
        case 'K': case 'Q': case 'R':
        case 'B': case 'N': case 'P': return true;
        default: return false;
    }
}

void validateBoard(const Board& b) {
    if (b.grid.empty()) return;

    size_t expected = b.grid[0].size();
    for (const auto& row : b.grid)                 // structural check
        if (row.size() != expected) throw BoardError("ROW_WIDTH_MISMATCH");

    for (const auto& row : b.grid)                 // token check
        for (const auto& tok : row)
            if (!isValidToken(tok)) throw BoardError("UNKNOWN_TOKEN");
}

std::string formatBoard(const Board& b) {
    std::ostringstream out;
    for (const auto& row : b.grid) {
        for (size_t j = 0; j < row.size(); ++j) {
            if (j) out << ' ';
            out << row[j];
        }
        out << '\n';
    }
    return out.str();
}



bool isEmpty(const std::string& tok) { return tok == "."; }
char colorOf(const std::string& tok) { return tok[0]; }
char pieceOf(const std::string& tok) { return tok[1]; }

// Maps a board color to its selection-array slot. Add cases here (and bump
// config::NUM_PLAYERS) the day the game supports more than two colors.
int playerIndexOf(char color) {
    switch (color) {
        case 'w': return 0;
        case 'b': return 1;
        default:  return -1;
    }
}

double cellDistance(int r1, int c1, int r2, int c2) {
    double dr = r2 - r1, dc = c2 - c1;
    return std::sqrt(dr * dr + dc * dc);
}

bool isLegalMove(const Board& /*board*/, const PieceMove& /*move*/, char /*piece*/) {
    return true;
}

void resolveMoves(GameState& st) {
    // Split into moves that have arrived and moves still in the air.
    std::vector<size_t>    due;
    std::vector<PieceMove> stillMoving;
    for (size_t i = 0; i < st.activeMoves.size(); ++i) {
        const PieceMove& m = st.activeMoves[i];
        if (st.elapsedMs >= m.startMs + m.durationMs) due.push_back(i);
        else                                          stillMoving.push_back(m);
    }

    // Deterministic landing order: earliest arrival first, launch order breaks ties.
    std::sort(due.begin(), due.end(), [&](size_t a, size_t b) {
        long ta = st.activeMoves[a].startMs + st.activeMoves[a].durationMs;
        long tb = st.activeMoves[b].startMs + st.activeMoves[b].durationMs;
        if (ta != tb) return ta < tb;
        return a < b;
    });

    for (size_t idx : due) {
        const PieceMove& m = st.activeMoves[idx];
        std::string& target = st.board.grid[m.toRow][m.toCol];   // never touch the source
        if (isEmpty(target) || colorOf(target) != m.piece[0]) {
            target = m.piece;                                    // land on empty, or capture an enemy
        } else {                                                 // a friendly is sitting there
            std::string& origin = st.board.grid[m.fromRow][m.fromCol];
            if (isEmpty(origin)) origin = m.piece;               // return home instantly
            // else: the piece is lost
        }
    }

    st.activeMoves = stillMoving;
}

// Index of whichever pending selection has been waiting longest, or -1 if none.
int oldestPendingSide(const GameState& st) {
    int side = -1;
    for (size_t i = 0; i < st.selections.size(); ++i) {
        if (!st.selections[i].active) continue;
        if (side < 0 || st.selections[i].selectedAtMs < st.selections[side].selectedAtMs)
            side = (int)i;
    }
    return side;
}

// Sends `side`'s pending piece toward (toRow, toCol) and clears that selection.
void sendMove(GameState& st, int side, int toRow, int toCol) {
    Selection& sel = st.selections[side];
    const std::string selected = st.board.grid[sel.row][sel.col];

    PieceMove m;
    m.fromRow = sel.row; m.fromCol = sel.col;
    m.toRow = toRow;     m.toCol = toCol;
    m.startMs = st.elapsedMs;
    m.piece   = selected;                              // carry the piece with the move

    char piece = pieceOf(selected);
    double speed = config::statsFor(piece).speedCellsPerSec;
    double dist  = cellDistance(m.fromRow, m.fromCol, toRow, toCol);
    m.durationMs = (speed > 0.0) ? (long)(dist / speed * 1000.0) : 0;

    if (isLegalMove(st.board, m, piece)) {
        st.board.grid[m.fromRow][m.fromCol] = ".";     // source empties the moment it lifts off
        st.activeMoves.push_back(m);                   // once airborne the piece is off the board
    }
    sel = Selection{};                                  // that side's selection clears once sent
}

void handleClick(GameState& st, int x, int y) {
    int col = x / config::CELL_SIZE;
    int row = y / config::CELL_SIZE;

    if (row < 0 || row >= st.board.rows() ||
        col < 0 || col >= st.board.cols())
        return;

    const std::string& token = st.board.grid[row][col];

    if (!isEmpty(token)) {
        int side = playerIndexOf(colorOf(token));
        if (side < 0 || side >= (int)st.selections.size()) return;   // unrecognized color

        if (st.selections[side].active) {
            // Clicking your own already-pending piece again: always a reselect,
            // never ambiguous -- a piece is never "an enemy" to its own side.
            st.selections[side] = {true, row, col, st.elapsedMs};
            return;
        }

        int target = oldestPendingSide(st);
        if (target >= 0) {
            // Someone else already has a piece pending, and this square holds a
            // different color -- it's an enemy square, so this click can only ever
            // be a SECOND click (a capture target), never a first click that starts
            // a fresh selection for the piece sitting here.
            sendMove(st, target, row, col);
            return;
        }

        // Nobody anywhere has anything pending: a genuine first click, so this
        // piece becomes side's own fresh selection.
        st.selections[side] = {true, row, col, st.elapsedMs};
        return;
    }

    // Empty cell: always a target, never a select. Completes whichever pending
    // selection has been waiting longest.
    int target = oldestPendingSide(st);
    if (target >= 0) sendMove(st, target, row, col);
}

void handleWait(GameState& st, long ms) {
    st.elapsedMs += ms;
    resolveMoves(st);
}

void runCommands(const std::vector<std::string>& commands, GameState& st) {
    for (const auto& command : commands) {
        std::istringstream ss(command);
        std::string verb;
        ss >> verb;

        if (verb == "click") {
            int x, y; ss >> x >> y;
            handleClick(st, x, y);
        } else if (verb == "wait") {
            long ms; ss >> ms;
            handleWait(st, ms);
        } else if (verb == "print") {
            std::string rest; std::getline(ss, rest);
            if (trim(rest) == "board") std::cout << formatBoard(st.board);
        }
        // unknown commands ignored this iteration
    }
}

int main() {
    std::string input, line;
    while (std::getline(std::cin, line)) input += line + '\n';

    Sections sections = parseSections(input);

    GameState state;
    state.board = parseBoard(sections.boardLines);
    state.selections.assign(config::NUM_PLAYERS, Selection{});

    try {
        validateBoard(state.board);
    } catch (const BoardError& e) {
        std::cout << "ERROR " << e.code() << '\n';
        return 0;
    }

    runCommands(sections.commandLines, state);
    return 0;
}