#ifndef __FS_H__
#define __FS_H__

#include <stdint.h>
#include "list.h"
#include "lock.h"
#include "utilities.h"

#define SEPARATOR         '/'
#define FILE_NAME_LENGTH  256

typedef enum 
{
	REGULAR,
	DIRECTORY
} file_type;

typedef enum
{
	O_READ =   bit(0),
	O_WRITE =  bit(1),
	O_APPEND = bit(2),
	O_CREAT  = bit(3),
	O_TRUNC  = bit(4)
} flags;

struct file
{
	file_type type;
	char name[FILE_NAME_LENGTH];
	struct list_head list_node;
	struct spinlock lock;

	uint64_t size;
	uint8_t *data;
	int size_level;

	struct list_head *children;
} __attribute((packed))__;

struct file_descriptor
{
	struct file *file;
	uint64_t offset;
	uint32_t flags;
};

void init_file_system();
struct file_descriptor* open(const char *path, uint32_t flags);
void close(struct file_descriptor* descriptor);
uint64_t read(struct file_descriptor *descriptor, void *buf, uint64_t size);
uint64_t write(struct file_descriptor *descriptor, const void *buf, uint64_t size);
int mkdir(const char *path);
struct list_head* readdir(const char *path);
void print_file_system();

#endif /* __FS_H__ */