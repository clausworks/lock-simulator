# lock-simulator
This program utilizes the [ncurses](https://en.wikipedia.org/wiki/Ncurses) library to implement a text-based GUI imitating a combination lock. The user uses the arrow keys to navigate the initial menu. The program displays information regarding how many tries the user has remaining to unlock the lock. It implements a visual interface for scrolling and selecting numbers. If the user wishes to reset the lock, he must provide an administrator password (`Think Different.`). If fails three consecutive attempts to unlock, the program is locked until a timer finishes or the administrator resets it. The initial combination is `10 20 30`.

Note that ncurses must be installed on your computer if you wish to run it.
