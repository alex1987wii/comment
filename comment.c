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
#include <string.h>
void usage(void)
{
	printf("Usage:comment [OPTIONS] File\n");
	printf("OPTIONS:\n");
	printf("        -h help\n");
	printf("        -f=[ao],append or override.\n");
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
int main(int argc,char *argv[])
{
	if(argc < 2)
		usage();
	unsigned int opt = 0;
	const char *args_for_f = NULL;
	const char *args_for_i = NULL;
	const char *args_for_o = NULL;
	const char *args_for_F = NULL;
	const char *args_for_l = NULL;
	const char *args_for_c = NULL;
	const char *args_for_L = NULL;
	int ch;
	while((ch = getopt(argc,argv,"+hf:i:o:F:l:c:dDL:")) != -1)
	{
		switch(ch)
		{
			case 'h':
				opt |= OPT_h;
				break;
			case 'f':
				opt |= OPT_f;
				args_for_f = optarg;
				break;
			case 'i':
				opt |= OPT_i;
				args_for_i = optarg;
				break;
			case 'o':
				opt |= OPT_o;
				args_for_o = optarg;
				break;
			case 'F':
				opt |= OPT_F;
				args_for_F = optarg;
				break;
			case 'l':
				opt |= OPT_l;
				args_for_l = optarg;
				break;
			case 'c':
				opt |= OPT_c;
				args_for_c = optarg;
				break;
			case 'd':
				opt |= OPT_d;
				break;
			case 'D':
				opt |= OPT_D;
				break;
			case 'L':
				opt |= OPT_L;
				args_for_L = optarg;
				break;
			default:
				usage();
				break;
		}
	}


	return 0;
}
