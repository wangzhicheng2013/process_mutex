#include "process_mutex.hpp"
process_mutex global_process_mutex;
int main() {
    if (!global_process_mutex.init()) {
        return -1;
    }
    int shmid = shared_memory_com::create_shm(".", 0x2222, sizeof(int), IPC_CREAT | 0666);
    if (shmid < 0) {
        return -1;
    }
    volatile int *x = shared_memory_com::get_shm<int>(shmid);
    *x = 0;
    int pid = fork();
    if (pid < 0) {
        std::cerr << "fork failed." << std::endl;
        return -1;
    }
    static const int N = 1000000;
    for (int i = 0;i < N;++i) {
        if (pid > 0) {
            global_process_mutex.lock();
            int temp = *x;
            *x = temp + 1;
             global_process_mutex.unlock();

        } 
        else { 
            global_process_mutex.lock();
            int temp = *x;
            *x = temp + 2;
            global_process_mutex.unlock();
        }
    }
    printf("pid= %d, x_address= %p, x= %d\n", getpid(), x, *x);
    sleep(1);
    if (pid > 0) { 
        shared_memory_com::destroy_shm(shmid, (const void *)x);
        printf("parent process exit.\n");
    }

    return 0;
}