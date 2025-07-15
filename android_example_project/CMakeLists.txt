fips_begin_app(MyAndroidApp android)
    fips_vs_warning_level(3)
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
        
        # 设置应用标签和包名
        set(ANDROID_APP_LABEL "My Android App")
        set(ANDROID_APP_PACKAGE "com.example.myandroidapp")
        
        # 设置屏幕方向
        set(ANDROID_SCREEN_ORIENTATION "landscape")
        
        # 设置主题
        set(ANDROID_APP_THEME "@android:style/Theme.NoTitleBar.Fullscreen")
    endif()
    
    # 添加编译选项
    if (MSVC)
        fips_compile_options(/W4)
    else()
        fips_compile_options(-Wall -Wextra -Wpedantic)
    endif()
    
    # 添加预处理器定义
    fips_compile_definitions(ORYOL_SAMPLES=1)
    
fips_end_app()