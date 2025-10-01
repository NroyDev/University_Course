#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define bool unsigned char
#define false 0
#define true 1

void errorMSG(const char* err){
	write(STDERR_FILENO, err, strlen(err));				// 輸出傳入的訊息
	const char* errno_msg = strerror(errno);			// 透過errno拿到當前錯誤資訊
	write(STDERR_FILENO, errno_msg, strlen(errno_msg));	// 輸出剛剛拿到的錯誤資訊
	write(STDERR_FILENO, "\n", 1);						// 輸出換行
	return;
}

void usage(){				// 輸出這個程式的用法到畫面上 stdout
	const char* msg = 
	"用法\
	\n	./tee [檔案]\
	\n	./tee -a [檔案]\
	\n";
	write(STDOUT_FILENO, msg, strlen(msg));

	return;
}

int main(int argc,char* argv[]){
    // -------------- args --------------
    const char* filepath = NULL;
    bool append_mode = false;
    int opt;

    while ((opt = getopt(argc, argv, "a")) != -1) {
        switch (opt) {
            case 'a':
                append_mode = true;
                break;
            default: // invalid
                usage();
                return -1;
        }
    }

    // 檢查是否有提供檔案路徑
    if (optind >= argc) {
        usage();
        return -1;
    }
    filepath = argv[optind];

	// -------------- 開啟檔案 --------------
	int fd = STDOUT_FILENO;
	if(append_mode){
		fd = open(filepath, O_CREAT | O_WRONLY | O_APPEND, S_IRWXU|S_IRWXG|S_IRWXO);	// 打開要寫到的檔案(append mode) 如果沒有就建立一個 權限全開
	}else{
		fd = open(filepath, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);		// 打開要寫到的檔案 (預設清空)如果沒有就建立一個 權限全開
	}
	if(fd == -1){	// 開檔失敗
		errorMSG("Open file failed - ");
		return -1;
	}

	// -------------- 讀取並輸出到 file and stdout --------------
	const unsigned int BUF_SIZE = 32;
	unsigned char buf[BUF_SIZE] = {};
	unsigned int numread = 0;
	while((numread = read(STDIN_FILENO,buf,BUF_SIZE)) > 0){	// 一直讀讀到EOF
		// 讀到的輸出到 stdout
		if(write(STDOUT_FILENO,buf,numread)!=numread){
			errorMSG("Write file(STDOUT) failed - ");
			return -1;
		}
		// 讀到的輸出到 指定的file	
		if(write(fd,buf,numread)!=numread){
			errorMSG("Write file(fd) failed - ");
			return -1; 
		}
	}
	if(numread==-1){
		errorMSG("Read STDIN failed - ");
		return -1;
	}
	
	// -------------- 關閉檔案 --------------
	if(close(fd)==-1){
		errorMSG("close file failed - ");
		return -1;
	}

	return 0;
}
