#include "idt/keyboard/command.h"
#include "mem/pmm.h"

void prepare_for_next_command() {
    // If there is a previously allocated command from getInput(), free its memory
    if (input.command != nullptr) {
        free(input.command); 
        input.command = nullptr;
    }
    
    input.len = 0;
    
    // Print the prompt
    vfs_print_prompt();
    
    // Lock the input boundary so the user cannot backspace over the prompt
    terminal.set_input_limit();  
}

int8_t tokenize_command(char* input, int& argc, char** argv) {
    argc = 0;
    char* ptr = input;

    while (*ptr != '\0' && argc < MAX_ARGC) {
        // Skip whitespace
        while (*ptr == ' ') ptr++;
        if (*ptr == '\0') break;

        // Save the start of the token
        if (*ptr == '"') {
            ptr++; // Skip opening quote
            argv[argc++] = ptr;
            while (*ptr != '"' && *ptr != '\0') ptr++;
        } else {
            argv[argc++] = ptr;
            while (*ptr != ' ' && *ptr != '\0') ptr++;
        }

        // Place a null terminator to end the current token
        if (*ptr != '\0') {
            *ptr = '\0';
            ptr++;
        }
    }
    return (argc > 0);
}

void sysCommand(const char *command) {
    // Early return: empty input
    if (command == nullptr || command[0] == '\0') return;

    // 1. Allocate memory for the command copy using PMM
    char* cmd_copy = (char*)pmm_alloc_blocks(1); 
    
    // Early return: Out of memory
    if (cmd_copy == nullptr) {
        kprintf("Error: System out of memory for command parsing.\n");
        return;
    }

    // 2. Prepare pointers and copy data
    int argc = 0;
    char* argv[MAX_ARGC]; 
    strncpy(cmd_copy, command, 4096 - 1); // Use block size limit
    cmd_copy[4095] = '\0';

    // 3. Tokenize in-place (argv will point inside cmd_copy)
    if (!tokenize_command(cmd_copy, argc, argv)) return;


    remove_cursor_shape();
    draw_cursor = false;
        
    // Execute the command
    execute_command(argc, argv);
    

    pmm_free_blocks(cmd_copy, 1);
}

void sysCommandAt(terminal_output_t *t, const char *command, point p) {
    spinlock_acquire(&terminal_lock);
    bool was_visible = cursor_visible;
    remove_cursor_shape();

    terminal_output_t temp = terminal;
    
    terminal = *t;
    terminal.cursor=p;
    terminal.direct = true;
              

    sysCommand(command);

    

    terminal = temp;
    terminal.direct = false;       
    draw_cursor = true;
    terminal_restore_cursor(was_visible);
    spinlock_release(&terminal_lock);
}

void command_handler() {
    // 1. Read the screen and allocate the command string
    char* cmd = getInput();

    // 2. If the user only pressed ENTER without typing, or an error occurred
    if (cmd == nullptr || input.len == 0) {
        prepare_for_next_command();
        return;
    }

    // 3. History Management: Shift older commands down, stopping at index 1
    for (int i = MAX_HISTORY - 1; i > 1; i--) {
        strncpy(history[i], history[i-1], MAX_COMMAND_LEN);
    }
    
    // Copy the newly executed command into slot 1 safely
    strncpy(history[1], cmd, MAX_COMMAND_LEN - 1);
    history[1][MAX_COMMAND_LEN - 1] = '\0';
    
    // Increment history count (leaving slot 0 reserved for the working draft)
    if (history_count < MAX_HISTORY - 1) {
        history_count++;
    }
    
    // Clear the working draft (slot 0) and reset the index pointer
    history[0][0] = '\0';
    history_index = 0; 

    // 4. Execute the command
    sysCommand(cmd);
    
    // 5. Prepare for the next command (this frees the memory allocated for 'cmd')
    prepare_for_next_command();
}

void refresh_command_line() {
    while (input.len > 0) {
        terminal.putchar('\b'); 
        input.len--;
    }

    
    for (int i = 0; history[history_index][i] != '\0'; i++) {
        terminal.putchar(history[history_index][i]); 
        input.len++;
    }
}