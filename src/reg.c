#include <stdlib.h>
#include <string.h>
//   
#include <unistd.h>
#include <sys/wait.h>
#include "mysql.h"

#include "fcgi_stdio.h"
#include "fcgi_config.h"

#include "util_cgi.h"
#include "make_log.h"
#include "redis_op.h"
#include "upload_fastDfs_redis.h"

#define CGI_DATA_MODULE "cgi"
#define CGI_DATA_PROC "data"

#include <stdlib.h>
#include "cJSON.h"

#define _HOST_  "127.0.0.1"   //����
#define _USER_  "root"   //�û� mysql���û�
#define _PASSWD_ "root"   //���� mysql
#define _DB_     "YUNPAN"  //��

int write_json_reg_reply(int resp, char * err)
{
     //����һ���յ��ĵ����� {}
     cJSON * json = cJSON_CreateObject();
    
     int i = 0;

     cJSON * array = NULL;
     cJSON_AddItemToObject(json, "code", cJSON_CreateNumber(resp));
     if(err != NULL)
     	{
     		cJSON_AddItemToObject(json, "msg", cJSON_CreateString(err));
     	}
     

     //��json�ṹ��ʽ������������
     char * out_json_buf = NULL;
		printf("%s", out_json_buf = cJSON_Print(json));
		LOG(CGI_DATA_MODULE,CGI_DATA_PROC, "json:[%s]", out_json_buf);
/*
     //���ļ�����д��
     FILE *fp = fopen("create.json","w");
     fwrite(buf,1,strlen(buf),fp);
     free(buf);
     fclose(fp);
*/
     //�ͷ�json�ṹ��ռ�õ��ڴ�
     cJSON_Delete(json);
     free(out_json_buf);

     return 0;
 }
 
 
 int write_mysql(char * query_string)
 {
 	int ret = 0;
 	MYSQL *mysql = NULL;
 	
 				char type[64] = {0};
        int type_len = 0;
        ret = query_parse_key_value(query_string, "type", type, &type_len);
        if(ret != 0)
        	{
        		LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "type err:%s", type);
        	}
        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "type:%s", type);
        
        char user[64] = {0};
        int user_len = 0;
        char pwd[64] = {0};
        int pwd_len = 0;
        char flower_name[64] = {0};
        int flower_name_len = 0;
        char tel[12] = {0};
        int tel_len = 0;
        char email[64] = {0};
        int email_len = 0;
        
        //user
        query_parse_key_value(query_string, "user", user, &user_len);
        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "user:%s", user);
        query_parse_key_value(query_string, "pwd", pwd, &pwd_len);
        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "pwd:%s", pwd);
        query_parse_key_value(query_string, "flower_name", flower_name, &flower_name_len);
        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "flower_name:%s", flower_name);
        query_parse_key_value(query_string, "tel", tel, &tel_len);
        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "tel:%s", tel);
        query_parse_key_value(query_string, "email", email, &email_len);
        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "email:%s", email);
        
    //1. mysql_init
    mysql = mysql_init(NULL);
    if(mysql == NULL){
      LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "mysql init err!");
    	ret = -1;
    	goto End;
    }
    //2. mysql_real_connect
    //MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag) 
    mysql = mysql_real_connect(mysql,_HOST_,_USER_,_PASSWD_,_DB_,0,NULL,0);
    if(mysql == NULL ){
    	LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "mysql connect err!");
    	ret = -1;
    	goto End;
    }
    LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "mysql connect succ!");
    
    char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
   	time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep); //ȡ�õ���ʱ��
    char timepp[128] = {0};
    sprintf(timepp,"%d%02d%d %s %d:%d:%d", (1900+p->tm_year), (1 + p->tm_mon), p->tm_mday, wday[p->tm_wday], p->tm_hour, p->tm_min, p->tm_sec);
    //��ǰʱ���

    struct timeval tv;

    struct tm* ptm;

    char time_str[128];



    gettimeofday(&tv, NULL);

    ptm = localtime(&tv.tv_sec);

    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", ptm);
    // �ڴ˴����ҵ����� 
    char rSql[4096]= {0};
    sprintf(rSql, "insert into %s (user_name,pwd, nicheng, tel, createtime, email) values ('%s', '%s', '%s', '%s', '%s', '%s')",

            "user_info", user, pwd, flower_name,  tel, time_str ,email);
    //ִ��
    if(mysql_query(mysql,rSql) ){
    	printf("mysql query err:%s\n",rSql);
    	LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "mysql query err:%s\n",rSql);
    	mysql_close(mysql);
    	ret = -1;
    	goto End;
    }
    LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "insert ok!");
    End:
    //3. mysql_close
    mysql_close(mysql);
 	return ret;
 }

int main(int argc, const char* argv[])
{
    int ret = 0;
 		

    while (FCGI_Accept() >= 0) {

        printf("Content-type: text/html\r\n"
                "\r\n");

        char *query_string = getenv("QUERY_STRING");

        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "query_string:%s", query_string);
        //query_string:cmd=newFile&fromId=0&count=8&user=
        //����query_string  �õ�cmd�� fileid�� count��user
        
        
        
        
        //дmysql
        ret = write_mysql(query_string);
        if(ret == 0)
        	{
        		write_json_reg_reply(0, NULL);
        	}else{
        		write_json_reg_reply(1, NULL);
        	}
    


    } /* while */

    return 0;
}
