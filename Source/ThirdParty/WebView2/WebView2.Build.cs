using System.IO;
using UnrealBuildTool;

public class WebView2 : ModuleRules
{
    
	public WebView2(ReadOnlyTargetRules Target) : base(Target)
	{
		string TargetPath = "$(ProjectDir)/Binaries/Win64/";
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
			string LibPath = Path.Combine(ModuleDirectory, "lib");
            
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "WebView2Loader.dll.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "WebView2LoaderStatic.lib"));
		}

		string SrcWebView2LoaderDll = Path.Combine(ModuleDirectory, "bin", "WebView2Loader.dll");
		string DstWebView2LoaderDll = Path.Combine(TargetPath, "WebView2Loader.dll");
		if (!File.Exists(DstWebView2LoaderDll))
		{
			if(!Directory.Exists(TargetPath)) Directory.CreateDirectory(TargetPath);
			File.Copy(SrcWebView2LoaderDll,DstWebView2LoaderDll);
		}

		RuntimeDependencies.Add(DstWebView2LoaderDll, SrcWebView2LoaderDll);
	}
}