# SISOP-4-2026-IT-095

# Laporan Resmi Praktikum Sistem Operasi Modul 4

## Identitas

Nama : Nur Rizki Syahbana
NRP : 5027251095

# Soal 1 - Save Asisten Kenz

## Deskripsi Soal

Pada soal ini dimista untuk membuat filesystem virtual menggunakan FUSE dengan bahasa C, Filesystem ini menampilkan isi folder `amba_files` tanpa mengubah file asli.
Selain itu, filesystem juga harus membuat satu file virtual bernama `tujuan.txt` yang hanya muncul pada mount directory dan tidak ada pada source directory.
Isi dari `tujuan.txt` membaca koordinat dari file `1.txt` sampai `7.txt` yang memiliki `KOORD:` lalu menggabungkannya menjadi satu kalimat.

# Penjelasan Program

Program dibuat menggunakan FUSE (Filesystem in Userspace).
FUSE memungkinkan user membuat filesystem sendiri di userspace tanpa harus membuat kernel module.
Pada program ini terdapat dua jenis file:

1. File passthrough

   * File asli dari folder `amba_files`
   * Isi file diteruskan langsung ke mount directory

2. File virtual

   * `tujuan.txt`
   * Tidak ada di source directory
   * Dibuat secara otomatis oleh program FUSE

Filesystem menggunakan callback:

* `getattr`
* `readdir`
* `open`
* `read`

# Penjelasan Kode

## 1. Menentukan Versi FUSE

```c
#define FUSE_USE_VERSION 31
```

Digunakan untuk menentukan bahwa program menggunakan FUSE versi 3.

## 2. Import Library

```c
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
```

Library digunakan untuk:

* operasi filesystem
* membaca file
* membaca directory
* string
* mengambil file

## 3. Variabel Global

```c
char dir_path[1024];
```

Variabel ini digunakan untuk menyimpan lokasi source directory yaitu `amba_files`.

## 4. Fungsi get_tujuan_content

```c
void get_tujuan_content(char *f_buf)
```

Fungsi ini digunakan untuk membuat isi virtual file `tujuan.txt`.

Langkah kerja fungsi:

1. Membuat teks :

```text
Tujuan Mas Amba:
```

2. Membaca file `1.txt` sampai `7.txt`

3. Mencari baris dengan :

```text
KOORD:
```

4. Mengambil fragment koordinat

5. Menggabungkan semua fragment

6. Menambahkan newline di akhir output

Tidak asa file asli `tujuan.txt` pada source directory.

## 5. Callback getattr

```c
static int xmp_getattr(...)
```

Fungsi ini digunakan untuk mengambil metadata file seperti:

* ukuran file
* permission
* tipe file

Jika file yang diminta adalah `tujuan.txt`, maka program membuat metadata virtual.

Jika file biasa, metadata diambil langsung dari file asli menggunakan `lstat`.

## 6. Callback readdir

```c
static int xmp_readdir(...)
```

Fungsi ini digunakan saat menjalankan:

```bash
ls
```

Program membaca semua isi folder `amba_files` lalu menampilkannya pada mount directory.

Selain itu program juga menambahkan file virtual:

```text
tujuan.txt
```

agar muncul pada mount point.

## 7. Callback open

```c
static int xmp_open(...)
```

Digunakan saat file dibuka.

Untuk `tujuan.txt`, program hanya mengizinkan mode read-only.

Untuk file biasa, program membuka file asli dari source directory.

## 8. Callback read

```c
static int xmp_read(...)
```

Digunakan saat membaca isi file.

Jika file yang dibaca adalah `tujuan.txt`, maka isi file dibuat langsung menggunakan fungsi:

```c
get_tujuan_content()
```

Jika file biasa, program membaca isi file asli menggunakan `pread`.

## 9. Struct fuse_operations

```c
static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .open = xmp_open,
    .read = xmp_read,
};
```

Struct ini digunakan untuk menghubungkan callback ke FUSE.

## 10. Fungsi main

```c
int main(int argc, char *argv[])
```

Fungsi utama program.

Program dijalankan dengan format:

```bash
./kenz_rescue <source_dir> <mount_dir>
```

Contoh:

```bash
./kenz_rescue amba_files mnt
```

Program menyimpan lokasi source directory lalu menjalankan filesystem FUSE menggunakan `fuse_main()`.

# Cara Menjalankan Program

## 1. Install FUSE

```bash
sudo apt update
sudo apt install fuse3 libfuse3-dev
```

## 2. Masuk ke Folder Soal

```bash
cd SISOP-4-2026-IT-095/soal_1
```

## 3. Compile Program

```bash
gcc kenz_rescue.c -o kenz_rescue `pkg-config fuse3 --cflags --libs`
```

## 4. Membuat Mount Directory

```bash
mkdir -p mnt
```

## 5. Menjalankan Filesystem

```bash
./kenz_rescue amba_files mnt
```

Jika berhasil, terminal akan diam karena filesystem sedang berjalan.

# Contoh Penggunaan

## Melihat Isi Mount Directory

```bash
ls mnt
```

Output:

```text
1.txt
2.txt
3.txt
4.txt
5.txt
6.txt
7.txt
tujuan.txt
```

## Membaca File Passthrough

```bash
cat mnt/1.txt
```

Isi file akan sama persis dengan:

```bash
cat amba_files/1.txt
```

## Membaca File Virtual

```bash
cat mnt/tujuan.txt
```

Output:

```text
Tujuan Mas Amba: <hasil gabungan koordinat>
```

## Mengecek tujuan.txt tidak Ada di Source Directory

```bash
ls amba_files
```

Output:

```text
1.txt
2.txt
3.txt
4.txt
5.txt
6.txt
7.txt
```

File `tujuan.txt` tidak ada secara fisik pada source directory.

# Unmount Filesystem

```bash
fusermount3 -u mnt
```

# Dokumentasi Output

## Compile Program

Tambahkan screenshot:

```text
gcc kenz_rescue.c -o kenz_rescue `pkg-config fuse3 --cflags --libs`
```

## Menjalankan Filesystem

Tambahkan screenshot:

```text
./kenz_rescue amba_files mnt
```

## Isi Mount Directory

Tambahkan screenshot:

```text
ls mnt
```

## Membaca tujuan.txt

Tambahkan screenshot:

```text
cat mnt/tujuan.txt
```

## Membandingkan File Passthrough

Tambahkan screenshot:

```text
diff mnt/1.txt amba_files/1.txt
```

## Unmount Filesystem

Tambahkan screenshot:

```text
fusermount3 -u mnt
```
