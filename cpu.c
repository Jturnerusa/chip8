#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define MEM_SIZE 4096
#define GFX_W 64
#define GFX_H 32
#define NUM_REGISTERS 16
#define STACK_SIZE 16
#define NUM_KEYS 16
#define FONTSET_LOCATION 0

struct cpu {	
	int keys[NUM_KEYS];
	int gfx[GFX_W][GFX_H];
	uint8_t mem[MEM_SIZE];
	uint8_t v[NUM_REGISTERS];
	uint16_t stack[STACK_SIZE];
	int dt;
	int st;
	uint8_t sp;
	uint16_t i;
	uint16_t pc;
	uint16_t opcode;
	unsigned int cycle_count;
};

int get_nnn(struct cpu * const c8) {	
	//The lowest 12 bits of the instruction.
	return c8->opcode & 0x0FFF;
}

int get_n(struct cpu * const c8) {
	//The lowest 4 bits of the instruction.
	return c8->opcode & 0x000F;
}

int get_x(struct cpu * const c8) {
	//The lower 4 bits of the high byte of the instruction.
	return (c8->opcode & 0x0F00) >> 8;
}

int get_y(struct cpu * const c8) {
	//The upper 4 bits of the low byte of the instruction.
	return (c8->opcode & 0x00F0) >> 4;
}

int get_kk(struct cpu * const c8) {
	//The lowest 8 bits of the instruction.
	return c8->opcode & 0x00FF;
}

//Opcodes start here

void CLS(struct cpu * const c8) {
	//Clear the display.
	int x;
	int y;
	for (x = 0; x < GFX_W; x++) {
		for (y = 0; y < GFX_H; y++) {
			c8->gfx[x][y] = 0;
		}
	}
	c8->pc += 2;
}

void RET(struct cpu * const c8) {
	//Returns from subroutine.
	if (c8->sp - 1 >= STACK_SIZE) {
		printf("Stack overflow.");
		exit(EXIT_FAILURE);
	}
	c8->pc = c8->stack[c8->sp--];
}

void JP_addr(struct cpu * const c8) {
	//Jump to location nnn.
	c8->pc = get_nnn(c8);
}

void CALL_addr(struct cpu * const c8) {
	//Call subroutine at location nnn.
	if (c8->sp + 1 >= STACK_SIZE) {
		printf("Stack overflow.");
		exit(EXIT_FAILURE);
	}
	c8->stack[++c8->sp] = c8->pc;
	c8->pc = get_nnn(c8);
}

void SE_Vx_byte(struct cpu * const c8) {
	//Skip instruction if Vx = kk.
	if (c8->v[get_x(c8)] == get_kk(c8)) {
		c8->pc += 2;
	}
	c8->pc += 2;
}

void SNE_Vx_byte(struct cpu * const c8) {
	//Skip instruction if Vx != kk.
	if (c8->v[get_x(c8)] != get_kk(c8)) {
		c8->pc += 2;
	}
	c8->pc += 2;
}

void SE_Vx_Vy(struct cpu * const c8) {
	//Skip next instruction if Vx = Vy
	if (c8->v[get_x(c8)] == c8->v[get_y(c8)]) {
		c8->pc += 2;
	}
	c8->pc += 2;
}

void LD_Vx_byte(struct cpu * const c8) {
	//Set Vx = kk.
	c8->v[get_x(c8)] = get_kk(c8);
	c8->pc += 2;
}

void ADD_Vx_byte(struct cpu * const c8) {
	//Set Vx = Vx + kk.
	c8->v[get_x(c8)] += get_kk(c8);
	c8->pc += 2;
}

void LD_Vx_Vy(struct cpu * const c8) {	
	//Set Vx = Vy.
	c8->v[get_x(c8)] = c8->v[get_y(c8)];
	c8->pc += 2;
}

void OR_Vx_Vy(struct cpu * const c8) {
	//Set Vx = Vx OR Vy.
	c8->v[get_x(c8)] |= c8->v[get_y(c8)];
	c8->pc += 2;
}

void AND_Vx_Vy(struct cpu * const c8) {
	//Set Vx = Vx AND Vy.
	c8->v[get_x(c8)] &= c8->v[get_y(c8)];
	c8->pc += 2;
}

void XOR_Vx_Vy(struct cpu * const c8) {
	//Set Vx = Vx XOR Vy.
	c8->v[get_x(c8)] ^= c8->v[get_y(c8)];
	c8->pc += 2;
}

void ADD_Vx_Vy(struct cpu * const c8) {
	//Set VF carry = 1 if Vx + Vy > 255 else 0. Set Vx = Vx + Vy & 0xFF.
	if (c8->v[get_x(c8)] + c8->v[get_y(c8)] > 255) {
		c8->v[15] = 1;
	}
	else {
		c8->v[15] = 0;
	}
	c8->v[get_x(c8)] += c8->v[get_y(c8)] & 0xFF;
	c8->pc += 2;
}

void SUB_Vx_Vy(struct cpu * const c8) {
	//If Vx > Vy set VF = 1 else 0. Set Vx = Vx - Vy.
	if (c8->v[get_x(c8)] > c8->v[get_y(c8)]) {
		c8->v[15] = 1;
	} 
	else {
		c8->v[15] = 0;
	}
	c8->v[get_x(c8)] -= c8->v[get_y(c8)];
	c8->pc += 2;
}

void SHR_Vx(struct cpu * const c8) {
	//If Vx's LSB = 1 set VF = 1 else 0. Set Vx = Vx / 2.
	if (c8->v[get_x(c8)] & 1) {
		c8->v[15] = 1;
	}
	else {
		c8->v[15] = 0;
	}
	c8->v[get_x(c8)] /= 2;
	c8->pc += 2;
}

void SUBN_Vx_Vy(struct cpu * const c8) {
	//If Vy > Vx set VF = 1 else 0. Set Vx = Vy - Vx.
	if (c8->v[get_y(c8)] > c8->v[get_x(c8)]) {
		c8->v[15] = 1;
	}
	else {
		c8->v[15] = 0;
	}
	c8->v[get_x(c8)] = c8->v[get_y(c8)] - c8->v[get_x(c8)];
	c8->pc += 2;
}

void SHL_Vx(struct cpu * const c8) {
	//If Vx's MSB is 1 set VF = 1 else 0. Set Vx = Vx * 2.
	if (c8->v[get_x(c8)] >> 7) {
		c8->v[15] = 1;
	}
	else {
		c8->v[15] = 0;
	}
	c8->v[get_x(c8)] *= 2;
	c8->pc += 2;
}

void SNE_Vx_Vy(struct cpu * const c8) {
	//Skip next instruction if Vx != Vy.
	if (c8->v[get_x(c8)] != c8->v[get_y(c8)]) {
		c8->pc += 2;
	}
	c8->pc += 2;
}

void LD_I_addr(struct cpu * const c8) {
	//Set I = nnn.
	c8->i = get_nnn(c8);
	c8->pc += 2;
}

void JP_V0_addr(struct cpu * const c8) {
	//Jump to location nnn + V0.
	c8->pc = get_nnn(c8) + c8->v[0];
}

void RND_Vx_byte(struct cpu * const c8) {
	//Set Vx = random byte AND kk.
	c8->v[get_x(c8)] = (rand() % 255) & get_kk(c8);
	c8->pc += 2;
}

void DRW_Vx_Vy_nibble(struct cpu * const c8) {
	/*
	*Display n-byte sprite starting at memory location I at (Vx, Vy),
	*set VF 1 if collision.
	*/
	int i;
	int i2;
	int byte;
	int pixel;
	int collision = 0; 
	int x = c8->v[get_x(c8)];
	int y = c8->v[get_y(c8)];
	//Check to see if the following loop can go out of bounds.
	if (c8->i + get_n(c8) >= MEM_SIZE) {
		printf("Read memory out of bounds during sprite draw.");
		exit(EXIT_FAILURE);
	}
	/*
	*First we fetch each byte that makes up our sprite. Then we Extract each 
	*bit of the byte that we fetched, this bit is a pixel. Then we check for
	*collisions by seeing if drawing the pixel will erase any current pixels. 
	*Finally we XOR the pixel to the corresponding location in the GFX memory.
	*/
	for (i = 0; i < get_n(c8); i++) {
		byte = c8->mem[i + c8->i];
		for (i2 = 0; i2 < 8; i2++) {
			pixel = (byte >> (7 - i2)) & 1;
			if (c8->gfx[(x + i2) % GFX_W][(y + i) % GFX_H] ^ pixel) {
				collision = 1;
			}
			c8->gfx[(x + i2) % GFX_W][(y + i) % GFX_H] ^= pixel;
		}
	}
	if (collision) {
		c8->v[15] = 1;
	}
	else {
		c8->v[15] = 0;
	}
	c8->pc += 2;
}

void SKP_Vx(struct cpu * const c8) {
	//Skip instruction if key with the value of Vx is pressed.
	if (c8->keys[c8->v[get_x(c8)]]) {
		c8->pc += 2;
	}
	c8->pc += 2;
}

void SKNP_Vx(struct cpu * const c8) {
	//Skip instruction if key with the value of Vx is not pressed.
	if (!c8->keys[c8->v[get_x(c8)]]) {
		c8->pc += 2;
	}
	c8->pc += 2;
}

void LD_Vx_DT(struct cpu * const c8) {
	//Set Vx = dt.
	c8->v[get_x(c8)] = c8->dt;
	c8->pc += 2;
}

void LD_Vx_K(struct cpu * const c8) {
	//Wait for a key press, store the value of the key in Vx.
	int i;
	for (i = 0; i < NUM_KEYS; i++) {
		if (c8->keys[i]) {
			c8->v[get_x(c8)] = i;
			c8->pc += 2;
		}
	}
}

void LD_DT_Vx(struct cpu * const c8) {
	//Set dt = Vx.
	c8->dt = c8->v[get_x(c8)];
	c8->pc += 2;
}

void LD_ST_Vx(struct cpu * const c8) {
	//Set st = Vx.
	c8->st = c8->v[get_x(c8)];
	c8->pc += 2;
}

void ADD_I_Vx(struct cpu * const c8) {
	//Set I = I + Vx.
	c8->i += c8->v[get_x(c8)];
	c8->pc += 2;
}

void LD_F_Vx(struct cpu * const c8) {
	//Set I = location for sprite for Vx.
	/*
	*Since each sprite is 5 bytes long we multiply by five,
	*then offset that by the starting location of the fontset
	*in memory.
	*/
	c8->i = (c8->v[get_x(c8)] * 5) + FONTSET_LOCATION;
	c8->pc += 2;
}

void LD_B_Vx(struct cpu * const c8) {
	//Store the BCD representation of Vx in memory locations I, I+1, I+2.
	if (c8->i + 2 >= MEM_SIZE) {
		printf("Wrote to memory out of bounds during opcode LD_B_Vx(0xFx33).");
		exit(EXIT_FAILURE);
	}
	c8->mem[c8->i] = (c8->v[get_x(c8)] / 100) % 10;
	c8->mem[c8->i + 1] = (c8->v[get_x(c8)] / 10) % 10;
	c8->mem[c8->i + 2] = c8->v[get_x(c8)] % 10;
	c8->pc += 2;
}

void LD_I_Vx(struct cpu * const c8) {
	//Store registers V0 through Vx in memory starting at location I.
	int i;
	if (c8->i + get_x(c8) >= MEM_SIZE) {
		printf("Wrote to memory out of bounds during opcode LD_I_Vx(Fx55)");
		exit(EXIT_FAILURE);
	}
	for (i = 0; i <= get_x(c8); i++) {
		c8->mem[i + c8->i] = c8->v[i];
	}
	c8->pc += 2;
}

void LD_Vx_I(struct cpu * const c8) {
	//Read registers V0 through Vx from memory starting at location I.
	int i;
	if (c8->i + get_x(c8) >= MEM_SIZE) {
		printf("Read memory out of bounds during opcode LD_Vx_I(Fx65)");
		exit(EXIT_FAILURE);
	}
	for (i = 0; i <= get_x(c8); i++) {
		c8->v[i] = c8->mem[i + c8->i];
	}
	c8->pc += 2;
}

//Opcodes end here.

void fetch_opcode(struct cpu * const c8) {
	if (c8->pc + 1 >= MEM_SIZE) {
		printf("PC overflow.");
		exit(EXIT_FAILURE);
	}
	c8->opcode = c8->mem[c8->pc];
	c8->opcode <<= 8;
	c8->opcode |= c8->mem[c8->pc + 1];
}

void execute_opcode(struct cpu * const c8) {
	switch(c8->opcode & 0xF000) {
		case 0x0:
			switch (c8->opcode & 0x00FF) {
				case 0xE0:
					CLS(c8);
					break;
				case 0xEE:
					RET(c8);
					break;
				default:
					printf("Unknown opcode %X at memory location %i\n",
						   c8->opcode, c8->pc
						   );
			}
			break;
		case 0x1000:
			JP_addr(c8);
			break;
		case 0x2000:
			CALL_addr(c8);
			break;
		case 0x3000:
			SE_Vx_byte(c8);
			break;
		case 0x4000:
			SNE_Vx_byte(c8);
			break;
		case 0x5000:
			SE_Vx_Vy(c8);
			break;
		case 0x6000:
			LD_Vx_byte(c8);
			break;
		case 0x7000:
			ADD_Vx_byte(c8);
			break;
		case 0x8000:
			switch(c8->opcode & 0x000F) {
				case 0x0:
					LD_Vx_Vy(c8);
					break;
				case 0x1:
					OR_Vx_Vy(c8);
					break;
				case 0x2:
					AND_Vx_Vy(c8);
					break;
				case 0x3:
					XOR_Vx_Vy(c8);
					break;
				case 0x4:
					ADD_Vx_Vy(c8);
					break;
				case 0x5:
					SUB_Vx_Vy(c8);
					break;
				case 0x6:
					SHR_Vx(c8);
					break;
				case 0x7:
					SUBN_Vx_Vy(c8);
					break;
				case 0xE:
					SHL_Vx(c8);
					break;
				default:
					printf("Unknown opcode %X at memory location %i\n", 
						   c8->opcode, c8->pc
						   );
			}
			break;
		case 0x9000:
			SNE_Vx_Vy(c8);
			break;
		case 0xA000:
			LD_I_addr(c8);
			break;
		case 0xB000:
			JP_V0_addr(c8);
			break;
		case 0xC000:
			RND_Vx_byte(c8);
			break;
		case 0xD000:
			DRW_Vx_Vy_nibble(c8);
			break;
		case 0xE000:
			switch(c8->opcode & 0x00FF) {
				case 0x9E:
					SKP_Vx(c8);
					break;
				case 0xA1:
					SKNP_Vx(c8);
					break;
				default:
					printf("Unknown opcode %X at memory location %i\n", 
						   c8->opcode, c8->pc
						   );
			}
			break;
		case 0xF000:
			switch(c8->opcode & 0x00FF) {
				case 0x07:
					LD_Vx_DT(c8);
					break;
				case 0x0A:
					LD_Vx_K(c8);
					break;
				case 0x15:
					LD_DT_Vx(c8);
					break;
				case 0x18:
					LD_ST_Vx(c8);
					break;
				case 0x1E:
					ADD_I_Vx(c8);
					break;
				case 0x29:
					LD_F_Vx(c8);
					break;
				case 0x33:
					LD_B_Vx(c8);
					break;
				case 0x55:
					LD_I_Vx(c8);
					break;
				case 0x65:
					LD_Vx_I(c8);
					break;
				default:
					printf("Unknown opcode %X at memory location %i\n", 
						   c8->opcode, c8->pc
						   );
			}
			break;
		default:
			printf("Unknown opcode %X at memory location %i\n", 
				   c8->opcode, c8->pc
				   );
	}
}

void reset_keys(struct cpu * const c8) {
	int i;
	for (i = 0; i < NUM_KEYS; i++) {
		c8->keys[i] = 0;
	}
}

void decrement_timers(struct cpu * const c8) {
	if (c8->dt > 0) {
		c8->dt--;
	}
	if (c8->st > 0) {
		c8->st--;
	}
}

void emulate_cycle(struct cpu * const c8) {
	fetch_opcode(c8);
	execute_opcode(c8);
	reset_keys(c8);
	decrement_timers(c8);
	c8->cycle_count++;
}

void load_fontset(struct cpu * const c8) {
	int i;
	int fontset[80] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};
	for (i = 0; i < 80; i++) {
		c8->mem[i + FONTSET_LOCATION] = fontset[i];
	}
}

void load_program(struct cpu * const c8, FILE * const rom) {
	int i;
	uint8_t buffer[MEM_SIZE - c8->pc];
	fread(buffer, sizeof(buffer[0]), sizeof(buffer) / sizeof(buffer[0]), rom);
	if ferror(rom) {
		printf("Failed while reading from ROM file.");
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < MEM_SIZE - c8->pc; i++) {
		c8->mem[i + c8->pc] = buffer[i];
	}
}

static int initialized = 0;

struct cpu * init_cpu(FILE * const rom) {
	int i;
	struct cpu * const c8 = malloc(sizeof(struct cpu));
	if (!initialized) {
		initialized = 1;
		srand(time(NULL));
	}
	if (c8 == NULL) {
		printf("Failed to allocate memory while initializing cpu.");
		exit(EXIT_FAILURE);
	}
	//Set default values for everything.
	reset_keys(c8);
	CLS(c8);
	for (i = 0; i < MEM_SIZE; i++) {
		c8->mem[i]= 0;
	}
	for (i = 0; i < NUM_REGISTERS; i++) {
		c8->v[i] = 0;
	}
	for (i = 0; i < STACK_SIZE; i++) {
		c8->stack[i] = 0;
	}
	c8->pc = 512;
	c8->dt = 0;
	c8->st = 0;
	c8->sp = 0;
	c8->i = 0;
	c8->opcode = 0;
	c8->cycle_count = 0;
	load_fontset(c8);
	load_program(c8, rom);
	return c8;
}

void dealloc_cpu(struct cpu * c8) {
	free(c8);
}

void set_key(struct cpu * const c8, int element) {
	if (element >= NUM_KEYS) {
		printf("Key out of range.");
		exit(EXIT_FAILURE);
	}
	c8->keys[element] = 1;
}

int get_gfx_w() {
	return GFX_W;
}

int get_gfx_h() {
	return GFX_H;
}

int get_pixel(struct cpu * const c8, int x, int y) {
	if (x * y >= GFX_W * GFX_H) {
		printf("Read GFX out of bounds.");
		exit(EXIT_FAILURE);
	}
	return c8->gfx[x][y];
}