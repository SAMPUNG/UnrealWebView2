#include "WebView2Viewer.h"
#include "WebView2CompositionHost.h"
#include "WebView2Window.h"
#include "WebView2Manager.h"
// #include "WebView2Subsystem.h"
#include "WebView2Log.h"
#include "Widgets/Images/SThrobber.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Brushes/SlateColorBrush.h"
#include "Components/SlateWrapperTypes.h"

#define LOCTEXT_NAMESPACE "WebView2Runtime"

SWebView2Viewer::SWebView2Viewer()
    : bDevTools(false),
      bShowAddressBar(false), bShowControls(false), bShowInitialThrobber(false), bShowTouchArea(false),
      CursorType(EMouseCursor::Type::Default), BackgroundColor(FColor(0, 0, 0, 0)),
      UniqueId(FGuid::NewGuid())
      , Handle(FReply::Unhandled())
      , WebViewWindowHandle(nullptr)
      , FixScale(1.0f)
{
}

SWebView2Viewer::~SWebView2Viewer()
{
    if (WebView2Window)
    {
        WebView2Window.Reset();
    }
}

void SWebView2Viewer::BuildViewport()
{
    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            .Visibility(
                (bShowControls || bShowAddressBar) ? EVisibility::Visible : EVisibility::Collapsed)
            + SHorizontalBox::Slot()
            .Padding(0, 5)
            .AutoWidth()
            [
                SNew(SHorizontalBox)
                .Visibility(bShowControls ? EVisibility::Visible : EVisibility::Collapsed)
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .Text(LOCTEXT("Back", "Back"))
                    .IsEnabled(this, &SWebView2Viewer::CanGoBack)
                    .OnClicked(this, &SWebView2Viewer::OnBackClicked)
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .Text(LOCTEXT("Forward", "Forward"))
                    .IsEnabled(this, &SWebView2Viewer::CanGoForward)
                    .OnClicked(this, &SWebView2Viewer::OnForwardClicked)
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .Text(this, &SWebView2Viewer::GetReloadButtonText)
                    .OnClicked(this, &SWebView2Viewer::OnReloadClicked)
                ]
                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Right)
                .Padding(5)
                [
                    SNew(STextBlock)
                    .Visibility(bShowAddressBar ? EVisibility::Collapsed : EVisibility::Visible)
                    .Text(this, &SWebView2Viewer::GetTitleText)
                    .Justification(ETextJustify::Right)
                ]
            ]
            + SHorizontalBox::Slot()
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Fill)
            .Padding(5.f, 5.f)
            [
                // A proper address bar widget should go here, for now we use a simple textbox.
                SAssignNew(InputText, SEditableTextBox)
                .Visibility(bShowAddressBar ? EVisibility::Visible : EVisibility::Collapsed)
                .OnTextCommitted(this, &SWebView2Viewer::OnUrlTextCommitted)
                .Text(this, &SWebView2Viewer::GetAddressBarUrlText)
                .SelectAllTextWhenFocused(true)
                .ClearKeyboardFocusOnCommit(true)
                .RevertTextOnEscape(true)
            ]
        ]
        + SVerticalBox::Slot()
        [
            SNew(SOverlay)
            + SOverlay::Slot()
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                SNew(SCircularThrobber)
                .Radius(10.0f)
                .ToolTipText(LOCTEXT("LoadingThrobberToolTip", "Loading page..."))
                .Visibility(this, &SWebView2Viewer::GetLoadingThrobberVisibility)
            ]
            + SOverlay::Slot()
            [
                SAssignNew(PositionOverlay, SOverlay)
            ]
        ]
    ];
}

FWebViewBounds SWebView2Viewer::CalculateBounds(const FGeometry& AllottedGeometry) const
{
    FWebViewBounds Result;

    // 获取缩放比例
    Result.Scale = AllottedGeometry.Scale;
    // 计算偏移量
    Result.Offset = AllottedGeometry.LocalToAbsolute(FVector2D::ZeroVector);
    // 获取绘制尺寸
    Result.Size = AllottedGeometry.GetDrawSize();

    // 尺寸边界检查
    if (Result.Size.X > 16000.f || Result.Size.X < 0.f)
    {
        Result.Size.X = 100.f;
    }
    if (Result.Size.Y > 16000.f || Result.Size.Y < 0.f)
    {
        Result.Size.Y = 100.f;
    }

    // 计算最终位置和尺寸
    Result.Position.x = Result.Offset.X;
    Result.Position.y = bShowAddressBar || bShowControls 
        ? Result.Offset.Y + 30 * Result.Scale 
        : Result.Offset.Y;
    
    Result.Dimensions.x = Result.Size.X;
    Result.Dimensions.y = bShowAddressBar || bShowControls 
        ? Result.Size.Y - 30 * Result.Scale 
        : Result.Size.Y;

    return Result;
}

bool SWebView2Viewer::CanGoBack() const
{
    return bCanGoBack;
}

bool SWebView2Viewer::CanGoForward() const
{
    return bCanGoForward;
}

FVector2D SWebView2Viewer::ComputeDesiredSize(float) const
{
    if (GetVisibility() == EVisibility::Collapsed)
    {
        return FVector2D::ZeroVector;
    }
    if (WebView2Window)
    {
        RECT bound = WebView2Window->GetBounds();
        return FVector2D(
            (static_cast<float>(bound.right - bound.left)),
            (static_cast<float>(bound.bottom - bound.top)));
    }
    return FVector2D::ZeroVector;
}

void SWebView2Viewer::Construct(const FArguments& InArgs, const TSharedRef<SWindow>& InParentWindowPtr)
{
    InitializeURL = InArgs._URL;
    BackgroundColor = InArgs._Color;
    OnMessageReceived = InArgs._NewOnMessageReceived;
    OnNavigationCompleted = InArgs._NewOnNavigationCompleted;
    OnNavigationStarting = InArgs._NewOnNavigationStarting;
    OnNewWindowRequested = InArgs._NewOnNewWindowRequested;
    OnCursorChanged = InArgs._NewOnCursorChanged;
    bDevTools = InArgs._DevTools;
    bShowAddressBar = InArgs._ShowAddressBar;
    bShowControls = InArgs._ShowControls;
    bShowTouchArea = InArgs._ShowTouchArea;
    bShowInitialThrobber = InArgs._ShowInitialThrobber;

    BuildViewport();
    
    void* Handler = InParentWindowPtr->GetNativeWindow()->GetOSWindowHandle();
    if (!Handler)
    {
        return;
    }
    WebViewWindowHandle = static_cast<HWND>(Handler);
    WebView2Window = FWebView2Manager::CreateWebview(
        WebViewWindowHandle,
        UniqueId,
        InitializeURL,
        BackgroundColor,
        bDevTools);
    
    SetupMessageHandler();
    SetupNavigationHandler();
    SetupOtherHandlers();
}

void SWebView2Viewer::OnWindowClosed()
{
    // 安全释放 WebView2 资源，彻底避免内存泄漏
    if (WebView2Window.IsValid())
    {
        // 解绑所有委托，避免野指针
        WebView2Window->OnNavigationCompleted.Unbind();
        // 关闭窗口
        WebView2Window->CloseWindow();
        // 主动释放智能指针
        WebView2Window.Reset();
    }
}

void SWebView2Viewer::ExecuteJavascript(const FString& Script) const
{
    if (WebView2Window)
    {
        WebView2Window->ExecuteScript(Script);
    }
}

void SWebView2Viewer::ExecuteJavascriptAsync(const FString& Script,
                                             FScriptCallback ScriptCallback) const
{
    if (WebView2Window)
    {
        return;
    }
    WebView2Window->ExecuteScript(Script, [ScriptCallback](const FString& Result)
    {
        if (ScriptCallback.IsBound())
        {
            ScriptCallback.Execute(Result);
        }
    });
}

FText SWebView2Viewer::GetAddressBarUrlText() const
{
    if (WebView2Window.IsValid())
    {
        return FText::FromString(URL);
    }
    return FText::GetEmpty();
}

EVisibility SWebView2Viewer::GetLoadingThrobberVisibility() const
{
    if (bShowInitialThrobber && !WebView2Window->IsInitialized())
    {
        return EVisibility::Visible;
    }
    return EVisibility::Hidden;
}

FText SWebView2Viewer::GetReloadButtonText() const
{
    static FText ReloadText = LOCTEXT("Reload", "Reload");
    static FText StopText = LOCTEXT("StopText", "Stop");

    if (WebView2Window.IsValid())
    {
        if (IsLoading())
        {
            return StopText;
        }
    }
    return ReloadText;
}

FText SWebView2Viewer::GetTitleText() const
{
    if (WebView2Window)
    {
        UE_LOG(LogTemp, Display, TEXT("OnDocumentTitleChanged: %s"), *Title);
        return FText::FromString(Title);
    }
    return LOCTEXT("InvalidWindow", "Browser Window is not valid/supported");
}

ESlateVisibility SWebView2Viewer::GetVisible() const
{
    if (WebView2Window.IsValid())
    {
        return WebView2Window->GetVisible();
    }
    return ESlateVisibility::Hidden;
}

void SWebView2Viewer::GoBack() const
{
    if (WebView2Window)
    {
        WebView2Window->GoBack();
    }
}

void SWebView2Viewer::GoForward() const
{
    if (WebView2Window)
    {
        WebView2Window->GoForward();
    }
}

bool SWebView2Viewer::HasKeyboardFocus() const
{
    return SCompoundWidget::HasKeyboardFocus();
}

bool SWebView2Viewer::IsLoading() const
{
    if (!WebView2Window)
    {
        return false;
    }
    return WebView2Window->GetDocumentLoadingState() == EWebView2DocumentState::Loading;
}

void SWebView2Viewer::LoadURL(const FString& InURL)
{
    URL = InURL;
    if (WebView2Window)
    {
        WebView2Window->LoadURL(InURL);
    }
}

void SWebView2Viewer::Reload() const
{
    if (WebView2Window)
    {
        WebView2Window->Reload();
    }
}

void SWebView2Viewer::ReLoad() const
{
    if (WebView2Window)
    {
        WebView2Window->Reload();
    }
}

void SWebView2Viewer::Stop() const
{
    if (WebView2Window)
    {
        WebView2Window->Stop();
    }
}

void SWebView2Viewer::StopLoad() const
{
    if (WebView2Window)
    {
        WebView2Window->Stop();
    }
}

FReply SWebView2Viewer::OnBackClicked() const
{
    GoBack();
    return FReply::Handled();
}

FReply SWebView2Viewer::OnForwardClicked() const
{
    GoForward();
    return FReply::Handled();
}

FReply SWebView2Viewer::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
    const FString Key = KeyEvent.GetKey().ToString();
    UE_LOG(LogWebView2, Log, TEXT("Key Down: %p"), &Key);
    return FReply::Unhandled();
}

FReply SWebView2Viewer::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
    const FString Key = KeyEvent.GetKey().ToString();
    UE_LOG(LogWebView2, Log, TEXT("Key Up: %p"), &Key);
    return FReply::Unhandled();
}

FReply SWebView2Viewer::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    // UWebView2Subsystem* Subsystem = UWebView2Subsystem::GetWebView2Subsystem();

    if (!WebView2Window)
    {
        return SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
    }

    if (CursorType != EMouseCursor::Type::Default)
    {
        return FReply::Handled();
    }

    return FReply::Unhandled();

    // 如果不在区域内，则调用基类默认行为
    // return SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
}

void SWebView2Viewer::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);
}

void SWebView2Viewer::OnMouseLeave(const FPointerEvent& MouseEvent)
{
    SCompoundWidget::OnMouseLeave(MouseEvent);
}

FReply SWebView2Viewer::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    // const UE::Slate::FDeprecateVector2DResult Position = MouseEvent.GetScreenSpacePosition();
    // UE_LOG(LogWebView2, Log, TEXT("Mouse Move: (%f, %f)"), Position.X, Position.Y);
    return SCompoundWidget::OnMouseMove(MyGeometry, MouseEvent);
}


int32 SWebView2Viewer::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                               const FSlateRect& MyCullingRect,
                               FSlateWindowElementList& OutDrawElements, int32 LayerId,
                               const FWidgetStyle& InWidgetStyle,
                               bool bParentEnabled) const
{
    const int32 Layer = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId,
                                           InWidgetStyle, bParentEnabled);
    if (!WebView2Window)
    {
        return Layer;
    }
    
    // 使用提取的计算函数
    const FWebViewBounds Layout = CalculateBounds(AllottedGeometry);

    FixScale = Layout.Scale;
    WebView2Window->SetBounds(Layout.Position, Layout.Dimensions);
    
    if (!WebView2Window->GetCompositionHost())
    {
        return Layer;
    }
    if (LayerId != WebView2Window->GetLayerID())
    {
        WebView2Window->SetLayerID(LayerId);
        WebView2Window->GetCompositionHost()->RefreshWebViewVisual();
    }
    return Layer;
}

FReply SWebView2Viewer::OnReloadClicked() const
{
    if (IsLoading())
    {
        StopLoad();
    }
    else
    {
        Reload();
    }
    return FReply::Handled();
}

void SWebView2Viewer::OnUrlTextCommitted(const FText& NewText, const ETextCommit::Type CommitType)
{
    if (CommitType == ETextCommit::OnEnter)
    {
        LoadURL(NewText.ToString());
    }
}


void SWebView2Viewer::SetBackgroundColor(FColor InBackgroundColor) const
{
    if (WebView2Window)
    {
        WebView2Window->SetBackgroundColor(InBackgroundColor);
    }
}

void SWebView2Viewer::SetDevDebugTool(const bool bEnabled) const
{
    if (WebView2Window)
    {
        WebView2Window->SetDevDebugTool(bEnabled);
    }
}

void SWebView2Viewer::SetMouseCursor(const EMouseCursor::Type MouseCursorType) const
{
    if (WebView2Window)
    {
        WebView2Window->SetMouseCursor(MouseCursorType);
    }
}

void SWebView2Viewer::SetVisible(ESlateVisibility InVisibility)
{
    if (WebView2Window.IsValid())
    {
        WebView2Window->SetVisible(InVisibility);
    }
    TAttribute<EVisibility> Visibility;
    switch (InVisibility)
    {
    case ESlateVisibility::Collapsed:
        Visibility.Set(EVisibility::Collapsed);
        break;
    case ESlateVisibility::Hidden:
        Visibility.Set(EVisibility::Hidden);
        break;
    case ESlateVisibility::Visible:
        Visibility.Set(EVisibility::Visible);
        break;
    case ESlateVisibility::HitTestInvisible:
        Visibility.Set(EVisibility::HitTestInvisible);
        break;
    case ESlateVisibility::SelfHitTestInvisible:
        Visibility.Set(EVisibility::SelfHitTestInvisible);
        break;
    }

    SetVisibility(Visibility);
}

void SWebView2Viewer::SetupMessageHandler()
{
    const float Alpha = bShowTouchArea ? 0.3 : 0;
    WebView2Window->OnMessageReceived.BindLambda([this, Alpha](const FString& Message)
    {
        (void)OnMessageReceived.ExecuteIfBound(Message);
        
        // 解析JSON
        TSharedPtr<FJsonObject> JSONObject;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

        if (!FJsonSerializer::Deserialize(Reader, JSONObject) || !JSONObject.IsValid())
        {
            return;
        }
        // 获取"position"数组
        FString Type;
        if (!JSONObject->TryGetStringField(TEXT("type"), Type))
        {
            return;
        }
        if (!Type.Equals("webPosition"))
        {
            return;
        }
        const TArray<TSharedPtr<FJsonValue>>* Positions;
        if (!JSONObject->TryGetArrayField(TEXT("position"), Positions))
        {
            return;
        }
        // 清除现有的位置标记和区域数据
        PositionOverlay->ClearChildren();
        Images.Empty();
        for (const TSharedPtr<FJsonValue>& PosValue : *Positions)
        {
            const TSharedPtr<FJsonObject> PosObj = PosValue->AsObject();
            if (!PosObj.IsValid())
            {
                continue;
            }
            // 提取坐标和尺寸
            const float X = PosObj->GetNumberField(TEXT("x")) / FixScale;
            const float Y = PosObj->GetNumberField(TEXT("y")) / FixScale;
            float Width = PosObj->GetNumberField(TEXT("width")) / FixScale;
            float Height = PosObj->HasField(TEXT("high"))
                               ? PosObj->GetNumberField(TEXT("high")) / FixScale
                               : PosObj->GetNumberField(TEXT("height")) / FixScale;

            // 创建位置标记 (视觉效果)
            TSharedPtr<SImage> NewImage;
            TSharedPtr<SBox> PositionBox =
                SNew(SBox)
                .WidthOverride(Width)
                .HeightOverride(Height)
                [
                    SAssignNew(NewImage, SImage)
                    .Image(new FSlateColorBrush(FLinearColor(1, 0, 0, Alpha)))
                ];

            Images.Add(NewImage);
            // 添加到PositionOverlay
            PositionOverlay->AddSlot()
                           .Padding(FMargin(X, Y, 0, 0)) // 使用 Padding 设置左上角位置
                           .HAlign(HAlign_Left)
                           .VAlign(VAlign_Top)
            [
                PositionBox.ToSharedRef()
            ];
        }
    });
}

void SWebView2Viewer::SetupNavigationHandler()
{
    const float Alpha = bShowTouchArea ? 0.3 : 0;
    WebView2Window->OnNavigationCompleted.BindLambda([this, Alpha](bool bSuccess)
    {
        (void)OnNavigationCompleted.ExecuteIfBound(bSuccess);

        // 清除现有的位置标记和区域数据
        PositionOverlay->ClearChildren();
        Images.Empty();
        // auto [bottom, left, right, top] = WebView2Window->GetBounds();
        // float SizeY = bottom - top;
        // float SizeX = right - left;

        // 创建位置标记
        TSharedPtr<SImage> NewImage;
        const TSharedPtr<SBox> PositionBox =
            SNew(SBox)
            [
                SAssignNew(NewImage, SImage)
                .Image(new FSlateColorBrush(FLinearColor(1, 0, 0, Alpha)))
            ];

        Images.Add(NewImage);
        // 添加到PositionOverlay
        PositionOverlay->AddSlot()
                       //.Padding(FMargin(OffsetX, OffsetY, 0, 0)) // 使用 Padding 设置左上角位置
                       .HAlign(HAlign_Fill)
                       .VAlign(VAlign_Fill)
        [
            PositionBox.ToSharedRef()
        ];
    });
}


void SWebView2Viewer::SetupOtherHandlers()
{
    WebView2Window->OnNavigationStarting.BindLambda([this](const FString& InURL)
    {
        (void)OnNavigationStarting.ExecuteIfBound(InURL);
    });
    WebView2Window->OnNewWindowRequested.BindLambda([this](const FString& InURL)
    {
        (void)OnNewWindowRequested.ExecuteIfBound(InURL);
    });

    WebView2Window->OnCanGoBack.BindLambda([this](const bool bInCanGoBack)
    {
        bCanGoBack = bInCanGoBack;
    });

    WebView2Window->OnCanGoForward.BindLambda([this](const bool bInCanGoForward)
    {
        bCanGoForward = bInCanGoForward;
    });
    WebView2Window->OnDocumentTitleChanged.BindLambda([this](FString InTitle)
    {
        Title = InTitle;
    });

    WebView2Window->OnSourceChanged.BindLambda([this](FString InURL)
    {
        URL = InURL;
    });

    WebView2Window->OnCursorChanged.BindLambda([this](const EMouseCursor::Type InCursor)
    {
        CursorType = InCursor;
        SetCursor(InCursor);
        (void)OnCursorChanged.ExecuteIfBound(InCursor);
    });
}

void SWebView2Viewer::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
    
    // 获取当前鼠标屏幕坐标
    const FVector2D ScreenMousePos = FSlateApplication::Get().GetCursorPos();

    // 遍历每个SImage
    for (TSharedPtr<SImage> ImageWidget : Images)
    {
        if (!ImageWidget.IsValid())
        {
            continue;
        }
        // 判断鼠标是否在这个Geometry范围内
        if (ImageWidget->GetCachedGeometry().IsUnderLocation(ScreenMousePos))
        {
            if (WebView2Window)
            {
                WebView2Window->bIsMouseOverPositionArea = true;
            }
            break; // 找到一个匹配区域即可退出循环
        }
        if (WebView2Window)
        {
            WebView2Window->bIsMouseOverPositionArea = false;
        }
    }
}

#undef LOCTEXT_NAMESPACE
