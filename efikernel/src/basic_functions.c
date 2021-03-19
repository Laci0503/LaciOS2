#include <basic_functions.h>
#include <kernel_main.h>

char hexletters[]="0123456789ABCDEF";

void int_to_text(uint64 n, uint8 string[]){
    uint64 div=n;
    uint8 index=0;
    if(div==0){
        string[0]='0';
        return;
    }
    char buffer[21];
    while (div>0){
        buffer[index]=div%10+'0';
        div=(uint64)div/10;
        index++;
    }
    for(uint8 i=0;i<21;i++){string[i]=0;}
    for(uint8 i=0;i<index;i++){
        string[i]=buffer[index-i-1];
    }
    return;
}
uint8 inb(uint16 port){
    uint8 ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "d"(port));
    return ret;
}
uint16 inb16(uint16 port){
    uint16 ret;
    __asm__ volatile("inw %1, %0" : "=a" (ret) : "Nd" (port));
    return ret;
}
uint32 inb32(uint16 port){
    uint32 ret = 0;
    asm volatile("inl %1, %0" : "=a"(ret) : "d"(port));
    return ret;
}
void outb(uint16 port, uint8 data)
{
    asm volatile("outb %0, %1" : : "a" (data), "Nd" (port));
    return;
}
void outb16(uint16 port, uint16 data)
{
    asm volatile("outw %0, %1" : : "a" (data), "Nd" (port));
    return;
}
void outb32(uint16 port, uint32 data){
    asm volatile("outl %0, %1" : : "a"(data), "Nd" (port));
}
void print_to_serial(char *buf){
    uint64 i=0;
    while(buf[i]!=0){
        outb(SERIAL_PORT,buf[i]);
        i++;
    }
}
void print_int_to_serial(uint64 n){
    char buf[30];
    for(uint8 i=0;i<30;i++)buf[i]=0;
    int_to_text(n,buf);
    print_to_serial(buf);
}
void int_to_text_hex(uint64 n, char *string){
    uint64 div=n;
    uint8 index=0;
    if(div==0){
        string[0]='0';
        return;
    }
    char buffer[21];
    while (div>0){
        buffer[index]=*(hexletters+div%16);
        div=(uint64)div/16;
        index++;
    }
    for(uint8 i=0;i<21;i++){string[i]=0;}
    for(uint8 i=0;i<index;i++){
        string[i]=buffer[index-i-1];
    }
    return;
}
void print_hex_to_serial(uint64 n){
    char buf[30];
    for(uint8 i=0;i<30;i++)buf[i]=0;
    buf[0]='0';
    buf[1]='x';
    int_to_text_hex(n,buf+2);
    print_to_serial(buf);
}
void int_to_text_bin(uint64 n, char string[]){
    uint64 div=n;
    uint8 index=0;
    if(div==0){
        string[0]='0';
        return;
    }
    char buffer[33];
    while (div>0){
        buffer[index]=*(hexletters+div%2);
        div=(uint64)div/2;
        index++;
    }
    for(uint8 i=0;i<33;i++){string[i]=0;}
    for(uint8 i=0;i<index;i++){
        string[i]=buffer[index-i-1];
    }
    return;
}
void print_bin_to_serial(uint64 n){
    char buf[30];
    for(uint8 i=0;i<30;i++)buf[i]=0;
    buf[0]='0';
    buf[1]='b';
    int_to_text_bin(n,buf+2);
    print_to_serial(buf);
}
void print_signed_to_serial(int64 number){
    char buffer[30];
    for(uint8 i=0;i<30;i++){buffer[i]=0;}
    uint64 n;
    if(number<0){
        n=-1*number;
        buffer[0]='-';
        char* t = buffer;
        t++;
        int_to_text(n, t);
    }else{
        n=number;
        int_to_text(n, buffer);
    }
    print_to_serial(buffer);
    return;
}
uint8 strcmp(char str1[], char str2[]){
    if(len(str1) == len(str2)){
        for(uint16 i=0; i<len(str1); i++){
            if(str1[i]!=str2[i]) return 0;
        }
        return 1;
    }else return 0;
}
uint16 len(char text[]){
    uint16 l = 0;
    while (text[l]!=0){
        l++;
    }
    return l;
}
uint64 string_to_int(char* string){
    uint64 length=len(string);
    if(length==0){
        return 0;
    }
    uint64 number=0;
    uint64 tmp=0;
    for(uint64 i=0;i<length;i++){
        if(string[i]>='0' && string[i]<='0'+9){
            tmp=string[i]-'0';
            for(uint64 j=1;j<length-i;j++){
                tmp*=10;
            }
            number+=tmp;
        }else{
            return 0;
        }
    }
    return number;
}
int64 signed_string_to_int(char* string){
    uint64 length=len(string);
    if(length==0){
        return 0;
    }
    int64 number=0;
    int64 tmp=0;
    for(uint64 i=(string[0]=='-');i<length;i++){
        if(string[i]>='0' && string[i]<='0'+9){
            tmp=string[i]-'0';
            for(uint64 j=1;j<length-i;j++){
                tmp*=10;
            }
            number+=tmp;
        }else{
            return 0;
        }
    }
    return number * (string[0]=='-' ? -1 : 1);
}
float80 string_to_double(char* string){
    uint64 stringlength=len(string);
    if(stringlength==0)return 0;
    if(contains(string,".") ^ contains(string,",")){
        
        float80 number=0;
        uint64 decimalPosition=indexOf(string,".")+indexOf(string,",");
        uint64 tmp=0;
        for(uint64 i=(string[0]=='-' ? 1 : 0);i<stringlength;i++){
            if(string[i]>='0' && string[i]<=('0'+9)){
                tmp=string[i]-'0';
                tmp*=pwr(10,decimalPosition-i-1);
                number+=tmp;
            }else if (string[i]=='.' || string[i]==','){
                decimalPosition=i;
                break;
            }else{
                return (float80)0;
            }
        }

        for(uint16 i=decimalPosition+1;i<stringlength;i++){
            if(string[i]>='0' && string[i]<='0'+9){
                number+=(1.0/((float80)pwr(10,i-decimalPosition)))*(float80)(string[i]-'0');
            }else{
                return (float80)0;
            }
        }
        return number * (string[0]=='-' ? -1.0 : 1.0);
    }else{
        return (float80)signed_string_to_int(string);
    }
}
uint64 pwr(uint64 base, uint64 exponent){
    uint64 exp = exponent-1;
    if(exponent==0)return 1;
    uint64 bs = base;
    while(exp != 0){
        bs *= base;
        exp--;
    }
    return bs;
}
uint64 min(uint64 n1, uint64 n2){
    if(n1 < n2)return n1; else return n2;
}
uint64 max(uint64 n1, uint64 n2){
    if(n1 > n2)return n1; else return n2;
}
float80 sqrt(float80 n){
  float80 lo = 0, hi = n, mid;
  for(uint64 i = 0 ; i < 1000 ; i++){
      mid = (lo+hi)/2;
      if(mid*mid == n) return mid;
      if(mid*mid > n) hi = mid;
      else lo = mid;
  }
  return mid;
}
void float_to_text(float80 n, char string[]){
    uint64 int_part=(uint64) n;
    int_to_text(n,string);
    uint64 l = len(string);
    string[l]='.';
    l++;
    n-=(float80)int_part;
    for(uint64 i=0;i<min(FLOAT_TO_TEXT_MAX_DIGETS,15-l-1);i++){
        uint64 digit=(uint64)(n*10.0);
        string[l+i]='0'+digit;
        n-=(float80)digit/10.0;
        if((uint64)(n*pwr(10,FLOAT_TO_TEXT_MAX_DIGETS))==0)break;
        n*=10.0;
    }
    return;
}
void print_float_to_serial(float80 n){
    char buf[80];
    for(uint64 i=0;i<80;i++)buf[i]=0;
    float_to_text(n,buf);
    print_to_serial(buf);
}
uint8 contains(char* src, char* substr){
    uint64 srclen=len(src);
    uint64 substrlen=len(substr);

    if(substrlen>srclen)return 0;
    if(substrlen==0)return 1;
    if(srclen==0)return 0;

    for(uint64 i=0;i<=srclen-substrlen;i++){
        if(memcmp(src+i,substr,substrlen)){
            return 1;
        }
    }
    return 0;
}
uint64 indexOf(char* src,char* substr){
    uint64 srclen=len(src);
    uint64 substrlen=len(substr);

    if(substrlen>srclen)return 0;
    if(substrlen==0)return 1;
    if(srclen==0)return 0;

    for(uint64 i=0;i<=srclen-substrlen;i++){
        if(memcmp(src+i,substr,substrlen)){
            return i;
        }
    }
    return 0;
}
void memcpy(uint8* from, uint8* to, uint64 amount){
    for(uint64 i=0;i<amount;i++){
        to[i]=from[i];
    }
}
void strcpy(char* from, char* to){
    uint64 i=0;
    while(from[i]!=0){
        to[i]=from[i];
        i++;
    }
    to[i]=0;
}
uint8 memcmp(uint8* m1, uint8* m2, uint64 length){
    for(uint64 i=0;i<length;i++){
        if(m1[i]!=m2[i]){
            return 0;
        }
    }
    return 1;
}
void memset(uint8* address, uint64 length, uint8 byte){
    for(uint64 i=0;i<length;i++){
        address[i]=byte;
    }
    return;
}
