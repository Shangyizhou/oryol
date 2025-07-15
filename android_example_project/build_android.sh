#!/bin/bash

# Android构建脚本
# 使用方法: ./build_android.sh [debug|release]

set -e

# 检查参数
BUILD_TYPE=${1:-debug}
if [ "$BUILD_TYPE" != "debug" ] && [ "$BUILD_TYPE" != "release" ]; then
    echo "错误: 构建类型必须是 'debug' 或 'release'"
    echo "使用方法: $0 [debug|release]"
    exit 1
fi

echo "开始构建Android应用 (类型: $BUILD_TYPE)"

# 检查环境变量
if [ -z "$ANDROID_NDK" ]; then
    echo "错误: 请设置 ANDROID_NDK 环境变量"
    echo "例如: export ANDROID_NDK=/path/to/android-ndk"
    exit 1
fi

if [ -z "$ANDROID_SDK" ]; then
    echo "错误: 请设置 ANDROID_SDK 环境变量"
    echo "例如: export ANDROID_SDK=/path/to/android-sdk"
    exit 1
fi

echo "Android NDK: $ANDROID_NDK"
echo "Android SDK: $ANDROID_SDK"

# 设置构建配置
if [ "$BUILD_TYPE" = "debug" ]; then
    FIPS_CONFIG="android-debug"
    echo "使用调试配置"
else
    FIPS_CONFIG="android-release"
    echo "使用发布配置"
fi

# 清理之前的构建
echo "清理之前的构建..."
./fips clean

# 构建项目
echo "构建Android应用..."
./fips build $FIPS_CONFIG

# 检查构建结果
if [ $? -eq 0 ]; then
    echo "构建成功!"
    echo "APK文件位置: fips-builds/MyAndroidApp/android-debug/app-debug.apk"
    
    # 显示APK信息
    if [ -f "fips-builds/MyAndroidApp/android-debug/app-debug.apk" ]; then
        echo "APK文件大小: $(du -h fips-builds/MyAndroidApp/android-debug/app-debug.apk | cut -f1)"
    fi
else
    echo "构建失败!"
    exit 1
fi

echo "构建完成!"