// 调用命令窗口进行VIC运行
// Created by HenryChin on 2024/7/22.
//

//PBE: vic classic是单一进程，效率没有image模式调用openmpi搞，但是cygwin无法编译vic_image, 显示linux clib execinfo.h missing，
//This header file is missing on some platforms: musl libc, FreeBSD 9.3, NetBSD 6.1, OpenBSD 6.9, Minix 3.1.8, AIX 5.1, HP-UX 11, Solaris 10, Cygwin, mingw, MSVC 14, Android 9.0.
// https://www.gnu.org/software/gnulib/manual/html_node/execinfo_002eh.html
//所以要再源码整体替换成Window运行库比较麻烦，参考中山大学 “VIC5” R包里面实现方式。
//因此考虑 用wsl 来调用 vic5 从而实现多进程
# include "cstdlib"

void runVIC(){
    system("dir"); // 调用 vic_image 进行径流模拟，全局文件在global_params
}
