//shell.c
#include <stddef.h>
#include <shell.h>
#include <usrlib.h>
#include <arkanoid.h>
#include <music.h>

//constantes para la definicion de arrays
#define USER_INPUT_SIZE 100
#define MAX_FUNCTIONS 100
#define MAX_ARGUMENTS_SIZE 20
#define MAX_CONCAT_PIPES 10

#define EOF 27  // ESC
#define GAME_RETURNING_KEY '\t'
#define CURSOR_COLOR 0x00FF00

typedef void (*shellFunction)(int, char**);

//Vars
    //Estructura para el guardado de los modulos. Puntero a la funcion pertinente,
    // nombre con el cual se la llama y una breve descripcion de su funcionamiento. 
    typedef struct{
        shellFunction function;
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
    static void loadFunction(char * string, shellFunction fn, char * desc);
    static shellFunction getFunction(char * functionName);

    //Funciones utilizadas para la operacion de la shell.
    static int readUserInput(char * buffer, int maxSize);
    static void processInstruction(char * userInput);

    //Funciones auxiliares funcionamiento pipes
    static int getPipesFunctions(char * arguments[], uint16_t pipeLocation[], uint16_t pipeCount, shellFunction fArr[]);
    static void freePipesResources(uint64_t childPid[], uint16_t pipesId[], uint16_t pipeCount, int isFg);
    static int getPipeLocations(char * arguments[], int argc, uint16_t pipeLocation[]);
    static void processPipe(char * arguments[], int argc, uint16_t pipeLocation[], uint16_t pipeCount, int isFg);

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
    static void loop(int argc, char ** argv);

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

    //IPC functions
    static void cat(int argc, char ** argv);
    static void wc();
    static void filter();

    //Test Agodios
    extern void test_mm();
    extern void test_processes();

    // Pruebas
    static void prueba(int argc, char ** argv);
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

            if(c == EOF)
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
    uint16_t pipeLocation[MAX_CONCAT_PIPES];

    int argCount = strtok(userInput, ' ', arguments, MAX_ARGUMENTS_SIZE);

    if(strcmp(arguments[argCount - 1], "&")){
        background = 1;
        argCount--;
    }

    int pipeCount = getPipeLocations(arguments, argCount, pipeLocation);

    if(pipeCount == -1){
        println("Incorrect use of pipes");
        return;

    } else if(pipeCount > 0){
        processPipe(arguments, argCount, pipeLocation, pipeCount, !background);
        return;
        
    } else {
        shellFunction function = getFunction(arguments[0]);

        if(function != NULL){
            if(initializeProccess((int (*)(int,char**))function, !background, argCount, arguments, 0) == 0)
                println("There was a problem creating the new process");
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
    loadFunction("dumpMM", (shellFunction)dumpMM, "Memory Manager Dump \n");
    loadFunction("dumpScheduler", (shellFunction)dumpScheduler,"Scheduler Dump \n");
    loadFunction("block", cmdBlock,"Block process given it's PID \n");
    loadFunction("unblock", cmdUnblock,"Unblock process given it's PID \n");
    loadFunction("kill", cmdKill,"Kill process given it's PID \n");
    loadFunction("getpid", (shellFunction)cmdGetPID,"Return running process PID \n");
    loadFunction("changePriority", cmdChangeProcessPriority,"Change process priority given it's PID \n");
    loadFunction("Lavander", (shellFunction)Lavander, "Plays an indie game's music");
    loadFunction("openSem", cmdCreateSem, "Create new Semaphore or Open an existing one \n");
    loadFunction("closeSem", cmdRemoveSem, "Close an existing semaphore \n");
    loadFunction("semWait", cmdSemWait, "Sem Wait \n");
    loadFunction("semPost", cmdSemPost, "Sem Post \n");
    loadFunction("dumpSem", (shellFunction)dumpSem, "Semaphores Dump \n");
    loadFunction("openPipe", cmdOpenPipe, "Create new Pipe or open an existing one \n");
    loadFunction("writePipe", cmdWritePipe, "Write String to pipe \n");
    loadFunction("readPipe", cmdReadPipe, "Read Char from Pipe \n");
    loadFunction("closePipe", cmdClosePipe, "Close Existing pipe \n");
    loadFunction("dumpPipes", (shellFunction)dumpPipes, "Pipes Dump \n");
    loadFunction("loop", loop, "Prints PID every certain amount of ticks \n");
    loadFunction("cat", cat, "Prints stdIn as it comes. You can configure the endkey, ESC by default \n");
    loadFunction("wc", (shellFunction)wc, "Prints how many lines had it's stdIn \n");
    loadFunction("filter", (shellFunction)filter, "Prints stdIn whithout vowels \n");
    loadFunction("testMM", (shellFunction)test_mm, "Test MM \n");
    loadFunction("testScheduler", (shellFunction)test_processes, "Test Scheduler \n");
    loadFunction("semtest", (shellFunction)semTester, "Sem Test \n");
    loadFunction("prueba", prueba, "Sem Test \n");
    loadFunction("shell", (shellFunction)startShell, "Sem Test \n");
    // loadFunction("Elisa", (shellFunction)forElisa, "Music for a student\n");semTester
    // loadFunction("Evangelion", (shellFunction)Evangelion, "Evangelion theme\n"); 
    // loadFunction("SadMusic", (shellFunction)Sadness, "Music to listen when you are sad");
    // loadFunction("Victory", (shellFunction)Victory, "Music to listen when you win");
    // loadFunction("Defeat", (shellFunction)Defeat, "Music to listen when you are happyn't");
}

static void loadFunction(char * string, shellFunction fn, char * desc){
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

static int getPipeLocations(char * arguments[], int argc, uint16_t pipeLocation[]){
    static char * pipe = "|";

    uint16_t count = 0;

    if(strcmp(arguments[0], pipe) || strcmp(arguments[argc - 1], pipe)) //No se puede empezar o terminar con pipe
        return -1;

    for(uint16_t i = 1; i < argc; i++){
        if(strcmp(arguments[i], pipe)){

            if(strcmp(arguments[i+1], pipe)) //No puede haber dos pipes seguidos
                return -1;
                
            pipeLocation[count++] = i;
        }
    }
    return count;
}

static shellFunction getFunction(char * functionName){
    for (int i = 0; i < functionsSize; i++) {
        if(strcmp(functionName, functions[i].name)){
            return functions[i].function;
        }
    }
    return NULL;
}

static void processPipe(char * arguments[], int argc, uint16_t pipeLocation[], uint16_t pipeCount, int isFg){
    shellFunction functionsArray[MAX_CONCAT_PIPES + 1];
    uint64_t childPid[MAX_CONCAT_PIPES + 1];
    uint16_t pipesId[MAX_CONCAT_PIPES];

    // Inicializa functionsArray con los respectivos punteros a funcion de cada funcion de la shell, en orden.
    // De haber algun nombre invalido, falla.
    if(getPipesFunctions(arguments, pipeLocation, pipeCount, functionsArray) == -1){
        println("One of the functions called between pipes is invalid");
        return;
    }

    static char defaultPipeName[] = "_shellPipe";
    char name[15]; // Buffer auxiliar para armar el nombre de los pipes
    uint16_t stdFd[2]; // Array donde guardamos los fd de los nuevos procesos. IN = 0; OUT = 1;

    int auxArgc;
    char ** auxArgv;

    // Inicializamos los procesos de derecha a izquierda

    // Armamos el nombre del ultimo pipe (id_defaultPipeName)
    uintToBase(pipeCount - 1, name, 10);
    strcat(name, defaultPipeName);

    // En pipesId me guardo los id de todos los pipes abiertos, para luego poder cerrarlos
    // El ultimo proceso escribe a pantalla y recibe del pipe a su izquierda, el cual creamos
    pipesId[pipeCount - 1] = openPipe(name);
    stdFd[0] = pipesId[pipeCount - 1];
    stdFd[1] = 0;
       
    for(uint16_t i = pipeCount - 1; i > 0; i--){

        // La cantidad de argumentos es el indice del pipe a la derecha, menos el que esta a la izquierda menos 1
        // Incluye el nombre de la funcion. Si es el ultimo, el pipe de la derecha vendria a ser equivalente a argc
        auxArgc = ((i < pipeCount - 1)? pipeLocation[i+1] : argc) - pipeLocation[i] - 1;

        // argv es el indice donde esta el nombre de la funcion, es decir, a la derecha del pipe.
        auxArgv = arguments + pipeLocation[i] + 1;

        // Armamos el nombre del pipe
        uintToBase(i - 1, name, 10);
        strcat(name, defaultPipeName);

        // Inicializamos el proceso a la derecha del pipe, por eso la funcion es i + 1.
        childPid[i + 1] = initializeProccess((int (*)(int,char**))functionsArray[i + 1], 0, auxArgc, auxArgv, stdFd);

        // Configuramos los fd del siguiente proceso a inicializar.
        // Su salida es la entrada del proceso creado anteriormente.
        // Su entrada es el pipe a su izquierda que hay que crear.
        pipesId[i - 1] = openPipe(name);
        stdFd[1] = stdFd[0];
        stdFd[0] = pipesId[i - 1];
    }

    // Para no abrir pipes demas, el segundo proceso debe ser creado a mano.
    // Es el correspondiente a i = 0.
    auxArgc = ((1 < pipeCount)? pipeLocation[1] : argc) - pipeLocation[0] - 1;
    auxArgv = arguments + pipeLocation[0] + 1;
    childPid[1] = initializeProccess((int (*)(int,char**))functionsArray[1], 0, auxArgc, auxArgv, stdFd);

    // El primer proceso es un caso particular pues no tiene pipe a su izquierda.
    // Su entrada, si es que usa, debera ser el teclado.
    // Este es el unico proceso que podria ser fg. Esto fue indicado por parametro.
    stdFd[1] = stdFd[0];
    stdFd[0] = 0;
    childPid[0] = initializeProccess((int (*)(int,char**))functionsArray[0], isFg, pipeLocation[0], arguments, stdFd);

    // Libera los pipes creados, de izquierda a derecha.
    // Para eso, se asegura mediante un wait que los hijos hayan terminado de usar el recurso.
    freePipesResources(childPid, pipesId, pipeCount, isFg);
}

static int getPipesFunctions(char * arguments[], uint16_t pipeLocation[], uint16_t pipeCount, shellFunction fArr[]){

    if((fArr[0] = getFunction(arguments[0])) == NULL)
        return -1;

    for(uint16_t p = 0; p < pipeCount; p++){
        if((fArr[p + 1] = getFunction(arguments[pipeLocation[p] + 1])) == NULL)
            return -1;
    }
    return 0;
}

static void freePipesResources(uint64_t childPid[], uint16_t pipesId[], uint16_t pipeCount, int isFg){

    // Si es foreground, el wait ya lo hubiese hecho automaticamente
    if(!isFg)
        waitChild(childPid[0]);
    
    for(uint16_t i = 0; i < pipeCount; i++){

        // Se le informa a los procesos del EOF mediante la convencion que establecio shell.
        // Se asume que los procesos siguen esta convencion.
        writePipe(pipesId[i], EOF);

        waitChild(childPid[i + 1]);

        closePipe(pipesId[i]);
    }
}


//Modulos
static void ticksElpased(int argcount, char * args[]){
    if(argcount > 0 && strcmp(args[0],"-s"))
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
    
    int aux = writeStringPipe(atoi(args[0]), args[1]);
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

static void prueba(int argc, char ** argv){
    char c;
    while((c = getChar()) != EOF)
        putchar(c);

    print(argv[0]);
    putchar('\n');
}

static void loop(int argc, char ** argv){
    uint16_t ticks;

    if(argc > 0)
        ticks = atoi(argv[0]);
    else
        ticks = 32;

    while(1){
        sleep(ticks);
        printint(getPID());
        putchar('\n');
    }
}

static void cat(int argc, char ** argv){
    char endChar;
    if(argc > 0)
        endChar = argv[0][0];
    else
        endChar = EOF;

    char c;
    while((c = getChar()) != endChar)
        putchar(c);
}

static void wc(){
    uint16_t counter = 1;
    char c;

    while((c = getChar()) != EOF){
        if(c == '\n')
            counter++;
    }

    printint(counter);
    putchar('\n');
}

static void filter(){

    char c;
    while((c = getChar()) != EOF){
        if(!isVowel(c))
            putchar(c);
    }
}