#ifndef _BASIC_FUNCTIONS_H
#define _BASIC_FUNCTIONS_H
#include <types.h>
#define FLOAT_TO_TEXT_MAX_DIGETS 14

void int_to_text(uint64 n, uint8 string[]);
uint8 inb(uint16 port);
uint16 inb16(uint16 port);
uint32 inb32(uint16 port);
void outb(uint16 port, uint8 data);
void outb16(uint16 port, uint16 data);
void outb32(uint16 port, uint32 data);
void print_to_serial(char* buf);
void print_int_to_serial(uint64 n);
void int_to_text_hex(uint64 n, char *string);
void print_hex_to_serial(uint64 n);
void int_to_text_bin(uint64 n, char string[]);
void print_bin_to_serial(uint64 n);
void print_signed_to_serial(int64 number);
void float_to_text(float80 n, char string[]);
uint8 strcmp(char str1[], char str2[]);
uint16 len(char text[]);
uint64 string_to_int(char* string);
int64 signed_string_to_int(char* string);
float80 string_to_double(char* string);
uint64 min(uint64 n1, uint64 n2);
uint64 max(uint64 n1, uint64 n2);
uint64 pwr(uint64 base, uint64 exponent);
float80 sqrt(float80 n);
void print_float_to_serial(float80 n);
uint8 contains(char* src, char* substr);
uint64 indexOf(char* src,char* substr);
void memcpy(uint8* from, uint8* to, uint64 amount);
void strcpy(char* from, char* to);
uint8 memcmp(uint8* m1, uint8* m2, uint64 length);
void memset(uint8* address, uint64 length, uint8 byte);

#endif