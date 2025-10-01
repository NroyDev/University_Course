
        /*
        // 測試 掃資料夾下的所有檔案
        for(int j=0;j<NDIRECT;++j){
        	int direct_addr = current_inode.addrs[j];
             if(direct_addr == 0){ // 沒東西
                 continue;
             }
             read_block(direct_addr, block_buf);
             struct dirent *de = (struct dirent*)block_buf;
             for(int k = 0; k < BSIZE / sizeof(struct dirent); k++, ++de){
             	if(de->inum == 0){ // 沒東西
                     continue;
                 }
                 printf("inode: %d name: %s\n",de->inum,de->name);
             }
        }
        int indirect_addr = current_inode.addrs[NDIRECT];
        if(indirect_addr != 0){
        	read_block(indirect_addr, (char*)buf);
            for(int j=0; j < NINDIRECT; ++j){
                int addr = ((int*)buf)[j];
                if(addr == 0){  // 沒東西跳過
                    continue;
                }
                
                read_block(addr, block_buf);
             	struct dirent *de = (struct dirent*)block_buf;
             	for(int k = 0; k < BSIZE / sizeof(struct dirent); k++, ++de){
             		if(de->inum == 0){ // 沒東西
               	      continue;
              	   }
              	   printf("inode: %d name: %s\n",de->inum,de->name);
             	}
            }
        }
        */
