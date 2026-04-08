#include "UnrealWebView2.h"
#include "WebView2Log.h"

#define LOCTEXT_NAMESPACE "FUnrealWebView2Module"

DEFINE_LOG_CATEGORY(LogWebView2);

void FUnrealWebView2Module::ShutdownModule()
{
    if (DllHandle)
    {
        FPlatformProcess::FreeDllHandle(DllHandle);
    }
}

void FUnrealWebView2Module::StartupModule()
{
    const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries/Win64/WebView2Loader.dll"));
    if (const void* Handle = FPlatformProcess::GetDllHandle(*Path); !Handle)
    {
        const int32 ErrorNum = FPlatformMisc::GetLastError();
        TCHAR ErrorMsg[1024];
        FPlatformMisc::GetSystemErrorMessage(ErrorMsg, 1024, ErrorNum);
        UE_LOG(LogWebView2, Error, TEXT("Failed to get WebView2Loader.dll handle for %s: %s (%d)"), *Path, ErrorMsg, ErrorNum);
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUnrealWebView2Module, UnrealWebView2);
