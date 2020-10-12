#include "paoias.h"

static uint32_t registers[0x20];
static int flags[0x10];
static uint32_t memory[0xffff];

static uint32_t array[] = {
    0x1829,
    0x8372,
    0x8000,
    0x0072,
    0x8731,
    0xa836,
    0xaaaa,
    0xded0,
    0x0083,
    0x8437,
    0x8373
};

static uint32_t array2[] = {
    0x8356,
    0x8a8d,
    0x1000,
    0x0002,
    0x0002,
    0x9236,
    0xa092,
    0xadda,
    0xded9,
    0xded2,
    0xded1
};

static int verbosity = 0;

static inline uint32_t cmd_code(uint32_t cmd) {
    return cmd >> 24;
}

static inline uint32_t op1(uint32_t cmd) {
    return cmd >> 16 & 0xff;
}

static inline uint32_t op2(uint32_t cmd) {
    return cmd >> 8 & 0xff;
}

static inline uint32_t literal(uint32_t cmd) {
    return cmd & 0xffff;
}

static inline uint32_t *addr(uint16_t offset) {
    return (uint32_t *)((char*)&memory + offset);
}

static inline void compare(uint32_t op1, uint32_t op2) {
    uint32_t res = op1 - op2;
    flags[zf] = !res;
    flags[sf] = res >> 31;
}

static inline void test(uint32_t op1, uint32_t op2) {
    compare(op1 & op2, 0);
}

static inline void init_array() {
    uint32_t *base = memory + (0x4000 / sizeof(uint32_t));
    *(base++) = sizeof(array) / sizeof(array[0]);
    memcpy(base, array, sizeof(array));
}

static inline void init_second_array() {
    if (sizeof(array) != sizeof(array2)) {
        fprintf(stderr, "ERROR: Array sizes not equals\n");
        exit(EXIT_FAILURE);
    }
    uint32_t *base = memory + (0x5000 / sizeof(uint32_t));
    *(base++) = sizeof(array2) / sizeof(array2[0]);
    memcpy(base, array2, sizeof(array2));
}

static inline void execute_command() {
    const uint32_t command = *addr(registers[eip]);
    switch (cmd_code(command)) {
        case mov_rm:
            registers[op1(command)] = *addr(literal(command));
            break;
        case mov_rl:
            registers[op1(command)] = literal(command);
            break;
        case mov_rt:
            registers[op1(command)] = *addr(registers[op2(command)]);
            break;
        case cmp_rr:
            compare(registers[op1(command)], registers[op2(command)]);
            break;
        case add_rl:
            registers[op1(command)] += literal(command);
            break;
        case sub_rl:
            registers[op1(command)] -= literal(command);
            break;
        case jz_r:
            if (flags[zf])
                registers[eip] += literal(command);
            break;
        case test_rr:
            test(registers[op1(command)], registers[op2(command)]);
            break;
        case cmov_gt_rr:
            if (flags[sf])
                registers[op1(command)] = registers[op2(command)];
            break;
        case mul_rr: {
            uint64_t res = registers[op1(command)] * registers[op2(command)];
            registers[op1(command)] = res >> 32;
            registers[op2(command)] = res & 0xffffffff;
            break;
        }
        case adc_rr:
            registers[op1(command)] += registers[op2(command)] + flags[cf];
            flags[cf] = 0;
            break;
        case add_rr:
            flags[cf] = (registers[op1(command)] + registers[op2(command)]) < registers[op1(command)];
            registers[op1(command)] += registers[op2(command)];
            break;
        case halt:
            print_state();
            exit(EXIT_SUCCESS);
        default:
            fprintf(stderr, "ERROR: invalid command %x\n", command);
            exit(EXIT_FAILURE);
    }

    registers[eip] += sizeof(uint32_t);
}

void exec_loop() {
    init_array();
    init_second_array();
    while (1) {
        if (verbosity) print_state();
        execute_command();
        if (verbosity) getchar();
    }
}

void print_state() {
    printf("EAX:\t%#010x\tEBX:\t%#010x\tECX:\t%#010x\tZF:\t%x\n",
           registers[eax], registers[ebx], registers[ecx], flags[zf]);
    printf("EDX:\t%#010x\tEIP:\t%#010x\tCMD:\t%#010x\tSF:\t%x\n",
           registers[edx], registers[eip], *addr(registers[eip]), flags[sf]);
    printf("ESI:\t%#010x\tEDI:\t%#010x\tEBP:\t%#010x\tCF:\t%x\n",
           registers[esi], registers[edi], registers[ebp], flags[cf]);
}

static inline uint16_t hex_to_uint16(const char * const hex) {
    return strtoul(hex, NULL, 16);
}

static inline uint16_t str_to_reg(const char * const str) {
    if (!strcmp(str, "eax")) return eax;
    if (!strcmp(str, "ebx")) return ebx;
    if (!strcmp(str, "ecx")) return ecx;
    if (!strcmp(str, "edx")) return edx;
    if (!strcmp(str, "eip")) return eip;
    if (!strcmp(str, "esi")) return esi;
    if (!strcmp(str, "edi")) return edi;
    if (!strcmp(str, "ebp")) return ebp;
    return -1;
}

static inline uint32_t cmd_rr(int code, char *op1, char *op2) {
    uint32_t command = 0;
    command |= code << 24;
    command |= str_to_reg(op1) << 16;
    command |= str_to_reg(op2) << 8;
    return command;
}

static inline uint32_t cmd_rl(int code, char *op1, char *op2) {
    uint32_t command = 0;
    command |= code << 24;
    command |= str_to_reg(op1) << 16;
    command |= hex_to_uint16(op2);
    return command;
}

static inline void write_exec_file() {
    FILE *executable = fopen("executable", "wb");
    if (!executable) {
        fprintf(stderr, "ERROR: Count not open file");
        exit(EXIT_FAILURE);
    }
    fwrite(memory, 0xfff, 1, executable);
    fclose(executable);
}

static inline void read_exec_file(const char * const filename) {
    FILE *executable = fopen(filename, "rb");
    if (!executable) {
        fprintf(stderr, "ERROR: Could not opne file %s", filename);
        exit(EXIT_FAILURE);
    }
    fread(memory, 0xfff, 1, executable);
    fclose(executable);
}

void parse(const char * const filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "ERROR: Could not opne file %s", filename);
        exit(EXIT_FAILURE);
    }

    char *line;
    size_t len = 0;
    ssize_t read;

    char cmd[16], op1[16], op2[16];

    uint32_t *command_pointer = memory;

    while ((read = getline(&line, &len, fp)) != -1) {
        if (read < 5 || line[0] == '/')
            continue;
        sscanf(line, "%s %s %s", cmd, op1, op2);

        uint32_t command = 0;
        
        if (!strcmp("MOV_RL", cmd)) {
            command = cmd_rl(mov_rl, op1, op2);
        } else if (!strcmp("MOV_RM", cmd)) {
            command = cmd_rl(mov_rm, op1, op2);
        } else if (!strcmp("MOV_RT", cmd)) {
            command = cmd_rr(mov_rt, op1, op2);
        } else if (!strcmp("ADD_RL", cmd)) {
            command = cmd_rl(add_rl, op1, op2);
        } else if (!strcmp("CMP_RR", cmd)) {
            command = cmd_rr(cmp_rr, op1, op2);
        } else if (!strcmp("CMOV_GT_RR", cmd)) {
            command = cmd_rr(cmov_gt_rr, op1, op2);
        } else if (!strcmp("SUB_RL", cmd)) {
            command = cmd_rl(sub_rl, op1, op2);
        } else if (!strcmp("TEST_RR", cmd)) {
            command = cmd_rr(test_rr, op1, op2);
        } else if (!strcmp("JZ_R", cmd)) {
            command |= jz_r << 24;
            command |= hex_to_uint16(op1);
        } else if (!strcmp("HALT", cmd)) {
            command |= halt << 24;
        } else if (!strcmp("MUL_RR", cmd)) {
            command = cmd_rr(mul_rr, op1, op2);
        } else if (!strcmp("ADC_RR", cmd)) {
            command = cmd_rr(adc_rr, op1, op2);
        } else if (!strcmp("ADD_RR", cmd)) {
            command = cmd_rr(add_rr, op1, op2);
        } else {
            fprintf(stderr, "ERROR: Not implemented\n");
        }

        printf("Command: %-7s; OP1: %-6s; OP2: %-6s; Opcode: %#010x\n", cmd, op1, op2, command);
        
        *op1 = *op2 = 0;

        *(command_pointer++) = command;
    }

    fclose(fp);
    if (line)
        free(line);
}

static inline void help(const char * const name) {
    printf("usage: %s [OPTIONS]\n", name);
    puts("  -c, --compile\tcompile asm\n" \
         "  -i, --interpret\trun asm\n" \
         "  -r, --run\trun executable\n" \
         "  -h, --help\tprint help");
}

int main(int argc, char **argv) {
    int opt, long_index;

    if (argc == 1) {
        help(argv[0]);
        exit(EXIT_SUCCESS);
    }

    while (optind < argc) {
        if ((opt = getopt_long(argc, argv, opts, long_options, &long_index)) == -1) {
            ++optind;
            continue;
        }
        switch (opt) {
            case 'v':
                ++verbosity;
                break;
            case 'c':
                parse(optarg);
                write_exec_file();
                break;
            case 'i':
                parse(optarg);
                exec_loop();
                break;
            case 'r':
                read_exec_file(optarg);
                exec_loop();
                break;
            case 'h':
                help(argv[0]);
                break;
            case 't': {
                uint64_t result = 0;
                for (int i = 0; i < sizeof(array) / sizeof(array[0]); ++i)
                    result += array[i] * array2[i];
                printf("%#016llx\n", result);
                break;
            }
        }
    }

    return EXIT_SUCCESS;
}
