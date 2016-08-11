/*
CMPT 361 Assignment 1 - FruitTetris implementation Sample Skeleton Code

- This is ONLY a skeleton code showing:
How to use multiple buffers to store different objects
An efficient scheme to represent the grids and blocks

- Compile and Run:
Type make in terminal, then type ./FruitTetris

This code is extracted from Connor MacLeod's (crmacleo@sfu.ca) assignment submission
by Rui Ma (ruim@sfu.ca) on 2014-03-04. 

Modified in Sep 2014 by Honghua Li (honghual@sfu.ca).
*/

#include "include/Angel.h"
#include <cstdlib>
#include <iostream>

using namespace std;


// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400; 
int ysize = 720;

// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)

// An array storing all possible orientations of all possible tiles
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
vec2 allRotationsLshape[4][4] = 
	{{vec2(0, 0), vec2(-1,0), vec2(1, 0), vec2(-1,-1)},
	{vec2(0, 1), vec2(0, 0), vec2(0,-1), vec2(1, -1)},
	{vec2(1, 1), vec2(-1,0), vec2(0, 0), vec2(1,  0)},
	{vec2(-1,1), vec2(0, 1), vec2(0, 0), vec2(0, -1)}};

vec2 allRotationsSshape[2][4] = 
	{{vec2(1, 0), vec2(0,0), vec2(0, -1), vec2(-1,-1)},
	{vec2(1, 0), vec2(1,-1), vec2(0,0), vec2(0, 1)}};


vec2 allRotationsIshape[2][4] = 
	{{vec2(1, 0), vec2(0,0), vec2(-1, 0), vec2(-2,0)},
	{vec2(0, 1), vec2(0, 0), vec2(0,-1), vec2(0, -2)}};

vec2 allRotationsTshape[4][4] = 
	{{vec2(0, 0), vec2(-1,0), vec2(1, 0), vec2(0,1)},
	{vec2(0, 0), vec2(-1,0), vec2(0, -1), vec2(0,1)},
	{vec2(0, 0), vec2(-1,0), vec2(1, 0), vec2(0,-1)},
	{vec2(0, 0), vec2(1,0), vec2(0, -1), vec2(0,1)}};



// colors
vec4 purple = vec4(1.0, 0.0, 1.0, 1.0);
vec4 red 	= vec4(1.0, 0.0, 0.0, 1.0); 
vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0); 
vec4 green  = vec4(0.0, 1.0, 0.0, 1.0); 
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0); 
vec4 white  = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = vec4(0.0, 0.0, 0.0, 1.0); 

 
 //array containing all the colors used for assigning tile colors
vec4 allColours[5] = {purple,red,yellow,green,orange};
vec4 tileColor[4] = {red, red, red, red};
int tileShape;
int rotCounter;
 //Global Variables

int yCord[4];//the y coordinate of the current tile that's being dropped
int xCord[4];//the x coordinate of the current tile that's being dropped

//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20]; 

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200];

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;

// VAO and VBO
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]); 

	// For each of the 4 'cells' of the tile,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = tilepos.x + tile[i].x; 
		GLfloat y = tilepos.y + tile[i].y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1); 
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// Two points are used by two triangles each
		vec4 newpoints[6] = {p1, p2, p3, p2, p3, p4}; 

		//tracks the location of the current tile
		xCord[i]= (int)x;
		yCord[i]= (int)y;

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*6*sizeof(vec4), 6*sizeof(vec4), newpoints); 
	}

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

// Called at the start of play and every time a tile is placed
void newtile()
{
	rotCounter =0;
	tilepos = vec2((rand() % 7+ 2) , 19); // Put the tile at the top of the board

	int randTile = rand()%4;
	// Update the geometry VBO of current tile
	if(randTile == 0){
		tileShape=0;
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[0][i]; // Get the 4 pieces of the new tile
	}
	if(randTile == 1){
		tileShape=1;
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsSshape[0][i]; // Get the 4 pieces of the new tile
	}
	if(randTile == 2){
		tileShape=2;
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsIshape[0][i]; // Get the 4 pieces of the new tile
	}
	if(randTile == 3){
		tileShape=3;
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsTshape[0][i]; // Get the 4 pieces of the new tile
	}

	updatetile(); 
	// Update the color VBO of current tile
	vec4 newcolours[24];
	for (int i = 0; i < 4; i++){
		int randColor = rand()%5;
		tileColor[i] = allColours[randColor];
		for (int j=0;j<6;j++){
			newcolours[6*i+j] = allColours[randColor]; // You should randomlize the color	
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

void initGrid()
{
	// ***Generate geometry data
	vec4 gridpoints[64]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
	vec4 gridcolours[64]; // One colour per vertex
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
		gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);
		
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 0, 1);
	}
	// Make all grid lines white
	for (int i = 0; i < 64; i++)
		gridcolours[i] = white;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}


void initBoard()
{
	// *** Generate the geometric data
	vec4 boardpoints[1200];
	for (int i = 0; i < 1200; i++)
		boardcolours[i] = black; // Let the empty cells on the board be black
	// Each cell is a square (2 triangles with 6 vertices)
	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 10; j++)
		{		
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			
			// Two points are reused
			boardpoints[6*(10*i + j)    ] = p1;
			boardpoints[6*(10*i + j) + 1] = p2;
			boardpoints[6*(10*i + j) + 2] = p3;
			boardpoints[6*(10*i + j) + 3] = p2;
			boardpoints[6*(10*i + j) + 4] = p3;
			boardpoints[6*(10*i + j) + 5] = p4;
		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false; 


	// *** set up buffer objects
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// No geometry for current tile initially
void initCurrentTile()
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init()
{
	// Load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	// Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(3, &vaoIDs[0]);

	// Initialize the grid, the board, and the current tile
	initGrid();
	initBoard();
	initCurrentTile();

	// The location of the uniform variables in the shader program
	locxsize = glGetUniformLocation(program, "xsize"); 
	locysize = glGetUniformLocation(program, "ysize");

	// Game initialization
	newtile(); // create new next tile

	// set to default
	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------

// Rotates the current tile, if there is room
void rotate()
{      
	for(int i=0;i<4;i++){
		if(tileShape==0){
			if(rotCounter%4==1 && tilepos.x==0){
				tile[i]=allRotationsLshape[(rotCounter+1)%4][i];
				tilepos.x++;
			}
			if(rotCounter%4==3 && tilepos.x==9){
				tile[i]=allRotationsLshape[(rotCounter+1)%4][i];	
				tilepos.x--;
			}
			else{
				tile[i]=allRotationsLshape[(rotCounter+1)%4][i];//other scenarios are fine
			}
		}
		if(tileShape==1){
			if(rotCounter%2==1 && tilepos.x==0){
				tile[i]=allRotationsSshape[(rotCounter+1)%2][i];	
				tilepos.x++;
			}
			else{
				tile[i]=allRotationsSshape[(rotCounter+1)%2][i];//other scenarios are fine
			}
		}
		if(tileShape==2){
			if (rotCounter%2==0){
				tile[i]=allRotationsIshape[(rotCounter+1)%2][i];
			}
			else{//this is to avoid collision with board edges
				tile[i]=allRotationsIshape[(rotCounter+1)%2][i];
				if (tilepos.x<2){
					tilepos.x +=2;//prevending left side collision	
				}
				if (tilepos.x>8){
					tilepos.x--;//prevending right side collision
				}
			}
		}
		if(tileShape==3){
			if(rotCounter%4==1 && tilepos.x==9){
				tile[i]=allRotationsTshape[(rotCounter+1)%4][i];
				tilepos.x--;
			}
			if(rotCounter%4==3 && tilepos.x==0){
				tile[i]=allRotationsTshape[(rotCounter+1)%4][i];	
				tilepos.x++;
			}
			else{
				tile[i]=allRotationsTshape[(rotCounter+1)%4][i];//other scenarios are fine
			}
		}

	}
	rotCounter++;
}

//-------------------------------------------------------------------------------------------------------------------

// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
void checkfullrow()
{
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	int check=0;
	for (int b=0;b<4;b++){//check for full row 4 times since thats the maximum possible
		for (int i=0;i<20;i++){//check each row
			for (int j=0;j<10;j++){//check for full row
				if (board[j][i]==true)
					check++;	
			}
			if (check==10){//cancel a whole row
				for (int k=0; k<60; k++)
					for (int a=i;a<20;a++){
						if (a<19){
							boardcolours[a*60+k] = boardcolours[(a+1)*60+k];	
						}
						else{
							boardcolours[a*60+k] = black;
						}
					}
				for (int k=0; k<10; k++)
					for (int a=i;a<20;a++){
						if (a<19){
							board[k][a] = board[k][a+1];
						}
						else{
							board[k][a] = false;
						}
					}
			}
			check=0;
		}
	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------
//Check for 3 fruits in a row

void checkFruit(){
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	for (int i=0;i<4;i++){
		//check for left side
		if (boardcolours[6*(10*(int)(tilepos.y+tile[i].y) + (int)(tilepos.x + tile[i].x))-6]==tileColor[i]){
			if(boardcolours[6*(10*(int)(tilepos.y+tile[i].y) + (int)(tilepos.x + tile[i].x))-12]==tileColor[i]){
				for (int j=0;j<18;j++){
					boardcolours[6*(10*(int)(tilepos.y+tile[i].y) + (int)(tilepos.x + tile[i].x)) -j + 5]=black;
				}
			}
		}
	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
	glBindVertexArray(0);
}
//-------------------------------------------------------------------------------------------------------------------

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile()
{
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	for (int i=0; i<4; i++){
		for (int j=0; j<=5; j++){
			boardcolours[6*(10*(int)(tilepos.y+tile[i].y) + (int)(tilepos.x + tile[i].x)) + j] = tileColor[i];
		}
		board[(int)(tilepos.x + tile[i].x)][(int)(tilepos.y+tile[i].y)] = true;
	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
	glBindVertexArray(0);
	checkfullrow();
}

//-------------------------------------------------------------------------------------------------------------------

// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool movetile(vec2 direction)
{
	bool moveCheck = true;
	
	for (int i=0;i<4;i++)
	{
		if ((tilepos.x+direction.x+tile[i].x)>9 || (tilepos.x+direction.x+tile[i].x)<0 || board[(int)(xCord[i]+direction.x)][(int)(yCord[i]+direction.y)])
			moveCheck = false;
	}
	if(moveCheck == true){
		tilepos.x = tilepos.x+direction.x;
		tilepos.y = tilepos.y+direction.y;
	}
	updatetile();
	return moveCheck;
}

//-------------------------------------------------------------------------------------------------------------------

void down(int a)
{
		tilepos.y--;
		updatetile();
		glutTimerFunc(350,down, 350);
		glutPostRedisplay();
	
}

//-------------------------------------------------------------------------------------------------------------------

// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{
	newtile();
	initBoard(); //reinitialize board
}
//-------------------------------------------------------------------------------------------------------------------

// Draws the game
void display()
{

	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
	glUniform1i(locysize, ysize);

	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board (10*20*2 = 400 triangles)

	glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 24); // Draw the current tile (8 triangles)

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 64); // Draw the grid lines (21+11 = 32 lines)


	glutSwapBuffers();
}

//-------------------------------------------------------------------------------------------------------------------

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

//-------------------------------------------------------------------------------------------------------------------

// Handle arrow key keypresses
void special(int key, int x, int y)
{
	switch(key) 
	{
		case GLUT_KEY_UP:
		    rotate();
		    break;
		case GLUT_KEY_RIGHT:
			movetile(vec2(1,0));
			break;
		case GLUT_KEY_LEFT:
			movetile(vec2(-1,0));
			break;
		case GLUT_KEY_DOWN:
			movetile(vec2(0,-1));
			break;
	}
	updatetile();
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // Both escape key and 'q' cause the game to exit
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': // 'r' key restarts the game
			restart();
			break;
	}
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

void idle(void)
{
	for (int i=0; i<4; i++){
			if(yCord[i]==0 || board[xCord[i]][yCord[i]-1] == true){//when the current tile reaches the bottom of the board or a collision happens we place the current tile and drop a new one
				settile();
				newtile();
			}
	}
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	srand (time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
	glutCreateWindow("Fruit Tetris");
	glewInit();
	init();
	glutTimerFunc(350,down,350);//drops the tile

	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop(); // Start main loop
	return 0;
}
