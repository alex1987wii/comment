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
	printf("        -h file comment.\n");
	printf("        -f=[ao],append or override.\n");
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
    int i;
    /*process file one by one*/
    for(i = 0; i < nr_file; ++i)
    {
		char buff[MAX_COMMENT_LEN];
          
        FILE *fpin = fopen(argv[optind+i],"r");
        if(fpin == NULL)
        {
            printf("%s open failed!\n",argv[optind+i]);
            break;
        }
 
        FILE *fptmp = fopen(TMPFILE,"w+");
        if(fptmp == NULL)
        {
            printf("%s create failed!\n",TMPFILE);
            fclose(fpin);
            break;
        }

		if(get_file_comment(argv[optind+i],buff,MAX_COMMENT_LEN)){
			/*error occur*/
			printf("%s can't get comment!\n");
			continue;
		}

		char buff[MAX_COMMENT_LEN];
		int comment_index = 0;
		if((opt & OPT_h)){
			while(true)
			{
				if(fgets(buff,MAX_COMMENT_LEN,fpin) == NULL && !feof(fpin))
				{
						clearerr(fpin);
						fclose(fpin);
						goto CONTINUE;
				}
#warning "bug here"
				/*notice that buff maybe not end with '\n' or '\r'*/
				comment_index = is_commend_line(buff,0);
				if(comment_index != -1)/*not empty line*/
					break;				

			}
			/*write file comment into tmpfile*/
			if((opt & OPT_f) || comment_index == -2)
			{
				fwrite(buff,strlen(buff),1,fptmp);

#ifndef NDEBUG
				printf("%s's comment had wroten into tmpfile!\n");
#endif
			}
#warning "just work here"	
			if(comment_index != -2 && buff[comment_index] == '*')
			{
				/*set file pointer jump over comment*/

			}
			
		}
        fclose(fpin);
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
		continue;

    }
	return 0;
}
/*must end with '\n' or '\r'
direction :0 forward, non-zero backward
  not comment line return -1 empty line, -2 not empty not comment line
  comment line return index of nextchar '/' or '*'
  */
static inline int is_comment_line(const char *line,int direction)
{
	const char *str = line;
	if(str == NULL)
		return -1;
	while(*str != '\n' || *str != '\r')
	{
		if(*str == '/')
		{
			if(*(str+1) == '/')
			{
				return str-line+1;
			}
			else if(direction == 0 && *(str+1) == '*')
				return str-line+1;

		}
		else if(direction && *str == '*' && *(str+1) == '/')
			return str-line;
		else if(!isspace(*str))
			return -2;
		++str;
	}
	return -1;

}
static inline int is_first_comment(const char *str,int n)
{
	if(str == NULL)
		return 0;
	int i = 0;
	while(*(str+i) && *(str+i) != '/' && i < n )
	{
		if(!isspace(*(str+i)))
			return 0;
		++i;
	}
	if(i == n || *(str+i) == 0)
		return 0;
	if(*(str+i+1) != '*')
		return 0;
	return i+1;

}
static inline int is_last_comment(const char *str,int n)
{
	if(str == NULL)
		return 0;
	int i = 0;
	while(*(str-i) && *(str-i) != '/' && i < n)
	{
		if(!isspace(*(str-i)))
			return 0;
		++i;
	}
	if(i == n && *(str-i) == 0)
		return 0;
	if(*(str-i-1) != '*')
		return 0;
	return i+1;
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
/*
 * return value: the size of read,which in valid_buf
 *      -1,if parse error
 *      -2,if buff end
 *      -3,if file end
 */
int get_valid_line(const char *buff,int bufsize,int start_pos,char *valid_buf,int *valid_buf_pos)
{
    int pos = start_pos;
    static char last_char = '\0';
    while(pos < bufsize)
    {
        switch(buff[pos])
        {
            case '(':
                push(stack,BRACKT1_LEFT);
                valid_buf[*valid_buf_pos++] = buff[pos];
                break;
            case '[':
                push(stack,BRACKT2_LEFT);
                valid_buf[*valid_buf_pos++] = buff[pos];
                break;

            case '{':
                push(stack,BRACKT3_LEFT);
                valid_buf[*valid_buf_pos++] = buff[pos];
                break;

            case ')':
                if(pop(stack) != BRACKT1_LEFT)
                  return -1;
                valid_buf[*valid_buf_pos++] = buff[pos];
                break;
            case ']':
                if(pop(stack) != BRACKT2_LEFT)
                  return -1;
                valid_buf[*valid_buf_pos++] = buff[pos];
                break;
            case '}':
                if(pop(stack) != BRACKT3_LEFT)
                  return -1;
                valid_buf[*valid_buf_pos++] = buff[pos];
                break;
            case '/':
                valid_buf[*valid_buf_pos++] = buff[pos];
                if(last_char == '*')
                {
                    if(pop(stack) != COMMENT_LEFT)
                      return -1;
                }

                break;
            case '*':
                valid_buf[*valid_buf_pos++] = buff[pos];
                if(last_char == '/')
                {
                    push(stack,COMMENT_LEFT);
                }
                break;

            case '\r':
                break;
            case '\n':
                /*if need to write into valid_buf?*/
                if(stack->length == 0)
                {
                    valid_buf[*valid_buf_pos++] = buff[pos];
                    valid_buf[*valid_buf_pos++] = '\0';
                    return pos;
                }
                else if(stack->elem[0] != BRACKT1_LEFT)
                {
                    *valid_buf_pos = 0;
                    valid_buf[0] = 0;
                }
                /*delete '\\' before '\n'*/
                if(*valid_buf_pos > 0 && last_char == '\\')
                  --*valid_buf_pos;
                break;
            case '\0':
                if(stack->length !=0)
                  return -1;
                valid_buf[*valid_buf_pos++] = buff[pos];
                last_char = 0;
                return -3;
            default:
                valid_buf[*valid_buf_pos++] = buff[pos];
                break;

        }

        last_char = buff[pos];
        if(*valid_buf_pos >= MAX_BUFF_LEN - 1)
        {
            printf("buf overflow!\n");
            exit(-1);
        }
        ++pos;
    }
    return -2;
}

/*simple test already*/
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
                printf("Function parameter too much!\n");
                exit(-1);
            }
            if(parameter_end)
            {
                if(*func == ';')
                  func_desc->Flags |= FUNC_DECLARE;
                break;
            }
        }

    }
    return 1;
}

