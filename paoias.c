#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

enum registers {
    esp,
    pc,
    REGISTERS_COUNT
};

enum flags {
    zf,
    cf,
    FLAGS_COUNT
};

enum instructions {
    cmd_push,
    cmd_pop,
    read,
    write,
    dup,
    swap,
    ror,
    rol,
    add,
    sub,
    cmp,
    jmp,
    jz,
    adc,
    mul,
    halt
};

static uint32_t registers[REGISTERS_COUNT];
static int flags[FLAGS_COUNT];

static uint32_t commands[0x1000];
static uint32_t *command_ptr = commands;
static uint32_t memory[0xffff];
static uint32_t *stack = memory + 0xfffe;

static int is_push = 0;

static uint32_t array[] = {
    0xdead,
    0x0904,
    0xbbef,
    0xcafe,
    0x8451,
    0xdd23
};

static uint32_t array2[] = {
    0xdead,
    0x0842,
    0x0372,
    0xaaee,
    0x1234,
    0x8362
};

void results() {
    uint32_t lab1 = 0;
    for (int i = 0; i < sizeof(array) / sizeof(array[0]); ++i)
        lab1 += array[i];
    
    printf("\tlab1:\t%#010x\t\t", lab1);
    
    uint64_t lab2 = 0;
    for (int i = 0; i < sizeof(array) / sizeof(array[0]); ++i)
        lab2 += array[i] * array2[i];
    
    printf("\tlab2:\t%#018llx\n", lab2);
}

void init_array() {
    uint32_t arr_size = sizeof(array) / sizeof(array[0]);
    *(memory) = arr_size;
    memcpy(memory + 1, array, sizeof(array));
    memcpy(memory + arr_size + 1, array2, sizeof(array2));
}

void push(const uint32_t value) {
    *(stack - ++registers[esp]) = value;
}

uint32_t pop() {
    if (!registers[esp]) {
        puts("ERROR: Stack underflow");
        exit(EXIT_FAILURE);
    }
    
    return *(stack - registers[esp]--);
}

void print_state() {
    printf(" Счетчик команд: %#010x (%d)", registers[pc], registers[pc]);
    
//    printf("\n Код следующей команды:\t%#010x", commands[registers[pc]]);
    printf("\tФлаги:");
    printf("\tZF: %x", flags[zf]);
    printf("\tCF: %x", flags[cf]);
    
    printf("\n Стек%s", registers[esp] ? ":" : ": пустой");
    
    for (uint32_t i = 1; i <= registers[esp]; ++i) {
        printf("\t(%x) %#010x", i, *(stack - i));
    }
    
    puts("\n");
}

void execute_command(const int verbose) {
    const uint32_t command = commands[registers[pc]];
    
    if (is_push) {
        is_push = 0;
        push(command);
        ++registers[pc];
        return;
    }
    
    switch (command) {
        case cmd_push:
            is_push = 1;
            break;
        case cmd_pop:
            pop();
            break;
        case read:
            push(memory[pop()]);
            break;
        case write: {
            uint32_t index = pop();
            uint32_t value = pop();
            memory[index] = value;
            break;
        }
        case dup: {
            uint32_t value = pop();
            push(value);
            push(value);
            break;
        }
        case swap: {
            uint32_t t1 = pop();
            uint32_t t2 = pop();
            push(t1);
            push(t2);
            break;
        }
        case ror: {
            uint32_t t1 = pop();
            uint32_t t2 = pop();
            uint32_t t3 = pop();
            push(t1);
            push(t3);
            push(t2);
            break;
        }
        case rol: {
            uint32_t t1 = pop();
            uint32_t t2 = pop();
            uint32_t t3 = pop();
            push(t2);
            push(t1);
            push(t3);
            break;
        }
        case add: {
            uint32_t op1 = pop();
            uint32_t op2 = pop();
            flags[cf] = (op1 + op2) < op1;
            push(op1 + op2);
            break;
        }
        case mul: {
            uint64_t res = pop() * pop();
            push(res >> 32);
            push(res & 0xffffffff);
            break;
        }
        case adc:
            push(pop() + pop() + flags[cf]);
            flags[cf] = 0;
            break;
        case sub: {
            uint32_t op2 = pop();
            uint32_t op1 = pop();
            push(op1 - op2);
            break;
        }
        case cmp:
            flags[zf] = !(pop() - pop());
            break;
        case jz: {
            uint32_t address = pop() - 1;
            if (flags[zf]) registers[pc] = address;
            break;
        }
        case jmp:
            registers[pc] = pop() - 1;
            break;
        case halt:
            if (!verbose)
                print_state();
            exit(EXIT_SUCCESS);
        default:
            fprintf(stderr, "ERROR: invalid command %x\n", command);
            exit(EXIT_FAILURE);
    }

    ++registers[pc];
}

void exec_loop(const int verbose) {
    init_array();
    while (1) {
        if (verbose) print_state();
        execute_command(verbose);
    }
}

void write_command(const char * const line, int print) {
    uint32_t literal, command;
    char cmd[128];
    
    sscanf(line, "%s //", cmd);
    
    if (!strcmp("push", cmd))
        command = cmd_push;
    else if (!strcmp("read", cmd))
        command = read;
    else if (!strcmp("write", cmd))
        command = write;
    else if (!strcmp("dup", cmd))
        command = dup;
    else if (!strcmp("swap", cmd))
        command = swap;
    else if (!strcmp("ror", cmd))
        command = ror;
    else if (!strcmp("rol", cmd))
        command = rol;
    else if (!strcmp("cmp", cmd))
        command = cmp;
    else if (!strcmp("jz", cmd))
        command = jz;
    else if (!strcmp("jmp", cmd))
        command = jmp;
    else if (!strcmp("add", cmd))
        command = add;
    else if (!strcmp("sub", cmd))
        command = sub;
    else if (!strcmp("halt", cmd))
        command = halt;
    else if (!strcmp("adc", cmd))
        command = adc;
    else if (!strcmp("mul", cmd))
        command = mul;
    else if (!strcmp("pop", cmd))
        command = cmd_pop;
    else if (sscanf(line, "%x", &command) == 0) {
        fprintf(stderr, "ERROR: Invalid command: %s\n", line);
        exit(EXIT_FAILURE);
    }
    
    if (print)
        printf("%s -> %#010x\n", cmd, command);
    
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
               "  compile [filename]\tcompile asm\n" \
               "  run [filename]\trun asm\n" \
               "  exec [filename]\tstep by step execute asm\n" \
               "  str [command]\t\tprint command asm code\n" \
               "\n  Example: %s exec sum.s\n", *argv, *argv);
        return EXIT_FAILURE;
    }
    
    const char *action = argv[1];
    const char *filename = argv[2];
    
    if (!strcmp(action, "compile")) {
        parse(filename, 1);
    } else if (!strcmp(action, "run")) {
        parse(filename, 0);
        exec_loop(0);
    } else if (!strcmp(action, "exec")) {
        parse(filename, 0);
        exec_loop(1);
    } else if (!strcmp(action, "test")) {
        results();
    } else if (!strcmp(action, "str")) {
        write_command(filename, 1);
    } else {
        printf("ERROR: unknown action given: %s\n", action);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
