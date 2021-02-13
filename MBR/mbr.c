#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

struct part {
		uint8_t boot;
		uint8_t CHS1[3];
		uint8_t type;
		uint8_t CHS2[3];
		uint32_t lba;
		uint32_t size;
}__attribute__((packed));

struct mbr {
		uint8_t boot[446];
		struct part p[4];
		uint16_t sig;
};

int main (int argc, char* argv[]){
		int fd;
		struct stat st;
		char* ptr;
		struct mbr* mbr;
		int i;
		fd = open(argv[1],O_RDWR);
		fstat (fd, &st);//sizeof_file
		ptr = (char*)mmap(NULL, st.st_size, PROT_WRITE, MAP_SHARED, fd, 0);

		mbr = (struct mbr*)ptr;
		mbr->sig = 0xaa55;
		for (i = 0; i < 4; i++){
			mbr->p[i].lba = 1 + (st.st_size/512 - 1)/9*i;
			mbr->p[i].size = (st.st_size/512 - 1)/9;
			mbr->p[i].type = 0xb;
			if (i == 3) {
				mbr->p[i].type = 0x5;
				mbr->p[i].size = (st.st_size/512 - 1)/9*6;

			}
		}

		for  (i = 0; i < 6; i++){
			mbr = (struct mbr*)(ptr + (1 + (st.st_size/512 - 1)/9*3)*512 
							+ (1+(st.st_size/512 - 1)/9)*i*512);
			mbr->sig = 0xaa55;
			mbr->p[0].lba = 1;
			mbr->p[0].size = (st.st_size/512 - 1)/9;
			mbr->p[0].type = 0xb;
			mbr->p[1].lba = (1 + (st.st_size/512 - 1)/9)*(i+1);
			mbr->p[1].size = (st.st_size/512 - 1)/9;
			mbr->p[1].type = 0x5;

			if (i == 5) {
				mbr->p[1].lba = 0;
				mbr->p[1].size = 0;
				mbr->p[1].type = 0;
			}
		}

		
		munmap(ptr, st.st_size);
		close(fd);
}
