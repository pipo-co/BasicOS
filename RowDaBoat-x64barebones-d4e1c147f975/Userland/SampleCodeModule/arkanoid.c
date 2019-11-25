#include <arkanoid.h>
#include <usrlib.h>
#include <music.h>

//Defines
    #define MIN_SCREEN_WIDTH 2*BRICKS_WIDTH
    #define MAX_SCREEN_WIDTH MAX_BRICKS_PER_ROW*BRICKS_WIDTH
    #define MIN_SCREEN_HEIGHT GUI_HEIGHT + 3*BRICKS_HEIGHT + 2*BRICKS_HEIGHT + 8*RADIUS
    
    #define MIN_BRICKS_PER_ROW 2
    #define MAX_BRICKS_PER_ROW 64

    #define BRICKS_PER_COLUMN 3

    #define BRICKS_HEIGHT 32
    #define BRICKS_WIDTH 64

    #define BRICK_PRESENT 1
    #define BRICK_BROKEN 0
    
    #define BACKGROUND_COLOR 0x000000

    #define GUI_HEIGHT 16

    #define BAR_WIDTH 128
    #define BAR_HEIGHT 32

    #define BAR_MOVEMENT 20

    #define RADIUS 12

    #define BALL_COLOR 0xffffff

    #define INVERT -1

    #define INIT_SPEED 5

    #define INITIAL_LIVES 3

    #define ESC 27

    #define TICKS_PER_SEC 18

    #define INIT_MOVES_PER_TURN 2
    #define MAX_MOVES_PER_TURN 4
//End Defines

//Estructura para el modelado de la pelota.
typedef struct{
    int xc;
    int yc;
    char vx;
    char vy;
} ball_t;

//Matriz para llevar el control de que bloque esta y que bloque no
int bricks[BRICKS_PER_COLUMN][MAX_BRICKS_PER_ROW];
ball_t ball;

//Variables dependientes del tamaño de la pantalla.
int screen_height, screen_width, bricks_per_row, initial_bar_x, bar_y;

//Variables del juego en si
int bar_x, lives, bricksLeft, ticksElapsedSinceStart, restartFlag;
int gameStarted = 0;
int movesPerTurn = INIT_MOVES_PER_TURN;

//Prototypes
    //Funcion encargada de inicializar las variables vinculadas al manejo de pantalla.
    static int initScreenInfo();
    
    //Si es una partida nueva, inicializa las variables vinculadas a la partida.
    static void initVariables();

    //Funcion encargada de inicializar y reinciar la posicion de la pelota, 
    // al principio del juego y cada vez que se pierde
    static void initBall();

    //Funcion principal.
    static void play();

    //Funcion que imprime el mensaje de bienvenida y espera hasta que el usuario interactue-
    static int welcomeScreen(enum gameMode mode);

    //Funciones para el dibujo y borrado de los ladrillos.
    static void printBricks();
    static void drawBrick(int x, int y);
    static void removeBrick(int row, int col);

    //Funciones para el dibujo y borrado de la pelota.
    static void drawBall();
    static void removeBall();
    static void midPointCircleDraw(int x_centre, int y_centre, int r, int color);

    //Funciones para el dibujo y borrado de la barra.
    static void drawBar();
    static void removeBar();

    //Funcion para imprimir la interfaz con la informacion de la partida actual.
    static void printGUI();
    
    //Funciones encargadas del movimiento de la pelota y la barra
    static void moveBarLeft();
    static void moveBarRight();
    static void moveBall();
    
    //Funciones encargadas del procesado de los rebotes
    static void tryVerticalBounce();
    static void tryHorizontalBounce();
    
    //Funcion que valida las condiciones de finializacion del juego
    static int gameOver();

    //Funcion que imprime el mensaje de salida y el puntaje de la partida terminada
    static void endGame();
//Prototypes

int gameAlreadyStarted(){
    return gameStarted;
}

void startArkanoid(enum gameMode mode){
    
    if(initScreenInfo())
        return;

    clearScreen();

    restartFlag = 1;

    //Loop para poder jugar multiples partidas sin salir del juego
    while(restartFlag){
        restartFlag = 0;

        if(welcomeScreen(mode)){
            clearScreen();
            return;
        }

        if(mode == NEW_GAME || (mode == CONTINUE && gameStarted == 0))
            initVariables();
        else if(gameOver()){
            endGame();
            clearScreen();
            if(restartFlag)
                initVariables();
            else 
                return;    
        }

        printBricks();
        drawBall();
        drawBar();
        printGUI();
        play();

        mode = NEW_GAME;
        clearScreen();
    }
}

static int welcomeScreen(enum gameMode mode){
    setCursorPos(((horizontalPixelCount() / CHAR_WIDTH / 2) - 10), verticalPixelCount() / CHAR_HEIGHT / 2);
    if(mode == NEW_GAME || gameStarted == 0 )
        printf("Move using A and D", 0x25d2e6, 0x000000);
    setCursorPos(((horizontalPixelCount() / CHAR_WIDTH / 2) - 10), verticalPixelCount() / CHAR_HEIGHT / 2 + 1);
    if(mode == NEW_GAME || gameStarted == 0 )
        printf("Press enter to start!", 0x25d2e6, 0x000000);
    else
        printf("Press enter to continue!", 0x25d2e6, 0x000000);

    char c;
    while ((c = getChar()) != '\n')
        if(c == '\t')
            return 1;

    clearScreen();

    return 0;
}

static int initScreenInfo(){
    screen_height = verticalPixelCount();
    screen_width = horizontalPixelCount();

    if(screen_height < MIN_SCREEN_HEIGHT || screen_width < MIN_SCREEN_WIDTH){
        setCursorPos(0,0);
        print("Screen is too small");
        return 1;
    }

    if(screen_width > MAX_SCREEN_WIDTH)
        screen_width = MAX_SCREEN_WIDTH;

    bricks_per_row = screen_width / BRICKS_WIDTH;
    initial_bar_x = screen_width / 2 - BAR_WIDTH / 2;

    screen_width = bricks_per_row * BRICKS_WIDTH;

    bar_y = screen_height - BRICKS_HEIGHT * 2;

    return 0;
}

static void initVariables(){
    
    bricksLeft = 0;
    for (int i = 0; i < BRICKS_PER_COLUMN; i++)
        for (int j = 0; j < bricks_per_row; j++){
            bricks[i][j] = BRICK_PRESENT;
            bricksLeft++;
        }
    
    gameStarted = 1;
    lives = INITIAL_LIVES;
    bar_x = initial_bar_x;
    ticksElapsedSinceStart = 0;
    initBall();
}

static void play(){
    
    int currentTick, lastTick = -1;
    char c;

    while (! gameOver()){

        //En base a la tecla presionada se mueve la barra o se sale del juego.
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
                clearScreen();
                return;
        } 
        //Se realiza una accion por cada tick distinto
        currentTick = getTicksElapsed();
        if(currentTick != lastTick){
            
            ticksElapsedSinceStart++;
            
            //Cada 15 segundos se aumenta la velocidad del movimiento de pelota
            if(movesPerTurn <= MAX_MOVES_PER_TURN && ticksElapsedSinceStart % (15*TICKS_PER_SEC) == 0){
                sysBeep(A, 5);
                movesPerTurn++;
            }
            //La cantidad de llamado a moveBall es directamente proporcional a la velocidad del juego
            for (int i = 0; i < movesPerTurn; i++)
                moveBall();

            //Actualizado de la informacion para el usuario.
            printGUI();
            
            lastTick = currentTick;
        }
    }
    endGame();   
}

static void endGame(){
    clearScreen();
    setCursorPos(horizontalPixelCount() / CHAR_WIDTH / 2, verticalPixelCount() / CHAR_HEIGHT / 2);
    print("GAME OVER");
    
    if( lives <= 0){
        removeBall();
        setCursorPos(screen_width / CHAR_WIDTH / 2 , screen_height / CHAR_HEIGHT / 2 + 1);
        print("You lost");
        setCursorPos(screen_width / CHAR_WIDTH / 2 - 3, screen_height / CHAR_HEIGHT / 2 + 2);
        print("Bricks left: ");
        printint(bricksLeft);
        setCursorPos(screen_width / CHAR_WIDTH / 2, screen_height / CHAR_HEIGHT / 2 + 3);
        print("Time: ");
        printint(ticksElapsedSinceStart);
        setCursorPos(screen_width / CHAR_WIDTH / 2 - 15, screen_height / CHAR_HEIGHT / 2 + 4);
        Defeat(); //Reproduce la cancion de derrota
    }else{
        setCursorPos(screen_width / CHAR_WIDTH / 2, screen_height / CHAR_HEIGHT / 2 + 1);
        println("You won!");
        setCursorPos(screen_width / CHAR_WIDTH / 2, screen_height / CHAR_HEIGHT / 2 + 2);
        print("Time: ");
        printint(ticksElapsedSinceStart);
        setCursorPos(screen_width / CHAR_WIDTH / 2 - 15, screen_height / CHAR_HEIGHT / 2 + 3);
        Victory(); //Reproduce la cancion de victoria
    }

    //Se le da al usuario la opcion de salor o de seguir juando
    print("Press escape to leave or enter to restart");
    char c;
    while ((c = getChar()) != ESC && c != '\t'){
            if (c == '\n'){
                restartFlag = 1;
                return;
            }
    }
    if(c == ESC)
        gameStarted = 0;
}

static int gameOver(){
    return lives <= 0 || bricksLeft <= 0;
}

static void initBall(){
    ball.vx = INIT_SPEED;
    ball.vy = INIT_SPEED;
    ball.xc = RADIUS + screen_width / 4;
    ball.yc = RADIUS + GUI_HEIGHT + BRICKS_HEIGHT * BRICKS_PER_COLUMN + BRICKS_HEIGHT / 2;
    movesPerTurn = INIT_MOVES_PER_TURN;
}
//Print and Remove
    //Imprime todos los ladrillos que esten presentes
    static void printBricks(){
        for (int i = 0; i < BRICKS_PER_COLUMN; i++){
            for (int j = 0; j < bricks_per_row; j++){
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
        bricksLeft--;
        printGUI();

        for (int i = 0; i < BRICKS_WIDTH; i++){
            for (int j = 0; j < BRICKS_HEIGHT; j++){
                drawPixel(x + i,y + j,0x000000);
            }
        }
    }

    //Dibuja la pelota en su posicion actual.
    static void drawBall(){
        int r = RADIUS;
        while(r > 0){
            midPointCircleDraw(ball.xc,ball.yc,r,BALL_COLOR);
            r--;
        }
    }

    //Borra la pelota.
    static void removeBall(){
        int r = RADIUS;
        while(r > 0){
            midPointCircleDraw(ball.xc,ball.yc,r,BACKGROUND_COLOR);
            r--;
        }
    }

    //Dibuja la barra donde este
    static void drawBar(){
        int x = bar_x;
        int y = bar_y;
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

    //Borra la barra
    static void removeBar(){
        int x = bar_x;
        int y = bar_y;
        for (int i = 0; i < BAR_WIDTH; i++){
            for (int j = 0; j < BAR_HEIGHT; j++){
                drawPixel(x+i,y+j,0x000000);
            }   
        }
    }

    static void printGUI(){
        setCursorPos(0,0);
        print("Lives left: ");
        printint(lives);
        print("     Bricks Left: ");
        printint(bricksLeft);
        print("      TIME: ");
        printint(ticksElapsedSinceStart);
    }

    //Source: https://www.geeksforgeeks.org/mid-point-circle-drawing-algorithm/
    static void midPointCircleDraw(int x_centre, int y_centre, int r, int color){ 
        int x = r, y = 0; 
        
        // Printing the initial point on the axes  
        // after translation 
        drawPixel(x + x_centre, y + y_centre, color); 
        
        // When radius is zero only a single 
        // point will be printed 
        if (r > 0) { 
            drawPixel(x + x_centre, -y + y_centre,color); 
            drawPixel(-x + x_centre, y + y_centre,color); 
            drawPixel(y + x_centre, x + y_centre,color); 
            drawPixel(-y + x_centre, -x + y_centre,color); 
        } 
        
        // Initialising the value of P 
        int P = 1 - r; 
        while (x > y) {  
            y++; 
            
            // Mid-point is inside or on the perimeter 
            if (P <= 0) 
                P = P + 2*y + 1; 
                
            // Mid-point is outside the perimeter 
            else{ 
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
            if (x != y) { 
                drawPixel(y + x_centre, x + y_centre,color); 
                drawPixel(-y + x_centre, x + y_centre,color); 
                drawPixel(y + x_centre, -x + y_centre,color); 
                drawPixel(-y + x_centre, -x + y_centre,color); 
            } 
        }  
    }
//End

//Movement
    static void moveBarLeft(){
        if(bar_x >= BAR_MOVEMENT){
            removeBar();
            bar_x-= BAR_MOVEMENT;
            drawBar();
        }
    }

    static void moveBarRight(){
        if(bar_x < screen_width - BAR_MOVEMENT - BAR_WIDTH){
            removeBar();
            bar_x+= BAR_MOVEMENT;
            drawBar();
        }
    }
    //Mueve la pelota en la direccion que traia y revisa si hubo una colision o si esta fuera 
    // de limites y se debe perder una vida y reiniciar la posicion de la pelota
    static void moveBall(){
        removeBall();
        ball.xc += ball.vx;
        ball.yc += ball.vy;
        if(ball.yc < bar_y){
            tryHorizontalBounce();
            tryVerticalBounce();
        }else{
            sysBeep(1000,5);
            lives--;
            printGUI();
            initBall();
        }
        drawBall();
    }

    //Funcion encargada de las colisiones horizontales.
    static void tryHorizontalBounce(){
        if (ball.xc + RADIUS >= screen_width ){
            ball.vx *= INVERT;
            ball.xc = screen_width - RADIUS - 1;
        }else if (ball.xc - RADIUS < 0){
        ball.vx *= INVERT;
        ball.xc = RADIUS;
        }
        else if(ball.yc < BRICKS_HEIGHT * BRICKS_PER_COLUMN  + GUI_HEIGHT ){
            if (bricks[(ball.yc - GUI_HEIGHT) / BRICKS_HEIGHT][(ball.xc + RADIUS + 1) / BRICKS_WIDTH] == BRICK_PRESENT ){
                removeBrick((ball.yc - GUI_HEIGHT) / BRICKS_HEIGHT,(ball.xc + RADIUS + 1) / BRICKS_WIDTH);
                ball.vx *= INVERT;
                ball.xc = ((ball.xc + RADIUS + 1) / BRICKS_WIDTH) * BRICKS_WIDTH - RADIUS - 1;
            } else if(bricks[(ball.yc - GUI_HEIGHT) / BRICKS_HEIGHT][(ball.xc - RADIUS - 1) / BRICKS_WIDTH] == BRICK_PRESENT ){
                removeBrick((ball.yc - GUI_HEIGHT) / BRICKS_HEIGHT, (ball.xc - RADIUS - 1) / BRICKS_WIDTH);
                ball.vx *= INVERT;
                ball.xc = ((ball.xc - RADIUS - 1) / BRICKS_WIDTH + 1 ) * BRICKS_WIDTH + RADIUS;
            }
        }
    }
    //Funcion encargda de las colisiones verticales. 
    static void tryVerticalBounce(){
        //CHOQUE CON TECHO
        if(ball.yc - RADIUS < GUI_HEIGHT){
            ball.vy *= INVERT;   
            ball.yc = BRICKS_HEIGHT + RADIUS;
        }//Choque con ladrillos
        else if( ball.yc - RADIUS < GUI_HEIGHT + BRICKS_HEIGHT * BRICKS_PER_COLUMN ){
            if ((ball.yc + RADIUS <= GUI_HEIGHT + BRICKS_HEIGHT * (BRICKS_PER_COLUMN - 1)) && bricks[(ball.yc + RADIUS - GUI_HEIGHT + 1) / BRICKS_HEIGHT][ball.xc / BRICKS_WIDTH] == BRICK_PRESENT ){
                removeBrick((ball.yc + RADIUS - GUI_HEIGHT + 1) / BRICKS_HEIGHT, ball.xc / BRICKS_WIDTH);
                ball.vy *= INVERT;
                ball.yc = ( (ball.yc + RADIUS - GUI_HEIGHT + 1) / BRICKS_HEIGHT ) * BRICKS_HEIGHT - 1 - RADIUS + GUI_HEIGHT;
            } else if(bricks[(ball.yc - GUI_HEIGHT - RADIUS - 1) / BRICKS_HEIGHT][ball.xc / BRICKS_WIDTH] == BRICK_PRESENT ){
                removeBrick((ball.yc - GUI_HEIGHT - RADIUS - 1) / BRICKS_HEIGHT, ball.xc / BRICKS_WIDTH);
                ball.vy *= INVERT;
                ball.yc = (((ball.yc - GUI_HEIGHT - RADIUS - 1) / BRICKS_HEIGHT) + 1 ) * BRICKS_HEIGHT + GUI_HEIGHT + RADIUS;
            }
        }//Choque barra
        else if( (ball.yc + RADIUS >= bar_y ) && (ball.xc >= bar_x) && (ball.xc < bar_x + BAR_WIDTH)){
            ball.vy *= INVERT;
            ball.yc = bar_y - RADIUS - 1;
        }
    }
//Movement