using System.IO;
using UnrealBuildTool;

public class WindowsImplementationLibrary : ModuleRules
{
    
	public WindowsImplementationLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
		}
	}
}