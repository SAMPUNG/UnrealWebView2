#pragma once
#include "WebView2Window.h"

class UNREALWEBVIEW2_API FWebView2Manager
{
public:
    FWebView2Manager();

    void Closed(const HWND Handler);

    static TSharedPtr<FWebView2Window> CreateWebview(HWND InHandler, const FGuid UniqueID, const FString& URL,
                                              FColor InBackgroundColor, bool bDevTools);

    static FWebView2Manager* GetInstance();

    TSharedPtr<FWebView2CompositionHost> GetMessageProcess(HWND Handler);

    /*
     * 消息处理函数（Windows平台）
     */
    bool HandleWindowMessage(HWND Handler, UINT Message, WPARAM WParam, LPARAM LParam);

    void Init();

    /*
    * 释放资源
    */
    void Shutdown();

    winrt::Windows::System::DispatcherQueueController DispatcherQueueController{nullptr};

    TMap<HWND, TWeakPtr<FWebView2CompositionHost>> WebView2CompositionHostMap;

private:
    static FWebView2Manager* WebView2ManagerInstance;
};
