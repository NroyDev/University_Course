#include "types.h"
#include "param.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"
#include "stat.h"

void check_superblock(struct superblock*);
void check_inodes(struct superblock* sb,unsigned char** mapinode,unsigned char** mapblock);
void check_directories(struct superblock* sb,unsigned char** mapinode,unsigned short* linkcount);
void check_bitmap(struct superblock* sb,unsigned char** mapblock);
void check_link(struct superblock* sb,unsigned char** mapinode,unsigned short* linkcount);

void* myalloc(unsigned int bytes);
void myfree(void* mem,unsigned int bytes);
int nbmap;

int fsck(){
    printf("[fsck] 開始檢查...\n");

    // 把 superblock 讀出來
    // 根據 fs.h 的敘述
    //      Disk layout:
    //      [ boot block | super block | log | inode blocks | free bit map | data blocks]
    // 可以知道 superblock 跟上課教的一樣在 block 1
    struct superblock sb;
    struct buf *bp;
    bp = bread(ROOTDEV, 1); // superblock 在block 1
    memmove(&sb, bp->data, sizeof(struct superblock));
    brelse(bp);
    nbmap = (sb.nblocks + BPB-1) / BPB;    //無條件進位

    // 分配空間
    unsigned char** mapinode = myalloc(sb.ninodes);	//mapinode[i]存 inode i是否有分配
    for(int i=0;i<sb.ninodes;++i){	// 初始化為0
        mapinode[i/PGSIZE][i%PGSIZE] = 0;
    }
    unsigned char** mapblock = myalloc(sb.size);	//mapblock[i]存 block i是否有分配
    for(int i=0;i<sb.size;++i){	// 初始化為0
        mapblock[i/PGSIZE][i%PGSIZE] = 0;
    }
    // unsigned short** linkcount = myalloc(sb.ninodes*sizeof(unsigned short));
    // for(int i=0;i<sb.ninodes;++i){
    //     mapinode[i/(PGSIZE/sizeof(unsigned short))][i%(PGSIZE/sizeof(unsigned short))] = 0;
    // }
    unsigned short linkcount[sb.ninodes];	// 存link count
    for(int i=0;i<sb.ninodes;++i){	// 初始化為0
        linkcount[i] = 0;
    }
    //printf("sb.size = %d\n",sb.size);
	
	// 進行檢查
    check_superblock(&sb);
    check_inodes(&sb,mapinode,mapblock);
    check_directories(&sb,mapinode,linkcount);
    check_bitmap(&sb,mapblock);
    check_link(&sb,mapinode,linkcount);
	
	// 釋放空間
    myfree(mapinode,sb.ninodes);
    myfree(mapblock,sb.size);
    // myfree(linkcount,sb.ninodes*sizeof(unsigned short));
    printf("[fsck] 檢查完畢\n");

    return 0;
}

// 分配空間  kernel中沒有malloc 用kalloc
// 這邊一律分配成2層的
void* myalloc(unsigned int bytes){
    unsigned char** table = kalloc();
    for(int i=0;i<bytes/PGSIZE+1;++i){
        table[i] = kalloc();
    }
    return table;
}
// 用來釋放空間的
void myfree(void* mem,unsigned int bytes){
    unsigned char** table = mem;
    for(int i=0;i<bytes/PGSIZE+1;++i){
        kfree(table[i]);
    }
    kfree(mem);
}
// 讀取硬碟上指定的位置 存到buffer中
void read_block(int block, void *buffer){   // 第幾個block, buffer
    struct buf *bp;
    bp = bread(ROOTDEV, block);
    memmove(buffer, bp->data, BSIZE);
    brelse(bp);
}
// 字串比較
int strcmp(char* a,char* b){
    int idx = 0;
    // 一個個比
    while(a[idx]!='\0' && b[idx]!='\0'){
        if(a[idx]>b[idx]){
            return 1;
        }else if(a[idx]<b[idx]){
            return -1;
        }
        ++idx;
    }
    if(a[idx]=='\0' && b[idx]!='\0'){
        return -1;
    }else if(a[idx]!='\0' && b[idx]=='\0'){
        return 1;
    }
    return 0;	// 0代表兩字串相等
}

// 檢查superblock有沒有問題
void check_superblock(struct superblock* sb){
    printf("    [fsck] 開始檢查 Superblock...\n");
    if(sb->magic != FSMAGIC){	// 檢查magic number
        printf("    [fsck] Error - superblock 內存的 magic number 與 xv6規定的不一樣!\n");
    }
    if(sb->size <= 0 || sb->nblocks <= 0 || sb->ninodes <= 0){	// 大小必不小於等於0
        printf("    [fsck] Error - superblock 內的 size, nblocks 或 ninodes <= 0\n");
    }
    if(sb->logstart == 0 || sb->inodestart == 0 || sb->bmapstart == 0){
        printf("    [fsck] Error - log 或 inode 或 bitmap 的起始 block 是 0\n");
    }
    if(sb->logstart >= sb->size || sb->inodestart >= sb->size || sb->bmapstart >= sb->size){
         printf("    [fsck] Error - log 或 inode 或 bitmap 的起始 block 超出範圍\n");
    }

    if((sb->logstart+sb->nlog>sb->size) || (sb->inodestart+sb->ninodes>sb->size) || (sb->bmapstart+nbmap>sb->size)){
         printf("    [fsck] Error - log 或 inode 或 bitmap 存的block會超出範圍\n");
    }

    // 根據 fs.h 的敘述
    //      Disk layout:
    //      [ boot block | super block | log | inode blocks | free bit map | data blocks]
    if(!(sb->logstart < sb->inodestart && sb->inodestart < sb->bmapstart)){
        printf("    [fsck] Error - log, inode, bitmap 的順序與xv6定義不同 [log | inode blocks | free bit map] (起始位置有問題)\n");
    }
  //  if(sb->logstart+sb->nlog>sb->inodestart){
  //       printf("    [fsck] Error - Log 與 inode 的 block 重疊\n");
  //  }
  //   if(sb->inodestart+sb->ninodes>sb->bmapstart){
  //       printf("    [fsck] Error - Inode 與 bitmap 的 block 重疊\n");
  //  }

    printf("    [fsck] Superblock 檢查完畢\n");
}

// 檢查 Inode 的 direct 和 indirect block
// 順便將哪些block有被分配的記錄下來
void check_inodes(struct superblock* sb,unsigned char** mapinode,unsigned char** mapblock){
    printf("    [fsck] 開始檢查 Inode的direct indirect...\n");
    int indirect_block_addrs[NINDIRECT];    // indrect block下存的address們


    // 讀取每個 inode
    char buf[BSIZE];
    for(int i=0; i<sb->ninodes;++i){
        // 讀取inode的資料
        struct dinode current_inode;
        int blockpos = sb->inodestart + (i/IPB);  // 這個inode所在的block位置
        read_block(blockpos,buf);
        current_inode = ((struct dinode*)buf)[i%IPB];

        if(current_inode.type == 0){    // 未被分配的
            mapinode[i/PGSIZE][i%PGSIZE] = 0;
            continue;
        }
        mapinode[i/PGSIZE][i%PGSIZE] = 1;

        if(current_inode.type < 0 || current_inode.type > T_DEVICE){
            printf("    [fsck] Error - inode %d 的 type 不正確: %d\n", i, current_inode.type);
        }

        // 檢查inode中的 direct部份
        for(int j=0;j<NDIRECT;++j){
            int direct_addr = current_inode.addrs[j];
            if(direct_addr == 0){ //沒有東西
                continue;
            }

            // 根據 fs.h 的敘述
            //      Disk layout:
            //      [ boot block | super block | log | inode blocks | free bit map | data blocks]
            if(direct_addr >= sb->size || direct_addr < sb->bmapstart+nbmap ){
                 printf("    [fsck] Error inode %d 的 direct address %d 超出範圍\n", i, direct_addr);
            }else{
                if(mapblock[direct_addr/PGSIZE][direct_addr%PGSIZE] == 1){
                    printf("    [fsck] Error - %d 這個位置被重複對應到 違反一個block只能被一個inode對到的原則 (在inode %d 的direct重複偵測到)\n", direct_addr, i);
                }
                mapblock[direct_addr/PGSIZE][direct_addr%PGSIZE] = 1;
            }
        }

        // 檢查 indirect的部份
        int indirect_addr = current_inode.addrs[NDIRECT];
        if(indirect_addr != 0){
            //      Disk layout:
            //      [ boot block | super block | log | inode blocks | free bit map | data blocks]
            if(indirect_addr >= sb->size || indirect_addr < sb->bmapstart+nbmap){
                 printf("    [fsck] Error inode %d 的 indirect address %d 超出範圍\n", i, indirect_addr);
            }else{
                if(mapblock[indirect_addr/PGSIZE][indirect_addr%PGSIZE] == 1){
                    printf("    [fsck] Error - %d 這個位置被重複對應到 違反一個block只能被一個inode對到的原則 (在inode %d 的indirect重複偵測到)\n", indirect_addr, i);
                }
                mapblock[indirect_addr/PGSIZE][indirect_addr%PGSIZE] = 1;

                // 往下讀indirect內究竟存了哪些address
                read_block(indirect_addr, (char*)indirect_block_addrs);
                for(int j=0;j<NINDIRECT;++j){
                    int in_indirect_addr = indirect_block_addrs[j];
                    if(in_indirect_addr == 0){  //沒有東西
                        continue;
                    }

            		//      Disk layout:
            		//      [ boot block | super block | log | inode blocks | free bit map | data blocks]
                    if(in_indirect_addr >= sb->size || in_indirect_addr < sb->bmapstart+nbmap){
                        printf("    [fsck] Error inode %d 的 indirect block存的 address %d 超出範圍\n", i, in_indirect_addr);
                    }else{
                        if(mapblock[in_indirect_addr/PGSIZE][in_indirect_addr%PGSIZE] == 1){
                        printf("    [fsck] Error - %d 這個位置被重複對應到 違反一個block只能被一個inode對到的原則 (在inode %d 的indirect下的address重複偵測到)\n", in_indirect_addr, i);
                        }
                        mapblock[in_indirect_addr/PGSIZE][in_indirect_addr%PGSIZE]  = 1;
                    }
                }
            }
        }
    }
    printf("    [fsck] Inode的direct indirect檢查完畢\n");
}

// 檢查Directory是否都有. ..
// 順便在走訪每個Directory的時候 紀錄每個block 的 link count
void check_directories(struct superblock* sb,unsigned char** mapinode,unsigned short* linkcount){
    printf("    [fsck] 開始檢查 Directories...\n");
    char block_buf[BSIZE];
    char buf[BSIZE];

    // 檢查每個 Directory
    for(int i = ROOTINO; i < sb->ninodes;++i){
        // 讀取inode
        struct dinode current_inode;
        int blockpos = sb->inodestart + (i/IPB);  // 這個inode所在的block位置
        read_block(blockpos,buf);
        current_inode = ((struct dinode*)buf)[i%IPB];
        // 我們要的是Directory 沒分配的也跳過
        if(mapinode[i/PGSIZE][i%PGSIZE] == 0 || current_inode.type != T_DIR){
            continue;
        }

        // 目標: 找到. ..
        int found_d = 0;
        int found_dd = 0;
        // 先掃這個inode的direct 部份
        for(int j = 0; j < NDIRECT;++j){
            int direct_addr = current_inode.addrs[j];
            if(direct_addr == 0){ // 沒東西
                continue;
            }
            // 先檢查指向的addr是否正常
            //      Disk layout:
            //      [ boot block | super block | log | inode blocks | free bit map | data blocks]
            if(direct_addr >= sb->size || direct_addr < sb->bmapstart + nbmap){
                printf("    [fsck] Error - inode %d 的direct block %d 不正確, 跳過這個direct block的檢查\n", i, direct_addr);
                continue;
            }
			
            read_block(direct_addr, block_buf);
            struct dirent *de = (struct dirent*)block_buf;
            // 便利整個block中存的dirent資料
            for(int k = 0; k < BSIZE / sizeof(struct dirent); k++, de++){
                if(de->inum == 0){ // 沒東西
                    continue;
                }
                
                //
                struct dinode temp;
                int blockpos = sb->inodestart + (de->inum/IPB);
                read_block(blockpos,buf);
                temp = ((struct dinode*)buf)[de->inum%IPB];
                if(temp.type == T_DIR){  // 存在著..指向這個資料夾
                	++linkcount[i];
                }
                //

                // 找到 . 了
                if(strcmp(de->name, ".") == 0){
                    found_d = 1;
                    // 檢查 . 是不是指向自己這個資料夾
                    if(de->inum != i){
                        printf("    [fsck] Error - (inode %d) directory 中的 . 不是指向自己而是指向 inode %d\n", i, de->inum);
                    }
                }else if(strcmp(de->name, "..") == 0){  // 找到 ..了
                    found_dd = 1;
                    // ROOTINO的..應該要指向自己
                    if(i == ROOTINO && de->inum != ROOTINO){
                        printf("    [fsck] Error - (inode %d) root directory 中的 .. 不是指向自己而是指向 inode %d\n", ROOTINO , de->inum);
                    }
                    
                    // 檢查 .. 是否指向有效的 inode
                    if(de->inum == 0 || de->inum >= sb->ninodes){ // 指向奇怪的地方 (超出範圍)
                         printf("    [fsck] Error - (inode %d) directory 中的 .. 指向無效的 inode (inode %d)\n", i, de->inum);
                    }else if(mapinode[de->inum/PGSIZE][de->inum%PGSIZE] == 0){  // 沒有被mapninode記下的inode必定有問題
                         printf("    [fsck] Error - (inode %d) directory 中的 .. 指到未被分配的 inode (inode %d)\n", i, de->inum);
                    }else{ // .. 指向的是不是資料夾
                    	/*
                        struct dinode temp;
                        // read_inode(de->inum, &temp);
                        
                        // char temp_buf[BSIZE];
                        int blockpos = sb->inodestart + (de->inum/IPB);  // 這個inode所在的block位置
                        // read_block(blockpos,temp_buf);
                        read_block(blockpos,buf);
                        // temp = ((struct dinode*)temp_buf)[de->inum%IPB];
                        temp = ((struct dinode*)buf)[de->inum%IPB];
                        */
                        if(temp.type != T_DIR){
                             printf("    [fsck] Error - (inode %d) directory 中的 .. 指向的不是directory (inode %d的type的是 %d)\n", i, de->inum, temp.type);
                        }
                    }
                }else{ // 順便檢查其他檔案(非 . ..)的inode是否有怪怪的地方
                    if(de->inum == 0 || de->inum >= sb->ninodes){ // 超出範圍
                        printf("    [fsck] Error - (inode %d) directory 中的 '%s' 指向無效的 inode (inode %d)\n", i, de->name, de->inum);
                    }else if(mapinode[de->inum/PGSIZE][de->inum%PGSIZE] == 0){
                        printf("    [fsck] EError - (inode %d) directory 中的 '%s' 指到未被分配的 inode (inode %d)\n", i, de->name, de->inum);
                    }else if(temp.type != T_DIR){ // 如果看起來正常 順便紀錄linkcount
                        ++linkcount[de->inum];
                    }
                }
            }
        }

        // 再掃這個inode的indirect 部份
        int indirect_addr = current_inode.addrs[NDIRECT];
        if(indirect_addr != 0){
            // 先檢查指向的addr是否正常
            //      Disk layout:
            //      [ boot block | super block | log | inode blocks | free bit map | data blocks]
            if(indirect_addr >= sb->size || indirect_addr < sb->bmapstart+nbmap){
                printf("    [fsck] Error - inode %d 的indirect block %d 不正確, 跳過這個indirect block的檢查\n", i, indirect_addr);
            }else{
                // int indirect_block_addrs[NINDIRECT];    // indrect block下存的address們
                // read_block(indirect_addr, (char*)indirect_block_addrs);
                read_block(indirect_addr, (char*)buf);
                for(int j=0; j < NINDIRECT; ++j){
                    int addr = ((int*)buf)[j];
                    if(addr == 0){  // 沒東西跳過
                        continue;
                    }
                    // 先檢查指向的addr是否正常
            		//      Disk layout:
            		//      [ boot block | super block | log | inode blocks | free bit map | data blocks]
                    if(addr >= sb->size || addr < sb->bmapstart+nbmap){
                        printf("    [fsck] Error - inode %d 的indirect block中存的addr %d 不正確, 跳過這個位置的檢查\n", i, addr);
                        continue;
                    }

                    read_block(addr, block_buf);
                    struct dirent *de = (struct dirent*)block_buf;
                    // 便利整個block中存的dirent資料
                    // 維護link count順便檢查
                    for(int k = 0; k < BSIZE / sizeof(struct dirent); k++, de++){
                        if(de->inum == 0){ // 沒東西
                            continue;
                        }
                        //if(strcmp(de->name, ".") != 0 && strcmp(de->name, "..") != 0){
                        if(de->inum == 0 || de->inum >= sb->ninodes){	// 超出範圍
                            printf("    [fsck] Error - (inode %d) directory 中的 '%s' 指向無效的 inode (inode %d)\n", i, de->name, de->inum);
                        }else if(mapinode[de->inum/PGSIZE][de->inum%PGSIZE] == 0){
                            printf("    [fsck] Error - (inode %d) directory 中的 '%s' 指到未被分配的 inode (inode %d)\n", i, de->name, de->inum);
                        }else{
                                //++linkcount[de->inum];
                        }
                        //}
                    }
                }
            }
        }

        if(!found_d){
            printf("    [fsck] Error - (inode %d) directory中找不到 .\n", i);
        }
        if(!found_dd){
            printf("    [fsck] Error - (inode %d) directory中找不到 ..\n", i);
        }
    }
    printf("    [fsck] Directory 檢查完畢\n");
}


// 檢查 bitmap的對應是否正常
// 看有沒有被bitmap標出來 但實際沒有使用的
// 或看看有沒有實際上有在用的 但沒被bitmap標出來
void check_bitmap(struct superblock* sb,unsigned char** mapblock){
    printf("    [fsck] 開始檢查 Bitmap...\n");

    char buf[BSIZE];
    // 根據 fs.h 的敘述
    //      Disk layout:
    //      [ boot block | super block | log | inode blocks | free bit map | data blocks]
    int start = sb->bmapstart + nbmap; // data block 的起點

	// 檢查每個data block
    for(int i=0; i< sb->nblocks;++i){
        int target_block = start + i;
		
		// 還是檢查一下現在要看的位置是否超過範圍 因為我們得到的資料可能會是壞掉的(superblock)
        if(target_block >= sb->size) {
            printf("    [fsck] Error- 檢查的位置 %d 超出範圍  (%d>=%d)\n", target_block, target_block, sb->size);
            continue;
        }

		// 讀取這個data block對應bitmap對應的block
        read_block(sb->bmapstart + (i/BPB), buf);
        // 在從讀出來的block取出對應的bit
        char bit = (buf[(i % BPB) / 8] >> ((i % BPB) % 8)) & 1;
        if(bit == 1 && mapblock[target_block/PGSIZE][target_block%PGSIZE] == 0) {
            printf("    [fsck] Error - bitmap 說 block %d(第 %d data block)已分配 但實際上沒有使用\n", target_block, i);
        }
        if(bit == 0 && mapblock[target_block/PGSIZE][target_block%PGSIZE] == 1) {
            printf("    [fsck]] Error - bitmap 說 block %d(第 %d data block)沒分配 但實際上有使用\n", target_block, i);
        }
    }
    printf("    [fsck] Bitmap 檢查完畢\n");
}


// 檢查linkcount
void check_link(struct superblock* sb,unsigned char** mapinode,unsigned short* linkcount){
    printf("    [fsck] 開始檢查Inode的Link Counts...\n");

    char buf[BSIZE];
    // 讀取每個block 看看他的link count有沒有問題
    for(int i = ROOTINO; i < sb->ninodes;++i){
        struct dinode current_inode;
        int blockpos = sb->inodestart + (i/IPB);  // 這個inode所在的block位置
        read_block(blockpos,buf);
        current_inode = ((struct dinode*)buf)[i%IPB];

        if(mapinode[i/PGSIZE][i%PGSIZE]){  // 是有分配的Inode
        	// directory至少會有 . 和 .. => link count >=2
            if(i!= ROOTINO && current_inode.type == T_DIR && current_inode.nlink < 2) {
                printf("    [fsck] Error - inode %d 是 directory inode 但其link count < 2\n", i);
            }
            // 我們算出的link count應該要與硬碟上 inode存的nlink一致
            if(i!= ROOTINO && current_inode.nlink != linkcount[i]){
                    printf("    [fsck] Error - inode %d 的link cout不一致  存在硬碟上的link count: %d, fsck算出的link count: %d.\n",i, current_inode.nlink, linkcount[i]);
            }

            if(current_inode.type != 0 && current_inode.nlink == 0) { // 任何已分配的 Inode nlink 不應為 0
                 printf("    [fsck] Error inode %d 已被分配 但它的 count link == 0 (存在著無法找到的檔案)\n",i);
            }

        }else{ // 沒有被分配的inode
            if(current_inode.type != 0) { // type 應該也是 0
                printf("    [fsck] Error - inode %d沒有被分配 但其type不為0 (其type為%d)\n", i, current_inode.type);
            }
            if(current_inode.nlink > 0) { // 其存的link count應為0
                printf("    [fsck] Error - inode %d沒有被分配  但其link count不為0 (其link count為%d)\n", i, current_inode.nlink);
            }
            if(linkcount[i] > 0) {	// 不應該有人link到它
                printf("    [fsck] Error - inode %d沒有被分配 但被指到 %d次\n", i, linkcount[i]);
            }
        }
    }
    printf("    [fsck] Inode link count 檢查完畢\n");
}
