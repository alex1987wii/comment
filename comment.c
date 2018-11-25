/***********************************************
*	Filename    :comment.c
*	Author      :alex
*	Date	    :Sun Nov 25 14:35:36 2018
*	Purpose     :
*	Description :
*
***********************************************/
#include "comment.h"
#include <stdlib.h>
void usage(void)
{
	printf("Usage:comment [OPTIONS] File\n");
	printf("OPTIONS:\n");
	printf("        --help\n");
	printf("        -f,--force=[ao],append or override.\n");
	printf("        -i Filelist(split by comma).\n");
	printf("        -o Filename,output filename.\n");
	printf("        -F Functionlist(split by comma)\n");
	printf("        -l Linelist(split by comma)\n");
	printf("        -c comment_char,such as @ !,as a prefix for comment.\n");
	printf("        -d comment on declaration.\n");
	printf("        -D comment on definition(default).\n");
	printf("        -L language,default is c.\n");
	exit(EXIT_SUCCESS);
}
int main(int argc,const char *argv[])
{
	if(argc < 2)
		usage();


	return 0;
}
