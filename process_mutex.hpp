#include <pthread.h>
#include "shared_memory_com.hpp"
// process_mutex should be defined in the global space inter different process
struct process_mutex_info {
    pthread_mutex_t lock;
    pthread_mutexattr_t lock_attr;
};
class process_mutex {
public:
    process_mutex() = default;
    ~process_mutex() {
        if (process_mutex_) {
            pthread_mutex_destroy(&(process_mutex_->lock));
            pthread_mutexattr_destroy(&(process_mutex_->lock_attr));
        }
        if (shared_memory_com::destroy_shm(process_mutex_shmid_, process_mutex_) < 0) {
            std::cerr << "process mutex destory failed." << std::endl;
        }
    }
public:
    bool init() {
        static const char *PATHNAME  = ".";
        static const int PROJ_ID = 0x7777;
        process_mutex_shmid_ = shared_memory_com::create_shm(PATHNAME, PROJ_ID, sizeof(process_mutex_info), IPC_CREAT | IPC_EXCL | 0666);
        if (process_mutex_shmid_ < 0) {
            std::cerr << "init process mutex failed." << std::endl;
            return false;
        }
        process_mutex_ = shared_memory_com::get_shm<process_mutex_info>(process_mutex_shmid_, SHM_R | SHM_W);
        if (!process_mutex_) {
            return false;
        }
        if (pthread_mutexattr_init(&(process_mutex_->lock_attr)) < 0) {
            return false;
        }
        if (pthread_mutexattr_setpshared(&(process_mutex_->lock_attr), PTHREAD_PROCESS_SHARED) < 0) {
            return false;
        }
        if (pthread_mutex_init(&(process_mutex_->lock), &(process_mutex_->lock_attr)) < 0) {
            return false;
        }
        return true;
    }
    inline void lock() {
        pthread_mutex_lock(&(process_mutex_->lock));
    }
    inline void unlock() {
        pthread_mutex_unlock(&(process_mutex_->lock));
    }
private:
    int process_mutex_shmid_ = 0;
    process_mutex_info *process_mutex_ = nullptr;
};