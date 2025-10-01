#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/stat.h"
#include "kernel/fs.h"

int main(){
	// 創見一個新的檔案用來測試
	int fd = open("test.txt",O_CREATE | O_RDWR);
	if(fd<0){
		exit(-1);
	}
	
	// 模擬寫入一些資料
	for(int i=0;i<10;++i){
		// 寫 0~9這些字到test.txt中
		char text[3] = "0\n";
		text[0] += i;		// 改成對應數字
		write(fd,text,3);	// 寫入
		if(i==5){
    		crashtest(); 	// 寫到一半崩潰
		}
	}

    return 0;
}

