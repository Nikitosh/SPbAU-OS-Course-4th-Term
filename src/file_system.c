#include "file_system.h"

#include "string.h"
#include "buddy_allocator.h"
#include "slab_allocator.h"
#include "memory.h"
#include "print.h"
#include "uart.h"

static struct slab_allocator* file_struct_allocator;
static struct slab_allocator* file_descriptor_struct_allocator;
static struct file root_directory;

void init_file(struct file *file, file_type type, const char *name)
{
	file->type = type;
	strncpy(file->name, name, strlen(name));	
	list_init(&file->list_node);
	init_lock(&file->lock);
	file->size = 0;
	file->size_level = 0;
	file->data = NULL;
	file->children = NULL;
}

void init_regular(struct file *regular, const char *name)
{
	init_file(regular, REGULAR, name);
	regular->size_level = 0;
	regular->data = allocate_page(regular->size_level);
}

void init_directory(struct file *directory, const char *name)
{
	init_file(directory, DIRECTORY, name);
}

void init_file_descriptor(struct file_descriptor *descriptor, struct file *file, uint64_t offset, uint32_t flags)
{
	descriptor->file = file;
	descriptor->offset = offset;
	descriptor->flags = flags;	
}

void add_to_directory(struct file *directory, struct file *file)
{
	if (directory->children == NULL)
		directory->children = &file->list_node;
	else
		list_add_tail(&file->list_node, directory->children);
}

int contains_directory(const char *path)
{
	int len = strlen(path);
	for (int i = 0; i < len; i++)
		if (path[i] == SEPARATOR)
			return i;
	return 0;
}

struct file* get_file(struct file *directory, const char *path, uint32_t directory_name_length, file_type type)
{
	lock(&directory->lock);
	if (!directory->children)
	{
		unlock(&directory->lock);
		return NULL;
	}
	struct list_head *first = directory->children;
	struct list_head *current = first;
	do
	{
		struct file *file = LIST_ENTRY(current, struct file, list_node);
		if (file->type == type && strncmp(file->name, path, directory_name_length) == 0)
		{
			unlock(&directory->lock);
			return file;	
		}
		current = current->next;
	} while (current != first);
	unlock(&directory->lock);
	return NULL;
}

struct file* get_terminal_directory(const char **path)
{
	struct file *current_directory = &root_directory;
	while (1)
	{
		int directory_name_length = contains_directory(*path);
		if (directory_name_length == 0)
			break;
		current_directory = get_file(current_directory, *path, directory_name_length, DIRECTORY);
	    if (current_directory == NULL)
			return NULL;
	    *path += directory_name_length + 1;
	}
	return current_directory;
}

int resize_file(struct file *file, uint64_t size)
{
	if (size > (uint64_t) SMALL_PAGE_SIZE * (1ll << (file->size_level - 1)) && size <= (uint64_t) SMALL_PAGE_SIZE * (1ll << (file->size_level)))
	{
		file->size = size;
		return 1;
   	}
   	uint32_t size_level = get_size_level(size);
   	uint8_t *data = allocate_page(size_level);
   	if (data == NULL)
   		return 0;
   	for (uint64_t i = 0; i < min(size, file->size); i++)
   		data[i] = file->data[i];
   	free_page(file->data, file->size_level);
   	file->data = data;
   	file->size_level = size_level;
   	file->size = size;
   	return 1;
}

void init_file_system()
{
    file_struct_allocator = (struct slab_allocator*) create_slab_allocator(sizeof(struct file), 1);
    file_descriptor_struct_allocator = (struct slab_allocator*) create_slab_allocator(sizeof(struct file_descriptor), 1);
    init_directory(&root_directory, "");
}

struct file_descriptor* open(const char *path, uint32_t flags)
{
	struct file *current_directory = get_terminal_directory(&path);
	if (!current_directory)
		return NULL;
	struct file *file = get_file(current_directory, path, strlen(path), REGULAR);
	if (file == NULL)
	{
    	if (!(flags & O_CREAT))
			return NULL;
		file = slab_allocator_allocate(file_struct_allocator);
		init_regular(file, path);
		add_to_directory(current_directory, file);
	}
	if (flags & O_TRUNC)
		resize_file(file, 0);
	struct file_descriptor *descriptor = slab_allocator_allocate(file_descriptor_struct_allocator);
	init_file_descriptor(descriptor, file, 0, flags);
	return descriptor;
}

void close(struct file_descriptor* descriptor)
{
	slab_allocator_free(descriptor);
}

uint64_t read(struct file_descriptor *descriptor, void *buf, uint64_t size)
{
	if (!(descriptor->flags & O_READ)) 
	{
		printf("File is not available for reading\n");
		return 0;
	}
	lock(&descriptor->file->lock);
	uint64_t result = 0;
	uint8_t *buffer = buf;
	while (result < size && descriptor->offset < descriptor->file->size)
		buffer[result++] = descriptor->file->data[descriptor->offset++];
	unlock(&descriptor->file->lock);
	return result;
}

uint64_t write(struct file_descriptor *descriptor, const void *buf, uint64_t size)
{
	if (!(descriptor->flags & O_WRITE)) 
	{
		printf("File is not available for writing\n");
		return 0;
	}
	lock(&descriptor->file->lock);
	struct file *file = descriptor->file;
	uint64_t old_size = file->size;
	uint64_t new_size = size;
	if (descriptor->flags & O_APPEND)
		new_size = file->size + size;
	int size_level = get_size_level(new_size);
	for (; size_level >= 0; size_level--)
	{
		if (resize_file(file, min(new_size, SMALL_PAGE_SIZE * 1ull << size_level)) == 1)
		{
		    new_size = min(new_size, SMALL_PAGE_SIZE * 1ull << size_level);
			break;
		}
	}
	const uint8_t *buffer = buf;
	if (descriptor->flags & O_APPEND)
	{
		if (new_size <= old_size)
		{
			unlock(&descriptor->file->lock);
			return 0;
		}
		for (uint64_t i = 0; i < new_size - old_size; i++)
			file->data[old_size + i] = buffer[i];			
		unlock(&descriptor->file->lock);
		return new_size - old_size;
	}
	for (uint64_t i = 0; i < new_size; i++)
		file->data[i] = buffer[i];
	unlock(&descriptor->file->lock);
	return new_size;
}

int mkdir(const char *path)
{
	struct file *current_directory = get_terminal_directory(&path);
	if (!current_directory)
		return -1;
	struct file *file = get_file(current_directory, path, strlen(path), DIRECTORY);
	if (file != NULL)
		return -1;
	file = slab_allocator_allocate(file_struct_allocator);
	init_directory(file, path);
	add_to_directory(current_directory, file);
	return 0;	
}

struct list_head* readdir(const char *path)
{
	struct file *directory = get_terminal_directory(&path);
	if (!directory)
		return NULL;
   	return directory->children;
}

void print_directory(struct file *directory, int offset)
{
	for (int i = 0; i < offset; i++)
		printf(" ");
	puts(directory->name);
	printf("\n");
	struct list_head *first = directory->children;
	if (first == NULL)
		return;
	struct list_head *current = first;
	do
	{
		print_directory(LIST_ENTRY(current, struct file, list_node), offset + 2);
		current = current->next;
	} while (current != first);
}

void print_file_system()
{
	print_directory(&root_directory, 0);
}
