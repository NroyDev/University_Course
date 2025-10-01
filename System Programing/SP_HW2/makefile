all:
	clear
	gcc -o ./tee ./tee.c
	gcc -o ./cp ./cp.c

clean:
	clear
	rm tee cp
	
test:
	@make all
	
	@echo "\n---------------- tee test START ----------------\n"
	@echo "\n##################################\n------- TEST A -------\n##################################\n"
	cat ./tee.c | tee ./testA.txt
	@echo "\n\n"
	cat ./tee.c | tee -a ./testA.txt
	@echo "\n\n"
	cat ./tee.c | tee -a ./testA.txt
	
	@echo "\n##################################\n------- TEST B -------\n##################################\n"
	cat ./tee.c | tee -a ./testB.txt
	@echo "\n\n"
	cat ./tee.c | tee -a ./testB.txt
	
	@echo "\n##################################\n------- TEST C -------\n##################################\n"
	cat ./tee.c | tee ./testC.txt
	@echo "\n\n"
	cat ./tee.c | tee ./testC.txt
	@echo "\n---------------- tee test END ----------------\n"
	
	@echo "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
	@echo "\n---------------- cp test START ----------------\n"
	@echo "\n##################################\n------- SPARSE FILE -------\n##################################\n"
	@dd if=./makefile of=sparse.bin bs=1K seek=1
	@dd if=./makefile of=sparse.bin bs=1K seek=2 conv=notrunc
	@dd if=./makefile of=sparse.bin bs=1K seek=3 conv=notrunc
	@dd if=./makefile of=sparse.bin bs=1K seek=4 conv=notrunc
	@dd if=./makefile of=sparse.bin bs=1K seek=5 conv=notrunc
	@dd if=./makefile of=sparse.bin bs=1K seek=8 conv=notrunc
	@dd if=./makefile of=sparse.bin bs=1K seek=9 conv=notrunc
	@dd if=./makefile of=sparse.bin bs=1K seek=12 conv=notrunc
	@dd if=./makefile of=sparse.bin bs=1K seek=32 conv=notrunc
	@dd if=./makefile of=sparse.bin bs=1K seek=128 conv=notrunc
	
	@echo "\n##################################\n------- EXECUTE -------\n##################################\n"
	./cp ./sparse.bin ./test.bin
	
	
	@echo "\n##################################\n------- COMPARE RESULT -------\n##################################\n"
	du ./test.bin ./sparse.bin
	@echo ""
	ls -ls ./test.bin ./sparse.bin
	@echo ""
	cmp ./test.bin ./sparse.bin
	@echo "\n---------------- cp test END ----------------\n"
	
testClean:
	clear
	make clean
	rm ./testA.txt ./testB.txt ./testC.txt
	rm ./test.bin ./sparse.bin
	
