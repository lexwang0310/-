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

#define _HOST_  "127.0.0.1"   //主机
#define _USER_  "root"   //用户 mysql的用户
#define _PASSWD_ "root"   //密码 mysql
#define _DB_     "YUNPAN"  //库

int write_json_reg_reply(char * resp, char * err)
{
     //创建一个空的文档对象 {}
     cJSON * json = cJSON_CreateObject();
    
     int i = 0;

     cJSON * array = NULL;
     cJSON_AddItemToObject(json, "code", cJSON_CreateString(resp));
     if(err != NULL)
     	{
     		cJSON_AddItemToObject(json, "msg", cJSON_CreateString(err));
     	}
     

     //将json结构格式化到缓冲区中
     char * out_json_buf = NULL;
		printf("%s", out_json_buf = cJSON_Print(json));
		LOG(CGI_DATA_MODULE,CGI_DATA_PROC, "json:[%s]", out_json_buf);
/*
     //打开文件用以写入
     FILE *fp = fopen("create.json","w");
     fwrite(buf,1,strlen(buf),fp);
     free(buf);
     fclose(fp);
*/
     //释放json结构所占用的内存
     cJSON_Delete(json);
     free(out_json_buf);

     return 0;
 }
 
 
 int polling_mysql(char * query_string)
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
        
        
        //user
        query_parse_key_value(query_string, "user", user, &user_len);
        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "user:[%s]", user);
        query_parse_key_value(query_string, "pwd", pwd, &pwd_len);
        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "pwd:[%s]", pwd);
        
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
    

    // 在此处添加业务代码 
    char rSql[4096]= {0};
    sprintf(rSql, "select pwd from %s where user_name = \'%s\'",

            "user_info", user);
    if(mysql_query(mysql,rSql) ){
		LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "mysql query err");
		ret = 1;
		goto End;
	}
            
            
    //处理查询结果集
    //处理查询结果集
    MYSQL_RES *result = mysql_store_result(mysql);
    MYSQL_ROW row;
    	//	MYSQL_ROW mysql_fetch_row(MYSQL_RES *result) 
		if ( (row=mysql_fetch_row(result)) != NULL) {
				LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "before compare row[0] = [%s]", row[0]);
				if(strlen(row[0]) != pwd_len) 
					{
						ret = 1;
					}
				if(memcmp(pwd, row[0], pwd_len) != 0)
					{
						ret = 1;
						LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "row[0]:%s", row[0]);
					}
				LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "after compare ret = [%d]", ret);
			
		}else{
			ret = 1;
			LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "mysql no record");
		}
    //执行
    /*
    if(mysql_query(mysql,rSql) ){
    	printf("mysql query err:%s\n",rSql);
    	LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "mysql query err:%s\n",rSql);
    	mysql_close(mysql);
    	ret = -1;
    	goto End;
    }
    LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "insert ok!");
    */
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
        //解析query_string  得到cmd， fileid， count，user
        
        
        
        
        //写mysql
        ret = polling_mysql(query_string);
        if(ret == 0)
        	{
        		write_json_reg_reply("000", NULL);
        	}else{
        		write_json_reg_reply("001", NULL);
        	}
    


    } /* while */

    return 0;
}
