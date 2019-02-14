#include <lcom/lcf.h>

/**
 * @brief struct sprite
 * 
 */
typedef struct {
	int x,y;
	int width, height;
	int xspeed,yspeed;
	char *map;
} Sprite;

/**
 * @brief gets the info about the mode
 * 
 */
int(vbe_get_mode_info__)(uint16_t mode);
/**
 * @brief Set the video mode object
 * 
 * @param mode	the mode to set 
 * @return int 	returns if everything went ok
 */
int set_video_mode(uint16_t mode);
/**
 * @brief draws a rectangle
 * 
 * @param x	inicial x coordenate 
 * @param y inicial y coordenate 
 * @param width	width of the rectangle 
 * @param height	height of the rectangle 
 * @param color	color of the rectangle 
 */
void draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);
/**
 * @brief Set the pixel object
 * 
 * @param x	inicial x coordenate	 
 * @param y	inicial y coordenate  
 * @param color	color of the pixel 
 */
void set_pixel(uint16_t x, uint16_t y,uint32_t color);
/**
 * @brief Draws a xpm into the buff
 * 
 * @param map	char array originated by the xpm 
 * @param width	width of the xpm 
 * @param height 
 * @param x 
 * @param y 
 * @return int 
 */
int DrawMap(char *map,int width,int height,uint16_t x,uint16_t y);
/**
 * @brief moves the xpm
 * 
 * @param sp	sprite 
 * @param deltaf	delta f 
 * @return int	returns if everything went ok
 */
int MoveMap(Sprite * sp,  unsigned int deltaf);
/**
 * @brief draws a pattern of rectangles
 * 
 * @param no_rectangles	number of rectangles to draw 
 * @param first	colour 
 * @param step	how much the colour fades 
 */
void drawPattern( uint8_t no_rectangles, uint32_t first, uint8_t step);
/**
 * @brief Determines the colour based on the step and first
 * 
 * @param no_rectangles 
 * @param first	colour  
 * @param step	how much the colour fades 
 * @param row	the begining row 
 * @param col	the begining col 
 * @return uint32_t	returns the colour to use
 */
uint32_t determineColour(uint8_t no_rectangles, uint32_t first,uint8_t step, uint8_t row,uint8_t col);
/**
 * @brief Get the mode controller object
 * 
 * @return int	if everything went ok
 */
int get_mode_controller();
/**
 * @brief Create a sprite object
 * 
 * @param xpm	xpm to move 
 * @param x	starting x
 * @param y	starting y
 * @param xspeed speed in the x coordenate
 * @param yspeed speed in the y coordenate 
 * @return Sprite*	returns a sprite
 */
Sprite *create_sprite(const char *xpm[], int x, int y, int xspeed, int yspeed);
/**
 * @brief draws a line from xOld, yOld to x, y
 * 
 * @param color	color of the line 
 * @param x	final x 
 * @param y	final y 
 * @param xOld	initial x 
 * @param yOld	initial y 
 */
void draw_line(uint32_t color,uint16_t x,uint16_t y,uint16_t xOld,uint16_t yOld);
/**
 * @brief fills the cursor with the backroung of where it is going to go
 * 
 * @param x	inicial x coordinate
 * @param y	inicial y coordinate 
 * @param width width of the cursor 
 * @param height	height of the cursor
 */
void fill(uint16_t x,uint16_t y,uint16_t width,uint16_t height);
/**
 * @brief ersases the cursor with the array from the fill function, restoring the values
 * 
 * @param x	inicial x coordinate	 
 * @param y	inicial y coordinate 
 * @param width	width of the cursor 
 * @param height	height of the cursor 
 */
void erase_cursor(uint16_t x,uint16_t y,uint16_t width,uint16_t height);
/**
 * @brief Get the Hres object
 * 
 * @return unsigned the horizontal resolution 
 */
unsigned getHres();
/**
 * @brief Get the Vres object
 * 
 * @return unsigned	the vertical resolution
 */
unsigned getVres();
/**
 * @brief draws the backgroung
 * 
 */
void drawColorSelection();
/**
 * @brief draws the water
 * 
 */
void drawLoop();
/**
 * @brief updates the choice selection
 * 
 */
void update_selection();
/**
 * @brief selects the color given a scan code
 * 
 * @param scanCode	scan code 	
 * @return uint32_t	color 
 */
uint32_t select_Color(uint8_t scanCode);
/**
 * @brief converts an scan code to a drawing
 * 
 * @param scan	scan code to convert
 * @param xmemory	inicial x coordinate	 
 * @param ymemory	inicial y coordinate 
 */
void ScanToDraw(uint8_t scan, int xmemory, int ymemory);
/**
 * @brief converts an int to a draw
 * 
 * @param num	number to convert  
 * @param x	inicial x coordinate 
 * @param y	inicial y coordinate 
 */
void IntToDraw(int num,int x,int y);
/**
 * @brief Draws the timer
 * 
 * @param min	minute to draw 
 * @param sec	second to draw 
 */
void DrawTimer(int min, int sec);
/**
 * @brief transfers the contents from the drawing buffer to the video mem
 * 
 */
void framePrint();
/**
 * @brief draws the cursor
 * 
 * @param x	inicial x coordinate 
 * @param y	inicial y coordinate 
 * @return int if everything went ok	 
 */
int DrawBrush( int16_t x, int16_t y);
/**
 * @brief draws the menu
 * 
 */
void Menu_draw();
/**
 * @brief selects options depending on the scan code and size between options
 * 
 * @param option 
 * @param width_size 
 */
void select_Options(uint8_t option,int width_size,int x);
/**
 * @brief tranfers the contents in the background to the video mem
 * 
 */
void backround_to_videomem();
/**
 * @brief draws the initial backgroung
 * 
 */
void inicialBackround();
/**
 * @brief draws the pause menu
 * 
 */
void continue_draw();
/**
 * @brief draws the new game menu
 * 
 */
void new_game_draw();
/**
 * @brief	tranfers the contents in the background to the drawing buf 
 * 
 */
void draw_To_background();
/**
 * @brief	tranfers the contents in the drawing buff to the background buf 
 * 
 */
void background_To_draw();
/**
 * @brief draws a word
 * 
 * @param x	inicial x coordinate	 
 * @param y	inicial x coordinate 
 */
void DrawWord(int x,int y);
/**
 * @brief Displays a word
 * 
 */
void DisplayWord();
/**
 * @brief Makes the cursor be able to travel through the whole screen
 * 
 * @param x	inicial x coordinate 
 * @param y	inicial y coordinate 
 * @return int	if everything went ok 
 */
int DrawBrushFullScreen(int16_t x, int16_t y);
/**
 * @brief draw brush for the continue menu
 * 
 * @param x	inicial x coordinate 
 * @param y	inicial y coordinate 
 * @return int	if everything went ok 
 */
int DrawBrush1Buff(int16_t x, int16_t y);
/**
 * @brief draws the highscore background
 * 
 */
void high_score_background();
/**
 * @brief draws the cat from the highscore animation
 * 
 */
void bongo_cat_1_draw();
/**
 * @brief draws the rest of the cat from the highscore animation
 * 
 */
void bongo_cat_2_draw();
/**
 * @brief presents the score in the screen
 * 
 * @param score	score from the player 
 * @param number_of_digits	number of digits of the score 
 */
void drawScore(int score, int number_of_digits);
/**
 * @brief presents a determinate number in the screen with a big font 
 * 
 * @param num	number to present 
 * @param x	inicial x coordinate	 
 * @param y	inicial y coordinate 
 */
void IntToBigDraw(int num, int x, int y);




