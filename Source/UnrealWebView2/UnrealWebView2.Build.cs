using System.IO;
using UnrealBuildTool;

public class UnrealWebView2 : ModuleRules
{
    public UnrealWebView2(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDefinitions.Add("USING_COROUTINES=1");
        bEnableExceptions = true;
        bUseUnity = false;
        CppStandard = CppStandardVersion.Cpp20;

        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicSystemLibraries.AddRange(["Dcomp.lib"]);

        PublicDependencyModuleNames.AddRange(
            [
                "Core",
                "DeveloperSettings",
                "HTTP",
                "Json",             // 用于 FJsonObject 等核心 JSON 功能
                "JsonUtilities",    // 可选，用于更便捷的结构体/类序列化（推荐）
                "UMG",
                "WebView2",
                "WindowsImplementationLibrary"
            ]
        );


        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(
                ["UnrealEd"]
            );
        }
        
        if (!PublicDefinitions.Exists(s => s.Contains("WINVER=0x0A00")))
        {
            PublicDefinitions.Add("WINVER=0x0A00");
            PublicDefinitions.Add("_WIN32_WINNT=0x0A00");
        }

        PublicIncludePaths.Add(Path.Combine(Target.WindowsPlatform.WindowsSdkDir,
            "Include",
            Target.WindowsPlatform.WindowsSdkVersion,
            "cppwinrt"));


        PublicIncludePaths.Add(Path.Combine(Target.WindowsPlatform.WindowsSdkDir,
            "Include",
            Target.WindowsPlatform.WindowsSdkVersion,
            "winrt"));

        PrivateDependencyModuleNames.AddRange(
            [
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UMG",
                "Engine",
                "InputCore",
                "RHI",
                "Json",
                "Renderer",
                "RenderCore"
            ]
        );

        UndefinedIdentifierWarningLevel = WarningLevel.Off;
    }
}