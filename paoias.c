#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

enum registers {
    eax,
    ebx,
    ecx,
    edx,
    eex,
    efx,
    egx,
    pc,
    REGISTERS_COUNT
};

enum flags {
    zf,
    sf,
    cf,
    FLAGS_COUNT
};

enum instructions {
    movl,
    memr,
    cmp,
    jmp,
    add,
    movr,
    mul,
    addr,
    adc,
    exit_cmd
};

enum jump_modes {
    any,
    eq
};

static uint32_t registers[REGISTERS_COUNT];
static int flags[FLAGS_COUNT];

static uint32_t memory[0xffff];
static uint32_t *command_ptr = memory;

static uint32_t array1[] = {
    0x9471,
    0x3837,
    0x8573,
    0x4916,
    0x0384,
    0xf0f0,
    0xeeee,
    0xaaaa,
    0xdddd,
    0x0001,
    0x4832
};

static uint32_t array2[] = {
    0x0123,
    0x4567,
    0x89ab,
    0xcdef,
    0x0011,
    0x2233,
    0x4455,
    0x6677,
    0x8899,
    0xaaff,
    0xdd00
};

void results() {
    uint32_t lab1 = 0;
    for (int i = 0; i < sizeof(array1) / sizeof(array1[0]); ++i)
        lab1 += array1[i];
    
    printf("\tlab1:\t%#010x\t\t", lab1);
    
    uint64_t lab2 = 0;
    for (int i = 0; i < sizeof(array1) / sizeof(array1[0]); ++i)
        lab2 += array1[i] * array2[i];
        
    
    printf("\tlab2:\t%#018llx\n", lab2);
}

void init_array() {
    uint32_t *base = memory + 0x1000;
    *(base++) = sizeof(array1) / sizeof(array1[0]);
    memcpy(base, array1, sizeof(array1));
    
    base = memory + 0x2000;
    *(base++) = sizeof(array2) / sizeof(array2[0]);
    memcpy(base, array2, sizeof(array2));
}

void print_state() {
    printf(" Счетчик команд:\t%#010x\t(%d)", registers[pc], registers[pc]);
    
    printf("\n Регистры:");
    
    printf("\t\teax:\t%#010x", registers[eax]);
    printf("\tebx:\t%#010x", registers[ebx]);
    printf("\tecx:\t%#010x", registers[ecx]);
    printf("\n edx:\t%#010x", registers[edx]);
    printf("\teex:\t%#010x", registers[eex]);
    printf("\tefx:\t%#010x", registers[efx]);
    printf("\tegx:\t%#010x", registers[egx]);
    
    printf("\n Флаги:");
    
    printf("\tsf:\t%x", flags[sf]);
    printf("\tzf:\t%x", flags[zf]);
    printf("\tcf:\t%x\n", flags[cf]);
}

uint8_t get_op1(uint32_t command) {
    return (command >> 16) & 0xf;
}

uint16_t get_op2(uint32_t command) {
    return command & 0xffff;
}

void execute_command(const int verbose) {
    const uint32_t command = memory[registers[pc]];
    
    switch (command >> 24) {
        case movl:
            registers[get_op1(command)] = get_op2(command);
            break;
        case movr:
            registers[get_op1(command)] = registers[get_op2(command)];
            break;
        case memr:
            registers[get_op1(command)] = memory[registers[get_op2(command)]];
            break;
        case cmp: {
            uint32_t res = registers[get_op1(command)] - registers[get_op2(command)];
            flags[zf] = !res;
            flags[sf] = res >> 31;
            break;
        }
        case jmp:
            switch (get_op1(command)) {
                case eq:
                    if (!flags[zf])
                        break;
                case any:
                    registers[pc] = get_op2(command) - 1;
                    break;
            }
            break;
        case addr: {
            uint32_t dest = registers[get_op1(command)];
            registers[get_op1(command)] += registers[get_op2(command)];
            flags[cf] = registers[get_op1(command)] < dest;
            break;
        }
        case add:
            registers[get_op1(command)] += get_op2(command);
            break;
        case mul: {
            uint32_t m1 = registers[get_op1(command)];
            uint32_t m2 = registers[get_op2(command)];
            uint64_t res = m1 * m2;
            registers[get_op1(command)] = res & 0xffffffff;
            registers[get_op2(command)] = res >> 32;
            break;
        }
        case adc:
            registers[get_op1(command)] += registers[get_op2(command)] + flags[cf];
            break;
        case exit_cmd:
            if (!verbose)
                print_state();
            exit(get_op2(command));
        default:
            fprintf(stderr, "ERROR: invalid command %#010x\n", command);
            exit(EXIT_FAILURE);
    }

    ++registers[pc];
}

void exec_loop(const int verbose) {
    init_array();
    while (1) {
        if (verbose) print_state();
        execute_command(verbose);
        if (verbose) getchar();
    }
}

uint16_t parse_operand(char *operand) {
    if (!strcmp("eax", operand))
        return eax;
    else if (!strcmp("ebx", operand))
        return ebx;
    else if (!strcmp("ecx", operand))
        return ecx;
    else if (!strcmp("edx", operand))
        return edx;
    else if (!strcmp("eex", operand))
        return eex;
    else if (!strcmp("efx", operand))
        return efx;
    else if (!strcmp("egx", operand))
        return egx;
    else if (!strcmp("any", operand))
        return any;
    else if (!strcmp("eq", operand))
        return eq;
    
    return strtoul(operand, NULL, 16);
}

uint32_t assemble(uint16_t command, char *operand1, char *operand2) {
    uint8_t op1 = parse_operand(operand1);
    uint16_t op2 = parse_operand(operand2);
    
    return (uint32_t)0 | command << 24 | op1 << 16 | op2;
}

void write_command(const char * const line, int print) {
    uint32_t command;
    char cmd[32], op1[32], op2[32];
    
    sscanf(line, "%s %s %s", cmd, op1, op2);
    
    if (!strcmp("memr", cmd))
        command = assemble(memr, op1, op2);
    else if (!strcmp("movl", cmd))
        command = assemble(movl, op1, op2);
    else if (!strcmp("cmp", cmd))
        command = assemble(cmp, op1, op2);
    else if (!strcmp("jmp", cmd))
        command = assemble(jmp, op1, op2);
    else if (!strcmp("add", cmd))
        command = assemble(add, op1, op2);
    else if (!strcmp("movr", cmd))
        command = assemble(movr, op1, op2);
    else if (!strcmp("mul", cmd))
        command = assemble(mul, op1, op2);
    else if (!strcmp("addr", cmd))
        command = assemble(addr, op1, op2);
    else if (!strcmp("adc", cmd))
        command = assemble(adc, op1, op2);
    else if (!strcmp("exit", cmd))
        command = assemble(exit_cmd, op1, op2);
    else {
        fprintf(stderr, "ERROR: Invalid command: %s\n", line);
        exit(EXIT_FAILURE);
    }
    
    if (print) {
        printf("%s %s %s -> %#010x\n", cmd, op1, op2, command);
        *op1 = *op2 = '\0';
    }
        
    
    *command_ptr++ = command;
}

void parse(const char * const filename, const int print) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "ERROR: Could not open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    char line[64];

    while (fgets((char*)&line, sizeof(char) * 64, fp)) {
        if (strlen(line) < 3 || line[0] == '/')
            continue;
        
        write_command(line, print);
    }

    fclose(fp);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: %s ...\n" \
               "  c [filename]\tcompile\n" \
               "  r [filename]\trun\n" \
               "  e [filename]\tstep by step execute\n" \
               "  s [command]\tprint single command asm code\n" \
               "\n  Example: %s e lab2.s\n", *argv, *argv);
        return EXIT_FAILURE;
    }
    
    const char *action = argv[1];
    const char *filename = argv[2];
    
    if (!strcmp(action, "c")) {
        parse(filename, 1);
    } else if (!strcmp(action, "r")) {
        parse(filename, 0);
        exec_loop(0);
    } else if (!strcmp(action, "e")) {
        parse(filename, 0);
        exec_loop(1);
    } else if (!strcmp(action, "t")) {
        results();
    } else if (!strcmp(action, "s")) {
        write_command(filename, 1);
    } else {
        printf("ERROR: unknown action given: %s\n", action);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
