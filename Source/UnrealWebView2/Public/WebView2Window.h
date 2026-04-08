#pragma once
#pragma warning(disable : 4191)

#include "CoreMinimal.h"

#include "Windows/WindowsHWrapper.h"
// ReSharper disable once CppUnusedIncludeDirective
// 注意：WebView2 接口依赖此头文件，禁止删除！
#include "Windows/AllowWindowsPlatformTypes.h"
// ReSharper disable once CppUnusedIncludeDirective
// 注意：WebView2 接口依赖此头文件，禁止删除！
#include "Windows/AllowWindowsPlatformAtomics.h"

#include "Stdafx.h"
#include <dcomp.h>

#include <atomic>
#include <winnt.h>
#include <com.h>
#include <winrt/Windows.UI.Composition.h>

// ReSharper disable once CppUnusedIncludeDirective
// 注意：WebView2 接口依赖此头文件，禁止删除！
#include "Windows/HideWindowsPlatformAtomics.h"
// ReSharper disable once CppUnusedIncludeDirective
// 注意：WebView2 接口依赖此头文件，禁止删除！
#include "Windows/HideWindowsPlatformTypes.h"

#include "WebView2DevToolsProtocol.h"
#include "WebView2Window.generated.h"

struct FCoreWebView2Settings;

class FWebView2CompositionHost;

namespace WinrtComp = winrt::Windows::UI::Composition;

USTRUCT(Blueprintable)
struct UNREALWEBVIEW2_API FWebView2DownloadInfo
{
    GENERATED_BODY()
    /**下载 URL 地址*/
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "WebView2|Info")
    FString URL;
    /**m下载 MIME 类型*/
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "WebView2|Info")
    FString MimeType;
    /** 下载内容描述 */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "WebView2|Info")
    FString ContentDisposition;

    FWebView2DownloadInfo()
    {
    }
};

UENUM(BlueprintType)
enum class EWebView2DownloadState:uint8
{
    Progress UMETA(DisplayName = "正在下载"),
    Completed UMETA(DisplayName = "下载完成"),
    Interrupted UMETA(DisplayName = "下载中断"),
};

DECLARE_DELEGATE_OneParam(FOnCursorChanged, const EMouseCursor::Type&)
DECLARE_DELEGATE_OneParam(FOnMessageReceived, const FString&)
DECLARE_DELEGATE_OneParam(FOnNavigationCompleted, const bool&)
DECLARE_DELEGATE_OneParam(FOnNavigationStarting, const FString&)
DECLARE_DELEGATE_OneParam(FOnNewWindowRequested, const FString&)

enum class EWebView2DocumentState
{
    Completed,
    Error,
    Loading,
    NoDocument
};

class UNREALWEBVIEW2_API FWebView2Window final : public TSharedFromThis<FWebView2Window>
{
public:
    DECLARE_DELEGATE(FOnWebView2Closed)
    DECLARE_DELEGATE_OneParam(FOnCanGoBack, const bool&)
    DECLARE_DELEGATE_OneParam(FOnCanGoForward, const bool&)
    DECLARE_DELEGATE_OneParam(FOnDocumentTitleChanged, const FString&)
    DECLARE_DELEGATE_OneParam(FOnEstimatedDownloadTime, const FString&)
    DECLARE_DELEGATE_OneParam(FOnSourceChanged, const FString&)
    DECLARE_DELEGATE_TwoParams(FOnDownloadStateChanged, const EWebView2DownloadState&, const FString&)
    DECLARE_DELEGATE_TwoParams(FOnDownloadProgress, const int64&, const int64&)
    
    FWebView2Window(HWND InHandler, const FGuid UniqueID, const FString& InInitialURL, FColor InBackgroundColor,
                    bool bInDevTools);
    ~FWebView2Window();

    void CloseWindow();

    HRESULT CreateControllerWithOptions();

    static void DeleteAllComponents();

    //执行前端脚本
    void ExecuteScript(const FString& Script, TFunction<void(const FString ResultString)> Callback = nullptr) const;

    /** 获取 WebView2 大小 */
    RECT GetBounds() const;
    /** 获取当前文档的加载状态。 */
    EWebView2DocumentState GetDocumentLoadingState() const;
    int32 GetLayerID() const;
    HWND GetMainWindow() const;
    FGuid GetUniqueID() const;
    /** 获取可见性 */
    ESlateVisibility GetVisible() const;
    ICoreWebView2* GetWebView() const;
    TSharedPtr<FWebView2CompositionHost> GetCompositionHost() const;
    ICoreWebView2Controller* GetWebViewController() const;
    ICoreWebView2Environment* GetWebViewEnvironment() const;

    /** 后退 */
    void GoBack() const;
    /** 前进 */
    void GoForward() const;

    void InitializeWebView();
    bool IsInitialized() const 
    {
        return bInitialized;
    };

    /** 加载网页 */
    void LoadURL(const FString& URL);

    void PutCoreSettings(FCoreWebView2Settings CoreWebView2Settings);
    /** 按键处理 */
    void PutHandled(bool bHandled);

    /** 重新加载 */
    void Reload() const;

    void SetBackgroundColor(FColor InBackgroundColor);
    /** 设置webview大小 */
    void SetBounds(RECT Rect);
    /** 设置 WebView2 大小 */
    void SetBounds(POINT Offset, POINT Size);
    void SetContainerVisual(winrt::Windows::UI::Composition::ContainerVisual InWebViewVisual);
    void SetDevDebugTool(bool bEnabled) const;
    void SetLayerID(int32 InLayerID);
    void SetMouseCursor(EMouseCursor::Type MouseCursorType);
    /** 设置可见性 */
    void SetVisible(ESlateVisibility InVisibility);
    /** 停止加载 */
    void Stop() const;


    //判断鼠标是否在位置区域内的标志  暂时放这里
    bool bIsMouseOverPositionArea = true;

    /** Accelerator Key Pressed Event Handler Token */
    EventRegistrationToken AcceleratorKeyPressedToken = {};
    EventRegistrationToken BytesReceivedChangedToken = {};
    /** Cursor Changed Event Handler Token */
    EventRegistrationToken CursorChangedToken = {};
    EventRegistrationToken DownloadStartingToken = {};
    EventRegistrationToken DocumentTitleChangedToken = {};
    EventRegistrationToken EstimatedEndTimeChanged = {};
    EventRegistrationToken HistoryChangedToken = {};
    EventRegistrationToken NavigationCompletedToken = {};
    EventRegistrationToken NavigationStartingToken = {};
    EventRegistrationToken NewWindowRequestedToken = {};
    EventRegistrationToken SourceChangedToken = {};
    EventRegistrationToken StateChangedToken = {};

    FOnCanGoBack OnCanGoBack;
    FOnCanGoForward OnCanGoForward;
    FOnCursorChanged OnCursorChanged;
    FOnDocumentTitleChanged OnDocumentTitleChanged;
    FOnDownloadProgress OnDownloadProgress;
    FOnDownloadStateChanged OnDownloadStateChanged;
    FOnEstimatedDownloadTime OnEstimatedDownloadTime;
    FOnNavigationCompleted OnNavigationCompleted;
    FOnNavigationStarting OnNavigationStarting;
    FOnNewWindowRequested OnNewWindowRequested;
    FOnSourceChanged OnSourceChanged;
    FOnMessageReceived OnMessageReceived;

    FWebView2DownloadInfo DownloadInfo;

    HWND MainWindow = nullptr;

    int64 TotalBytesToReceive = 0;

    Microsoft::WRL::ComPtr<ICoreWebView2> WebView;
    Microsoft::WRL::ComPtr<ICoreWebView2_4> WebView4;
    Microsoft::WRL::ComPtr<ICoreWebView2CompositionController> CompositionController;
    Microsoft::WRL::ComPtr<ICoreWebView2Controller> Controller;
    Microsoft::WRL::ComPtr<ICoreWebView2DownloadOperation> DownloadOperation;
    Microsoft::WRL::ComPtr<ICoreWebView2Environment> WebViewEnvironment;
    Microsoft::WRL::ComPtr<ICoreWebView2ExperimentalControllerOptions2> ControllerOptions2;
    Microsoft::WRL::ComPtr<IDCompositionDevice> CompositionDevice;

    winrt::Windows::UI::Composition::ContainerVisual WebViewVisual = {nullptr};

protected:
    /** 捕获键盘事件回调函数 */
    HRESULT OnAcceleratorKeyPressed(ICoreWebView2Controller* Sender,
                                                ICoreWebView2AcceleratorKeyPressedEventArgs* Args);
    /** 处理下载进度 */
    HRESULT OnBytesReceivedChanged(ICoreWebView2DownloadOperation* Sender, IUnknown* Args);
    /** 创建一个新的网页链接的回调函数 */
    HRESULT OnNewWindowCallback(ICoreWebView2* Sender,
                                                         ICoreWebView2NewWindowRequestedEventArgs* Args);
    /** 创建一个runtime.webview的环境设置的初始化回调函数 */
    HRESULT OnEnvironmentCreated(HRESULT Result, ICoreWebView2Environment* Environment);
    /** 创建一个runtime.webview的WebView2Controller控件的初始化回调函数 */
    HRESULT OnControllerCreated(HRESULT Result, ICoreWebView2Controller* Controller);
    /** 光标改变时的回调函数 */
    HRESULT OnCursorChangedCallback(ICoreWebView2CompositionController* Sender, IUnknown* Args);
    /** 标题修改后 */
    HRESULT OnDocTitleChanged(ICoreWebView2* Sender, IUnknown* Args);
    /** 下载状态发生改变，On DownLoad Change State Event */
    HRESULT OnDownLoadStateCallback(ICoreWebView2DownloadOperation* Sender, IUnknown* Args);
    /** 开始下载 */
    HRESULT OnDownloadStarting(ICoreWebView2* Sender, ICoreWebView2DownloadStartingEventArgs* Args);
    /** 当预计下载时间发生变化时，通知此事件 */
    HRESULT OnEstimatedEndTimeChanged(ICoreWebView2DownloadOperation* Sender, IUnknown* Args);
    /** 当历史记录发生变化时候 */
    HRESULT OnHistoryChanged(ICoreWebView2* Sender, IUnknown* Args);
    /** 信息回调 */
    HRESULT OnMessageReceivedCallback(ICoreWebView2* Webview, ICoreWebView2WebMessageReceivedEventArgs* Args);
    /** 加载完成时的回调函数，可能成功，可能失败 */
    HRESULT OnNavigationEnd(ICoreWebView2* Sender, ICoreWebView2NavigationCompletedEventArgs* Args);
    /** 开始加载网页时的回调函数 */
    HRESULT OnNavigationStarted(ICoreWebView2* Sender, ICoreWebView2NavigationStartingEventArgs* Args);
    /** 网页链接修改后 */
    HRESULT OnSourceChangedCallback(ICoreWebView2* Sender, ICoreWebView2SourceChangedEventArgs* Args);

private:
    void Cleanup(const bool CleanupUserDataFolder);
    void CloseWebView();
    static HRESULT DCompositionCreateDevice2(IUnknown* RenderingDevice, REFIID RefIID, void** PPV);
    static void HandleEnvironmentError(const HRESULT Hresult);
    static FString InterruptReasonToString(const COREWEBVIEW2_DOWNLOAD_INTERRUPT_REASON InterruptReason);
    void ProcessBrowserExited(Microsoft::WRL::ComPtr<ICoreWebView2Environment5> environment5);
    static HRESULT TryCreateDispatcherQueue();

    bool bDevTools;
    bool bInitialized;

    EMouseCursor::Type MouseCursor = EMouseCursor::Default;

    ESlateVisibility Visible;

    EventRegistrationToken BrowserExitedEventToken = {};
    EventRegistrationToken MessageReceiveToken = {};
    EventRegistrationToken MoveFocusRequestedToken = {};
    EventRegistrationToken ZoomFactorChangedToken = {};

    EWebView2DocumentState DocumentState;

    FColor BackgroundColor;
    FString InitialURL;
    FGuid ID;

    int32 LayerID;

    RECT WebuiBound;

    TSharedPtr<FWebView2CompositionHost> CompositionHost;

    UINT32 NewestBrowserPID;

    wil::com_ptr<FCdpCompletionHandler> CaptureHandler;
};
