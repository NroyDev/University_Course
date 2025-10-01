#include<stdio.h>
#include<dirent.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<unistd.h>
#include<time.h>
#include<errno.h>
#include<string.h>

#include<grp.h>
#include<pwd.h>

// 這邊是方便我寫用的 C 預設沒bool (要額外include)
#define false 0
#define true 1
#define bool unsigned char
#define PATH_MAXSIZE 512

// 輸出檔案的詳細資料
void mystat(char* path){
	struct stat s;		// stat 規定的結構 存查詢的結果
	if(stat(path,&s)<0){	// 把路徑和儲存結構丟進去執行
		printf("ERROR: %s\n",strerror(errno)); // 輸出是怎樣的錯誤
		exit(-1);
	}
    
    // 有哪些權限 User
    if(s.st_mode & S_IRUSR){printf("r");}else{printf("_");}	// 讀
    if(s.st_mode & S_IWUSR){printf("w");}else{printf("_");}	// 寫
    if(s.st_mode & S_IXUSR){printf("x");}else{printf("_");}	// 執行
    // 有哪些權限 Group
    if(s.st_mode & S_IRGRP){printf("r");}else{printf("_");}	// 讀
    if(s.st_mode & S_IWGRP){printf("w");}else{printf("_");}	// 寫
    if(s.st_mode & S_IXGRP){printf("x");}else{printf("_");}	// 執行
    // 有哪些權限 Other
    if(s.st_mode & S_IROTH){printf("r");}else{printf("_");}	// 讀
    if(s.st_mode & S_IWOTH){printf("w");}else{printf("_");}	// 寫
    if(s.st_mode & S_IXOTH){printf("x");}else{printf("_");}	// 執行
    printf(" ");
    printf("%5s ",getpwuid(s.st_uid)->pw_name);	// User 名字
    printf("%5s ",getgrgid(s.st_gid)->gr_name);	// Group 名字
    printf("%4ld ",s.st_nlink);	// link count
    printf("%8ld ",s.st_size);	// 檔案大小(bytes)
    // 檔案類型
    switch (s.st_mode & S_IFMT) {
        case S_IFDIR:  printf("  資料夾 ");      break;
        case S_IFREG:  printf("一般檔案 ");      break;
        default:       printf("   其他 ");       break;
    }
    
    char timeb[20]; // 存時間的字串
    strftime(timeb,20,"%Y/%m/%d %H:%M",localtime(&s.st_mtime)); // 重新格式化字串 以比較小的(字串長度短點)除存
    printf("%16s |", timeb);	// 修改時間
    
    //printf("%5d %5d %4ld %4o %5ld %16s ",s.st_uid,s.st_gid,s.st_nlink,s.st_mode&(512-1),s.st_size,timeb); // 輸出檔案的詳細資料

	return;
}

// 輸出資料夾內的檔案
void mylistfile(const char* path,bool detail){
    DIR* dirptr = opendir(path);    //打開資料夾
    if(dirptr == NULL){             //打開失敗
        printf("開啟 %s 失敗\n",path);
        exit(-1);
    }

    printf("'%s' 內的檔案們:\n",path);
    char temp[PATH_MAXSIZE]; //存路徑的

    struct dirent* e;
    if(detail){ //要詳細顯示
        printf("   (U/G/O) USER GROUP LINK 檔案大小\n"); // 輸出表的header
    	printf("      權限 名稱  名稱   數  (Bytes) 檔案類型     最後修改時間 | 檔案名稱\n"); // 輸出表的header
    	printf("--------------------------------------------------------------+--------\n");
    }
    // 嘗試把所有資聊夾內的所有東西輸出出來
    while(true){
        e = readdir(dirptr);
        if(e==NULL){    // 沒其他東西了 離開
            break;
        }

        if(detail){//要詳細顯示
            // 處理路徑
            strcpy(temp,path);  
            strcat(temp,"/");
            strcat(temp,e->d_name);
            // 輸出該檔案的詳細資料
            mystat(temp);
            printf(" ");
        }
        // 輸出檔案名子
        printf("%s\n",e->d_name);
    }
}

// 輸出程式的argument
void usage(){
    printf("用法:\n");
    printf("  顯示當前資料夾的所有檔案                 ./HW7_listfiles\n");
    printf("  顯示當前資料夾的所有檔案(包括檔案的詳細資料)./HW7_listfiles -l\n");
    printf("  顯示指定路徑中的所有檔案                 ./HW7_listfiles [路徑]\n");
    printf("  顯示指定路徑中的所有檔案(包括檔案的詳細資料)./HW7_listfiles -l [路徑]\n");
}

int main(int argc,char* argv[]){
    // 這邊就是處理參數 對應該怎樣執行
    if(argc==1){    // 顯示當前資料夾的所有檔案
        char cwd[PATH_MAXSIZE];
        getcwd(cwd,PATH_MAXSIZE); // 得到當前路徑
        mylistfile(cwd,false);
    }else if(argc==2){  
        if(strcmp(argv[1],"-l")==0){    // 顯示當前資料夾的所有檔案(包括檔案的詳細資料)
            char cwd[PATH_MAXSIZE];
            getcwd(cwd,PATH_MAXSIZE);   // 得到當前路徑
            mylistfile(cwd,true);
        }else{                          // 顯示指定路徑中的所有檔案
            mylistfile(argv[1],false);
        }
    }else if(argc==3){
        if(strcmp(argv[1],"-l")==0){    // 顯示指定路徑中的所有檔案(包括檔案的詳細資料)
            mylistfile(argv[2],true);
        }else{
            usage();
        }
    }else{
        usage();
    }

    return 0;
}
