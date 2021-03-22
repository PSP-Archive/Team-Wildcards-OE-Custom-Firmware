typedef struct{
unsigned char filename [12]; // 8.3 naming max
unsigned long size; // size of data
unsigned char* data; // variable sized data
} fakegzf;
