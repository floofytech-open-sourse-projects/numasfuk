#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>

#define TAPE_SIZE 16
#define STR_MAX   64
#define BC_MAX    65536
#define MAX_INS   16384

uint8_t bytecode[BC_MAX];
int bc_len = 0;

int instr_bc_pos[MAX_INS];
int instr_src_idx[MAX_INS];
int instr_count = 0;

typedef struct {
    int bc_pos;      // where to patch in bytecode
    int raw_index;   // raw source index (J/Z/N argument)
} Fixup;

Fixup fixups[1024];
int fix_count = 0;

char tape[TAPE_SIZE][STR_MAX+1];
int ptr = 0;
int regv = 0;

/* ---------------- SAFE EMIT ---------------- */

void emit8(uint8_t b) {
    if (bc_len >= BC_MAX) {
        printf("bytecode overflow\n");
        exit(1);
    }
    bytecode[bc_len++] = b;
}

void emit32(int v) {
    if (bc_len + 4 > BC_MAX) {
        printf("bytecode overflow\n");
        exit(1);
    }
    memcpy(&bytecode[bc_len], &v, 4);
    bc_len += 4;
}

/* ---------------- INSTRUCTION REGISTRATION ---------------- */

void add_instr_entry(int src_idx) {
    if (instr_count >= MAX_INS) {
        printf("too many instructions\n");
        exit(1);
    }
    instr_bc_pos[instr_count]  = bc_len;
    instr_src_idx[instr_count] = src_idx;
    instr_count++;
}

/* ---------------- SINGLE-INSTRUCTION COMPILER ---------------- */

int compile_one(const char *src, int *pi);

/* Skip whitespace and comments, return new index */
int skip_ws_and_comments(const char *src, int i) {
    for (;;) {
        while (isspace((unsigned char)src[i])) i++;
        if (src[i] == '#') {
            while (src[i] && src[i] != '\n') i++;
            continue;
        }
        break;
    }
    return i;
}

int compile_one(const char *src, int *pi) {
    int i = *pi;
    i = skip_ws_and_comments(src, i);
    if (!src[i]) {
        *pi = i;
        return 0; // no more instructions
    }

    int idx = i;
    char c = src[i++];

    /* SIMPLE OPS: 1,2,3,4,7,9,A,B,E */
    if (strchr("123479ABE", c)) {
        add_instr_entry(idx);
        emit8((uint8_t)c);
        *pi = i;
        return 1;
    }

    /* NUMERIC ARG OPS: 0,+,-,J,Z,N,6 */
    if (strchr("0+-JZN6", c)) {
        if (src[i++] != '(') {
            printf("syntax error: expected '(' after %c at %d\n", c, idx);
            exit(1);
        }
        char buf[32] = {0};
        int p = 0;
        while (src[i] && src[i] != ')') {
            buf[p++] = src[i++];
        }
        if (!src[i]) {
            printf("unclosed numeric arg after %c at %d\n", c, idx);
            exit(1);
        }
        i++; // skip ')'
        int val = atoi(buf);

        if (c == '6') {
            // Repeat the NEXT instruction val times at compile time
            int after = skip_ws_and_comments(src, i);
            if (!src[after]) {
                printf("6(%d) at end of file (no next instruction)\n", val);
                exit(1);
            }
            int tmp = after;
            for (int r = 0; r < val; r++) {
                int local = tmp;
                if (!compile_one(src, &local)) {
                    printf("6(%d) could not compile next instruction\n", val);
                    exit(1);
                }
                // all repeats use the same source index for jumps if needed
            }
            // advance main pointer past that one instruction
            int dummy = after;
            compile_one(src, &dummy); // compile once more to know where it ends
            i = dummy;
            *pi = i;
            return 1; // 6 itself is not an instruction
        }

        // real numeric-arg instruction
        add_instr_entry(idx);
        emit8((uint8_t)c);

        if (c == 'J' || c == 'Z' || c == 'N') {
            // store fixup with raw source index
            if (fix_count >= (int)(sizeof(fixups)/sizeof(fixups[0]))) {
                printf("too many fixups\n");
                exit(1);
            }
            fixups[fix_count].bc_pos    = bc_len;
            fixups[fix_count].raw_index = val;
            fix_count++;
            emit32(0); // placeholder
        } else {
            emit32(val);
        }

        *pi = i;
        return 1;
    }

    /* HEX/BIN/C: 5(hex), 8(bin), C(hex-byte) */
    if (c == '5' || c == '8' || c == 'C') {
        if (src[i++] != '(') {
            printf("syntax error: expected '(' after %c at %d\n", c, idx);
            exit(1);
        }
        char buf[256] = {0};
        int p = 0;
        while (src[i] && src[i] != ')') {
            buf[p++] = src[i++];
        }
        if (!src[i]) {
            printf("unclosed literal after %c at %d\n", c, idx);
            exit(1);
        }
        i++; // skip ')'

        add_instr_entry(idx);
        emit8((uint8_t)c);

        if (c == '5') {
            int len = (int)strlen(buf) / 2;
            emit32(len);
            for (int k = 0; k < len; k++) {
                char byte[3] = { buf[k*2], buf[k*2+1], 0 };
                emit8((uint8_t)strtoul(byte, NULL, 16));
            }
        } else if (c == '8') {
            emit8((uint8_t)strtoul(buf, NULL, 2));
        } else { // 'C'
            emit8((uint8_t)strtoul(buf, NULL, 16));
        }

        *pi = i;
        return 1;
    }

    printf("unknown token '%c' at %d\n", c, idx);
    exit(1);
}

/* ---------------- FULL COMPILER ---------------- */

void compile(const char *src) {
    int i = 0;
    while (1) {
        int before = i;
        if (!compile_one(src, &i)) break;
        if (i == before) break;
    }

    /* Resolve jumps: raw source index -> nearest instruction >= that index */
    for (int f = 0; f < fix_count; f++) {
        int raw = fixups[f].raw_index;
        int target_bc = -1;
        for (int j = 0; j < instr_count; j++) {
            if (instr_src_idx[j] >= raw) {
                target_bc = instr_bc_pos[j];
                break;
            }
        }
        if (target_bc < 0) {
            printf("bad jump target raw index %d\n", raw);
            exit(1);
        }
        memcpy(&bytecode[fixups[f].bc_pos], &target_bc, 4);
    }
}

/* ---------------- VM ---------------- */

void run() {
    int ip = 0;

    while (ip < bc_len) {
        uint8_t op = bytecode[ip++];

        switch (op) {

            case '1': {
                int len = (int)strlen(tape[ptr]);
                if (len < STR_MAX) {
                    tape[ptr][len]   = '1';
                    tape[ptr][len+1] = 0;
                }
            } break;

            case '2':
                ptr = (ptr + 1) % TAPE_SIZE;
                break;

            case '3':
                strcpy(tape[ptr], "0.0");
                break;

            case '4': {
                char buf[256];
                if (fgets(buf, sizeof(buf), stdin))
                    buf[strcspn(buf, "\n")] = 0;
                else
                    buf[0] = 0;
                strncpy(tape[ptr], buf, STR_MAX);
                tape[ptr][STR_MAX] = 0;
            } break;

            case '5': {
                int len;
                memcpy(&len, &bytecode[ip], 4);
                ip += 4;
                for (int k = 0; k < len; k++)
                    putchar(bytecode[ip++]);
            } break;

            case '7': {
                int len = (int)strlen(tape[ptr]);
                if (len < STR_MAX) {
                    tape[ptr][len]   = '0' + (rand() % 10);
                    tape[ptr][len+1] = 0;
                }
            } break;

            case '8':
                putchar(bytecode[ip++]);
                break;

            case '9':
                for (int t = 0; t < TAPE_SIZE; t++)
                    printf("%s\n", tape[t]);
                return;

            case 'A':
                ptr = (ptr + 1) % TAPE_SIZE;
                break;

            case 'B':
                ptr = (ptr - 1 + TAPE_SIZE) % TAPE_SIZE;
                break;

            case 'E':
                regv = (tape[ptr][0] == 0) ? 0 : 1;
                break;

            case '0':
                memcpy(&regv, &bytecode[ip], 4);
                ip += 4;
                break;

            case '+': {
                int v;
                memcpy(&v, &bytecode[ip], 4);
                ip += 4;
                regv += v;
            } break;

            case '-': {
                int v;
                memcpy(&v, &bytecode[ip], 4);
                ip += 4;
                regv -= v;
            } break;

            case 'C': {
                unsigned want = bytecode[ip++];
                unsigned got  = (unsigned char)tape[ptr][0];
                regv = (want == got) ? 1 : 0;
            } break;

            case 'J': {
                int target;
                memcpy(&target, &bytecode[ip], 4);
                ip = target;
            } break;

            case 'Z': {
                int target;
                memcpy(&target, &bytecode[ip], 4);
                ip += 4;
                if (regv == 0) ip = target;
            } break;

            case 'N': {
                int target;
                memcpy(&target, &bytecode[ip], 4);
                ip += 4;
                if (regv != 0) ip = target;
            } break;

            default:
                printf("bad opcode %c\n", op);
                exit(1);
        }
    }
}

/* ---------------- MAIN ---------------- */

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: %s program.naf\n", argv[0]);
        return 1;
    }

    srand((unsigned)time(NULL));

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("open");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *src = malloc(sz + 1);
    if (!src) {
        printf("oom\n");
        fclose(f);
        return 1;
    }

    fread(src, 1, sz, f);
    src[sz] = 0;
    fclose(f);

    compile(src);
    run();

    free(src);
    return 0;
}
