//shell.c
#include <shell.h>
#include <usrlib.h>
#include <arkanoid.h>
#include <music.h>

//constantes para la definicion de arrays
#define USER_INPUT_SIZE 50
#define MAX_FUNCTIONS 100
#define MAX_ARGUMENTS_SIZE 5

#define END_OF_EXECUTION_KEY 27
#define GAME_RETURNING_KEY '\t'
#define CURSOR_COLOR 0x00FF00

//Vars
    //Estructura para el guardado de los modulos. Puntero a la funcion pertinente,
    // nombre con el cual se la llama y una breve descripcion de su funcionamiento. 
    typedef struct{
        void (*function)(int argcount, char * args[]);
        char * name;
        char * description;
    }functionPackage;
    
    functionPackage functions[MAX_FUNCTIONS];
    //Cantidad de funciones disponibles
    int functionsSize = 0;

    //int cursorTick = 0;

//End
//Protoripos
    //Funciones para el cargado del fuctionArray. Carga todos los modulos disponibles.
    static void loadFunctions();
    static void loadFunction(char * string, void (*fn)(), char * desc);

    //Funciones utilizadas para la operacion de la shell.
    static int readUserInput(char * buffer, int maxSize);
    static void processInstruction(char * userInput);

    //Funciones auxiliares para tener un cursor parpadeante. 
    //static void tickCursor();
    //static void turnOffCursor();
    
    //Modulos - Funciones ejecutables desde la shell
    //inforeg: imprimir el valor de todos los registros
    extern void getRegs(int argcount, char * args[]);
    void printRegs(uint64_t * regs); //Llamada por getRegs en assembler.
    
    //printmem: imprime en pantalla el valor hexadecimal de los primeros 32 bytes 
    // a partir de la direccion recibida como argumento
    static void printmem(int argcount, char * args[]);

    //clock: imprime la hora actual en horario GMT-3
    static void printCurrentTime(int argcount, char * args[]);
    static void printTime(enum time id);

    //help: funcion que imprime las funciones disponibles y su respectiva descripcion
    static void help(int argcount, char * args[]);

    //triggerException: funciones demostrativas de las rutinas de atencion de las excepciones 
    // 0 y 6 (division por cero y opcode invalido, respectivamente).
    static void triggerException0(int argcount, char * args[]);
    static void triggerException6(int argcount, char * args[]);

    //arkanoid: juego arkanoid. Partida nueva o continuada.
    static void arkanoid(int argcount, char * args[]);

    //Modulos adicionales
    
    //ticksElpased: funcion demostrativa de la syscall 0.
    //Imprime los ticks actuales.
    static void ticksElpased(int argcount, char * args[]);

    //printArgs: funcion demostrativa del parseado de argumentos. 
    // Imprime todos los argumentos que recibe.
    static void printArgs(int argcount, char * args[]);

    //beep: emite un sonido breve.
    static void playSound(int argcount, char * args[]);

    //Scheduler Commands
    static void cmdBlock(int argcount, char * args[]);
    static void cmdUnblock(int argcount, char * args[]);
    static void cmdKill(int argcount, char * args[]);
    static void cmdGetPID();
    static void cmdChangeProcessPriority(int argcount, char * args[]);

    //Semaphore Commands
    static void cmdCreateSem(int argcount, char * args[]);
    static void cmdRemoveSem(int argcount, char * args[]);
    static void cmdSemWait(int argcount, char * args[]);
    static void cmdSemPost(int argcount, char * args[]);

    //Pipe Commands
    static void cmdOpenPipe(int argcount, char * args[]);
    static void cmdWritePipe(int argcount, char * args[]);
    static void cmdReadPipe(int argcount, char * args[]);
    static void cmdClosePipe(int argcount, char * args[]);

    //Test Agodios
    extern void test_mm();
//End

static void semTester();

void startShell(){
    //Se cargan los modulos
    loadFunctions();
    //clearScreen();
    setCursorPos(0,getScreenHeight() - 1);
    char userInput[USER_INPUT_SIZE];
    /*  comentado help
    printf("Bienvenido a shell, estas son las funciones disponibles: \n", 0x5CFEE4, 0);
    help(0,0); */
    printf("Fleshy: $>", 0x5CFEE4, 0);

    //Se espera hasta que se reciba un enter y luego, se procesa el texto ingresado.
    //Si coincide con el nombre de una funcion se la ejecuta, sino se vuelve a modo lectura.
    while(readUserInput(userInput, USER_INPUT_SIZE)){
        processInstruction(userInput);
        setCursorPos(0,getScreenHeight() - 1);
        printf("Fleshy: $>", 0x5CFEE4, 0);
    }
}

//Funcion encargada de la lectura del texto ingresado por el usuario.
//Se encarga de guardarlo en un buffer para luego ser procesado. Maneja borrado, 
// tecla especial para volver al juego y tecla especial para el corte de ejecucion.
static int readUserInput(char * buffer, int maxSize){
    
    int counter = 0;
    char c;
    
    //int currentTimerTick;
    //int lastTimerTick = -1;
    
    while((counter < maxSize - 1) && (c = getChar()) != '\n' ){

        /*//Parpadeo del cursor.
        currentTimerTick = getTicksElapsed();
        if(currentTimerTick != lastTimerTick && currentTimerTick % 10 == 0){
            tickCursor();
            lastTimerTick = currentTimerTick;
        }*/

        //Procesado de la tecla presionada
        if(c){
            //turnOffCursor();

            if(c == END_OF_EXECUTION_KEY)
                return 0;

            if(c == GAME_RETURNING_KEY){ //Tecla para arrancar arkanoid si hay un juego empezado.
                if(gameAlreadyStarted()){
                    startArkanoid(CONTINUE);
                    buffer[0] = 0;
                    counter = 0;
                    return 1;
                }
            }

            if( c == '\t')
                c = ' ';

            if( c != '\b'){
                putchar(c);

                buffer[counter++] = c;

            } else if(counter > 0){
                putchar('\b');
                counter--;
            }
        }
    }
    //turnOffCursor();
    buffer[counter++] = '\0';
    putchar('\n');
    return 1;
}

//Funcion encargada de procesar el texto recibido. Se guardan los argumentos en un array 
// y se verifica si el texto ingresado valida con el nombre de una funcion para asi llamarla.
static void processInstruction(char * userInput){
    int background = 0;

    char * arguments[MAX_ARGUMENTS_SIZE];
    int argCount = strtok(userInput, ' ', arguments, MAX_ARGUMENTS_SIZE);

    if(strcmp(arguments[argCount - 1], "&")){
        background = 1;
        argCount--;
    }

    for (int i = 0; i < functionsSize; i++){
        if(strcmp(arguments[0], functions[i].name)){
            if(background)
                initializeProccess((int (*)(int,char**))functions[i].function, 0, argCount, arguments, 0);
            else
                functions[i].function(argCount - 1, arguments + 1);
            return;
        }
    }
    if(*userInput != 0){
        print(userInput);
        println(" not found");
    }
}

//Cargado de los modulos
static void loadFunctions(){
    loadFunction("inforeg", getRegs, "Prints all the registers \n");
    loadFunction("ticks", ticksElpased, "Prints ticks elapsed from start. Arg: -s for seconds elapsed \n");
    loadFunction("printArgs", printArgs, "Prints all its arguments\n ");
    loadFunction("help", help, "Prints the description of all functions \n");
    loadFunction("clock", printCurrentTime, "Prints the current time. Args: -h prints current hours.  -m prints current minutes.  -s prints current seconds.\n");
    loadFunction("printmem", printmem, "Makes a 32 Bytes memory dump to screen from the address passed by argument.\nAddress in hexadecimal and 0 is not valid.\n" );
    loadFunction("triggerException0", triggerException0, "Triggers Exception number 0 \n");
    loadFunction("triggerException6", triggerException6, "Triggers Exception number 6 \n");
    loadFunction("arkanoid", arkanoid, "Arkanoid Game! Args: No args for new game. -c to continue last game.\n");
    loadFunction("beep", playSound, "Plays a beep \n");
    loadFunction("dumpMM", (void (*)(int, char**))dumpMM, "Memory Manager Dump \n");
    loadFunction("dumpScheduler", (void (*)(int, char**))dumpScheduler,"Scheduler Dump \n");
    loadFunction("block", cmdBlock,"Block process given it's PID \n");
    loadFunction("unblock", cmdUnblock,"Unblock process given it's PID \n");
    loadFunction("kill", cmdKill,"Kill process given it's PID \n");
    loadFunction("getpid", (void (*)(int, char**))cmdGetPID,"Return running process PID \n");
    loadFunction("changePriority", cmdChangeProcessPriority,"Change process priority given it's PID \n");
    loadFunction("Lavander", (void (*)(int, char**))Lavander, "Plays an indie game's music");
    loadFunction("openSem", cmdCreateSem, "Create new Semaphore or Open an existing one \n");
    loadFunction("closeSem", cmdRemoveSem, "Close an existing semaphore \n");
    loadFunction("semWait", cmdSemWait, "Sem Wait \n");
    loadFunction("semPost", cmdSemPost, "Sem Post \n");
    loadFunction("dumpSem", (void (*)(int, char**))dumpSem, "Semaphores Dump \n");
    loadFunction("testMM", (void (*)(int, char**))test_mm, "Test MM \n");
    loadFunction("openPipe", cmdOpenPipe, "Create new Pipe or open an existing one \n");
    loadFunction("writePipe", cmdWritePipe, "Write String to pipe \n");
    loadFunction("readPipe", cmdReadPipe, "Read Char from Pipe \n");
    loadFunction("closePipe", cmdClosePipe, "Close Existing pipe \n");
    loadFunction("dumpPipes", (void (*)(int, char**))dumpPipes, "Pipes Dump \n");
    loadFunction("semtest", (void (*)(int, char**))semTester, "Sem Test \n");
    // loadFunction("Elisa", (void (*)(int, char**))forElisa, "Music for a student\n");semTester
    // loadFunction("Evangelion", (void (*)(int, char**))Evangelion, "Evangelion theme\n"); 
    // loadFunction("SadMusic", (void (*)(int, char**))Sadness, "Music to listen when you are sad");
    // loadFunction("Victory", (void (*)(int, char**))Victory, "Music to listen when you win");
    // loadFunction("Defeat", (void (*)(int, char**))Defeat, "Music to listen when you are happyn't");
}

static void loadFunction(char * string, void (*fn)(int, char**), char * desc){
    functions[functionsSize].function = fn;
    functions[functionsSize].name = string;
    functions[functionsSize].description = desc;
    functionsSize++;
}

// static void tickCursor(){
//     if(cursorTick)
//         putchar('\b');
//     else
//         putcharf(' ', 0, CURSOR_COLOR);
    
//     cursorTick = !cursorTick;
// }

// static void turnOffCursor(){
//     if(cursorTick)
//         putchar('\b');
//     cursorTick = 0;
// }

//Modulos
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

void printRegs(uint64_t * regs){
     static char * regNames[] = {
        "RAX: ", "RBX: ", "RCX: ", "RDX: ", "RBP: ", "RDI: ", "RSI: ",
        "R8: ", "R9: ", "R10: ", "R11: ", "R12: ", "R13: ", "R14: ", "R15: ",
        "IP: ", "RSP: "
    };

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
    __asm__("ud2"); //https://mudongliang.github.io/x86/html/file_module_x86_id_318.html
}

static void playSound(int argcount, char * args[]){
    sysBeep(A,5);
}

static void arkanoid(int argcount, char * args[]){
    if(argcount == 0)
        startArkanoid(NEW_GAME);
    else if (strcmp(args[0], "-c"))
        startArkanoid(CONTINUE); 
    else 
        println("Wrong Arguments");
}

static void cmdBlock(int argcount, char * args[]){
    if(argcount < 1){
        println("Need process PID");
        return;
    }
    block(atoi(args[0]));
}

static void cmdUnblock(int argcount, char * args[]){
    if(argcount < 1){
        println("Need process PID");
        return;
    }
    unblock(atoi(args[0]));
}

static void cmdKill(int argcount, char * args[]){
    if(argcount < 1){
        println("Need process PID");
        return;
    }
    kill(atoi(args[0]));
}

static void cmdGetPID(){
    printint(getPID());
    putchar('\n');
}

static void cmdChangeProcessPriority(int argcount, char * args[]){
    if(argcount < 2){
        println("Need process PID and new Priority");
        return;
    }
    changeProccessPriority(atoi(args[0]), atoi(args[1]));
}

static void cmdCreateSem(int argcount, char * args[]){
    if(argcount < 2){
        println("Need semaphore name and initial value");
        return;
    }
    
    int32_t aux = createSem(args[0], atoi(args[1]));
    if(aux == -1)
        println("Error creating sem");
    else {
        print("Sem code: ");
        printint(aux);
        putchar('\n');
    }    
}

static void cmdSemWait(int argcount, char * args[]){
    if(argcount < 1){
        println("Need a semaphore valid code");
        return;
    }
    
    int aux = semWait(atoi(args[0]));
    if(aux == -1)
        println("Error in semWait");
}

static void cmdSemPost(int argcount, char * args[]){
    if(argcount < 1){
        println("Need a semaphore valid code");
        return;
    }
    
    int aux = semPost(atoi(args[0]));
    if(aux == -1)
        println("Error in semPost");
}

static void cmdRemoveSem(int argcount, char * args[]){
    if(argcount < 1){
        println("Need a semaphore valid code");
        return;
    }
    removeSem(atoi(args[0]));
}

static void cmdOpenPipe(int argcount, char * args[]){
    if(argcount < 1){
        println("Need a pipe name");
        return;
    }
    int aux = openPipe(args[0]);
    if(aux == -1)
        println("Error creating pipe");
    else {
        print("Pipe code: ");
        printint(aux);
        putchar('\n');
    }
}

static void cmdWritePipe(int argcount, char * args[]){
    if(argcount < 2){
        println("Need a pipe valid code and string to write");
        return;
    }
    
    int aux = writePipe(atoi(args[0]), args[1]);
    if(aux == -1)
        println("Error in writePipe");
}

static void cmdReadPipe(int argcount, char * args[]){
    if(argcount < 1){
        println("Need a pipe valid code");
        return;
    }
    
    char c = readPipe(atoi(args[0]));
    if(c == 0)
        println("Error in readPipe");
    putchar(c);
    putchar('\n');
}

static void cmdClosePipe(int argcount, char * args[]){
    if(argcount < 1){
        println("Need a pipe valid code");
        return;
    }
    closePipe(atoi(args[0]));
}


static void semTester(){
    uint16_t sem = createSem("nachocapo", 0);
    printint(sem); putchar('\n');
    println("Hola, un post y me bloqueo");
    if(semWait(sem) == -1)
        println("Error!");
    println("Hola, me desbloquearon, otro fav(post) y me bloqueo!");
    removeSem(sem);
}