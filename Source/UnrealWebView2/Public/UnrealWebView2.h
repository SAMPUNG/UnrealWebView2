#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FUnrealWebView2Module final : public IModuleInterface
{
public:
    virtual void ShutdownModule() override;
    virtual void StartupModule() override;

private:
    void* DllHandle = nullptr;
};
