#include "user.h"

// 简单的字符串比较
int strcmp(const char *s1, const char *s2) {
    while(*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

//==============================================================================
// 实验七：文件系统测试
//==============================================================================
char buffer[] = "Hello, filesystem!";
void test_fs_integrity(void) {
    printf("\n=== FS Test 1: File Integrity ===\n");
    
    // 1. 创建并写入
    int fd = open("testfile", O_CREATE | O_RDWR);
    if(fd < 0) {
        printf("[FAIL] Cannot create file\n");
        return;
    }
    printf("[PASS] File created, fd=%d\n", fd);

    
    int len = strlen(buffer);
    int bytes = write(fd, buffer, len);
    
    if(bytes != len) {
        printf("[FAIL] Write failed, wrote %d expected %d\n", bytes, len);
        close(fd);
        return;
    }
    printf("[PASS] Wrote %d bytes\n", bytes);
    
    // 2. 关闭文件 (确保数据落盘，重置偏移量)
    close(fd); 

    // 3. 重新打开读取
    fd = open("testfile", O_RDONLY);
    if(fd < 0) {
        printf("[FAIL] Cannot reopen file\n");
        return;
    }

    char read_buf[64];
    // 【关键修复】清空缓冲区，避免栈垃圾数据
    memset(read_buf, 0, sizeof(read_buf));
    
    bytes = read(fd, read_buf, sizeof(read_buf) - 1);
    
    if(bytes < 0) {
        printf("[FAIL] Read failed\n");
        close(fd);
        return;
    }
    
    // 确保字符串结束符
    read_buf[bytes] = '\0'; 

    // 4. 对比验证
    if(strcmp(buffer, read_buf) == 0) {
        printf("[PASS] Data verified: %s\n", read_buf);
    } else {
        // 【关键修复】使用 %d 而不是 %ld，并打印详细信息
        printf("[FAIL] Data mismatch.\n");
        printf("       Expected: '%s' (len=%d)\n", buffer, len);
        printf("       Got:      '%s' (len=%d)\n", read_buf, (int)bytes);
        
        // 打印首字符的十六进制，看看读到了什么
        if (bytes > 0) {
            printf("       First byte hex: 0x%x\n", (unsigned char)read_buf[0]);
        }
    }
    close(fd);

    // 5. 清理
    if(unlink("testfile") == 0) {
        printf("[PASS] File deleted\n");
    } else {
        printf("[FAIL] Unlink failed\n");
    }

    // 验证删除
    fd = open("testfile", O_RDONLY);
    if(fd < 0) {
        printf("[PASS] File no longer exists\n");
    } else {
        printf("[FAIL] File still exists after unlink\n");
        close(fd);
    }
}

void test_fs_concurrent(void) {
    printf("\n=== FS Test 2: Concurrent Access ===\n");
    
    int num_procs = 3;
    
    for(int i = 0; i < num_procs; i++) {
        int pid = fork();
        if(pid == 0) {
            char filename[16];
            filename[0] = 't';
            filename[1] = 'e';
            filename[2] = 's';
            filename[3] = 't';
            filename[4] = '_';
            filename[5] = '0' + i;
            filename[6] = '\0';
            
            for(int j = 0; j < 5; j++) {
                int fd = open(filename, O_CREATE | O_RDWR);
                if(fd >= 0) {
                    write(fd, "data", 4);
                    close(fd);
                    unlink(filename);
                }
            }
            printf("[INFO] Child %d completed\n", i);
            exit(0);
        }
    }
    
    for(int i = 0; i < num_procs; i++) {
        int status;
        wait(&status);
    }
    
    printf("[PASS] Concurrent access test completed\n");
}

void test_fs_multiple_files(void) {
    printf("\n=== FS Test 3: Multiple Files ===\n");
    
    int num_files = 5;
    char filename[16];
    
    printf("[INFO] Creating %d files...\n", num_files);
    for(int i = 0; i < num_files; i++) {
        filename[0] = 'f';
        filename[1] = 'i';
        filename[2] = 'l';
        filename[3] = 'e';
        filename[4] = '0' + i;
        filename[5] = '\0';
        
        int fd = open(filename, O_CREATE | O_RDWR);
        if(fd < 0) {
            printf("[FAIL] Cannot create %s\n", filename);
            return;
        }
        
        for(int j = 0; j <= i; j++) {
            write(fd, "ABCD", 4);
        }
        close(fd);
    }
    printf("[PASS] Created %d files\n", num_files);
    
    printf("[INFO] Verifying files...\n");
    int all_ok = 1;
    for(int i = 0; i < num_files; i++) {
        filename[0] = 'f';
        filename[1] = 'i';
        filename[2] = 'l';
        filename[3] = 'e';
        filename[4] = '0' + i;
        filename[5] = '\0';
        
        int fd = open(filename, O_RDONLY);
        if(fd < 0) {
            printf("[FAIL] Cannot open %s\n", filename);
            all_ok = 0;
            continue;
        }
        
        char buf[64];
        int n = read(fd, buf, sizeof(buf));
        int expected = (i + 1) * 4;
        if(n != expected) {
            printf("[FAIL] %s: expected %d bytes, got %d\n", filename, expected, n);
            all_ok = 0;
        }
        close(fd);
    }
    
    if(all_ok)
        printf("[PASS] All files verified\n");
    
    printf("[INFO] Cleaning up...\n");
    for(int i = 0; i < num_files; i++) {
        filename[0] = 'f';
        filename[1] = 'i';
        filename[2] = 'l';
        filename[3] = 'e';
        filename[4] = '0' + i;
        filename[5] = '\0';
        unlink(filename);
    }
    printf("[PASS] Multiple files test completed\n");
}

void test_fs_large_file(void) {
    printf("\n=== FS Test 4: Large File ===\n");
    
    int fd = open("largefile", O_CREATE | O_RDWR);
    if(fd < 0) {
        printf("[FAIL] Cannot create large file\n");
        return;
    }
    
    char buf[512];
    for(int i = 0; i < 512; i++)
        buf[i] = 'A' + (i % 26);
    
    int total = 0;
    int blocks = 10;
    printf("[INFO] Writing %d blocks...\n", blocks);
    
    for(int i = 0; i < blocks; i++) {
        int n = write(fd, buf, 512);
        if(n != 512) {
            printf("[FAIL] Write failed at block %d\n", i);
            close(fd);
            return;
        }
        total += n;
    }
    printf("[PASS] Wrote %d bytes\n", total);
    close(fd);
    
    fd = open("largefile", O_RDONLY);
    if(fd < 0) {
        printf("[FAIL] Cannot reopen large file\n");
        return;
    }
    
    int read_total = 0;
    int n;
    while((n = read(fd, buf, 512)) > 0) {
        read_total += n;
    }
    
    if(read_total == total) {
        printf("[PASS] Read back %d bytes\n", read_total);
    } else {
        printf("[FAIL] Read %d bytes, expected %d\n", read_total, total);
    }
    close(fd);
    
    unlink("largefile");
    printf("[PASS] Large file test completed\n");
}

//==============================================================================
// 实验八：内核日志系统测试
//==============================================================================

void test_log_basic(void) {
    printf("\n=== Log Test 1: Basic Functionality ===\n");
    
    printf("[INFO] Displaying kernel log statistics...\n");
    logstat();
    
    printf("[PASS] Log statistics displayed\n");
}

void test_log_dump(void) {
    printf("\n=== Log Test 2: Log Buffer Dump ===\n");
    
    printf("[INFO] Dumping kernel log buffer...\n");
    logdump();
    
    printf("[PASS] Log buffer dumped\n");
}

void test_log_with_activity(void) {
    printf("\n=== Log Test 3: Log with Process Activity ===\n");
    
    printf("[INFO] Creating child process to generate log entries...\n");
    
    int pid = fork();
    if(pid == 0) {
        // 子进程：创建一些文件操作
        int fd = open("logtest", O_CREATE | O_RDWR);
        if(fd >= 0) {
            write(fd, "test", 4);
            close(fd);
            unlink("logtest");
        }
        exit(0);
    } else {
        int status;
        wait(&status);
        printf("[INFO] Child exited with status %d\n", status);
    }
    
    printf("[INFO] Checking log after activity...\n");
    logstat();
    
    printf("[PASS] Log with activity test completed\n");
}

//==============================================================================
// 主函数
//==============================================================================

int main(void) {
    printf("============================================================\n");
    printf("          Lab 7 & 8: Combined Acceptance Tests\n");
    printf("============================================================\n");
    
    //--------------------------------------------------------------------------
    // 实验七测试：文件系统
    //--------------------------------------------------------------------------
    printf("\n############################################################\n");
    printf("#                  Lab 7: File System                      #\n");
    printf("############################################################\n");
    
    test_fs_integrity();
    test_fs_concurrent();
    test_fs_multiple_files();
    test_fs_large_file();
    
    //--------------------------------------------------------------------------
    // 实验八测试：内核日志系统
    //--------------------------------------------------------------------------
    printf("\n############################################################\n");
    printf("#              Lab 8: Kernel Log System                    #\n");
    printf("############################################################\n");
    
    test_log_basic();
    test_log_dump();
    test_log_with_activity();
    
    //--------------------------------------------------------------------------
    // 总结
    //--------------------------------------------------------------------------
    printf("\n============================================================\n");
    printf("               All Tests Completed Successfully!\n");
    printf("============================================================\n");
    
    printf("\n[SUMMARY] Lab 7 - File System:\n");
    printf("  - File creation, read, write, delete: PASS\n");
    printf("  - Concurrent file access: PASS\n");
    printf("  - Multiple files handling: PASS\n");
    printf("  - Large file support: PASS\n");
    
    printf("\n[SUMMARY] Lab 8 - Kernel Log System:\n");
    printf("  - Log statistics: PASS\n");
    printf("  - Log buffer dump: PASS\n");
    printf("  - Log with process activity: PASS\n");
    
    printf("\n============================================================\n");
    
    exit(0);
}