#pragma once
#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "WebView2Message.generated.h"

UENUM(BlueprintType)
enum class EKeyboardKey : uint8
{
    Zero UMETA(DisplayName = "0"),
    One UMETA(DisplayName = "1"),
    Two UMETA(DisplayName = "2"),
    Three UMETA(DisplayName = "3"),
    Four UMETA(DisplayName = "4"),
    Five UMETA(DisplayName = "5"),
    Six UMETA(DisplayName = "6"),
    Seven UMETA(DisplayName = "7"),
    Eight UMETA(DisplayName = "8"),
    Nine UMETA(DisplayName = "9"),

    A UMETA(DisplayName = "A"),
    B UMETA(DisplayName = "B"),
    C UMETA(DisplayName = "C"),
    D UMETA(DisplayName = "D"),
    E UMETA(DisplayName = "E"),
    F UMETA(DisplayName = "F"),
    G UMETA(DisplayName = "G"),
    H UMETA(DisplayName = "H"),
    I UMETA(DisplayName = "I"),
    J UMETA(DisplayName = "J"),
    K UMETA(DisplayName = "K"),
    L UMETA(DisplayName = "L"),
    M UMETA(DisplayName = "M"),
    N UMETA(DisplayName = "N"),
    O UMETA(DisplayName = "O"),
    P UMETA(DisplayName = "P"),
    Q UMETA(DisplayName = "Q"),
    R UMETA(DisplayName = "R"),
    S UMETA(DisplayName = "S"),
    T UMETA(DisplayName = "T"),
    U UMETA(DisplayName = "U"),
    V UMETA(DisplayName = "V"),
    W UMETA(DisplayName = "W"),
    X UMETA(DisplayName = "X"),
    Y UMETA(DisplayName = "Y"),
    Z UMETA(DisplayName = "Z"),

    SpaceBar UMETA(DisplayName = "空格键"),
    Escape UMETA(DisplayName = "退出"),
    Left UMETA(DisplayName = "左"),
    Right UMETA(DisplayName = "右"),
    Up UMETA(DisplayName = "上"),
    Down UMETA(DisplayName = "下"),

    F1 UMETA(DisplayName = "F1"),
    F2 UMETA(DisplayName = "F2"),
    F3 UMETA(DisplayName = "F3"),
    F4 UMETA(DisplayName = "F4"),
    F5 UMETA(DisplayName = "F5"),
    F6 UMETA(DisplayName = "F6"),
    F7 UMETA(DisplayName = "F7"),
    F8 UMETA(DisplayName = "F8"),
    F9 UMETA(DisplayName = "F9"),
    F10 UMETA(DisplayName = "F10"),
    F11 UMETA(DisplayName = "F11"),
    F12 UMETA(DisplayName = "F12"),

    Undefined UMETA(DisplayName = "无效输入"),
};

UENUM(BlueprintType)
enum class EWebMessageType : uint8
{
    MouseCursor UMETA(DisplayName = "鼠标光标"),

    CameraMovement UMETA(DisplayName = "相机移动"),
    CameraReset UMETA(DisplayName = "相机复位"),

    Pressed UMETA(DisplayName = "键位摁下"),
    Released UMETA(DisplayName = "键位释放"),

    Undefined UMETA(DisplayName = "无效类型"),
};

UCLASS(BlueprintType)
class UNREALWEBVIEW2_API UWebMessageParser : public UObject
{
    GENERATED_BODY()

public:
    // 解析 Web 光标类型
    UFUNCTION(BlueprintCallable, Category = "WebView2")
    static EMouseCursor::Type ParseCursor(const FString& Type)
    {
        const FString Upper = Type.ToUpper();
        if (Upper == "POINTER")
        {
            return EMouseCursor::Hand;
        }
        if (Upper == "TEXT")
        {
            return EMouseCursor::TextEditBeam;
        }
        return EMouseCursor::Default;
    }

    // 解析键盘输入字符
    UFUNCTION(BlueprintCallable, Category = "WebView2")
    static EKeyboardKey ParseKey(const FString& Key)
    {
        // 空格条键位
        if (Key == " ")
        {
            return EKeyboardKey::SpaceBar;
        }
        // 方向键位
        if (Key == "ArrowDown")
        {
            return EKeyboardKey::Down;
        }
        if (Key == "ArrowLeft")
        {
            return EKeyboardKey::Left;
        }
        if (Key == "ArrowRight")
        {
            return EKeyboardKey::Right;
        }
        if (Key == "ArrowUp")
        {
            return EKeyboardKey::Up;
        }
        // 数字键位
        if (Key == "0")
        {
            return EKeyboardKey::Zero;
        }
        if (Key == "1")
        {
            return EKeyboardKey::One;
        }
        if (Key == "2")
        {
            return EKeyboardKey::Two;
        }
        if (Key == "3")
        {
            return EKeyboardKey::Three;
        }
        if (Key == "4")
        {
            return EKeyboardKey::Four;
        }
        if (Key == "5")
        {
            return EKeyboardKey::Five;
        }
        if (Key == "6")
        {
            return EKeyboardKey::Six;
        }
        if (Key == "7")
        {
            return EKeyboardKey::Seven;
        }
        if (Key == "8")
        {
            return EKeyboardKey::Eight;
        }
        if (Key == "9")
        {
            return EKeyboardKey::Nine;
        }
        // 英文字母键位全部使用大写字母来判断
        const FString Upper = Key.ToUpper();
        if (Upper == "A")
        {
            return EKeyboardKey::A;
        }
        if (Upper == "B")
        {
            return EKeyboardKey::B;
        }
        if (Upper == "C")
        {
            return EKeyboardKey::C;
        }
        if (Upper == "D")
        {
            return EKeyboardKey::D;
        }
        if (Upper == "E")
        {
            return EKeyboardKey::E;
        }
        if (Upper == "F")
        {
            return EKeyboardKey::F;
        }
        if (Upper == "G")
        {
            return EKeyboardKey::G;
        }
        if (Upper == "H")
        {
            return EKeyboardKey::H;
        }
        if (Upper == "I")
        {
            return EKeyboardKey::I;
        }
        if (Upper == "J")
        {
            return EKeyboardKey::J;
        }
        if (Upper == "K")
        {
            return EKeyboardKey::K;
        }
        if (Upper == "L")
        {
            return EKeyboardKey::L;
        }
        if (Upper == "M")
        {
            return EKeyboardKey::M;
        }
        if (Upper == "N")
        {
            return EKeyboardKey::N;
        }
        if (Upper == "O")
        {
            return EKeyboardKey::O;
        }
        if (Upper == "P")
        {
            return EKeyboardKey::P;
        }
        if (Upper == "Q")
        {
            return EKeyboardKey::Q;
        }
        if (Upper == "R")
        {
            return EKeyboardKey::R;
        }
        if (Upper == "S")
        {
            return EKeyboardKey::S;
        }
        if (Upper == "T")
        {
            return EKeyboardKey::T;
        }
        if (Upper == "U")
        {
            return EKeyboardKey::U;
        }
        if (Upper == "V")
        {
            return EKeyboardKey::V;
        }
        if (Upper == "W")
        {
            return EKeyboardKey::W;
        }
        if (Upper == "X")
        {
            return EKeyboardKey::X;
        }
        if (Upper == "Y")
        {
            return EKeyboardKey::Y;
        }
        if (Upper == "Z")
        {
            return EKeyboardKey::Z;
        }
        // 其他键位暂时不处理
        return EKeyboardKey::Undefined;
    }

    // 解析 Web 消息类型
    UFUNCTION(BlueprintCallable, Category = "WebView2")
    static EWebMessageType ParseType(const FString& Type)
    {
        const FString Upper = Type.ToUpper();
        if (Upper == "KEYDOWN")
        {
            return EWebMessageType::Pressed;
        }
        if (Upper == "KEYUP")
        {
            return EWebMessageType::Released;
        }
        if (Upper == "CURSOR")
        {
            return EWebMessageType::MouseCursor;
        }
        if (Upper == "CAMERAMOVE")
        {
            return EWebMessageType::CameraMovement;
        }
        if (Upper == "CAMERARESET")
        {
            return EWebMessageType::CameraReset;
        }
        return EWebMessageType::Undefined;
    }

    // URL 反转义
    UFUNCTION(BlueprintCallable, Category = "WebView2")
    static FString UnescapeUrl(const FString& EncodedUrl)
    {
        const FStringView UrlView(EncodedUrl);
        return FGenericPlatformHttp::UrlDecode(UrlView);
    }
};
