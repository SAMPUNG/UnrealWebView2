#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Windows/WindowsApplication.h"
#include "WebView2Subsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebMessageReceived, FString, Message);

DECLARE_DELEGATE_OneParam(FOnMessageReceivedNactive, const FString&)

class FWebviewWindowsMessageHandler : public IWindowsMessageHandler
{
public:
    virtual bool ProcessMessage(HWND Handler, uint32 Message, WPARAM WParam, LPARAM LParam, int32& OutResult) override;
};

UCLASS()
class UNREALWEBVIEW2_API UWebView2Subsystem : public UEngineSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    virtual void Deinitialize() override;

    virtual TStatId GetStatId() const override;
    static UWebView2Subsystem* GetSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    virtual bool IsTickable() const override;

    virtual void Tick(float DeltaTime) override;

    //判断鼠标是否在位置区域内的标志
    bool bIsMouseOverPositionArea = true;

    FOnMessageReceivedNactive OnMessageReceivedNative;
    
    UPROPERTY(BlueprintAssignable, Category = "WebView|Event")
    FOnWebMessageReceived OnWebMessageReceived;

private:
    static void OnEndPIE(bool bIsSimulating);

    /** 处理 GameInstance 启动的回调函数 */
    void OnGameInstanceStarted(UGameInstance* GameInstance);

    /** 存储 GameInstance 启动委托的句柄 */
    FDelegateHandle OnStartGameInstance;

    /** 注册消息处理器 */
    static void RegisterMessageHandler();
};
