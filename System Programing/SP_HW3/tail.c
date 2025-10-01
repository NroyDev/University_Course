#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>

#define bool int
#define true 1
#define false 0


// 用來處理錯誤訊息 會呼叫exit(-1) 把程式幹掉
void error_msg(const char* msg, bool use_errno){
	write(STDERR_FILENO, msg, strlen(msg));
	if(use_errno){
		const char* errmsg = strerror(errno);
		write(STDERR_FILENO, errmsg, strlen(errmsg));
	}
	write(STDERR_FILENO, "\n", 1);
	exit(-1);
}

// 把buffer清空(以\0填充)
void zero_buf(char* buf,const int BUF_SIZE){
	for(int i=0;i<BUF_SIZE;++i){  // 全部都填
		buf[i] = '\0';
	}
}

void usage(){
	const char* msg = "用法\n\ttail [-n num] file\n";
	write(STDOUT_FILENO, msg, strlen(msg));
	return;
}

int main(int argc, const char* argv[]){
	const char* file_path = NULL;	// 要讀取的檔案路徑
	long long num = 10;	//題目預設10行
	
	// ---------------------------------- 參數 ----------------------------------
	if(argc == 2){
		file_path = argv[1];
	}else if(argc == 4 && strcmp(argv[1],"-n")==0){
		file_path = argv[3];

		// 需要檢查 num 會不會不是數字
		char* endptr = NULL;
		errno = 0;	// strtoll 不會將errno重設為0
		num = strtoll(argv[2], &endptr, 10);	// 10=>decimal
		if(endptr == argv[2]){
			usage();
			error_msg("Error: 未找到任何有效的數字可以進行轉換",false);
		}else if(errno != 0){
			error_msg("Error: stroll - ", true);
		}else if(num < 0){
			error_msg("Error: 不允許讀取負數行", false);
		}else if(*endptr != '\0'){
			const char* msg = "Warning: 參數中存在無法轉換部份 ";
			write(STDERR_FILENO, msg, strlen(msg));
			write(STDERR_FILENO, endptr, strlen(endptr));
			write(STDERR_FILENO, "\n", 1);
		}
	}else{
		usage();
		return 0;
	}

	// ---------------------------------- 開啟檔案 ----------------------------------
	int fd = 0;
	if((fd = open(file_path, O_RDONLY)) == -1){
		error_msg("Error: open - ", true);
	}

	// ---------------------------------- 找到起始位置 ----------------------------------
	const unsigned int BUF_SIZE = 16384;	// buffer 的大小
	unsigned char buf[BUF_SIZE];		// 讀取用的buffer 防止過多呼叫read write這類system call 造成的巨大開銷
	unsigned long long start_pos = 0;	// 指示最後num行該從哪個地方開始讀起
	if((start_pos = lseek(fd, 0, SEEK_END)) ==-1){		// 移動至EOF
		error_msg("Error: lseek - ", true);
	}
	unsigned long long line = 0;	// 當前找到了幾行的開頭
	// 標準POSIX中 一行的定義是 [文字們]\n 組成的
	// 所以我們讀第一個字的時候必然會先讀到一個\n (在POSIX標準文件上)
	bool isfirst_read = true;	// 用來跳過\nEOF
	
	// 第一部份 能讀滿整個buffer
	while(line<num && start_pos>= BUF_SIZE){
		start_pos -= BUF_SIZE;						// 往前移動一個block
		if(lseek(fd, start_pos, SEEK_SET) == -1){	// 從這個block的開頭開始讀
			error_msg("Error: lseek - ", true);
		}
		
		zero_buf(buf,BUF_SIZE);						// 因為read不會清空buffer 也不會在尾巴加上\0
		if(read(fd, buf,BUF_SIZE)!=BUF_SIZE){		// 讀一個block
			error_msg("Error: read - ", true);
		}
		for(int i=BUF_SIZE-1; i>=0; --i){			// 從該block的尾巴開始讀起
			if(isfirst_read){						// 如果EOF前面的那個字是\n直接吃掉 因為不算找到一行的開頭 對於非標準POSIX文件(非\nEOF) 讀到不是\n 吃掉也沒差
				isfirst_read = false;
			}else if(buf[i]=='\n' && (++line)>=num){// 判斷是不是找到最終開始讀的位置
				start_pos += i+1;	// 會是換行符號的下一個符號		把位置移動到正確的位置(加上該block的位置的offset) 
				break;
			}
		}
	}
	
	if(line<num && start_pos > 0){	// 可能會有小於一個BUF_SIZE的block 讀取這塊
		if(lseek(fd, 0, SEEK_SET) == -1){			// 從這個block的開頭開始讀
			error_msg("Error: lseek - ", true);
		}
		
		zero_buf(buf,BUF_SIZE);						// 因為read不會清空buffer 也不會在尾巴加上\0
		// 需要讀 0~(start_pos-1) => 共 start_pos bytes
		if(read(fd,buf,BUF_SIZE)!=start_pos){		// 理論上要讀到這多 (-1 的狀況剛好也會被弄近來)
			error_msg("Error: read - ", true);
		}
		for(int i=start_pos-1; i>=0; --i){			// 從該block的尾巴開始讀起
			if(isfirst_read){						// 如果EOF前面的那個字是\n直接吃掉 因為不算找到一行的開頭 對於非標準POSIX文件(非\nEOF) 讀到不是\n 吃掉也沒差
				isfirst_read = false;
			}else if(buf[i]=='\n' && (++line)>=num){// 判斷是不是找到最終開始讀的位置
				start_pos = i+1;	// 會是換行符號的下一個符號		把位置移動到正確的位置(該block的位置的offset) 
				break;
			}
		}
		if(line<num){		// 都沒找到 從頭reply
			start_pos = 0;
		}
	}
	
	// --------------------------------- 印出指定的那幾行 ---------------------------------- 
	if(lseek(fd, start_pos, SEEK_SET) == -1){
		error_msg("Error: lseek - ", true);
	}
	int numread = 0;
	zero_buf(buf,BUF_SIZE);	// 因為read不會清空buffer 也不會在尾巴加上\0
	while((numread = read(fd, buf, BUF_SIZE)) > 0){
		if(write(STDOUT_FILENO, buf, numread) != numread){
			error_msg("Erorr: write - ", true);
		}
		zero_buf(buf,BUF_SIZE);	// 因為read不會清空buffer 也不會在尾巴加上\0
	}
	if(numread == -1){
		error_msg("Error: read - ", true);
	}
	
	
	// -------------- 關閉檔案 --------------
	if(close(fd)==-1){
		error_msg("Erorr: close - ", true);
	}

	return 0;
}
