#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <dirent.h>
#include <libgen.h>
#include <string.h>
#include <ctype.h>
#include<unistd.h>
#include<math.h>


void* checkDir(void* arg);
void* checkFile(void* arg);
#define MAXCHAR 1000
pthread_mutex_t locker = PTHREAD_MUTEX_INITIALIZER;
pthread_t id[1000];
int O2 =1;

//Thread stuff
//--------------------------------------------------------------------------------
typedef struct File{
  //***CHANGES***********//

char* fileName;
int totalWords;
struct File* next;
struct File* prev;
struct File* act_head;
struct Tokens* tokenList;
}file;
typedef struct threadPasser{
  char* ptPath;
  struct dirent* ptDent;
  int arrPosition;
  file* actual_head;
}tPasser;

//Linked list that consists of a thread and a pointer to the next node
void printFileStruct(file* fprinter){
  while(fprinter!=NULL){
    printf("File: %s with Words: %d\n",fprinter->fileName,fprinter->totalWords );
  fprinter=fprinter->next;
  }
}
typedef struct node{
  pthread_t idd;
  char* dirName;
  struct node* next;
  int arrPosition;
}node_t;
//Function to create a new thread node
node_t* create_new_node(pthread_t ids, char* directoryName){
  node_t* res = malloc(sizeof(node_t));
  res ->dirName = directoryName;

  res -> idd = ids;
  res->next=NULL;
  return res;
}

file* fHead;
//Function to join and print out all the threads created for files and directories
void printlist(node_t *head){
  node_t* pTemp = head;
  while(pTemp!=NULL){
   // printf("Directory Name: %s \n",pTemp->dirName);
   // printf("thread id = %ld\n",pthread_self());
    pthread_join(pTemp->idd,NULL);
    pTemp=pTemp->next;
  }
  
}

//----------------------------------------------------------------------------------





typedef struct Tokens{
char* tokenName;
float wordFrequency;
struct Tokens* next;
struct Tokens* prev;
}token;



//A Doubly linked list for the tokens... for alphabetical insertion

//Goes through a files tokens list and updates the values with a discrete distribution;
void calculateDistribution(file* arg){
  int totalWords = arg->totalWords;
  
  token* iterToken=arg->tokenList;
  while(iterToken!=NULL){
    iterToken->wordFrequency=iterToken->wordFrequency/totalWords;
    iterToken=iterToken->next;
  }
};
void printTokenList(file* arg){

  
  token* iterToken=arg->tokenList;
  while(iterToken){
    printf("%f ,",iterToken->wordFrequency);
    iterToken=iterToken->next;
  }
  printf("\n");
};


void addToFileStruct(file* addThis, file* head){
  file* pntr = head;
  while(pntr->next!=NULL){
    pntr = pntr->next;
  }
  pntr->next=addThis;
  return;
}


//Traverse all entries in the directory.... Look for other directories or files
//IF a directory we want to create a new directory thread... if it is a File we want to create a new file thread... Otherwise we ignore the entry 
pthread_mutex_t l2;
void* checkDir(void* arg){

  tPasser* directory = (tPasser*)arg;
  
  DIR *openDir = opendir(directory->ptPath);
  struct dirent* dent;
  int arrpos = directory->arrPosition;
  dent = readdir(openDir); 
  dent = readdir(openDir); 
  while((dent = readdir(openDir)) != NULL){
    //We find a directory
    
    if(dent->d_type == 4){
      
       char* newName = (char*)malloc(strlen(directory->ptPath)+strlen(dent->d_name)+3);      
       strcpy(newName,directory->ptPath);
       strcat(newName,"/");
       strcat(newName,dent->d_name);      
     //  printf("Opening directory... : %s\n",newName);
       tPasser* newDirectory = malloc(sizeof(tPasser));
       
       newDirectory->ptPath = newName;        
       newDirectory->ptDent = dent;
       newDirectory->actual_head=directory->actual_head;
      pthread_mutex_lock(&l2);
       pthread_create(&id[O2],NULL,checkDir,newDirectory);
         O2+=1;
      pthread_mutex_unlock(&l2);
     
   
    }else if(dent->d_type==8){
       char* newName = (char*)malloc(strlen(directory->ptPath)+strlen(dent->d_name)+3);  
      
       strcpy(newName,directory->ptPath);
       strcat(newName,"/");
       strcat(newName,dent->d_name);      
       //printf("Opening file... : %s\n",newName);
       tPasser* newDirectory = malloc(sizeof(tPasser));
       newDirectory->ptPath = newName;        
       newDirectory->ptDent = dent;
       newDirectory->actual_head=directory->actual_head;
       //newName="";
       
      pthread_mutex_lock(&l2);
       pthread_create(&id[O2],NULL,checkFile,(void*)newDirectory);
       O2+=1;
      pthread_mutex_unlock(&l2);

    }
  }
  
 
  
  closedir(openDir);
 // free((tPasser*)arg);
  pthread_exit(NULL);
}
/*Tokenize every file. The program has no way of determining whether a file contains text or not, so it must assume every file contains text.*/

file* fHead;
void* checkFile(void* arg){
  tPasser* myFile = (tPasser*)arg;
  file* newFileMember = malloc(sizeof(file));
  
  
  //pthread_mutex_t lock = newFileMember->lock;
  //Lock on new file Entry
newFileMember->fileName=myFile->ptPath;
newFileMember->act_head=myFile->actual_head; 
  //Traverse the file... for every unique word create a new node. - Insert in order
 // printf("FILE: %s \n" ,newFileMember->fileName);
  FILE * fp = (FILE*)malloc(sizeof(FILE));
 
  fp = fopen(myFile->ptPath,"r");
  if(fp==NULL){
    pthread_exit(NULL);
  }
  //Using the buffer check if the word is present in the tokens LL if it is increment the token for that word by 1 ... If it is not create a new node holding that string and insert

  char wordBuffer[MAXCHAR];
  int c;
  int i = 0;
  int wordCount = 0;
  //char* holder;
  token* headToken = malloc(sizeof(token));
  do{
    
    c = fgetc(fp);
   
    if(ispunct(c)){
      continue;
    }
    //If we reach a space wordbuffer will contain a token
    if(isspace(c) || c == EOF){
      if(strcmp(wordBuffer,"\t") == 0){
        
        break;
      }
      wordCount++;
      //printf("Word Buffer: %s word count: %d \n", wordBuffer,wordCount);
    
      if(wordCount == 1){
        headToken->tokenName= malloc(sizeof(char)*strlen(wordBuffer));
        strcpy(headToken->tokenName,wordBuffer);
        headToken->wordFrequency = 1;
        memset(wordBuffer,0,MAXCHAR);
        //continue;
      }
    
      //If wordcount !=1
      else{
        token *ptr = headToken;
        while(ptr!=NULL){
         
          //Same word so we dont need to create a new token
          if(strcmp(ptr->tokenName, wordBuffer)==0){
            ptr->wordFrequency+=1;
            break;
          }else if(strcmp(ptr->tokenName, wordBuffer) > 0){
            //if ptr->prev is NULL we are on headToken
            if(strcmp(ptr->tokenName,headToken->tokenName) == 0){
              token* temp = malloc(sizeof(token));
              temp->tokenName = (char*)malloc(sizeof(char)*strlen(wordBuffer));
              strcpy(temp->tokenName,wordBuffer);
              //printf("REPLACING HEAD %s\n", temp->tokenName);
              temp -> wordFrequency = 1;
              ptr->prev = temp;
              temp->next = ptr;
              headToken = temp;
              memset(wordBuffer, 0, MAXCHAR);
              break;
            }else{
              //printf("REPLACING RANDO %s\n",ptr->tokenName);
              token* temp = ptr->prev;
              token* newNode = malloc(sizeof(token));
              newNode->tokenName = (char*)malloc(sizeof(char)*strlen(wordBuffer));
              strcpy(newNode->tokenName,wordBuffer);
              newNode->wordFrequency = 1;
              temp->next = newNode;
              newNode->prev = temp;
              newNode->next = ptr;
              ptr->prev = newNode;
              memset(wordBuffer,0,MAXCHAR);
              break;
            }
          }else if(ptr->next == NULL){
            token* newNode = malloc(sizeof(token));
            newNode->tokenName= (char*)malloc(sizeof(char)*strlen(wordBuffer));
            strcpy(newNode->tokenName,wordBuffer);
            newNode -> wordFrequency = 1;
            ptr->next = newNode;
            newNode->prev = ptr;
            memset(wordBuffer,0,MAXCHAR);
            break;
          }
          ptr = ptr->next;
        }

      }
      i = 0;
      memset(wordBuffer,0,MAXCHAR);
    }else{
      wordBuffer[i] = c;
      i++;
    }
    if(feof(fp) || c == EOF){
      break;
    }
  }while(1); 
 
if(strcmp(headToken->tokenName,"") == 0 || strcmp(headToken->tokenName,(char*)"\n") == 0){
   
    float sub = headToken->wordFrequency;
    wordCount = wordCount - sub;
    headToken = headToken->next;
    headToken->prev = NULL;
  } 

  
  newFileMember ->totalWords=wordCount;

  newFileMember -> tokenList = headToken; 

  pthread_mutex_lock(&locker);
  calculateDistribution(newFileMember);
  addToFileStruct(newFileMember, newFileMember->act_head);
  free(myFile);
  pthread_mutex_unlock(&locker);
  file* fprinter = newFileMember->act_head;
 
  token* printer = headToken;

  //Now we need to add the file to the filestruct
  
   //printFileStruct(fprinter);
//printf("HER1E\n");


 fclose(fp);
  pthread_exit(NULL);

}

void JSD(file* head1, file* head2){
  token* curr1 = head1->tokenList;
  //printf("%s temperpedic: \n" , head1->tokenList->tokenName);
  token* curr2 = head2->tokenList;
  token* meanHead = malloc(sizeof(token));
  token* meanPtr = meanHead;
  int headUsed = 0;
  //printf("um %s %s\n",curr1->tokenName,curr2->tokenName);
  //int i = 0;
  while(curr1 != NULL && curr2 != NULL){
    meanPtr->next = malloc(sizeof(token));
    if(strcmp(curr1->tokenName,curr2->tokenName) == 0){
      meanPtr->tokenName = malloc(strlen(curr1->tokenName));
      strcpy(meanPtr->tokenName,curr1->tokenName);
      printf("Here!\n");
      printf("Word 1: %f\n", curr1->wordFrequency);
      printf("Word 2: %f\n", curr2->wordFrequency);
      meanPtr -> wordFrequency = (((curr1->wordFrequency)+ (curr2->wordFrequency))/2);
      meanPtr = meanPtr->next;
      curr1 = curr1->next;
      curr2 = curr2->next;
    }else if(strcmp(curr1->tokenName,curr2->tokenName) > 0){
     meanPtr->tokenName = malloc(strlen(curr2->tokenName));
      strcpy(meanPtr->tokenName,curr2->tokenName);
      meanPtr->wordFrequency = ((curr2->wordFrequency)/2);
      meanPtr = meanPtr->next;
      curr2 = curr2->next;
    }else if(strcmp(curr1->tokenName,curr2->tokenName) < 0){
      meanPtr->tokenName = malloc(strlen(curr1->tokenName));
      strcpy(meanPtr->tokenName,curr1->tokenName);
      meanPtr->wordFrequency = ((curr1->wordFrequency)/2);
      meanPtr = meanPtr->next;
      curr1 = curr1->next;
    }
  }
  if(curr1 == NULL && curr2 != NULL){
    while(curr2 != NULL){
      meanPtr->tokenName = curr2->tokenName;
      meanPtr->wordFrequency = ((curr2->wordFrequency)/2);
      meanPtr->next = malloc(sizeof(token));
      meanPtr = meanPtr->next;
      curr2 = curr2->next;
    }
  }
  else if(curr2 == NULL && curr1 != NULL){
     while(curr1 != NULL){
      meanPtr->tokenName = curr1->tokenName;
      //******CHANGE*****
      meanPtr->wordFrequency = ((curr1->wordFrequency)/2);
      meanPtr->next = malloc(sizeof(token));
      meanPtr = meanPtr->next;
      curr1 = curr1->next;
    }
  }
  //printf("curr1 %s",curr1->tokenName);
  meanPtr = meanHead;

  //Start finding KLD
  curr1 = head1->tokenList;
  curr2 = head2->tokenList;
  float logJSD =0;
  float div = 0;
  while(meanPtr->next != NULL){
    printf("N1: %s , N2: %s N3: %s\n",curr1 ->tokenName,curr2->tokenName,meanPtr->tokenName);
    printf("Word freq: %f \n", meanPtr->wordFrequency);
    if(curr1 !=NULL && strcmp(curr1->tokenName,meanPtr->tokenName) == 0){
      div = (curr1->wordFrequency)/(meanPtr->wordFrequency); 
      
     // printf("N1: %s , N2: %s N3: %s\n",curr1 ->tokenName,curr2->tokenName,meanPtr->tokenName);
      logJSD = log10f(div);
      if(logJSD== -1.0 / 0.0){
        logJSD= 0;
      }
      printf("LOG JSD: %f \n", logJSD);
      printf("DIV: %f \n", div);
      curr1->wordFrequency = curr1->wordFrequency * logJSD;
      if(curr1->next != NULL){
        curr1 = curr1->next;
      }
    }
     if(curr2 != NULL && strcmp(curr2->tokenName,meanPtr->tokenName) == 0){
      div = (curr2->wordFrequency)/(meanPtr->wordFrequency); 
      logJSD = log10f(div);
       printf("LOG JSD 2: %f \n", logJSD);
      printf("DIV 2: %f \n", div);
      curr2->wordFrequency = curr2->wordFrequency * logJSD;
      if(curr2->next != NULL){
        curr2 = curr2->next;
      }
    }
   
    meanPtr = meanPtr->next;
  }
  float curr1KLD = 0;
  float curr2KLD = 0;
  curr1 = head1->tokenList;
  curr2 = head2->tokenList;
  printf("\n");
  while(curr1 != NULL){
    if(!isnan(curr1->wordFrequency)){
       curr1KLD = curr1KLD + (curr1->wordFrequency);
    }
    printf("CURR1: %f \n", curr1KLD);
  
   // printf("WF1 ,%f ",curr1->wordFrequency);
    curr1 = curr1->next;
  }
  while(curr2 != NULL){
    if(!isnan(curr2->wordFrequency)){
    curr2KLD = curr2KLD + (curr2->wordFrequency);
    }
        printf("CURR2: %f \n", curr2KLD);
    curr2 = curr2->next;
  }
  //find the mean of the two KLDs for the final answer
  float JSD = (curr1KLD + curr2KLD)/2;
 // printf("%f , %f, JSD: %f\n",curr1KLD,curr2KLD ,JSD);
  if(JSD >= 0 && JSD <= 0.1){
    printf("\033[1;31m");
    printf("%lf",JSD);
    printf("\033[0m");
    printf(" %s and %s\n",head1->fileName, head2->fileName);
  }else if(JSD > 0.1 && JSD <= 0.15){
    printf("\033[1;33m");
    printf("%lf",JSD);
    printf("\033[0m");
    printf(" %s and %s\n",head1->fileName, head2->fileName);
  }else if(JSD > 0.15 && JSD <= 0.2){
    printf("\033[1;32m");
    printf("%lf",JSD);
    printf("\033[0m");
    printf(" %s and %s\n",head1->fileName, head2->fileName);
  }else if(JSD > 0.2 && JSD <= 0.25){
    printf("\033[1;36m");
    printf("%lf",JSD);
    printf("\033[0m");
    printf(" %s and %s\n",head1->fileName, head2->fileName);
  }else if(JSD > 0.25 && JSD <= 0.3){
    printf("\033[1;34m");
    printf("%lf",JSD);
    printf("\033[0m");
    printf(" %s and %s\n",head1->fileName, head2->fileName);
  }else if(JSD > 0.3){
    printf("%lf %s and %s\n",JSD, head1->fileName, head2->fileName);
  }
}



int main(int argc, char** argv){
fHead =(file*)malloc(sizeof(file));
fHead->fileName="t";


DIR *pDir;
struct dirent *dentt;
pDir = opendir(argv[1]);
//Checks if the command line passes in a directory, otherwise exits the process.
//
if(pDir == NULL){
  printf("You have to pass in a directory in the command linee\n");
  exit(1);
}
dentt = readdir(pDir);

if(dentt->d_type != 4){
  printf("You have to pass in a directory in the command line\n");
  exit(1);
}
//Starting struct
tPasser* starter;
char* start = malloc(sizeof(strlen(argv[1])+3));
//start[0]='.';
//start[1]='/';
//printf("name: %s\n",dent->d_name);
strcpy(start, argv[1]);
starter ->ptDent = dentt;
starter ->ptPath = start;
starter->arrPosition=0;
starter->actual_head=fHead;
//printf("start: %s\n",starter->ptPath);
//printf("%s\n" ,starter->ptPath);
//Passes in the root directory struct into the dir function
node_t* pHead;
node_t* tHead;

pthread_create(&id[0],NULL,checkDir,(void*)starter);
sleep(4);
printf("AFTER threads\n");
int j = 0;
for(;j<1000;j++){
  pthread_join(id[j],NULL);
  
}
printlist(pHead);
printf("check sir\n");
file* headPtr = fHead->next;
while(headPtr != NULL&&headPtr->fileName!=NULL){
  
  printf("DOING THIS FILE: %s\n", headPtr->fileName);
  
 printTokenList(headPtr);
  headPtr = headPtr->next;

}

file *filePtr = fHead->next;
file *filePtr2 = filePtr->next;
while(filePtr != NULL&&filePtr2!=NULL){
  while(filePtr2 != NULL){
  //  printf("JSD IN IT %s, %s\n",filePtr->fileName,filePtr2->fileName);
    JSD(filePtr,filePtr2);

    filePtr2 = filePtr2->next;
  //  printf("AFTER IT \n");
  }
  filePtr = filePtr->next;
  filePtr2 = filePtr->next;
}

printf("HER2E\n");
return 0;
}
