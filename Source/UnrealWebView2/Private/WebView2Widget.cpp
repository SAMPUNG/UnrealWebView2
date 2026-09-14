#include "WebView2Widget.h"
#include "WebView2Log.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

UWebView2Widget::UWebView2Widget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
      , bDevTools(false), bShowAddressBar(false), bShowControls(false), bShowTouchArea(false)
      , BackgroundColor(FColor(0, 0, 0, 0))
      , URL(TEXT("https://bing.com/"))
{
}

void UWebView2Widget::ExecuteJavascript(const FString& Script) const
{
    if (WebView2)
    {
        WebView2->ExecuteJavascript(Script);
    }
}

void UWebView2Widget::ExecuteJavascriptAsync(const FString& Script, FScriptCallback Callback) const
{
    if (WebView2)
    {
        WebView2->ExecuteJavascriptAsync(Script, SWebView2Viewer::FScriptCallback::CreateLambda(
                                             [Callback](const FString& Data)
                                             {
                                                 if (Callback.IsBound())
                                                 {
                                                     Callback.Execute(Data);
                                                 }
                                             }));
    }
}

void UWebView2Widget::GoBack() const
{
    if (WebView2)
    {
        WebView2->GoBack();
    }
}

void UWebView2Widget::GoForward() const
{
    if (WebView2)
    {
        WebView2->GoForward();
    }
}

void UWebView2Widget::LoadURL(const FString InURL)
{
    URL = InURL;
    if (WebView2)
    {
        WebView2->LoadURL(InURL);
    }
}

void UWebView2Widget::OnLoaded(const bool& bSuccess) const
{
    OnLoadCompleted.Broadcast(bSuccess);
}

void UWebView2Widget::OnMessageReceivedCallback(const FString& Message)
{
    // 0. 先去掉首尾双引号（如果有）
    FString InvalidMessage = Message;
    // UE_LOG(LogWebView2, Log, TEXT("Message: %s"), *Message);
    if (Message.Len() >= 2 && Message[0] == '"' && Message[Message.Len() - 1] == '"')
    {
        InvalidMessage = Message.Mid(1, Message.Len() - 2).Replace(TEXT("\\\""), TEXT("\""));
        // UE_LOG(LogWebView2, Error, TEXT("Mid String: %s"), *InvalidMessage);
    }
    // 1. 创建 JSON Reader
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InvalidMessage);
    // 2. 反序列化 JSON 字符串到 JsonObject
    TSharedPtr<FJsonObject> JsonObject;
    if (!FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        UE_LOG(LogWebView2, Error, TEXT("Failed to parse JSON from string."));
        OnMessageReceived.Broadcast(InvalidMessage);
        return;
    }
    if (!JsonObject.IsValid())
    {
        UE_LOG(LogWebView2, Error, TEXT("JSON object is invalid."));
        OnMessageReceived.Broadcast(InvalidMessage);
        return;
    }
    // 3. 从 JsonObject 中提取字段
    const EWebMessageType Type = UWebMessageParser::ParseType(JsonObject->GetStringField(TEXT("type")));
    // 3.1 键盘输入
    if (Type == EWebMessageType::Pressed || Type == EWebMessageType::Released)
    {
        const EKeyboardKey Key = UWebMessageParser::ParseKey(JsonObject->GetStringField(TEXT("key")));
        OnKeyboardInput.Broadcast(Key, Type);
        return;
    }
    // 3.2 相机移动
    if (Type == EWebMessageType::Movement)
    {
        const FString MovementKey = JsonObject->GetStringField(TEXT("key"));
        UE_LOG(LogWebView2, Log, TEXT("Camera Movement: %s"), *MovementKey);
        if (FVector Movement(0.f, 0.f, 0.f); Movement.InitFromString(MovementKey))
        {
            OnCameraMoved.Broadcast(Movement);
        }
        else
        {
            UE_LOG(LogWebView2, Error, TEXT("Failed to parse camera movement from string."));
        }
        return;
    }
    // 3.3 排除鼠标输入的情况
    if (Type != EWebMessageType::MouseCursor)
    {
        UE_LOG(LogWebView2, Warning, TEXT("Invalid Message: %s"), *InvalidMessage);
        OnMessageReceived.Broadcast(InvalidMessage);
        return;
    }
    // 3.4 最后处理鼠标输入
    const FString WebCursor = JsonObject->GetStringField(TEXT("cursor"));
    UE_LOG(LogWebView2, Log, TEXT("Mouse Cursor: %s"), *WebCursor);
    const EMouseCursor::Type EngineMouseCursor = UWebMessageParser::ParseCursor(WebCursor);
    const EBlueprintCursor MouseCursor = static_cast<EBlueprintCursor>(EngineMouseCursor);
    OnWebCursorChanged(EngineMouseCursor);
    OnCursorChanged.Broadcast(MouseCursor);
}

void UWebView2Widget::OnWebCursorChanged(const EMouseCursor::Type EngineMouseCursor)
{
    if (!WebView2)
    {
        UE_LOG(LogWebView2, Error, TEXT("WebView2 Viewer is Invalid"));
        return;
    }
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        UE_LOG(LogWebView2, Error, TEXT("First Player Controller is Invalid"));
        return;
    }
    APawn* CurrentPawn = PlayerController->GetPawn();
    if (!CurrentPawn)
    {
        UE_LOG(LogWebView2, Error, TEXT("Current Pawn is Invalid"));
        return;
    }
    WebView2->SetMouseCursor(EngineMouseCursor);
    SetCursor(EngineMouseCursor);
    // 根据光标样式判定交互是否穿透
    if (EngineMouseCursor == EMouseCursor::Default || EngineMouseCursor == EMouseCursor::None)
    {
        UE_LOG(LogWebView2, Log, TEXT("Interactive Mouse Cursor: True"));
        CurrentPawn->EnableInput(PlayerController);
        SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    else
    {
        UE_LOG(LogWebView2, Log, TEXT("Interactive Mouse Cursor: False"));
        CurrentPawn->DisableInput(PlayerController);
        SetVisibility(ESlateVisibility::Visible);
    }
}

void UWebView2Widget::OnNavigationStarting(const FString& NewURL) const
{
    OnLoadStart.Broadcast(NewURL);
}

void UWebView2Widget::OnNewWindowCallback(const FString& NewURL) const
{
    OnNewWindowRequested.Broadcast(NewURL);
}

TSharedRef<SWidget> UWebView2Widget::RebuildWidget()
{
    if (GEngine && GEngine->GameViewport)
    {
        return
                SAssignNew(WebView2, SWebView2Viewer, GEngine->GameViewport->GetWindow().ToSharedRef())
                .URL(URL)
                .Color(BackgroundColor)
                .DevTools(bDevTools)
                .ShowAddressBar(bShowAddressBar)
                .ShowControls(bShowControls)
                .ShowTouchArea(bShowTouchArea)
                .NewOnNewWindowRequested_UObject(this, &UWebView2Widget::OnNewWindowCallback)
                .NewOnMessageReceived_UObject(this, &UWebView2Widget::OnMessageReceivedCallback)
                .NewOnNavigationCompleted_UObject(this, &UWebView2Widget::OnLoaded)
                .NewOnNavigationStarting_UObject(this, &UWebView2Widget::OnNavigationStarting);
    }
    return SNew(SBox)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock).Text(FText::FromString(TEXT("this is webview2 ui")))
        ];
}

void UWebView2Widget::ReleaseSlateResources(const bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);

    if (WebView2)
    {
        WebView2->OnWindowClosed();
    }
    WebView2.Reset();
}

void UWebView2Widget::Reload() const
{
    if (WebView2)
    {
        WebView2->ReLoad();
    }
}

void UWebView2Widget::SendMessage(const FString Message) const
{
    // Build JS Code
    FString JSCode = "window.getMessage(";
    JSCode += Message;
    JSCode += ")";

    UE_LOG(LogWebView2, Log, TEXT("Execute Javascript: %s"), *JSCode);
    if (WebView2)
    {
        WebView2->ExecuteJavascript(JSCode);
    }
}

void UWebView2Widget::SetBackgroundColor(const FColor InBackgroundColor) const
{
    if (WebView2)
    {
        WebView2->SetBackgroundColor(InBackgroundColor);
    }
}

void UWebView2Widget::SetVisible(const ESlateVisibility InVisibility)
{
    if (WebView2)
    {
        WebView2->SetVisible(InVisibility);
    }

    SetVisibility(InVisibility);
}

void UWebView2Widget::Stop() const
{
    if (WebView2)
    {
        WebView2->Stop();
    }
}
