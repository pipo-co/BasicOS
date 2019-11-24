#include <shell.h>
#include <usrlib.h>
#include <arkanoid.h>

#define USER_INPUT_SIZE 50
#define MAX_FUNCTIONS 20
#define MAX_ARGUMENTS_SIZE 5
#define ESC 27
#define CURSOR_COLOR 0x00FF00

enum time{HOURS = 4, MINUTES = 2, SECONDS = 0};
enum chords{A=880, AS= 932, B=988, C=523, CS=554, D=587, DS=622, E=659, F=698, FS=740, G=784, GS=831, S=250000 };

//Vars
    typedef struct{
        int (*function)(int argcount, char * args[]);
        char * name;
        char * description;
    }functionPackage;
    
    functionPackage functions[MAX_FUNCTIONS];
    int functionsSize = 0;

    int cursorTick = 0;

//End
//Protoripos
    static int readUserInput(char * buffer, int maxSize);
    static void loadFunction(char * string, int (*fn)(), char * desc);
    static int processInstruction(char * userInput);
    static void loadFunctions();
    static int ticksElpased(int argcount, char * args[]);
    static int printArgs(int argcount, char * args[]);
    static int help(int argcount, char * args[]);
    static int printRegs(int argcount, char * args[]);
    static int printCurrentTime(int argcount, char * args[]);
    static void printTime(enum time id);
    static int printmem(int argcount, char * args[]);
    static int triggerException0(int argcount, char * args[]);
    static int triggerException6(int argcount, char * args[]);
    static int playSound(int argcount, char * args[]);
    static int Lavander(int argcount, char * args[]);
    static int forElisa(int argcount, char * args[]);
    static int Evangelion(int argcount, char * args[]);
    static int Defeat(int argcount, char * args[]);
    static int Victory(int argcount, char * args[]);
    static void turnOffCursor();
    static void tickCursor();
    static int arkanoid(int argcount, char * args[]);
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

static int processInstruction(char * userInput){
    char * arguments[MAX_ARGUMENTS_SIZE];
    int argCount = strtok(userInput,' ', arguments, MAX_ARGUMENTS_SIZE);
    for (int i = 0; i < functionsSize; i++){
        if(strcmp(arguments[0], functions[i].name)){
            return functions[i].function(argCount - 1, arguments + 1);
        }
    }
    if(*userInput != 0){
        print(userInput);
        println(" not found");
    }
    return 1;
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
    loadFunction("beep",&playSound, "Plays a beep \n");
    loadFunction("Lavander",&Lavander, "Plays an indie game's music\n");
    loadFunction("Elisa", &forElisa, "Music for a student \n");
    loadFunction("arkanoid", &arkanoid, "Arkanoid Game! Args:\n No args for new game.\n -c to continue last game.\n");
    loadFunction("Evangelion", &Evangelion, "Evangelion theme \n"); 
    loadFunction("SadMusic", &Defeat, "Music to listen when you are sad \n");
    loadFunction("HappyMusic", &Victory, "Music to listen when you are happy \n");
}

static void loadFunction(char * string, int (*fn)(), char * desc){
    functions[functionsSize].function = fn;
    functions[functionsSize].name = string;
    functions[functionsSize].description = desc;
    functionsSize++;
}


static int ticksElpased(int argcount, char * args[]){
    if(strcmp(args[0],"-s"))
        printint(getTicksElapsed() / 18);
    else
        printint(getTicksElapsed());
    putchar('\n');
    return 0;
}

static int printArgs(int argcount, char * args[]){
    for (int i = 0; i < argcount; i++)
        println(args[i]);
    
    return 0;
}

static int help(int argcount, char * args[]){

    if(argcount >= 1){
        for (int i = 0; i < functionsSize; i++){
            if(strcmp(functions[i].name, args[0])){
                print("Function ");
                println(functions[i].name);
                println(functions[i].description);
                return 0;
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
    return 0;
}

static int printRegs(int argcount, char * args[]){
    static char * regNames[] = {
        "RAX: ", "RBX: ", "RCX: ", "RDX: ", "RBP: ", "RDI: ", "RSI: ",
        "R8: ", "R9: ", "R10: ", "R11: ", "R12: ", "R13: ", "R14: ", "R15: "
    };
	 uint64_t * regs = getRegs();
	char buffer[20];
    for (int i = 0; i < 15; i++){
        uintToBase(regs[i],buffer,16);
	    print(regNames[i]);
	    println(buffer);
    }
    return 0;
}

static int printCurrentTime(int argcount, char * args[]){
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
    return 0;
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

static int printmem(int argcount, char * args[]){
    uint64_t num;
    char buffer[50];
    if(argcount < 1 || (num = hexstringToInt(args[0])) == 0 ){
        println("invalid argument");
        return 1;
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
    return 0;
}

static int triggerException0(int argcount, char * args[]){
    int a = 0;
    int b = 4/a;
    return b;
}

static int triggerException6(int argcount, char * args[]){
    void (*function)();
    function = 0;
    function();
    return 0;
}

static int playSound(int argcount, char * args[]){
    sysBeep(1000,5);
    return 0;
}
static int Victory(int argcount, char * args[]){
    /*
    sysBeep(D/2,4);
    sysBeep(S,4);
    sysBeep(D/2,4);
    sysBeep(S,4);
    sysBeep(D/2,4);
    sysBeep(S,4);*/

    sysBeep(G/2,8);
    sysBeep(D,8);

    sysBeep(C,4);
    sysBeep(B/2,4);
    sysBeep(A/2,4);
    sysBeep(G,8);
    sysBeep(D,8);

    sysBeep(C,4);
    sysBeep(B/2,4);
    sysBeep(A/2,4);
    sysBeep(G,8);
    sysBeep(D,8);

    sysBeep(C,4);
    sysBeep(B/2,4);
    sysBeep(C,4);
    sysBeep(A/2,8);
    
    /*sysBeep(D/2,4);
    sysBeep(D/2,4);
    

    sysBeep(G/2,32);
    sysBeep(D,32);

    sysBeep(C,8);
    sysBeep(B/2,8);
    sysBeep(A/2,8);
    sysBeep(G,32);
    sysBeep(D,16);

    sysBeep(C,8);
    sysBeep(B/2,8);
    sysBeep(A/2,8);
    sysBeep(G,32);
    sysBeep(D,16);

    sysBeep(C,8);
    sysBeep(B/2,8);
    sysBeep(C,8);
    sysBeep(A/2,32);
    sysBeep(D/2,8);
    sysBeep(D/2,8);
    */
   return 0;
}
static int Defeat(int argcount, char * args[]){
   
    sysBeep(B/2,8);
    sysBeep(A/2,8);
    sysBeep(E/4,24);
    sysBeep(A/2,8);
    sysBeep(C,8);
    sysBeep(D,20);
    sysBeep(A/2,32);
    sysBeep(C,20);
    sysBeep(B/2,8);
    sysBeep(C,8);
    sysBeep(D,8);
    sysBeep(G/2,8);
    sysBeep(E,20);
    sysBeep(D,20);

    sysBeep(B/2,8);
    sysBeep(A/2,8);
    sysBeep(E/4,24);
    sysBeep(A/2,8);
    sysBeep(C,8);
    sysBeep(D,20);
    sysBeep(A/2,32);
    sysBeep(C,20);
    sysBeep(B/2,8);
    sysBeep(C,8);
    sysBeep(D,8);
    sysBeep(G/2,8);
    sysBeep(A/2,32);
    sysBeep(S,16);

    sysBeep(S,8);
    sysBeep(A/2,8);
    sysBeep(B/2,8);
    sysBeep(C,8);
    sysBeep(D,8);
    sysBeep(E,8);
    sysBeep(A/2,8);
    sysBeep(C,16);
    sysBeep(B/2,24);
    sysBeep(A/2,32);
    sysBeep(S,16);

    sysBeep(C,8);
    sysBeep(D,8);
    sysBeep(C,8);
    sysBeep(B/2,8);
    sysBeep(S,6);
    sysBeep(A/2,8);
    sysBeep(E,16);
    sysBeep(S,6);
    sysBeep(E,20);
    sysBeep(D,20);
    sysBeep(S,8);
    sysBeep(D,8);
    sysBeep(E,8);
    sysBeep(A/2,8);
    sysBeep(C,16);
    sysBeep(B/2,8);
    sysBeep(A/2,32);
    sysBeep(C,8);
    sysBeep(D,8);
    sysBeep(C,8);
    sysBeep(B/2,16);
    sysBeep(E/4,16);
    sysBeep(A/2,24);
    sysBeep(F/2,8);
    sysBeep(A/2,8);
    sysBeep(D,16);
    sysBeep(B/2,16);
    sysBeep(G/2,16);
    sysBeep(A/2,32);
    return 0;
    
}


static int forElisa(int argcount, char * args[]){
    sysBeep(E,6);
    sysBeep(DS,6);
    sysBeep(E,6);
    sysBeep(E/2,6);
    sysBeep(E,6);
    sysBeep(DS,6);
    sysBeep(E,6);
    sysBeep(DS,6);
    sysBeep(E,6);
    sysBeep(B/2,6);
    sysBeep(DS,6);
    sysBeep(C,6);
    sysBeep(A/2,12);
    sysBeep(C/2,6);
    sysBeep(E/2,6);
    sysBeep(A/2,6);
    sysBeep(B/2,12);
    sysBeep(E/2,6);
    sysBeep(GS/2,6);
    sysBeep(B/2,6);
    sysBeep(C,12);

    sysBeep(E/2,6);
    sysBeep(E,6);
    sysBeep(DS,6);
    sysBeep(E,6);
    sysBeep(DS,6);
    sysBeep(E,6);
    sysBeep(DS,6);
    sysBeep(E,6);
    sysBeep(B/2,6);
    sysBeep(DS,6);
    sysBeep(C,6);
    sysBeep(A/2,12);
    sysBeep(C/2,6);
    sysBeep(E/2,6);
    sysBeep(A/2,6);
    sysBeep(B/2,12);
    sysBeep(E/2,6);
    sysBeep(C,6);
    sysBeep(B/2,6);
    sysBeep(A/2,12);
    
    return 0;
}

static int Evangelion(int argcount, char * args[]){
    sysBeep(C,10);
    sysBeep(DS,6);
    sysBeep(F,6);
    sysBeep(DS,6);
    sysBeep(F,4);
    sysBeep(F,4);
    sysBeep(F,4);
    sysBeep(AS,6);
    sysBeep(GS,4);
    sysBeep(G,4);
    sysBeep(F,4);
    sysBeep(G,8);
    sysBeep(G,6);
    sysBeep(AS,6);
    sysBeep(B+50,6);
    sysBeep(F,8);
    sysBeep(DS,4);
    sysBeep(AS,4);
    sysBeep(AS,4);
    sysBeep(G,4);
    sysBeep(AS,6);
    sysBeep(AS,6);
    sysBeep(B+50,16);

    return 0;
}

static int Lavander(int argcount, char * args[]){
    sysBeep(C,8);
    sysBeep(G,8);
    sysBeep(B-10,8);
    sysBeep(F-7,8);
    sysBeep(C,8);
    sysBeep(G,8);
    sysBeep(B-10,8);
    sysBeep(F-7,8);
    sysBeep(C,8);
    sysBeep(G,8);
    sysBeep(B-10,8);
    sysBeep(F-7,8);
    sysBeep(C,8);
    sysBeep(G,8);
    sysBeep(B-10,8);
    sysBeep(F-7,8);

    sysBeep(G,8);
    sysBeep(E-30,8);
    sysBeep(G,8);
    sysBeep(E-30,8);

    sysBeep(F,8);
    sysBeep(D,8);
    sysBeep(F,8);
    sysBeep(D,8);

    sysBeep(G,8);
    sysBeep(F,8);
    sysBeep(E,8);
    sysBeep(B,8);
    sysBeep(1140,8);
    sysBeep(E,8);
    sysBeep(C,8);
    sysBeep(E,8);

    sysBeep(G,8);
    sysBeep(E-40,8);
    sysBeep(G,8);
    sysBeep(E-40,8);

    sysBeep(F,8);
    sysBeep(D,8);
    sysBeep(F,8);
    sysBeep(D,8);

    sysBeep(B,8);
    sysBeep(G,8);
    sysBeep(F,8);
    sysBeep(B,8);
    sysBeep(1140,8);
    sysBeep(E,8);
    sysBeep(1140,8);
    sysBeep(E,8);
    
    sysBeep(C,8);
    sysBeep(G,8);
    sysBeep(B-10,8);
    sysBeep(F-7,8);
    sysBeep(C,8);
    sysBeep(G,8);
    sysBeep(B-10,8);
    sysBeep(F-7,8);
    sysBeep(C,8);
    sysBeep(G,8);
    sysBeep(B-10,8);
    sysBeep(F-7,8);
    sysBeep(C,8);
    sysBeep(G,8);
    sysBeep(B-10,8);
    sysBeep(F-7,8);
    return 0;
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

static int arkanoid(int argcount, char * args[]){
    if(argcount == 0)
        startArkanoid(NEW_GAME);
    else if (strcmp(args[0], "-c"))
        startArkanoid(CONTINUE); 
    else 
        println("Wrong Arguments");
    
    
    return 0;
}