/***********************************************
*	Filename    :comment.h
*	Author      :alex
*	Date	    :Sun Nov 25 14:28:07 2018
*	Purpose     :
*	Description :
*
***********************************************/

#ifndef	COMMENT_H
#define	COMMENT_H
#ifdef _WIN32
#include <windows.h>

#elif defined(__linux__)

#include <stdio.h>
#include <unistd.h>

#else
#error "only support windows and linux OS"
#endif
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_COMMENT_LEN		2048
#define MAX_BUFF_LEN        4096 
#define MAX_LINE_LEN		2048
#define MAX_NAME_LEN		256
#define MAX_LIST            64

#define TMPFILE     ".tmp.swp"

#define MAX_STACK		1023
typedef struct _stack_t {
	int length;
	int elem[MAX_STACK];
}stack_t;

inline void push(struct _stack_t *stack,int value)
{
	stack->elem[stack->length++] = value;
}
inline int pop(struct _stack_t *stack)
{
	if(stack->length <= 0){
		printf("stack overflow!\n");
		exit(-1);
	}
	stack->length--;
	return stack->elem[stack->length];
}
inline int peek(struct _stack_t *stack)
{
	assert(stack->length > 0);
	return stack->elem[stack->length-1];
}

typedef enum {
	LANG_C,
	LANG_CPP,
	LANG_JAVA,
	LANG_PYTHON
}lang_t;

#if 0
typedef struct _file_desc_t{
	lang_t lang;
	char *comment;
}file_desc_t;
struct _file_desc_t *get_file_desc(const char *file);
#endif


#define FUNC_INLINE		0x00000001
#define FUNC_STATIC		0x00000002
#define FUNC_DECLARE    0x00000004
#define FUNC_DEFINITION	0x00000008
#define FUNC_NORETURN	0x00000010
#define FUNC_INTERRUPT	0x00000020
#define FUNC_TYPEDEF	0x00000040
#define FUNC_CONST		0x00000080

typedef struct _func_desc_t{
	int Flags;
    int func_index;
    int argc;
	char parameter[MAX_LIST][MAX_NAME_LEN];
}func_desc_t;
int get_func_desc(const char *func,struct _func_desc_t *line);
static inline int is_valid_func(const struct _func_desc_t *func_desc)
{
	return (func_desc->func_index > 0 && !(func_desc->Flags & FUNC_TYPEDEF));
}

int get_file_comment(const char *filename,char *comment,int bufsize,int comment_char);
int get_func_comment(const struct _func_desc_t *func_desc,char *comment,int bufsize,int comment_char);

typedef enum {
	OPT_i = 0x00000001,
	OPT_s = 0x00000002,
	OPT_d = 0x00000004,
	OPT_D = 0x00000008,
	OPT_h = 0x00000010,
	OPT_f = 0x00000020,
	OPT_L = 0x00000040,
	OPT_c = 0x00000080,
	OPT_l = 0x00000100,
	OPT_F = 0x00000200,
	OPT_o = 0x00000400,
    OPT_u = 0x00000800,
    OPT_MAX=OPT_u,
}OPT;
#define DEFAULT_OPT     (OPT_D | OPT_s)
#define OPT_FILTER		(OPT_i | OPT_s | OPT_d | OPT_D)
#define OPT_FORCE_O     (OPT_MAX<<1)


#define BRACKT1_LEFT        1
#define BRACKT2_LEFT        2
#define BRACKT3_LEFT        3
#define COMMENT_LEFT        4

#define BRACKT1_RIGHT       11
#define BRACKT2_RIGHT       12
#define BRACKT3_RIGHT       13
#define COMMENT_RIGHT       14

#define SCOPE_GLOBAL		0
#define SCOPE_BRACKT1		1
#define SCOPE_BRACKT2		2
#define SCOPE_BRACKT3		4
#define SCOPE_DMARK			8
#define SCOPE_SMARK			16
#define SCOPE_COMMENT_LINE	32
#define SCOPE_COMMENT		64

#endif


