#include "WebView2Manager.h"

#include "Windows/WindowsHWrapper.h"
#include "Windows/AllowWindowsPlatformTypes.h"
// ReSharper disable once CppUnusedIncludeDirective
// 注意：WebView2 接口依赖此头文件，禁止删除！
#include "Windows/AllowWindowsPlatformAtomics.h"

#include <DispatcherQueue.h>
// ReSharper disable once CppUnusedIncludeDirective
// 注意：WebView2 接口依赖此头文件，禁止删除！
#include "Windows/HideWindowsPlatformAtomics.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include "WebView2Settings.h"
#include "WebView2CompositionHost.h"
#include "WebView2Log.h"
FWebView2Manager* FWebView2Manager::WebView2ManagerInstance = nullptr;

namespace WinSystem = winrt::Windows::System;

FWebView2Manager::FWebView2Manager()
{
}

void FWebView2Manager::Closed(const HWND Handler)
{
    if (!Handler)
    {
        return;
    }

    if (WebView2CompositionHostMap.Contains(Handler))
    {
        if (WebView2CompositionHostMap.Find(Handler)->Pin())
        {
            WebView2CompositionHostMap.Find(Handler)->Pin()->DestroyWinCompVisualTree();
        }
    }
    WebView2CompositionHostMap.Remove(Handler);
}

TSharedPtr<FWebView2Window> FWebView2Manager::CreateWebview(HWND InHandler, const FGuid UniqueID, const FString& URL,
                                                            FColor InBackgroundColor, const bool bDevTools)
{
    TSharedPtr<FWebView2Window> WebView2Window = MakeShared<FWebView2Window>(
        InHandler, UniqueID, URL, InBackgroundColor, bDevTools);
    return WebView2Window;
}

FWebView2Manager* FWebView2Manager::GetInstance()
{
    if (!WebView2ManagerInstance)
    {
        WebView2ManagerInstance = new FWebView2Manager;
        WebView2ManagerInstance->Init();
    }
    return WebView2ManagerInstance;
}

TSharedPtr<FWebView2CompositionHost> FWebView2Manager::GetMessageProcess(HWND Handler)
{
    if (WebView2CompositionHostMap.Contains(Handler))
    {
        if (WebView2CompositionHostMap.Find(Handler)->Pin())
        {
            return WebView2CompositionHostMap.Find(Handler)->Pin();
        }
    }

    TSharedPtr<FWebView2CompositionHost> WebView2CompositionHost = MakeShared<FWebView2CompositionHost>(
        Handler, DispatcherQueueController);
    WebView2CompositionHost->Initialize();
    WebView2CompositionHostMap.Add(Handler, WebView2CompositionHost);
    return WebView2CompositionHost;
}

bool FWebView2Manager::HandleWindowMessage(HWND Handler, UINT Message, WPARAM WParam, LPARAM LParam)
{
    if (!Handler)
    {
        return false;
    }
    if (WebView2CompositionHostMap.Num() == 0)
    {
        return false;
    }

    for (TPair<HWND, TWeakPtr<FWebView2CompositionHost>>& WebView2CompositionHost : WebView2CompositionHostMap)
    {
        if (WebView2CompositionHost.Value.IsValid() && Handler == WebView2CompositionHost.Key)
        {
            WebView2CompositionHost.Value.Pin()->MouseMessage(Message, WParam, LParam);
        }
    }

    return false;
}

void FWebView2Manager::Init()
{
    UWebView2Settings* Settings = UWebView2Settings::Get();

    if (Settings->WebView2Mode != EWebView2Mode::VISUAL_WINCOMP)
    {
        return;
    }
    

    if (DispatcherQueueController != nullptr)
    {
        return;
    }
    
    HRESULT Hresult = E_FAIL;
    static decltype(::CreateDispatcherQueueController)* fnCreateDispatcherQueueController = nullptr;
    
    if (fnCreateDispatcherQueueController == nullptr)
    {
        const HMODULE Module = ::LoadLibraryEx(L"CoreMessaging.dll", nullptr, 0);
        if (Module != nullptr)
        {
            fnCreateDispatcherQueueController =
                reinterpret_cast<decltype(::CreateDispatcherQueueController)*>(
                    ::GetProcAddress(Module, "CreateDispatcherQueueController"));
        }
    }
    
    if (fnCreateDispatcherQueueController != nullptr)
    {
        WinSystem::DispatcherQueueController controller{nullptr};
        DispatcherQueueOptions options{
            sizeof(DispatcherQueueOptions), DQTYPE_THREAD_CURRENT, DQTAT_COM_STA
        };
        Hresult = fnCreateDispatcherQueueController(
            options, reinterpret_cast<ABI::Windows::System::IDispatcherQueueController**>(
                winrt::put_abi(controller)));
        DispatcherQueueController = controller;
    }

    if (!SUCCEEDED(Hresult))
    {
        UE_LOG(LogWebView2, Error, TEXT("Create with Windowless WinComp Visual Failed:"
                   "WinComp compositor creation failed."
                   "Current OS may not support WinComp."));
    }
}

void FWebView2Manager::Shutdown()
{
    for (TPair<HWND, TWeakPtr<FWebView2CompositionHost>>& It : WebView2CompositionHostMap)
    {
        if (It.Value.Pin())
        {
            It.Value.Pin()->DestroyWinCompVisualTree();
        }
    }

    WebView2CompositionHostMap.Empty();

    if (WebView2ManagerInstance)
    {
        delete WebView2ManagerInstance;
        WebView2ManagerInstance = nullptr;
    }
}
