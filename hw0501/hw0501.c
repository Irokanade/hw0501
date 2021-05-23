//
//  hw0501.c
//  hw0501
//
//  Created by michaelleong on 21/05/2021.
//

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <math.h>

struct option long_options[] = {
    {"enc", 1, NULL, 'e'},
    {"dec", 1, NULL, 'd'},
    {"output", 1, NULL, 'o'},
    { 0, 0, 0, 0},
};

uint64_t util_getFdSize( int fd );
void encodeBase64(const unsigned char *data, size_t length, FILE *outputFile);
unsigned int convertStrBinToDec(char *binary, size_t len);
void printBits(size_t const size, void const * const ptr);
unsigned int getIndexEncodingTable(char encodedChar);
void decodeBase64(const unsigned char *data, size_t length, FILE *outputFile);

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};


int main(int argc, char* argv[]) {
    int c;
    int index;
    
    char encodeFileName[129] = {0};
    char decodeFileName[129] = {0};
    char outputFileName[129] = {0};
    
    int encodeOption = 0;
    int decodeOption = 0;
    
    //get options long
    while ( ( c = getopt_long( argc, argv, "e:d:o:", long_options, &index ) ) != -1 )
    {
        //printf( "index: %d\n", index );
        switch( c )
        {
            case 'e':
                printf("option e\n");
                strncpy(encodeFileName, optarg, strlen(optarg));
                printf("encode file name: %s\n", encodeFileName);
                encodeOption = 1;
                break;
            case 'd':
                printf("option d\n");
                strncpy(decodeFileName, optarg, strlen(optarg));
                printf("decode file name: %s\n", decodeFileName);
                decodeOption = 1;
                break;
                
            case 'o':
                printf("option: o\n");
                strncpy(outputFileName, optarg, strlen(optarg));
                printf("output file name: %s\n", outputFileName);
                break;
                
            case '?':
                printf("invalid option >:[\n");
                printf("use '--help for more options'");
                break;
            default:
                printf("invalid option >:[\n");
                printf("use '--help for more options'");
                break;
        }
    }
    
    //encode option
    if(encodeOption) {
        int encodeDataFD = open(encodeFileName, O_RDWR);
        
        //open the file descriptor
        if(encodeDataFD == -1 )
        {
            printf( "Open file error!\n" );
            return -1;
        }
        
        unsigned char *encodeDataPointer = NULL;
        uint64_t encodeDataFileSize = util_getFdSize(encodeDataFD);
        encodeDataPointer = mmap( 0, encodeDataFileSize, PROT_READ | PROT_WRITE, MAP_SHARED, encodeDataFD, 0 );
        //printf("size: %u\n", encodeDataFileSize);
        FILE *outputFile = NULL;
        
        if((outputFile = fopen(outputFileName, "wb")) == NULL) {
            printf("Open output file error >:[\n");
            return -1;
        }
        
        encodeBase64(encodeDataPointer, encodeDataFileSize, outputFile);
        
        munmap(encodeDataPointer, encodeDataFileSize);
        close(encodeDataFD);
        fclose(outputFile);
        
    } else if(decodeOption) {
        /*for(size_t i = 0; i < 32; i++) {
            printf("%d ", encoding_table[i]);
        }*/
        
        int decodeDataFD = open(decodeFileName, O_RDWR);
        
        //open the file descriptor
        if(decodeDataFD == -1 )
        {
            printf( "Open file error!\n" );
            return -1;
        }
        
        unsigned char *decodeDataPointer = NULL;
        uint64_t decodeDataFileSize = util_getFdSize(decodeDataFD);
        decodeDataPointer = mmap( 0, decodeDataFileSize, PROT_READ | PROT_WRITE, MAP_SHARED, decodeDataFD, 0 );
        
        FILE *outputFile = NULL;
        
        if((outputFile = fopen(outputFileName, "wb")) == NULL) {
            printf("Open output file error >:[\n");
            return -1;
        }
        
        decodeBase64(decodeDataPointer, decodeDataFileSize, outputFile);
        
        munmap(decodeDataPointer, decodeDataFileSize);
        close(decodeDataFD);
        fclose(outputFile);
    }
    
    
    
    return 0;
}

// Assumes little endian
void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    
    for (i = (int)size-1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}

void encodeBase64(const unsigned char *data, size_t length, FILE *outputFile) {
    char base64Bin[7] = {'0'}; //6 bits one null char
    base64Bin[6] = 0;
    int index = 0;
    int padding = 0;
    
    int totalBase64Char = (ceil((double)(length*8)/6)); //number of base64 characters
    while(totalBase64Char % 4 != 0) {
        totalBase64Char++; //add '=' padding character
        padding++;
    }
    
    for(int i = 0; i < length; i++) {
        //printf("%c ", *(data+i));
        for(int j = 8-1; j >= 0; j--) {
            //printf("%u", (*(data+i) >> j) & 1);
            if(index > 5) {
                unsigned int base64 = convertStrBinToDec(base64Bin, strlen(base64Bin));
                //printf("%c", encoding_table[base64]);
                //fwrite(&encoding_table[base64], 1, 1, outputFile);
                fprintf(outputFile, "%c", encoding_table[base64]);
                memset(base64Bin, '0', 6);
                base64Bin[6] = 0;
                index = 0;
                base64Bin[index] = ((*(data+i) >> j) & 1) + '0';
            } else {
                base64Bin[index] = ((*(data+i) >> j) & 1) + '0';
            }
            index++;
        }
    }
    
    unsigned int base64 = convertStrBinToDec(base64Bin, strlen(base64Bin));
    printf("%c", encoding_table[base64]);
    //fwrite(&encoding_table[base64], 1, 1, outputFile);
    fprintf(outputFile, "%c", encoding_table[base64]);
    
    for(size_t i = 0; i < padding; i++) {
        //printf("=");
        //fwrite("=", 1, 1, outputFile);
        fputc('=', outputFile);
    }
}

void decodeBase64(const unsigned char *data, size_t length, FILE *outputFile) {
    char bufferBits[25] = {'0'}; //4 bytes and one null terminating character
    bufferBits[24] = 0;
    int index = 0;
    int padding = 0;
    
    for(int i = 0; i < length; i++) {
        //printf("%c : ", *(data+i));
        //yprintf("%u\n", getIndexEncodingTable(*(data+i)));
        unsigned int encoded6bits = getIndexEncodingTable(*(data+i));
        
        if(encoded6bits == 64) {
            padding++;
            continue;
        }
        
        for(int j = 8-1-2; j >= 0; j--) {
            //get the first 6 bits
            if(index > 23) {
                for(int k = 0; k < 23; k += 8) {
                    unsigned int decodedByte = convertStrBinToDec(&bufferBits[k], 8);
                    //printf("%x ", decodedByte);
                    fwrite(&decodedByte, 1, 1, outputFile);
                }
                memset(bufferBits, '0', 24);
                bufferBits[24] = 0;
                index = 0;
                bufferBits[index] = ((encoded6bits >> j) & 1) + '0';
            } else {
                bufferBits[index] = ((encoded6bits >> j) & 1) + '0';
            }
            index++;
        }
    }
    
    for(size_t i = 0; i < 23 - (padding*6); i += 8) {
        unsigned int decodedByte = convertStrBinToDec(&bufferBits[i], 8);
        //printf("%x ", decodedByte);
        fwrite(&decodedByte, 1, 1, outputFile);
    }
}

uint64_t util_getFdSize( int fd )
{
    struct stat statbuf;
    
    if( fstat( fd, &statbuf ) < 0 )
    {
        close( fd );
        return -1;
    }
    
    return statbuf.st_size;
}

unsigned int convertStrBinToDec(char *binary, size_t len) {
    unsigned int sum = 0;
    
    for(int i = (int)len-1; i >= 0; i--) {
        //printf("%u", binary[i]-'0');
        sum += (binary[i]-'0')*pow(2, (len-1)-i);
    }
    
    return sum;
}

unsigned int getIndexEncodingTable(char encodedChar) {
    for(unsigned int i = 0; i < 64; i++) {
        //printf("%d ", encoding_table[i]);
        if(encodedChar == encoding_table[i]) {
            //printf("%u\n", i);
            return i;
        }
    }
    
    //else return 64;
    return 64;
}
