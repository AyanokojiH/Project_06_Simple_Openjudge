#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h> 
#include <dirent.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>

#define MAX_FILE_SIZE 1024 * 1024 
#define MAX_FILENAME 256
#define MAX_TIME 2000
#define MAX_MEMORY 65536
#define RED   "\x1B[31m"
#define BLUE  "\x1B[34m"
#define RESET "\x1B[0m"

typedef struct {
    char filename[MAX_FILENAME];
    int number;
} InputFile;

int compare(const void *a, const void *b) {
    const InputFile *file1 = (const InputFile *)a;
    const InputFile *file2 = (const InputFile *)b;
    return file1->number - file2->number;
}

void handle_alarm(int sig) {
    // 处理超时信号
    printf(RED "TIME LIMIT EXCEEDED\n" RESET);
    exit(EXIT_FAILURE);
}

void handle_sigxfsz(int sig) {
    // 处理超出文件大小限制的信号
    printf(RED "MEMORY LIMIT EXCEEDED\n" RESET);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <n>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "n must be a positive integer\n");
        exit(EXIT_FAILURE);
    }

    char newdir[MAX_FILENAME];
    snprintf(newdir, sizeof(newdir), "info%d", n);

    bool accepted = true;
    DIR *dir;
    struct dirent *ent;
    InputFile files[30];  // 假设最多有30个测试用例
    int file_count = 0;

    chdir(newdir);

    // 检查std_input和std_output文件夹是否存在
    if (access("std_input", F_OK) == -1) {
        perror("std_input directory does not exist");
        exit(EXIT_FAILURE);
    }
    if (access("std_output", F_OK) == -1) {
        perror("std_output directory does not exist");
        exit(EXIT_FAILURE);
    }

    // 打开std_input文件夹
    dir = opendir("std_input");
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    // 读取所有文件名并存储
    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, "stdin", 5) == 0 && strstr(ent->d_name, ".txt") != NULL) {
            char *p = ent->d_name + 5; // 跳过"stdin"
            int num = atoi(p); // 提取数字
            snprintf(files[file_count].filename, sizeof(files[file_count].filename), "std_input/%s", ent->d_name);
            files[file_count].number = num;
            file_count++;
        }
    }

    // 关闭目录
    closedir(dir);

    // 对文件名进行排序
    qsort(files, file_count, sizeof(InputFile), compare);

    // 遍历排序后的文件名
    for (int i = 0; i < file_count; i++) {
        const char *input_filename = files[i].filename;
        int num = files[i].number;
        char output_filename[MAX_FILENAME];
        char std_output_filename[MAX_FILENAME];
        char user_output[MAX_FILE_SIZE];
        char std_output[MAX_FILE_SIZE];
        FILE *user_file, *std_file;
        int status;
        pid_t pid;
        struct timeval start, end;
        struct rusage usage;  // 用于存储资源使用情况
        long mtime, seconds, useconds;
        int diff;

        snprintf(output_filename, sizeof(output_filename), "user_output/user_output%d.txt", num);
        snprintf(std_output_filename, sizeof(std_output_filename), "std_output/stdout%d.txt", num);

        // 检查文件是否存在
        if (access(input_filename, F_OK) == -1) {
            perror("Input file does not exist");
            continue;
        }
        if (access(std_output_filename, F_OK) == -1) {
            perror("Std output file does not exist");
            continue;
        }

        gettimeofday(&start, NULL); // 记录开始时间

        // 创建子进程
        pid = fork();
        if (pid == -1) {
            perror("fork");
            continue; // 如果fork失败，跳过当前测试用例
        }

        if (pid == 0) {
            // 子进程
            struct rlimit limit;
            limit.rlim_cur = MAX_MEMORY * 1024; // 设置内存限制
            limit.rlim_max = MAX_MEMORY * 1024;
            if (setrlimit(RLIMIT_AS, &limit) == -1) {
                perror("setrlimit");
                exit(EXIT_FAILURE);
            }

            int input_fd = open(input_filename, O_RDONLY);
            if (input_fd == -1) {
                perror("open input file");
                exit(EXIT_FAILURE);
            }

            int output_fd = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd == -1) {
                close(input_fd);
                perror("open output file");
                exit(EXIT_FAILURE);
            }

            dup2(input_fd, STDIN_FILENO);
            dup2(output_fd, STDOUT_FILENO);
            close(input_fd);
            close(output_fd);

            // 使用相对路径调用 ans 文件
            execlp("../ans", "../ans", (char *)NULL);
            perror("execlp");
            exit(EXIT_FAILURE);
        } else {
            // 父进程
            // 设置时间限制
            signal(SIGALRM, handle_alarm);
            alarm(MAX_TIME / 1000); // 设置超时时间为MAX_TIME秒

            // 设置内存限制
            signal(SIGXFSZ, handle_sigxfsz);

            waitpid(pid, &status, 0);
            alarm(0); // 取消时间限制
            gettimeofday(&end, NULL); // 记录结束时间

            // 获取子进程的资源使用情况
            if (getrusage(RUSAGE_CHILDREN, &usage) == -1) {
                perror("getrusage");
                continue;
            }

            // 计算实际运行时间
            seconds = end.tv_sec - start.tv_sec;
            useconds = end.tv_usec - start.tv_usec;
            mtime = ((seconds) * 1000 + useconds / 1000.0) + 0.5;

            // 读取输出文件并比较
            user_file = fopen(output_filename, "r");
            if (user_file == NULL) {
                perror("fopen user output file");
                continue; // 如果打开文件失败，跳过当前测试用例
            }

            std_file = fopen(std_output_filename, "r");
            if (std_file == NULL) {
                fclose(user_file);
                perror("fopen std output file");
                continue; // 如果打开文件失败，跳过当前测试用例
            }

            size_t user_output_size = fread(user_output, 1, MAX_FILE_SIZE, user_file);
            size_t std_output_size = fread(std_output, 1, MAX_FILE_SIZE, std_file);
            fclose(user_file);
            fclose(std_file);

            // 比较输出
            diff = strncmp(user_output, std_output, user_output_size < std_output_size ? user_output_size : std_output_size);
            if (diff == 0 && mtime <= MAX_TIME) {
                printf("Case %d: Time %ldms,    Mem %ld KB,     Diff = 0,   Ok accepted\n", num, mtime, usage.ru_maxrss / 1024);
            } 
            else if (diff == 0 && mtime > MAX_TIME) {
                printf(RED "Case %d: Time %ldms,    Mem %ld KB,    Diff = 0,  TIME LIMIT EXCEEDED" RESET "\n\n", num, mtime, usage.ru_maxrss / 1024);
                return 0;
            }
            else if (diff == 0 && usage.ru_maxrss / 1024 >= MAX_MEMORY){
                printf(RED "Case %d: Time %ldms,     Mem %ld KB,     Diff = 0,  MEMORY LIMIT EXCEEDED" RESET "\n\n", num, mtime, usage.ru_maxrss / 1024);
                return 0;
            }
            else {
                printf(RED "Case %d: Time %ldms,    Mem %ld KB,     Diff = -1   " RESET, num, mtime, usage.ru_maxrss / 1024);
                printf(RED "WRONG ANSWER\n\n" RESET);
                accepted = false;
                printf(RED "WRONG ANSWER\n" RESET);
                return 0;
            }
        }
    }

    if (accepted) {
        printf(BLUE "ACCEPTED\n" RESET);
    }

    return 0;
}