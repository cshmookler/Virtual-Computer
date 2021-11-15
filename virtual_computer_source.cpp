#include <GL/freeglut.h>
#include <iostream>
#include <fstream>

// Useful links for FreeGLUT and OpenGL:
//    http://freeglut.sourceforge.net/docs/api.php
//    https://en.wikibooks.org/wiki/OpenGL_Programming

// Read README.md for a brief description of this project

// Declare constants

	// Window constants
	const char * WIN_DEFAULT_TITLE = "Virtual Computer",
			   * WIN_OP_LOG_DIR = "operation_log.txt";

	const int WIN_POS_X = 500,
			  WIN_POS_Y = 500,
			  WIN_WIDTH = 600,
			  WIN_HEIGHT = 600,
			  PIXEL_COUNT_X = 32,
			  PIXEL_COUNT_Y = 32;

	const GLfloat PIXEL_SIZE_X = 2.0f / PIXEL_COUNT_X,
				  PIXEL_SIZE_Y = 2.0f / PIXEL_COUNT_Y;

	// Window icon constants
	const int WIN_ICON_WIDTH = 48,
			  WIN_ICON_HEIGHT = 48;

	const char * WIN_ICON_DIR = "assets/icon.ico";

	// Timer constants
	const int TIMER_MAIN = 1;

	// Virtual Computer constants
	const int VC_CLOCK_SPEED = 1000, // Instructions executed per second

			  VC_RAM_SIZE = 4096,

			  // Operation codes
			  VC_OP_LDA = 0,
			  VC_OP_LAA = 1,
			  VC_OP_ADD = 2,
			  VC_OP_SBD = 3,
			  VC_OP_ADA = 4,
			  VC_OP_SBA = 5,
			  VC_OP_STR = 6,
			  VC_OP_STD = 7,
			  VC_OP_SSD = 8,
			  VC_OP_JMP = 9,
			  VC_OP_JIZ = 10,
			  VC_OP_JIE = 11,
			  VC_OP_JII = 12,
			  VC_OP_JBT = 13,
			  VC_OP_GIN = 14,
			  VC_OP_SOT = 15,

			  // ALU constants
			  VC_ALU_ADD = 1,
			  VC_ALU_SUB = 2,
			  VC_ALU_OTHER = 3;

			  // Constants for pheripherals

	const char * VC_ROM = "data/bin_data/rom.dat",
			   * VC_DRIVE_1 = "data/bin_data/drive_1.dat";

// Declare variables

	// Pixel color array
	GLubyte pixelStoredColor[PIXEL_COUNT_X][PIXEL_COUNT_Y][4] = {0},
			pixelDisplayColor[PIXEL_COUNT_X][PIXEL_COUNT_Y][4] = {0};

	// Virtual Computer variables
		// All VC variables are set to 0 (zero) by default
	int ram[VC_RAM_SIZE] = {0},
		iar = 0,
		rA = 0,
		rB = 0,
		rC = 0,
		aluOp = 0,

		// Temporarily store input sent to from certain output devices
		VC_IH_cache[4096] = {0}, // words with even indices are the origin device and words with odd indices are the input data
		VC_IH_cache_stored = 0,
		VC_IH_cache_pos = 0,

		// Temporarily store output sent to certain output devices
		VC_OH_cache[5] = {0},
		VC_OH_stored = 0,

		opLog[VC_RAM_SIZE][4] = {0},
		opCount = 0;

	bool flag[3] = {false},
		 opOverflow = false;

// Declare and define functions
bool VC_main(void);
void VC_alu(int op);
int VC_IH();
void VC_OH(int io_device, int operand);
void VC_updateLog(void);
void WIN_main(int timerId);
void WIN_display(void);
void WIN_sizeChange(int w, int h);

int main(int argc, char** argv)
{
	// Init GLUT and create a window
	glutInit(&argc, argv);                          // Pass 'argc' and 'argv' to GLUT
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);    // Window display mode
	glutInitWindowPosition(WIN_POS_X, WIN_POS_Y);   // Default position
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);      // Default width and height
	glutCreateWindow(WIN_DEFAULT_TITLE);  // Create the window
	
	// Set custom icon
		// https://stackoverflow.com/questions/12748103/how-to-change-freeglut-main-window-icon-in-c
		// Kudos to szamil for this solution
	HWND hwnd = FindWindow(NULL, WIN_DEFAULT_TITLE);
	HANDLE icon = LoadImage(GetModuleHandle(NULL), WIN_ICON_DIR, IMAGE_ICON, WIN_ICON_WIDTH, WIN_ICON_HEIGHT, LR_LOADFROMFILE | LR_COLOR);
	SendMessage(hwnd, (UINT)WM_SETICON, ICON_BIG, (LPARAM)icon);

	// Set default window clear color
	glClearColor(255, 255, 255, 0); // White
 
	// Register callbacks
	glutDisplayFunc(WIN_display);    // Called to render the window
	glutReshapeFunc(WIN_sizeChange); // Called when the window size is changed
	glutCloseFunc(VC_updateLog);

	// Init Virtual Computer
	// Load data from ROM to RAM
	std::ifstream source(VC_ROM, std::ios::binary);
	if(!source.is_open())
	{
		std::cout << "Error: ROM file failed to open" << std::endl;
		return 0;
	}

	char c;
	int tempNum,
		num;
	bool byteType = true;
	for(int i = 1; i < VC_RAM_SIZE * 2; i++)
	{
		if(!source.get(c))
			break;

		tempNum = (int)c;
		if(tempNum < 0)
			tempNum += 256;

		if(byteType)
			num = tempNum * 256;
		else
			ram[(i / 2) - 1] = num + tempNum;

		byteType = !byteType;
	}

	source.close();

	// Start timers
	glutTimerFunc(1000 / VC_CLOCK_SPEED, WIN_main, TIMER_MAIN);

	// Display test
	for(GLubyte x = 0; x < PIXEL_COUNT_X; x++)
	{
		for(GLubyte y = 0; y < PIXEL_COUNT_Y; y++)
		{
			pixelDisplayColor[(int)x][(int)y][0] = x * 8;
			pixelDisplayColor[(int)x][(int)y][1] = y * 8;
			pixelDisplayColor[(int)x][(int)y][2] = y * 8;
		}
	}

	// Enter GLUT event processing cycle.
	glutMainLoop();

	return 1;
}

// This function is called before an instruction is executed in the virtual computer
void WIN_main(int timerId)
{
	// Reset main timer
	glutTimerFunc(1000 / VC_CLOCK_SPEED, WIN_main, TIMER_MAIN);

	// Virtual Computer executes one instruction and updates the display if requested
	if(VC_main() == 1)
		WIN_display();
}

void WIN_display(void)
{
	// Clear color buffer
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw
	glBegin(GL_QUADS);

		int pixelColor_x = 0,
			 pixelColor_y = 0;

		for(GLfloat x = -1.0f; x <= 1.0f; x += PIXEL_SIZE_X)
		{
			for(GLfloat y = -1.0f; y <= 1.0f; y += PIXEL_SIZE_Y)
			{
				glColor3ub(
							  pixelDisplayColor[pixelColor_x][pixelColor_y][0], 
							  pixelDisplayColor[pixelColor_x][pixelColor_y][1], 
							  pixelDisplayColor[pixelColor_x][pixelColor_y][2]
							  );
				glVertex2f(x,                y               );
				glVertex2f(x + PIXEL_SIZE_X, y               );
				glVertex2f(x + PIXEL_SIZE_X, y + PIXEL_SIZE_Y);
				glVertex2f(x,                y + PIXEL_SIZE_Y);

				pixelColor_y += 1;
			}

			pixelColor_x += 1;
			pixelColor_y = 0;
		}

	glEnd();

	glutSwapBuffers();
}

void WIN_sizeChange(int w, int h)
{
	glutReshapeWindow(WIN_WIDTH, WIN_HEIGHT);
}

bool VC_main(void)
{
	// Get instruction
	int opCode = ram[iar] >> 12,
		 operand = ram[iar] % 4096;

	// Execute instruction
	bool incIar = true;

	opLog[opCount][0] = iar;
	opLog[opCount][1] = opCode;
	
	switch(opCode)
	{
		case VC_OP_LDA:
			rA = operand;
			VC_alu(aluOp);
			opLog[opCount][2] = rA;
			break;
		case VC_OP_LAA:
			rA = ram[operand];
			VC_alu(aluOp);
			opLog[opCount][2] = rA;
			opLog[opCount][3] = operand;
			break;
		case VC_OP_ADD:
			rB = operand;
			aluOp = VC_ALU_ADD;
			VC_alu(aluOp);
			opLog[opCount][2] = rB;
			break;
		case VC_OP_SBD:
			rB = operand;
			aluOp = VC_ALU_SUB;
			VC_alu(aluOp);
			opLog[opCount][2] = rB;
			break;
		case VC_OP_ADA:
			rB = ram[operand];
			aluOp = VC_ALU_ADD;
			VC_alu(aluOp);
			opLog[opCount][2] = rB;
			opLog[opCount][3] = operand;
			break;
		case VC_OP_SBA:
			rB = ram[operand];
			aluOp = VC_ALU_SUB;
			VC_alu(aluOp);
			opLog[opCount][2] = rB;
			opLog[opCount][3] = operand;
			break;
		case VC_OP_STR:
			ram[operand] = rC;
			opLog[opCount][2] = rC;
			opLog[opCount][3] = operand;
			break;
		case VC_OP_STD:
		{
			rC = ~(~rA | rB);
			ram[operand] = rC;
			aluOp = VC_ALU_OTHER;
			VC_alu(aluOp);
			opLog[opCount][2] = rC;
			opLog[opCount][3] = operand;
			break;
		}
		case VC_OP_SSD:
		{
			int rA_temp = rA;
			for(int i = 0; i < rB % 16; i++)
			{
				if(rA_temp % 2 != 0) // If rA is odd
					rA_temp += 65536;
				rA_temp >>= 1; // Shift down by 1
			}
			rC = rA_temp;
			aluOp = VC_ALU_OTHER;
			VC_alu(aluOp);
			opLog[opCount][2] = rC;
			opLog[opCount][3] = operand;
			break;
		}
		case VC_OP_JMP:
			iar = operand;
			incIar = false;
			opLog[opCount][2] = operand;
			break;
		case VC_OP_JIZ:
			if(flag[0])
			{
				iar = operand;
				incIar = false;
				opLog[opCount][2] = 1;
				opLog[opCount][3] = operand;
			}
			else
			{
				opLog[opCount][2] = 0;
			}
			break;
		case VC_OP_JIE:
			if(flag[1])
			{
				iar = operand;
				incIar = false;
				opLog[opCount][2] = 1;
				opLog[opCount][3] = operand;
			}
			else
			{
				opLog[opCount][2] = 0;
			}
			break;
		case VC_OP_JII:
			if(flag[2])
			{
				iar = operand;
				incIar = false;
				opLog[opCount][2] = 1;
				opLog[opCount][3] = operand;
			}
			else
			{
				opLog[opCount][2] = 0;
			}
			break;
		case VC_OP_JBT:
		{
			if(rA & rB == rB)
			{
				iar = operand;
				incIar = false;
				opLog[opCount][2] = 1;
				opLog[opCount][3] = operand;
			}
			else
			{
				opLog[opCount][2] = 0;
			}
			break;
		}
		case VC_OP_GIN:
			ram[operand] = VC_IH();
			opLog[opCount][2] = operand;
			opLog[opCount][3] = ram[operand];
			break;
		case VC_OP_SOT:
			VC_OH(rA, operand);
			opLog[opCount][2] = rA;
			opLog[opCount][3] = operand;
			break;
	}

	// Increment IAR
	if(incIar)
	{
		iar += 1;
		if(iar >= VC_RAM_SIZE)
			iar = 0;
	}

	// Increment operation count
	opCount += 1;
	if(opCount >= VC_RAM_SIZE)
	{
		opCount = 0;
		opOverflow = true;
	}

	return 0;
}

void VC_alu(int op)
{
	long temp;

	if(op == VC_ALU_ADD)
	{
		temp = rA + rB;

		if(temp >= 65536)
		{
			rC = temp - 65536;
			flag[1] = true;
		}
		else
		{
			rC = temp;
			flag[1] = false;
		}
	}
	else if(op == VC_ALU_SUB)
	{
		temp = rA - rB;

		if(temp < 0)
		{
			rC = temp + 65536;
			flag[1] = true;
		}
		else
		{
			rC = temp;
			flag[1] = false;
		}
	}
	else if(op == VC_ALU_OTHER)
	{
		// Don't do anything
	}

	if(rC == 0)
		flag[0] = true;
	else
		flag[0] = false;
}

int VC_IH(bool operation, int word = 0)
{
	if(operation == true) // write to the IH
	{
		VC_IH_cache_pos += 1;
		if(VC_IH_cache_pos >= 4096)
		{
			VC_IH_cache_pos = 0;
		}

		VC_IH_cache[VC_IH_cache_pos] = word;

		VC_IH_cache_stored += 1;

		return 0;
	}
	else // read from the IH
	{
		if(VC_IH_cache_stored != 0)
		{
			int oldPos = VC_IH_cache_pos;

			VC_IH_cache_pos -= 1;
			if(VC_IH_cache_pos < 0)
			{
				VC_IH_cache_pos = 4095;
			}

			VC_IH_cache_stored -= 1;

			return VC_IH_cache[oldPos]
		}
		else
		{
			return 0;
		}
	}
}

void VC_OH(int io_device, int operand)
{

}

void VC_updateLog(void)
{
	std::ofstream target(WIN_OP_LOG_DIR, std::ios::trunc);

	for(int i = 0; i < opCount; i++)
	{
		target << "iar: " << opLog[i][0] << "   | ";

		switch(opLog[i][1])
		{
			case VC_OP_LDA:
				target << "LDA | rA <= " << opLog[i][2];
				break;
			case VC_OP_LAA:
				target << "LAA | rA <= ram[" << opLog[i][3] << "]   | (rA <= " << opLog[i][2] << ")";
				break;
			case VC_OP_ADD:
				target << "ADD | rA + rB   | (rB <= " << opLog[i][2] << ")";
				break;
			case VC_OP_SBD:
				target << "SBD | rA - rB   | (rB <= " << opLog[i][2] << ")";
				break;
			case VC_OP_ADA:
				target << "ADA | rA + rB   | (rB <= ram[" << opLog[i][3] << "])   | (rB <= " << opLog[i][2] << ")";
				break;
			case VC_OP_SBA:
				target << "SBA | rA - rB   | (rB <= ram[" << opLog[i][3] << "])   | (rB <= " << opLog[i][2] << ")";
				break;
			case VC_OP_STR:
				target << "STR | ram[" << opLog[i][3] << "] <= " << opLog[i][2];
				break;
			case VC_OP_STD:
				target << "STD | ram[" << opLog[i][3] << "] <= " << opLog[i][2];
				break;
			case VC_OP_SSD:
				target << "SSD | ram[" << opLog[i][3] << "] <= " << opLog[i][2];
				break;
			case VC_OP_JMP:
				target << "JMP | jump: " << opLog[i][2];
				break;
			case VC_OP_JIZ:
				target << "JIZ | ";
				if(opLog[i][2])
					target << "jump: " << opLog[i][3] << "   | zero flag was true";
				else
					target << "did not jump   | zero flag was false";
				break;
			case VC_OP_JIE:
				target << "JIE | ";
				if(opLog[i][2])
					target << "jump: " << opLog[i][3] << "   | extra flag was true";
				else
					target << "did not jump   | extra flag was false";
				break;
			case VC_OP_JII:
				target << "JII | ";
				if(opLog[i][2])
					target << "jump: " << opLog[i][3] << "   | input flag was true";
				else
					target << "did not jump   | input flag was false";
				break;
			case VC_OP_JBT:
				target << "JBT | ";
				if(opLog[i][2])
					target << "jump: " << opLog[i][3] << "   | all selected bits were true";
				else
					target << "did not jump   | one or more of the selected bits were false";
				break;
			case VC_OP_GIN:
				target << "GIN | ram[" << opLog[i][2] << "] <= " << opLog[i][3];
				break;
			case VC_OP_SOT:
				target << "SOT | outputDevice(" << opLog[i][2] << ") <= " << opLog[i][3];
				break;
		}

		target << "\n";
	}

	target.close();
}
