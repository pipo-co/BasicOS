#include <arkanoid.h>
#include <usrlib.h>

//Defines
    #define BRICKS_PER_COLUMN 3
    #define BRICKS_PER_ROW SCREEN_WIDTH / BRICKS_WIDTH

    #define BACKGROUND_COLOR 0x000000

    #define SCREEN_WIDTH 1024
    #define SCREEN_HEIGHT 768

    #define GUI_HEIGHT BRICKS_HEIGHT
    #define BRICKS_HEIGHT 32
    #define BRICKS_WIDTH 64

    #define BAR_WIDTH 128
    #define BAR_HEIGHT 32
    #define BAR_Y RADIUS * 87

    #define BAR_MOVEMENT 5

    #define RADIUS 8

    #define BALL_COLOR 0xffffff

    #define INVERT -1

    #define BRICK_PRESENT 1
    #define BRICK_BROKEN 0
//End Defines

typedef struct{
    int xc;
    int yc;
    char vx;
    char vy;
} ball_t;

int bricks[BRICKS_PER_COLUMN][BRICKS_PER_ROW];
ball_t ball;
int bar_x, lives;


static void drawBrick(int x, int y);
static void tryVerticalBounce();
static void drawBall();
static void initBall();
static void drawBar();
static void play();
static void removeBall();
static void printBricks();
static void initGame();
static void removeBar();
static void moveBarLeft();
static void moveBarRight();
static void midPointCircleDraw(int x_centre, int y_centre, int r, int color);
static int gameOver();
static void tryHorizontalBounce();
static void exitGame();
static void moveBall();

void startArkanoid(){
    initGame();

    printBricks();
    drawBall();
    drawBar();
    play();
}

static void play(){
    
    int currentTick, lastTick = -1;
    int speed = 1;
    char c;

    while (! gameOver()){

        c = getChar();
        switch(c){
            case 'a':
            case 'A':
                moveBarLeft();
                break;
            case 'd':
            case 'D':
                moveBarRight();
                break;
            case '\t':
                return exitGame();
        }
        

        currentTick = getTicksElapsed();
        if(currentTick != lastTick && (currentTick % speed) == 0){
            moveBall();
            //while (getChar() != '\n');
            
            lastTick = currentTick;
        }
    }
    
}

static int gameOver(){
    return 0;
}

static void exitGame(){

}

static void moveBarLeft(){
    if(bar_x >= BAR_MOVEMENT){
        removeBar();
        bar_x-= BAR_MOVEMENT;
        drawBar();
    }
}
static void moveBarRight(){
    if(bar_x < SCREEN_WIDTH - BAR_MOVEMENT - BAR_WIDTH){
        removeBar();
        bar_x+= BAR_MOVEMENT;
        drawBar();
    }
}

static void moveBall(){
    removeBall();
    ball.xc += ball.vx;
    ball.yc += ball.vy;
    if(ball.yc < SCREEN_HEIGHT){
        tryHorizontalBounce();
        tryVerticalBounce();
    }else{
        lives--;
        //updateLives();
        initBall();
    }
    drawBall();
}

static void initGame(){
    
    for (int i = 0; i < BRICKS_PER_COLUMN; i++)
        for (int j = 0; j < BRICKS_PER_ROW; j++)
            if(i != 2)
                bricks[i][j] = BRICK_BROKEN;
            else
                bricks[i][j] = BRICK_PRESENT;

    lives = 3;
    bar_x = 500;
    initBall();
}

static void initBall(){
    ball.vx = RADIUS;
    ball.vy = RADIUS;
    ball.xc = RADIUS * 1;
    ball.yc = RADIUS * 3;
}
//Print and Remove
    static void printBricks(){
        for (int i = 0; i < BRICKS_PER_COLUMN; i++){
            for (int j = 0; j < BRICKS_PER_ROW; j++){
                if(bricks[i][j] == BRICK_PRESENT)
                    drawBrick(j*BRICKS_WIDTH, GUI_HEIGHT + i*BRICKS_HEIGHT);
            }
        }
    }

    static void drawBrick(int x, int y){
        for (int i = 0; i < BRICKS_WIDTH; i++){
            for (int j = 0; j < BRICKS_HEIGHT; j++){
                if ( i == 0)
                    drawPixel(x + i,y + j,0x000000);
                else if( i <=4 || j <=4)
                    drawPixel(x + i,y + j, 0x4587f7 + ( y / BRICKS_HEIGHT) * 0xA555BB);
                else if( i  >=60 ||  j >=28)
                    drawPixel(x + i,y + j, 0x1c5cc9 + ( y / BRICKS_HEIGHT) * 0xA555BB);
                else
                    drawPixel(x + i,y + j, 0x2b66cc + ( y / BRICKS_HEIGHT) * 0xA555BB);
            }
        }

    }

    static void removeBrick(int row, int col){
        int x = col * BRICKS_WIDTH;
        int y = row * BRICKS_HEIGHT + GUI_HEIGHT;
        bricks[row][col] = BRICK_BROKEN;
        for (int i = 0; i < BRICKS_WIDTH; i++){
            for (int j = 0; j < BRICKS_HEIGHT; j++){
                drawPixel(x + i,y + j,0x000000);
            }
        }
    }

    static void drawBall(){
        int r = RADIUS;
        while(r > 0){
            midPointCircleDraw(ball.xc,ball.yc,r,BALL_COLOR);
            r--;
        }
    }

    static void removeBall(){
        int r = RADIUS;
        while(r > 0){
            midPointCircleDraw(ball.xc,ball.yc,r,BACKGROUND_COLOR);
            r--;
        }
    }

    static void drawBar(){
        int x = bar_x;
        int y = BAR_Y;
        for (int i = 0; i < BAR_WIDTH; i++){
            for (int j = 0; j < BAR_HEIGHT; j++){
                if(i%BAR_WIDTH <=4 || j%BAR_HEIGHT<=4)
                    drawPixel(x+i,y+j,0x4587f7);
                else if(i%BAR_WIDTH >=128 || j%BAR_HEIGHT>=28)
                    drawPixel(x+i,y+j,0x1c5cc9);
                else
                    drawPixel(x+i,y+j,0x2b66cc);
            }   
        }
    }

    static void removeBar(){
        int x = bar_x;
        int y = BAR_Y;
        for (int i = 0; i < BAR_WIDTH; i++){
            for (int j = 0; j < BAR_HEIGHT; j++){
                drawPixel(x+i,y+j,0x000000);
            }   
        }
    }

    //https://www.geeksforgeeks.org/mid-point-circle-drawing-algorithm/
    static void midPointCircleDraw(int x_centre, int y_centre, int r, int color){ 
        int x = r, y = 0; 
        
        // Printing the initial point on the axes  
        // after translation 
        drawPixel(x + x_centre, y + y_centre, color); 
        
        // When radius is zero only a single 
        // point will be printed 
        if (r > 0) 
        { 
            drawPixel(x + x_centre, -y + y_centre,color); 
            drawPixel(-x + x_centre, y + y_centre,color); 
            drawPixel(y + x_centre, x + y_centre,color); 
            drawPixel(-y + x_centre, -x + y_centre,color); 
        } 
        
        // Initialising the value of P 
        int P = 1 - r; 
        while (x > y) 
        {  
            y++; 
            
            // Mid-point is inside or on the perimeter 
            if (P <= 0) 
                P = P + 2*y + 1; 
                
            // Mid-point is outside the perimeter 
            else
            { 
                x--; 
                P = P + 2*y - 2*x + 1; 
            } 
            
            // All the perimeter points have already been printed 
            if (x < y) 
                break; 
            
            // Printing the generated point and its reflection 
            // in the other octants after translation 
            drawPixel(x + x_centre, y + y_centre,color); 
            drawPixel(-x + x_centre, y + y_centre,color); 
            drawPixel(x + x_centre, -y + y_centre,color); 
            drawPixel(-x + x_centre, -y + y_centre,color); 
            
            // If the generated point is on the line x = y then  
            // the perimeter points have already been printed 
            if (x != y) 
            { 
                drawPixel(y + x_centre, x + y_centre,color); 
                drawPixel(-y + x_centre, x + y_centre,color); 
                drawPixel(y + x_centre, -x + y_centre,color); 
                drawPixel(-y + x_centre, -x + y_centre,color); 
            } 
        }  
    }
//End



static void tryHorizontalBounce(){
    if (ball.xc + RADIUS >= SCREEN_WIDTH ){
        ball.vx *= INVERT;
        ball.xc = SCREEN_WIDTH - RADIUS - 1;
    }else if (ball.xc - RADIUS < 0){
       ball.vx *= INVERT;
       ball.xc = RADIUS;
    }
    
    // else if(ball.yc < BRICKS_HEIGHT * BRICKS_PER_COLUMN + 1 + RADIUS){
    //     if (bricks[(ball.yc - 1) / BRICKS_HEIGHT][(ball.xc + 1) / BRICKS_WIDTH] == BRICK_PRESENT ){
    //         removeBrick((ball.yc - 1) / BRICKS_HEIGHT, (ball.xc + 1) / BRICKS_WIDTH);
    //         ball.vx *= INVERT;
    //     } else if(bricks[(ball.yc - 1) / BRICKS_HEIGHT][(ball.xc - 1) / BRICKS_WIDTH] == BRICK_PRESENT ){
    //         removeBrick((ball.yc - 1) / BRICKS_HEIGHT, (ball.xc - 1) / BRICKS_WIDTH);
    //         ball.vx *= INVERT;
    //     }
    // }
}

static void tryVerticalBounce(){
    //CHOQUE CON TECHO
    if(ball.yc - RADIUS < GUI_HEIGHT){
        ball.vy *= INVERT;   
        ball.yc = BRICKS_HEIGHT + RADIUS;
    }//Choque con ladrillos
    else if( ball.yc - RADIUS < GUI_HEIGHT + BRICKS_HEIGHT * BRICKS_PER_COLUMN ){
        if ((ball.yc + RADIUS <= GUI_HEIGHT + BRICKS_HEIGHT * (BRICKS_PER_COLUMN - 1)) && bricks[(ball.yc + RADIUS - GUI_HEIGHT + 1) / BRICKS_HEIGHT][ball.xc / BRICKS_WIDTH] == BRICK_PRESENT ){
            removeBrick((ball.yc) / BRICKS_HEIGHT, ball.xc / BRICKS_WIDTH);
            ball.vy *= INVERT;
            ball.yc = ( (ball.yc + RADIUS - GUI_HEIGHT + 1) / BRICKS_HEIGHT ) * BRICKS_HEIGHT - 1 - RADIUS + GUI_HEIGHT;
        } else if(bricks[(ball.yc - GUI_HEIGHT - RADIUS - 1) / BRICKS_HEIGHT][ball.xc / BRICKS_WIDTH] == BRICK_PRESENT ){
            removeBrick((ball.yc - GUI_HEIGHT - RADIUS - 1) / BRICKS_HEIGHT, ball.xc / BRICKS_WIDTH);
            ball.vy *= INVERT;
            ball.yc = (((ball.yc - GUI_HEIGHT - RADIUS - 1) / BRICKS_HEIGHT) + 1 ) * BRICKS_HEIGHT + GUI_HEIGHT + RADIUS;
        }
    }else if( (ball.yc + RADIUS >= BAR_Y ) /*&& (ball.xc >= bar_x) && (ball.xc < bar_x + BAR_WIDTH)*/){
        ball.vy *= INVERT;
        ball.yc = BAR_Y - RADIUS - 1;
    }
}