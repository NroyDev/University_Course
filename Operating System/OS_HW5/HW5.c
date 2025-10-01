#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<stdbool.h>

typedef struct Node{    // struct of nodes on the tree
	char* key;
	void* value;
	struct Node* left;
	struct Node* right;
	pthread_mutex_t lock;
} Node;
typedef struct Tree{    // struct of tree
	struct Node* root;  // root of tree
	int sizeofVal;      // how many byte is the value of nodes, only can modify at setup after init
	pthread_mutex_t lock;
} Tree;

// inti Tree
void initialize(Tree* tree){
	tree->root = NULL;
	tree->sizeofVal = 4;	// default 4 byte (int)
	pthread_mutex_init(&(tree->lock),NULL);
}
// setup Tree val size (should be called immediate after initialize Tree)
// only can be called once!!!
void setupTree(Tree* tree,int sizeofVal){
	tree->sizeofVal = sizeofVal;
}

// node constructor
Node* createNode(const Tree* tree,char* key,void* value){
	Node* newNode = (Node*)malloc(sizeof(Node));		// alloc a memory space for the node structure
	// key
	int keysize = strlen(key)+1;
	newNode->key = (char*)malloc(keysize*sizeof(char));
	memcpy(newNode->key,key,keysize);			// copy the key
	// value
	newNode->value = (void*)malloc(tree->sizeofVal);	// sizeofVal won't modify after setup, so no need lock
	memcpy(newNode->value,value,tree->sizeofVal);
	// children
	newNode->left = NULL;
	newNode->right = NULL;
	// lock
	pthread_mutex_init(&(newNode->lock),NULL);
	
	return newNode;
}
// node deconstructor
void freeNode(Node* node){
	node->left = NULL;
	node->right = NULL;
	free(node->key);	// release the space of key;
	free(node->value);	// release the space of val
	pthread_mutex_destroy(&(node->lock));	// desory the lock
	free(node);		// release the space of struct node
}

// add
void addNode(Tree* tree,Node** current,char* key,void* value){
	pthread_mutex_lock(&((*current)->lock));// lock: prevnet the data modify by another threads
	if(strcmp((*current)->key,key)>0){	// should be add in left subtree
		if((*current)->left==NULL){		// left is empty=>create node and put it there 
			Node* newNode = createNode(tree,key,value);
			(*current)->left = newNode;
		}else{	// recurisive to find the node to modify;
			addNode(tree,&((*current)->left),key,value);
		}
	}else if(strcmp((*current)->key,key)<0){// should be add in right subtree
		if((*current)->right==NULL){		// right is empty=>create node and put it there 
			Node* newNode = createNode(tree,key,value);
			(*current)->right = newNode;
		}else{	// recurisive to find the node to modify;
			addNode(tree,&((*current)->right),key,value);
		}
	}else{		// key collision => we do nothing? (The homework seems no description how to deal with it)
		printf("Same key (%s), nothing happened\n",key);
	}
	pthread_mutex_unlock(&((*current)->lock));	// release the lock
}
void add(Tree* tree,char* key,void* value){
	pthread_mutex_lock(&(tree->lock));	// lock: prevnet the data modify by another threads
	if(tree->root==NULL){	// the tree is empty
		Node* newNode = createNode(tree,key,value);
		tree->root = newNode;	// the newnode is root now
	}else{			// the tree is not empty
		addNode(tree,&(tree->root),key,value);
	}
	pthread_mutex_unlock(&(tree->lock));	// release the lock
}

// delete
void popNode(Node** current){	// delete(pop) the node and find the suitable node the replace it
	if((*current)->left==NULL && (*current)->right==NULL){	// both empty
		freeNode((*current));
		(*current) = NULL;
	}else if((*current)->right==NULL){ // left is not empty=> replace by left
		Node* temp = (*current);
		(*current) = (*current)->left;
		freeNode(temp);
	}else if((*current)->left==NULL){ // right is not empty=> replace by left
		Node* temp = (*current);
		(*current) = (*current)->right;
		freeNode(temp);
	}else{	// otherwise => replace by inorder successor
		Node* inorder = (*current)->right;
		Node* prev = NULL;
		while(inorder->left!=NULL){
			prev = inorder;
			inorder = inorder->left;
		}
		if(prev!=NULL){
			prev->left = inorder->right;
			inorder->left  = (*current)->left;
			inorder->right = (*current)->right;
		}else{	// now inorder is (*current)->right
			inorder->left = (*current)->left;
		}
		Node* temp = (*current);
		(*current) = inorder;
		freeNode(temp);
	}
}

void deleteNode(Tree* tree,Node** current,char* key){	
	pthread_mutex_lock(&((*current)->lock)); 	// lock: prevnet the data modify by another threads
	if(strcmp((*current)->key,key)>0){	// the node we want to delete is at left subtree
		if((*current)->left==NULL){	// not found case
			printf("NOT FOUND\n");
		}else{
			deleteNode(tree,&((*current)->left),key);
		}
	}else if(strcmp((*current)->key,key)<0){// the node we want to delete is at right subtree
		if((*current)->right==NULL){	// not found case
			printf("NOT FOUND\n");
		}else{
			deleteNode(tree,&((*current)->right),key);
		}
	}else{	// delete this node
		popNode(current);
	}
	if((*current) != NULL){
		pthread_mutex_unlock(&((*current)->lock));	// release the lock
	}
}
void delete(Tree* tree,char* key){
	if(tree==NULL){
		printf("NULL tree PTR\n");
		return;
	}
	pthread_mutex_lock(&(tree->lock));		// lock: prevnet the data modify by another threads
	if(tree->root==NULL){	// empty tree
		printf("Empty Tree, failed\n");
	}else{
		deleteNode(tree,&(tree->root),key);
	}
	pthread_mutex_unlock(&(tree->lock));		// release the lock
}
// lookup
bool lookupNode(Tree* tree,Node* current,char* key,void** value){	
	pthread_mutex_lock(&(current->lock));	// lock: prevnet the data modify by another threads
	if(strcmp(current->key,key)>0){		// at left subtree
		if(current->left==NULL){	// not exist case
			value = NULL;
			pthread_mutex_unlock(&(current->lock));	// release the lock
			return false;
		}else{
			bool ret = lookupNode(tree,current->left,key,value);
			pthread_mutex_unlock(&(current->lock));	// release the lock
			return ret;
		}
	}else if(strcmp(current->key,key)<0){	// at right subtree
		if(current->right==NULL){	// not exist case
			value = NULL;
			pthread_mutex_unlock(&(current->lock));	// release the lock
			return false;
		}else{
			bool ret = lookupNode(tree,current->right,key,value);
			pthread_mutex_unlock(&(current->lock));	// release the lock
			return ret;
		}
	}else{	// is this node!!!!!
		*value = (void*)malloc(tree->sizeofVal);
		memcpy(*value,current->value,tree->sizeofVal);
		pthread_mutex_unlock(&(current->lock));	// release the lock
		return true;
	}
}
bool lookup(Tree* tree,char* key,void** value){
	bool ret = false;
	pthread_mutex_lock(&(tree->lock));	// lock: prevnet the data modify by another threads
	if(tree->root==NULL){
		printf("ERROR, EMPTY TREE\n");
	}else{
		ret = lookupNode(tree,tree->root,key,value);
	}
	pthread_mutex_unlock(&(tree->lock));	// release the lock
	return ret;
}

// not requrie func below
// ----------------------------------------------------------------------------------------

// print
int printNode(FILE*fp,Node* current){	// inorder visit and print the key of node
	if(current==NULL){
		return 0;
	}
	pthread_mutex_lock(&(current->lock));// lock: prevnet the data modify by another threads
	int cnt = 1;	// count how many nodes in this subtree(include subtree root)
	cnt += printNode(fp,current->left);
	fprintf(fp,"%s\n",current->key);
	cnt += printNode(fp,current->right);
	pthread_mutex_unlock(&(current->lock));// release the lock
	return cnt;
}
int print(FILE*fp,Tree* tree){
	if(tree==NULL){
		printf("ERROR, NULL TREE PTR\n");
		return 0;
	}else{
		int cnt = 0;	// count how many nodes in this tree(include subtree root)
		pthread_mutex_lock(&(tree->lock));	// lock: prevnet the data modify by another threads
		cnt += printNode(fp,tree->root);
		pthread_mutex_unlock(&(tree->lock));	// release the lock
		return cnt;
	}
}

//

Tree* t;

// ---------------------------------------------------
void* work1(void* data){
	FILE* fp = fopen("input1.txt","r");
	FILE* fp2 = fopen("output1.txt","w");
	if(fp==NULL||fp2==NULL){
		printf("OPEN FAILED");
		pthread_exit(NULL);
	}
	char op[10];
	char input[30];
	int val;
	while(fscanf(fp,"%s",op) && strcmp(op,"e")!=0){
		//printf("Thread1 ");
		if(strcmp(op,"a")==0){
			fscanf(fp,"%s %d",input,&val);
			//printf("THREAD1 %s\n",input);
			add(t,input,&val);
		}else if(strcmp(op,"d")==0){	
			fscanf(fp,"%s",input);
			delete(t,input);
		}else if(strcmp(op,"p")==0){
			int cnt = print(fp2,t);
			fprintf(stdout,"%d items in total\n",cnt);
		}else if(strcmp(op,"l")==0){
			int *value = NULL;
			fscanf(fp,"%s",input);
			lookup(t,input,(void**)&value);
			if(value==NULL){
				fprintf(fp2,"NOT FOUND %s\n",input);
			}else{
				fprintf(fp2,"FIND VAL OF %s: %d\n",input,*value);
			}
		}
	}
	fclose(fp2);
	fclose(fp);
	
	pthread_exit(NULL);
}
void* work2(void* data){
	FILE* fp = fopen("input2.txt","r");
	FILE* fp2 = fopen("output2.txt","w");
	if(fp==NULL||fp2==NULL){
		printf("OPEN FAILED");
		pthread_exit(NULL);
	}
	char op[10];
	char input[30];
	int val;
	while(fscanf(fp,"%s",op) && strcmp(op,"e")!=0){
		//printf("Thread2 ");
		if(strcmp(op,"a")==0){
			fscanf(fp,"%s %d",input,&val);
			add(t,input,&val);
		}else if(strcmp(op,"d")==0){	
			fscanf(fp,"%s",input);
			//printf("THREAD2 %s\n",input);
			delete(t,input);
		}else if(strcmp(op,"p")==0){
			int cnt = print(fp2,t);
			fprintf(stdout,"%d items in total\n",cnt);
		}else if(strcmp(op,"l")==0){
			int *value = NULL;
			fscanf(fp,"%s",input);
			lookup(t,input,(void**)&value);
			if(value==NULL){
				fprintf(fp2,"NOT FOUND %s\n",input);
			}else{
				fprintf(fp2,"FIND VAL OF %s: %d\n",input,*value);
			}
		}
	}
	fclose(fp2);
	fclose(fp);
	
	pthread_exit(NULL);
}

void work_sequentail(){
	FILE* fp = fopen("input_sequential.txt","r");
	FILE* fp2 = fopen("output_sequential.txt","w");
	if(fp==NULL||fp2==NULL){
		printf("OPEN FAILED");
		return;
	}
	char op[10];
	char input[30];
	int val;
	while(fscanf(fp,"%s",op) && strcmp(op,"e")!=0){
		if(strcmp(op,"a")==0){
			fscanf(fp,"%s %d",input,&val);
			add(t,input,&val);
		}else if(strcmp(op,"d")==0){	
			fscanf(fp,"%s",input);
			delete(t,input);
		}else if(strcmp(op,"p")==0){
			int cnt = print(fp2,t);
			fprintf(stdout,"%d items in total\n",cnt);
		}else if(strcmp(op,"l")==0){
			int *value = NULL;
			fscanf(fp,"%s",input);
			lookup(t,input,(void**)&value);
			if(value==NULL){
				fprintf(fp2,"NOT FOUND %s\n",input);
			}else{
				fprintf(fp2,"FIND VAL OF %s: %d\n",input,*value);
			}
		}
	}
	fclose(fp2);
	fclose(fp);
	
	return;
}

// ---------------------------------------------------
int main(){
	t = (Tree*)malloc(sizeof(Tree));
	initialize(t);
	setupTree(t,sizeof(int));
	
	// we run 2 thread that call add delete lookup
	// thread1 use input1.txt as input, output1.txt as output
	// thread1 add 1095 items, delete 185 items
	// thread2 use input2.txt as input, output2.txt as output
	// thread2 add 1089 items, delete 176 items
	pthread_t t1,t2;
  	pthread_create(&t1, NULL, work1, "work1");
  	pthread_create(&t2, NULL, work2, "work2");
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	
	// then save the inorder print into result_concurrently.txt
	// and count how many items in the tree in the end
	FILE* fp = fopen("result_concurrently.txt","w");
	if(fp==NULL){
		printf("OPEN FAILED\n");
		return 0;
	}
	int cnt = print(fp,t);
	fprintf(fp,"%d items in total\n",cnt);
	fclose(fp);
	// there are 1823 items in total (1095+1089-185-176)
	
	// now let's run it without thread
	//printf("SEQUENTAIL ROUND\n");
	t = (Tree*)malloc(sizeof(Tree));
	initialize(t);
	setupTree(t,sizeof(int));
	// use input_sequential.txt as input, output_sequential.txt as output
	work_sequentail();
	// then save the inorder print into result_sequential.txt
	// and count how many items in the tree in the end
	FILE* fp2 = fopen("result_sequential.txt","w");
	if(fp2==NULL){
		printf("OPEN FAILED\n");
		return 0;
	}
	cnt = print(fp2,t);
	fprintf(fp2,"%d items in total\n",cnt);
	fclose(fp2);
	// there are 1823 items in total
	
	// result_sequential.txt result_concurrently.txt has same content.
	printf("\tDone\n");
	printf("\tPlease cheack result_sequential.txt and result_concurrently.txt for the tree data result\n");
	printf("\tPlease cheack output1.txt and output2.txt for the search result by thread1/thread2\n");
	
	return 0;
}


/*
int main(){
	Tree* t = (Tree*)malloc(sizeof(Tree));
	initialize(t);
	setupTree(t,sizeof(int));
	
	char op[10];
	char input[30];
	int val;
	// command:
	// add node
	// - a [key] [value]
	// delete node
	// - d [key]
	// lookup node
	// - l [key]
	// print the tree keys(inorder)
	// - p
	// exit
	// - e
	while(scanf("%s",op) && strcmp(op,"e")!=0){
		if(strcmp(op,"a")==0){
			scanf("%s %d",input,&val);
			add(t,input,&val);
		}else if(strcmp(op,"d")==0){	
			scanf("%s",input);
			delete(t,input);
		}else if(strcmp(op,"p")==0){
			int cnt = print(t);
			printf("%d items in total\n",cnt);
		}else if(strcmp(op,"l")==0){
			int *value = NULL;
			scanf("%s",input);
			lookup(t,input,(void**)&value);
			if(value==NULL){
				printf("NOT FOUND\n");
			}else{
				printf("FIND VAL: %d\n",*value);
			}
		}
	}
	return 0;
}
*/

