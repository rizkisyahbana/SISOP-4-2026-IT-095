#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>

char dir_path[1024];

// Mengambil fragmen KOORD: dari 1.txt - 7.txt
void get_tujuan_content(char *f_buf) {
    strcpy(f_buf, "Tujuan Mas Amba: ");
    char line[512];

    for (int i = 1; i <= 7; i++) {
        char path_asal[1024];
        snprintf(path_asal, sizeof(path_asal), "%s/%d.txt", dir_path, i);
        FILE *f = fopen(path_asal, "r");
        if (f) {
            while (fgets(line, sizeof(line), f)) {
                if (strncmp(line, "KOORD:", 6) == 0) {
                    char *fragment = line + 6;
                    // Bersihkan spasi depan
                    while (*fragment == ' ') fragment++;
                    // Bersihkan newline belakang
                    fragment[strcspn(fragment, "\r\n")] = 0;
                    strcat(f_buf, fragment);
                    break;
                }
            }
            fclose(f);
        }
    }
    strcat(f_buf, "\n");
}

static int xmp_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void) fi;
    memset(stbuf, 0, sizeof(struct stat));

    // Handle file virtual tujuan.txt
    if (strcmp(path, "/tujuan.txt") == 0) {
        char temp[2048];
        get_tujuan_content(temp);
        stbuf->st_mode = S_IFREG | 0444; // Read only
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(temp);
        return 0;
    }

    // Passthrough file asli
    char fpath[1024];
    snprintf(fpath, sizeof(fpath), "%s%s", dir_path, path);
    int res = lstat(fpath, stbuf);
    if (res == -1) return -errno;
    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    (void) offset; (void) fi; (void) flags;

    char fpath[1024];
    snprintf(fpath, sizeof(fpath), "%s%s", dir_path, path);

    DIR *dp = opendir(fpath);
    if (dp == NULL) return -errno;

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0, 0)) break;
    }
    closedir(dp);

    // Selipkan tujuan.txt di ls
    if (strcmp(path, "/") == 0) {
        filler(buf, "tujuan.txt", NULL, 0, 0);
    }
    return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path, "/tujuan.txt") == 0) {
        if ((fi->flags & O_ACCMODE) != O_RDONLY) return -EACCES;
        return 0;
    }
    int res;
    char fpath[1024];
    snprintf(fpath, sizeof(fpath), "%s%s", dir_path, path);
    res = open(fpath, fi->flags);
    if (res == -1) return -errno;
    close(res);
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;

    if (strcmp(path, "/tujuan.txt") == 0) {
        char temp[2048];
        get_tujuan_content(temp);
        size_t len = strlen(temp);
        if (offset < len) {
            if (offset + size > len) size = len - offset;
            memcpy(buf, temp + offset, size);
        } else size = 0;
        return size;
    }

    char fpath[1024];
    snprintf(fpath, sizeof(fpath), "%s%s", dir_path, path);
    int fd = open(fpath, O_RDONLY);
    if (fd == -1) return -errno;
    int res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;
    close(fd);
    return res;
}

static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .open    = xmp_open,
    .read    = xmp_read,
};

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: ./kenz_rescue <source_dir> <mount_dir>\n");
        return 1;
    }
    realpath(argv[1], dir_path); // Dapatkan absolute path
    
    // Sesuaikan parameter untuk fuse_main
    argv[1] = argv[2];
    argc--;
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
