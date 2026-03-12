#include "idt/keyboard/command.h"
#include "mem/pmm.h"

void prepare_for_next_command() {

    strinit(input_buffer,MAX_INPUT_LEN); 
    input_len=0;
    input_pos=0;


    kprintf("MythOS>");        
    terminal_set_input_limit();  
}


void sysCommand(const char *command) {
    if (command == NULL || command[0] == '\0') return;

    //-------------------------------SPECIAL CASES------------------------------------------
    if (strcmp(command, "reboot") == 0) {
        kprintf("Restarting...\n");
        outb(0x64, 0xFE); 
    }


    //-------------------------------FINDING ARGS------------------------------------------
    char input_buffer_temp[MAX_INPUT_LEN];
    strcpy(input_buffer_temp, command);
    
    char (*args)[MAX_COMMAND_LEN] = (char (*)[MAX_COMMAND_LEN]) pmm_alloc_blocks(8);
    for(int i=0; i < MAX_COMMAND_ARGS; i++) args[i][0] = '\0';

    uint8_t num_args=0;
    char* token = strtok(input_buffer_temp, ' ');
    
    while (token != NULL) {
        if (num_args>= MAX_COMMAND_ARGS) {
            kprintf("Error: Too many arguments.\n");
            return;
        }
        strcpy(args[num_args++], token);
        token = strtok(NULL, ' ');
    }

    if (num_args<=0) {
        pmm_free_blocks(args, 8);
        return;
    }
    switch (execute_command(args)) {

        case 0: kprintf("%s: command not found\n", args[0]); break;
        case 2: kprintf("%s: not enough parameters\n", args[0]); break;


    } 
    
    pmm_free_blocks(args, 8);
}

void sysCommandAt(const char *command, uint16_t x, uint16_t y) {

    uint16_t tempx = terminal_data.cursor_col;
    uint16_t tempy = terminal_data.cursor_row;
    terminal_set_cursor(&terminal_data, x, y);
    sysCommand(command);
    terminal_set_cursor(&terminal_data, tempx, tempy);


}

void command_handler() {
    input_buffer[input_len] = '\0'; 

    if (input_len > 0) {
       
        for (int i = MAX_HISTORY - 1; i > 0; i--) {
            for (int j = 0; j < MAX_INPUT_LEN; j++) {
                history[i][j] = history[i-1][j];
            }
        }
        strcpy(history[0], input_buffer);
        if (history_count < MAX_HISTORY) history_count++;
    }
    
    history_index = -1; 

   
    sysCommand(input_buffer);
}

void refresh_command_line() {
    
    while (input_pos < input_len) {
        terminal_data.cursor_col++;
        column_behaviour(&terminal_data);
        input_pos++;
    }

    while (input_len > 0) {
        terminal_putchar('\b'); 
        input_len--;
        input_pos--;            
    }

    if (history_index == -1) {
        input_buffer[0] = '\0';
        return;
    }

    for (int i = 0; history[history_index][i] != '\0'; i++) {
        input_buffer[i] = history[history_index][i];
        terminal_putchar(input_buffer[i]); 
        input_len++;
        input_pos++; 
    }
    
    // Chiudi correttamente la stringa
    input_buffer[input_len] = '\0'; 
}
