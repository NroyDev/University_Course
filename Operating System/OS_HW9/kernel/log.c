#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"

struct logheader {
  int n;				// 當前的transaction 中有幾筆log
  int block[LOGSIZE];	// 第i比log 要寫到block[i]這個block 
};

struct log {
  struct spinlock lock;	// 整個log的鎖 確保不會同時有好幾個process同時操作 造成遺些奇怪的事情發生
  int start;			// 從disk 上的哪個block開始寫
  int size;				// 多少個block log可以寫 
  int outstanding; 		// 多少transaction進行中(transaction begin 還沒transaction end)
  int committing;		// 是否正在commit 
  int dev;				// 寫到哪個devce
  struct logheader lh;
};
struct log log;

static void recover_from_log(void);
static void commit();

// 系統啟動時 呼叫的函數
// 負責初始話 log
void
initlog(int dev, struct superblock *sb)
{
  if (sizeof(struct logheader) >= BSIZE)
    panic("initlog: too big logheader");

  // 把鎖和一些資料田進去
  initlock(&log.lock, "log");	// 鎖初始話
  log.start = sb->logstart;		// 起點
  log.size = sb->nlog;			// 大小
  log.dev = dev;				// decvice
  recover_from_log();
}

// 把disk上log區域的資料 搬到data區域 
static void
install_trans(int recovering)
{
  // 把log的資料全部搬回data區域 有多少 搬到哪裡都記載header中
  int tail;

  for (tail = 0; tail < log.lh.n; tail++) {
    struct buf *lbuf = bread(log.dev, log.start+tail+1); 	// from log
    struct buf *dbuf = bread(log.dev, log.lh.block[tail]);  // to   data
    memmove(dbuf->data, lbuf->data, BSIZE);  // 複製過去
    bwrite(dbuf);  // 寫入Disk
    if(recovering == 0)
      bunpin(dbuf);
    brelse(lbuf);
    brelse(dbuf);
  }
}

// 從硬碟上讀取 log header 的資料
static void
read_head(void)
{
  // 從硬碟上讀出來 我們把header 放在 log.start這個位置 其他log的資料從log.start+1的這個位置擺起
  struct buf *buf = bread(log.dev, log.start);
  struct logheader *lh = (struct logheader *) (buf->data);
  int i;
  log.lh.n = lh->n;					// 把從disk中讀到的header中 block# 的資料一個個的存到記憶體中
  for (i = 0; i < log.lh.n; i++) {	// 把從disk中讀到的header中 block# 的資料一個個的存到記憶體中
    log.lh.block[i] = lh->block[i];
  }
  brelse(buf);
}

// log header 的資料 寫回硬碟
static void
write_head(void)
{
  // 我們把header 放在 log.start這個位置 其他log的資料從log.start+1的這個位置擺起
  struct buf *buf = bread(log.dev, log.start);
  struct logheader *hb = (struct logheader *) (buf->data);
  int i;
  hb->n = log.lh.n;					// 把記憶體中的資料 一個個存到準備寫回disk的buffer中
  for (i = 0; i < log.lh.n; i++) {	// 把記憶體中的資料 一個個存到準備寫回disk的buffer中
    hb->block[i] = log.lh.block[i];
  }
  bwrite(buf);
  brelse(buf);
}

static void
recover_from_log(void)
{
  // 如果log中還有資料沒搬到硬碟上 => 就搬過去
  read_head();		// 把header的資料讀起來 方便待會recover檢查有沒有東西
  install_trans(1); // 把disk上log區域的資料 搬到data區域 
  log.lh.n = 0;		// 全部搬完後 就把當前的log數量清零 因為都搬完了
  write_head();		// 把header的更新寫回硬碟
}

// 如其名 開始一個transcation
void
begin_op(void)
{
  // 先把鎖拿起來 確保不會在接下來的地方context swith過去回來後某些資料改變 導致奇怪的行為發生
  acquire(&log.lock);
  while(1){
    if(log.committing){			// 如果正在commit也先等等
      sleep(&log, &log.lock);		// 好耶 睡覺
    } else if(log.lh.n + (log.outstanding+1)*MAXOPBLOCKS > LOGSIZE){
      sleep(&log, &log.lock);		// 好耶 睡覺
    } else {
      log.outstanding += 1;		// 更新狀態 現在系統中有v新人在進行transaction
      release(&log.lock);		// 該更新判斷的都結束了 放鎖
      break;					// 離開
    }
  }
}

// 如其名 結束transcation 把東西commit到硬碟上
void
end_op(void)
{
  int do_commit = 0;

  // 先把鎖拿起來 確保不會在接下來的地方context swith過去回來後某些資料改變 導致奇怪的行為發生
  acquire(&log.lock);
  log.outstanding -= 1;
  if(log.committing)	// 明明還有人沒結束 但正在commit => 奇怪
    panic("log.committing");
  if(log.outstanding == 0){	// 沒人在transaction了 可以commit 收工回家了
    do_commit = 1;
    log.committing = 1;
  } else {
    wakeup(&log);
  }
  release(&log.lock);

  if(do_commit){ // 可以commit
    commit();
    // 先把鎖拿起來 確保不會在接下來的地方context swith過去回來後某些資料改變 導致奇怪的行為發生
    acquire(&log.lock);
    log.committing = 0;
    wakeup(&log);
    release(&log.lock);
  }
}

// 把記憶體中的log資料 寫到硬碟上
static void
write_log(void)
{
  // 我們把header 放在 log.start這個位置 其他log的資料從log.start+1的這個位置擺起
  int tail;

  for (tail = 0; tail < log.lh.n; tail++) {
    struct buf *to = bread(log.dev, log.start+tail+1);     // 硬碟上的log block
    struct buf *from = bread(log.dev, log.lh.block[tail]); // 記憶體
    memmove(to->data, from->data, BSIZE);
    bwrite(to);  // 寫回硬碟
    brelse(from);
    brelse(to);
  }
}

// commit到硬碟上
static void
commit()
{
  if (log.lh.n > 0) {	// 如果有東西可以寫到disk
    write_log();     	// 把記憶體中的log資料 寫到硬碟上
    write_head();    	// 當前的header也先寫到硬碟上 防止待會搬移中斷電等異常發生 方便loginit可以replay
    install_trans(0); 	// 把log區域的資料搬回dataf區域 
    log.lh.n = 0;
    write_head();    	// 東西完成了 header更新回硬碟
  }
}

// 用log_write()代替bwrite():
// 外面原本用bwrite的地方都換成這個
// 讓所有寫入都透過這個 journal system 寫入
void
log_write(struct buf *b)
{
  int i;

  // 先把鎖拿起來 確保不會在接下來的地方context swith過去回來後某些資料改變 導致奇怪的行為發生
  acquire(&log.lock);
  if (log.lh.n >= LOGSIZE || log.lh.n >= log.size - 1)
    panic("too big a transaction");
  if (log.outstanding < 1)
    panic("log_write outside of trans");

  // 尋找它應該要在哪裡
  for (i = 0; i < log.lh.n; i++) {
    if (log.lh.block[i] == b->blockno) // 如果log中也有這個區塊的資料 可以直接覆蓋就資料 省空間
      break;
  }
  log.lh.block[i] = b->blockno;
  if (i == log.lh.n) {  // 如果沒有舊區域能覆蓋
    bpin(b);
    log.lh.n++;
  }
  release(&log.lock);
}

