#include "types.h"
#include "param.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"
#include "stat.h"

// 呼叫這個就能引發系統崩潰 讓我們方便測試系統
void crashtest(){
	panic("系統崩潰了 (故意)");
}
