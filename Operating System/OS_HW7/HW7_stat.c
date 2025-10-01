#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<unistd.h>
#include<time.h>

#include<errno.h>
#include<string.h>
#define false 0
#define true 1
#define bool unsigned char

// 用來輸出指定路徑上的資料 如果detail==true 會詳系列出資料
void mystat(char* path,bool detail){
	// 有參考 在terminal 打 man 'stat(2)'
	struct stat s;		// stat 規定的結構 存查詢的結果
	if(stat(path,&s)<0){	// 把路徑和儲存結構丟進去執行
		printf("ERROR: %s\n",strerror(errno)); // 輸出是怎樣的錯誤
		exit(-1);
	}
	
	// 下面就是輸出 把除存查詢資料的結果輸出出來 是怎樣的資料看print的左側
	printf("stat: '%s' 的一些檔案資訊:\n",path);
	printf("  檔案大小(File Size): %ld bytes\n",s.st_size);
	printf("  已分配的block數(Allocated Blocks): %ld\n",s.st_blocks);
	printf("  Link counts: %ld\n",s.st_nlink);
	if(detail){	// 如果detail==true 會詳系列出資料
		printf("  --------------------\n");
		printf("  IO Block Size: 	%ld\n",s.st_blksize);
		printf("  I-node number: 	%ld\n", s.st_ino);
		printf("  Mode: 		%o\n",s.st_mode);
		printf("    - User  權限: ");
		if(s.st_mode & S_IRUSR){printf("讀 ");}		// 讀
		if(s.st_mode & S_IWUSR){printf("寫 ");}		// 寫
		if(s.st_mode & S_IXUSR){printf("執行 ");}	// 執行
		printf("\n");
		printf("    - Group 權限: ");
		if(s.st_mode & S_IRGRP){printf("讀 ");}		// 讀
		if(s.st_mode & S_IWGRP){printf("寫 ");}		// 寫
		if(s.st_mode & S_IXGRP){printf("執行 ");}	// 執行
		// 有哪些權限 Other
		printf("\n");
		printf("    - Other 權限: ");
		if(s.st_mode & S_IROTH){printf("讀 ");}		// 讀
		if(s.st_mode & S_IWOTH){printf("寫 ");}		// 寫
		if(s.st_mode & S_IXOTH){printf("執行 ");}	// 執行
		printf("\n");

		printf("  擁有者 UID: 		%d\n",s.st_uid);
		printf("  群組 GID: 		%d\n",s.st_gid);
		printf("  檔案類型:             ");
		switch (s.st_mode & S_IFMT) {
		   case S_IFDIR:  printf("資料夾\n");                   break;
		   case S_IFREG:  printf("一般檔案\n");                 break;
		   case S_IFBLK:  printf("block device\n");            break;
		   case S_IFCHR:  printf("character device\n");        break;
		   case S_IFIFO:  printf("FIFO/pipe\n");               break;
		   case S_IFLNK:  printf("symlink\n");                 break;
		   case S_IFSOCK: printf("socket\n");                  break;
		   default:       printf("unknown?\n");                break;
	   }
	   printf("  --------------------\n");
	   printf("  上次存取時間(Time of last access): \t\t%s", ctime(&s.st_atime));
	   printf("  上次修改時間(Time of last modification): \t%s", ctime(&s.st_mtime));
	   printf("  上次狀態改變時間(Last status change): \t%s", ctime(&s.st_ctime));
	}

	return;
}

// 觀察題目的link count 變化
void observe(){
	char* path = "./mystat_ovserve";
	char temp[512]={'\0'};	// 暫存路徑用

	printf("create %s\n", path);
	if(mkdir(path,S_IRWXU)<0){ // 創見一個資料夾 其權限是可寫可讀可執行
		printf("ERROR: %s\n",strerror(errno)); // 輸出是怎樣的錯誤
		exit(-1);
	}
	mystat(path,false);	// 輸出簡略資料 讓我們可以觀察link count
	printf("-------------------------------------------------------\n");
	
	printf("在 %s 中 create一個資料夾\n",path);
	strcpy(temp,path);
	strcat(temp,"/1"); // 心資料夾
	if(mkdir(temp,S_IRWXU)<0){ // 創見一個資料夾 其權限是可寫可讀可執行
		printf("ERROR: %s\n",strerror(errno)); // 輸出是怎樣的錯誤
		exit(-1);
	}
	mystat(path,false);	// 輸出簡略資料 讓我們可以觀察link count
	printf("-------------------------------------------------------\n");
	printf("在 %s 中 create一個資料夾\n",path);
	strcpy(temp,path);
	strcat(temp,"/2");
	if(mkdir(temp,S_IRWXU)<0){ // 創見一個資料夾 其權限是可寫可讀可執行
		printf("ERROR: %s\n",strerror(errno)); // 輸出是怎樣的錯誤
		exit(-1);
	}
	mystat(path,false);	// 輸出簡略資料 讓我們可以觀察link count
	printf("-------------------------------------------------------\n");
	printf("在 %s 中 create一個資料夾\n",path);
	strcpy(temp,path);
	strcat(temp,"/3");
	if(mkdir(temp,S_IRWXU)<0){ // 創見一個資料夾 其權限是可寫可讀可執行
		printf("ERROR: %s\n",strerror(errno)); // 輸出是怎樣的錯誤
		exit(-1);
	}
	mystat(path,false);	// 輸出簡略資料 讓我們可以觀察link count
	printf("-------------------------------------------------------\n");
	strcpy(temp,path);
	strcat(temp,"/1");
	printf("刪除 %s\n", temp);
	if(rmdir(temp)<0){ // 刪除資料夾 讓下次observe 不會遇到已經建立了資料夾的ERROR
		printf("ERROR: %s\n",strerror(errno)); // 輸出是怎樣的錯誤
		exit(-1);
	}
	mystat(path,false);	// 輸出簡略資料 讓我們可以觀察link count
	printf("-------------------------------------------------------\n");
	strcpy(temp,path);
	strcat(temp,"/2");
	printf("刪除 %s\n", temp);
	if(rmdir(temp)<0){ // 刪除資料夾 讓下次observe 不會遇到已經建立了資料夾的ERROR
		printf("ERROR: %s\n",strerror(errno)); // 輸出是怎樣的錯誤
		exit(-1);
	}
	mystat(path,false);	// 輸出簡略資料 讓我們可以觀察link count
	printf("-------------------------------------------------------\n");
	strcpy(temp,path);
	strcat(temp,"/3");
	printf("刪除 %s\n", temp);
	if(rmdir(temp)<0){ // 刪除資料夾 讓下次observe 不會遇到已經建立了資料夾的ERROR
		printf("ERROR: %s\n",strerror(errno)); // 輸出是怎樣的錯誤
		exit(-1);
	}
	mystat(path,false);	// 輸出簡略資料 讓我們可以觀察link count
	printf("-------------------------------------------------------\n");
	printf("刪除 %s\n", path);
	if(rmdir(path)<0){ // 刪除資料夾 讓下次observe 不會遇到已經建立了資料夾的ERROR	
		printf("ERROR: %s\n",strerror(errno)); // 輸出是怎樣的錯誤
		exit(-1);
	}

	printf("在初始創建資料夾時 該資料夾就存在兩個link\n");
	printf("可以發現當該資料夾下面的資料夾越多, 其link count就越大\n");
	printf("在該資料夾底下創建一個資料夾 link count++\n");
	printf("在該資料夾底下刪除一個資料夾 link count--\n");
	printf("後來去查資料 其實是其底下中的資料夾中有個../的路徑指向它 他自己有./的路徑指向它\n");
	printf("所以創建=>../ 指向變多 link count++\n");
	printf("所以刪除=>../ 指向便少 link count--\n");
}

// 輸出程式的argument
void usage(){
	printf("用法:\n");
	printf("  詳細的顯示該路徑的stat資料 ./HW_stat [路徑]\n");
	printf("  簡略的顯示該路徑的stat資料 ./HW_stat -s [路徑]\n");
	printf("  題目中要做的觀察          ./HW_stat -observe\n");
	return;
}

int main(int argc,char* argv[]){
	if(argc<2){
		printf("缺少一些參數!\n");
		usage();
	}else{	// 這邊就是處理參數 對應該怎樣執行
		if(argc==2){	// 一個而外參數
			if(strcmp(argv[1],"-observe")==0){	
				observe(); // 題目要求的觀察
			}else{
				mystat(argv[1],true);	// 詳意輸出該路徑之檔案資料
			}
		}else if(argc==3){
			if(strcmp(argv[1],"-s")==0){
				mystat(argv[2],false); // 簡略輸出該路徑之檔案資料
			}else{
				usage(); // 沒有match的指令 => 提示輸出
			}
		}else{
			usage(); // 沒有match的指令 => 提示輸出
		}
	}

	return 0;
}
