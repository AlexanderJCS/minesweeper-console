#include <windows.h>
#include <iostream>
#include <conio.h>
#include <vector>


const int WIDTH = 10;
const int HEIGHT = 10;
const int BOMB_CHANCE = 7;  // Lower is more likely


void ShowConsoleCursor(bool showFlag)
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = showFlag; // set the cursor visibility
    SetConsoleCursorInfo(out, &cursorInfo);
}


struct Gridspace
{
    bool bomb = false;
    bool flagged = false;
    bool revealed = false;
    short adjacentBombs = 0;
};


class Game
{
    std::vector<std::vector<Gridspace>> grid;
    std::vector<int> cursorPos = { 0, 0 };
    bool firstReveal = true;

    void generateGrid()
    {
        for (int i = 0; i < WIDTH; i++)
        {
            std::vector<Gridspace> row;
            
            for (int j = 0; j < HEIGHT; j++)
            {
                Gridspace g;
                
                if (rand() % BOMB_CHANCE == 0)
                {
                    g.bomb = true;
                }
                
                row.push_back(g);
            }
            
            grid.push_back(row);
        }
    }
    
    void generateAdjacentBombValues()
    {
        for (int y = 0; y < grid.size(); y++)
        {
            for (int x = 0; x < grid[0].size(); x++)
            {
                if (grid[y][x].bomb == true)
                {
                    continue;
                }
                
                for (int i = -1; i <= 1; i++)
                {
                    for (int j = -1; j <= 1; j++)
                    {
                        if (y + i < 0 || y + i >= grid.size() || x + j < 0 || x + j >= grid[0].size())
                        {
                            continue;
                        }
                        
                        if (grid[y + i][x + j].bomb == true)
                        {
                            grid[y][x].adjacentBombs++;
                        }
                    }
                }
            }
        }
    }

    void draw(bool ignoreRevealed)
    {
        std::cout << "\033[1;1H";

		// Draw top border
        for (int i = 0; i <= grid.size(); i++)
        {
            std::cout << "##";
        }

        std::cout << "\n";

        for (int y = 0; y < HEIGHT; y++)
        {
            std::cout << "#";
			
            for (int x = 0; x < WIDTH; x++)
            {
                if (x == cursorPos[0] && y == cursorPos[1])
                {
                    std::cout << "\u001b[35m";
                }

                if (grid[y][x].flagged)
                {
                    // Remove color if the cursor is highlighting it
                    if (x == cursorPos[0] && y == cursorPos[1])
                    {
                        std::cout << "F";
                    }
                    
                    else
                    {
                        std::cout << "\u001b[31mF";
                    }
                }

                else if (!grid[y][x].revealed && !ignoreRevealed)
                {
                    std::cout << "?";
                }
                
                else if (grid[y][x].adjacentBombs)
                {
                    if (!(x == cursorPos[0] && y == cursorPos[1]))
                    {
                        std::cout << "\u001b[34m";
                    }
					
                    std::cout << grid[y][x].adjacentBombs;
                }
                
                else if (grid[y][x].bomb)
                {
                    std::cout << "B";
                }

                else
                {
                    if (cursorPos[0] == x && cursorPos[1] == y)
                    {
                        std::cout << "^";
                    }
					
                    else
                    {
                        std::cout << " ";
                    }
                }

                // Reset the colors
                std::cout << " \u001b[0m";
            }
            std::cout << "#\n";
        }

        for (int i = 0; i <= grid.size(); i++)
		{
			std::cout << "##";
		}
    }

    void getInput()
    {
        if (!_kbhit())
        {
            return;
        }

        switch (_getch())
        {
        case 'w':
            if (cursorPos[1] > 0)
            {
                cursorPos[1] -= 1;
            }
            
            break;

        case 's':
            if (cursorPos[1] + 1 < HEIGHT)
            {
                cursorPos[1] += 1;
            }
            
            break;

        case 'a':
            if (cursorPos[0] > 0)
            {
                cursorPos[0] -= 1;
            }

            break;

        case 'd':
            if (cursorPos[0] < WIDTH - 1)
            {
                cursorPos[0] += 1;
            }

            break;

        case 'f':
            // Flip the boolean
            grid[cursorPos[1]][cursorPos[0]].flagged ^= true;
            break;

        case 'e':
            if (grid[cursorPos[1]][cursorPos[0]].flagged)
            {
                break;
            }

            if (firstReveal)
            {
                firstReveal = false;
				
                while (grid[cursorPos[1]][cursorPos[0]].bomb ||
                    grid[cursorPos[1]][cursorPos[0]].adjacentBombs)
                {
					// Regenrate the board
					grid.clear();
					generateGrid();
					generateAdjacentBombValues();
                }
            }

            revealAdjacentSpaces(cursorPos[0], cursorPos[1]);
            break;
        }
    }

    void revealAdjacentSpaces(int x, int y)
    {
		// If the x or y is outside the grid, return
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        {
            return;
        }
		
		// Don't reveal spaces that have already been revealed
        if (grid[y][x].revealed)
		{
			return;
		}
        
        grid[y][x].revealed = true;
		
		// If the space is next to a bomb, don't reveal more
        if (grid[y][x].adjacentBombs)
        {
            return;
        }

        for (int i = y - 1; i < y + 2; i++)
        {
            for (int j = x - 1; j < x + 2; j++)
            {
                if (i == y && j == x)
                {
                    continue;
                }

                revealAdjacentSpaces(j, i);
            }
        }
    }

    bool won()
    {
        for (int y = 0; y < grid.size(); y++)
        {
            for (int x = 0; x < grid[0].size(); x++)
            {
				// If it is a bomb and it hasn't been flagged, return false
				// If it is not a bomb and if it has been flagged, return false
				// If it is not a bomb and if it has not been revealed, return false
                if ((grid[y][x].bomb && !grid[y][x].flagged) ||
                    (!grid[y][x].bomb && grid[y][x].flagged) ||
                    (!grid[y][x].bomb && !grid[y][x].revealed))
                {
                    return false;
                }
            }
        }
        
		return true;
    }

    bool lost()
    {
        for (int y = 0; y < grid.size(); y++)
        {
            for (int x = 0; x < grid[0].size(); x++)
            {
                // If a bomb has been revealed, return true
                if (grid[y][x].bomb && grid[y][x].revealed)
                {
                    return true;
                }
            }
        }
		
        return false;
    }

public:
    void run()
    {
		
        generateGrid();
        generateAdjacentBombValues();

        int i = 0;  // used to execute a command after n amount of frames
		
        while (true)
        {	
            if (won())
            {
                draw(true);
				std::cout << "You won!\n";
				break;
            }
			
            if (lost())
            {
                draw(true);
                std::cout << "You lost.\n";
                break;
            }

            draw(false);
            getInput();
			
            if (i == 10)
            {
                i = 0;
                // Allows for resizing of the window without losing the cursor settings
                ShowConsoleCursor(false);  
            }

			i++;

            Sleep(30);
        }

        Sleep(5000);
    }
};

int main()
{
    ShowConsoleCursor(false);
    srand(time(0));

    Game game;
    game.run();
}
