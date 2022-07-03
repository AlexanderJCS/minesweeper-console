#include <windows.h>
#include <iostream>
#include <conio.h>
#include <vector>


const int WIDTH = 15;
const int HEIGHT = 15;
const int BOMB_CHANCE = 6;  // Lower is more likely, 1 / BOMB_CHANCE of a space being a bomb.


/*
This function is used to hide the cursor in the console to prevent the cursor from being distracting.
*/

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
    bool firstReveal = true;  // First reveal says if the first selection of a space was made.

    int width = WIDTH;
	int height = HEIGHT;

	/*
    Generates a grid that is WIDTH x HEIGHT.
	Each space has a 1 / BOMB_CHANCE chance of being a bomb.
    */

    void generateGrid()
    {
        for (int i = 0; i < width; i++)
        {
            std::vector<Gridspace> row;
            
            for (int j = 0; j < height; j++)
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
    
	/*
    Used at the start of the game to generate the numbers to tell the player if that space is next to a bomb.
    */

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

	/*
    Draws the board. 
    The ignoreRevealed argument is used to ignore if the gridspace is revealed or not.
	This is useful when the game is over and the entire board needs to be shown.
    */
	
    void draw(bool ignoreRevealed)
    {
        std::cout << "\033[1;1H";

		// Draw top border
        for (int i = 0; i <= grid.size(); i++)
        {
            std::cout << "##";
        }

        std::cout << "\n";

        for (int y = 0; y < height; y++)
        {
            std::cout << "#";
			
            for (int x = 0; x < width; x++)
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

	/*
    Gets input from the user and executes an appropriate action.
    */

    void getInput()
    {
        if (!_kbhit())
        {
            return;
        }

        switch (_getch())
        {
        case 'w':
            moveCursor(0, -1);
            break;

        case 's':
            moveCursor(0, 1);
            break;

        case 'a':
            moveCursor(-1, 0);
            break;

        case 'd':
            moveCursor(1, 0);
            break;

        case 'f':
            if (!grid[cursorPos[1]][cursorPos[0]].revealed)
            {
                // Flip the boolean
                grid[cursorPos[1]][cursorPos[0]].flagged ^= true;
            }
            
            break;

        case 'e':
            selectSpace();
            break;
        }
    }


    void moveCursor(int xDiff, int yDiff)
    {
        if (cursorPos[0] + xDiff <= width - 1 && cursorPos[0] + xDiff >= 0 && 
            cursorPos[1] + yDiff <= height - 1 && cursorPos[1] + yDiff >= 0)
        {
			cursorPos[0] += xDiff;
			cursorPos[1] += yDiff;
        }
    }

	/*
    Selects the space for the cursor
    */
	
    void selectSpace()
    {
        if (grid[cursorPos[1]][cursorPos[0]].flagged)
        {
            return;
        }

		// If the first action was made, make sure that you are not selecting a number or a bomb.
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
    }

	/*
    This method reveals all spaces in a certain region.
	This is used when the player selects a space that is not numbered or a bomb.
    */

    void revealAdjacentSpaces(int x, int y)
    {
		// If the x or y is outside the grid, return
        if (x < 0 || x >= width || y < 0 || y >= height)
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

		// Reveal all adjacent spaces (e.g. top left, top, bottom left, etc.)
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

	/*
    Checks if the player won the game.
	Returns true if the player won, false if the player lost or if the game is still ongoing.
    */

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

	/*
    Returns true if the player lost the game.
    Returns false if the game is still ongoing.
    */
	
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
		// Get the width and height of the game
        std::cout << "Enter the width of the game (type 0 for default): ";
        std::cin >> width;
		std::cout << "Enter the height of the game (type 0 for default): ";
		std::cin >> height;

        if (width == 0)
        {
            width = WIDTH;
        }
		
		if (height == 0)
		{
			height = HEIGHT;
		}
		
        system("cls");

        generateGrid();
        generateAdjacentBombValues();
		
        while (true)
        {
			for (int i = 0; i < 10; i++)
            {
                if (won())
                {
                    draw(true);
                    std::cout << "\nYou won!\n";
                    break;
                }

                if (lost())
                {
                    draw(true);
                    std::cout << "\nYou lost.\n";
                    break;
                }

                draw(false);
                getInput();


                Sleep(30);
            }

            // Allows for resizing of the window without losing the cursor settings
            ShowConsoleCursor(false);
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
