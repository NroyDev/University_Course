#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

void errorMSG(const char* err){
	write(STDERR_FILENO, err, strlen(err));				// 輸出傳入的訊息
	const char* errno_msg = strerror(errno);			// 透過errno拿到當前錯誤資訊
	write(STDERR_FILENO, errno_msg, strlen(errno_msg));	// 輸出剛剛拿到的錯誤資訊
	write(STDERR_FILENO, "\n", 1);						// 輸出換行
	return;
}

void usage(){
	const char* usage_msg =
	"用法\
	\n	./cp sourceFile destFile\
	\n";

	write(STDOUT_FILENO, usage_msg, strlen(usage_msg));
	return;
}



int skipHoleWrite(unsigned char* buf,unsigned int size,int target_fd){
	while(size>0){
		// 看看  buffer 內的開頭 有多少byte的\0
		int skipped = 0;
		for(int i=0; i<size; ++i){
			if(buf[i]!='\0'){
				break;
			}
			++skipped;
		}
		if(skipped){	// 跳過開頭的\0
			if(lseek(target_fd,skipped,SEEK_CUR)==-1){
				errorMSG("lseek - ");
				return -1;
			}
		}	
		
		// 找看看從第一個非\0的位置開始 有多少非\0 byte可以寫 把他們寫進去
		int bytes_toWrite = 0;
		for(int i=skipped;i<size;++i){
			if(buf[i]=='\0'){
				break;
			}
			++bytes_toWrite;
		}
		if(write(target_fd,buf+skipped,bytes_toWrite) != bytes_toWrite){		// 確認都有寫進去
			errorMSG("Write dest file failed - ");	// 沒有=>error
			return -1;
		}

		size -= (skipped+bytes_toWrite);
		buf  += (skipped+bytes_toWrite);
	}
	return 0;
}



int main(int argc,const char* argv[]){
	if(argc!=3){
		usage();
		return -1;
	}
	const char* source = argv[1];
	const char* dest = argv[2];

	// -------------- 開啟檔案 --------------
	int fds = open(source, O_RDONLY);						// 唯讀(複製不存在的檔案怪怪的)
	if(fds==-1){	//  source 開檔失敗
		errorMSG("Open source file failed - ");
		return -1;
	}
	int fdd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXG);	// 唯寫 如果不存在就開新的 權限全開
	if(fdd==-1){	//  dest 開檔失敗
		errorMSG("Open dest file failed - ");
		return -1;
	}

	// -------------- source 複製到 dest --------------
	const unsigned int BUF_SIZE = 8192;
	unsigned char buf[BUF_SIZE] = {};
	unsigned int numread = 0;
	while((numread = read(fds,buf,BUF_SIZE)) > 0){	// 一直讀讀到EOF
		// 題目要求製造相同的holes在target file中
		
		if(skipHoleWrite(buf,numread,fdd)==-1){
			return -1;
		}
	}
	if(numread == -1){
		errorMSG("Read source file failed - ");
		return -1;
	}
	
	// -------------- 關閉檔案 --------------
	if(close(fds)==-1 || close(fdd)==-1){
		errorMSG("close file failed - ");
		return -1;
	}

	return 0;
}
