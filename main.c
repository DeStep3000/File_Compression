#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <locale.h>

// Структура для представления узла в дереве Хаффмана
struct HuffmanNode {
    unsigned char data; // Данные символа
    unsigned int frequency; // Частота символа
    struct HuffmanNode* left; // Левый потомок
    struct HuffmanNode* right; // Правый потомок
};

// Структура для представления очереди приоритетов
struct PriorityQueue {
    unsigned int size; // Размер очереди
    unsigned int capacity; // Емкость очереди
    struct HuffmanNode** nodes; // Массив узлов
};

// Создание нового узла дерева Хаффмана
struct HuffmanNode* createNode(unsigned char data, unsigned int frequency) {
    struct HuffmanNode* node = (struct HuffmanNode*)malloc(sizeof(struct HuffmanNode));
    node->data = data;
    node->frequency = frequency;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// Создание очереди приоритетов
struct PriorityQueue* createPriorityQueue(unsigned int capacity) {
    struct PriorityQueue* queue = (struct PriorityQueue*)malloc(sizeof(struct PriorityQueue));
    queue->size = 0;
    queue->capacity = capacity;
    queue->nodes = (struct HuffmanNode**)malloc(capacity * sizeof(struct HuffmanNode*));
    return queue;
}

// Проверка, является ли очередь приоритетов пустой
bool isQueueEmpty(struct PriorityQueue* queue) {
    return queue->size == 0;
}

// Обмен двух узлов в очереди приоритетов
void swapNodes(struct HuffmanNode** a, struct HuffmanNode** b) {
    struct HuffmanNode* temp = *a;
    *a = *b;
    *b = temp;
}

// Просеивание вниз в очереди приоритетов
void heapify(struct PriorityQueue* queue, int index) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < queue->size && queue->nodes[left]->frequency < queue->nodes[smallest]->frequency)
        smallest = left;

    if (right < queue->size && queue->nodes[right]->frequency < queue->nodes[smallest]->frequency)
        smallest = right;

    if (smallest != index) {
        swapNodes(&queue->nodes[index], &queue->nodes[smallest]);
        heapify(queue, smallest);
    }
}

// Добавление узла в очередь приоритетов
void enqueue(struct PriorityQueue* queue, struct HuffmanNode* node) {
    if (queue->size == queue->capacity)
        return;

    int i = queue->size;
    queue->nodes[i] = node;
    queue->size++;

    while (i != 0 && queue->nodes[(i - 1) / 2]->frequency > queue->nodes[i]->frequency) {
        swapNodes(&queue->nodes[(i - 1) / 2], &queue->nodes[i]);
        i = (i - 1) / 2;
    }
}

// Удаление и возврат узла с наименьшей частотой из очереди приоритетов
struct HuffmanNode* dequeue(struct PriorityQueue* queue) {
    if (isQueueEmpty(queue))
        return NULL;

    struct HuffmanNode* node = queue->nodes[0];
    queue->nodes[0] = queue->nodes[queue->size - 1];
    queue->size--;

    heapify(queue, 0);

    return node;
}

// Построение дерева Хаффмана на основе частот символов
struct HuffmanNode* buildHuffmanTree(unsigned char data[], unsigned int frequency[], unsigned int size) {
    struct HuffmanNode *left, *right, *top;

    // Создание очереди приоритетов и заполнение ее узлами-листьями
    struct PriorityQueue* queue = createPriorityQueue(size);
    for (int i = 0; i < size; ++i)
        enqueue(queue, createNode(data[i], frequency[i]));

    // Построение дерева Хаффмана
    while (!isQueueEmpty(queue)) {
        left = dequeue(queue);
        right = dequeue(queue);

        top = createNode('$', left->frequency + right->frequency);
        top->left = left;
        top->right = right;

        enqueue(queue, top);
    }

    // Извлечение корня дерева Хаффмана
    return dequeue(queue);
}

// Запись бита в байт
void writeBit(FILE* file, unsigned char* currentByte, unsigned char* bitPosition, bool bit) {
    *currentByte <<= 1;
    if (bit)
        *currentByte |= 1;

    (*bitPosition)++;
    if (*bitPosition == 8) {
        fwrite(currentByte, sizeof(unsigned char), 1, file);
        *currentByte = 0;
        *bitPosition = 0;
    }
}

// Чтение бита из байта
bool readBit(FILE* file, unsigned char* currentByte, unsigned char* bitPosition) {
    if (*bitPosition == 0) {
        fread(currentByte, sizeof(unsigned char), 1, file);
        *bitPosition = 8;
    }

    bool bit = (*currentByte >> (*bitPosition - 1)) & 1;
    (*bitPosition)--;
    return bit;
}

// Запись заголовка с информацией о дереве Хаффмана в сжатый файл
void writeHeader(FILE* compressedFile, struct HuffmanNode* root, unsigned int* dataSize) {
    if (root == NULL)
        return;

    if (root->left == NULL && root->right == NULL) {
        // Листовой узел
        writeBit(compressedFile, NULL, NULL, true);
        fwrite(&root->data, sizeof(unsigned char), 1, compressedFile);
        (*dataSize)++;
    } else {
        // Внутренний узел
        writeBit(compressedFile, NULL, NULL, false);
        (*dataSize)++;
        writeHeader(compressedFile, root->left, dataSize);
        writeHeader(compressedFile, root->right, dataSize);
    }
}

// Чтение заголовка из сжатого файла и построение дерева Хаффмана
struct HuffmanNode* readHeader(FILE* compressedFile, unsigned char* currentByte, unsigned char* bitPosition) {
    bool isLeaf = readBit(compressedFile, currentByte, bitPosition);

    if (isLeaf) {
        unsigned char data;
        fread(&data, sizeof(unsigned char), 1, compressedFile);
        return createNode(data, 0);
    } else {
        struct HuffmanNode* left = readHeader(compressedFile, currentByte, bitPosition);
        struct HuffmanNode* right = readHeader(compressedFile, currentByte, bitPosition);
        struct HuffmanNode* node = createNode('$', 0);
        node->left = left;
        node->right = right;
        return node;
    }
}

// Сжатие файла с использованием алгоритма Хаффмана
void compressFile(const char* sourceFilePath, const char* compressedFilePath) {
    FILE* sourceFile = fopen(sourceFilePath, "rb");
    if (sourceFile == NULL) {
        printf("Не удалось открыть исходный файл.\n");
        return;
    }

    // Подсчет частоты символов в исходном файле
    unsigned int frequency[256] = {0};
    unsigned char data;
    while (fread(&data, sizeof(unsigned char), 1, sourceFile))
        frequency[data]++;

    fclose(sourceFile);

    // Построение дерева Хаффмана
    struct HuffmanNode* root = buildHuffmanTree((unsigned char[]){0}, frequency, 256);

    // Открытие сжатого файла для записи
    FILE* compressedFile = fopen(compressedFilePath, "wb");
    if (compressedFile == NULL) {
        printf("Не удалось открыть сжатый файл.\n");
        return;
    }

    // Запись заголовка в сжатый файл
    unsigned int dataSize = 0;
    writeHeader(compressedFile, root, &dataSize);

    // Открытие исходного файла для чтения
    sourceFile = fopen(sourceFilePath, "rb");
    if (sourceFile == NULL) {
        printf("Не удалось открыть исходный файл.\n");
        return;
    }

    // Кодирование данных и запись в сжатый файл
    unsigned char currentByte = 0;
    unsigned char bitPosition = 0;
    while (fread(&data, sizeof(unsigned char), 1, sourceFile)) {
        struct HuffmanNode* current = root;
        while (current->left != NULL && current->right != NULL) {
            bool bit = (data >> bitPosition) & 1;
            if (bit)
                current = current->right;
            else
                current = current->left;

            bitPosition++;
            if (bitPosition == 8) {
                bitPosition = 0;
                fread(&data, sizeof(unsigned char), 1, sourceFile);
            }
        }
        writeBit(compressedFile, &currentByte, &bitPosition, (current->data >> 7) & 1);
    }

    // Запись последнего неполного байта, если есть
    while (bitPosition != 0)
        writeBit(compressedFile, &currentByte, &bitPosition, 0);

    // Освобождение памяти, закрытие файлов
    fclose(sourceFile);
    fclose(compressedFile);
    free(root);
}

// Распаковка сжатого файла
void decompressFile(const char* compressedFilePath, const char* decompressedFilePath) {
    FILE* compressedFile = fopen(compressedFilePath, "rb");
    if (compressedFile == NULL) {
        printf("Не удалось открыть сжатый файл.\n");
        return;
    }

    // Чтение заголовка из сжатого файла и построение дерева Хаффмана
    unsigned char currentByte = 0;
    unsigned char bitPosition = 0;
    struct HuffmanNode* root = readHeader(compressedFile, &currentByte, &bitPosition);

    // Открытие распакованного файла для записи
    FILE* decompressedFile = fopen(decompressedFilePath, "wb");
    if (decompressedFile == NULL) {
        printf("Не удалось открыть распакованный файл.\n");
        return;
    }

    // Раскодирование данных и запись в распакованный файл
    struct HuffmanNode* current = root;
    while (fread(&currentByte, sizeof(unsigned char), 1, compressedFile)) {
        for (int i = 7; i >= 0; --i) {
            bool bit = (currentByte >> i) & 1;
            if (bit)
                current = current->right;
            else
                current = current->left;

            if (current->left == NULL && current->right == NULL) {
                fwrite(&current->data, sizeof(unsigned char), 1, decompressedFile);
                current = root;
            }
        }
    }

    // Освобождение памяти, закрытие файлов
    fclose(compressedFile);
    fclose(decompressedFile);
    free(root);
}

int main() {
    setlocale(LC_ALL,".1251");

    const char* sourceFilePath = "input.txt";
    const char* compressedFilePath = "compressed.bin";
    const char* decompressedFilePath = "decompressed.txt";

    compressFile(sourceFilePath, compressedFilePath);
    decompressFile(compressedFilePath, decompressedFilePath);

    printf("Сжатие и распаковка завершены.\n");

    return 0;
}
// моё болото