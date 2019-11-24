#include <shell.h>
#include <usrlib.h>
#include <arkanoid.h>
#include <music.h>

#define USER_INPUT_SIZE 50
#define MAX_FUNCTIONS 20
#define MAX_ARGUMENTS_SIZE 5
#define ESC 27
#define CURSOR_COLOR 0x00FF00

enum time{HOURS = 4, MINUTES = 2, SECONDS = 0};

//Vars
    typedef struct{
        void (*function)(int argcount, char * args[]);
        char * name;
        char * description;
    }functionPackage;
    
    functionPackage functions[MAX_FUNCTIONS];
    int functionsSize = 0;

    int cursorTick = 0;

//End
//Protoripos
    static int readUserInput(char * buffer, int maxSize);
    static void loadFunction(char * string, void (*fn)(), char * desc);
    static void processInstruction(char * userInput);
    static void loadFunctions();
    static void ticksElpased(int argcount, char * args[]);
    static void printArgs(int argcount, char * args[]);
    static void help(int argcount, char * args[]);
    static void printRegs(int argcount, char * args[]);
    static void printCurrentTime(int argcount, char * args[]);
    static void printTime(enum time id);
    static void printmem(int argcount, char * args[]);
    static void triggerException0(int argcount, char * args[]);
    static void triggerException6(int argcount, char * args[]);
    static void playSound(int argcount, char * args[]);
    static void turnOffCursor();
    static void tickCursor();
    static void arkanoid(int argcount, char * args[]);
    static void playLavander(int argcount, char * args[]);
    static void playForElisa(int argcount, char * args[]);
    static void playDefeat(int argcount, char * args[]);
    static void playVictory(int argcount, char * args[]);
    static void playEvangelion(int argcount, char * args[]);
//End

void startShell(){
    loadFunctions();
    clearScreen();
    setCursorPos(0,getScreenHeight() - 1);
    char userInput[USER_INPUT_SIZE];
    printf("Fleshy: $>", 0x5CFEE4, 0);

    while(readUserInput(userInput,USER_INPUT_SIZE)){
        processInstruction(userInput);
        setCursorPos(0,getScreenHeight() - 1);
        printf("Fleshy: $>", 0x5CFEE4, 0);
    }
}

static int readUserInput(char * buffer, int maxSize){
    
    int counter = 0;
    char c;
    int currentTimerTick;
    int lastTimerTick = -1;
    
    while((counter < maxSize - 1) && (c = getChar()) != '\n' ){

        currentTimerTick = getTicksElapsed();
        if(currentTimerTick != lastTimerTick && currentTimerTick % 10 == 0){
            tickCursor();
            lastTimerTick = currentTimerTick;
        }

        if(c){
            turnOffCursor();

            if(c == ESC)
                return 0;

            if(c == '\t'){ //Tecla para arrancar arkanoid si hay un juego empezado.
                if(gameAlreadyStarted()){
                    startArkanoid(CONTINUE);
                    buffer[0] = 0;
                    counter = 0;
                    return 1;
                }
                c = ' ';
            }

            if( c != '\b'){
                putchar(c);

                buffer[counter++] = c;

            } else if(counter > 0){
                putchar('\b');
                counter--;
            }
        }
    }
    turnOffCursor();
    buffer[counter++] = '\0';
    putchar('\n');
    return 1;
}

static void processInstruction(char * userInput){
    char * arguments[MAX_ARGUMENTS_SIZE];
    int argCount = strtok(userInput,' ', arguments, MAX_ARGUMENTS_SIZE);
    for (int i = 0; i < functionsSize; i++){
        if(strcmp(arguments[0], functions[i].name)){
            functions[i].function(argCount - 1, arguments + 1);
            return;
        }
    }
    if(*userInput != 0){
        print(userInput);
        println(" not found");
    }
}

static void loadFunctions(){
    loadFunction("inforeg",&printRegs, "Prints all the registers \n");
    loadFunction("ticks",&ticksElpased, "Prints ticks elapsed from start.\nArg: -s for seconds elapsed \n");
    loadFunction("printArgs",&printArgs, "Prints all its arguments\n ");
    loadFunction("help",&help, "Prints the description of all functions \n");
    loadFunction("clock",&printCurrentTime, "Prints the current time. Args:\n -h prints current hours. \n -m prints current minutes. \n -s prints current seconds.\n");
    loadFunction("printmem",&printmem, "Makes a 32 Bytes memory dump to screen from the address passed by argument.\nAddress in hexadecimal and 0 is not valid.\n" );
    loadFunction("triggerException0",&triggerException0, "Triggers Exception number 0 \n");
    loadFunction("triggerException6",&triggerException6, "Triggers Exception number 6 \n");
    loadFunction("arkanoid", &arkanoid, "Arkanoid Game! Args:\n No args for new game.\n -c to continue last game.\n");
    loadFunction("beep",&playSound, "Plays a beep \n");
    loadFunction("Lavander",&playLavander, "Plays an indie game's music\n");
    loadFunction("Elisa", &playForElisa, "Music for a student \n");
    loadFunction("Evangelion", &playEvangelion, "Evangelion theme \n"); 
    loadFunction("SadMusic", &playDefeat, "Music to listen when you are sad \n");
    loadFunction("HappyMusic", &playVictory, "Music to listen when you are happy \n");
}

static void loadFunction(char * string, void (*fn)(), char * desc){
    functions[functionsSize].function = fn;
    functions[functionsSize].name = string;
    functions[functionsSize].description = desc;
    functionsSize++;
}


static void ticksElpased(int argcount, char * args[]){
    if(strcmp(args[0],"-s"))
        printint(getTicksElapsed() / 18);
    else
        printint(getTicksElapsed());
    putchar('\n');
}

static void printArgs(int argcount, char * args[]){
    for (int i = 0; i < argcount; i++)
        println(args[i]);
}

static void help(int argcount, char * args[]){

    if(argcount >= 1){
        for (int i = 0; i < functionsSize; i++){
            if(strcmp(functions[i].name, args[0])){
                print("Function ");
                println(functions[i].name);
                println(functions[i].description);
                return;
            }
        }
        print(args[0]);
        println(" is not a command. Here is a list of all commands:");
    }

    for (int i = 0; i < functionsSize; i++){
        print("Function ");
        println(functions[i].name);
        println(functions[i].description);
    }
}

static void printRegs(int argcount, char * args[]){
    static char * regNames[] = {
        "RAX: ", "RBX: ", "RCX: ", "RDX: ", "RBP: ", "RDI: ", "RSI: ",
        "R8: ", "R9: ", "R10: ", "R11: ", "R12: ", "R13: ", "R14: ", "R15: ",
        "IP: ", "RSP: "
    };
	 uint64_t * regs = getRegs();
	char buffer[20];
    for (int i = 0; i < 17; i++){
        uintToBase(regs[i],buffer,16);
	    print(regNames[i]);
	    println(buffer);
    }
}

static void printCurrentTime(int argcount, char * args[]){
    if(argcount == 0){
        printTime(HOURS);
        putchar(':');
        printTime(MINUTES);
        putchar(':');
        printTime(SECONDS);
    }else{
        if(strcmp(args[0],"-h"))
            printTime(HOURS);
        else if(strcmp(args[0],"-m"))
            printTime(MINUTES);
        else if(strcmp(args[0],"-s"))
            printTime(SECONDS);
        else
            print("Wrong argument");
    }
    putchar('\n');
}

static void printTime(enum time id){
    char buffer[3];
    int aux;
    switch (id){
    case SECONDS:
        uintToBase(getTime(SECONDS),buffer,10);
        break;
    case MINUTES:
        uintToBase(getTime(MINUTES),buffer,10);
        break;
    case HOURS:
        aux = getTime(HOURS);
        if(aux < 3)
            aux = 24 - 3 + aux;
        else
            aux -= 3;
        uintToBase(aux,buffer,10);
        break;
    default:
        return;
    }
    print(buffer);
}

static void printmem(int argcount, char * args[]){
    uint64_t num;
    char buffer[50];
    if(argcount < 1 || (num = hexstringToInt(args[0])) == 0 ){
        println("invalid argument");
        return;
    }
    uint8_t * address = (uint8_t *) num;

    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 8; j++){
            uintToBase(*(address + 8*i + j), buffer, 16);
            print(buffer);
            putchar(' ');
        }
        putchar('\n'); 
    }
}

static void triggerException0(int argcount, char * args[]){
    int a = 0;
    int b = 4/a;
    a = b;
    return;
}

static void triggerException6(int argcount, char * args[]){
    void (*function)();
    function = 0;
    function();
}

static void playSound(int argcount, char * args[]){
    sysBeep(1000,5);
}

static void tickCursor(){
    if(cursorTick)
        putchar('\b');
    else
        putcharf(' ', 0, CURSOR_COLOR);
    
    cursorTick = !cursorTick;
}

static void turnOffCursor(){
    if(cursorTick)
        putchar('\b');
    cursorTick = 0;
}

static void arkanoid(int argcount, char * args[]){
    if(argcount == 0)
        startArkanoid(NEW_GAME);
    else if (strcmp(args[0], "-c"))
        startArkanoid(CONTINUE); 
    else 
        println("Wrong Arguments");
}

static void playLavander(int argcount, char * args[]){
    Lavander();
}

static void playVictory(int argcount, char * args[]){
    Victory();
}

static void playForElisa(int argcount, char * args[]){
    forElisa();
}

static void playEvangelion(int argcount, char * args[]){
    Evangelion();
}

static void playDefeat(int argcount, char * args[]){
    Defeat();
}