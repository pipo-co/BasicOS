#include <keyboardDriver.h>

#define BUFFER_SIZE 15

//Keyboard scancode to ascii
//English keyboard
//Source: http://www.cs.umd.edu/~hollings/cs412/s98/project/proj1/scancode
//Full scancode table: https://www.shsu.edu/~csc_tjm/fall2005/cs272/scan_codes.html

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

    char keyBuffer[BUFFER_SIZE];
    unsigned int bufferCount = 0;
//End Variables

int storeKey(){
    int scancode;
	while(keyboardActivated()){
        scancode = getKeyboardScancode();

        processScancode(scancode);

        if(scancodeHasAscii(scancode)){
            storeInBuffer(scancodeToAscii(scancode));
            return 1;
        }
    }
    return 0; //No habia nada
}

int getScancode(){
    if(!keyboardActivated())
        return -1; //No hay nada

    int scancode = getKeyboardScancode();

    processScancode(scancode);

    return scancode;
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