#Chess game

### Terminal (Linux) Based Chess game

Terminal Linux based text-only multiplayer chess game

2 players only, no AI avaliable, feel free to add

**chess_core.c** - core file, with moves and check (mate), the chess *engine*, may be used to implement Graphical User Interface

**chess_ui.c** - User interface and networking implementation

**chess.h** - header file, colors and pieces definitions, functions declarations

**conf** - *configure* file, usage: *./conf*, compiles to local bin folder *$HOME/bin/*, gcc warning are deactivated

### Gameplay

use [from row][from column][to row][to column] to move piece, e.g. *b2d2*, moves piece from B2 to D2 (if possible)

Note: *May not be totally completed, feel free to amend it (and possibly pull it)*
