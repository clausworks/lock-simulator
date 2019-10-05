// NICHOLAS BRUNET

// ANTELOPE VALLEY COLLEGE
// CIS 161, FALL 2018
// R. BIRITWUM

// FINAL PROJECT
// LAST UPDATED 12/5/18

/* ABOUT THIS PRORAM
 * This program utilizes the "curses" library to implement a text-based GUI
 * imitating a combination lock. The user uses the arrow keys to navigate
 * the initial menu. The program displays information regarding how many
 * tries the user has remaining to unlock the lock. It implements a visual
 * interface for scrolling and selecting numbers. If the user wishes to
 * reset the lock, he must provide an administrator password ("Think
 * Different."). If fails three consecutive attempts to unlock, the program
 * is locked until a timer finishes or the administrator resets it.
 */

/* CREDITS
 * First and foremost, this project would have been impossible without the
 * "NCurses" library (UNIX/MacOS) and "PDCurses" (Windows). The "NCURSES
 * Programming HOWTO" file was absolutely indispensible in learning how
 * to use the library. (http://www.tldp.org/HOWTO/NCURSES-Programming-HOWTO/)
 *
 * Also, thanks to Joseph Gallagher for the idea of implementing a scrolling
 * mechanism to select numbers.
 *
 * And, of course, a variety of websites, especially Stack Overflow and C For
 * Dummies, which were essential for debugging purposes and the wait()
 * function.
 *
 * This program was compiled using gcc on MacOS Mojave and mingw on Windows 10.
 * It was created using the following editors:
 *                      Atom (from GitHub: atom.io)
 *                      Xcode (from Apple: developer.apple.com/xcode)
 *                      Brackets (from Adobe: brackets.io).
 *
 * Made on a Mac.
 */


#include <stdio.h>
#include <curses.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>



// GLOBAL VARIABLES
WINDOW *menu_win;
WINDOW *scroller_win;
WINDOW *choices_win;
WINDOW *lock_win;
int SCROLL_POSITION = 1;
bool UNLOCKED = false;
int UNLOCK_ATTEMPTS = 0;
int ROWS; // number of rows in console window
int COLS; // number of columns in console window





// FUNCTION PROTOTYPES //
void unlock_icon(void);
bool display_menu(int[]);
void unlock(int[]);
void lock(void);
void reset(int[]);

void unlock_icon(void);
void lock_icon(void);

void get_choices(int[]);
void scroll_left(void);
void scroll_right(void);
void highlight_left(int*);
void highlight_right(int*);
void set_choice(int);
void print_fail(void);

void fail_delay(int);
void wait(int);
bool admin_login(void);


// MAIN //
int main(int argc, const char * argv[]) {
    int combination[3] = {10, 20, 30}; // to store actual combination
    int ch;         // for getting user key input
    bool continue_program = true;

    // set up the window
    initscr(); // begin ncurses mode
    clear();
    raw();  // enable reading single characters, not just whole lines
    noecho();  // don't echo characters when user types them
    curs_set(0);
    start_color();
    use_default_colors();

    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_BLUE, COLOR_BLACK);
    init_pair(7, COLOR_BLACK, COLOR_WHITE);
    init_pair(8, COLOR_RED, COLOR_WHITE);
    init_pair(9, COLOR_YELLOW, COLOR_WHITE);
    init_pair(10, COLOR_GREEN, COLOR_WHITE);

    getmaxyx(stdscr, ROWS, COLS); // get height and width of console window
                                  // getmaxy() is a macro, don't pass by reference
    if (ROWS < 24) {
        printw("Minimum console size is 80x24. Press any key to exit.");
        getch();
        endwin();
        return 1;
    }

    if (COLS < 80) {
        printw("Minimum console size is 80x24. Press any key to exit.");
        getch();
        endwin();
        return 1;
    }

    refresh(); // refresh main window before displaying new windows

    lock_win = newwin(9, 20, 14, (COLS - 14) / 2); // lock icon; centered

    while (continue_program == true) {
        if (UNLOCK_ATTEMPTS < 3) {
            continue_program = display_menu(combination);
        } else {
            // print_fail();
            fail_delay(10000);
        }
    }

    clear();
    refresh();

    delwin(lock_win);
    clear();
    endwin(); // end ncurses mode


    return 0;
}




// CUSTOM FUNCTIONS //

bool display_menu(int combination[]) {
    int ch;      // to receive L/R arrow keys
    int cmd_num = 1; // which command to use (1/2/3)
    bool exitLoop = false;
    WINDOW *unlock_attempts_win;

    clear();
    refresh();


    mvprintw(0, 0, "- Use the left and right arrow keys to navigate the menu.");
    mvprintw(1, 0, "- Press ENTER to select an option.");

    if (UNLOCKED) {
        menu_win = newwin(3, 25, 4, (COLS - 24) / 2 - 1);
        mvwprintw(menu_win, 1, 1, "  LOCK   RESET   QUIT  ");
        mvwchgat(menu_win, 1, 2, 6, A_BOLD | A_STANDOUT, 0, NULL); // item 1 ON

        unlock_icon();
    } else {
        menu_win = newwin(3, 27, 4, (COLS - 26) / 2 - 1);
        mvwprintw(menu_win, 1, 1, "  UNLOCK   RESET   QUIT  ");
        mvwchgat(menu_win, 1, 2, 8, A_BOLD | A_STANDOUT, 0, NULL); // item 1 ON

        lock_icon();
    }

    unlock_attempts_win = newwin(1, 16, 8, (COLS - 16) / 2);
    mvwprintw(unlock_attempts_win, 0, 0, " %d  tries left.", 3 - UNLOCK_ATTEMPTS);

    switch (UNLOCK_ATTEMPTS) {
        case 0:
            mvwchgat(unlock_attempts_win, 0, 0, 3, A_BOLD | A_STANDOUT, 10, NULL);
            break;
        case 1:
            mvwchgat(unlock_attempts_win, 0, 0, 3, A_BOLD | A_STANDOUT, 9, NULL);
            break;
        case 2:
            mvwchgat(unlock_attempts_win, 0, 0, 3, A_BOLD | A_STANDOUT, 8, NULL);
            break;
        default:
            break;
    }

    keypad(menu_win, TRUE);
    box(menu_win, 0, 0);

    wrefresh(menu_win);
    wrefresh(unlock_attempts_win);
    refresh();

    while (exitLoop == false) {
        ch = wgetch(menu_win);

        switch (ch) {
            case KEY_LEFT:
                highlight_left(&cmd_num);
                break;

            case KEY_RIGHT:
                highlight_right(&cmd_num);
                break;

            case '\n':
                exitLoop = true;
                break;

            default:
                break;
        }
    }

    wclear(menu_win);
    wrefresh(menu_win);
    delwin(menu_win);

    wclear(unlock_attempts_win);
    wrefresh(unlock_attempts_win);
    delwin(unlock_attempts_win);

    switch(cmd_num) {
        case 1: // unlock
            if (UNLOCKED) {
                lock();
            } else {
                unlock(combination);
            }
            return true;
        case 2: // reset
            reset(combination);
            return true;
        case 3: // quit
            return false;
        default:
            return true;
    }
}



void unlock(int combination[]) {
    int choices[3];

    clear();
    refresh();

    lock_icon();

    scroller_win = newwin(3, 32, 7, (COLS - 32) / 2);  // number picker; centered
    choices_win  = newwin(2, 38, 11, (COLS - 38) / 2);  // displays choices; centered

    keypad(scroller_win, TRUE); // turn on function (& arrow) key recognition

    get_choices(choices);

    wclear(scroller_win);
    wrefresh(scroller_win);
    delwin(scroller_win);

    wclear(choices_win);
    wrefresh(choices_win);
    delwin(choices_win);

    move(0, 0); clrtoeol();
    move(1, 0); clrtoeol();
    move(2, 0); clrtoeol();
    refresh();

    if (   choices[0] == combination[0]
        && choices[1] == combination[1]
        && choices[2] == combination[2]) {

        unlock_icon();
        UNLOCKED = true;
        UNLOCK_ATTEMPTS = 0;
    } else {
        UNLOCK_ATTEMPTS++;
    }
    SCROLL_POSITION = 1;
}



void lock() {
    lock_icon();
    UNLOCKED = false;
}



void reset(int combination[]) {
    char ch;
    bool exitLoop = false;

    clear();
    refresh();

    mvprintw(0, 0, "Only the administrator can reset the lock.");
    mvprintw(1, 0, "Press ENTER if you are the Administrator.");

    while (exitLoop == false) {
        ch = getch();
        if (ch == '\n') {
            if (admin_login()) {
                exitLoop = true;
            } else {
                return; // faulty admin login
            }
        }
    }


    scroller_win = newwin(3, 32, 7, (COLS - 32) / 2);  // number picker; centered
    choices_win  = newwin(2, 39, 11, (COLS -38) / 2);  // displays choices; centered

    keypad(scroller_win, TRUE); // turn on function (& arrow) key recognition

    mvprintw(0, 0, "To unlock the lock, use the left and right ARROW KEYS to scroll.");
    mvprintw(1, 0, "Then, press the SPACE BAR to select a number. Do this three times.");

    get_choices(combination);

    lock_icon();
    UNLOCKED = false;
    SCROLL_POSITION = 1;
    UNLOCK_ATTEMPTS = 0;

    wclear(scroller_win);
    wrefresh(scroller_win);
    delwin(scroller_win);

    wclear(choices_win);
    wrefresh(choices_win);
    delwin(choices_win);

    move(0, 0); clrtoeol();
    move(1, 0); clrtoeol();
    refresh();
}



void get_choices(int choices[]) {
    int choice_num = 1;
    bool exitLoop = false;
    int ch;

    // main window
    mvprintw(0, 0, "- Use the left and right ARROW KEYS to scroll.");
    mvprintw(1, 0, "- Press the SPACE BAR to select a number.");
    mvprintw(2, 0, "- Select three numbers and press ENTER.");

    // scroller window
    mvwprintw(scroller_win, 0, 3, "------------\\/------------");
    mvwprintw(scroller_win, 1, 0, "<- 37 38 39 40  1  2  3  4  5 ->");
    mvwprintw(scroller_win, 2, 3, "------------/\\------------");
    mvwchgat(scroller_win, 0, 15, 2, A_BOLD, 5, NULL);
    mvwchgat(scroller_win, 1, 15, 2, A_BOLD, 5, NULL);
    mvwchgat(scroller_win, 2, 15, 2, A_BOLD, 5, NULL);

    // choices window
    mvwprintw(choices_win, 0, 5, "You have selected:  __ __ __");

    wrefresh(choices_win);
    refresh();

    while (choice_num <= 3 && exitLoop == false) { // FIXME: only three spacebar presses
        ch = wgetch(scroller_win);

        switch (ch) {
            case KEY_LEFT:
                scroll_left();
                break;
            case KEY_RIGHT:
                scroll_right();
                break;
            case ' ':
                set_choice(choice_num);
                choices[choice_num - 1] = SCROLL_POSITION;
                choice_num++;
                break;
            default:
                break;
        }
        refresh();
    }

    while (true) {
        ch = wgetch(scroller_win);
        if (ch == '\n') {
            return;
        }
    }
}



void set_choice(int choice_num) {
    char numberString[3] = "";
    char tmp[3] = "";

    if (SCROLL_POSITION < 10) {
        strcat(numberString, " ");
        sprintf(tmp, "%d", SCROLL_POSITION);
        strcat(numberString, tmp);
    } else {
        sprintf(numberString, "%d", SCROLL_POSITION);
    }

    switch (choice_num) {
        case 1:
            mvwprintw(choices_win, 0, 25, numberString);
            break;
        case 2:
            mvwprintw(choices_win, 0, 28, numberString);
            break;
        case 3:
            mvwprintw(choices_win, 0, 31, numberString);
            mvwprintw(choices_win, 1, 0, "Press ENTER to submit the combination.");
            mvwchgat(choices_win, 1, 0, -1, A_BOLD, 2, NULL);
            break;
        default:
            mvprintw(1, 0, "ERROR: %d", choice_num);
            break;
    }
    wrefresh(choices_win);
}



void scroll_left() {
    char newString[32] = ""; // for middle line of scroller_win
    char numberString[3] = ""; // for concatenating number to string
    int curNumber; // number to be concatenated

    // decrease center of scroller
    --SCROLL_POSITION;
    if (SCROLL_POSITION < 1) {
        SCROLL_POSITION += 40;
    }

    // set leftmost item of scroller
    curNumber = SCROLL_POSITION - 4;
    if (curNumber < 1) {
        curNumber += 40;
    }

    // build new string to print
    for (int i = 0; i < 9; i++) {
        if (curNumber > 40) {
            curNumber -= 40;
        }

        if (curNumber < 10) {
            strcat(newString, " ");
        }
        sprintf(numberString, "%d", curNumber); // convert int to string
        strcat(newString, numberString);
        strcat(newString, " ");

        curNumber++;
    }

    mvwprintw(scroller_win, 1, 3, newString);
    mvwchgat(scroller_win, 1, 15, 2, A_BOLD, 5, NULL);
    wrefresh(scroller_win);
}



void scroll_right() {
    char newString[32] = ""; // for middle line of scroller_win
    char numberString[3] = ""; // for concatenating number to string
    int curNumber; // number to be concatenated

    // decrease center of scroller
    ++SCROLL_POSITION;
    if (SCROLL_POSITION > 40) {
        SCROLL_POSITION -= 40;
    }

    // set leftmost item of scroller
    curNumber = SCROLL_POSITION - 4;
    if (curNumber < 1) {
        curNumber += 40;
    }

    // build new string to print
    for (int i = 0; i < 9; i++) {
        if (curNumber > 40) {
            curNumber -= 40;
        }

        if (curNumber < 10) {
            strcat(newString, " ");
        }
        sprintf(numberString, "%d", curNumber); // convert int to string
        strcat(newString, numberString);
        strcat(newString, " ");

        curNumber++;
    }

    mvwprintw(scroller_win, 1, 3, newString);
    mvwchgat(scroller_win, 1, 15, 2, A_BOLD, 5, NULL);
    wrefresh(scroller_win);
}



void unlock_icon() {
    wclear(lock_win);

    mvwprintw(lock_win, 0, 0, "             ####   ");
    mvwprintw(lock_win, 1, 0, "           ##    ## ");
    mvwprintw(lock_win, 2, 0, "          ##      ##");
    mvwprintw(lock_win, 3, 0, "          ##      ##");
    mvwprintw(lock_win, 4, 0, "##############");
    mvwprintw(lock_win, 5, 0, "## -------- ##");
    mvwprintw(lock_win, 6, 0, "## UNLOCKED ##");
    mvwprintw(lock_win, 7, 0, "## -------- ##");
    mvwprintw(lock_win, 8, 0, "##############");

    mvwchgat(lock_win, 5, 3, 8, A_BOLD, 2, NULL);
    mvwchgat(lock_win, 6, 3, 8, A_BOLD, 2, NULL);
    mvwchgat(lock_win, 7, 3, 8, A_BOLD, 2, NULL);

    wrefresh(lock_win);
}



void lock_icon() {
    wclear(lock_win);

    mvwprintw(lock_win, 0, 0, "     ####     ");
    mvwprintw(lock_win, 1, 0, "   ##    ##   ");
    mvwprintw(lock_win, 2, 0, "  ##      ##  ");
    mvwprintw(lock_win, 3, 0, "  ##      ##  ");
    mvwprintw(lock_win, 4, 0, "##############");
    mvwprintw(lock_win, 5, 0, "## -------- ##");
    mvwprintw(lock_win, 6, 0, "##  LOCKED  ##");
    mvwprintw(lock_win, 7, 0, "## -------- ##");
    mvwprintw(lock_win, 8, 0, "##############");

    mvwchgat(lock_win, 5, 3, 8, A_BOLD, 3, NULL);
    mvwchgat(lock_win, 6, 3, 8, A_BOLD, 3, NULL);
    mvwchgat(lock_win, 7, 3, 8, A_BOLD, 3, NULL);

    wrefresh(lock_win);
}



void highlight_left(int* cmd_num) {
    if (UNLOCKED) {
        switch(*cmd_num) {
            case 1:
                mvwchgat(menu_win, 1, 2, 6, A_NORMAL, 0, NULL); // item 1 OFF
                mvwchgat(menu_win, 1, 17, 6, A_BOLD | A_STANDOUT, 0, NULL); // item 3 ON
                *cmd_num = 3;
                break;
            case 2:
                mvwchgat(menu_win, 1, 9, 7, A_NORMAL, 0, NULL); // item 2 OFF
                mvwchgat(menu_win, 1, 2, 6, A_BOLD | A_STANDOUT, 0, NULL); // item 1 ON
                *cmd_num = 1;
                break;
            case 3:
                mvwchgat(menu_win, 1, 17, 6, A_NORMAL, 0, NULL); // item 3 OFF
                mvwchgat(menu_win, 1, 9, 7, A_BOLD | A_STANDOUT, 0, NULL); // item 2 ON
                *cmd_num = 2;
                break;
            default:
                break;

        }
    } else {
        switch(*cmd_num) {
            case 1:
                mvwchgat(menu_win, 1, 2, 8, A_NORMAL, 0, NULL); // item 1 OFF
                mvwchgat(menu_win, 1, 19, 6, A_BOLD | A_STANDOUT, 0, NULL); // item 3 ON
                *cmd_num = 3;
                break;
            case 2:
                mvwchgat(menu_win, 1, 11, 7, A_NORMAL, 0, NULL); // item 2 OFF
                mvwchgat(menu_win, 1, 2, 8, A_BOLD | A_STANDOUT, 0, NULL); // item 1 ON
                *cmd_num = 1;
                break;
            case 3:
                mvwchgat(menu_win, 1, 19, 6, A_NORMAL, 0, NULL); // item 3 OFF
                mvwchgat(menu_win, 1, 11, 7, A_BOLD | A_STANDOUT, 0, NULL); // item 2 ON
                *cmd_num = 2;
                break;
            default:
                break;

        }
    }
}



void highlight_right(int* cmd_num) {
    if (UNLOCKED) {
        switch(*cmd_num) {
            case 1:
                mvwchgat(menu_win, 1, 2, 6, A_NORMAL, 0, NULL); // item 1 OFF
                mvwchgat(menu_win, 1, 9, 7, A_BOLD | A_STANDOUT, 0, NULL); // item 2 ON
                *cmd_num = 2;
                break;
            case 2:
                mvwchgat(menu_win, 1, 9, 7, A_NORMAL, 0, NULL); // item 2 OFF
                mvwchgat(menu_win, 1, 17, 6, A_BOLD | A_STANDOUT, 0, NULL); // item 3 ON
                *cmd_num = 3;
                break;
            case 3:
                mvwchgat(menu_win, 1, 17, 6, A_NORMAL, 0, NULL); // item 3 OFF
                mvwchgat(menu_win, 1, 2, 6, A_BOLD | A_STANDOUT, 0, NULL); // item 1 ON
                *cmd_num = 1;
                break;
            default:
                break;

        }
    } else {
        switch(*cmd_num) {
            case 1:
                mvwchgat(menu_win, 1, 2, 8, A_NORMAL, 0, NULL); // item 1 OFF
                mvwchgat(menu_win, 1, 11, 7, A_BOLD | A_STANDOUT, 0, NULL); // item 2 ON
                *cmd_num = 2;
                break;
            case 2:
                mvwchgat(menu_win, 1, 11, 7, A_NORMAL, 0, NULL); // item 2 OFF
                mvwchgat(menu_win, 1, 19, 6, A_BOLD | A_STANDOUT, 0, NULL); // item 3 ON
                *cmd_num = 3;
                break;
            case 3:
                mvwchgat(menu_win, 1, 19, 6, A_NORMAL, 0, NULL); // item 3 OFF
                mvwchgat(menu_win, 1, 2, 8, A_BOLD | A_STANDOUT, 0, NULL); // item 1 ON
                *cmd_num = 1;
                break;
            default:
                break;

        }
    }
}



void print_fail() {
    char ch;
    bool exitLoop = false;

    clear();
    refresh();

    refresh();

    while (exitLoop == false) {
        ch = getch();
        if (ch == '\n') {
            if (admin_login()) {
                exitLoop = true;
            } else {
                return; // faulty admin login
            }
        }
    }
/*
    attrset(COLOR_PAIR(8));

    for (int i = 0; i < 5; i++) {


        mvprintw(1, (COLS - 78) / 2, "##########     ###########         ########            ##############");
        mvprintw(2, (COLS - 78) / 2, "##########     ####     ###        ########           ##############");
        mvprintw(3, (COLS - 78) / 2, "####           ####      ###         ####            #####");
        mvprintw(4, (COLS - 78) / 2, "##########     ##############        ####           #####");
        mvprintw(5, (COLS - 78) / 2, "##########     ##############        ####          #####");
        mvprintw(6, (COLS - 78) / 2, "####           ####                  ####         #####");
        mvprintw(7, (COLS - 78) / 2, "##########     ####                ########      ##############");
        mvprintw(8, (COLS - 78) / 2, "##########     ####                ########     ##############");

        mvprintw(10, (COLS - 78) / 2, "##########     #########           ########            ####");
        mvprintw(11, (COLS - 78) / 2, "##########     ###    ###          ########           ####");
        mvprintw(12, (COLS - 78) / 2, "####           ###     ###           ####            ####");
        mvprintw(13, (COLS - 78) / 2, "##########     ############          ####           ####");
        mvprintw(14, (COLS - 78) / 2, "##########     #############         ####          ####");
        mvprintw(15, (COLS - 78) / 2, "####           ###        ###        ####         ####");
        mvprintw(16, (COLS - 78) / 2, "####           ###         ###     ########      ##############");
        mvprintw(17, (COLS - 78) / 2, "####           ###          ###    ########     ##############");

        refresh();
        wait(500);

        clear();
        refresh();
        wait(500);

}

    clear();
    attrset(A_NORMAL);
    refresh();*/
}




void fail_delay(int milliseconds) {
    bool timerDone = false;
    long pause;
    clock_t now, then;
    int minutes;
    int seconds;
    int ch; // for getting keyboard input

    nocbreak();


    seconds = milliseconds / 1000;

    // minutes = milliseconds / 1000 / 60; // approximate


    mvprintw(0, 0, "You have been locked out. Wait %02d seconds before trying again.", seconds);

    mvprintw(2, 0, "Or, ask the Administrator to reset the program.");
    mvprintw(3, 0, "Press ENTER if you are the Administrator.");

    mvchgat(0, 0, 25, A_BOLD | A_STANDOUT, 8, NULL);

    pause = milliseconds*(CLOCKS_PER_SEC / 1000);
    now = then = clock(); // initialize timer


    for (int i = 0; i < (milliseconds / 1000) && timerDone == false; i++) {
        // now = clock();
        mvprintw(0, 31, "%02d", seconds - i);
        halfdelay(10);
        ch = getch();

        switch (ch) {
            case '\n':
                if (admin_login()) { // admin successfully unlocked
                    timerDone = true;
                    UNLOCK_ATTEMPTS = 0; // reset program
                }
                break;
            default:
                break;
        }
    }

    UNLOCK_ATTEMPTS = 0; // reset program

    cbreak();
}


// wait() function from c-for-dummies.com:
// https://c-for-dummies.com/blog/?p=69
void wait(int milliseconds)
{
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
}


bool admin_login() {
    WINDOW *admin_welcome_win;
    WINDOW *apple_win;
    WINDOW *passwd_win;
    bool passwd_correct = false;
    char userInput[50];
    int cur_color;

    char apple_logo[15][32] =
        {
            "                  .@'",
            "               .@@@'",
            "              .@'",
            "      .@@@@@.     .@@@@@.",
            "   .@@@@@@@@@@@@@@@@@@@@@@",
            " .@@@@@@@@@@@@@@@@@@@@@@'",
            ".@@@@@@@@@@@@@@@@@@@@@'",
            "@@@@@@@@@@@@@@@@@@@@@'",
            "@@@@@@@@@@@@@@@@@@@@@.",
            "@@@@@@@@@@@@@@@@@@@@@@.",
            "'@@@@@@@@@@@@@@@@@@@@@@@.",
            " '@@@@@@@@@@@@@@@@@@@@@@@@@",
            "  '@@@@@@@@@@@@@@@@@@@@@@@'",
            "    '@@@@@@@@@@@@@@@@@@@'",
            "       '@@@@'    '@@@@'"
        };


    clear();
    refresh();

    admin_welcome_win = newwin(1, 24, 1, (COLS - 24) / 2);
    apple_win = newwin(15, 32, 3, (COLS - 32) / 2);
    passwd_win = newwin(2, 33, 20, (COLS - 32) / 2);

    mvwprintw(admin_welcome_win, 0, 0, "Welcome, Administrator.");
    wrefresh(admin_welcome_win);
    wait(2000);

    for (int i = 0; i < 15; i++) {
        mvwprintw(apple_win, i, 0, apple_logo[i]);
        wait(65);
        wrefresh(apple_win);
    }

    wait(500);
    mvwprintw(passwd_win, 0, 0, "Enter password: ");


    wrefresh(admin_welcome_win);
    wrefresh(apple_win);
    wrefresh(passwd_win);

    echo();
    curs_set(1);

    while (passwd_correct == false) {
        wmove(passwd_win, 0, 16);
        wgetstr(passwd_win, userInput);
        if (strcmp(userInput, "Think Different.") == 0) {
            passwd_correct = true;
        }
    }

    noecho();
    curs_set(0);

    wait(500);

    for (int i = 14; i >= 0; i--) {
        if (i > 12) {
            mvwchgat(apple_win, i, 0, -1, A_BOLD, 6, NULL);
        } else if (i > 10) {
            mvwchgat(apple_win, i, 0, -1, A_BOLD, 5, NULL);
        } else if (i > 8) {
            mvwchgat(apple_win, i, 0, -1, A_BOLD, 4, NULL);
        } else if (i > 6) {
            mvwchgat(apple_win, i, 0, -1, A_BOLD, 3, NULL);
        } else if (i > 4) {
            mvwchgat(apple_win, i, 0, -1, A_BOLD, 1, NULL);
        } else {
            mvwchgat(apple_win, i, 0, -1, A_BOLD, 2, NULL);
        }

        wait(65);
        wrefresh(apple_win);
    }

    // getch();
    wait(2000);

    // getch();

    wclear(admin_welcome_win);
    wclear(apple_win);
    wclear(passwd_win);
    wrefresh(admin_welcome_win);
    wrefresh(apple_win);
    wrefresh(passwd_win);
    delwin(admin_welcome_win);
    delwin(apple_win);
    delwin(passwd_win);


    return true;
}
