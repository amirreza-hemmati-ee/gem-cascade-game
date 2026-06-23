# Gem Cascade Game 🎮

A professional, terminal-based match-3 puzzle game (similar to Candy Crush) implemented in pure C++.

## Features ✨
* **Smart Board Generation:** Automatically initializes a playable board with zero pre-existing matches.
* **Recursive Combo System:** Dynamically detects cascading matches, calculates multipliers, and updates scores continuously.
* **State Persistence:** Includes a robust file-based Save/Load feature using standard I/O streams (`save.txt`).
* **Enhanced Terminal UI:** Utilizes ANSI escape codes to render beautifully colored gems and smooth reaction animations.
* **Real-time Statistics:** Displays current system time and live session elapsed time trackers.

## How to Run 🚀

1. Clone the repository and navigate to the project directory.
2. Compile the source code using a C++11 (or later) compliant compiler:
   ```bash
   g++ main.cpp -o GemCascade
