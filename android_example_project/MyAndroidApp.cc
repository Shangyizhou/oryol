//------------------------------------------------------------------------------
//  MyAndroidApp.cc
//  Android示例应用
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Core/Main.h"
#include "Core/Log.h"
#include "Core/Time/Clock.h"
#include "Gfx/Gfx.h"
#include "Input/Input.h"
#include "Core/private/android/androidBridge.h"
#include "android/sensor.h"

using namespace Oryol;

class MyAndroidApp : public App {
public:
    AppState::Code OnInit();
    AppState::Code OnRunning();
    AppState::Code OnCleanup();
    
    // Android生命周期回调
    void OnSuspend() override;
    void OnResume() override;
    void OnWindowResize(int width, int height) override;

private:
    DrawState drawState;
    Id shader;
    Id mesh;
    Id texture;
    
    // 变换矩阵
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    glm::mat4 mvpMatrix;
    
    // 动画相关
    float rotationAngle;
    TimePoint lastFrameTime;
    
    // Android传感器数据
    glm::vec3 accelerometerData;
    glm::vec3 gyroscopeData;
    
    // 处理传感器事件
    void onSensorEvent(const ASensorEvent* event);
    
    // 更新变换矩阵
    void updateTransform();
    
    // 渲染场景
    void renderScene();
    
    // 处理触摸输入
    void handleTouchInput();
};

// 全局Android应用状态
android_app* OryolAndroidAppState = nullptr;

// Android NDK入口函数
extern "C" void android_main(struct android_app* app) {
    OryolAndroidAppState = app;
    // Oryol会自动调用MyAndroidApp
}

OryolMain(MyAndroidApp);

//------------------------------------------------------------------------------
AppState::Code
MyAndroidApp::OnInit() {
    Log::Info("MyAndroidApp::OnInit() - 应用初始化开始\n");
    
    // 设置图形系统
    auto gfxSetup = GfxSetup::Window(800, 600, "My Android App");
    gfxSetup.HighDPI = true;  // 支持高DPI显示
    gfxSetup.SampleCount = 4; // 4x MSAA抗锯齿
    Gfx::Setup(gfxSetup);
    
    // 初始化变换矩阵
    this->viewMatrix = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f),  // 相机位置
        glm::vec3(0.0f, 0.0f, 0.0f),  // 目标点
        glm::vec3(0.0f, 1.0f, 0.0f)   // 上方向
    );
    
    this->projMatrix = glm::perspective(
        glm::radians(45.0f),  // 视野角度
        800.0f / 600.0f,      // 宽高比
        0.1f,                  // 近平面
        100.0f                 // 远平面
    );
    
    this->modelMatrix = glm::mat4(1.0f);
    this->rotationAngle = 0.0f;
    this->lastFrameTime = Clock::Now();
    
    // 创建顶点数据 - 一个彩色立方体
    const float vertices[] = {
        // 前面 (红色)
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f,
        
        // 后面 (绿色)
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 1.0f,
        
        // 左面 (蓝色)
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 1.0f,
        
        // 右面 (黄色)
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 1.0f,
        
        // 上面 (紫色)
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f, 1.0f,
        
        // 下面 (青色)
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f, 1.0f,
    };
    
    // 索引数据
    const uint16_t indices[] = {
        0,  1,  2,    0,  2,  3,   // 前面
        4,  6,  5,    4,  7,  6,   // 后面
        8,  10, 9,    8,  11, 10,  // 左面
        12, 13, 14,   12, 14, 15,  // 右面
        16, 18, 17,   16, 19, 18,  // 上面
        20, 21, 22,   20, 22, 23   // 下面
    };
    
    // 创建网格
    auto meshSetup = MeshSetup::FromData();
    meshSetup.NumVertices = 24;
    meshSetup.NumIndices = 36;
    meshSetup.Layout = {
        { VertexAttr::Position, VertexFormat::Float3 },
        { VertexAttr::Color0, VertexFormat::Float4 }
    };
    meshSetup.AddPrimitiveGroup({0, 36});
    this->mesh = Gfx::CreateResource(meshSetup, vertices, sizeof(vertices), indices, sizeof(indices));
    
    // 创建着色器
    this->shader = Gfx::CreateResource(Shader::Setup());
    
    // 创建渲染管线
    auto ps = PipelineSetup::FromLayoutAndShader(meshSetup.Layout, this->shader);
    ps.DepthStencilState.DepthWriteEnabled = true;
    ps.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    ps.RasterizerState.CullFaceEnabled = true;
    ps.RasterizerState.CullFace = Face::Back;
    this->drawState.Pipeline = Gfx::CreateResource(ps);
    this->drawState.Mesh[0] = this->mesh;
    
    // Android特定：设置传感器回调
    auto bridge = _priv::androidBridge::ptr();
    if (bridge && bridge->isValid()) {
        bridge->setSensorEventCallback([this](const ASensorEvent* event) {
            this->onSensorEvent(event);
        });
        Log::Info("Android传感器回调已设置\n");
    } else {
        Log::Warn("Android桥接不可用\n");
    }
    
    Log::Info("MyAndroidApp::OnInit() - 应用初始化完成\n");
    return App::OnInit();
}

//------------------------------------------------------------------------------
AppState::Code
MyAndroidApp::OnRunning() {
    // 计算帧时间
    auto now = Clock::Now();
    auto deltaTime = now - this->lastFrameTime;
    float dt = deltaTime.AsSeconds();
    this->lastFrameTime = now;
    
    // 更新动画
    this->rotationAngle += dt * 45.0f; // 每秒旋转45度
    
    // 根据传感器数据调整旋转
    if (glm::length(this->accelerometerData) > 0.1f) {
        this->modelMatrix = glm::rotate(this->modelMatrix, dt * 2.0f, 
                                       glm::normalize(this->accelerometerData));
    }
    
    // 更新变换矩阵
    this->updateTransform();
    
    // 处理触摸输入
    this->handleTouchInput();
    
    // 渲染场景
    this->renderScene();
    
    // 检查是否需要退出
    return Gfx::QuitRequested() ? AppState::Cleanup : AppState::Running;
}

//------------------------------------------------------------------------------
AppState::Code
MyAndroidApp::OnCleanup() {
    Log::Info("MyAndroidApp::OnCleanup() - 应用清理\n");
    Gfx::Discard();
    return App::OnCleanup();
}

//------------------------------------------------------------------------------
void
MyAndroidApp::OnSuspend() {
    Log::Info("MyAndroidApp::OnSuspend() - 应用暂停\n");
    // 保存状态、暂停音频等
}

//------------------------------------------------------------------------------
void
MyAndroidApp::OnResume() {
    Log::Info("MyAndroidApp::OnResume() - 应用恢复\n");
    // 恢复状态、重新初始化等
}

//------------------------------------------------------------------------------
void
MyAndroidApp::OnWindowResize(int width, int height) {
    Log::Info("MyAndroidApp::OnWindowResize() - 窗口大小改变: %d x %d\n", width, height);
    
    // 更新投影矩阵
    float aspectRatio = (float)width / (float)height;
    this->projMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
}

//------------------------------------------------------------------------------
void
MyAndroidApp::onSensorEvent(const ASensorEvent* event) {
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
                event->data[0],
                event->data[1],
                event->data[2]
            );
            break;
    }
}

//------------------------------------------------------------------------------
void
MyAndroidApp::updateTransform() {
    // 重置模型矩阵
    this->modelMatrix = glm::mat4(1.0f);
    
    // 应用旋转
    this->modelMatrix = glm::rotate(this->modelMatrix, 
                                   glm::radians(this->rotationAngle), 
                                   glm::vec3(0.0f, 1.0f, 0.0f));
    
    // 计算MVP矩阵
    this->mvpMatrix = this->projMatrix * this->viewMatrix * this->modelMatrix;
}

//------------------------------------------------------------------------------
void
MyAndroidApp::renderScene() {
    // 开始渲染通道
    Gfx::BeginPass();
    
    // 应用绘制状态
    Gfx::ApplyDrawState(this->drawState);
    
    // 设置MVP矩阵
    Gfx::ApplyUniformBlock(this->mvpMatrix);
    
    // 绘制立方体
    Gfx::Draw();
    
    // 结束渲染通道
    Gfx::EndPass();
    
    // 提交帧
    Gfx::CommitFrame();
}

//------------------------------------------------------------------------------
void
MyAndroidApp::handleTouchInput() {
    if (Input::TouchpadAttached()) {
        const auto& touchState = Input::TouchpadState();
        for (int i = 0; i < touchState.NumTouches; i++) {
            const auto& touch = touchState.Touches[i];
            
            if (touch.Started) {
                Log::Info("触摸开始: 位置(%f, %f)\n", touch.Position.x, touch.Position.y);
                
                // 根据触摸位置调整旋转速度
                if (touch.Position.x < 0.5f) {
                    this->rotationAngle -= 10.0f; // 向左旋转
                } else {
                    this->rotationAngle += 10.0f; // 向右旋转
                }
            }
            
            if (touch.Moved) {
                // 根据触摸移动调整相机位置
                float deltaX = touch.Movement.x * 0.01f;
                float deltaY = touch.Movement.y * 0.01f;
                
                // 这里可以添加相机控制逻辑
            }
            
            if (touch.Ended) {
                Log::Info("触摸结束: 位置(%f, %f)\n", touch.Position.x, touch.Position.y);
            }
        }
    }
}