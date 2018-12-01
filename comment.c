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
/*these varibles should clear for every file*/
static struct _stack_t *stack = NULL;
static unsigned int scope = SCOPE_GLOBAL;
static unsigned int last_line_type = 0;

static const char *date = NULL;
static const char *user = NULL;

void usage(void)
{
	printf("Usage:comment [OPTIONS] File\n");
	printf("OPTIONS:\n");
	printf("        -h file comment.\n");
	printf("        -f force override the comment.\n");
	printf("        -o Filename,output filename.\n");
	printf("        -F Functionlist(split by comma)\n");
	printf("        -l Linelist(split by comma)\n");
	printf("        -c comment_char,such as @ !,as a prefix for comment.\n");
	printf("        -i comment on inline function.\n");
	printf("        -s comment on static function(default).\n");
	printf("        -d comment on declaration.\n");
	printf("        -D comment on definition(default).\n");
    printf("        -u don't use default options.\n");
	printf("        -L language,default is c.\n");

	exit(EXIT_SUCCESS);
}
int main(int argc,char *argv[])
{
	if(argc < 2)
		usage();

	unsigned int opt = 0;
	const char *args_for_o = NULL;
	const char *args_for_F = "";
	const char *args_for_l = "";
	const char *args_for_c = "=";
	const char *args_for_L = "=c";
	int ch;
	while((ch = getopt(argc,argv,":hfio:F:l:c:sdDuL::")) != -1)
	{
		switch(ch)
		{
			case 'h':
				opt |= OPT_h;
				break;
			case 'f':
				opt |= OPT_f;
				break;
			case 'i':
				opt |= OPT_i;
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
            case 's':
                opt |= OPT_s;
                break;
            case 'u':
                opt |= OPT_u;
                break;
            case ':':
			default:
                usage();
				break;
		}
	}


	/* init stack*/
	stack = (struct _stack_t *)malloc(sizeof(struct _stack_t));
	if(stack == NULL)
	{
		printf("stack malloc failed!\n");
		exit(-1);
	}
#ifndef NDEBUG
    printf("args_for_F = %s\n",args_for_F);
    printf("args_for_l = %s\n",args_for_l);
    printf("args_for_c = %s\n",args_for_c);
    printf("args_for_L = %s\n",args_for_L);
    printf("optopt = %s\n",optopt);
    printf("filelist: ");
    int i;
    for(i = optind; i < argc; ++i)
    {
        printf("%s\t",argv[i]);
        
    }
    printf("\n");
#endif
    /*check args*/
    lang_t lang = LANG_C;
    char comment_char = 0;
    
    if((opt & OPT_u) == 0)
      opt |= DEFAULT_OPT;

    if((opt & OPT_c) && args_for_c)
	{
        if(*args_for_c == '=')
          comment_char = *(args_for_c+1);
        else
          comment_char = *args_for_c;
	}

    if((opt & OPT_L) && args_for_L){
        if(!strcmp(args_for_L,"=java") || !strcmp(args_for_L,"java"))
          lang = LANG_JAVA;
        else if(!strcmp(args_for_L,"=cpp") || !strcmp(args_for_L,"cpp"))
          lang = LANG_CPP;
        else if(!strcmp(args_for_L,"=python") || !strcmp(args_for_L,"python"))
          lang = LANG_PYTHON;
        else if(strcmp(args_for_L,"=c") && strcmp(args_for_L,"c"))
          usage();
    }
    if(lang != LANG_C)
    {
        printf("Only support c language yet!\n");
        exit(EXIT_SUCCESS);
    }
    /* we can do our real work now*/

    int nr_file = argc - optind;
    if(nr_file <= 0)
      usage();
    else if(nr_file > 1 && (opt & (OPT_o | OPT_l)))
    {
        printf("Can't specify the output file or line number for mutiple input!\n");
        exit(EXIT_SUCCESS);
    }
    else if((opt & OPT_l) && (opt &(OPT_F)))
    {
        printf("Can't specify line number and function list at the same time!\n");
        exit(EXIT_SUCCESS);
    }
    char *func_list[MAX_LIST];
    char *line_list[MAX_LIST];

    int nr_func = strip_str_by_comma(args_for_F,MAX_LIST,func_list);
    int nr_line = strip_str_by_comma(args_for_l,MAX_LIST,line_list);
    int line_array[MAX_LIST];
    int nr = parse_number(line_array,line_list,nr_line);
    if(nr != nr_line)
    {
        printf("Line number error!\n");
        usage();
    }
	/*malloc func_desc memory*/
	struct _func_desc_t *func_desc = (struct _func_desc_t *)malloc(sizeof(struct _func_desc_t));
	if(func_desc == NULL)
	{
		printf("out of memory.\n");
		goto QUIT;

	}
	time_t current_time;
	time(&current_time);
	date = asctime(localtime(&current_time));
	user = getlogin();
    /*process file one by one*/
	int i;
    for(i = 0; i < nr_file; ++i)
    {
		char buff[MAX_BUFF_LEN];
		char comment_buff[MAX_COMMENT_LEN];
		char line_buff[MAX_LINE_LEN];
		int  valid_line_index = 0;
          
        FILE *fpin = fopen(argv[optind+i],"r");
        if(fpin == NULL)
        {
            printf("%s open failed!\n",argv[optind+i]);
            continue;
        }
 
        FILE *fptmp = fopen(TMPFILE,"w+");
        if(fptmp == NULL)
        {
            printf("%s create failed!\n",TMPFILE);
            fclose(fpin);
            break;
        }

		if(get_file_comment(argv[optind+i],comment_buff,MAX_COMMENT_LEN)){
			/*error occur*/
			printf("%s can't get comment!\n");
			continue;
		}


		/*init varibles for every file*/
		memset(stack,0,sizeof(struct _stack_t));
		scope = SCOPE_GLOBAL;
		last_line_type = 0;
		int last_last_line_type = 0;
		int buff_pos = 0;
		int line_buff_pos = 0;


		/*parse next valid line in file*/

		/*get buff until file end*/
		int file_loop = 1;
		while(file_loop)
		{
			int read_len = fread(buff,sizeof(char),MAX_BUFF_LEN,fpin);
			if(MAX_BUFF_LEN != read_len)
			{
				if(!feof(fpin))
				{
					printf("read %s error\n");
					clearerr(fpin);
					goto CONTINUE;
				}
				/*file read complete*/
				file_loop = 0;
				buff[read_len] = 0;
			}
			/*process every buff until it complete*/
			buff_pos = 0;
			while(1)
			{
				int ret = get_valid_line(buff,MAX_BUFF_LEN,buff_pos,line_buff,&line_buff_pos);
				if(ret >= 0)
				{
					buff_pos = ret + 1;
					/*parse line*/
					/*reset func_desc*/
					memset(func_desc,0,sizeof(struct _func_desc_t));
					ret = get_func_desc(line_buff,func_desc);
					if(ret == 0)
					{
						printf("%s:parse error.\n",argv[optind+i]);
						goto CONTINUE;
					}
					else if(ret == 1)
					{
						if(is_valid_func(func_desc))
						{
							/*get function comment*/

#warning "work here"
						}
						else if(last_line_type == 1)/*this line is a comment*/
						{
							last_last_line_type = 1;//save last line type;
							if(valid_line_index == 0 && (opt & OPT_f) && (opt & OPT_h))
							{
								//write file comment into tmpfie,and jump over buff commet

							}
						}
						else if(is_space_line(line_buff))
						{

							//write space line into tmpfile
							goto last_line_type_save;
						}
						else
						{
							last_last_line_type = 0;
							
							//write file comment tmpfile
							if(valid_line_index == 0 && (opt & OPT_h))
							{

							}
							//then write this line into tmpfile

						}
						last_line_type = 0;
						valid_line_index++;
last_line_type_save:
					}
#ifndef NDEBUG
					else
					{
						printf("get_valid_line return value error.\n");
						exit(-1);
					}
#endif

				}
				else if(ret == -1)
				{
					printf("%s:parse error.\n",argv[optind+i]);
					goto CONTINUE;
				}
				else if(ret == -2)
				{
					break;//buff end
				}
#ifndef NDEBUG
				else if(ret == -3)
				{
					file_loop = 0;//file end
				}
				else
				{
					printf("get_valid_line return value error!\n");
					exit(-1);
				}
#endif
			}
		}
		//old code
        FILE *fpout;
        if((opt & OPT_o) && args_for_o)
        {
            fpout = fopen(args_for_o,"w+");
            if(fpout == NULL){
                printf("%s create failed!\n",args_for_o);
                break;
            }
        }
        else
        {
            fpout = fopen(argv[optind+i],"w+");
            if(fpout == NULL){
                printf("%s write failed!\n",argv[optind+i]);
                break;
            }
        }
        /*copy tmpfile to destination*/
CONTINUE:
		fclose(fpin);
		fclose(fptmp);

    }
	free(func_desc);
QUIT:
	free(stack);
	return 0;
}

static inline int is_space_line(const char *line)
{
	while(*line)
	{
		if(!isspace(*line))
			return 0;
		++line;
	}
	return 1;
}
static inline const char *get_file_suffix(const char *file)
{
    const char *str = file + strlen(file);
    while(str >= file && *str != '.')
      --str;
    if(str < file)
      return NULL;
    return str;
}
static inline int parse_number(int *num,char *str[],int n)
{
    int i;
    for(i = 0; i < n; ++i)
    {
        int tmp = atoi(str[i]);
        if(tmp == 0)
          break;
        *num++ = tmp;
    }
    return i;
}
static inline int strip_str_by_comma(char *str,int argc,char *argv[])
{
    int count = 0;
    while(*str)
    {
        while(*str == ',')
        {
            *str++ = 0;
        }
        argv[count++] = str;
        if(count >= argc)
        break;
    }
    return count;
}

static inline int line_end_with(const char *line)
{
	while(isspace(*line))
		++line;
	return *line;
}

/*simple test already*/
/*return value:0 failed,1 success,but need to look insight the func_desc to judge if it's function*/
int get_func_desc(const char *func,struct _func_desc_t *func_desc)
{
    if(func == NULL)
      return 0;
    char word[MAX_NAME_LEN];
    int i = 0;
    int parameter_start = 0;
    int parameter_end = 0;
    int argument_start = 0;
    char last_split_char = 0;

    memset(func_desc,0,sizeof(struct _func_desc_t));
    while(*func!= '\n' && *func != 0)/*parse line*/
    {
        i = 0;
        while(1)/*get word*/
        {
            if(isalnum(*func) || *func == '_')
            {
                if(i >= MAX_NAME_LEN)
                {
                    word[MAX_NAME_LEN-1] = 0;
#ifndef NDEBUG
					printf("%s name too long!\n",word);
#endif
                    return 0;
                }
                word[i++] = *func;
            }
            else
            {
                if(*func == '(')
                {
                    last_split_char = '(';
                    parameter_start = 1;
                }
                else if(*func == ')')
                {
                    last_split_char = ')';
                    parameter_end = 1;
                }
                else if(*func == ',')
                  last_split_char = ',';
                else if(*func == ';')
                  last_split_char = ';';
                else 
                  last_split_char = 0;
                word[i] = 0;
                ++func;
                break;

            }
            ++func;
        }
        if(word[0] == 0)
          continue;
		if(!strcmp(word,"static"))
			func_desc->Flags |= FUNC_STATIC;
		else if(!strcmp(word,"inline"))
			func_desc->Flags |= FUNC_INLINE;
		else if(!strcmp(word,"const"))
			func_desc->Flags |= FUNC_CONST;
		else if(!strcmp(word,"noreturn"))
			func_desc->Flags |= FUNC_NORETURN;
		else if(!strcmp(word,"interrupt"))
			func_desc->Flags |= FUNC_INTERRUPT;
		else if(!strcmp(word,"extern"))
			func_desc->Flags |= FUNC_DECLARE;
		else if(!strcmp(word,"typedef"))
			func_desc->Flags |= FUNC_TYPEDEF;
		else if(!strcmp(word,"struct"))
			;
		else //parameter name
		{
			strcpy(func_desc->parameter[func_desc->argc],word);
			if(parameter_start)
			{
				func_desc->func_index = func_desc->argc;
				parameter_start = 0;
				argument_start = 1;
				func_desc->argc++;
			}
			if(argument_start == 0 || last_split_char == ',' || last_split_char == ')')
				func_desc->argc++;
			if(func_desc->argc >= MAX_LIST)
			{
#ifndef NDEBUG
				printf("Function parameter too much!\n");
#endif
				return 0;
			}
			if(parameter_end)
			{
				if(line_end_with(func) == ';')
					func_desc->Flags |= FUNC_DECLARE;
				else if(func_desc->Flags & FUNC_DECLARE)//"extern" keyword int front but not end with ';'
					return 0;
				break;
			}
		}

	}
	if(is_valid_func(func_desc) && !(func_desc->Flags & FUNC_DECLARE))
		func_desc->Flags |= FUNC_DEFINITION;

    return 1;
}
/*
 * return value: the size of read,which in valid_buf
 *      -1,if parse error,
 *      -2,if buff end
 *      -3,if file end
 */
//clear scope only when scope & SCOPE_COMMENT_LINE  after invoke get_valid_line one time
int get_valid_line(const char *buff,int bufsize,int start_pos,char *valid_buf,int *valid_buf_pos)
{
	int pos = start_pos;
	static char last_char = '\0';
	while(pos < bufsize)
	{
		switch(buff[pos])
		{
			case '(':
				if(scope < SCOPE_DMARK)
				{
					push(stack,BRACKT1_LEFT);
					scope |= SCOPE_BRACKT1;
				}
				break;
			case '[':
				if(scope < SCOPE_DMARK)
				{
					push(stack,BRACKT2_LEFT);
					scope |= SCOPE_BRACKT2;
				}
				break;

			case '{':
				if(scope < SCOPE_DMARK)
				{
					push(stack,BRACKT3_LEFT);
					scope |= SCOPE_BRACKT3;
				}
				break;

			case ')':
				if(scope < SCOPE_DMARK)
				{
					if(pop(stack) != BRACKT1_LEFT)
						goto parse_err;
					scope &= ~SCOPE_BRACKT1;
				}
				break;
			case ']':
				if(scope < SCOPE_DMARK)
				{
					if(pop(stack) != BRACKT2_LEFT)
						goto parse_err;
					scope &= ~SCOPE_BRACKT2;
				}
				break;
			case '}':
				if(scope < SCOPE_DMARK)
				{
					if(pop(stack) != BRACKT3_LEFT)
						goto parse_err;
					scope &= ~SCOPE_BRACKT3;
				}
				break;
			case '/':
				if(last_char == '*')
				{
					if(pop(stack) != COMMENT_LEFT)
						goto parse_err;
					scope &= ~SCOPE_COMMENT;
				}
				else if(last_char == '/' && scope == SCOPE_GLOBAL)
				{
					scope |= SCOPE_COMMENT_LINE;
					last_line_type = 1;
				}

				break;
			case '*':
				if(last_char == '/')
				{
					push(stack,COMMENT_LEFT);
					scope |= SCOPE_COMMENT;
					last_line_type = 1;
				}
				break;
			case '\"':
				if(!((scope & SCOPE_COMMENT)||(scope & SCOPE_COMMENT_LINE)))
				{
					if(last_char == '\\')
						break;
					else if(stack->length > 0 && peek(stack) == '\"')
					{
						pop(stack);
						scope &= ~SCOPE_DMARK;
					}
					else
					{
						push(stack,'\"');
						scope |= SCOPE_DMARK;
					}
				}
				break;
			case '\'':
				if(!((scope & SCOPE_COMMENT)||(scope & SCOPE_COMMENT_LINE)))
				{
					if(last_char == '\\')
						break;
					else if(stack->length > 0 && peek(stack) == '\'')
					{
						pop(stack);
						scope &= ~SCOPE_SMARK;
					}
					else
					{
						push(stack,'\'');
						scope |= SCOPE_SMARK;
					}
				}
				break;
			case '\\':
				if(!((scope & SCOPE_COMMENT)||(scope & SCOPE_COMMENT_LINE)))
				{
					if(last_char == '\\')
					{
						last_char = 0;//
						goto NEXT;
					}
				}
				break;

            case '\r':
				last_char ='\r';
				goto NEXT2;
            case '\n'://end
				if(last_char == '\\')
				{
					/*delete '\\' before '\n'*/
					last_char = ' ';
#ifndef NDEBUG
					if(*valid_buf_pos <= 0)
						goto parse_err;
#endif
					--(*valid_buf_pos);
					goto NEXT2;
				}
				else if(stack->length == 0)
                {
					
					scope &= ~SCOPE_COMMENT_LINE;
					if(scope != SCOPE_GLOBAL)
						goto parse_err;
                    valid_buf[(*valid_buf_pos)++] = buff[pos];
                    valid_buf[(*valid_buf_pos)++] = '\0';
					*valid_buf_pos = 0;
                    return pos;
				}
				/* I don't want get the whole statment in case of buff overflow,except this situation:
				   int main(int param1,
				   int param2);
				 */
				else if(stack->elem[0] == BRACKT1_LEFT)
				{
					last_char = '\n';
					goto NEXT2;
				}
				*valid_buf_pos = 0;
				valid_buf[0] = 0;
				break;
            case '\0':
                if(stack->length !=0)
                  goto parse_err;
                valid_buf[(*valid_buf_pos)++] = buff[pos];
                last_char = 0;
                return -3;
            default:
                break;

        }

        last_char = buff[pos];
NEXT:
		valid_buf[(*valid_buf_pos)++] = buff[pos];
NEXT2:
        if(*valid_buf_pos >= MAX_BUFF_LEN - 1)
        {
#ifndef NDEBUG
            printf("buf overflow!\n");
#endif
			goto parse_err;
        }
        ++pos;
    }
    return -2;
parse_err:
	*valid_buf_pos = 0;
	return -1;
	
}

int get_func_comment(const struct _func_desc_t *func_desc,char *buff,int bufsize)
{
	snprintf(buff,bufsize,"/************************************************\n*    Function Name : %s\n*    Return Value  : \n",
			func_desc->parameter[func_desc->func_index]);
	int i;
	int paramters = func_desc->argc - func_desc->func_index - 1;
	if(paramters == 0)
		snprintf(buff+strlen(buff),bufsize-strlen(buff),"*    Parameters    : NULL\n");
	else
	{
		int max_word_len = 0;
		for(i = 0; i < paramters; ++i)
		{
			int word_len = strlen(func_desc->parameter[func_desc->func_index+i+1]);
			max_word_len = max_word_len < word_len ? word_len : max_word_len;
		}
		for(i = 0; i < paramters; ++i)
		{
			snprintf(buff+strlen(buff),bufsize-strlen(buff),"*    %-*s: \n",max_word_len,
					func_desc->parameter[func_desc->func_index+i+1]);
		}

	}
	snprintf(buff+strlen(buff),bufsize-strlen(buff),"*    Description   : \n");
	snprintf(buff+strlen(buff),bufsize-strlen(buff),"*    History       : \n*    Modify Date   : %s\n*    Author        : %s\n************************************************/\n",
			date,user);
	return 1;
}
int get_file_comment(const char *filename,char *buff,int bufsize)
{
	snprintf(buff,bufsize,"/************************************************\n*    Copy Right    : GPL\n*    File Name     : %s\n*    Author        : %s\n*    Version       : v0.1\n*    History       : \n*    Modify Date   : %s\n*    Description   : \n************************************************/\n",
			filename,user,date);
	return 0;

}
