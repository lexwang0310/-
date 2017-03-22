#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "make_log.h"
#include "redis_op.h"

#include "upload_fastDfs_redis.h"


//在redis中写入FILEID_NAME_HASH，FILEID_TIME_HASH， FILEID_TYPE_HASH几个哈希表，
//把fastfds中fileid的文件的原文件名和创建时间和类型写进去
int write_redis(char * fileid, char * filename, char * file_type, char * url, char * user)
{
    int retn = 0;
    redisContext *conn = NULL;
    


    conn = rop_connectdb_nopwd("127.0.0.1", "6379");
    if (conn == NULL) {
        LOG(REDIS_OP_MODULE, REDIS_OP_PROC, "conn error\n");
				retn = -1;
        goto END;
    }

    
    char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
   	time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep); //取得当地时间
    char timepp[128] = {0};
    sprintf(timepp,"%d%02d%d %s %d:%d:%d", (1900+p->tm_year), (1 + p->tm_mon), p->tm_mday, wday[p->tm_wday], p->tm_hour, p->tm_min, p->tm_sec);
    retn = rop_hset(conn, FILEID_NAME_HASH, fileid, filename);
    if (retn != 0) {
        LOG(REDIS_OP_MODULE, REDIS_OP_PROC, "rop_hset FILEID_NAME_HASH error\n");
        printf("rop_hset FILEID_NAME_HASH error\n");
        retn = -1;
        goto END;
    }
    retn = rop_hset(conn, FILEID_TIME_HASH, fileid, timepp);
    if (retn != 0) {
        LOG(REDIS_OP_MODULE, REDIS_OP_PROC, "rop_hset FILEID_TIME_HASH error\n");
        printf("rop_hset FILEID_TIME_HASH error\n");
        retn = -1;
        goto END;
    }
    
    retn = rop_hset(conn, FILEID_TYPE_HASH, fileid, file_type);
    if (retn != 0) {
        LOG(REDIS_OP_MODULE, REDIS_OP_PROC, "rop_hset FILEID_TYPE_HASH error\n");
        printf("rop_hset FILEID_TYPE_HASH error\n");
        retn = -1;
        goto END;
    }
    

    retn = rop_hset(conn, FILEID_URL_HASH, fileid, url);
    if (retn != 0) {
        LOG(REDIS_OP_MODULE, REDIS_OP_PROC, "rop_hset FILEID_URL_HASH error\n");
        printf("rop_hset FILEID_URL_HASH error\n");
        retn = -1;
        goto END;
    }
    
    retn = rop_hset(conn, FILEID_USER_HASH, fileid, user);
    if (retn != 0) {
        LOG(REDIS_OP_MODULE, REDIS_OP_PROC, "rop_hset FILEID_USER_HASH error\n");
        printf("rop_hset FILEID_USER_HASH error\n");
        retn = -1;
        goto END;
    }
    
    /*
        retn = rop_zset_increment(conn, FILE_HOT_ZSET, fileid);
    if (retn != 0) {
        LOG(REDIS_OP_MODULE, REDIS_OP_PROC, "rop_zset_increment FILE_HOT_ZSET error\n");
        printf("rop_zset_increment FILE_HOT_ZSET error\n");
        retn = -1;
        goto END;
    }
    */
    
    
	redisReply *reply = NULL;

	reply = redisCommand(conn, "ZINCRBY %s 1 %s", FILE_HOT_ZSET, fileid);
	//rop_test_reply_type(reply);
	if (reply == NULL) {
		LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "[-][GMS_REDIS]Add or increment table: %s,member: %s Error:%s,%s\n", FILE_HOT_ZSET, fileid,reply->str, conn->errstr);
		
		retn = -1;
		goto END;
	}

    

END:
    if (conn != NULL) {
        rop_disconnect(conn);
    }
    
  freeReplyObject(reply);
	return retn;

	return retn;
}





//find 'substr' from a fixed-length buffer   
//('full_data' will be treated as binary data buffer)  
//return NULL if not found  
char* memstr(char* full_data, int full_data_len, char* substr) 
{ 
    if (full_data == NULL || full_data_len <= 0 || substr == NULL) { 
        return NULL; 
    } 

    if (*substr == '\0') { 
        return NULL; 
    } 

    int sublen = strlen(substr); 

    int i; 
    char* cur = full_data; 
    int last_possible = full_data_len - sublen + 1; 
    for (i = 0; i < last_possible; i++) { 
        if (*cur == *substr) { 
            //assert(full_data_len - i >= sublen);  
            if (memcmp(cur, substr, sublen) == 0) { 
                //found  
                return cur; 
            } 
        } 
        cur++; 
    } 

    return NULL; 
} 


