#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHARMAX 256
#define SIZE 300000
enum BOOL{FALSE, TRUE};
char lastStr[CHARMAX] = "input.txt";
char argv[CHARMAX][CHARMAX];

typedef struct {
    unsigned char value[SIZE];
    int size;
} data;

typedef struct {
    short listChar[SIZE];
    int size, freq;
} info;

typedef struct NODE {
    enum BOOL isEnd;
    unsigned char symb;
    struct NODE *left, *right;
} node;

int getBit(data x, int id) {
    if (x.size <= id) {
        printf("Sorry. My fault\n");
        return -1;
    }
    return ((x.value[id / 8] >> (id % 8)) & 1);
}

void setBit(data * x, int bit) {
    if (bit) {
        x->value[x->size / 8] |= (1 << (x->size % 8));
    } else {
        x->value[x->size / 8] &= (1 << (x->size % 8)) - 1;
    }
    x->size++;
}


void getNewFileName(char nameOut[CHARMAX], char nameIn[CHARMAX], const char * insStr) {
    int dot = strlen(nameIn) - 1;
    while (nameIn[dot] != '.'){
        dot--;
    }
    strncat(nameOut, nameIn, dot);
    strcat(nameOut, "[");
    strcat(nameOut, insStr);
    strcat(nameOut, "]");
    if (!strcmp(insStr, "compress")) {
        strcat(nameOut, ".ada");
    } else {
        strcat(nameOut, ".txt");
    }
    strcpy(lastStr, nameOut);
}

node * root = NULL;

void deleteRoot(node ** curNode) {
    if ((*curNode)->right) {
        deleteRoot(&(*curNode)->right);
    }
    if ((*curNode)->left) {
        deleteRoot(&(*curNode)->left);
    }
    free(*curNode);
}

void add(data x, unsigned char symb) {
    node * rootPtr = root;
    for (int i = x.size - 1; i >= 0; i--) {
        int bit = getBit(x, i);
        root->isEnd = FALSE;
        if (bit) {
            if (!root->right) {
                root->right = (node *)malloc(sizeof(node));
                root->right->right = NULL;
                root->right->left = NULL;
            }
            root = root->right;
        } else {
            if (!root->left) {
                root->left = (node *)malloc(sizeof(node));
                root->left->right = NULL;
                root->left->left = NULL;
            }
            root = root->left;
        }
        if (!i) {
            root->isEnd = TRUE;
            root->symb = symb;
        }
    }
    root = rootPtr;
}

unsigned char strOut[SIZE];

node * curNodePtr;
void go(int bit, size_t * sizeOut) {
    if (bit) {
        curNodePtr = curNodePtr->right;
    } else {
        curNodePtr = curNodePtr->left;
    }
    if (curNodePtr->isEnd) {
        strOut[*sizeOut] = curNodePtr->symb;
        (*sizeOut)++;
        curNodePtr = root;
    }
}

void decompress() {
    if (!strcmp(argv[1], "\\last")) {
        strcpy(argv[1], lastStr);
    }
    FILE * compressedFile = fopen(argv[1], "rb");
    if (compressedFile == NULL) {
        printf("Error during file opening\n");
        return;
    }

    unsigned char * strIn = malloc(SIZE * sizeof(unsigned char));
    data * codes = malloc(CHARMAX * sizeof(data));

    root = (node *)malloc(sizeof(node));
    root->left = NULL;
    root->right = NULL;
    size_t sizeIn = fread(strIn, 1, SIZE, compressedFile), sizeOut = 0;
    fclose(compressedFile);
    printf("|=====|\n|+");

    int cntDiff = strIn[0], nowId = 1;
    for (int i = 0; i < cntDiff; i++) {
        for (int j = 0; j < strIn[nowId + 1]; j++) {
            setBit(&codes[strIn[nowId]], (strIn[nowId + 2 + j / 8] >> (j % 8)) & 1);
        }
        add(codes[strIn[nowId]], strIn[nowId]);
        nowId += (strIn[nowId + 1] + 7) / 8 + 2;
    }
    printf("+");

    int uslessBits = strIn[nowId];
    curNodePtr = &(*root);
    for (int i = sizeIn * 8 - uslessBits - 1; i >= (nowId + 1) * 8; i--) {
        go((strIn[i / 8] >> (i % 8)) & 1, &sizeOut);
    }
    printf("+");

    for (int i = 0; i * 2 < sizeOut; i++) {
        unsigned char tmp = strOut[i];
        strOut[i] = strOut[sizeOut - i - 1];
        strOut[sizeOut - i - 1] = tmp;
    }
    printf("+");

    char nameOut[CHARMAX] = "\0";
    getNewFileName(nameOut, argv[1], "normal");
    FILE * decompressedFile = fopen(nameOut, "wb");
    fwrite(strOut, 1, sizeOut, decompressedFile);
    fclose(decompressedFile);
    deleteRoot(&root);
    free(strIn);
    free(codes);
    printf("+|\n");

    printf("Successful decompression!\nThe new file is called \"%s\"\n", nameOut);
    return;
}

void compress() {
    if (!strcmp(argv[1], "\\last")) {
        strcpy(argv[1], lastStr);
    }
    FILE * inputFile = fopen(argv[1], "r");
    int cntSymbols[CHARMAX] = {0}, strSize = 0;
    if (inputFile == NULL) {
        printf("Error during file opening :(\n");
        return;
    }

    unsigned char * strFile = malloc(SIZE * sizeof(unsigned char));
    unsigned char * buffer  = malloc(SIZE * sizeof(unsigned char));
    data * listData = malloc(CHARMAX * sizeof(data));
    info * queue = malloc(CHARMAX * sizeof(info));

    while (TRUE) {
        int symb = fgetc(inputFile);
        if (symb == EOF) {
            break;
        }
        strFile[strSize] = symb;
        strSize++;
        cntSymbols[symb]++;
    }
    fclose(inputFile);
    printf("|=====");
    for (int i = 0; i < strSize / 10000; i++) {
        printf("=");
    }
    printf("|\n|");
    printf("+");

    int cntDiff = 0;
    for (int i = 0; i < CHARMAX; i++) {
        listData[i].size = 0;
        if (cntSymbols[i]) {
            queue[cntDiff].freq = cntSymbols[i];
            queue[cntDiff].listChar[0] = i;
            queue[cntDiff].size = 1;
            cntDiff++;
        }
    }
    printf("+");

    for (int cnt = cntDiff; cnt > 1; cnt--) {
        int posMinMin = 0, posMinMax = -1;
        for (int i = 1; i < cnt; i++) {
            if (queue[i].freq <= queue[posMinMin].freq) {
                posMinMax = posMinMin;
                posMinMin = i;
            } else if (posMinMax == -1 || queue[i].freq <= queue[posMinMax].freq) {
                posMinMax = i;
            }
        }
        queue[posMinMin].freq += queue[posMinMax].freq;
        for (int i = 0; i < queue[posMinMin].size; i++) {
            setBit(&listData[queue[posMinMin].listChar[i]], 0);
        }
        for (int i = 0; i < queue[posMinMax].size; i++) {
            setBit(&listData[queue[posMinMax].listChar[i]], 1);
        }
        while (queue[posMinMax].size--) {
            queue[posMinMin].listChar[queue[posMinMin].size] =
                    queue[posMinMax].listChar[queue[posMinMax].size];
            queue[posMinMin].size++;
        }
        queue[posMinMax] = queue[cnt - 1];
    }
    if (cntDiff == 1) {
        setBit(&listData[queue[0].listChar[0]], 0);
    }
    printf("+");

    buffer[0] = (unsigned char)cntDiff;
    int bufferSize = 1, bitLength = 0;
    for (int i = 0; i < CHARMAX; i++) {
        if (i == 7) continue;
        bitLength += cntSymbols[i] * listData[i].size;
        if (listData[i].size == 0) continue;
        buffer[bufferSize++] = (unsigned char)i;
        buffer[bufferSize++] = (unsigned char)listData[i].size;
        for (int sz = 0; sz < listData[i].size; sz += 8) {
            buffer[bufferSize++] = listData[i].value[sz / 8];
        }
    }
    printf("+");

    buffer[bufferSize] = (8 - (unsigned char)(bitLength % 8)) % 8;
    bufferSize++;
    int nowSz = 0;
    for (int i = 0; i < strSize; i++) {
        for (int sz = 0; sz < listData[strFile[i]].size; sz++) {
            int bit = getBit(listData[strFile[i]], sz);
            if (bit) {
                buffer[bufferSize] |= (1 << (nowSz % 8));
            } else {
                buffer[bufferSize] &= (1 << (nowSz % 8)) - 1;
            }
            nowSz++;
            if (nowSz % 8 == 0) {
                bufferSize++;
            }
        }
        if ((i + 1) % 10000 == 0) {
            printf("+");
        }
    }
    if (nowSz % 8) {
        bufferSize++;
    }
    printf("+|\n");

    char nameOut[CHARMAX] = "\0";
    getNewFileName(nameOut, argv[1], "compress");
    FILE * compressedFile = fopen(nameOut, "wb");
    fwrite(buffer, 1, bufferSize, compressedFile);
    fclose(compressedFile);

    free(strFile);
    free(buffer);
    free(listData);
    free(queue);

    printf("Successful compression!\nThe new file is called \"%s\"\n", nameOut);
    return;
}

void parse(char command[CHARMAX], int * argc) {
    for (int i = 0; i < CHARMAX; i++) {
        argv[i][0] = '\0';
    }
    for (int i = 0; command[i]; i++) {
        if (command[i] == ' ' && (i == 0 || command[i - 1] == ' ')) {
            continue;
        }
        if (command[i] == ' ') {
            (*argc)++;
            continue;
        }
        strncat(argv[*argc], command + i, 1);
        if (!command[i + 1]) {
            (*argc)++;
        }
    }
}

void help() {
    printf("There are 4 commands now: \"help\", \"compress [path or \\last]\", \"decompress [path or \\last]\", \"exit\"\n");
}


void myExit() {
    fflush(stdin);
    exit(0);
}

char * namesCommands[CHARMAX];
int cntArgs[CHARMAX], cntCommands;
void (*functions[])() = {help, compress, decompress, myExit};

void init() {
    cntCommands = 5;

    namesCommands[0] = "help";
    namesCommands[1] = "compress";
    namesCommands[2] = "decompress";
    namesCommands[3] = "exit";

    cntArgs[0] = 1;
    cntArgs[1] = 2;
    cntArgs[2] = 2;
    cntArgs[3] = 1;
}

int main() {
    init();
    while (TRUE) {
        printf("[You:] ");
        char command[CHARMAX];
        strset(command, '\0');
        scanf("%[^\n]", command);
        int argc = 0;
        parse(command, &argc);
        for (int i = 0; i < cntCommands + 1; i++) {
            if (i == cntCommands) {
                printf("Incorrect query, please try \"help\"\n");
                break;
            }
            if (argc == cntArgs[i] && !strcmp(argv[0], namesCommands[i])) {
                functions[i]();
                break;
            }
        }
        fflush(stdin);
    }
    return 0;
}