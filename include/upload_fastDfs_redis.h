#ifndef _UPLOAD_FASTDFS_REDIS_H
#define _UPLOAD_FASTDFS_REDIS_H


#define REDIS_OP_MODULE "test"
#define REDIS_OP_PROC "redis_op"


#define REDIS_OP_MODULE "test"
#define REDIS_OP_PROC "redis_op"
#define FILE_HOT_ZSET										"FILE_HOT_ZSET"
#define FILEID_NAME_HASH                "FILEID_NAME_HASH"
#define FILEID_TIME_HASH                "FILEID_TIME_HASH"
#define FILEID_URL_HASH                 "FILEID_URL_HASH"
#define FILEID_USER_HASH                 "FILEID_USER_HASH"
/*--------------------------------.
| 文件类型                        |
`--------------------------------*/
#define FILE_TYPE_BMP           "1"
#define FILE_TYPE_ZIP           "2"
#define FILE_TYPE_VEDIO         "3"
#define FILE_TYPE_MEDIA         "4"
#define EVENT_TYPE_TXT          "5"
#define EVENT_TYPE_OTHER        "6"

/*------------------------------------------.
| 文件ID和文件类型对应表 (HASH)             |
| Key:     FILEID_TYPE_HASH                 |
| field:  file_id                           |
| value:  file_type                         |
| redis 语句                                |
|    hset key field value                   |
|    hget key field                         |
`------------------------------------------*/
#define FILEID_TYPE_HASH                 "FILEID_TYPE_HASH"

enum FILE_MOTION{
 FILE_UPLOAD =  4000,
 FILE_DOWNLOAD,
 FILE_DELETE,
};

#define SERV_PORT 8000

int fileapi(const char * filepath/*in*/, 
            int motion/*in*/, 
            char * fileResult/*out*/);
int geturl(const char * fileid/*in*/,  char * fileResult/*out*/);

//在redis中写入FILEID_NAME_HASH，FILEID_TIME_HASH， FILEID_TYPE_HASH几个哈希表，
//把fastfds中fileid的文件的原文件名和创建时间和类型写进去
int write_redis(char * fileid, char * filename, char * file_type, char * url, char * user);


//find 'substr' from a fixed-length buffer   
//('full_data' will be treated as binary data buffer)  
//return NULL if not found  
char* memstr(char* full_data, int full_data_len, char* substr);

#endif
