#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <iostream>
namespace shared_memory_com {
    struct shm_info {
        size_t size = 0;
        int attach_num = 0;
    };
    static int create_shm(const char *path, int id, size_t size, int flags) {
        key_t key = ftok(path, id);
        if (key < 0) {
            std::cerr << "ftok error." << std::endl;
            return -1;
        }
        int shmid = 0;
        if ((shmid = shmget(key, size, flags)) < 0) {
            std::cerr << "shmget error." << std::endl;
            return -2;
        }
        return shmid;
    }
    template <typename T>
    static T *get_shm(int shmid, int flags = 0) {
        return static_cast<T *>(shmat(shmid, nullptr, flags));
    }
    static int destroy_shm(int shmid, const void *shm_addr) {
        if (shmid < 0) {
            std::cerr << "shm id:" << shmid << " error." << std::endl;
            return -1;
        }
        if (shmctl(shmid, IPC_RMID, nullptr) < 0) {     // remove first
            std::cerr << "shmctl IPC_RMID error." << std::endl;
            return -2;
        }
        if (shm_addr) {                                 // shmdt secondly
            shmdt(shm_addr);
        }
        return 0;
    }
    static int remove_shm_by_shmid(int shmid) {
        if (shmid < 0) {
            std::cerr << "shm id:" << shmid << " error." << std::endl;
            return -1;
        }
        if (shmctl(shmid, IPC_RMID, nullptr) < 0) {     // remove first
            std::cerr << "shmctl IPC_RMID error." << std::endl;
            return -2;
        }
        return 0;
    }
    static bool get_shm_info_by_shmid(int shmid, shm_info &info) {
        struct shmid_ds buf = { 0 };
        if (shmctl(shmid, IPC_STAT, &buf)) {
            return false;
        }
        info.size = buf.shm_segsz;
        info.attach_num = buf.shm_nattch;
        return true;
    }
    static size_t get_shm_size_by_key(key_t key) {
        char cmd[64] = "";
        snprintf(cmd, sizeof(cmd), "ipcs -m | grep 0x%08x | awk '{print $5}'", key);
        FILE *fp = popen(cmd, "r");
        if (!fp) {
            return 0;
        }
        char buf[64] = "";
        size_t size = 0;
        if (fgets(buf, sizeof(buf), fp)) {
            size = atoi(buf);
        }
        pclose(fp);
        usleep(100); // let subprocess to end
        return size;
    }
    static bool remove_shm_by_key(key_t key) {
        char cmd[64] = "";
        snprintf(cmd, sizeof(cmd), "ipcs -m | grep 0x%08x | awk '{print $2}'", key);
        FILE *fp = popen(cmd, "r");
        if (!fp) {
            return false;
        }
        char shmid[64] = "";
        bool succ = false;
        if (fgets(shmid, sizeof(shmid), fp)) {
            snprintf(cmd, sizeof(cmd), "ipcrm -m %s", shmid);
            succ = !system(cmd);
            usleep(100); // let subprocess to end
        }
        pclose(fp);
        return succ;
    }
}