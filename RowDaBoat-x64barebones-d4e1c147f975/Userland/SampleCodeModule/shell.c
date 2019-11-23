#include <shell.h>
#include <usrlib.h>

#define USER_INPUT_SIZE 20
#define MAX_FUNCTIONS 10
#define ESC 27

//Vars
    typedef struct{
        int (*function)();
        char * name;
    }functionPackage;
    functionPackage functions[MAX_FUNCTIONS];
    int functionsSize = 0;

//End
//Protoripos
    static int readUserInput(char * buffer, int maxSize);
    static void loadFunction(char * string, int (*fn)());
    static void processInstruction(char * userInput);
    static void loadFunctions();
    static int inforeg();
    static int ticksElpased();
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
    
    while((counter < maxSize - 1) && (c = getChar()) != '\n' ){
        if(c){

            if(c == ESC)
                return 0;

            if( c != '\b'){
                putchar(c);
                if(c == '\t')
                    c = ' ';
                buffer[counter++] = c;
            } else if(counter > 0){
                putchar('\b');
                counter--;
            }
        }
    }
    buffer[counter++] = '\0';
    putchar('\n');
    return 1;
}

static void processInstruction(char * userInput){
    for (int i = 0; i < functionsSize; i++){
        if(strcmp(userInput, functions[i].name)){
            return functions[i].function();
        }
    }
    print(userInput);
    println(" not found");
}

static void loadFunctions(){
    loadFunction("inforeg",&inforeg);
    loadFunction("ticks",&ticksElpased);
}

static void loadFunction(char * string, int (*fn)()){
    functions[functionsSize].function = fn;
    functions[functionsSize].name = string;
    functionsSize++;
}

//TODO
static int inforeg(){
    println("AINT DON YEET PAL");
    return 0;
}

static int ticksElpased(){
    printint(getTicksElapsed());
    putchar('\n');
    return 0;
}
