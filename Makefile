
CC=gcc
CPPFLAGS= -I./include -I/usr/local/include/hiredis/ -I/usr/include/mysql/ -L/usr/lib64/mysql/
CFLAGS=-Wall 
LIBS= -lmemcached -lhiredis -lfcgi -lm  -lmysqlclient -lstdc++ -ldl -lpthread -lrt

#找到当前目录下所有的.c文件
src = $(wildcard ./src/*.c)

#将当前目录下所有的.c  转换成.o给obj
obj = $(patsubst %.c, %.o, $(src))


upload_with=upload_with
#redis_test_command = redis_test_command.o
data_cgi = data_cgi
reg_cgi= reg_cgi
login_cgi = login_cgi


target=$(upload_with) $(redis_test_command) $(data_cgi) $(reg_cgi) $(login_cgi)


ALL:$(target)



#main程序
$(upload_with):./src/upload4.c ./src/make_log.o ./src/dsapi2.o  ./src/redis_test_command.c ./src/redis_op.o ./src/util_cgi.o 
	$(CC) $^ -o $@ $(LIBS) $(CPPFLAGS)
	
	
#$(redis_test_command):./src/redis_test_command.c ./src/redis_op.c ./src/make_log.c
	#$(CC) $^ -o $@ $(LIBS) $(CPPFLAGS)

$(data_cgi):./src/make_log.c ./src/util_cgi.o  ./src/redis_op.o  ./src/cJSON.o  ./src/data.c  ./src/redis_test_command.c
	$(CC) $^ -o $@ $(LIBS) $(CPPFLAGS)
	
$(reg_cgi):./src/make_log.c   ./src/util_cgi.o ./src/cJSON.o ./src/reg.c
	$(CC) $^ -o $@ $(LIBS) $(CPPFLAGS)
	
$(login_cgi):./src/make_log.c   ./src/util_cgi.o ./src/cJSON.o ./src/login.c
	$(CC) $^ -o $@ $(LIBS) $(CPPFLAGS)

upload4.o:upload4.c
#gcc upload4.c -o src/upload4.o -lfcgi



#生成所有的.o文件
$(obj):%.o:%.c
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS) 


#clean指令

clean:
	-rm -rf $(obj) $(target) ./test/*.o

distclean:
	-rm -rf $(obj) $(target) ./test/*.o

#将clean目标 改成一个虚拟符号
.PHONY: clean ALL distclean
