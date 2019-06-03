#ifndef CPU_H_
#define CPU_H_

struct cpu;

int get_gfx_w();

int get_gfx_h();

int get_pixel(struct cpu * const c8, int x, int y);

void set_key(struct cpu * const c8, int element);

void emulate_cycle(struct cpu * const c8);

struct cpu * init_cpu(FILE * const rom_ptr);

void dealloc_cpu(struct cpu * c8);

#endif