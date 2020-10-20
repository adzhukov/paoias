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
    exit_cmd,
    mov,
    movl,
    movto,
    mem,
    cmp,
    je,
    addto,
    subfrom,
    jl,
    jmp,
    add,
    mul,
    adc
};

static uint32_t registers[REGISTERS_COUNT];
static int flags[FLAGS_COUNT];

static uint32_t memory[0xffff];
static uint32_t commands[0xffff];

static uint32_t *command_ptr = commands;

static uint32_t array1[] = {
    0x0123,
    0x4567,
    0x89ab,
    0xcdef,
    0x0123,
    0x4567,
    0x89ab,
    0xcdef,
    0x0123,
    0x4567,
    0x89ab,
    0xcdef
};

static uint32_t array2[] = {
    0xfedc,
    0xba98,
    0x7654,
    0x3210,
    0xfedc,
    0xba98,
    0x7654,
    0x3210,
    0xfedc,
    0xba98,
    0x7654,
    0x3210,
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
    uint32_t *base = memory;
    *(base++) = sizeof(array1) / sizeof(array1[0]);
    memcpy(base, array1, sizeof(array1));
    memcpy(base + *memory, array2, sizeof(array2));
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

uint16_t get_operand(uint32_t command) {
    return command & 0xffff;
}

void execute_command(const int verbose) {
    const uint32_t command = commands[registers[pc]];
    
    switch (command >> 16) {
        case mov:
            registers[eax] = registers[get_operand(command)];
            break;
        case movl:
            registers[eax] = get_operand(command);
            break;
        case movto:
            registers[get_operand(command)] = registers[eax];
            break;
        case mem:
            registers[eax] = memory[registers[get_operand(command)]];
            break;
        case cmp: {
            uint32_t res = registers[eax] - registers[get_operand(command)];
            flags[zf] = !res;
            flags[sf] = res >> 31;
            break;
        }
        case je:
            if (flags[zf])
                registers[pc] = get_operand(command) - 1;
            break;
        case jl:
            if (flags[sf])
                registers[pc] = get_operand(command) - 1;
            break;
        case jmp:
            registers[pc] = get_operand(command) - 1;
            break;
        case addto: {
            uint32_t dest = registers[get_operand(command)];
            registers[get_operand(command)] += registers[eax];
            flags[cf] = registers[get_operand(command)] < dest;
            break;
        }
        case subfrom:
            registers[get_operand(command)] -= registers[eax];
            break;
        case add:
            registers[eax] += get_operand(command);
            break;
        case mul: {
            uint32_t m1 = registers[eax];
            uint32_t m2 = registers[get_operand(command)];
            uint64_t res = m1 * m2;
            registers[eax] = res & 0xffffffff;
            registers[get_operand(command)] = res >> 32;
            break;
        }
        case adc:
            registers[get_operand(command)] += registers[eax] + flags[cf];
            break;
        case exit_cmd:
            if (!verbose)
                print_state();
            exit(get_operand(command));
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
    
    return REGISTERS_COUNT;
}

uint32_t assemble(uint16_t command, char *operand) {
    uint16_t op = parse_operand(operand);
    if (op == REGISTERS_COUNT)
        op = strtoul(operand, NULL, 16);
    
    return (uint32_t)0 | command << 16 | op;
}

void write_command(const char * const line, int print) {
    uint32_t command;
    char cmd[32], op[32];
    
    sscanf(line, "%s %s", cmd, op);
    
    if (!strcmp("mov", cmd))
        command = assemble(mov, op);
    else if (!strcmp("movl", cmd))
        command = assemble(movl, op);
    else if (!strcmp("movto", cmd))
        command = assemble(movto, op);
    else if (!strcmp("mem", cmd))
        command = assemble(mem, op);
    else if (!strcmp("cmp", cmd))
        command = assemble(cmp, op);
    else if (!strcmp("je", cmd))
        command = assemble(je, op);
    else if (!strcmp("addto", cmd))
        command = assemble(addto, op);
    else if (!strcmp("subfrom", cmd))
        command = assemble(subfrom, op);
    else if (!strcmp("jl", cmd))
        command = assemble(jl, op);
    else if (!strcmp("jmp", cmd))
        command = assemble(jmp, op);
    else if (!strcmp("add", cmd))
        command = assemble(add, op);
    else if (!strcmp("mul", cmd))
        command = assemble(mul, op);
    else if (!strcmp("adc", cmd))
        command = assemble(adc, op);
    else if (!strcmp("exit", cmd))
        command = assemble(exit_cmd, op);
    else {
        fprintf(stderr, "ERROR: Invalid command: %s\n", line);
        exit(EXIT_FAILURE);
    }
    
    if (print)
        printf("%s %s -> %#010x\n", cmd, op, command);
    
    *command_ptr++ = command;
}

void parse(const char * const filename, const int print) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "ERROR: Could not open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    char line[256];

    while (fgets((char*)&line, sizeof(char) * 256, fp)) {
        if (strlen(line) < 3 || line[0] == '/')
            continue;
        
        write_command(line, print);
    }

    fclose(fp);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: %s ...\n" \
               "  compile [filename]\tcompile\n" \
               "  run [filename]\trun\n" \
               "  execute [filename]\tstep by step execute\n" \
               "  string [command]\tprint single command asm code\n" \
               "\n  Example: %s e lab2.s\n", *argv, *argv);
        return EXIT_FAILURE;
    }
    
    const char *action = argv[1];
    const char *filename = argv[2];
    
    if (!strcmp(action, "compile")) {
        parse(filename, 1);
    } else if (!strcmp(action, "run")) {
        parse(filename, 0);
        exec_loop(0);
    } else if (!strcmp(action, "execute")) {
        parse(filename, 0);
        exec_loop(1);
    } else if (!strcmp(action, "test")) {
        results();
    } else if (!strcmp(action, "string")) {
        write_command(filename, 1);
    } else {
        printf("ERROR: unknown action given: %s\n", action);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
