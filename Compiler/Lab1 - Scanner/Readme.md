1. **Lex 版本:** 
    - flex 2.6.4
2. **作業平台:**
    - OS    : Ubuntu 24.04.4 LTS x86_64 
    - Kernel: 6.17.0-19-generic 
3. **執行方式:**
    ```sh
        # Step1. Compile the lex.l
        make && clear

        # Step2. Run the scanner
        ./scanner                   # stdin 手動輸入
        ./scanner < [test flie]     # 透過測資檔案自動輸入
        # 透過測資檔案自動輸入 並將測資的內容放在螢幕輸出的最上方
        clear && make && cat [test file] | tee /dev/tty | ./scanner     
    ```                                                                   
4. **你/妳如何處理這份規格書上的問題:**

    我先在 Definition 區域，透過 Regular Expression 先把規格書上面所有 valid 的 pattern 寫出來
    碰到比較複雜得 Real，就先把它分解成比較小的 Pattern 定義起來，最後在合起來得到複雜的 Pattern
    寫完了 Valid 的 Pattern 後，我就接著寫這些 Valid pattern 的 rules，基本上就是維護當前第幾行
    第幾個字，然後輸出 pattern 位置與對應的名字，再透測資驗證 Valid Pattern 的部份有沒有問題。

    寫完了 Valid definitions 和 rules 後，我才開始撰寫 invalid pattern 的部份，我主要是透過規格書
    上面撰寫出的例子與否定規格書上面規定的方法，寫出 invalid pattern，然後寫出對應的 rules，
    在透過測資驗證有沒有問題。

    最後我在對整個 .l 細修一下，新增對於一些 invalid pattern 輸出 invalid 的原因，輸出該 String 
    實際上對應的字串以及稍微調整一下其他輸出後就差不多了。

    所以我主要是透過上面這樣，把規格分成幾個部份，分別完成驗證，慢慢的完成這整份作業。

5. **你/妳寫這個作業所遇到的問題:**

    這份作業對我來說，遇到的最大問題是如何造出各個 Pattern 的 Regular Expression，因為 flex 的
    pattern match 是最長匹配優先，再來才是定義順序的優先級 break the tie，不是單純的定義順序決定
    所以有時候 Expression 寫的不好，會讓 scanner 不小心抓太長，然後把應該是兩個 pattern 的合併到
    一個更長的 pattern，出現一些不是我想要的效果，因此撰寫這些 Expression 花了我最長的時間，要想
    辦法不要讓各個 pattern 混在一起，要盡量寫一個好的 pattern。
    
    Comment 是我遇到第二大的問題，因為我不知道要怎麼寫一個好的 expression 能同時包住跨行的註解，
    又能抓出 invalid comment，又能忽略 comment 中的文字對於其他pattern 的解析，
    (在comment 中 123 應該要解釋為 comment，不該被解釋為 integer)
    (因為最長匹配，在invalid pattern 中寫 \*\) 會讓invalid comment 蓋住 valid comment)
    最後發現可以透過遇到 comment 開頭 (* 後，進入 COMMENT state，讓這時候的 match pattern 只適用
    於其 state 專屬的 pattern，解決 pattern 混雜得問題。然後想到可以在輸出時，不用一次全部完成，
    可以分批慢慢輸出完成一行，解決 Comment 輸出格式問題。

6. **所有測試檔執行出來的結果，存成圖片或文字檔**
