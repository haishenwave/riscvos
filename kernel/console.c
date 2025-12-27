void uart_putc(char c);

void console_putc(char c){
    if(c=='\n') uart_putc('\r');
    uart_putc(c);
}

void console_puts(const char* s){
    for(; *s; s++) console_putc(*s);
}

void clear_screen(void){
    console_puts("\x1b[2J\x1b[H");  //clean and move to lefttop
}