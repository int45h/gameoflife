#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <random>
#include <thread> 
#include <chrono>

#include <ncurses.h>
#include <panel.h>

#define COLS game_display.cols
#define ROWS game_display.rows

enum Flags
{
    OSCILLATOR
};

/* Screen information */
struct Screen
{
    int cols, rows, tick, depth;
    WINDOW * win;
    
    Screen()
    {
        initscr();
        cbreak();
        noecho();
        
        getmaxyx(stdscr, rows, cols);
        
        win = newwin(rows, cols, 0, 0);
        wrefresh(win);

        clear();
    }
};

/* Game State */
struct State
{
    public: 
    bool * screen;
    bool * update;

    Screen game_display;

    /* Randomized seed for Raifu Geimu */
    State()
    {
        screen = (bool*) calloc(COLS*ROWS, sizeof(bool));
        update = (bool*) calloc(COLS*ROWS, sizeof(bool));
    
        std::default_random_engine cell;
        std::uniform_int_distribution<int> distribution(0, 1);

        for(auto i = 0; i < COLS; i++)
            for(auto j = 0; j < ROWS; j++)
            {
                bool NuCell = (bool) distribution(cell); 
                update[(j * COLS) + i] = NuCell;
            } 

        memcpy(screen, update, COLS*ROWS*sizeof(bool));
        game_display.tick = 50;
    }

    /* Set tick speed of geimu */
    void setTick(unsigned int usec)
    {
        game_display.tick = usec;
    }
};

/* Game tick */
inline void Tick(Screen& game_display)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(game_display.tick));
}

inline void Display(State& game_state);
inline bool GetCell(int x, int y, State& game_state);

/* Update the game state according to the rules of Conway's Game of Life */
void Update(State& game_state)
{
    for(auto i = 1; i < game_state.COLS-1; i++)
        for(auto j = 1; j < game_state.ROWS-1; j++)
        {
            /* Get count of neighboring cells */
            int neighbors  = GetCell(i-1, j-1, game_state) + GetCell(i  , j-1, game_state) + GetCell(i+1, j-1, game_state) + 
                             GetCell(i-1, j  , game_state) + 0                             + GetCell(i+1, j  , game_state) + 
                             GetCell(i-1, j+1, game_state) + GetCell(i  , j+1, game_state) + GetCell(i+1, j+1, game_state);
            
            /* Update cells */
            if(game_state.screen[(j * game_state.COLS) + i])
                game_state.update[(j * game_state.COLS) + i] = (neighbors == 3 || neighbors == 2) ? 1 : 0;
            else
                game_state.update[(j * game_state.COLS) + i] = (neighbors == 3) ? 1 : 0;
        }

        memcpy(game_state.screen, game_state.update, game_state.COLS*game_state.ROWS*sizeof(bool));
        Tick(game_state.game_display);
}   

/* Get Cell State */
inline bool GetCell(int x, int y, State& game_state)
{
    return game_state.screen[y*game_state.COLS + x];
}

/* Update display */
inline void Display(State& game_state)
{
    char next_char;

    for(auto i = 0; i < game_state.COLS; i++)
        for(auto j = 0; j < game_state.ROWS; j++)
        {
            next_char = (game_state.screen[(j * game_state.COLS) + i]) ? '#' : ' ';
            mvwaddch(game_state.game_display.win, j, i, next_char); 
        }
}

/* Driver Code */
int main(int argc, char** argv)
{ 
    State game_state;
    const char * message = "Stream will be starting soon.";
    
    if(argc >= 2)
        game_state.setTick(atoi(argv[1]));

    WINDOW * title = newwin(5, strlen(message)+4, (game_state.ROWS/2)-3, (game_state.COLS/2)-((strlen(message)+4)/2));    

    box(title, 0, 0);    
    mvwprintw(title, 2, 2, message);

    PANEL * front = new_panel(title);
    PANEL * back  = new_panel(game_state.game_display.win);

    show_panel(back);
    show_panel(front);
    
    while(true)
    {
        Update(game_state);
        Display(game_state);

        update_panels();
        doupdate();
    }

    endwin();
    return 0;
}