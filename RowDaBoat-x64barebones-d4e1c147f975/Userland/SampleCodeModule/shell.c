#include <shell.h>
#include <usrlib.h>

#define USER_INPUT_SIZE 50
#define MAX_FUNCTIONS 10
#define MAX_ARGUMENTS_SIZE 5
#define ESC 27

//Vars
    typedef struct{
        int (*function)(int argcount, char * args[]);
        char * name;
        char * description;
    }functionPackage;
    
    functionPackage functions[MAX_FUNCTIONS];
    int functionsSize = 0;

//End
//Protoripos
    static int readUserInput(char * buffer, int maxSize);
    static void loadFunction(char * string, int (*fn)(), char * desc);
    static int processInstruction(char * userInput);
    static void loadFunctions();
    static int inforeg(int argcount, char * args[]);
    static int ticksElpased(int argcount, char * args[]);
    static int printArgs(int argcount, char * args[]);
    static int help(int argcount, char * args[]);
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

static int processInstruction(char * userInput){
    char * arguments[MAX_ARGUMENTS_SIZE];
    int argCount = strtok(userInput,' ', arguments, MAX_ARGUMENTS_SIZE);
    for (int i = 0; i < functionsSize; i++){
        if(strcmp(arguments[0], functions[i].name)){
            return functions[i].function(argCount - 1, arguments + 1);
        }
    }
    print(userInput);
    println(" not found");
    return 1;
}

static void loadFunctions(){
    loadFunction("inforeg",&inforeg, "Prints the value of all registers \n");
    loadFunction("ticks",&ticksElpased, "Prints ticks elapsed from start.\nArg: -s for seconds elapsed \n");
    loadFunction("printArgs",&printArgs, "Prints all its arguments\n ");
    loadFunction("help",&help, "Prints the description of all functions \n");
}

static void loadFunction(char * string, int (*fn)(), char * desc){
    functions[functionsSize].function = fn;
    functions[functionsSize].name = string;
    functions[functionsSize].description = desc;
    functionsSize++;
}

//TODO
static int inforeg(int argcount, char * args[]){
    println("AINT DON YEET PAL");
    return 0;
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
    for (int i = 0; i < functionsSize; i++){
        print("Function :");
        println(functions[i].name);
        println(functions[i].description);
    }
    
}