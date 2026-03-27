#include "idt/keyboard/command.h"
#include "mem/pmm.h"
// Make sure you have included the header for malloc/free here!

void prepare_for_next_command() {
    // If there is a previously allocated command from getInput(), free its memory
    if (input.command != nullptr) {
        free(input.command); 
        input.command = nullptr;
    }
    
    input.len = 0;
    
    // Print the prompt
    kprintf("MythOS>");
    
    // Lock the input boundary so the user cannot backspace over the prompt
    terminal.set_input_limit();  
}

void sysCommand(const char *command) {
    if (command == NULL || command[0] == '\0') {
        return;    
    }

    

    //-------------------------------FINDING ARGS------------------------------------------
    char input_buffer_temp[MAX_INPUT_LEN];
    strcpy(input_buffer_temp, command);
    
    char (*args)[MAX_COMMAND_LEN] = (char (*)[MAX_COMMAND_LEN]) pmm_alloc_blocks(2);
    for(int i = 0; i < MAX_COMMAND_ARGS; i++) args[i][0] = '\0';

    uint8_t num_args = 0;
    char* token = strtok(input_buffer_temp, ' ');
    
    while (token != NULL) {
        if (num_args >= MAX_COMMAND_ARGS) {
            kprintf("Error: Too many arguments.\n");
            pmm_free_blocks(args, 8);
            return;
        }
        strcpy(args[num_args++], token);
        token = strtok(NULL, ' ');
    }

    if (!(hash(args[0])==hash("fault") || hash(args[0])==hash("check") && hash(args[1])==hash("logs"))) 
        klog("%s(%s)", __func__, command);

    if (num_args <= 0) {
        pmm_free_blocks(args, 2);
        return;
    }
    
    remove_cursor_shape();
    draw_cursor = false;
    switch (execute_command(args)) {
        case 0: kprintf("%s: command not found\n", args[0]); break;
        case 2: kprintf("%s: not enough parameters\nTry 'help %s' for more information\n", args[0], args[0]); break;
    } 
    
    pmm_free_blocks(args, 2);
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