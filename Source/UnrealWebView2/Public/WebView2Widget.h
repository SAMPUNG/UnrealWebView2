#pragma once
#include "CoreMinimal.h"
#include "Components/Widget.h"
// #include "Blueprint/UserWidget.h"
#include "WebView2Message.h"
#include "WebView2Viewer.h"
#include "WebView2Widget.generated.h"

/** 定义一个蓝图可见的光标类型别名 */
UENUM(BlueprintType)
enum class EBlueprintCursor : uint8
{
    /** Causes no mouse cursor to be visible */
    None = static_cast<uint8>(EMouseCursor::None),
    /** Default cursor (arrow) */
    Default = static_cast<uint8>(EMouseCursor::Default),
    /** Text edit beam */
    TextEditBeam = static_cast<uint8>(EMouseCursor::TextEditBeam),
    /** Resize horizontal */
    ResizeLeftRight = static_cast<uint8>(EMouseCursor::ResizeLeftRight),
    /** Resize vertical */
    ResizeUpDown = static_cast<uint8>(EMouseCursor::ResizeUpDown),
    /** Resize diagonal */
    ResizeSouthEast = static_cast<uint8>(EMouseCursor::ResizeSouthEast),
    /** Resize other diagonal */
    ResizeSouthWest = static_cast<uint8>(EMouseCursor::ResizeSouthWest),
    /** MoveItem */
    CardinalCross = static_cast<uint8>(EMouseCursor::CardinalCross),
    /** Target Cross */
    Crosshairs = static_cast<uint8>(EMouseCursor::Crosshairs),
    /** Hand cursor */
    Hand = static_cast<uint8>(EMouseCursor::Hand),
    /** Grab Hand cursor */
    GrabHand = static_cast<uint8>(EMouseCursor::GrabHand),
    /** Grab Hand cursor closed */
    GrabHandClosed = static_cast<uint8>(EMouseCursor::GrabHandClosed),
    /** a circle with a diagonal line through it */
    SlashedCircle = static_cast<uint8>(EMouseCursor::SlashedCircle),
    /** Eye-dropper cursor for picking colors */
    EyeDropper = static_cast<uint8>(EMouseCursor::EyeDropper),
    /** Custom cursor shape for platforms that support setting a native cursor shape. Same as specifying None if not set. */
    Custom = static_cast<uint8>(EMouseCursor::Custom),
    /** Number of cursors we support */
    TotalCursorCount = static_cast<uint8>(EMouseCursor::TotalCursorCount)
};

UCLASS()
class UNREALWEBVIEW2_API UWebView2Widget : public UWidget
{
public:
    GENERATED_UCLASS_BODY()

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraMoved, const FVector&, Movement);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCursorChanged, EBlueprintCursor, Cursor);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKeyboardInput, EKeyboardKey, Key, EWebMessageType, State);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadCompleted, bool, bSuccess);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadStart, FString, URL);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageReceived, const FString&, Message);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewWindowRequested, FString, URL);
    
    DECLARE_DYNAMIC_DELEGATE_OneParam(FScriptCallback, FString, Data);
    
    /** 执行前端脚本 */
    UFUNCTION(BlueprintCallable, Category="WebView2")
    void ExecuteJavascript(const FString& Script) const;
    
    /** 执行前端脚本 */
    UFUNCTION(BlueprintCallable, Category="WebView2")
    void ExecuteJavascriptAsync(const FString& Script, FScriptCallback Callback) const;

    /** 后退 */
    UFUNCTION(BlueprintCallable, Category="WebView2")
    void GoBack() const;

    /** 前进 */
    UFUNCTION(BlueprintCallable, Category="WebView2")
    void GoForward() const;

    /** 加载网页 */
    UFUNCTION(BlueprintCallable, Category="WebView2")
    void LoadURL(const FString InURL);

    /** 重写此函数以构造你的 Slate 界面 */
    virtual TSharedRef<SWidget> RebuildWidget() override;

    virtual void ReleaseSlateResources(bool bReleaseChildren) override;

    /** 重新加载 */
    UFUNCTION(BlueprintCallable, Category="WebView2")
    void Reload() const;

    /** 发送消息 */
    UFUNCTION(BlueprintCallable, Category="WebView2")
    void SendMessage(const FString Message) const;

    /** 设置背景演示，若 Alpha=0，则表现为透明背景 */
    UFUNCTION(BlueprintCallable, Category="Appearence")
    void SetBackgroundColor(FColor InBackgroundColor) const;

    UFUNCTION(BlueprintCallable, Category="Widget")
    virtual void SetVisible(ESlateVisibility InVisibility);

    /** 停止加载 */
    UFUNCTION(BlueprintCallable, Category="WebView2")
    void Stop() const;

    /** 开发调试工具 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WebView2")
    bool bDevTools;
    
    /** 是否显示网页链接栏 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WebView2")
    bool bShowAddressBar;

    /** 是否显示控制栏 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WebView2")
    bool bShowControls;

    /** 是否显示网页可点击区域 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WebView2")
    bool bShowTouchArea;

    /** 背景颜色，不可半透明，但可以背景透明 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WebView2")
    FColor BackgroundColor;

    UPROPERTY(BlueprintAssignable, Category = "WebView2|Event")
    FOnCameraMoved OnCameraMoved;

    UPROPERTY(BlueprintAssignable, Category = "WebView2|Event")
    FOnKeyboardInput OnKeyboardInput;
    
    UPROPERTY(BlueprintAssignable, Category = "WebView2|Event")
    FOnCursorChanged OnCursorChanged;

    UPROPERTY(BlueprintAssignable, Category = "WebView2|Event")
    FOnLoadCompleted OnLoadCompleted;

    UPROPERTY(BlueprintAssignable, Category = "WebView2|Event")
    FOnLoadStart OnLoadStart;

    UPROPERTY(BlueprintAssignable, Category = "WebView2|Event")
    FOnMessageReceived OnMessageReceived;

    UPROPERTY(BlueprintAssignable, Category = "WebView2|Event")
    FOnNewWindowRequested OnNewWindowRequested;

    /** 网页的默认链接地址 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WebView2")
    FString URL;

protected:
    UFUNCTION()
    void OnLoaded(const bool& bSuccess) const;

    UFUNCTION()
    void OnMessageReceivedCallback(const FString& Message);

    UFUNCTION()
    void OnNavigationStarting(const FString& NewURL) const;

    UFUNCTION()
    void OnNewWindowCallback(const FString& NewURL) const;

private:
    /** 处理鼠标光标变化 */
    void OnWebCursorChanged(const EMouseCursor::Type EngineMouseCursor);
    
    TSharedPtr<SWebView2Viewer> WebView2;
};
