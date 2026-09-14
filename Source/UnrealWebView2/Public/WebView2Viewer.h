#pragma once
#include "CoreMinimal.h"
#include "WebView2Window.h"

class SConstraintCanvas;

struct FWebViewBounds
{
    FVector2D Offset;
    FVector2D Size;
    POINT Position;
    POINT Dimensions;
    float Scale;
};

class UNREALWEBVIEW2_API SWebView2Viewer : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SWebView2Viewer)
            : _URL(TEXT("https://bing.com/"))
              , _Color(FColor(0, 0, 0, 0))
              , _DevTools(false)
              , _ShowAddressBar(false)
              , _ShowControls(false)
              , _ShowInitialThrobber(true)
              , _ShowTouchArea(false)
        {
        }

        /** 网页链接 */
        SLATE_ARGUMENT(FString, URL)
        /** 网页背景颜色 */
        SLATE_ARGUMENT(FColor, Color)
        /** Whether to enable dev tools. */
        SLATE_ARGUMENT(bool, DevTools)
        /** Whether to show an address bar. */
        SLATE_ARGUMENT(bool, ShowAddressBar)
        /** Whether to show standard controls like Back, Forward, Reload etc. */
        SLATE_ARGUMENT(bool, ShowControls)
        /** Whether to show a throbber overlay during browser initialization. */
        SLATE_ARGUMENT(bool, ShowInitialThrobber)
        /**是否显示网页可点击区域*/
        SLATE_ARGUMENT(bool, ShowTouchArea)

        SLATE_EVENT(FOnMessageReceived, NewOnMessageReceived)
        SLATE_EVENT(FOnNavigationCompleted, NewOnNavigationCompleted)
        SLATE_EVENT(FOnNavigationStarting, NewOnNavigationStarting)
        SLATE_EVENT(FOnNewWindowRequested, NewOnNewWindowRequested)
        SLATE_EVENT(FOnCursorChanged, NewOnCursorChanged)
    SLATE_END_ARGS()

    DECLARE_DELEGATE_OneParam(FScriptCallback, const FString& /*Data*/)

    SWebView2Viewer();
    virtual ~SWebView2Viewer() override;

    /** 如果浏览器可以向后导航则返回 true。 */
    bool CanGoBack() const;
    /** 如果浏览器可以向前导航，则返回 true。 */
    bool CanGoForward() const;

    virtual FVector2D ComputeDesiredSize(float) const override;

    void Construct(const FArguments& InArgs, const TSharedRef<SWindow>& InParentWindowPtr);

    void ExecuteJavascript(const FString& Script) const;

    void ExecuteJavascriptAsync(const FString& Script,
                       FScriptCallback ScriptCallback = FScriptCallback()) const;

    /** 获取地址栏中显示的 URL，这可能不是框架中当前加载的 URL */
    FText GetAddressBarUrlText() const;
    /** 获取当前网页标题 */
    FText GetTitleText() const;
    ESlateVisibility GetVisible() const;

    /** 后退 */
    void GoBack() const;
    /** 前进 */
    void GoForward() const;

    virtual bool HasKeyboardFocus() const override;

    /** Document 当前是否正在加载 */
    bool IsLoading() const;

    /** 加载网页 */
    void LoadURL(const FString& InURL);

    virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;
    virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;
    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
                          FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
                          bool bParentEnabled) const override;
    
    // 窗口关闭时的回调，用于自动清理
    void OnWindowClosed();

    /** 重新加载 */
    void ReLoad() const;
    /** 重新加载页面 */
    void Reload() const;

    void SetBackgroundColor(FColor InBackgroundColor) const;
    void SetDevDebugTool(bool bEnabled) const;
    void SetMouseCursor(EMouseCursor::Type MouseCursorType) const;
    void SetVisible(ESlateVisibility InVisibility);

    /** 停止加载 */
    void Stop() const;
    /** 停止加载 */
    void StopLoad() const;

    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
    
    /** 是否显示开发调试工具 */
    bool bDevTools;
    /** 是否显示地址栏 */
    bool bShowAddressBar;
    /** 是否显示控制栏 */
    bool bShowControls;
    /** The initial throbber setting */
    bool bShowInitialThrobber;
    /** 是否线上控制栏 */
    bool bShowTouchArea;

    FOnCursorChanged OnCursorChanged;
    FOnNavigationCompleted OnNavigationCompleted;
    FOnNavigationStarting OnNavigationStarting;
    FOnNewWindowRequested OnNewWindowRequested;
    FOnMessageReceived OnMessageReceived;

private:
    void BuildViewport();

    /** Calculate the bounds of the web view */
    FWebViewBounds CalculateBounds( const FGeometry& AllottedGeometry) const;
    
    /** Get whether loading throbber should be visible */
    EVisibility GetLoadingThrobberVisibility() const;
    /** Get text for reload button depending on status */
    FText GetReloadButtonText() const;

    /** Navigate backwards. */
    FReply OnBackClicked() const;
    /** Navigate forwards. */
    FReply OnForwardClicked() const;
    /** Reload or stop loading */
    FReply OnReloadClicked() const;
    /** Invoked whenever text is committed in the address bar. */
    void OnUrlTextCommitted(const FText& NewText, const ETextCommit::Type CommitType);
    
    void SetupMessageHandler();
    void SetupNavigationHandler();
    void SetupOtherHandlers();
    
    bool bCanGoBack = false;
    bool bCanGoForward = false;

    EMouseCursor::Type CursorType;

    FString InitializeURL;
    FColor BackgroundColor;
    FGuid UniqueId;
    FReply Handle;
    FString Title;
    FString URL;

    HWND WebViewWindowHandle;

    // 缩放比
    mutable float FixScale;

    TSharedPtr<FWebView2Window> WebView2Window;
    // 绝对布局容器
    TSharedPtr<SConstraintCanvas> Canvas;
    /** Editable text widget used for an address bar */
    TSharedPtr<SEditableTextBox> InputText;
    TSharedPtr<SOverlay> PositionOverlay;
    TArray<TSharedPtr<SImage>> Images;
};
