#ifndef FS_H
#define FS_H

#define FS_MAX_NAME 32
#define FS_FILE     1
#define FS_DIR      2

typedef struct fs_node {
    char name[FS_MAX_NAME];
    int type;                  // FS_FILE or FS_DIR
    char* content;             // Data content (for files)
    unsigned int size;         // Size of content in bytes
    unsigned int capacity;     // Allocated capacity in bytes
    struct fs_node* parent;    // Parent directory node
    struct fs_node* next;      // Next sibling in the same folder
    struct fs_node* children;  // First child (for directories)
} fs_node_t;

// Initialize the filesystem, creating the root directory "/"
void fs_init(void);

// Getters and setters for root and current working directory (CWD)
fs_node_t* fs_get_root(void);
fs_node_t* fs_get_cwd(void);
void fs_set_cwd(fs_node_t* node);

// Node operations
fs_node_t* fs_create_file(fs_node_t* parent, const char* name);
fs_node_t* fs_create_dir(fs_node_t* parent, const char* name);
fs_node_t* fs_find_child(fs_node_t* parent, const char* name);
fs_node_t* fs_resolve_path(const char* path);

// Writing to file (overwriting or appending)
int fs_write_file(fs_node_t* file, const char* content, int append);

// Helper to construct absolute path string of a node
void fs_get_absolute_path(fs_node_t* node, char* buffer);

#endif
