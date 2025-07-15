# Oryol 安卓平台渲染流程图

## 1. 初始化流程

### 1.1 模块初始化流程图
```
应用启动
    ↓
Gfx::Setup()
    ↓
┌─────────────────────────────────────┐
│ 创建全局状态 (_state)               │
│ - gfxSetup                          │
│ - displayManager                    │
│ - renderer                          │
│ - resourceContainer                 │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 设置组件指针 (gfxPointers)          │
│ - displayMgr → &displayManager      │
│ - renderer → &renderer              │
│ - resContainer → &resourceContainer │
│ - meshPool → &resourceContainer.meshPool │
│ - shaderPool → &resourceContainer.shaderPool │
│ - texturePool → &resourceContainer.texturePool │
│ - pipelinePool → &resourceContainer.pipelinePool │
│ - renderPassPool → &resourceContainer.renderPassPool │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ displayManager.SetupDisplay()       │
│ ↓                                   │
│ eglDisplayMgr::SetupDisplay()       │
│ ↓                                   │
│ 1. eglGetDisplay()                  │
│ 2. eglInitialize()                  │
│ 3. eglChooseConfig()                │
│ 4. eglBindAPI(EGL_OPENGL_ES_API)    │
│ 5. eglCreateContext()               │
│ 6. eglCreateWindowSurface()         │
│ 7. eglMakeCurrent()                 │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ renderer.setup()                    │
│ ↓                                   │
│ glRenderer::setup()                 │
│ ↓                                   │
│ 1. 创建全局VAO (Core Profile)       │
│ 2. 设置深度模板状态                 │
│ 3. 设置混合状态                     │
│ 4. 设置光栅化状态                   │
│ 5. 初始化状态缓存                   │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ resourceContainer.setup()           │
│ ↓                                   │
│ 1. 初始化资源池                     │
│ 2. 设置工厂指针                     │
│ 3. 注册运行循环回调                 │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 注册系统事件处理                     │
│ Core::PreRunLoop()->Add()           │
└─────────────────────────────────────┘
    ↓
初始化完成
```

### 1.2 安卓窗口集成流程
```
安卓应用启动
    ↓
android_main()
    ↓
┌─────────────────────────────────────┐
│ 创建OryolAndroidAppState            │
│ - 包含ANativeWindow* window         │
│ - 包含其他安卓状态                  │
└─────────────────────────────────────┘
    ↓
eglDisplayMgr::SetupDisplay()
    ↓
┌─────────────────────────────────────┐
│ 获取安卓原生窗口                    │
│ EGLNativeWindowType window =        │
│     OryolAndroidAppState->window    │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 获取EGL配置的视觉ID                 │
│ eglGetConfigAttrib(eglDisplay,      │
│     eglConfig, EGL_NATIVE_VISUAL_ID,│
│     &format)                        │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 获取窗口尺寸                        │
│ int32_t w = ANativeWindow_getWidth(window)  │
│ int32_t h = ANativeWindow_getHeight(window) │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 处理高DPI                           │
│ if (!gfxSetup.HighDPI) {            │
│     w/=2; h/=2;                     │
│ }                                   │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 设置缓冲区几何                      │
│ ANativeWindow_setBuffersGeometry(   │
│     window, w, h, format)           │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 创建EGL窗口表面                     │
│ eglCreateWindowSurface(eglDisplay,  │
│     eglConfig, window, NULL)        │
└─────────────────────────────────────┘
    ↓
窗口集成完成
```

## 2. 渲染循环流程

### 2.1 主渲染循环
```
应用主循环
    ↓
┌─────────────────────────────────────┐
│ 处理系统事件                        │
│ displayManager.ProcessSystemEvents()│
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 开始渲染通道                        │
│ Gfx::BeginPass()                    │
│ ↓                                   │
│ glRenderer::beginPass()             │
│ ↓                                   │
│ 1. 设置渲染目标                     │
│ 2. 应用清除操作                     │
│ 3. 重置状态缓存                     │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 应用绘制状态                        │
│ Gfx::ApplyDrawState()               │
│ ↓                                   │
│ 1. 查找Pipeline资源                 │
│ 2. 查找Mesh资源                     │
│ 3. 查找Texture资源                  │
│ 4. glRenderer::applyDrawState()     │
│    ↓                               │
│    - 绑定着色器程序                 │
│    - 设置顶点属性                   │
│    - 绑定纹理                       │
│    - 应用Uniform块                  │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 执行绘制调用                        │
│ Gfx::Draw()                         │
│ ↓                                   │
│ glRenderer::draw()                  │
│ ↓                                   │
│ 1. 绑定顶点缓冲                     │
│ 2. 绑定索引缓冲                     │
│ 3. glDrawElements/glDrawArrays      │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 结束渲染通道                        │
│ Gfx::EndPass()                      │
│ ↓                                   │
│ glRenderer::endPass()               │
│ ↓                                   │
│ 1. 解绑渲染目标                     │
│ 2. 重置状态                         │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 提交帧                              │
│ Gfx::CommitFrame()                  │
│ ↓                                   │
│ glRenderer::commitFrame()           │
│ ↓                                   │
│ eglDisplayMgr::Present()            │
│ ↓                                   │
│ eglSwapBuffers()                    │
└─────────────────────────────────────┘
    ↓
渲染循环继续
```

### 2.2 资源创建流程
```
Gfx::CreateResource()
    ↓
┌─────────────────────────────────────┐
│ 资源容器创建                        │
│ gfxResourceContainer::createResource()│
│ ↓                                   │
│ 1. 分配资源ID                       │
│ 2. 获取资源池                       │
│ 3. 创建资源对象                     │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 工厂初始化                          │
│ glFactory::initMesh() /             │
│ glFactory::initTexture() /          │
│ glFactory::initShader()             │
│ ↓                                   │
│ 1. 验证设置参数                     │
│ 2. 创建OpenGL对象                   │
│ 3. 上传数据                         │
│ 4. 设置属性                         │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 资源状态设置                        │
│ 设置ResourceState::Valid            │
└─────────────────────────────────────┘
    ↓
资源创建完成
```

## 3. 状态管理流程

### 3.1 状态缓存验证
```
ApplyDrawState()
    ↓
┌─────────────────────────────────────┐
│ 验证Mesh状态                        │
│ validateMeshState()                 │
│ ↓                                   │
│ if (newMesh != curMesh) {           │
│     bindMesh(newMesh);              │
│     curMesh = newMesh;              │
│ }                                   │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 验证Shader状态                      │
│ validateShaderState()               │
│ ↓                                   │
│ if (newShader != curShader) {       │
│     bindShader(newShader);          │
│     curShader = newShader;          │
│ }                                   │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 验证Texture状态                     │
│ validateTextureState()              │
│ ↓                                   │
│ 遍历纹理单元                        │
│ 绑定新纹理到对应单元                │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 验证Pipeline状态                    │
│ validatePipelineState()             │
│ ↓                                   │
│ 设置顶点属性                        │
│ 设置渲染状态                        │
└─────────────────────────────────────┘
    ↓
状态应用完成
```

### 3.2 资源更新流程
```
Gfx::UpdateVertices() / UpdateIndices()
    ↓
┌─────────────────────────────────────┐
│ 查找资源                            │
│ resourceContainer.lookupMesh(id)    │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 检查更新策略                        │
│ if (Usage::Stream) {                │
│     // 双缓冲更新                   │
│     slot = (activeSlot + 1) % 2;    │
│     glBindBuffer(GL_ARRAY_BUFFER,   │
│         glBuffers[slot]);           │
│     glBufferData(GL_ARRAY_BUFFER,   │
│         size, data, GL_STREAM_DRAW);│
│     activeSlot = slot;              │
│ } else {                            │
│     // 直接更新                     │
│     glBindBuffer(GL_ARRAY_BUFFER,   │
│         glBuffers[0]);              │
│     glBufferSubData(GL_ARRAY_BUFFER,│
│         offset, size, data);        │
│ }                                   │
└─────────────────────────────────────┘
    ↓
更新完成
```

## 4. 错误处理流程

### 4.1 OpenGL错误检测
```
OpenGL API调用
    ↓
┌─────────────────────────────────────┐
│ ORYOL_GL_CHECK_ERROR()              │
│ ↓                                   │
│ GLenum err = glGetError();          │
│ if (err != GL_NO_ERROR) {           │
│     o_error("OpenGL error: 0x%04X", │
│         err);                       │
│ }                                   │
└─────────────────────────────────────┘
    ↓
错误处理完成
```

### 4.2 资源验证流程
```
#if ORYOL_DEBUG
    ↓
┌─────────────────────────────────────┐
│ 验证设置参数                        │
│ validateTextureSetup()              │
│ validateMeshSetup()                 │
│ validatePipelineSetup()             │
│ ↓                                   │
│ 1. 检查参数有效性                   │
│ 2. 检查资源限制                     │
│ 3. 检查格式支持                     │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 验证资源绑定                        │
│ validateMeshes()                    │
│ validateTextures()                  │
│ ↓                                   │
│ 1. 检查资源存在性                   │
│ 2. 检查资源兼容性                   │
│ 3. 检查绑定状态                     │
└─────────────────────────────────────┘
    ↓
验证完成
#endif
```

## 5. 性能优化流程

### 5.1 垃圾回收流程
```
Gfx::CommitFrame()
    ↓
┌─────────────────────────────────────┐
│ 垃圾回收                            │
│ gfxResourceContainer::GarbageCollect()│
│ ↓                                   │
│ for (const Id& id : destroyQueue) { │
│     destroyResource(id);            │
│ }                                   │
│ destroyQueue.Clear();               │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 更新待处理加载器                    │
│ for (auto& loader : pendingLoaders) {│
│     loader->Load();                 │
│     if (loader->State == Ready) {   │
│         pendingLoaders.Remove(loader);│
│     }                               │
│ }                                   │
└─────────────────────────────────────┘
    ↓
垃圾回收完成
```

### 5.2 异步加载流程
```
Gfx::LoadResource()
    ↓
┌─────────────────────────────────────┐
│ 创建占位符资源                      │
│ prepareAsync(loader->Setup())       │
│ ↓                                   │
│ 1. 分配资源ID                       │
│ 2. 设置初始状态                     │
│ 3. 返回资源ID                       │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 添加到待处理列表                    │
│ pendingLoaders.Add(loader)          │
└─────────────────────────────────────┘
    ↓
┌─────────────────────────────────────┐
│ 异步加载处理                        │
│ 在GarbageCollect()中处理            │
│ ↓                                   │
│ loader->Load()                      │
│ ↓                                   │
│ 1. 加载数据                         │
│ 2. 初始化资源                       │
│ 3. 设置完成状态                     │
└─────────────────────────────────────┘
    ↓
异步加载完成
```

这些流程图展示了Oryol在安卓平台上的完整渲染流程，从初始化到渲染循环，再到资源管理和性能优化，形成了一个完整的渲染系统。