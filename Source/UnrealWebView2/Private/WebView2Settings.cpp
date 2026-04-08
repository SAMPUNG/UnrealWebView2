#include "WebView2Settings.h"

UWebView2Settings::UWebView2Settings(const FObjectInitializer& ObjectInitializer)
    :bMuted(false)
    ,WebView2Mode(EWebView2Mode::VISUAL_WINCOMP)
{
}

UWebView2Settings* UWebView2Settings::Get()
{
    UWebView2Settings* Settings = GetMutableDefault<UWebView2Settings>();
    return Settings;
}

FName UWebView2Settings::GetCategoryName() const
{
    return TEXT("WebView2");
}
