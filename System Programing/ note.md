# Signal(Ch.06)
- raise(): 送一個 signal 給自己
    - 行為等價 kill(getpid(), SIGINT)

- signal caught/pending
- signal dispositon(處置): default action (預設)
    - 系統處理 signal 預設會是 default action(可能是kill掉), user 也可以自定義
    - 每個 signal 都有自己的 default action
    - 可透過 sigaction() 自定義 signal disposition
    - 可透過 sigignore() 忽略 signal
- Signals 究竟都有哪些呢？
    - kill -l 可以列出所有的 signal（bash的比較好看）
    - `期末考會考`： `SIGKILL` `SIGSTOP`
        - SIGKILl 和 SIGSTOP 是例外，要擋也擋不住！
    - Ctrl+C: 觸發 SIGINT
    - 系統中允許讓 user define 的 signal: SIGUSR1 SIGUSR2
    - Child process 掛掉的時候會觸發的 signal: SIGCHLD
- signal 被 block 時，是 pending(posted) 的狀態

Signal 示意圖：

    |<-----[pending(posted)]----->|
    v                             v
    |-----------------------------|----> time
    Generate                      Delivered(Caught)

- 自定義 signal handler
    - 一般來說不會有參數或回傳值，但是可以在 sigaction() 透過 siginfo_t 傳遞
- 等待訊號: sigsuspend()
    - 使用傳入的mask，暫停process，等待signal到達，恢復執行
    - 也可以 sigpause(signo) 等待特定 signal，但是
    我看 man 好像建議直接用 sigsuspend()，可能是可攜性有問題，沒那麼通用。
- 設定幾秒後發送signal(SIGALRM): alarm()
    - alarm(int second);        // second 秒後觸發 SIGALRM
    - 如果參數設為 0 => cancel 所有 alarm
- `小考會考`: `alarm()` `signal()` `signal mask` `sigsuspend` `sigaction()`...
    - signal()置換default action
    - signal mask: sigemptyset()...

工業界中長出現這樣的狀況(熱機後固定一段時間做某建事情)：

    |<-init->|<-dt->|<-dt->|<-dt->|<-dt->|<-dt->|
    v  (t1)  v      v      v      v      v      v
    |--------|------|------|------|------|------|----> time

- 可以透過 setitimer() 在 it_value.tv_sec 秒後，固定每 it_interval.tv_sec 秒做alarm()
- 見 **Module 6.pdf, slide 23 of 26**


# Threads(Ch.07)
- thread 不會共用 stack segement
- 把資料放在 golbal(Data Segment)，在threads間可以共用，但要注意 race condition
