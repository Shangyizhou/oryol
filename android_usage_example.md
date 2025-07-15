# Android使用Oryol库完整指南

## 概述

Oryol是一个轻量级的C++11 3D编程框架，专门为移动和Web平台设计。在Android上使用Oryol需要了解其特殊的架构设计和平台抽象层。

## 1. Android架构特点

### 1.1 平台抽象层
Oryol在Android上使用以下关键组件：
- **Android Native Activity**: 通过`android_native_app_glue`与Android系统交互
- **EGL**: 用于OpenGL ES上下文管理
- **OpenGL ES 3.0**: 图形渲染后端
- **Android传感器**: 加速度计、陀螺仪等

### 1.2 核心桥接机制
```cpp
// Android桥接类，连接Oryol应用和Android系统
class androidBridge {
    // 处理Android生命周期事件
    static void onAppCmd(struct android_app* app, int32_t cmd);
    
    // 传感器事件处理
    void setSensorEventCallback(std::function<void(const ASensorEvent*)> cb);
    
    // 主循环控制
    bool onFrame();
};
```

## 2. 基本Android应用结构

### 2.1 主应用类
```cpp
#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"
#include "Input/Input.h"

using namespace Oryol;

class MyAndroidApp : public App {
public:
    AppState::Code OnInit();
    AppState::Code OnRunning();
    AppState::Code OnCleanup();
    
private:
    DrawState drawState;
    Id shader;
    Id mesh;
};

// 关键：使用OryolMain宏，这会自动处理Android入口点
OryolMain(MyAndroidApp);
```

### 2.2 Android入口点
```cpp
// 在应用主文件中声明全局变量
android_app* OryolAndroidAppState = nullptr;

// Android NDK入口函数
void android_main(struct android_app* app) {
    OryolAndroidAppState = app;
    // Oryol会自动调用你的应用类
}
```

## 3. 完整的Android示例

### 3.1 基础三角形渲染示例
```cpp
// MyAndroidApp.cc
#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"
#include "Input/Input.h"

using namespace Oryol;

class MyAndroidApp : public App {
public:
    AppState::Code OnInit();
    AppState::Code OnRunning();
    AppState::Code OnCleanup();

private:
    DrawState drawState;
    Id shader;
    Id mesh;
    
    // Android特定：处理传感器事件
    void onSensorEvent(const ASensorEvent* event);
};

OryolMain(MyAndroidApp);

//------------------------------------------------------------------------------
AppState::Code
MyAndroidApp::OnInit() {
    // 设置图形系统 - Android会自动使用EGL
    Gfx::Setup(GfxSetup::Window(800, 600, "My Android App"));
    
    // 创建顶点数据
    const float vertices[] = {
        // 位置 (x, y, z)     // 颜色 (r, g, b, a)
         0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f, 1.0f,
    };
    
    // 创建网格
    auto meshSetup = MeshSetup::FromData();
    meshSetup.NumVertices = 3;
    meshSetup.Layout = {
        { VertexAttr::Position, VertexFormat::Float3 },
        { VertexAttr::Color0, VertexFormat::Float4 }
    };
    meshSetup.AddPrimitiveGroup({0, 3});
    this->mesh = Gfx::CreateResource(meshSetup, vertices, sizeof(vertices));
    
    // 创建着色器
    this->shader = Gfx::CreateResource(Shader::Setup());
    
    // 创建渲染管线
    auto ps = PipelineSetup::FromLayoutAndShader(meshSetup.Layout, this->shader);
    this->drawState.Pipeline = Gfx::CreateResource(ps);
    this->drawState.Mesh[0] = this->mesh;
    
    // Android特定：设置传感器回调
    auto bridge = _priv::androidBridge::ptr();
    if (bridge && bridge->isValid()) {
        bridge->setSensorEventCallback([this](const ASensorEvent* event) {
            this->onSensorEvent(event);
        });
    }
    
    return App::OnInit();
}

//------------------------------------------------------------------------------
AppState::Code
MyAndroidApp::OnRunning() {
    // 开始渲染通道
    Gfx::BeginPass();
    
    // 应用绘制状态
    Gfx::ApplyDrawState(this->drawState);
    
    // 绘制三角形
    Gfx::Draw();
    
    // 结束渲染通道
    Gfx::EndPass();
    
    // 提交帧
    Gfx::CommitFrame();
    
    // 检查是否需要退出
    return Gfx::QuitRequested() ? AppState::Cleanup : AppState::Running;
}

//------------------------------------------------------------------------------
AppState::Code
MyAndroidApp::OnCleanup() {
    Gfx::Discard();
    return App::OnCleanup();
}

//------------------------------------------------------------------------------
void
MyAndroidApp::onSensorEvent(const ASensorEvent* event) {
    switch (event->type) {
        case ASENSOR_TYPE_ACCELEROMETER:
            // 处理加速度计数据
            Log::Info("Accel: x=%f, y=%f, z=%f\n", 
                     event->acceleration.x, 
                     event->acceleration.y, 
                     event->acceleration.z);
            break;
            
        case ASENSOR_TYPE_GAME_ROTATION_VECTOR:
            // 处理陀螺仪数据
            Log::Info("Gyro: x=%f, y=%f, z=%f\n", 
                     event->data[0], 
                     event->data[1], 
                     event->data[2]);
            break;
    }
}
```

### 3.2 着色器文件
```glsl
// shaders.glsl
@vs vs
@uniform mat4 mvp ModelViewProjection
@attribute vec4 position Position
@attribute vec4 color0 Color0
@varying vec4 color
void main() {
    gl_Position = mvp * position;
    color = color0;
}
@end

@fs fs
@varying vec4 color
void main() {
    gl_FragColor = color;
}
@end

@program Triangle vs fs
```

## 4. Android特定功能

### 4.1 触摸输入处理
```cpp
// 在OnRunning()中添加触摸处理
AppState::Code MyAndroidApp::OnRunning() {
    // 处理触摸输入
    if (Input::TouchpadAttached()) {
        const auto& touchState = Input::TouchpadState();
        for (int i = 0; i < touchState.NumTouches; i++) {
            const auto& touch = touchState.Touches[i];
            if (touch.Started) {
                Log::Info("Touch started at: %f, %f\n", touch.Position.x, touch.Position.y);
            }
            if (touch.Moved) {
                Log::Info("Touch moved to: %f, %f\n", touch.Position.x, touch.Position.y);
            }
            if (touch.Ended) {
                Log::Info("Touch ended at: %f, %f\n", touch.Position.x, touch.Position.y);
            }
        }
    }
    
    // ... 其他渲染代码
}
```

### 4.2 生命周期管理
```cpp
// Android应用生命周期会自动处理，但你可以监听状态变化
class MyAndroidApp : public App {
public:
    // 应用暂停时调用
    void OnSuspend() override {
        Log::Info("App suspended\n");
        // 保存状态、暂停音频等
    }
    
    // 应用恢复时调用
    void OnResume() override {
        Log::Info("App resumed\n");
        // 恢复状态、重新初始化等
    }
    
    // 窗口大小改变时调用
    void OnWindowResize(int width, int height) override {
        Log::Info("Window resized to: %d x %d\n", width, height);
        // 更新视口、重新计算投影矩阵等
    }
};
```

## 5. 构建配置

### 5.1 CMakeLists.txt
```cmake
# 应用CMakeLists.txt
fips_begin_app(MyAndroidApp android)
    fips_files(MyAndroidApp.cc)
    oryol_shader(shaders.glsl)
    fips_deps(Gfx Input)
    
    # Android特定配置
    if (ANDROID)
        # 设置Android API级别
        set(ANDROID_NATIVE_API_LEVEL 21)
        
        # 添加Android权限
        set(ANDROID_PERMISSIONS 
            "android.permission.INTERNET"
            "android.permission.ACCESS_NETWORK_STATE"
        )
        
        # 设置应用权限
        set(ANDROID_APP_PERMISSIONS ${ANDROID_PERMISSIONS})
    endif()
fips_end_app()
```

### 5.2 AndroidManifest.xml
```xml
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="com.example.myandroidapp"
    android:versionCode="1"
    android:versionName="1.0">

    <uses-sdk android:minSdkVersion="21" android:targetSdkVersion="33" />
    
    <!-- 权限声明 -->
    <uses-permission android:name="android.permission.INTERNET" />
    <uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" />
    
    <!-- 硬件特性 -->
    <uses-feature android:glEsVersion="0x00030000" android:required="true" />
    <uses-feature android:name="android.hardware.touchscreen" android:required="false" />
    <uses-feature android:name="android.hardware.sensor.accelerometer" android:required="false" />
    
    <application
        android:label="My Android App"
        android:icon="@mipmap/ic_launcher"
        android:theme="@android:style/Theme.NoTitleBar.Fullscreen">
        
        <activity
            android:name="android.app.NativeActivity"
            android:label="My Android App"
            android:configChanges="orientation|keyboardHidden|screenSize"
            android:screenOrientation="landscape"
            android:exported="true">
            
            <meta-data android:name="android.app.lib_name" android:value="MyAndroidApp" />
            <meta-data android:name="android.app.func_name" android:value="android_main" />
            
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
```

## 6. 性能优化技巧

### 6.1 内存管理
```cpp
// 使用Oryol的自定义容器而不是STL
Array<float> vertices;  // 替代 std::vector
ArrayMap<String, Id> resources;  // 替代 std::map

// 预分配内存
vertices.Reserve(1000);
```

### 6.2 渲染优化
```cpp
// 使用批处理减少绘制调用
class MyAndroidApp : public App {
private:
    // 批量绘制多个对象
    void renderBatch() {
        Gfx::BeginPass();
        Gfx::ApplyDrawState(this->drawState);
        
        // 一次绘制多个实例
        for (int i = 0; i < numObjects; i++) {
            // 更新变换矩阵
            updateTransform(i);
            Gfx::Draw();
        }
        
        Gfx::EndPass();
        Gfx::CommitFrame();
    }
};
```

### 6.3 异步资源加载
```cpp
// 使用Oryol的异步IO系统
void MyAndroidApp::loadResources() {
    // 异步加载纹理
    Id texture = Gfx::LoadResource(TextureLoader::Create(TextureSetup::FromFile("texture.png")));
    
    // 异步加载着色器
    Id shader = Gfx::LoadResource(ShaderLoader::Create(ShaderSetup::FromFile("shaders.glsl")));
    
    // 检查加载状态
    if (Gfx::QueryResourceInfo(texture).State == ResourceState::Valid) {
        // 纹理加载完成
        this->texture = texture;
    }
}
```

## 7. 调试和日志

### 7.1 日志系统
```cpp
// 使用Oryol的日志系统
Log::Info("Application started\n");
Log::Warn("Warning message\n");
Log::Error("Error message\n");

// 条件日志
o_assert_dbg(condition);  // 调试断言
o_assert(condition);      // 发布断言
```

### 7.2 性能分析
```cpp
// 使用Oryol的跟踪系统
#include "Core/Trace.h"

void MyAndroidApp::OnRunning() {
    // 开始跟踪
    Trace::BeginFrame();
    
    // 跟踪渲染时间
    Trace::Begin("Render");
    Gfx::BeginPass();
    Gfx::ApplyDrawState(this->drawState);
    Gfx::Draw();
    Gfx::EndPass();
    Trace::End();
    
    Gfx::CommitFrame();
    
    // 结束跟踪
    Trace::EndFrame();
}
```

## 8. 常见问题和解决方案

### 8.1 编译问题
```bash
# 确保Android NDK路径正确
export ANDROID_NDK=/path/to/android-ndk

# 使用fips构建
./fips build android

# 如果遇到链接错误，检查库依赖
./fips build android --verbose
```

### 8.2 运行时问题
```cpp
// 检查Android桥接状态
auto bridge = _priv::androidBridge::ptr();
if (bridge && bridge->isValid()) {
    // 桥接正常工作
} else {
    Log::Error("Android bridge not available\n");
}

// 检查OpenGL ES版本
if (Gfx::QueryFeature(GfxFeature::TextureFloat)) {
    // 支持浮点纹理
} else {
    // 降级到其他格式
}
```

### 8.3 性能问题
```cpp
// 监控帧率
class MyAndroidApp : public App {
private:
    TimePoint lastFrameTime;
    int frameCount = 0;
    
    void updateFPS() {
        frameCount++;
        auto now = Clock::Now();
        auto elapsed = now - lastFrameTime;
        
        if (elapsed.AsSeconds() >= 1.0) {
            float fps = frameCount / elapsed.AsSeconds();
            Log::Info("FPS: %.2f\n", fps);
            frameCount = 0;
            lastFrameTime = now;
        }
    }
};
```

## 9. 最佳实践

### 9.1 代码组织
- 将Android特定代码与通用代码分离
- 使用条件编译处理平台差异
- 遵循Oryol的模块化设计原则

### 9.2 资源管理
- 使用Oryol的资源句柄系统
- 及时释放不需要的资源
- 使用异步加载避免阻塞主线程

### 9.3 错误处理
- 使用Oryol的断言系统
- 检查资源加载状态
- 提供降级方案

这个指南涵盖了在Android上使用Oryol库的主要方面。记住，Oryol的设计理念是轻量化和跨平台，所以在Android上使用时要注意性能优化和平台特定的功能。