
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

#define DEFAULT_CHAR_COLOR	0x07	/* 0000 0111 黑底白字 */
#define RED	0x04	/* 0000 0111 黑底hong字 */
PRIVATE int esc;
PRIVATE int escState;
PRIVATE char store[100];
PRIVATE int strIndex;
PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_do_read(TTY* p_tty);
PRIVATE void tty_do_write(TTY* p_tty);
PRIVATE void put_key(TTY* p_tty, u32 key);

/* include/console.h
 CONSOLE 
typedef struct s_console
{
	unsigned int	current_start_addr;	/当前显示到了什么位置	  
	unsigned int	original_addr;		 当前控制台对应显存位置 
	unsigned int	v_mem_limit;		 当前控制台占的显存大小 
	unsigned int	cursor;			 当前光标位置 
}CONSOLE;
*/

/* include/tty.h
typedef struct s_tty
{
	u32	in_buf[TTY_IN_BYTES];	/TTY 输入缓冲区 
	u32*	p_inbuf_head;		 指向缓冲区中下一个空闲位置 
	u32*	p_inbuf_tail;		 指向键盘任务应处理的键值 
	int	inbuf_count;		 缓冲区中已经填充了多少 

	struct s_console *	p_console;
}TTY;*/
/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty()
{
	TTY*	p_tty;
	esc = 0;
	escState = 1;
	strIndex = 0;
	init_keyboard();

	for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
		init_tty(p_tty);
	}
	select_console(0);
	while (1) {
		for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
			tty_do_read(p_tty);
			tty_do_write(p_tty);
		}
	}
}

/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY* p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	init_screen(p_tty);
}

/*======================================================================*
				in_process
		        用终端p_tty来显示 keyboard_read()解析后的扫描码 
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty, u32 key)
{
        char output[2] = {'\0', '\0'};
		if((key & MASK_RAW) == ESC){
			if(esc == 0){
	             // 奇数次按esc，表示准备查找
				//enter ESC	
				// disp_str("jinru esc");
				esc = 1;
			}
			else{
				// 偶数次按esc，表示结束查找
				//clear 
						for(int i=0;i<strIndex;i++){
							put_key(p_tty,'\b');
						}
						strIndex = 0;
						for(int i=0;i<100;i++){
							store[i] = '\0';
						}
				clear(p_tty->p_console);
				esc = 0;
				escState = 1;
			}
		}
		else if(escState == 1){
			// disp_str("123");
				if (!(key & FLAG_EXT)) {
				put_key(p_tty, key);
		}
        else {
                int raw_code = key & MASK_RAW;
                switch(raw_code) {
                case ENTER:
					if(esc == 1){
						store[strIndex] = '\0';
						search(p_tty->p_console,store);
						
						escState = 0;
					}
					else{
					put_key(p_tty, '\n');
					}
					break;
                case BACKSPACE:
					put_key(p_tty, '\b');
						break;
				case TAB:
					put_key(p_tty,'\t');
					break;
                case UP:
                        if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
				scroll_screen(p_tty->p_console, SCR_DN);
                        }
			break;
		case DOWN:
			if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
				scroll_screen(p_tty->p_console, SCR_UP);
			}
			break;
		case F1:
		case F2:
		case F3:
		case F4:
		case F5:
		case F6:
		case F7:
		case F8:
		case F9:
		case F10:
		case F11:
		case F12:
			/* Alt + F1~F12 */
			if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
				select_console(raw_code - F1);
			}
			break;
                default:
                        break;
                }
        }
}
}

/*======================================================================*
			      put_key
				  用key初始化p_tty->p_inbuf_head++
*======================================================================*/
PRIVATE void put_key(TTY* p_tty, u32 key)
{
	if (p_tty->inbuf_count < TTY_IN_BYTES) {
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}
}


/*======================================================================*
			      tty_do_read
 *======================================================================*/
PRIVATE void tty_do_read(TTY* p_tty)
{
	if (is_current_console(p_tty->p_console)) {
		keyboard_read(p_tty);
	}
}


/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY* p_tty)
{
	if (p_tty->inbuf_count) {
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;
	
        // 根据esc判断终端字符颜色
		if(esc == 1){
			out_char(p_tty->p_console, ch,RED);
			store[strIndex++] = ch;
		}
		else{
			out_char(p_tty->p_console,ch,DEFAULT_CHAR_COLOR);
		}
		
	}
}


