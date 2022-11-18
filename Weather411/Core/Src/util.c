#include "util.h"
//#include "UartRingbuffer.h"

int util_atoi_n(char *c, int n) {
    int result = 0;
    int multiplier = 1;
//
//    Uart_write('#', pc_uart);
//    Uart_write(&c, pc_uart);



    if (c && *c == '-') {
        multiplier = -1;
        c++;
    }

    else if (c && *c == '+') {
            multiplier = 1;
            c++;
    }

    else {
        multiplier = 1;
    }

    while (*c && n) {
        if (*c < '0' || *c > '9') {
            return result * multiplier;
        }



        result *= 10;
        result += *c - '0';
        c++;
        n--;
    }
    return (result * multiplier);
}

int appendStr(char *dst, char *src, int beginIndex) {
    int di = beginIndex;
    int si = 0;
//    int append = 0;

    while (src[si]) {
//        if (dst[di] == 0) {
//            append = 1;
//        }
//        if (append) {
            dst[di] = src[si];
            si++;
//        }
        di++;
    }
    dst[di] = 0;

    return di;
}
