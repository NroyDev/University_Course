#include<stdio.h>
#include<dirent.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<unistd.h>

#include<errno.h>
#include<string.h>
// 這邊是方便我寫用的 C 預設沒bool (要額外include)
#define true 1
#define false 0
#define bool unsigned char
#define PATH_MAXSIZE 4096

void myrs(const char* path,const int level){
    DIR* dirptr = opendir(path);	// 開啟指定路徑的資料夾
    if(dirptr == NULL){			//打開失敗
        printf("開啟 %s 失敗\n",path);
        exit(-1);
    }

    char temp[PATH_MAXSIZE];		// 用來儲存路徑的buffer
    struct dirent* e;
    while(true){	// 嘗試把所有資聊夾內的所有東西輸出出來 如果是資料夾就遞迴下去找
        e = readdir(dirptr);
        if(e==NULL){	// 沒其他東西了 離開
            break;
        }

        if(strcmp(e->d_name,".")!=0 && strcmp(e->d_name,"..")!=0){ // 不能對這兩個遞迴 會無窮遞迴
            for(int i=0;i<level;++i){		// 輸出樹狀圖的線
                printf("|   ");
            }
            printf("|-- %s\n",e->d_name);	// 輸出樹狀圖的線
	
	    // 造出該資料夾中該檔案的路徑
            strcpy(temp,path);
            strcat(temp,"/");
            strcat(temp,e->d_name);
            struct stat s;		// stat 規定的結構 存查詢的結果
            if(stat(temp,&s)<0){	// 把路徑和儲存結構丟進去執行
                printf("ERROR: %s\n",strerror(errno)); // 輸出是怎樣的錯誤
                exit(-1);
            }
            if(S_ISDIR(s.st_mode)){	// 如果是資料夾
                myrs(temp,level+1);	// 就往下遞迴
            }
        }
    }
    return;
}

// 輸出程式的argument
void usage(){
    printf("用法:\n");
    printf("從當前路徑往下遞迴列出所有檔案./HW7_rs\n");
    printf("從指定路徑往下遞迴列出所有檔案./HW7_rs [路徑]\n");
}

int main(int argc,char* argv[]){
    // 這邊就是處理參數 對應該怎樣執行
    if(argc==1){	// default 從當前路徑往下遞迴列出所有檔案
        char path[PATH_MAXSIZE];	// 暫存路徑
        getcwd(path,PATH_MAXSIZE);	// 得到當前路徑
        printf("%s\n",path);
        myrs(path,0);	// 往下遞迴輸出
    }else if(argc==2){	// 從指定路徑往下遞迴列出所有檔案
        printf("%s\n",argv[1]);
        myrs(argv[1],0);	// 往下遞迴輸出
    }else{
        usage();
    }
}
