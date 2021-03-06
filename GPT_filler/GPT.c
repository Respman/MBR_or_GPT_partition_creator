#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>


struct part {
	uint8_t boot;
	uint8_t CHS1[3];
	uint8_t type;
	uint8_t CHS2[3];
	uint32_t LBA;
	uint32_t size;
}__attribute__((packed));

struct mbr {
	uint8_t boot[446];
	struct part p[4];
	uint16_t sig;
};

struct gpt_header{
	uint8_t signature[8];
	uint32_t revision;
	uint32_t header_size;
	uint32_t header_CRC32;
	uint32_t reserved;
	uint64_t my_LBA;
	uint64_t altemate_LBA;
	uint64_t first_usable_LBA;
	uint64_t last_usable_LBA;
	uint8_t disk_GUID[16];
	uint64_t partition_entry_LBA;
	uint32_t number_of_partition_entries;
	uint32_t size_of_partition_entry;
	uint32_t partition_entry_array_CRC32;
}__attribute__((packed));

struct gpt_partition_table_part {
	uint8_t partition_type_GUID[16];
	uint8_t unique_partition_GUID[16];
	uint64_t starting_LBA;
	uint64_t ending_LBA;
	uint64_t attributes;
	uint8_t partition_name[72];
}__attribute__((packed));



uint32_t CRC32(uint8_t* ptr, uint32_t len){
	uint32_t crc = (uint32_t)(~0);
	uint32_t tmp;
	int k;

	while (len--){
		crc ^= *ptr++;
		for (k = 0; k < 8; k++){
			tmp = crc >> 1;
			crc = tmp^((crc&1) ? (0xEDB88320) : 0);
		}
	}
	return crc^(uint32_t)(~0);
}


int main(int argc, char** argv){
	int fd_1, i, j;
	FILE* fd;
	struct mbr* mbr;
	struct gpt_header* gpt_header, *gpt_header_2;
	struct gpt_partition_table_part* p;
	uint8_t sig_gpt_header[] = {0x45, 0x46, 0x49, 0x20, 0x50, 0x41, 0x52, 0x54};
	uint8_t sig_gpt_GUID[] = {0xAF, 0x3D, 0xC6, 0x0F, 0x83, 0x84, 0x72, 
			0x47, 0x8E, 0x79, 0x3D, 0x69, 0xD8, 0x47, 0x7D, 0xE4};
	uint64_t p_end; 
	uint32_t N;
	struct stat st, st_1;
	int size, size_file, size_tmp;
	char* ptr, *ptr_1;
	
	N = argc - 2;

	srand(time(NULL));

	fd = fopen(argv[1], "wb");


	ptr = (char*)malloc(2*512);
	size = 2*512;

	for(i = 0; i < 2*512; i++) ptr[i] = 0;
	
	mbr = (struct mbr*)ptr;
	mbr->sig = 0xaa55;
	mbr->p[0].LBA = 1;
	mbr->p[0].type = 0xee;
	
	gpt_header = (struct gpt_header*)(ptr + 512);
	gpt_header->revision = 0x00010000;
	gpt_header->header_size = 0x5C;
	gpt_header->header_CRC32 = 0x0;
	gpt_header->reserved = 0x0;
	gpt_header->my_LBA = 0x01;

	for (i = 0; i < 8; i++) gpt_header->signature[i] = sig_gpt_header[i];
	for (i = 0; i < 16; i++) gpt_header->disk_GUID[i] = rand() % 256;
	gpt_header->partition_entry_LBA = 0x02;
	gpt_header->number_of_partition_entries = N;
	gpt_header->size_of_partition_entry = 0x80;
	gpt_header->first_usable_LBA = 2 + (N/4 + ((N%4)? 1 : 0));
	size_file = 0;
	size_tmp = 0;
	ptr = (char*)realloc(ptr, size + 512*(N/4 + ((N%4)? 1 : 0)));
	size += 512*(N/4 + ((N%4)? 1 : 0));
	for (i = 0; i < N; i++){
		p = (struct gpt_partition_table_part*)(ptr + 2*512 + 128*i);
		for (j = 0; j < 16; j++){
			p->partition_type_GUID[j] = sig_gpt_GUID[j];
			p->unique_partition_GUID[j] = rand() % 256;
		}
		
		fd_1 = open(argv[2 + i], O_RDWR);
		fstat (fd_1, &st_1);
		
		size_file += (st_1.st_size/512 + ((st_1.st_size%512)? 1 : 0))*512 ;

		p->starting_LBA = gpt_header->first_usable_LBA + size_tmp/512 + 1;
		p->ending_LBA = gpt_header->first_usable_LBA + size_file/512;
		p->attributes = 0x0;
		
		close(fd_1);
		size_tmp = size_file;
	}

	gpt_header->altemate_LBA = gpt_header->first_usable_LBA + size_file/512 + (N/4 + ((N%4)? 1 : 0));
	gpt_header->last_usable_LBA = gpt_header->first_usable_LBA + size_file/512;
	gpt_header->partition_entry_array_CRC32 = CRC32((ptr + gpt_header->partition_entry_LBA*512), gpt_header->number_of_partition_entries * gpt_header->size_of_partition_entry);
	gpt_header->header_CRC32 = CRC32((ptr + gpt_header->my_LBA * 512), gpt_header->header_size);
	
	mbr->p[0].size = gpt_header->first_usable_LBA + size_file/512 + (N/4 + ((N%4)? 1 : 0));

	fwrite(ptr, 1, size, fd);
	
	for (i = 0; i < N; i++){	
		fd_1 = open(argv[2 + i], O_RDWR);
		fstat (fd_1, &st_1);
		ptr_1 = (char*)mmap(NULL, st_1.st_size, PROT_READ, MAP_SHARED, fd_1, 0);
		fwrite(ptr_1, 1, st_1.st_size, fd);
		munmap(ptr_1, st_1.st_size);
		close(fd_1);
		size_tmp = (st_1.st_size/512 + ((st_1.st_size%512)? 1 : 0))*512 - st_1.st_size;
		ptr_1 = (char*)malloc(size_tmp);
		for (j = 0; j < size_tmp; j++){
			ptr_1[j] = 0;
		}
		fwrite(ptr_1, 1, size_tmp, fd);
		free(ptr_1);

	}

	ptr_1 = (char*)malloc((N/4 + ((N%4)? 1 : 0))*512 + 512);

	for (i = 0; i < size-2*512; i++){
		ptr_1[i] = ptr[2*512 + i];
	}
	
	gpt_header_2 = (struct gpt_header*)(ptr_1 + (N/4 + ((N%4)? 1 : 0))*512);
	gpt_header_2->revision = 0x00010000;
	gpt_header_2->header_size = 0x5C;
	gpt_header_2->header_CRC32 = 0x0;
	gpt_header_2->reserved = 0x0;
	gpt_header_2->altemate_LBA = 0x01;
	gpt_header_2->my_LBA = gpt_header->first_usable_LBA + size_file/512 + (N/4 + ((N%4)? 1 : 0));
	gpt_header_2->first_usable_LBA = 2 + (N/4 + ((N%4)? 1 : 0));
	gpt_header_2->last_usable_LBA = gpt_header->first_usable_LBA + size_file/512;
	for (i = 0; i < 8; i++) gpt_header_2->signature[i] = sig_gpt_header[i];
	for (i = 0; i < 16; i++) gpt_header_2->disk_GUID[i] = gpt_header->disk_GUID[i];
	gpt_header_2->partition_entry_LBA = gpt_header->first_usable_LBA + size_file/512;
	gpt_header_2->number_of_partition_entries = N;
	gpt_header_2->size_of_partition_entry = 0x80;
	gpt_header_2->partition_entry_array_CRC32 = CRC32((ptr_1), gpt_header_2->number_of_partition_entries * gpt_header_2->size_of_partition_entry);
	gpt_header_2->header_CRC32 = CRC32((ptr_1 + (N/4 + ((N%4)? 1 : 0))*512), gpt_header_2->header_size);

	fwrite(ptr_1, 1, (512 +(N/4 + ((N%4)? 1 : 0))*512), fd);

	free(ptr_1);
	free(ptr);
	fclose(fd);

	return 0;
}
