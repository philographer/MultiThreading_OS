#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


int arr[10000]; //전역배열
int total = 0; //전체 크기
int count = 0;; //카운트

struct idx{
    int start_idx; //시작 인덱스
    int end_idx; //끝 인덱스
    int last; //4번째 쓰레드인지 검사하기 위해(merge thread인지)
};

void thread_end(){
    count++; //count 증가
}

// 쓰레드 함수
void sort(void *data)
{
    struct idx *index_sort = (struct idx*)data; // 데이터를 가져와서 포인터로 구조체 설정
    int i1, i2; //2중 for문을 위해
    int start = index_sort->start_idx; //시작 인덱스
    int end = index_sort->end_idx; //시작 인덱스
    int last = index_sort->last; //merge쓰레드인지 검사
    
    //마지막 합치는 쓰레드라면(merge thread)
    if(last){ //스핀락 => count 즉 3개의 앞의 처리가 다 될때까지 loop
        while(count != 3);
    }
    
    for(i1=start; i1 <= end; i1++){ //bubble sort (split된 데이터들은 시작값, 끝값에 의해 정렬), (merge된 배열은 처음부터 끝까지 정렬)
        for(i2=i1; i2 <= end; i2++){
            if(arr[i1] > arr[i2]){
                int temp = arr[i1];
                arr[i1] = arr[i2];
                arr[i2] = temp;
            }
        }
    }
    
    if(!last){ // merge thread가 아니라면 카운트 증가
        count++;
    }
    
    pthread_exit(NULL); //모든 작업을 처리후 쓰레드 종료, 리턴값 null
}

int main(int argc, char* argv[])
{
    FILE *inputFile;
    FILE *outputFile;
    
    
    pthread_t thread[3]; //쓰레드 3개 생성
    pthread_t mergeThread; //마지막 합치는 merge thread
    struct idx splited_arr[3]; //시작 끝값을 나타내는 구조체 설정
    struct idx merged_arr; //mergethread의 array
    int size[3] = {0,}; //쓰레드1, 쓰레드2, 쓰레드3의 size 배열
    
    char input[10001]; // 입력값 5000자 까지
    int i = 0; // 루프를 위해
    char* temp; //string을 자르기 위해 임시 포인터
    int quotient = 0; //몫
    int remainder = 0; //나머지
    
    //입출력할 파일을 생성
    inputFile = fopen(argv[1], "r");
    outputFile = fopen("output.txt", "w");
    fgets(input, 10000, inputFile);
    temp = strtok(input, " "); //string tokenize
    for(;;){
        arr[total] = atoi(temp); //스페이스바를 기준으로 배열에 집어넣음
        total++;
        temp = strtok(NULL, " "); //string tokenize
        if(temp == NULL) break;
    }
    
    //배열을 자연스럽게 3등분하기 위해 3으로 나눈 몫을 할당하고 나머지가 있으면 1씩 더 크기를 할당해줌
    quotient = total / 3;
    remainder = total % 3;
    
    for(i = 0; i < 3; i++){
        size[i] = quotient;
    }
    
    for(i = 1; i <= remainder; i++){ //나머지가 있으면 1씩 더 크기를 할당해줌
        size[i] += 1;
    }
    
    
    //3등분된 배열들 (시작값, 끝값, merge쓰레드인지 표기)
    splited_arr[0].start_idx = 0;
    splited_arr[0].end_idx = size[0]-1;
    splited_arr[0].last = 0;
    
    splited_arr[1].start_idx = size[0];
    splited_arr[1].end_idx = size[0] + size[1] - 1;
    splited_arr[1].last = 0;
    
    splited_arr[2].start_idx = size[0] + size[1];
    splited_arr[2].end_idx = size[0] + size[1] + size[2] - 1;
    splited_arr[2].last = 0;
    
    
    //3등분된 배열들이 합쳐질 배열
    merged_arr.start_idx = 0; //시작값 처음
    merged_arr.end_idx = size[0] + size[1] + size[2] - 1; //끝값 마지막
    merged_arr.last = 1; //merge thread임을 표기
    
    if (pthread_create(&thread[0], NULL, (void*)sort, &splited_arr[0]) < 0){ //쓰레드1. 구조체1을 sort의 argument로 보냄
        perror("쓰레드1 만들기 에러");
        exit(0);
    }
    if (pthread_create(&thread[1], NULL, (void*)sort, &splited_arr[1])){ //쓰레드2. 구조체2을 sort의 argument로 보냄
        perror("쓰레드2 만들기 에러");
        exit(0);
    }
    if (pthread_create(&thread[2], NULL, (void*)sort, &splited_arr[2])){ //쓰레드3. 구조체3을 sort의 argument로 보냄
        perror("쓰레드3 만들기 에러");
        exit(0);
    }
    if (pthread_create(&mergeThread, NULL, (void*)sort, &merged_arr)){ //merge 쓰레드, 1,2,3 쓰레드를 합쳐서 정렬
        perror("merge 쓰레드 만들기 에러");
        exit(0);
    }
    
    //쓰레드가 종료되기를 기다림 반환값 NULL
    pthread_join(thread[0], NULL);
    pthread_join(thread[1], NULL);
    pthread_join(thread[2], NULL);
    pthread_join(mergeThread, NULL);
    
    //전체 정렬된 후 출력
    for(i=0; i < total; i++){
        fprintf(outputFile, "%d ", arr[i]);
    }
	fprintf(outputFile, "%s", "\n");
	
	fclose(inputFile);
	fclose(outputFile);

	printf("\n");
    
    return 1;
}
