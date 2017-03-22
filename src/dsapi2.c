#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include "upload_fastDfs_redis.h"

#include "make_log.h"

/*
#define 	FILE_UPLOAD  4000
#define 	FILE_DOWNLOAD  4001
#define 	FILE_DELETE  4002
*/





#define SERV_PORT 8000

int fileapi(const char * filepath/*in*/, 
						int motion/*in*/, 
						char * fileResult/*out*/)
{
	int ret = 0;

	pid_t pid;
	int pipefd1[2];
	int pipefd2[2];
	pipe(pipefd1);
	pipe(pipefd2);
	pid = fork();
	if(pid == 0)
			{
				close(pipefd1[0]);
				close(pipefd2[0]);
				dup2(pipefd1[1], STDOUT_FILENO);//正常情况，传输fileid
				dup2(pipefd2[1], STDERR_FILENO);//异常，传输错误
				switch(motion)
				{
					case FILE_UPLOAD:
						{
						execlp("fdfs_upload_file", "fdfs_upload_file", "/etc/fdfs/client.conf",filepath, NULL);
						return ret;
					}
					case FILE_DELETE:
						execlp("fdfs_delete_file", "fdfs_delete_file", "/etc/fdfs/client.conf",filepath, NULL);
						return ret;
					case FILE_DOWNLOAD:
						execlp("fdfs_download_file", "fdfs_download_file", "/etc/fdfs/client.conf",filepath, NULL);
						return ret;
					default:
						return ret;
				}
				
			}else if(pid > 0)
				{
					close(pipefd1[1]);
					close(pipefd2[1]);
					char buf1[1024] = {0};//正常情况，传输fileid
					char buf2[1024] = {0};//异常，传输错误
					read(pipefd1[0], buf1, 1024);
					read(pipefd2[0], buf2, 1024);
					if(strlen(buf1) != 0)
						{
							if(buf1[strlen(buf1) - 1] == '\n')
								{
									buf1[strlen(buf1) - 1] = '\0';
								}
							LOG("1111", "222", "%s ,%d", buf1, 100);
							strcpy(fileResult, buf1);
							
							
						}
					if(strlen(buf2) != 0){
						LOG("1111", "222", "%s ,%d", buf2, 100);
						ret = -1;
						strcpy(fileResult, buf2);
					}
					
				}
			
	return ret;			
}

int fileapi2(const char * filepath/*in*/, int motion/*in*/, char ** fileResult/*out*/)
{
	int ret = 0;
	int fdout = 0, fderr = 0;
				dup2(fdout, STDOUT_FILENO);
				dup2(fderr, STDERR_FILENO);
				switch(motion)
				{
					case FILE_UPLOAD:
						execlp("fdfs_upload_file", "fdfs_upload_file", "/etc/fdfs/client.conf",filepath, NULL);
						break;
					case FILE_DELETE:
						execlp("fdfs_delete_file", "fdfs_delete_file", "/etc/fdfs/client.conf",filepath, NULL);
						break;
					case FILE_DOWNLOAD:
						execlp("fdfs_download_file", "fdfs_download_file", "/etc/fdfs/client.conf",filepath, NULL);
						break;
				}
				
			
			
	return ret;			
}



int geturl(const char * fileid/*in*/,  
						char * fileResult/*out*/)
{
	int ret = 0;

	pid_t pid;
	int pipefd1[2];
	int pipefd2[2];
	pipe(pipefd1);
	pipe(pipefd2);
	pid = fork();
	if(pid == 0)
			{
				close(pipefd1[0]);
				close(pipefd2[0]);
				dup2(pipefd1[1], STDOUT_FILENO);//正常情况，传输fileid
				dup2(pipefd2[1], STDERR_FILENO);//异常，传输错误
				
						execlp("fdfs_file_info", "fdfs_file_info", "/etc/fdfs/client.conf",fileid, NULL);
						return ret;
					
				
				
			}else if(pid > 0)
				{
					close(pipefd1[1]);
					close(pipefd2[1]);
					char buf1[1024] = {0};//正常情况，传输fileid
					char buf2[1024] = {0};//异常，传输错误
					read(pipefd1[0], buf1, 1024);
					read(pipefd2[0], buf2, 1024);
					if(strlen(buf1) != 0)
						{
							if(buf1[strlen(buf1) - 1] == '\n')
								{
									buf1[strlen(buf1) - 1] = '\0';
								}
							LOG("1111", "222", "%s ,%d", buf1, 100);
							strcpy(fileResult, buf1);
							
							
						}
					if(strlen(buf2) != 0){
						LOG("1111", "222", "%s ,%d", buf2, 100);
						ret = -1;
						strcpy(fileResult, buf2);
					}
					
				}
				
			char * ip_start = memstr(fileResult, strlen(fileResult), "source ip address:") + 19;
			char temp[128] = {0};
			char * ip_end = memstr(ip_start, strlen(ip_start), "\n");
			*ip_end = '\0';
			strcpy(temp, ip_start);
			memset(fileResult, 0, strlen(fileResult));
			sprintf(fileResult,"http://%s/%s", temp,fileid);
			//source ip address:
	return ret;			
}









int main2()
{
	char fileResult[1024] = {0};
	int ret = geturl("group1/M00/00/00/wKhVo1jN-JuADSOXAAGETx8WMWE649.jpg",fileResult/*out*/);
	if(ret == -1)
		{
			printf("err\n");
		}
		printf("#%s#\n%ld", fileResult, strlen(fileResult));
	
	return 0;
}


int main1(int argc, char *argv[])
{
	if(argc != 2)
		{
			printf("usage: %s filename\n", argv[0]);
			exit(1);
		}
	//char file[1024] = "11.c";
	char * file = argv[1];
	int motion = FILE_UPLOAD;
	char fileid[1024] = {0};
	char fileResult[1024] = {0};
	
	int ret = fileapi(file, motion, fileid);//上传文件
	if(ret == -1)
		{
			printf("FILE_UPLOAD err\n");
		}else{
			printf("FILE_UPLOAD success\n");
			printf("fileid=#%s#\n%ld\n", fileid, strlen(fileid));
		}
		
		
		
	
		ret = fileapi(fileid, FILE_DOWNLOAD, fileResult);//下载文件
	if(ret == -1)
		{
			printf("FILE_DOWNLOAD err\n");
		}else{
			printf("FILE_DOWNLOAD success\n");
		}
		
		ret = fileapi(fileid, FILE_DELETE, fileResult);//删除云端文件
	if(ret == -1)
		{
			printf("FILE_DELETE err\n");
		}else{
			printf("FILE_DELETE success\n");
			
		}
		

		

	
	return 0;
}