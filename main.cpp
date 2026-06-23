#include <iostream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <unordered_map>
#include <cmath>   
#include <thread>  // For sleep (time delay)

using namespace std;

// Main timer for game start
using Clock = chrono::steady_clock;
static auto gameStart = Clock::now();

// --- Utility Functions ---

// Get current system time (HH:MM:SS)
string nowString() {
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    tm tm{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    ostringstream oss;
    oss << setfill('0')
        << setw(2) << tm.tm_hour << ':'
        << setw(2) << tm.tm_min  << ':'
        << setw(2) << tm.tm_sec;
    return oss.str();
}

// Calculate elapsed time since game start
string formatElapsed() {
    auto now  = Clock::now();
    auto secs = chrono::duration_cast<chrono::seconds>(now - gameStart).count();
    int h = secs / 3600;
    int m = (secs % 3600) / 60;
    int s = secs % 60;
    ostringstream oss;
    oss << setfill('0') << setw(2) << h << ':'
        << setw(2) << m << ':' << setw(2) << s;
    return oss.str();
}

// Sleep duration in milliseconds
void sleepMs(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}

// Clear terminal screen
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// --- Data Structures ---

struct Cell {
    char gem;       // Gem type (A, B, C, ...)
    bool remove;    // Flagged for removal?
};

using Board = vector<vector<Cell>>;

// Color map for rendering gems in the terminal
const unordered_map<char, string> gemChar = {
    {'A', "\x1b[31mA\x1b[0m"}, // red
    {'B', "\x1b[32mB\x1b[0m"}, // green
    {'C', "\x1b[33mC\x1b[0m"}, // yellow
    {'D', "\x1b[34mD\x1b[0m"}, // blue
    {'E', "\x1b[35mE\x1b[0m"}, // magenta
    {'X', "\x1b[41mX\x1b[0m"}  // Highlight for removal
};

// --- Game Logic Functions ---

char randomGem() { return char('A' + rand() % 5); }

// Print the game board and statistics
void printBoard(const Board& b, int score, int comboChain) {
    int M = b.size(), N = b[0].size();

    cout << "==========================================\n";
    cout << " Time: " << nowString()
         << " | Elapsed: " << formatElapsed() << "\n";
    cout << " Score: " << score;
    if (comboChain > 0) {
        cout << " | \x1b[33mCOMBO CHAIN: " << comboChain << "x\x1b[0m";
    }
    cout << "\n==========================================\n\n";

    cout << "     ";
	for (int j = 0; j < N; ++j)
        cout << setw(2) << setfill('0') << j << "  ";
	cout << "\n";
    cout << "    " << string(N * 4, '-') << "\n";

    for (int i = 0; i < M; ++i) {
        cout << setw(2) << setfill('0') << i << " | ";
        for (int j = 0; j < N; ++j) {
            char g = b[i][j].gem;
            // If the cell is flagged, display it with a highlighted background
            if (b[i][j].remove) {
                cout << "\x1b[41m" << g << "\x1b[0m   "; 
            } else {
                auto it = gemChar.find(g);
                if (it != gemChar.end()) cout << it->second << "   ";
                else                     cout << g << "   ";
            }
        }
        cout << "\n";
    }
    cout << "\n";
}

bool inBounds(int i, int j, int M, int N) { 
    return i >= 0 && i < M && j >= 0 && j < N; 
}

// Smart board initialization without any initial matches
void initBoardSmart(Board& b, int x) {
    int M = b.size(), N = b[0].size();
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            char gem;
            bool match;
            do {
                match = false;
                gem = randomGem();
                
                // Horizontal match check
                if (j >= x - 1) {
                    bool rowMatch = true;
                    for (int k = 1; k < x; ++k) if (b[i][j - k].gem != gem) rowMatch = false;
                    if (rowMatch) match = true;
                }

                // Vertical match check
                if (!match && i >= x - 1) {
                    bool colMatch = true;
                    for (int k = 1; k < x; ++k) if (b[i - k][j].gem != gem) colMatch = false;
                    if (colMatch) match = true;
                }
            } while (match);
            
            b[i][j].gem = gem;
            b[i][j].remove = false;
        }
    }
}

// Identify and flag matches on the board
bool markMatches(Board& b, int x) {
    int M = b.size(), N = b[0].size();
    bool any = false;

    for (auto& row : b)
        for (auto& cell : row)
            cell.remove = false;

    // Horizontal match detection
    for (int i = 0; i < M; ++i) {
        int j = 0;
        while (j < N) {
            char g = b[i][j].gem;
            int k = j;
            while (k < N && b[i][k].gem == g) ++k;
            int len = k - j;
            if (g != '.' && len >= x) {
                any = true;
                for (int t = j; t < k; ++t) b[i][t].remove = true;
            }
            j = k;
        }
    }

    // Vertical match detection
    for (int j = 0; j < N; ++j) {
        int i = 0;
        while (i < M) {
            char g = b[i][j].gem;
            int k = i;
            while (k < M && b[k][j].gem == g) ++k;
            int len = k - i;
            if (g != '.' && len >= x) {
                any = true;
                for (int t = i; t < k; ++t) b[t][j].remove = true;
            }
            i = k;
        }
    }
    return any;
}

// Remove flagged cells and replace with empty space placeholders
int removeMarked(Board& b) {
    int removed = 0;
    for (auto& row : b)
        for (auto& cell : row)
            if (cell.remove) {
                cell.gem = '.';
                cell.remove = false;
                ++removed;
            }
    return removed;
}

// Apply gravity to drop gems down and spawn new ones at the top
void applyGravity(Board& b) {
    int M = b.size(), N = b[0].size();
    for (int j = 0; j < N; ++j) {
        int writeRow = M - 1;
        for (int i = M - 1; i >= 0; --i)
            if (b[i][j].gem != '.') {
                b[writeRow][j].gem = b[i][j].gem;
                --writeRow;
            }
        while (writeRow >= 0) {
            b[writeRow][j].gem = randomGem();
            --writeRow;
        }
    }
}

// Recursively process cascades and combo chain reactions
void updateBoardRecursive(Board& b, int x, int& score, int& comboStep) {
    if (!markMatches(b, x)) return;

    clearScreen();
    printBoard(b, score, comboStep);
    cout << ">>> Matched! Exploding... <<<\n";
    sleepMs(700);

    int removed = removeMarked(b);
    long long roundScore = (removed * 10) * pow(2, comboStep);
    score += roundScore;

    clearScreen();
    printBoard(b, score, comboStep);
    cout << ">>> Points: " << roundScore << " (Combo x" << (comboStep + 1) << ") <<<\n";
    sleepMs(400);

    applyGravity(b);

    clearScreen();
    printBoard(b, score, comboStep);
    cout << ">>> Gravity applied. Checking reactions... <<<\n";
    sleepMs(500);

    ++comboStep;
    updateBoardRecursive(b, x, score, comboStep);
}

// Attempt to swap two gems and resolve matches if valid
bool trySwapAndResolve(Board& b, int x, int r, int c, char dir, int& score) {
    int M = b.size(), N = b[0].size();
    int dr = 0, dc = 0;
    if (dir == 'U') dr = -1;
    else if (dir == 'D') dr = 1;
    else if (dir == 'L') dc = -1;
    else if (dir == 'R') dc = 1;
    else return false;

    int nr = r + dr, nc = c + dc;
    if (!inBounds(r, c, M, N) || !inBounds(nr, nc, M, N)) return false;

    swap(b[r][c].gem, b[nr][nc].gem);

    Board temp = b;
    if (!markMatches(temp, x)) {
        swap(b[r][c].gem, b[nr][nc].gem);
        return false;
    }

    int comboStep = 0;
    updateBoardRecursive(b, x, score, comboStep);
    return true;
}

// --- File I/O ---

bool saveGame(const string& filename, const Board& b, int x, int score) {
    ofstream out(filename);
    if (!out) return false;
    int M = b.size(), N = b[0].size();
    out << M << ' ' << N << ' ' << x << "\n";
    out << score << "\n";
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) out << b[i][j].gem << " ";
        out << "\n";
    }
    return true;
}

bool loadGame(const string& filename, Board& b, int& x, int& score) {
    ifstream in(filename);
    if (!in) return false;
    int M, N;
    if (!(in >> M >> N >> x >> score)) return false;

    b.assign(M, vector<Cell>(N));
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < N; ++j) {
            in >> b[i][j].gem;
            b[i][j].remove = false;
        }
    return true;
}

// --- Main ---

int main() {
    srand((unsigned)time(nullptr));

    Board board;
    int M = 8, N = 8, x = 3;
    int score = 0;

    cout << "GEM CASCADE - Project No. 1\n";
    cout << "1) New Game\n2) Load Game\nChoose: ";
    string menuChoice;
    cin >> menuChoice;

    bool loaded = false;
    if (menuChoice == "2") {
        if (loadGame("save.txt", board, x, score)) {
            cout << "Game Loaded Successfully!\n";
            loaded = true;
            sleepMs(1000);
        } else {
            cout << "Load failed or file not found. Starting New Game...\n";
            sleepMs(1500);
        }
    }

    if (!loaded) {
        cout << "Enter rows (M): "; 
        while(!(cin >> M)) { cout << "Invalid number. Rows: "; cin.clear(); cin.ignore(1000,'\n'); }
        
        cout << "Enter cols (N): ";
        while(!(cin >> N)) { cout << "Invalid number. Cols: "; cin.clear(); cin.ignore(1000,'\n'); }
        
        cout << "Enter match length (x): ";
        while(!(cin >> x) || x > min(M,N)/2) { 
            cout << "Invalid x (must be < half dimension). x: "; 
            cin.clear(); cin.ignore(1000,'\n'); 
        }

        board.assign(M, vector<Cell>(N));
        score = 0;
        initBoardSmart(board, x);
    }

    while (true) {
        clearScreen();
        printBoard(board, score, 0);
        
        cout << "COMMANDS:\n";
        cout << "  [Row] [Col] [Dir: U/D/L/R] -> Move Gem\n";
        cout << "  S -> Save Game\n";
        cout << "  Q -> Quit\n";
        cout << "> ";

        string input;
        cin >> input;

        if (input == "Q" || input == "q") {
            cout << "Are you sure? (y/n): ";
            char confirm; cin >> confirm;
            if (confirm == 'y' || confirm == 'Y') break;
            continue;
        }
        
        if (input == "S" || input == "s") {
            if (saveGame("save.txt", board, x, score)) cout << "\n[!] Game Saved successfully to save.txt\n";
            else cout << "\n[!] Error saving game.\n";
            cout << "Press Enter to continue...";
            cin.ignore(1000, '\n'); cin.get();
            continue;
        }

        int r;
        try {
            r = stoi(input);
        } catch (...) {
            cout << "Invalid input! Enter Row number or Command (S/Q).\n";
            cin.ignore(1000, '\n'); 
            sleepMs(1000);
            continue;
        }

        int c; 
        char dir;
        if (!(cin >> c >> dir)) {
            cout << "Invalid move format! Use: Row Col Dir\n";
            cin.clear(); cin.ignore(1000, '\n');
            sleepMs(1000);
            continue;
        }
        
        dir = toupper(dir);
        
        if (!trySwapAndResolve(board, x, r, c, dir, score)) {
            cout << "\n\x1b[31mInvalid Move! No match created.\x1b[0m\n";
            sleepMs(1000);
        }
    }

    cout << "Final Score: " << score << "\nGoodbye!\n";
    return 0;
}
