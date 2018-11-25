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

#define MAX_COMMENT_LEN		2048
#define MAX_NAME_LEN		256

typedef enum {
	LANG_C,
	LANG_CPP,
	LANG_JAVA,
	LANG_PHONY
}lang_t;

typedef struct _file_desc_t{
	lang_t lang;
	char *comment;
}file_desc_t;
struct _file_desc_t *get_file_desc(const char *file);

#define FUNC_CONST		0x00000001
#define FUNC_STATIC		0x00000002
#define FUNC_INLINE		0x00000004
#define FUNC_NORETURN	0x00000008
#define FUNC_INTERRUPT	0x00000010

typedef struct _func_desc_t{
	int func_type;
	char **name_list;
}func_desc_t;
struct _func_desc_t *get_func_desc(const char *func);


#endif
