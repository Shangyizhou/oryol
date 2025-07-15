# Android示例项目

这是一个使用Oryol库开发的完整Android应用示例，展示了如何在Android平台上使用Oryol进行3D图形渲染、触摸输入处理和传感器数据获取。

## 功能特性

- **3D图形渲染**: 渲染一个彩色旋转立方体
- **触摸输入**: 支持多点触摸，可以通过触摸控制立方体旋转
- **传感器支持**: 集成加速度计和陀螺仪数据
- **生命周期管理**: 正确处理Android应用的生命周期事件
- **自适应显示**: 支持不同屏幕尺寸和方向

## 项目结构

```
android_example_project/
├── MyAndroidApp.cc          # 主应用代码
├── shaders.glsl             # 着色器文件
├── CMakeLists.txt           # 构建配置
├── AndroidManifest.xml      # Android清单文件
├── build_android.sh         # 构建脚本
└── README.md               # 项目说明
```

## 环境要求

### 必需工具
- **Android NDK** (推荐版本: r25或更高)
- **Android SDK** (推荐版本: API 33或更高)
- **CMake** (3.18或更高)
- **Python** (3.7或更高，用于fips构建工具)
- **C++编译器** (支持C++11)

### 支持的Android版本
- **最低版本**: Android 5.0 (API 21)
- **目标版本**: Android 13 (API 33)
- **OpenGL ES**: 3.0或更高

## 安装和配置

### 1. 设置环境变量

```bash
# 设置Android NDK路径
export ANDROID_NDK=/path/to/android-ndk

# 设置Android SDK路径
export ANDROID_SDK=/path/to/android-sdk

# 将Android工具添加到PATH
export PATH=$PATH:$ANDROID_SDK/platform-tools
export PATH=$PATH:$ANDROID_SDK/tools
```

### 2. 克隆Oryol项目

```bash
git clone https://github.com/floooh/oryol.git
cd oryol
```

### 3. 复制示例项目

```bash
# 将示例项目复制到Oryol根目录
cp -r android_example_project/ code/Samples/
```

### 4. 构建项目

```bash
# 进入示例目录
cd code/Samples/android_example_project

# 使用构建脚本
chmod +x build_android.sh
./build_android.sh debug
```

## 构建选项

### 调试构建
```bash
./build_android.sh debug
```

### 发布构建
```bash
./build_android.sh release
```

### 手动构建
```bash
# 清理构建
./fips clean

# 构建Android应用
./fips build android-debug

# 或者构建发布版本
./fips build android-release
```

## 安装和运行

### 1. 连接Android设备

```bash
# 检查设备连接
adb devices
```

### 2. 安装APK

```bash
# 安装调试版本
adb install fips-builds/MyAndroidApp/android-debug/app-debug.apk

# 或者使用fips命令
./fips install android-debug
```

### 3. 运行应用

```bash
# 启动应用
adb shell am start -n com.example.myandroidapp/android.app.NativeActivity

# 或者使用fips命令
./fips run android-debug
```

## 代码说明

### 主要类结构

```cpp
class MyAndroidApp : public App {
public:
    // 应用生命周期
    AppState::Code OnInit();
    AppState::Code OnRunning();
    AppState::Code OnCleanup();
    
    // Android特定回调
    void OnSuspend() override;
    void OnResume() override;
    void OnWindowResize(int width, int height) override;

private:
    // 渲染相关
    DrawState drawState;
    Id shader, mesh, texture;
    
    // 变换矩阵
    glm::mat4 modelMatrix, viewMatrix, projMatrix, mvpMatrix;
    
    // 动画和传感器
    float rotationAngle;
    TimePoint lastFrameTime;
    glm::vec3 accelerometerData, gyroscopeData;
    
    // 私有方法
    void onSensorEvent(const ASensorEvent* event);
    void updateTransform();
    void renderScene();
    void handleTouchInput();
};
```

### 关键功能实现

#### 1. Android入口点
```cpp
// 全局Android应用状态
android_app* OryolAndroidAppState = nullptr;

// Android NDK入口函数
extern "C" void android_main(struct android_app* app) {
    OryolAndroidAppState = app;
    // Oryol会自动调用MyAndroidApp
}

OryolMain(MyAndroidApp);
```

#### 2. 传感器处理
```cpp
void MyAndroidApp::onSensorEvent(const ASensorEvent* event) {
    switch (event->type) {
        case ASENSOR_TYPE_ACCELEROMETER:
            this->accelerometerData = glm::vec3(
                event->acceleration.x,
                event->acceleration.y,
                event->acceleration.z
            );
            break;
            
        case ASENSOR_TYPE_GAME_ROTATION_VECTOR:
            this->gyroscopeData = glm::vec3(
                event->data[0], event->data[1], event->data[2]
            );
            break;
    }
}
```

#### 3. 触摸输入处理
```cpp
void MyAndroidApp::handleTouchInput() {
    if (Input::TouchpadAttached()) {
        const auto& touchState = Input::TouchpadState();
        for (int i = 0; i < touchState.NumTouches; i++) {
            const auto& touch = touchState.Touches[i];
            
            if (touch.Started) {
                // 根据触摸位置调整旋转
                if (touch.Position.x < 0.5f) {
                    this->rotationAngle -= 10.0f;
                } else {
                    this->rotationAngle += 10.0f;
                }
            }
        }
    }
}
```

## 自定义和扩展

### 添加新的渲染对象

1. 定义顶点数据
2. 创建网格资源
3. 在渲染循环中绘制

### 添加新的传感器

1. 在`onSensorEvent`中添加新的传感器类型处理
2. 在AndroidManifest.xml中声明传感器权限

### 修改着色器

1. 编辑`shaders.glsl`文件
2. 重新构建项目

## 调试技巧

### 1. 查看日志

```bash
# 查看应用日志
adb logcat | grep MyAndroidApp

# 或者使用标签过滤
adb logcat -s Oryol
```

### 2. 性能分析

```bash
# 使用Android Studio Profiler
# 或者使用adb命令
adb shell dumpsys gfxinfo com.example.myandroidapp
```

### 3. 调试构建

```bash
# 构建调试版本
./build_android.sh debug

# 使用gdb调试
adb shell gdbserver :5039 /data/local/tmp/MyAndroidApp
```

## 常见问题

### 1. 编译错误

**问题**: `fatal error: 'android/sensor.h' file not found`
**解决**: 确保ANDROID_NDK环境变量正确设置

**问题**: `undefined reference to 'android_main'`
**解决**: 确保在代码中正确声明了`android_main`函数

### 2. 运行时错误

**问题**: 应用崩溃
**解决**: 检查日志输出，确保所有资源正确初始化

**问题**: 传感器数据不更新
**解决**: 检查AndroidManifest.xml中的传感器权限声明

### 3. 性能问题

**问题**: 帧率低
**解决**: 
- 减少绘制调用
- 优化着色器
- 使用批处理渲染

## 最佳实践

### 1. 内存管理
- 使用Oryol的资源句柄系统
- 及时释放不需要的资源
- 避免频繁的内存分配

### 2. 性能优化
- 使用批处理减少绘制调用
- 优化着色器代码
- 合理使用LOD（细节层次）

### 3. 用户体验
- 正确处理生命周期事件
- 提供适当的触摸反馈
- 支持多种屏幕尺寸

## 许可证

本项目遵循Oryol项目的许可证条款。

## 贡献

欢迎提交问题报告和改进建议！

## 更多资源

- [Oryol官方文档](https://floooh.github.io/oryol/)
- [Android NDK文档](https://developer.android.com/ndk)
- [OpenGL ES文档](https://www.khronos.org/opengles/)