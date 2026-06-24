#include "ports.h"
#include "screen.h"
#include "keyboard.h"
#include "string.h"
#include "mem.h"
#include "fs.h"

// Reboot CPU via keyboard controller CPU reset line
void reboot() {
    print_string("Resetting CPU via 8042 PS/2 controller...\n");
    for (volatile int i = 0; i < 0x2000000; i++); // Short busy-wait delay
    port_byte_out(0x64, 0xFE);                   // Pulse CPU reset line
}

// Utility to trim whitespace from a string
void trim(char* str) {
    int len = strlen(str);
    if (len == 0) return;
    
    int start = 0;
    while (str[start] == ' ' || str[start] == '\t') {
        start++;
    }
    
    int end = len - 1;
    while (end >= start && (str[end] == ' ' || str[end] == '\t' || str[end] == '\r' || str[end] == '\n')) {
        end--;
    }
    
    int i;
    for (i = start; i <= end; i++) {
        str[i - start] = str[i];
    }
    str[i - start] = '\0';
}

// Parse and run shell commands in C
void process_command(const char* cmd) {
    char cmd_part[64];
    char file_part[64];
    int redirect_type = 0; // 0 = none, 1 = '>', 2 = '>>'
    
    int len = strlen(cmd);
    int redirect_idx = -1;
    for (int i = 0; i < len; i++) {
        if (cmd[i] == '>') {
            redirect_idx = i;
            if (cmd[i+1] == '>') {
                redirect_type = 2; // >>
            } else {
                redirect_type = 1; // >
            }
            break;
        }
    }
    
    if (redirect_idx != -1) {
        strncpy(cmd_part, cmd, redirect_idx);
        cmd_part[redirect_idx] = '\0';
        
        const char* file_src = cmd + redirect_idx + (redirect_type == 2 ? 2 : 1);
        strcpy(file_part, file_src);
        
        trim(cmd_part);
        trim(file_part);
    } else {
        strcpy(cmd_part, cmd);
        trim(cmd_part);
    }
    
    // Redirection processing
    if (redirect_type > 0) {
        if (strncmp(cmd_part, "echo ", 5) == 0 || strcmp(cmd_part, "echo") == 0) {
            const char* text = "";
            if (strncmp(cmd_part, "echo ", 5) == 0) {
                text = cmd_part + 5;
            }
            char clean_text[64];
            strcpy(clean_text, text);
            trim(clean_text);
            
            int clean_len = strlen(clean_text);
            if (clean_len >= 2 && clean_text[0] == '"' && clean_text[clean_len - 1] == '"') {
                for (int j = 1; j < clean_len - 1; j++) {
                    clean_text[j - 1] = clean_text[j];
                }
                clean_text[clean_len - 2] = '\0';
            }
            
            // Resolve or create file
            fs_node_t* file_node = fs_resolve_path(file_part);
            if (!file_node) {
                char parent_path[64];
                char file_name[32];
                int last_slash = -1;
                int file_part_len = strlen(file_part);
                for (int j = 0; j < file_part_len; j++) {
                    if (file_part[j] == '/') {
                        last_slash = j;
                    }
                }
                
                fs_node_t* parent_dir = 0;
                if (last_slash == -1) {
                    parent_dir = fs_get_cwd();
                    strcpy(file_name, file_part);
                } else {
                    if (last_slash == 0) {
                        parent_dir = fs_get_root();
                    } else {
                        strncpy(parent_path, file_part, last_slash);
                        parent_path[last_slash] = '\0';
                        parent_dir = fs_resolve_path(parent_path);
                    }
                    strcpy(file_name, file_part + last_slash + 1);
                }
                
                if (!parent_dir || parent_dir->type != FS_DIR) {
                    print_string("Error: parent directory does not exist.\n");
                    return;
                }
                
                file_node = fs_create_file(parent_dir, file_name);
                if (!file_node) {
                    print_string("Error: failed to create file.\n");
                    return;
                }
            }
            
            int append = (redirect_type == 2);
            if (fs_write_file(file_node, clean_text, append) < 0) {
                print_string("Error: failed to write to file.\n");
            }
        } else {
            print_string("Error: redirection is only supported for 'echo' command.\n");
        }
        return;
    }
    
    // Command execution
    if (strcmp(cmd_part, "help") == 0) {
        print_string("Available Protected Mode commands:\n");
        print_string("  help             - Show this bare-metal command list\n");
        print_string("  info             - Display raw processor state information\n");
        print_string("  clear            - Clear screen (direct VGA memory write)\n");
        print_string("  exit / reboot    - Pulse reset line to reboot computer\n");
        print_string("  ls               - List directory contents\n");
        print_string("  pwd              - Print current directory path\n");
        print_string("  mkdir <dir>      - Create a new directory\n");
        print_string("  cd <dir>         - Change current directory\n");
        print_string("  cat <file>       - Display contents of a file\n");
        print_string("  echo <text>      - Print text to screen\n");
        print_string("  echo <text> > f  - Redirect text to file (overwrite)\n");
        print_string("  echo <text> >> f - Redirect text to file (append)\n");
    } else if (strcmp(cmd_part, "info") == 0) {
        print_string("System State Information (32-bit PMODE C-Kernel):\n");
        print_string("  CPU Mode: 32-bit Protected Mode (compiled in C)\n");
        print_string("  Video RAM: 0xB8000 (direct VGA Text Buffer)\n");
        print_string("  Keyboard driver: direct port I/O 0x60/0x64 (polling)\n");
        print_string("  Memory Allocator: dynamic heap initialized at 1MB\n");
        print_string("  Filesystem: RAM-based filesystem (RAMFS) active\n");
    } else if (strcmp(cmd_part, "clear") == 0) {
        clear_screen();
    } else if (strcmp(cmd_part, "reboot") == 0 || strcmp(cmd_part, "exit") == 0) {
        reboot();
    } else if (strcmp(cmd_part, "pwd") == 0) {
        char path_buffer[128];
        fs_get_absolute_path(fs_get_cwd(), path_buffer);
        print_string(path_buffer);
        print_string("\n");
    } else if (strcmp(cmd_part, "ls") == 0) {
        fs_node_t* cwd = fs_get_cwd();
        fs_node_t* child = cwd->children;
        if (!child) {
            print_string("(directory is empty)\n");
        } else {
            while (child) {
                if (child->type == FS_DIR) {
                    print_string("  [DIR] ");
                    print_string(child->name);
                    print_string("\n");
                } else {
                    print_string("  [FIL] ");
                    print_string(child->name);
                    print_string(" (");
                    char size_str[16];
                    itoa(child->size, size_str);
                    print_string(size_str);
                    print_string(" bytes)\n");
                }
                child = child->next;
            }
        }
    } else if (strncmp(cmd_part, "mkdir ", 6) == 0) {
        const char* dir_name = cmd_part + 6;
        char parent_path[64];
        char name[32];
        int last_slash = -1;
        int dir_len = strlen(dir_name);
        for (int j = 0; j < dir_len; j++) {
            if (dir_name[j] == '/') {
                last_slash = j;
            }
        }
        
        fs_node_t* parent_dir = 0;
        if (last_slash == -1) {
            parent_dir = fs_get_cwd();
            strcpy(name, dir_name);
        } else {
            if (last_slash == 0) {
                parent_dir = fs_get_root();
            } else {
                strncpy(parent_path, dir_name, last_slash);
                parent_path[last_slash] = '\0';
                parent_dir = fs_resolve_path(parent_path);
            }
            strcpy(name, dir_name + last_slash + 1);
        }
        
        if (!parent_dir) {
            print_string("Error: parent directory does not exist.\n");
        } else {
            fs_node_t* new_dir = fs_create_dir(parent_dir, name);
            if (!new_dir) {
                print_string("Error: failed to create directory.\n");
            }
        }
    } else if (strcmp(cmd_part, "cd") == 0) {
        fs_set_cwd(fs_get_root());
    } else if (strncmp(cmd_part, "cd ", 3) == 0) {
        const char* path = cmd_part + 3;
        fs_node_t* target = fs_resolve_path(path);
        if (!target) {
            print_string("Error: directory not found.\n");
        } else if (target->type != FS_DIR) {
            print_string("Error: path is not a directory.\n");
        } else {
            fs_set_cwd(target);
        }
    } else if (strncmp(cmd_part, "cat ", 4) == 0) {
        const char* path = cmd_part + 4;
        fs_node_t* target = fs_resolve_path(path);
        if (!target) {
            print_string("Error: file not found.\n");
        } else if (target->type != FS_FILE) {
            print_string("Error: path is a directory.\n");
        } else {
            if (target->content && target->size > 0) {
                print_string(target->content);
                print_string("\n");
            } else {
                print_string("(file is empty)\n");
            }
        }
    } else if (strcmp(cmd_part, "echo") == 0) {
        print_string("\n");
    } else if (strncmp(cmd_part, "echo ", 5) == 0) {
        const char* text = cmd_part + 5;
        char clean_text[64];
        strcpy(clean_text, text);
        trim(clean_text);
        
        int clean_len = strlen(clean_text);
        if (clean_len >= 2 && clean_text[0] == '"' && clean_text[clean_len - 1] == '"') {
            for (int j = 1; j < clean_len - 1; j++) {
                clean_text[j - 1] = clean_text[j];
            }
            clean_text[clean_len - 2] = '\0';
        }
        print_string(clean_text);
        print_string("\n");
    } else {
        print_string("Error: Command not found in C PMODE. Type 'help'.\n");
    }
}

// C Entry Point
void kernel_main() {
    mem_init();
    fs_init();
    
    clear_screen();
    print_string("=========================================\n");
    print_string("   Antigravity Kernel Space (32-bit C)   \n");
    print_string("   Direct Hardware Access - BIOS Disabled\n");
    print_string("=========================================\n");
    print_string("Type 'help' for bare-metal C commands.\n\n");
    
    char input_buffer[64];
    char path_buffer[128];
    while (1) {
        fs_get_absolute_path(fs_get_cwd(), path_buffer);
        print_string("kernel# ");
        print_string(path_buffer);
        print_string("> ");
        read_line(input_buffer);
        if (input_buffer[0] != '\0') {
            process_command(input_buffer);
        }
    }
}
