// Copyright Project Ascendant. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ProjectAscendantEditorTarget : TargetRules
{
	public ProjectAscendantEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "ProjectAscendant" });
	}
}
