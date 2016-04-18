#include "initramfs.h"

#include <stdint.h>
#include "memory_map.h"
#include "utilities.h"
#include "string.h"
#include "print.h"
#include "memory.h"
#include "uart.h"
#include "file_system.h"

extern const uint32_t mboot_info;

struct module_descriptor
{
	uint32_t mod_start;
	uint32_t mod_end;
	uint32_t string;
	uint32_t reserved;
};

void load_module(struct module_descriptor module)
{
	uint8_t *address = (uint8_t*) (uint64_t) va(module.mod_start);
	while ((uint64_t) address + sizeof(struct cpio_header) <= (uint64_t) va(module.mod_end))
	{
		address = align(address, CPIO_ALIGNMENT);
		struct cpio_header *header = (struct cpio_header*) address;
		uint32_t file_name_length = convert_from_hex(header->namesize, CPIO_ARRAY_LENGTH);
		uint32_t file_size = convert_from_hex(header->filesize, CPIO_ARRAY_LENGTH);
		address += sizeof(struct cpio_header);
		if (strncmp((const char*) address, END_OF_ARCHIVE, strlen(END_OF_ARCHIVE)) == 0)
			break;
		static char file_name[FILE_NAME_LENGTH];
		strncpy(file_name, (const char*) address, file_name_length);
		file_name[file_name_length] = 0;
		address += file_name_length;
		address = align(address, CPIO_ALIGNMENT);
		uint32_t mode = convert_from_hex(header->mode, CPIO_ARRAY_LENGTH);
		if (S_ISDIR(mode))
			mkdir(file_name);
		if (S_ISREG(mode))
		{
		 	struct file_descriptor *file = open(file_name, O_CREAT | O_WRITE);
		 	write(file, address, file_size);
		 	close(file);
		}
		address += file_size;
	}
}

void init_initramfs()
{
	uint64_t mboot_info_va = (uint64_t) va(*(uint64_t*) va((uint64_t) &mboot_info));
	uint32_t flags = *(uint64_t*) (uint64_t) (mboot_info_va + FLAGS_OFFSET);
	if (!get_bit(flags, MODS_BIT))
	{
		printf("No available modules\n");
		return;
	}
	uint32_t mods_count = *(uint64_t*) (uint64_t) (mboot_info + MODS_COUNT_OFFSET);
	uint64_t mods_addr = (uint64_t) va(*(uint64_t*) (uint64_t) (mboot_info + MODS_ADDR_OFFSET));
	struct module_descriptor* module_descriptor_addr = (struct module_descriptor*) (uint64_t) mods_addr;
	for (int i = 0; i < (int) mods_count; i++) 
	{
		if (module_descriptor_addr[i].mod_end - module_descriptor_addr[i].mod_start >= sizeof(struct cpio_header)
			&& strncmp(((struct cpio_header*) (uint64_t) va(module_descriptor_addr[i].mod_start))->magic, MAGIC, strlen(MAGIC)) == 0)
		{
			load_module(module_descriptor_addr[i]);
			mark_as_reserved(module_descriptor_addr[i].mod_start, module_descriptor_addr[i].mod_end);									
		}	
	}
} 