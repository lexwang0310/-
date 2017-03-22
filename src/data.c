
#include <stdlib.h>
#include <string.h>
//   
#include <unistd.h>
#include <sys/wait.h>

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

int zhuanma(char * raw_input, char * sub, char * sub2)
{
	int ret = 0, raw_input_len = 0, sub_len = 0;
	char * pos = NULL;
	char buf[128] = {0};
	raw_input_len = strlen(raw_input);
	sub_len = strlen(sub);
	pos = memstr(raw_input, raw_input_len, sub);
	strncpy(buf, raw_input, pos - raw_input);
	sprintf(buf, "%s%s%s",buf, sub2, pos + sub_len);
	memset(raw_input, 0, raw_input_len);
	strcpy(raw_input, buf);
	return ret;
}
int write_json(int count, RFIELDS fields, RVALUES type_values, RVALUES name_values,RVALUES time_values, RVALUES url_values, int * pv)
{
     //创建一个空的文档对象 {}
     cJSON * json = cJSON_CreateObject();
    
     int i = 0;

     cJSON * array = NULL;
     cJSON_AddItemToObject(json, "games", array = cJSON_CreateArray());
     for(;i < count; i++){
     
	     //在数组上添加对象 
	    cJSON *obj = NULL;
	    cJSON_AddItemToArray(array, obj = cJSON_CreateObject());
	    cJSON_AddItemToObject(obj, "id", cJSON_CreateString(fields[i]));
	    cJSON_AddItemToObject(obj, "kind", cJSON_CreateString(type_values[i]));
	    cJSON_AddItemToObject(obj, "title_m", cJSON_CreateString(name_values[i]));
	    cJSON_AddItemToObject(obj, "title_s", cJSON_CreateString("文件title_s"));
	    cJSON_AddItemToObject(obj, "descrip", cJSON_CreateString(time_values[i]));
	    cJSON_AddItemToObject(obj, "picurl_m", cJSON_CreateString("http://192.168.41.57/static/file_png/png.png"));
	    cJSON_AddItemToObject(obj, "url", cJSON_CreateString(url_values[i]));
	    cJSON_AddItemToObject(obj, "pv", cJSON_CreateNumber(pv[i]));
	    cJSON_AddItemToObject(obj, "hot", cJSON_CreateNumber(0));
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

int hot_zset_increase(char * query_string)
{
	int ret = 0;
	char fileId[128] = {0};
	int fileId_len = 0;
	redisContext *conn = NULL;
	redisReply *reply = NULL;
	ret = query_parse_key_value(query_string, "fileId", fileId, &fileId_len);
        if(ret != 0)
        	{
        		LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "fileId err:%s", fileId);
        	}
        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "fileId:%s", fileId);
   
   while(memstr(fileId, strlen(fileId), "%2F"))
   	{
   		zhuanma(fileId, "%2F", "/");
   	}
   	LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "fileId:%s", fileId);
    conn = rop_connectdb_nopwd("127.0.0.1", "6379");
    if (conn == NULL) {
        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "conn error\n");
				ret = -1;
        goto END;
    }

	reply = redisCommand(conn, "ZINCRBY %s 1 %s", FILE_HOT_ZSET, fileId);
	//rop_test_reply_type(reply);
	if (reply == NULL) {
		LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "[-][GMS_REDIS]Add or increment table: %s,member: %s Error:%s,%s\n", FILE_HOT_ZSET, fileId,reply->str, conn->errstr);
		
		ret = -1;
		goto END;
	}
END:
		if (conn != NULL) {
	        rop_disconnect(conn);
	    }
	  if (reply != NULL) {
	        freeReplyObject(reply);
	    }
	    return ret;
}


int show_newFile(char * query_string)
{
	int ret = 0;
		redisContext *conn = NULL;
		RFIELDS fields = NULL;
    RVALUES type_values = NULL;
    RVALUES name_values = NULL;
    RVALUES time_values = NULL;
    RVALUES url_values = NULL;
    int *pv= NULL;
	 	char fromId[8] = {0}, count[8] = {0}, user[64] = {0};
    int fromId_len = 0, count_len = 0, user_len = 0;
        
        ret = query_parse_key_value(query_string, "fromId", fromId, &fromId_len);
        if(ret != 0)
        	{
        		LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "fromId err:%s", fromId);
        	}
        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "fromId:%s", fromId);
        
        ret = query_parse_key_value(query_string, "count", count, &count_len);
        if(ret != 0)
        	{
        		LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "count err:%s", count);
        	}
        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "count:%s", count);
        
        ret = query_parse_key_value(query_string, "user", user, &user_len);
        if(ret != 0)
        	{
        		LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "user err:%s", user);
        	}
        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "user:%s", user);
        
        

        //FILE_INFO_LIST (fileid1-- fileid2---fileid3)
        //LRANGE FILE_INFO_LIST fromid count-1 
            int fromid = atoi(fromId);
				    int count_int = atoi(count);
				   
				    conn = rop_connectdb_nopwd("127.0.0.1", "6379");
				    if (conn == NULL) {
				        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "conn error\n");
								ret = -1;
				        goto END;
				    }
				    
				
				    fields = malloc(count_int * FIELD_ID_SIZE);
				    memset(fields, 0, count_int * FIELD_ID_SIZE);
				       
					int count_get = rop_zset_nget(conn, FILE_HOT_ZSET, fields, fromid, count_int);
					if(count_get == -1)
						{
							LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "rop_zset_nget err");
							goto END;
						}
					LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "rop_zset_nget num %d", count_get);
					
					type_values = malloc(count_get * VALUES_ID_SIZE);
					name_values = malloc(count_get * VALUES_ID_SIZE);
					time_values = malloc(count_get * VALUES_ID_SIZE);
					url_values =  malloc(count_get * VALUES_ID_SIZE);
					pv = (int *)malloc(count_get * sizeof(int));
					memset(type_values, 0, count_get * VALUES_ID_SIZE);
					memset(name_values, 0, count_get * VALUES_ID_SIZE);
					memset(time_values, 0, count_get * VALUES_ID_SIZE);
					memset(url_values, 0, count_get * VALUES_ID_SIZE);
					memset(pv, 0, count_get * sizeof(int));
					 LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "before  %d", count_get);

        //for fileid in fileidarray:
        // hget name_hash  fileid ----> name
        // hget url_hash  fileid ----> url
        int i = 0;
        for(; i <  count_get; i ++)
		    {
		    	//gettype
		    	ret =  rop_hget(conn, FILEID_TYPE_HASH, fields[i], type_values[i]);
		    	if(ret == -1)
		    		{
		    			LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "rop_hget FILEID_TYPE_HASH err");
		    			goto END;
		    		}
		    	//getname
		    	ret =  rop_hget(conn, FILEID_NAME_HASH, fields[i], name_values[i]);
		    	if(ret == -1)
		    		{
		    			LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "rop_hget FILEID_NAME_HASH err");
		    			goto END;
		    		}
		    	//gettime
		    	ret =  rop_hget(conn, FILEID_TIME_HASH, fields[i], time_values[i]);
		    	if(ret == -1)
		    		{
		    			LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "rop_hget FILEID_TIME_HASH err");
		    			goto END;
		    		}
		    	//geturl
		    	ret =  rop_hget(conn, FILEID_URL_HASH, fields[i], url_values[i]);
		    	if(ret == -1)
		    		{
		    			LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "rop_hget FILEID_URL_HASH err");
		    			goto END;
		    		}
		    	//get pv 
		    	pv[i] = rop_zset_get_score(conn, FILE_HOT_ZSET, fields[i]);
		    	if(pv[i] == -1)
		    		{
		    			LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "rop_zset_get_score err");
		    			goto END;
		    		}
		    }    
		    LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "all done %d", i);
		    

				
        //需要将数据 封装成一个json文件
				ret = write_json(count_get,  fields,  type_values,  name_values, time_values,  url_values,  pv);
        //json_str
        //char json_str[4096] = {0};
        //printf("json_str");


        //char json_str[4096] = {0};
        //FILE*fp = fopen("create.json", "r");
        //fread(json_str, 1, 4096, fp);
        //将json数据返回给前段，前端进行展示
        //LOG(CGI_DATA_MODULE,CGI_DATA_PROC, "json:[%s]", json_str);
        LOG(CGI_DATA_MODULE,CGI_DATA_PROC, "json returned");
        //printf("%s", json_str);
        

       // fclose(fp);
        //unlink("create.json");
        
END:
		if (conn != NULL) {
	        rop_disconnect(conn);
	    }
		 if(fields != NULL)
		 	{
			 	free(fields);
			 	fields = NULL;
			}
			if(type_values != NULL)
		 	{
			 	free(type_values);
			 	type_values = NULL;
			}
			if(name_values != NULL)
		 	{
			 	free(name_values);
			 	name_values = NULL;
			}
			if(time_values != NULL)
		 	{
			 	free(time_values);
			 	time_values = NULL;
			}
			if(url_values != NULL)
		 	{
			 	free(url_values);
			 	url_values = NULL;
			}
			if(pv != NULL)
		 	{
			 	free(pv);
			 	pv = NULL;
			}
			return ret;
}

int main ()
{
		int ret = 0;
 		

    while (FCGI_Accept() >= 0) {

        printf("Content-type: text/html\r\n"
                "\r\n");

        char *query_string = getenv("QUERY_STRING");

        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "query_string:%s", query_string);
        //query_string:cmd=newFile&fromId=0&count=8&user=
        //解析query_string  得到cmd， fileid， count，user
        
        char cmd[64] = {0};
        int cmd_len = 0;
        ret = query_parse_key_value(query_string, "cmd", cmd, &cmd_len);
        if(ret != 0)
        	{
        		LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "cmd err:%s", cmd);
        	}
        LOG(CGI_DATA_MODULE, CGI_DATA_PROC, "cmd:%s", cmd);
        
        if(strcmp(cmd, "increase") == 0)
        	{
        		hot_zset_increase(query_string);
        	}
        	else if(strcmp(cmd, "newFile") == 0)
        		{
        			show_newFile(query_string);
        		}
        
        
       




    } /* while */


    return 0;
}
