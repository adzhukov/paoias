#ifndef paoias_h
#define paoias_h

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

const char * const opts = "i:r:c:ht";
static struct option long_options[] = {
    {"interpret", required_argument, NULL, 'i'},
    {"run", required_argument, NULL, 'r'},
    {"compile", required_argument, NULL, 'c'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}
};


enum registers {
    eax,
    ebx,
    ecx,
    edx,
    eip,
    esi,
    edi,
    ebp
};

enum flags {
    zf,
    sf,
    cf
};

enum instructions {
    invalid,
    
    mov_rm,
    mov_rl,
    mov_rt,
    cmp_rr,

    add_rl,
    
    sub_rl,
    
    jz_r,
    test_rr,
    
    cmov_gt_rr,
    
    add_rr,
    adc_rr,
    mul_rr,
    
    halt
};

#endif
