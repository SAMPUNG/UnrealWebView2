#include "WebView2CompositionHost.h"
#include "WebView2Log.h"
#include "WebView2Settings.h"
#include "WebView2Window.h"
#include "Components/SlateWrapperTypes.h"

FWebView2CompositionHost::FWebView2CompositionHost(HWND Handler,
                                                   winrt::Windows::System::DispatcherQueueController QueueController)
    : ClickLayerID(0), WindowHandler(Handler), MDispatcherQueueController(QueueController)

{
    UWebView2Settings* Settings = UWebView2Settings::Get();

    if (Settings->WebView2Mode == EWebView2Mode::VISUAL_WINCOMP)
    {
        if (QueueController)
        {
            MCompositor = winrt::Windows::UI::Composition::Compositor();
        }
    }
}

FWebView2CompositionHost::~FWebView2CompositionHost()
{
    DestroyWinCompVisualTree();
}

void FWebView2CompositionHost::ConvertCoordinates(HWND Window, POINT& Point, UINT Message)
{
    if (Message == WM_MOUSEWHEEL ||
        Message == WM_MOUSEHWHEEL ||
        Message == WM_NCRBUTTONDOWN || 
        Message == WM_NCRBUTTONUP)
    {
        ::ScreenToClient(Window, &Point);
    }
}

POINT FWebView2CompositionHost::ConvertToLocal(
    POINT ScreenPoint, const RECT& WebViewBounds) const
{
    POINT LocalPoint;
    LocalPoint.x = ScreenPoint.x - WebViewBounds.left;
    LocalPoint.y = ScreenPoint.y - WebViewBounds.top;
    return LocalPoint;
}

void FWebView2CompositionHost::CreateCompositionRoot()
{
    //创建根容器
    MRootVisual = MCompositor.CreateContainerVisual();
    MRootVisual.RelativeSizeAdjustment({1.0f, 1.0f});
    MRootVisual.Offset({0, 0, 0});
    MTarget.Root(MRootVisual);

    //创建拥于存放2DUI的容器，2DUI的网页容器层级要高于3DUI容器
    UIVisual = MCompositor.CreateContainerVisual();
    MRootVisual.Children().InsertAtTop(UIVisual);
    UIVisual.RelativeSizeAdjustment({1.0f, 1.0f});
    UIVisual.Offset({0, 0, 0});
}

void FWebView2CompositionHost::CreateDesktopWindowTarget()
{
    namespace abi = ABI::Windows::UI::Composition::Desktop;

    auto interop = MCompositor.as<abi::ICompositorDesktopInterop>();
    winrt::check_hresult(interop->CreateDesktopWindowTarget(
        WindowHandler, false, reinterpret_cast<abi::IDesktopWindowTarget**>(winrt::put_abi(MTarget))));
}

void FWebView2CompositionHost::CreateWebViewVisual(TSharedRef<FWebView2Window> WebView2Window)
{
    UE_LOG(LogWebView2, Log, TEXT("WebView2Window is Created"));

    if (UIVisual && WebView2Window->Controller)
    {
        winrt::Windows::UI::Composition::ContainerVisual NewChild = MCompositor.CreateContainerVisual();
        UIVisual.Children().InsertAtTop(NewChild);
        RECT bound = WebView2Window->GetBounds();
        NewChild.Offset({static_cast<float>(bound.left), static_cast<float>(bound.top), 0});
        const winrt::Windows::Foundation::Numerics::float2 WebViewSize = {
            (static_cast<float>(bound.right - bound.left)),
            (static_cast<float>(bound.bottom - bound.top))
        };
        NewChild.Size(WebViewSize);
        // webViewVisual.CompositeMode()
        WebView2Window->SetContainerVisual(NewChild);
        WebView2Window->CompositionController->put_RootVisualTarget(NewChild.as<IUnknown>().get());

        //每创建一个新的UI，则增加一下层级ID
        ClickLayerID++;
        WebView2Window->SetLayerID(ClickLayerID);
    }

    WebViewWindowMap.Add(WebView2Window->GetUniqueID().ToString(), WebView2Window);
    UE_LOG(LogWebView2, Log, TEXT("WebView2Window is Created"));
}

void FWebView2CompositionHost::DestroyWinCompVisualTree()
{
    if (!WebViewWindowMap.IsEmpty())
    {
        for (TPair<FString, TSharedRef<FWebView2Window>>& Element : WebViewWindowMap)
        {
            if (Element.Value.ToSharedPtr().IsValid() && UIVisual)
            {
                UIVisual.Children().Remove(Element.Value->WebViewVisual);
            }
        }
    }

    if (MRootVisual)
    {
        MRootVisual.Children().RemoveAll();
        MRootVisual = nullptr;

        MTarget.Root(nullptr);
        MTarget = nullptr;
    }
}

void FWebView2CompositionHost::DestroyVisualAsWebview(TSharedRef<FWebView2Window> WebView2Window)
{
    if (WebView2Window.ToSharedPtr().IsValid())
    {
        WebViewWindowMap.Find(WebView2Window->GetUniqueID().ToString());
        WebViewWindowMap.Remove(WebView2Window->GetUniqueID().ToString());
    }
}

void FWebView2CompositionHost::DispatchMouseMessage(
    TSharedPtr<FWebView2Window> TargetWindow,
    UINT Message, WPARAM WParam, POINT LocalPoint) const
{
    const DWORD MouseData = ExtractMouseData(Message, WParam);
    
    TargetWindow->CompositionController->SendMouseInput(
        static_cast<COREWEBVIEW2_MOUSE_EVENT_KIND>(Message),
        static_cast<COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS>(GET_KEYSTATE_WPARAM(WParam)),
        MouseData,
        LocalPoint);
}

DWORD FWebView2CompositionHost::ExtractMouseData(UINT Message, WPARAM WParam) const
{
    if (Message == WM_MOUSEWHEEL || Message == WM_MOUSEHWHEEL)
    {
        return GET_WHEEL_DELTA_WPARAM(WParam);
    }
    return 0;
}

TArray<TSharedRef<FWebView2Window>> FWebView2CompositionHost::FindWebviewFromPoint(POINT Point)
{
    TArray<TSharedRef<FWebView2Window>> Windows;
    for (TPair<FString, TSharedRef<FWebView2Window>>& Element : WebViewWindowMap)
    {
        if (!Element.Value.ToSharedPtr())
        {
            continue;
        }
        if (Element.Value->WebViewVisual)
        {
            auto Offset = Element.Value->WebViewVisual.Offset();
            auto Size = Element.Value->WebViewVisual.Size();
            if ((Point.x >= Offset.x) && (Point.x < Offset.x + Size.x) && (Point.y >= Offset.y) &&
                (Point.y < Offset.y + Size.y))
            {
                if (Element.Value->GetVisible() == ESlateVisibility::Visible && Element.Value->bIsMouseOverPositionArea)
                {
                    Windows.Add(Element.Value);
                }
            }
        }
    }

    return Windows;
}

HWND FWebView2CompositionHost::GetMainWindowHandle() const
{
    return WindowHandler;
}

void FWebView2CompositionHost::Initialize()
{
    if (MCompositor)
    {
        CreateDesktopWindowTarget();
        //跟图元，里面用于放多个webview空间
        CreateCompositionRoot();
    }
}

bool FWebView2CompositionHost::MouseMessage(UINT Message, WPARAM WParam, LPARAM LParam)
{
    // 如果 MRootVisual 无效
    if (!MRootVisual)
    {
        return false;
    }

    // 忽略 WM_CLOSE 和 WM_MOUSELEAVE 消息
    if (Message == WM_CLOSE || Message == WM_MOUSELEAVE)
    {
        return false;
    }

    // 坐标提取，将屏幕坐标转换为客户端坐标
    POINT Point;
    POINTSTOPOINT(Point, LParam);
    ConvertCoordinates(WindowHandler, Point, Message);

    // 查找目标 WebView 窗口
    TArray<TSharedRef<FWebView2Window>> HitTestResults = FindWebviewFromPoint(Point);
    if (HitTestResults.Num() == 0)
    {
        return false;
    }

    // 选择最顶层窗口
    TSharedPtr<FWebView2Window> TargetWindow = SelectTopWindow(HitTestResults);
    if (!TargetWindow)
    {
        return false;
    }

    // 发送鼠标消息到 WebView 
    const POINT LocalPoint = ConvertToLocal(Point, TargetWindow->GetBounds());
    DispatchMouseMessage(TargetWindow, Message, WParam, LocalPoint);
    
    return true;
}

void FWebView2CompositionHost::RefreshWebViewVisual()
{
    if (UIVisual)
    {
        TMap<int32, TSharedRef<FWebView2Window>> LayerWebMap;
        for (TPair<FString, TSharedRef<FWebView2Window>>& Element : WebViewWindowMap)
        {
            if (Element.Value.ToSharedPtr().IsValid())
            {
                LayerWebMap.Add(Element.Value->GetLayerID(), Element.Value);
            }
        }

        LayerWebMap.KeySort([](const int& A, const int& B)
        {
            return (A < B);
        });

        for (TPair<int32, TSharedRef<FWebView2Window>>& Element : LayerWebMap)
        {
            UIVisual.Children().Remove(Element.Value->WebViewVisual);
            UIVisual.Children().InsertAtTop(Element.Value->WebViewVisual);
        }
    }
}

TSharedPtr<FWebView2Window> FWebView2CompositionHost::SelectTopWindow(
    const TArray<TSharedRef<FWebView2Window>>& Candidates) const
{
    if (Candidates.Num() == 0)
    {
        return nullptr;
    }

    TSharedPtr<FWebView2Window> TopWindow = Candidates[0];
    for (int32 i = 1; i < Candidates.Num(); ++i)
    {
        if (Candidates[i]->GetLayerID() > TopWindow->GetLayerID())
        {
            TopWindow = Candidates[i];
        }
    }
    return TopWindow;
}
