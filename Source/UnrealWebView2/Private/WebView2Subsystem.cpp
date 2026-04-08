#include "WebView2Subsystem.h"
#include "WebView2CompositionHost.h"
#include "WebView2Log.h"
#include "WebView2Manager.h"
#include "Misc/CommandLine.h"

bool FWebviewWindowsMessageHandler::ProcessMessage(HWND Handler, uint32 Message, WPARAM WParam, LPARAM LParam,
                                                   int32& OutResult)
{
    //WebView自己会捕获键盘，这里不需要给他同步键盘事件
    const UWebView2Subsystem* Subsystem = UWebView2Subsystem::GetSubsystem();

    if (!Subsystem->bIsMouseOverPositionArea)
    {
        return false;
    }

    if (Message == WM_KEYDOWN || Message == WM_KEYUP || Message == WM_SYSKEYDOWN || Message == WM_SYSKEYUP)
    {
        return false;
    }

    return FWebView2Manager::GetInstance()->HandleWindowMessage(Handler, Message, WParam, LParam);
}

static FWebviewWindowsMessageHandler MessageHandler;

/** 标识是否已经注册过 */
bool GMessageHandlerRegistered = false;

void UWebView2Subsystem::Deinitialize()
{
    if (FSlateApplication::IsInitialized())
    {
        const TSharedPtr<FWindowsApplication> WindowsApplication = StaticCastSharedPtr<FWindowsApplication>(
            FSlateApplication::Get().GetPlatformApplication());
        if (WindowsApplication.IsValid())
        {
            WindowsApplication->RemoveMessageHandler(MessageHandler);
        }
    }

    // 重置状态
    GMessageHandlerRegistered = false;

    FWebView2Manager::GetInstance()->Shutdown();


    Super::Deinitialize();
}

TStatId UWebView2Subsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UWebView2Subsystem, STATGROUP_Tickables);
}

UWebView2Subsystem* UWebView2Subsystem::GetSubsystem()
{
    if (GEngine)
    {
        return GEngine->GetEngineSubsystem<UWebView2Subsystem>();
    }
    else
    {
        UE_LOG(LogWebView2, Error, TEXT("UWebView2Subsystem does not exist!"));
        return nullptr;
    }
}

void UWebView2Subsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    OnStartGameInstance = FWorldDelegates::OnStartGameInstance.AddUObject(
        this, &UWebView2Subsystem::OnGameInstanceStarted);

    FWebView2Manager::GetInstance()->Init();
// #if WITH_EDITOR
//     FEditorDelegates::EndPIE.AddUObject(this, &UWebView2Subsystem::OnEndPIE);
// #endif

    // 初始化鼠标悬停状态为 true
    bIsMouseOverPositionArea = true;

    Super::Initialize(Collection);

    // 命令行是否有离屏渲染参数
    if (FParse::Param(FCommandLine::Get(), TEXT("RenderOffScreen"))) {
        UE_LOG(LogWebView2, Warning, TEXT("WebView2: CLI with RenderOffScreen, skipping registration"));
        return;
    }
    
    // 重置状态标志并尝试注册
    GMessageHandlerRegistered = false;
    RegisterMessageHandler();
}

bool UWebView2Subsystem::IsTickable() const
{
    return true;
}

void UWebView2Subsystem::OnEndPIE(bool bIsSimulating)
{
    if (const TSharedPtr<SWindow> Window = GEngine->GameViewport->GetWindow().ToSharedRef())
    {
        const HWND Handle2Window = static_cast<HWND>(Window->GetNativeWindow()->GetOSWindowHandle());
        if (!Handle2Window)
        {
            return;
        }
        FWebView2Manager::GetInstance()->Closed(Handle2Window);
    }
}

// ReSharper disable once CppMemberFunctionCanBeMadeStatic
// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UWebView2Subsystem::OnGameInstanceStarted(UGameInstance* GameInstance)
{
    if (GameInstance)
    {
        // 在每次 GameInstance 启动时重置变量
        bIsMouseOverPositionArea = true;
    }
    else
    {
        UE_LOG(LogWebView2, Warning, TEXT("OnGameInstanceStarted called with null GameInstance?"));
    }
}

void UWebView2Subsystem::RegisterMessageHandler()
{
    // 如果已经注册过，直接返回
    if (GMessageHandlerRegistered)
    {
        return;
    }
    
    // 检查是否有有效的游戏视口和窗口
    if (!GEngine || !GEngine->GameViewport)
    {
        UE_LOG(LogWebView2, Warning, TEXT("WebView2: No valid GameViewport, skipping registration"));
        return;
    }

    // 检查窗口是否有效
    const TSharedPtr<SWindow> Window = GEngine->GameViewport->GetWindow();
    if (!Window.IsValid())
    {
        UE_LOG(LogWebView2, Warning, TEXT("WebView2: No valid window (offscreen rendering mode), skipping registration"));
        return;
    }

    // 检查 Slate 应用
    if (!FSlateApplication::IsInitialized())
    {
        UE_LOG(LogWebView2, Warning, TEXT("WebView2: SlateApplication not initialized, skipping registration"));
        return;
    }

    const TSharedPtr<FWindowsApplication> WindowsApplication = StaticCastSharedPtr<FWindowsApplication>(
        FSlateApplication::Get().GetPlatformApplication()
    );
    if (!WindowsApplication.IsValid())
    {
        UE_LOG(LogWebView2, Warning, TEXT("WebView2: WindowsApplication not valid, skipping registration"));
        return;
    }
    
    // 所有条件满足，注册消息处理器
    WindowsApplication->AddMessageHandler(MessageHandler);
    GMessageHandlerRegistered = true;
    
    UE_LOG(LogWebView2, Log, TEXT("WebView2 MessageHandler registered successfully"));
}

void UWebView2Subsystem::Tick(float DeltaTime)
{   
    // 命令行是否有离屏渲染参数
    if (FParse::Param(FCommandLine::Get(), TEXT("RenderOffScreen"))) {
        // UE_LOG(LogWebView2, Warning, TEXT("WebView2: CLI with RenderOffScreen, skipping registration"));
        return;
    }
    
    // 如果未注册且不在等待状态，直接返回（可能是离屏渲染或窗口无效）
    if (!GMessageHandlerRegistered)
    {
        RegisterMessageHandler();
        return;
    }
    
    const TSharedPtr<SWindow> Window = GEngine->GameViewport->GetWindow();
    const HWND Handle2Window = static_cast<HWND>(Window->GetNativeWindow()->GetOSWindowHandle());
    if (!Handle2Window)
    {
        return;
    }
    FWebView2Manager* WebView2Manager = FWebView2Manager::GetInstance();
    const TSharedPtr<FWebView2CompositionHost> CompositionHost = WebView2Manager->GetMessageProcess(Handle2Window);
    if (!CompositionHost)
    {
        return;
    }
    
    for (const TPair<FString, TSharedRef<FWebView2Window>>& Element : CompositionHost->WebViewWindowMap)
    {
        bIsMouseOverPositionArea = Element.Value->bIsMouseOverPositionArea;
    }

    if (!bIsMouseOverPositionArea)
    {
        // UE_LOG(LogWebView2, Warning, TEXT("Subsystem Tick, Is Mouse Over Position Area: %hhd"), bIsMouseOverPositionArea);
        // const TSharedPtr<SWindow> CurrentWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
        // if (CurrentWindow.IsValid())
        // {
        //     void* NativeWindow = CurrentWindow->GetNativeWindow()->GetOSWindowHandle();
        //     ::SetFocus(static_cast<HWND>(NativeWindow));
        // }
    }
}
