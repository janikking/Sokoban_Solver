//#define NDEBUG
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>
using namespace std;

const int WIDTH = 10;
const int HEIGHT = 10;
int node = 0;
struct Pos
{
    int col;
    int row;
};

Pos maxPos = {0,0};

enum Wind {North,East,South,West};
const Pos north = {0,-1};
const Pos east = {1,0};
const Pos south = {0,1};
const Pos west = {-1,0};

const vector<Pos> Winds {north,east,south,west};

struct Puzzle
{
    char board[WIDTH][HEIGHT];
    Pos position;
};

struct Candidate
{
    Puzzle candidate;
    int parent_candidate;
};

Pos operator+(Pos a, Pos b)
{
    Pos sum;
    sum.col = a.col + b.col;
    sum.row = a.row + b.row;
    return sum;
}

Pos operator-(Pos a, Pos b)
{
    Pos sum;
    sum.col = a.col - b.col;
    sum.row = a.row - b.row;
    return sum;
}

Pos operator*(int a, Pos b)
{
    Pos product;
    product.col = a * b.col;
    product.row = a * b.row;
    return product;
}

ostream& operator<<(ostream& out, const Pos& position)
{
    out << "{column " << position.col
        << ", row " << position.row
        << "}\n";
    return out;
}
bool operator<(Pos a, Pos b)
{
    return a.col < b.col && a.row < b.row;
}

void solve_depth(vector<Puzzle>& attempt, vector<Puzzle>& shortest, int max_depth);

///Get puzzle

string get_filename()
{
    string filename;
    ifstream input;
    while(!input.is_open())
    {
        cout << "Input a filename: ";
        cin >> filename;
        input.open(filename);
    }
    cout << "The file " << filename << " has opened succesfully.\n\n";

    input.close();
    return filename;
}

void get_puzzle_size(string filename,Pos& maxPos)
{
    assert(filename != "");
    //Opens file with filename to store the dimensions of the board in maxPos.
    maxPos = {-1,0};
    ifstream input(filename);
    char testerC;
    string testerS;
    while(testerC != '\n')
    {
        input.get(testerC);
        maxPos.col++;
    }
    while(!input.eof())
    {
        getline(input,testerS);
        maxPos.row++;
    }
    input.close();
}


void get_file(Puzzle& p, string filename)
{
    assert(filename != "");
    //Fill p with the array and the starting position.
    ifstream input(filename);
    for(int i = 0; i < maxPos.row; i++)
    {
        for(int j = 0; j <= maxPos.col; j++)
        {
            input.get(p.board[i][j]);
            if(p.board[i][j] == 'w' || p.board[i][j] == 'W')
            {
                p.position = {j,i};
            }
        }
    }
    input.close();
}

///General

void print_array(Puzzle p)
{
    assert(true);
    //prints the board
    for(int i = 0; i < maxPos.row; i++)
    {
        for(int j = 0; j < maxPos.col; j++)
        {
            cout << p.board[i][j];
        }
        cout << endl;
    }
    cout << endl;
}

void move_empty(Puzzle& p, Pos a, Pos b)
{
    assert((a < maxPos && b < maxPos));
    //Moves the worker from a to b. If a box can be pushed push this as well. When a worker or a box is pushed over an empty destination cell capitalize them.
    char placeholder;
    if(p.board[a.row][a.col] == 'w' && p.board[b.row][b.col] == 'b' && p.board[2*b.row-a.row][2*b.col-a.col] != '*')
    {
        move_empty(p,b,2*b-a);
    }
    else if(p.board[a.row][a.col] == 'b' && p.board[b.row][b.col] == '.')
    {
        p.board[b.row][b.col] = ' ';
        p.board[a.row][a.col] = 'B';
    }
    else if(p.board[a.row][a.col] == 'w' && p.board[b.row][b.col] == '.')
    {
        p.board[b.row][b.col] = ' ';
        p.board[a.row][a.col] = 'W';
    }
    else if(p.board[a.row][a.col] == 'W' && p.board[b.row][b.col] == ' ')
    {
        p.board[b.row][b.col] = '.';
        p.board[a.row][a.col] = 'W';
    }
    placeholder = p.board[a.row][a.col];
    p.board[a.row][a.col] = p.board[b.row][b.col];
    p.board[b.row][b.col] = placeholder;
    p.position = b;
}

bool puzzle_ready(Puzzle p)
{
    assert(true);
    //returns true if puzzle has no character 'b' in it
    for(int i = 0; i < maxPos.row; i++)
    {
        for(int j = 0; j < maxPos.col; j++)
        {
            if(p.board[i][j] == 'b')
            {
                return false;
            }
        }
    }
    return true;
}
bool can_go(Puzzle p, Pos direction)
{
    assert(true);
    //returns true if the worker can walk in a direction.
    if(p.board[p.position.row + direction.row][p.position.col+direction.col] == 'b')
    {
        if(p.board[p.position.row + 2*direction.row][p.position.col+ 2*direction.col] == '*')
        {
            return false;
        }
        return true;
    }
    else
    {
        return p.board[p.position.row + direction.row][p.position.col+direction.col] != '*' && p.board[p.position.row + direction.row][p.position.col+direction.col] != 'B';
    }
}

///Breadth

bool puzzle_present(vector<Candidate> c, int i, Puzzle p)
{
    assert(true);
    //returns true if the current board state was found in another state in c.
    for(i; i >= 0; i--)
    {
        for(int j = 0; j < maxPos.row; j++)
        {
            for(int k = 0; k < maxPos.col; k++)
            {
                if(c[i].candidate.board[j][k] != p.board[j][k])
                {
                    return false;
                }
            }
        }
    }
    return true;
}
void show_path(vector<Candidate> c, int i)
{
    assert(true);
    //prints the path and displays the amount of steps.
    vector<Puzzle> print (0);
    int counter = -1;
    while(i != -1)
    {
        print.push_back(c[i].candidate);
        i = c[i].parent_candidate;
        counter++;
    }
    for(int j = print.size()-1; j >= 0 ; j--)
    {
        print_array(print[j]);
    }
    cout << "Shortest steps: " << counter << "\n\n";
}
void tries_breadth(vector<Candidate>&  c, int i, Pos next)
{
    assert(true);
    //moves a board to the next state and adds them to c if the boardstate hasn't been encountered yet.
    Puzzle newp = c[i].candidate;
    move_empty(newp,newp.position,next);
    Candidate newc = {newp, i};
    if (!puzzle_present(c,i,newp))
    {
        c.push_back(newc);
    }
}
void solve_breadth(Puzzle start)
{
    assert(true);
    //executes a breadth-first search and shows the path if the board can be solved.
    vector<Candidate> c = {{start,-1}} ;
    int i = 0 ;
    while(i < c.size() && !puzzle_ready(c[i].candidate))
    {
        Puzzle p = c[i].candidate;
        for(int j = North; j <= West; j++)
        {
            if(can_go(p, Winds[j]))
            {
                //cout << "Try " << node << endl;
                node++;
                tries_breadth(c, i, p.position + Winds[j]);
            }
        }
        i = i+1;
    }

    if(i < c.size())
    {
        show_path(c, i);
    }

}

///Depth

bool puzzle_present_depth(Puzzle p, vector<Puzzle> c)
{
    assert(c.size() > 0);
    //returns true if the current board state was found in another state in c.
    for(int i = 0; i < c.size(); i++)
    {
        for(int j = 0; j < maxPos.row; j++)
        {
            for(int k = 0; k < maxPos.col; k++)
            {
                if(c[i].board[j][k] != p.board[j][k])
                {
                    return false;
                }
            }
        }
    }
    return true;
}

void show_vector(vector<Puzzle> p)
{
    assert(p.size() > 0);
    //prints out every boardstate stored in p
    int i;
    for(i = 0; i < p.size() ; i++)
    {
        print_array(p[i]);
    }
    cout << "Shortest steps: " << p.size()-1 << "\n\n";
}
void tries_depth( vector<Puzzle>& attempt,  Pos next, vector<Puzzle>& shortest, int max_depth)
{
    assert(true);
    //moves a board to the next state. After this prepair, recurse with solve_depth and repair if the boardstate hasn't been encountered yet.
    Puzzle newp = attempt[attempt.size()-1]  ;
    move_empty(newp, newp.position, next);
    if (!puzzle_present_depth(newp, attempt))
    {
        attempt.push_back(newp);// prepare
        solve_depth(attempt, shortest, max_depth);
        attempt.pop_back(); // repair
    }
}

void solve_depth(vector<Puzzle>& attempt, vector<Puzzle>& shortest, int max_depth)
{
    assert(true);
    //executes a depth-first search.
    const int CURRENT = attempt.size() ;
    const int BEST    = shortest.size() ;
    Puzzle p = attempt[CURRENT-1]  ;
    if(BEST > 0 && CURRENT>= BEST)
    {
        return;
    }
    else if(CURRENT > max_depth+1)
    {
        return;
    }
    else if(puzzle_ready(p))
    {
        shortest = attempt;
        return;
    }
    else
        for(int j = 0; j <= 3; j++)
        {
            if(can_go(p, Winds[j]))
            {
                node++;
                tries_depth(attempt, attempt[attempt.size()-1].position + Winds[j], shortest, max_depth);
            }
        }
}

/**< Made by Janik Kreuning s1027402 and Axel van Abkoude s1021909 */
int main()
{
    int max_depth;
    Puzzle board;
    string filename = get_filename();
    get_puzzle_size(filename,maxPos);
    get_file(board,filename);
    print_array(board);
    vector<Puzzle> Attempt = {board};
    vector<Puzzle> Solution (0);
    string choice;
    //cout << "Starting position: " << board.position << endl;
    cout << "Type breadth (B) for breadth first search or Type depth (D) for depth first search.\n>";
    cin >> choice;
    if(choice == "breadth" || choice == "B")
    {
        cout << "Breadth first search: \n\n";
        solve_breadth(board);
        cout << "Solved in " << node << " tries." << endl;
    }
    else if(choice == "depth" || choice == "D")
    {
        cout << "Depth first search: \n\nInput the maximum depth: ";
        cin >> max_depth;
        cout << "\nMaximum depth: "<< max_depth << "\n\n";
        solve_depth(Attempt,Solution,max_depth);
        show_vector(Solution);
        cout << "Solved in " << node << " tries." << endl;
    }
    else
    {
        cout << "Not a valid input." << endl;
    }
    return 0;
}
