#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

// 函数声明
void list_directory();
void open_file();
void write_file();
void read_file();
void modify_permissions();
void view_permissions();

int main() {
    int choice;

    while (1) {
        printf("请输入功能号：\n");
        printf("1: 列出当前文件夹下所有文件名\n");
        printf("2: 打开已存在的文件或创建新文件\n");
        printf("3: 写文件\n");
        printf("4: 读文件\n");
        printf("5: 修改文件权限并提示可修改的权限位\n");
        printf("6: 查看当前文件权限\n");
        printf("0: 退出\n");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                list_directory();
                break;
            case 2:
                open_file();
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
            case 0:
                printf("程序退出\n");
                return 0;
            default:
                printf("无效的功能号，请重新输入\n");
        }
        printf("\n");
    }
}

// 列出当前文件夹下所有文件名
void list_directory() {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(".");
    if (dir == NULL) {
        perror("opendir failed");
        return;
    }

    printf("当前目录下的文件和文件夹：\n");
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);
}

// 打开已存在的文件或创建新文件
void open_file() {
    char filename[256];
    FILE *file;

    printf("请输入文件名：");
    scanf("%s", filename);

    file = fopen(filename, "a+");
    if (file == NULL) {
        perror("fopen failed");
        return;
    }

    printf("文件打开或创建成功\n");
    fclose(file);
}

// 写文件
void write_file() {
    char filename[256];
    char content[1024];
    FILE *file;

    printf("请输入文件名：");
    scanf("%s", filename);

    file = fopen(filename, "w");
    if (file == NULL) {
        perror("fopen failed");
        return;
    }

    printf("请输入文件内容（输入EOF结束，Windows下按Ctrl+Z）：\n");
    fflush(stdin);
    while (fgets(content, sizeof(content), stdin) != NULL) {
        fputs(content, file);
    }

    fclose(file);
    printf("文件写入成功\n");
}

// 读文件
void read_file() {
    char filename[256];
    char buffer[1024];
    FILE *file;

    printf("请输入文件名：");
    scanf("%s", filename);

    file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen failed");
        return;
    }

    printf("文件内容：\n");
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }

    fclose(file);
}

// 修改文件权限并提示可修改的权限位
void modify_permissions() {
    char filename[256];
    int permissions;

    printf("请输入文件名：");
    scanf("%s", filename);

    printf("可修改的权限位：\n");
    printf("读权限：4\n");
    printf("写权限：2\n");
    printf("执行权限：1\n");
    printf("请输入新的权限值（例如：755）：");
    scanf("%o", &permissions);

    if (chmod(filename, permissions) == -1) {
        perror("chmod failed");
        return;
    }

    printf("文件权限修改成功\n");
}

// 查看当前文件权限
void view_permissions() {
    char filename[256];
    struct stat stat_buf;

    printf("请输入文件名：");
    scanf("%s", filename);

    if (stat(filename, &stat_buf) == -1) {
        perror("stat failed");
        return;
    }

    printf("文件权限：%o\n", stat_buf.st_mode & 0777);
}
