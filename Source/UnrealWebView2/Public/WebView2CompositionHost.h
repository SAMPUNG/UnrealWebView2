#pragma once
#include "Windows/WindowsHWrapper.h"
#include "Windows/AllowWindowsPlatformTypes.h"
// ReSharper disable once CppUnusedIncludeDirective
// 注意：WebView2 接口依赖此头文件，禁止删除！
#include "Windows/AllowWindowsPlatformAtomics.h"
// ReSharper disable once CppUnusedIncludeDirective
// 注意：WebView2 接口依赖此头文件，禁止删除！
#include <Stdafx.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <windows.ui.composition.interop.h>
#include <winrt/Windows.UI.Composition.h>
// ReSharper disable once CppUnusedIncludeDirective
// 注意：WebView2 接口依赖此头文件，禁止删除！
#include "Windows/HideWindowsPlatformAtomics.h"
#include "Windows/HideWindowsPlatformTypes.h"

class FWebView2Window;

class UNREALWEBVIEW2_API FWebView2CompositionHost
{
public:
    FWebView2CompositionHost(HWND Handler, winrt::Windows::System::DispatcherQueueController QueueController);
    ~FWebView2CompositionHost();

    void CreateWebViewVisual(TSharedRef<FWebView2Window> WebView2Window);

    void DestroyVisualAsWebview(TSharedRef<FWebView2Window> WebView2Window);
    // 删除所有网页
    void DestroyWinCompVisualTree();

    /*
    * 寻找当前鼠标位置的网页控件
    */
    TArray<TSharedRef<FWebView2Window>> FindWebviewFromPoint(POINT Point);

    HWND GetMainWindowHandle() const;

    void Initialize();

    bool MouseMessage(UINT Message, WPARAM WParam, LPARAM LParam);

    void RefreshWebViewVisual();

    TMap<FString, TSharedRef<FWebView2Window>> WebViewWindowMap;

protected:
    // 创建组合根
    void CreateCompositionRoot();

    void CreateDesktopWindowTarget();

private:
    // 转换坐标系统
    void ConvertCoordinates(HWND Window, POINT& Point, UINT Message);

    // 转换坐标为 WebView2 局部坐标
    POINT ConvertToLocal(POINT ScreenPoint,
                         const RECT& WebViewBounds) const;

    // 发送鼠标消息到 WebView
    void DispatchMouseMessage(TSharedPtr<FWebView2Window> TargetWindow,
                              UINT Message, WPARAM WParam, POINT LocalPoint) const;

    // 提取鼠标消息的额外数据
    DWORD ExtractMouseData(UINT Message, WPARAM WParam) const;

    // 从候选窗口中选择最顶层
    TSharedPtr<FWebView2Window> SelectTopWindow(
        const TArray<TSharedRef<FWebView2Window>>& Candidates) const;

    int32 ClickLayerID;

    HWND WindowHandler;

    winrt::Windows::UI::Composition::Compositor MCompositor{nullptr};
    winrt::Windows::System::DispatcherQueueController MDispatcherQueueController{nullptr};
    winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget MTarget{nullptr};
    winrt::Windows::UI::Composition::ContainerVisual MRootVisual{nullptr};
    // UI 存放的插槽
    winrt::Windows::UI::Composition::ContainerVisual UIVisual{nullptr};
};
