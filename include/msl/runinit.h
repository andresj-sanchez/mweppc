#ifndef MSL_RUNINIT_H
#define MSL_RUNINIT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*init_function)(void);
struct runinit_table {
    unsigned long length;
    init_function functions[1];
};
void RunInit(struct runinit_table* table);
int __detect_cpu_instruction_set(void);

#ifdef __cplusplus
}
#endif

#endif
