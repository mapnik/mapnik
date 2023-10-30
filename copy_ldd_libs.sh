#!/usr/bin/env bash
# 用法：./copy_ldd_libs.sh <可执行程序> <目标目录>
# 功能：拷贝一个可执行程序的ldd依赖的所有库到指定目录
# 参考：[1] [2]

# 检查参数个数
if [ $# -ne 2 ]; then
    echo "参数错误！"
    echo "用法：./copy_ldd_libs.sh <可执行程序> <目标目录>"
    exit 1
fi

# 检查可执行程序是否存在
if [ ! -f $1 ]; then
    echo "可执行程序不存在！"
    exit 2
fi

# 检查目标目录是否存在，如果不存在则创建
if [ ! -d $2 ]; then
    mkdir -p $2
fi

# 使用ldd命令获取依赖的库列表，并过滤掉不需要的行和列
ldd $1 | grep -v 'linux-vdso' | grep -v 'ld-linux' | awk '{print $3}' > libs.txt

# 遍历库列表，使用cp命令拷贝到目标目录，并保留原始的符号链接
while read lib; do
    cp -L $lib $2
done < libs.txt

# 删除临时文件
rm libs.txt

# 打印完成信息
echo "拷贝完成！"
