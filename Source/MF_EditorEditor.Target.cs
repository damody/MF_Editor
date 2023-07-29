// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MF_EditorEditorTarget : TargetRules
{
	public MF_EditorEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		//DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "MF_Editor" } );
	}
}
