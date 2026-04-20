#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// 虚拟磁盘配置
#define DISK_SIZE 1024 * 1024 // 1MB虚拟磁盘
#define BLOCK_SIZE 512        // 块大小
#define MAX_FILES 16          // 最大文件数
#define MAX_FILENAME 32       // 最大文件名长度

// 虚拟磁盘结构
typedef struct {
    char name[MAX_FILENAME];
    int size;
    int start_block;
    int block_count;
    int permissions; // 权限位
} FileEntry;

// 虚拟磁盘全局变量
FILE *disk_file = NULL;
FileEntry file_table[MAX_FILES];
int file_count = 0;
char disk_filename[256];

// 函数声明
void init_disk();
void format_disk();
void list_files();
void create_file();
void write_file();
void read_file();
void modify_permissions();
void view_permissions();
void exit_program();
void save_file_table();
void load_file_table();

int main() {
    int choice;

    printf("===== 虚拟磁盘文件系统 =====\n");
    printf("请输入虚拟磁盘文件名：");
    scanf("%s", disk_filename);

    init_disk();

    while (1) {
        printf("\n请输入功能号：\n");
        printf("1: 列出虚拟磁盘中的文件\n");
        printf("2: 创建文件\n");
        printf("3: 写文件\n");
        printf("4: 读文件\n");
        printf("5: 修改文件权限\n");
        printf("6: 查看文件权限\n");
        printf("7: 格式化虚拟磁盘\n");
        printf("0: 退出\n");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                list_files();
                break;
            case 2:
                create_file();
                break;
            case 3:
                write_file();
                break;
            case 4:
                read_file();
                break;
            case 5:
                modify_permissions();
                break;
            case 6:
                view_permissions();
                break;
            case 7:
                format_disk();
                break;
            case 0:
                exit_program();
                return 0;
            default:
                printf("无效的功能号，请重新输入\n");
        }
    }
}

// 初始化虚拟磁盘
void init_disk() {
    // 尝试打开虚拟磁盘文件
    disk_file = fopen(disk_filename, "rb+");
    if (disk_file == NULL) {
        // 文件不存在，创建新的虚拟磁盘
        disk_file = fopen(disk_filename, "wb+");
        if (disk_file == NULL) {
            perror("创建虚拟磁盘失败");
            exit(1);
        }
        // 初始化磁盘空间
        char buffer[BLOCK_SIZE] = {0};
        for (int i = 0; i < DISK_SIZE / BLOCK_SIZE; i++) {
            fwrite(buffer, 1, BLOCK_SIZE, disk_file);
        }
        // 初始化文件表
        memset(file_table, 0, sizeof(file_table));
        file_count = 0;
        save_file_table();
        printf("虚拟磁盘创建成功\n");
    } else {
        // 文件存在，加载文件表
        load_file_table();
        printf("虚拟磁盘加载成功\n");
    }
}

// 格式化虚拟磁盘
void format_disk() {
    // 清空文件表
    memset(file_table, 0, sizeof(file_table));
    file_count = 0;
    
    // 清空磁盘空间
    fseek(disk_file, 0, SEEK_SET);
    char buffer[BLOCK_SIZE] = {0};
    for (int i = 0; i < DISK_SIZE / BLOCK_SIZE; i++) {
        fwrite(buffer, 1, BLOCK_SIZE, disk_file);
    }
    
    save_file_table();
    printf("虚拟磁盘格式化成功\n");
}

// 保存文件表到磁盘
void save_file_table() {
    fseek(disk_file, 0, SEEK_SET);
    fwrite(&file_count, sizeof(int), 1, disk_file);
    fwrite(file_table, sizeof(FileEntry), MAX_FILES, disk_file);
}

// 从磁盘加载文件表
void load_file_table() {
    fseek(disk_file, 0, SEEK_SET);
    fread(&file_count, sizeof(int), 1, disk_file);
    fread(file_table, sizeof(FileEntry), MAX_FILES, disk_file);
}

// 列出虚拟磁盘中的文件
void list_files() {
    printf("虚拟磁盘中的文件：\n");
    printf("%-20s %-10s %-10s\n", "文件名", "大小", "权限");
    printf("----------------------------------------\n");
    
    for (int i = 0; i < file_count; i++) {
        printf("%-20s %-10d %-10o\n", 
               file_table[i].name, 
               file_table[i].size, 
               file_table[i].permissions);
    }
    
    if (file_count == 0) {
        printf("虚拟磁盘中无文件\n");
    }
}

// 创建文件
void create_file() {
    if (file_count >= MAX_FILES) {
        printf("虚拟磁盘文件数量已达上限\n");
        return;
    }
    
    char filename[MAX_FILENAME];
    printf("请输入文件名：");
    scanf("%s", filename);
    
    // 检查文件是否已存在
    for (int i = 0; i < file_count; i++) {
        if (strcmp(file_table[i].name, filename) == 0) {
            printf("文件已存在\n");
            return;
        }
    }
    
    // 分配文件表项
    strcpy(file_table[file_count].name, filename);
    file_table[file_count].size = 0;
    file_table[file_count].start_block = (file_count + 1) * 2; // 跳过文件表占用的块
    file_table[file_count].block_count = 1;
    file_table[file_count].permissions = 0644; // 默认权限
    
    file_count++;
    save_file_table();
    printf("文件创建成功\n");
}

// 写文件
void write_file() {
    char filename[MAX_FILENAME];
    char content[1024];
    int file_index = -1;
    
    printf("请输入文件名：");
    scanf("%s", filename);
    
    // 查找文件
    for (int i = 0; i < file_count; i++) {
        if (strcmp(file_table[i].name, filename) == 0) {
            file_index = i;
            break;
        }
    }
    
    if (file_index == -1) {
        printf("文件不存在\n");
        return;
    }
    
    printf("请输入文件内容（Windows下按Ctrl+Z，Linux下按Ctrl+D结束）：\n");
    fflush(stdin);
    
    // 清空文件内容
    file_table[file_index].size = 0;
    
    // 写入文件内容
    int total_written = 0;
    while (fgets(content, sizeof(content), stdin) != NULL) {
        int len = strlen(content);
        int offset = file_table[file_index].start_block * BLOCK_SIZE + total_written;
        
        fseek(disk_file, offset, SEEK_SET);
        fwrite(content, 1, len, disk_file);
        
        total_written += len;
        file_table[file_index].size = total_written;
    }
    
    save_file_table();
    printf("文件写入成功\n");
    clearerr(stdin);
}

// 读文件
void read_file() {
    char filename[MAX_FILENAME];
    int file_index = -1;
    
    printf("请输入文件名：");
    scanf("%s", filename);
    
    // 查找文件
    for (int i = 0; i < file_count; i++) {
        if (strcmp(file_table[i].name, filename) == 0) {
            file_index = i;
            break;
        }
    }
    
    if (file_index == -1) {
        printf("文件不存在\n");
        return;
    }
    
    printf("文件内容：\n");
    
    // 读取文件内容
    char buffer[BLOCK_SIZE];
    int offset = file_table[file_index].start_block * BLOCK_SIZE;
    int remaining = file_table[file_index].size;
    
    fseek(disk_file, offset, SEEK_SET);
    
    while (remaining > 0) {
        int read_size = remaining < BLOCK_SIZE ? remaining : BLOCK_SIZE;
        fread(buffer, 1, read_size, disk_file);
        fwrite(buffer, 1, read_size, stdout);
        remaining -= read_size;
    }
    
    printf("\n");
}

// 修改文件权限
void modify_permissions() {
    char filename[MAX_FILENAME];
    int permissions;
    int file_index = -1;
    
    printf("请输入文件名：");
    scanf("%s", filename);
    
    // 查找文件
    for (int i = 0; i < file_count; i++) {
        if (strcmp(file_table[i].name, filename) == 0) {
            file_index = i;
            break;
        }
    }
    
    if (file_index == -1) {
        printf("文件不存在\n");
        return;
    }
    
    printf("可修改的权限位：\n");
    printf("读权限：4\n");
    printf("写权限：2\n");
    printf("执行权限：1\n");
    printf("请输入新的权限值（例如：755）：");
    scanf("%o", &permissions);
    
    file_table[file_index].permissions = permissions;
    save_file_table();
    printf("文件权限修改成功\n");
}

// 查看文件权限
void view_permissions() {
    char filename[MAX_FILENAME];
    int file_index = -1;
    
    printf("请输入文件名：");
    scanf("%s", filename);
    
    // 查找文件
    for (int i = 0; i < file_count; i++) {
        if (strcmp(file_table[i].name, filename) == 0) {
            file_index = i;
            break;
        }
    }
    
    if (file_index == -1) {
        printf("文件不存在\n");
        return;
    }
    
    printf("文件权限：%o\n", file_table[file_index].permissions);
}

// 退出程序
void exit_program() {
    if (disk_file != NULL) {
        fclose(disk_file);
    }
    printf("程序退出\n");
}
