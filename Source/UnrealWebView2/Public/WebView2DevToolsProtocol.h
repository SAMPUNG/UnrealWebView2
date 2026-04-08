#pragma once
#include "WebView2.h"

class FCdpCompletionHandler : public ICoreWebView2CallDevToolsProtocolMethodCompletedHandler
{
public:
    virtual ~FCdpCompletionHandler() = default;

    virtual ULONG STDMETHODCALLTYPE AddRef() override
    {
        return RefCount.fetch_add(1, std::memory_order_relaxed) + 1;
    }

    // ICoreWebView2CallDevToolsProtocolMethodCompletedHandler 方法
    virtual IFACEMETHODIMP Invoke(const HRESULT Result, const LPCWSTR ResultJson) override
    {
        if (SUCCEEDED(Result))
        {
            Response = std::wstring(ResultJson);
        }
        return S_OK;
    }


    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID Id, void** PpvObject) override
    {
        if (Id == IID_IUnknown || Id == __uuidof(ICoreWebView2CallDevToolsProtocolMethodCompletedHandler))
        {
            *PpvObject = static_cast<ICoreWebView2CallDevToolsProtocolMethodCompletedHandler*>(this);
            AddRef();
            return S_OK;
        }
        *PpvObject = nullptr;
        return E_NOINTERFACE;
    }

    virtual ULONG STDMETHODCALLTYPE Release() override
    {
        const ULONG NewCount = RefCount.fetch_sub(1, std::memory_order_release) - 1;
        if (NewCount == 0)
        {
            delete this;
        }
        return NewCount;
    }
    
    std::wstring Response;

private:
    std::atomic<long> RefCount{1};
};
