#define DO_TRACE 1

#if DO_TRACE
#define TRACE(x) x
#else
#define TRACE(x)
#endif

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <bitset>

uint8_t* read_prog_to_heap(char* prog_path, int* prog_length){
	FILE* fd = fopen(prog_path, "r");
	if(fd == nullptr) {
		printf("Failed to open file %s\n", prog_path);
		exit(-1);
	}

	fseek(fd, 0x0, 0x2);
	*prog_length = ftell(fd);
	rewind(fd);
	uint8_t* new_mem = (uint8_t*)malloc(*prog_length);
	if(new_mem == nullptr) {
		printf("Failed to allocate new memory !\n");
		exit(-1);
	}

	fread(new_mem, 0x1, 1222, fd);
	fclose(fd);
	return new_mem;
}

int core(uint8_t* program_in_mem_addr, int* program_length){
	uint8_t* next_instruction = program_in_mem_addr;
	uint8_t* new_malloc = (uint8_t*)malloc(0x1000);
	uint8_t* pnew_malloc = new_malloc;
	uint8_t* current_instruction = {0};
	uint8_t* r = {0};

	printf("addr of new_malloc: %p\naddr of pnew_malloc: %p\n", &new_malloc, &pnew_malloc);
	printf("pnew_malloc dereferenced : %p\nvalue at pnew_malloc dereferenced : %d\n", &(*(uint8_t*)pnew_malloc), *(uint8_t*)pnew_malloc);
	printf("new_malloc dereferenced : %p\nvalue at new_malloc dereferenced : %d\n", &(*(uint8_t*)pnew_malloc), *(uint8_t*)new_malloc);
	
	while(true){
		if(program_in_mem_addr > next_instruction){
			printf("Something unexpected happened while running the program!\n");
			exit(-1);
		}

		if(next_instruction > program_in_mem_addr+*program_length){
			printf("Something unexpected happened while running the program!\n");
			exit(-1);
		}	

		current_instruction = next_instruction;
		next_instruction += 1;
		//printf("RUNNING CURRENT OPCODE : %02x\n\n", *current_instruction);
		//getchar();
		if(*current_instruction > 0x28){
			printf("Invalid opcode ! (%d > %d)\n", *current_instruction, 0x28);
			exit(-1);
		}

		switch(*current_instruction){
			case 0:
				break;
			case 1: // stack-2 += stack-1
				TRACE(printf("ADD %02x, %02x\n", *(pnew_malloc - 2), *(pnew_malloc - 1)));
				*(pnew_malloc-2) += *(pnew_malloc-1);
				pnew_malloc -= 1;
				break;
			case 2:
				TRACE(printf("SUB %02x, %02x\n", *(pnew_malloc - 2), *(pnew_malloc - 1)));
				*(pnew_malloc-2) -= *(pnew_malloc-1);
				pnew_malloc -= 1;
				break;
			case 3:
				TRACE(printf("AND %02x, %02x\n", *(pnew_malloc - 2), *(pnew_malloc - 1)));
				*(pnew_malloc-2) &= *(pnew_malloc-1);
				pnew_malloc -= 1;
				break;
			case 4:
				TRACE(printf("OR %02x, %02x\n", *(pnew_malloc - 2), *(pnew_malloc - 1)));
				*(pnew_malloc-2) |= *(pnew_malloc-1);	
				pnew_malloc -= 1;
				break;
			case 5:
				TRACE(printf("XOR %02x, %02x\n", *(pnew_malloc - 2), *(pnew_malloc - 1)));
				*(pnew_malloc-2) ^= *(pnew_malloc-1);
				pnew_malloc -= 1;
				break;
			case 6:
				TRACE(printf("SHL %02x, %02x\n", *(pnew_malloc - 2), *(pnew_malloc - 1)));
				*(pnew_malloc-2) <<= *(pnew_malloc-1);
				pnew_malloc -= 1;
				break;
			case 7:
				TRACE(printf("SHR %02x, %02x\n", *(pnew_malloc - 2), *(pnew_malloc - 1)));
				*(pnew_malloc-2) >>= *(pnew_malloc-1);
				pnew_malloc -= 1;
				break;
			case 8:
				*pnew_malloc = getchar();
				TRACE(printf("GET %02x\n", *(pnew_malloc)));
				pnew_malloc += 1;
				break;
			case 9:
				TRACE(printf("PUT %02x\n", *(pnew_malloc - 1)));
				pnew_malloc -= 1;
				putchar(*pnew_malloc);
				break;
			case 0xa:
				r = next_instruction;
				TRACE(printf("PUSH %02x %c\n", *r, *r));
				next_instruction += 1;
				*pnew_malloc = *r;
				pnew_malloc += 1;
				break;
			case 0xe:
				TRACE(printf("POP? %02x\n", *pnew_malloc));
				pnew_malloc -= 1;
				break;
			default:
				TRACE(printf("INSTRUCTION NOT CODED\n"));
				break;
			/*case 0xb:
				if(*(pnew_malloc - 1) < 0){
					next_instruction = ((next_instruction + *(next_instruction + 1)) | (*next_instruction << 8));
				}
				next_instruction += 2;
				break;
			case 0xc:
				if(*(pnew_malloc - 1) == 0){
					next_instruction = ((next_instruction + (*(next_instruction + 1))) | (*next_instruction << 8));
				}
				next_instruction += 2;
				break;
			case 0xd:
				next_instruction = (((next_instruction + (*(next_instruction+1))) | (*next_instruction << 8)) + 2);
				break;
				*/
		}

		if(new_malloc > pnew_malloc){
			printf("Error: Stack Underflow at 0x%0x4lx\n", (next_instruction - program_in_mem_addr));
			exit(-1);
		}
		if(pnew_malloc > new_malloc + 0x1000){
			printf("Error: Stack Overflow at 0x%0x4lx\n", (program_in_mem_addr - next_instruction));
			exit(-1);
		}
	}


	return 0;

}


int main(int argc, char** argv){
	if(argc != 2){
		printf("Usage : %s <program>\n", argv[0]);
		exit(-1);
	}
	
	int prog_length = 0;
	uint8_t* some_mem = read_prog_to_heap(argv[1], &prog_length);
	
	printf("prot_length after reference modification : %d\n", prog_length);
	printf("some_mem addr : %p\n", &some_mem);
	getchar();
	core(some_mem, &prog_length);
	return 0;
}

