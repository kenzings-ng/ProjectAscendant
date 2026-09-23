// Copyright Project Ascendant. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ProjectAscendantTarget : TargetRules
{
	public ProjectAscendantTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "ProjectAscendant" });
	}
}
