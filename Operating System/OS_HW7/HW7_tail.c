#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>

// 這邊是方便我寫用的 C 預設沒bool (要額外include)
#define true 1
#define false 0
#define bool unsigned char

#define blocksize 1024  // 一個block 多大

// 回傳指定路徑檔案之大小
int filesize(const char* path){
	struct stat s;		// stat 規定的結構 存查詢的結果
	if(stat(path,&s)<0){	// 把路徑和儲存結構丟進去執行
		printf("ERROR: %s\n",strerror(errno)); // 輸出是怎樣的錯誤
		exit(-1);
	}
  return s.st_size;
}

// 把buffer清空(以\0填充)
void zeroBuffer(char* buffer,const int buffersize){
  for(int i=0;i<buffersize;++i){  // 全部都填
    buffer[i] = '\0';
  }
}

// 輸出指定路徑的檔案 最後n行的內容
void mytail(const char* path,const int n){
  printf("-----------讀取 %s 最後 %d 行的資料-----------\n",path,n);
  int fd = open(path,O_RDONLY); //以唯獨打開 我們只是要讀 不用改
  if(fd<0){
    printf("ERROR: %s\n",strerror(errno)); // 輸出是怎樣的錯誤
    exit(-1);
  }

  char* buffer = (char*)malloc((blocksize+1)*sizeof(char)); // read內容 用的buffer
  int pos = filesize(path); // 存開始讀的位置 初始化為檔案的最底部
  int offset = 0;     // 存最後輸出時 從該block開頭偏移多少後 開始輸出
  int lines = 0;      // 存當前已經找到幾行了
  bool found = false; // 有沒有找到起點

  // 分成兩部份 第一部份是目標以讀滿整個buffer讀
  // 第二部份 讀不滿整個buffer

  // 第一部份 能讀滿整個buffer
  while(pos >= blocksize){
    pos -= blocksize;       // 往前移動一個block
    lseek(fd,pos,SEEK_SET); // 從這個block的開頭開始讀

    zeroBuffer(buffer,blocksize+1); // 因為read不會清空buffer 也不會在尾巴加上\0
    read(fd,buffer,blocksize);      // 讀一個block
    for(int i=blocksize-1;i>=0;--i){// 從該block的尾巴開始讀起
      if(buffer[i]=='\n'){  //遇見換行符號
        if((++lines)==n){   // 判斷是不是找到最終開始讀的位置
          offset = i+1;     // 會是換行符號的下一個符號
          found = true;     // 我找到了
          break;
        }
      }
    }
    if(found){  // 找到就別找了
      break;
    }
  }

  if(found){  // 找到了
    lseek(fd,pos+offset,SEEK_SET);  // 從那個位置開始
  }else{      // 沒找到
    // 第二部份 讀不滿整個buffer
    zeroBuffer(buffer,blocksize+1); // 因為read不會清空buffer 也不會在尾巴加上\0
    read(fd,buffer,pos+1);          // 讀剩下的那一塊（剩下的內容）
    for(int i=pos-1;i>=0;--i){      // 從尾巴往回看
      if(buffer[i]=='\n'){  //遇見換行符號
        if((++lines)==n){   // 判斷是不是找到最終開始讀的位置
          offset = i+1;     // 會是換行符號的下一個符號
          break;
        }
      }
    }
    lseek(fd,offset,SEEK_SET);  // 從那個位置開始
  }

  // 從找到的地方開始得起
  zeroBuffer(buffer,blocksize+1);     // 因為read不會清空buffer 也不會在尾巴加上\0
  while(read(fd,buffer,blocksize)>0){ // 讀到底
    printf("%s",buffer);              // 輸出
    zeroBuffer(buffer,blocksize+1);   // 因為read不會清空buffer 也不會在尾巴加上\0
  }


  if(close(fd)<0){  // 關閉檔案
    printf("ERROR: %s\n",strerror(errno)); // 輸出是怎樣的錯誤
    exit(-1);
  }
  free(buffer); // 把記憶體還回去
  printf("\n-----------輸出結束-----------\n");
}

// 輸出程式的argument
void usage(){
  printf("用法\n");
  printf("  輸出最後幾行的資料 ./HW7_tail -[行數] [路徑]\n");
}

int main(int argc,char* argv[]){
  // 這邊就是處理參數 對應該怎樣執行
  if(argc==3){
    mytail(argv[2],atoi(argv[1]+1));
  }else{
    usage();
  }
  // mytail("../makefile",5);
}