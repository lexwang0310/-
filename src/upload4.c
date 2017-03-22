/*
 * echo.c --
 *
 *	Produce a page containing all FastCGI inputs
 *
 *
 * Copyright (c) 1996 Open Market, Inc.
 *
 * See the file "LICENSE.TERMS" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */
#ifndef lint
static const char rcsid[] = "$Id: echo.c,v 1.5 1999/07/28 00:29:37 roberts Exp $";
#endif /* not lint */

#include "fcgi_config.h"
#include "upload_fastDfs_redis.h"

#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <process.h>
#else
extern char **environ;
#endif

#include "fcgi_stdio.h"
#include <string.h>
#include <fcntl.h>
#include "make_log.h"

#define CGI_DATA_MODULE "cgi"
#define CGI_DATA_PROC "data"



static void PrintEnv(char *label, char **envp)
{
    printf("%s:<br>\n<pre>\n", label);
    for ( ; *envp != NULL; envp++) {
        printf("%s\n", *envp);
    }
    printf("</pre><p>\n");
}

int main ()
{
    char **initialEnv = environ;
    int count = 0;
    char * file = NULL;
    char *p = NULL;
    char *query_string = getenv("QUERY_STRING");
    char user[64] = {0};
    int user_len = 0;
    //user
    query_parse_key_value(query_string, "user", user, &user_len);
    LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "user:%s", user);
    while (FCGI_Accept() >= 0) {
        char *contentLength = getenv("CONTENT_LENGTH");
        int len;
				
        
	printf("Content-type: text/html\r\n"
	    "\r\n"
	    "<title>FastCGI echo</title>"
	    "<h1>FastCGI echo</h1>\n"
            "Request number %d,  Process ID: %d<p>\n", ++count, getpid());

        if (contentLength != NULL) {
            len = strtol(contentLength, NULL, 10);
        }
        else {
            len = 0;
        }

        if (len <= 0) {
	    printf("No data from standard input.<p>\n");
        }
        else {
            int i, ch;

	    printf("Standard input:<br>\n<pre>\n");
	    printf("<br />-----------------%d<br /><br /><br />", len);
	    file = (char *)malloc(len);
	    memset(file, 0, len);
	    p = file;
            for (i = 0; i < len; i++) {
                if ((ch = getchar()) < 0) {
										
                    printf("Error: Not enough bytes received on standard input<p>\n");
                    break;
		}
								*p = ch;
								p++;
                //putchar(ch);
            }
            printf("<br />-----------------%d<br /><br /><br />", len);
            //printf("<br />111<br />");
            
        //解析文件名
        char filename[256] = {0};
    		char fdfs_file_path[256] = {0};
    		char type[128] = {0};
        char *filename_begin = memstr(file,len, "filename")+10;
        char *filename_end = memstr(filename_begin, 1024, "\"");
        memcpy(filename, filename_begin, filename_end - filename_begin);
        
        printf("<br />-------filename----------%s<br /><br /><br />", filename);
        char *type_begin = memstr(filename_end,1024, "Content-Type:")+14;
        char * type_end = memstr(type_begin,1024, "\r\n");
         
        memcpy(type, type_begin, type_end - type_begin);
        printf("<br />--------type---------%s<br /><br /><br />", type);
        
        
        //解析file
        char * first = NULL;//每句的头指针
        char * end = NULL;//每句的尾指针（就是'\r\n'的'\r位置'）
        int temp_len = 0;//截出的长度
        
         char * new_first = NULL;//下一句的头
        int newlen = 0;//字符段截之后新的长度
				int count = 0;
				
				char * file_start = NULL;
				char * file_end = NULL;

				char boundary[1024] = {0};
				
        #if 1
        
        new_first = file;
        newlen = len;
        
        int fd = open(filename, O_RDWR | O_CREAT, 0666);
        while(newlen > 0)
        {
        first = new_first;
        end = memstr(first,newlen, "\r\n");
        
        
        temp_len = end - first;
       
        if(count == 0)
        	{
        		memcpy(boundary, first, temp_len);
        	}

         if(count == 4)
         	{
         		file_start = first;
         		break;
         	}
          
        count++;
        new_first = end + 2;
        newlen = newlen - temp_len -2; 
        	
        }
        file_end = memstr(file_start,(file + len - file_start), boundary) - 2;//还有个"\r\n"
        //写文件
        
          write(fd,  file_start, file_end - file_start);  
          close(fd);
        
        #endif
        
        //上传文件
        char fileid[1024] = {0};
        int ret = fileapi(filename, FILE_UPLOAD, fileid);//上传文件
				if(ret == -1)
					{
						printf("FILE_UPLOAD err\n");
					}else{
						printf("FILE_UPLOAD success\n");
						printf("fileid=#%s#\n%ld\n", fileid, strlen(fileid));
					}
        
        char url[1024] = {0};
         geturl(fileid, url);
         sprintf(url, "http://192.168.41.57/%s", fileid);//这个因为给别人看的时候，geturl生成的是85网段的
        
        //写redis
        ret = write_redis((char *)fileid, (char *)filename, (char *)type, url, user);
        if(ret != 0)
        	{
        		printf("write redis err\n");
        	}
        	else{
        		printf("write redis success!\n");
        		printf("fileid: %s, filename: %s, filetype : %s\n", fileid, filename, type);
        	}
        	
        	//删除本地文件
        	ret = unlink(filename);
        	if(ret != 0)
        	{
        		printf("unlink err\n");
        	}
        	else{
        		printf("unlink succ\n");
        	}
        	
        	printf("<br /> <br /><br /><br />\n");
        
            printf("\n</pre><p>\n");
        }
        
        


        PrintEnv("Request environment", environ);
        PrintEnv("Initial environment", initialEnv);
    } /* while */

    return 0;
}
