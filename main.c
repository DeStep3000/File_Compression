#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TREE_HT 256

typedef struct MinHeapNode {
    char data;
    unsigned freq;
    struct MinHeapNode *left, *right;
} MinHeapNode;

typedef struct MinHeap {
    unsigned size;
    unsigned capacity;
    MinHeapNode **array;
} MinHeap;

typedef struct HuffmanNode {
    char data;
    unsigned freq;
    struct HuffmanNode *left, *right;
} HuffmanNode;

typedef struct HuffmanTree {
    HuffmanNode *root;
} HuffmanTree;

MinHeapNode *newMinHeapNode(char data, unsigned freq) {
    MinHeapNode *node = (MinHeapNode *) malloc(sizeof(MinHeapNode));
    node->left = node->right = NULL;
    node->data = data;
    node->freq = freq;
    return node;
}

MinHeap *createMinHeap(unsigned capacity) {
    MinHeap *minHeap = (MinHeap *) malloc(sizeof(MinHeap));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (MinHeapNode **) malloc(minHeap->capacity * sizeof(MinHeapNode *));
    return minHeap;
}

void swapMinHeapNode(MinHeapNode **a, MinHeapNode **b) {
    MinHeapNode *t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(MinHeap *minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
    if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;
    if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;
    if (smallest != idx) {
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

MinHeapNode *extractMin(MinHeap *minHeap) {
    MinHeapNode *temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    --minHeap->size;
    minHeapify(minHeap, 0);
    return temp;
}

void insertMinHeap(MinHeap *minHeap, MinHeapNode *minHeapNode) {
    ++minHeap->size;
    int i = minHeap->size - 1;
    while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    minHeap->array[i] = minHeapNode;
}

MinHeap *createAndBuildMinHeap(char data[], int freq[], int size) {
    MinHeap *minHeap = createMinHeap(size);
    for (int i = 0; i < size; ++i)
        minHeap->array[i] = newMinHeapNode(data[i], freq[i]);
    minHeap->size = size;
    for (int i = (size - 1) / 2; i >= 0; i--)
        minHeapify(minHeap, i);
    return minHeap;
}

HuffmanNode *buildHuffmanTree(char data[], int freq[], int size) {
    HuffmanNode *left, *right, *top;
    MinHeap *minHeap = createAndBuildMinHeap(data, freq, size);
    while (minHeap->size != 1) {
        left = extractMin(minHeap);
        right = extractMin(minHeap);
        top = (HuffmanNode *) malloc(sizeof(HuffmanNode));
        top->data = '$';
        top->freq = left->freq + right->freq;
        top->left = left;
        top->right = right;
        insertMinHeap(minHeap, (MinHeapNode *) top);
    }
    return (HuffmanNode *) extractMin(minHeap);
}

void printCodes(HuffmanNode *root, int arr[], int top, FILE *fp) {
    if (root->left) {
        arr[top] = 0;
        printCodes(root->left, arr, top + 1, fp);
    }
    if (root->right) {
        arr[top] = 1;
        printCodes(root->right, arr, top + 1, fp);
    }
    if (!root->left && !root->right) {
        fprintf(fp, "%c:", root->data);
        for (int i = 0; i < top; ++i)
            fprintf(fp, "%d", arr[i]);
        fprintf(fp, "\n");
    }
}

void HuffmanCodes(char data[], int freq[], int size, FILE *fp) {
    HuffmanNode *root = buildHuffmanTree(data, freq, size);
    int arr[MAX_TREE_HT], top = 0;
    printCodes(root, arr, top, fp);
}

void encode(FILE *input_fp, FILE *output_fp, FILE *code_fp) {
    char data[MAX_TREE_HT];
    int freq[MAX_TREE_HT] = {0};
    char c;
    while ((c = fgetc(input_fp)) != EOF) {
        ++freq[c];
    }
    int size = 0;
    for (int i = 0; i < 256; ++i) {
        if (freq[i]) {
            data[size++] = i;
        }
    }
    HuffmanCodes(data, freq, size, code_fp);
    fseek(input_fp, 0, SEEK_SET);
    unsigned char buffer = 0;
    int bit_count = 0;
    while ((c = fgetc(input_fp)) != EOF) {
        char code[MAX_TREE_HT];
        fseek(code_fp, 0, SEEK_SET);
        while (fgets(code, MAX_TREE_HT, code_fp) != NULL) {
            char symbol;
            int len = strlen(code) - 2;
            sscanf(code, "%c:", &symbol);
            if (symbol == c) {
                for (int i = 0; i < len; ++i) {
                    if (code[i + 2] == '1') {
                        buffer |= (1 << (7 - bit_count));
                    }
                    ++bit_count;
                    if (bit_count == 8) {
                        fputc(buffer, output_fp);
                        buffer = 0;
                        bit_count = 0;
                    }
                }
                break;
            }
        }
    }
    if (bit_count > 0) {
        buffer = (8 - bit_count);
        fputc(buffer, output_fp);
    }
}


int main() {
    printf("Hello, my user.\nThis program allows you to compress files using the Huffman algorithm."
           "\nFirst, create a file and write its name to the console.\n");
    char from[50];
    scanf("%s", from);
    FILE *input_fp = fopen(from, "rb");
    FILE *output_fp = fopen("output.bin", "wb");
    FILE *code_fp = fopen("code.txt", "w");

    printf("input_fp = %p\n", input_fp);
    printf("output_fp = %p\n", output_fp);
    printf("code_fp = %p\n", code_fp);

    if (!input_fp){
            fprintf(stderr, "Error while opening files\n");
            return 1;
        }
    fseek(input_fp, 0L, SEEK_END);
    long length = ftell(input_fp);
    char freq[length];
    fseek(input_fp, 0, SEEK_SET);
    for (int i = 0; i < length; ++i) {
        freq[(unsigned char) fgetc(input_fp)]++;
    }
//    printf("input_fp = %p\n", input_fp);
//    printf("output_fp = %p\n", output_fp);
//    printf("code_fp = %p\n", code_fp);

    encode(input_fp, output_fp, code_fp);
    fclose(input_fp);
    fclose(output_fp);
    fclose(code_fp);
    return 0;
}
