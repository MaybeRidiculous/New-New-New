#include <stdio.h>
#include <pthread.h>
#include<semaphore.h>
#include <unistd.h>

#define NUMBER_OF_PRODUCER 3
#define NUMBER_OF_CONSUMER 4
#define SIEZE_OF_BUFFER 4

pthread_mutex_t buffer_mutex = { { 0, 0, 0, 0, 0, __PTHREAD_SPINS, { (struct __pthread_internal_list *)0, 0 } } };
sem_t full;
sem_t empty;

//模拟拿到的data,
char data[] = "hello world, data1 data2 data3 data4 data5 data6 data7, Whatever is worth doing is worth doing well, Happiness is a way station between too much and too little. In love folly is always sweet.";
int buffer[SIEZE_OF_BUFFER] = {0};
int proCount = 0;//生产的次数

int isproducing[NUMBER_OF_PRODUCER] = {0};//生产者是否在生产
int isconsuming[NUMBER_OF_CONSUMER] = {0};//消费者是否在消费

//定义临界资源--->缓冲池：4个缓冲区,一个buffer为4B
// void **buffer_poll;//指向一地址，该地址指向一个地址

void *producer(void *ptr){
    while (1)
    {
        printf("producer%d thread id = %d, producer入口，当前empty = %d, full = %d\n", *(int*)ptr, pthread_self(), empty.__align, full.__align);
        while (empty.__align <= 0){
            printf("producer%d thread id = %d, 等待消费者取走货物，empty = %d, full = %d\n", *(int*)ptr, pthread_self(), empty.__align, full.__align);
            sleep(1);
        }//如果空闲的buffer数量 ≤ 0啥也不动, 也不要去占有mutex

        pthread_mutex_lock(&buffer_mutex);//加锁操作, 拿到lock 才去操作buffer
        sem_wait(&empty);//empty-- 看看empty能够--，能的话代表有空间，使buffer数量--

        //找到第一个不是1的bufferindex
        int i, product_index = -1;
        for (i = 0; i < SIEZE_OF_BUFFER; i++)
        {
            if(buffer[i] == 0){
                product_index = i;
            }
        }
        
        // //产品放入buffer
        buffer[product_index] = proCount++;
        printf("producer%d 生产货物%d, 放在了buffer[%d]里\n", *(int*)ptr, proCount, product_index);
        // int product_index = (proCount + 1) % SIEZE_OF_BUFFER;

        sem_post(&full);
        pthread_mutex_unlock(&buffer_mutex);//unlock
    }
    
}

void *consumer(void *ptr){
    while (1)
    {
        printf("\t\t\t\t\tconsumer%d thread id = %d, comsumer入口，当前empty = %d, full = %d\n", *(int*)ptr, pthread_self(), empty.__align, full.__align);
        while (full.__align <= 0){
            printf("\t\t\t\t\tconsumer%d thread id = %d, 等待生产者生产货物，empty = %d, full = %d\n", *(int*)ptr, pthread_self(), empty.__align, full.__align);
            sleep(1);
        }//如果满的buffer数量 ≤ 0啥也不动, 也不要去占有mutex

        pthread_mutex_lock(&buffer_mutex);//加锁再操作
        sem_wait(&full);//full-- full--，能的话代表有满的buffer
        
        //找到第一个存放有东西的地方
        int i, product_index = -1;
        for (i = 0; i < SIEZE_OF_BUFFER; i++)
        {
            if(buffer[i] != 0){
                product_index = i;
            }
        }

        //消费者动作，要清理buffer？？
        printf("\t\t\t\t\tcomsumer%d 取走 buffer[%d]里的货物-->%d\n", *(int*)ptr, product_index, buffer[product_index]);
        buffer[product_index] = 0;

        sem_post(&empty);
        pthread_mutex_unlock(&buffer_mutex);//unlock
    }
}

int main(int argc, char **argv){
    // printf("============= sizeof buffer:%d, buffer[0]:%d, buffer[0][0]:%d =============\n", sizeof(buffer), sizeof(buffer[0]), sizeof(buffer[0][0]));
    // printf("============= buffer addr:%p, buffer[0] addr:%p, buffer[0][0] addr:%p =============\n", buffer, buffer[0], &buffer[0][0]);
    // printf("============= buffer : %s, buffer[0] + 1 : %s, buffer[0][0] : %c =============\n", buffer, buffer[0], buffer[0][0]);

    sem_init(&full, 0, 0);//初始化到empty.__align
    sem_init(&empty, 0, SIEZE_OF_BUFFER);//初始化到full.__align

    int producerId[NUMBER_OF_PRODUCER];
    int consumerId[NUMBER_OF_CONSUMER];

    pthread_t producer_thread_id[NUMBER_OF_PRODUCER];//线程的id
    pthread_t consumer_thread_id[NUMBER_OF_CONSUMER];//线程的id

    int i,j;
    
    for(i = 0; i < NUMBER_OF_PRODUCER; i++){
        producerId[i] = i;
        if(pthread_create(&producer_thread_id[i], NULL, (void*)producer, (void*)&producerId[i]) != 0){
            printf("create producer[%d] error",i);
        }
    }

    
    for(i = 0; i < NUMBER_OF_CONSUMER; i++){
        consumerId[i] = i;
        if(pthread_create(&consumer_thread_id[i], NULL, (void*)consumer, (void*)&consumerId[i]) != 0){
            printf("create consumer[%d] error",i);
        }
    }
    
    for(j = 0; j < NUMBER_OF_PRODUCER; j++){
        if(pthread_join(producer_thread_id[j],NULL) != 0){
            printf("wait producer[%d] exit error",i);//waiting the chile_thread exit
        }else{
            printf("childthread exit\n"); 
        }
    }

    for(j = 0; j < NUMBER_OF_CONSUMER; j++){
        if(pthread_join(consumer_thread_id[j],NULL) != 0){
            printf("wait consumer[%d] exit error",i);//waiting the chile_thread exit
        }else{
            printf("childthread exit\n"); 
        }
    }

    

}
