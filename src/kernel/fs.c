#include "fs.h"
#include "mem.h"
#include "string.h"

static fs_node_t* root_node = 0;
static fs_node_t* cwd_node = 0;

void fs_init(void) {
    root_node = (fs_node_t*)malloc(sizeof(fs_node_t));
    if (root_node) {
        strcpy(root_node->name, "/");
        root_node->type = FS_DIR;
        root_node->content = 0;
        root_node->size = 0;
        root_node->capacity = 0;
        root_node->parent = 0;
        root_node->next = 0;
        root_node->children = 0;
        
        cwd_node = root_node;
    }
}

fs_node_t* fs_get_root(void) {
    return root_node;
}

fs_node_t* fs_get_cwd(void) {
    return cwd_node;
}

void fs_set_cwd(fs_node_t* node) {
    if (node && node->type == FS_DIR) {
        cwd_node = node;
    }
}

static fs_node_t* fs_create_node(fs_node_t* parent, const char* name, int type) {
    if (!parent || parent->type != FS_DIR) return 0;
    
    unsigned int name_len = strlen(name);
    if (name_len == 0 || name_len >= FS_MAX_NAME) return 0;
    
    // Check if a child with this name already exists
    if (fs_find_child(parent, name)) return 0;
    
    fs_node_t* node = (fs_node_t*)malloc(sizeof(fs_node_t));
    if (!node) return 0;
    
    strcpy(node->name, name);
    node->type = type;
    node->content = 0;
    node->size = 0;
    node->capacity = 0;
    node->parent = parent;
    node->next = 0;
    node->children = 0;
    
    // Add to parent's children list
    if (!parent->children) {
        parent->children = node;
    } else {
        fs_node_t* curr = parent->children;
        while (curr->next) {
            curr = curr->next;
        }
        curr->next = node;
    }
    
    return node;
}

fs_node_t* fs_create_file(fs_node_t* parent, const char* name) {
    return fs_create_node(parent, name, FS_FILE);
}

fs_node_t* fs_create_dir(fs_node_t* parent, const char* name) {
    return fs_create_node(parent, name, FS_DIR);
}

fs_node_t* fs_find_child(fs_node_t* parent, const char* name) {
    if (!parent || parent->type != FS_DIR) return 0;
    
    fs_node_t* curr = parent->children;
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return 0;
}

fs_node_t* fs_resolve_path(const char* path) {
    if (!path || strlen(path) == 0) return cwd_node;
    
    fs_node_t* curr = cwd_node;
    int i = 0;
    
    if (path[0] == '/') {
        curr = root_node;
        i = 1;
    }
    
    char token[FS_MAX_NAME];
    int token_len = 0;
    
    while (path[i] != '\0') {
        if (path[i] == '/') {
            i++;
            continue;
        }
        
        token_len = 0;
        while (path[i] != '\0' && path[i] != '/') {
            if (token_len < FS_MAX_NAME - 1) {
                token[token_len++] = path[i];
            }
            i++;
        }
        token[token_len] = '\0';
        
        if (token_len == 0) continue;
        
        if (strcmp(token, ".") == 0) {
            // Do nothing, current dir
        } else if (strcmp(token, "..") == 0) {
            if (curr->parent) {
                curr = curr->parent;
            }
        } else {
            fs_node_t* child = fs_find_child(curr, token);
            if (!child) {
                return 0; // Path component not found
            }
            curr = child;
        }
    }
    
    return curr;
}

int fs_write_file(fs_node_t* file, const char* content, int append) {
    if (!file || file->type != FS_FILE) return -1;
    
    unsigned int content_len = strlen(content);
    
    if (append) {
        unsigned int new_size = file->size + content_len;
        if (new_size + 1 > file->capacity) {
            unsigned int new_capacity = file->capacity == 0 ? 32 : file->capacity * 2;
            while (new_capacity < new_size + 1) {
                new_capacity *= 2;
            }
            char* new_content = (char*)malloc(new_capacity);
            if (!new_content) return -1;
            
            if (file->content) {
                strcpy(new_content, file->content);
                free(file->content);
            } else {
                new_content[0] = '\0';
            }
            file->content = new_content;
            file->capacity = new_capacity;
        }
        strcpy(file->content + file->size, content);
        file->size = new_size;
    } else {
        if (content_len + 1 > file->capacity) {
            if (file->content) {
                free(file->content);
            }
            unsigned int new_capacity = 32;
            while (new_capacity < content_len + 1) {
                new_capacity *= 2;
            }
            file->content = (char*)malloc(new_capacity);
            if (!file->content) {
                file->size = 0;
                file->capacity = 0;
                return -1;
            }
            file->capacity = new_capacity;
        }
        strcpy(file->content, content);
        file->size = content_len;
    }
    
    return 0;
}

void fs_get_absolute_path(fs_node_t* node, char* buffer) {
    if (!node) {
        strcpy(buffer, "");
        return;
    }
    if (node == root_node) {
        strcpy(buffer, "/");
        return;
    }
    
    fs_node_t* path_nodes[16];
    int depth = 0;
    fs_node_t* curr = node;
    while (curr && curr != root_node && depth < 16) {
        path_nodes[depth++] = curr;
        curr = curr->parent;
    }
    
    buffer[0] = '\0';
    for (int i = depth - 1; i >= 0; i--) {
        strcat(buffer, "/");
        strcat(buffer, path_nodes[i]->name);
    }
}
