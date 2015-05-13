/*
 *This code is loosely base off the project found here http://www.kosbie.net/cmu/fall-10/15-110/handouts/snake/snake.html
 *
 *This is a Snake Game to be used with an Adafruit NeoPixel Sheild and
 *a PS/2 Keyboard.
 *
 *How to Play:
 *
 *The Snake will start on the bottom of the board
 *The Snake is shown as a blue dot
 *To Start the game press Enter on the Keyboard, the snake will start to move up
 *The Green Dot is the Snakes food and the grow the Snake by one
 *To Reset the game press the Delete Button
 *
 *Rules:
 *If the Snake goes outside the game zone aka off the board the game is over
 *If the Snake goes back onto itself, the game is over
 *
 *This program is free to be used by all, Have Fun. Please, if you mod this code,
 *put somthing in the header to let others know where it came from. Thanks.
 *
 *Created by Mario Avenoso <mario@mtechcreations.com> Mario.mtechcreations.com
 *11/14/13
 */

#include <Adafruit_NeoPixel.h>//can be found at https://github.com/adafruit/Adafruit_NeoPixel
#include <PS2Keyboard.h>//can be found at http://playground.arduino.cc/Main/PS2Keyboard

#define PIN 6//data pin for NeoPixel

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(256, PIN, NEO_GRB + NEO_KHZ800);

#define X 16//this is the depth of the field
#define Y 16//this is the length of the field

const int DataPin = 8;//PS/2 pin 1
const int IRQpin =  2;//PS/2 pin 5

PS2Keyboard keyboard;//create a new keyboard

//global vars
int hrow=0,hcol=0;//sets the row and col of the snake head
bool game = true;//game is good
bool start = false;//start the game with true
bool ignoreNextTimer=false;//tells the game timer weither or not it should run
//When true the game timer will not update due to the update by a keypress
int sx=4,sy=3;//set snake location
long previousMillis = 0;//used for the game timer
long interval = 350; //how long between each update of the game
unsigned long currentMillis = 0;//used for the game timer

int sDr=-1,sDc=0;//used to keep last direction, start off going up

//int array[Y * X];
uint16_t SetElement(uint16_t, uint16_t);//2D array into a number

int gameBoard[X][Y] = //game field, 0 is empty, -1 is food, >0 is snake
{
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},	
};

void setup()
{
	Serial.begin(9600);//used for debugging
	hrow=sx;//set the row of the snake head
	hcol=sy;//set the col of the snake head
	randomSeed(analogRead(0));//used to help make a better random number
	keyboard.begin(DataPin, IRQpin);//start the keybaord
	strip.begin();//start the NeoPixel Sheild
	strip.show(); // Initialize all pixels to 'off'
	resetGame();//clear and set the game
	
}

void loop()
{
	
	currentMillis = millis();//get the current time
	//game clock
	if(currentMillis - previousMillis >= interval) {//check if the 
	// current time - the last time is greater the the delay time you want
		previousMillis = currentMillis;
		if (game&&start&&ignoreNextTimer==false)//checks to make sure the game is good and running
		{
			
			//moveSnake(sDr,sDc);
			drawBoard();
			updateGame();
			//delay(1000);
			
		}
		ignoreNextTimer=false;//resets the ignore bool
	}
	
	if (keyboard.available()) {//checks to see if there was a keypress
		checkKeyboard();
	}
		
			
	
		
}

void checkKeyboard()
{
	if (keyboard.available()) {//checks to see if there was a keypress
		
		// read the next key
		char c = keyboard.read();
		if (c == PS2_LEFTARROW || c == 'a')//move left
		{
			Serial.print("[Left]");
			if (game&&start)
			{
				moveSnake(0,-1);
				ignoreNextTimer=true;
			}
			
			
		}
		else if (c == PS2_RIGHTARROW || c == 'd')//move right
		{
			Serial.print("[Right]");
			if (game&&start)
			{
				moveSnake(0,1);
				ignoreNextTimer=true;
			}
		}
		else if (c == PS2_UPARROW || c == 'w')//move up
		{
			Serial.print("[Up]");
			if (game&&start)
			{
				moveSnake(-1,0);
				ignoreNextTimer=true;
			}
		}
		else if (c == PS2_DOWNARROW || c == 's')//move down
		{
			Serial.print("[Down]");
			if (game&&start)
			{
				moveSnake(1,0);
				ignoreNextTimer=true;
			}
		}
		else if(c == PS2_DELETE)//reset the game
		{
			resetGame();
		}
		else if (c==PS2_ENTER)//start the game
		{
			start = true;
			drawBoard();
		}
	}
}

void updateGame()
{
	if (game && start)//checks to make sure the game is running and good
	{
		moveSnake(sDr,sDc);//keeps the Snake moving in the current direction
		
	}
	if (game && start))//checks to make sure the game is running and good
	{
		
		drawBoard();//update the screen
	}
	
}

void resetGame()
{
	resetBoard();
	sDr=-1;
	sDc=0;
	loadSnake();
	placeFood();
	findSnakeHead();//find where the snake is starting from
	game=true;
	start=false;
	ignoreNextTimer=false;
	drawBoard();
}

void placeFood()
{
	int rx=0,ry=0;
	rx = random(0,X-1);
	ry = random(0,Y-1);
	if (gameBoard[rx][ry]>0)
	{
		
		while(gameBoard[rx][ry]>0)
		{
			
			rx = random(0,X-1);
			ry = random(0,Y-1);
			if (gameBoard[rx][ry]==0)
			{
				gameBoard[rx][ry]=-1;
				//drawBoard();
				break;
			}
		}
	}
	else
	{
		gameBoard[rx][ry]=-1;
		//drawBoard();
	}
	
}

void loadSnake()
{
	gameBoard[sx][sy]=1;
	
}
void resetBoard()
{
	for(int x=0;x<X;x++)
	{
		for(int y =0;y< Y;y++)
		{
			gameBoard[x][y]=0;
		}
		
	}
	loadSnake();
}

void gameOver()
{
	game = false;
	start = false;
	for(int light=0;light<255;light++)
	{
		for(int i =0;i< strip.numPixels();i++)
		{
			strip.setPixelColor(i,strip.Color(light,0,0));
		}
		strip.show();
		delay(15);
	}
	
	strip.show();
}

void moveSnake(int row, int col)//row and col
{
	
	sDr = row;
	sDc = col;
	Serial.print(sDr);
	Serial.print(",");
	Serial.println(sDc);
	int new_r=0,new_c=0;
	new_r=hrow+row;
	new_c=hcol+col;
	if (new_r>=X||new_r<0||new_c>=Y||new_c<0)
	{
		gameOver();
	}
	else if(gameBoard[new_r][new_c]>0)
	{
		gameOver();
	}
	else if (gameBoard[new_r][new_c]==-1)
	{
		gameBoard[new_r][new_c] = 1+gameBoard[hrow][hcol];
		hrow=new_r;
		hcol=new_c;
		placeFood();
		drawBoard();
	}
	else
	{
		gameBoard[new_r][new_c] = 1+gameBoard[hrow][hcol];
		hrow=new_r;
		hcol=new_c;
		removeTail();
		drawBoard();
	}
	
}

void removeTail()
{
	for (int x=0;x<X;x++)
	{
		for (int y=0;y<Y;y++)
		{
			if(gameBoard[x][y]>0)
			{
				gameBoard[x][y]--;
			}
		}
	}
}

void drawBoard()
{
	clear_dsp();
	for (int x=0;x<X;x++)
	{
		for (int y=0;y<Y;y++)
		{
			if(gameBoard[x][y]==-1)//food
			{
				strip.setPixelColor(SetElement(x,y),strip.Color(0,100,0));
			}
			
			else if(gameBoard[x][y]==0)//empty
			{
				strip.setPixelColor(SetElement(x,y),strip.Color(0,0,0));
			}
			else
			{
				strip.setPixelColor(SetElement(x,y),strip.Color(0,0,10));
			}
			
		}
	}
	strip.show();
}

void findSnakeHead()
{
	hrow=0;//clearing out old location
	hcol=0;//clearing out old location
	
	for (int x=0;x<X;x++)
	{
		for (int y=0;y<Y;y++)
		{
			if (gameBoard[x][y]>gameBoard[hrow][hcol])
			{
				hrow=x;
				hcol=y;
			}
		}
	}
}


void clear_dsp()
{
	for(int i =0;i< strip.numPixels();i++)
	{
		strip.setPixelColor(i,strip.Color(0,0,0));
	}
	strip.show();
}

uint16_t SetElement(uint16_t row, uint16_t col)
{
	//array[width * row + col] = value;
	return Y * row+col;
}

