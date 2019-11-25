//keyboardDriver.capsLock
// Driver del teclado, obtencion de los scancodes y devolucion de los caracteres correspondientes

#include <keyboardDriver.h>

#define BUFFER_SIZE 15

//Keyboard scancode to ascii
//English keyboard
//Source: http://www.cs.umd.edu/~hollings/cs412/s98/project/proj1/scancode
//Full scancode table: https://www.shsu.edu/~csc_tjm/fall2005/cs272/scan_codes.html

// Matriz con los codigos ascii asociados a los scancodes de las teclas. La primer entrada 
// es el valor comun, y el segundo es el valor cuando esta el shift activado.
// Si devuelve 0 es una tecla sin ascii asociado.
static char asccode[MAX_SC_TO_ASCII][2] ={
	{   0,0   }, {ESC , ESC}, { '1','!' }, { '2','@' },
    { '3','#' }, { '4','$' }, { '5','%' }, { '6','^' },
	{ '7','&' }, { '8','*' }, { '9','(' }, { '0',')' },
    { '-','_' }, { '=','+' }, {'\b','\b'}, {'\t','\t'},
    { 'q','Q' }, { 'w','W' }, { 'e','E' }, { 'r','R' },
    { 't','T' }, { 'y','Y' }, { 'u','U' }, { 'i','I' },
    { 'o','O' }, { 'p','P' }, { '[','{' }, { ']','}' },
    {'\n','\n'}, {   0,0   }, { 'a','A' }, { 's','S' },
    { 'd','D' }, { 'f','F' }, { 'g','G' }, { 'h','H' },
    { 'j','J' }, { 'k','K' }, { 'l','L' }, { ';',':' },
    {'\'','\"'}, { '`','~' }, {   0,0   }, { '\\','|'},
    { 'z','Z' }, { 'x','X' }, { 'c','C' }, { 'v','V' },
    { 'b','B' }, { 'n','N' }, { 'm','M' }, { ',','<' },
    { '.','>' }, { '/','?' }, {   0,0   }, {   0,0   },
    {   0,0   }, { ' ',' ' }
};

//Obtencion del scancode del registro del teclado
extern unsigned char getKeyboardScancode();

//Static Prototypes
    static int isLetter(int scancode);
    static void processScancode(unsigned char scancode);
    static int scancodeHasAscii(int scancode);
    static char scancodeToAscii(int scancode);
    static void storeInBuffer(char c);
    static char getKeyFromBuffer();
//End Static Prototypes

//Variables
    static int shiftActivated = 0;
    static int capsLock = 0;

    //Buffer con los codigos ascii de las teclas precionadas, forma de cola (FIFO)
    char keyBuffer[BUFFER_SIZE];
    unsigned int bufferCount = 0;
//End Variables


int storeKey(){
    int scancode;
	while(keyboardActivated()){
        scancode = getKeyboardScancode();

        //Se analiza si es una tecla especial (Shift, Ctrl, etc).
        processScancode(scancode);

        //Para las teclas que tienen ascii se lo almacena en el buffer
        if(scancodeHasAscii(scancode)){
            storeInBuffer(scancodeToAscii(scancode));
            return 1;
        }
    }
    return 0; //No habia nada
}

int isShiftActivated(){
    return shiftActivated;
}

int isCapsActivated(){
    return capsLock;
}

char getKey(){
    if(bufferCount <= 0)
        return 0;
    
    return getKeyFromBuffer();
}

static void processScancode(unsigned char scancode){
    if(scancode == LSHFT_DOWN || scancode == RSHFT_DOWN)
            shiftActivated += 1;
        else if(scancode == LSHFT_UP || scancode == RSHFT_UP) 
            shiftActivated -= 1;
        else if(scancode == CAPSLOCK_DOWN)
            capsLock = !capsLock;
}

static void storeInBuffer(char c){
    if(bufferCount < BUFFER_SIZE)
        keyBuffer[bufferCount++] = c;
}

//Funcion auxiliar para el manejo de la cola
static char getKeyFromBuffer(){
    //saco la primer letra del Buffer y lo muevo todo
    char aux = keyBuffer[0];

    for (int i = 1; i < bufferCount; i++)
        keyBuffer[i-1] = keyBuffer[i];

    bufferCount--;

    return aux;
}

static char scancodeToAscii(int scancode){
    if(!scancodeHasAscii(scancode))
        return -1; //No tiene ascii

    if(capsLock && isLetter(scancode))
        return asccode[scancode][capsLock];
    
	return asccode[scancode][shiftActivated > 0];
}

static int scancodeHasAscii(int scancode){
	return scancode >= 0 && scancode < MAX_SC_TO_ASCII && asccode[scancode][0] != 0;
}

static int isLetter(int scancode){
	char ascii = asccode[scancode][0];
	return ascii >= 'a' && ascii <= 'z';
}