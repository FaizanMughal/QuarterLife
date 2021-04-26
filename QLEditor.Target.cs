// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class QLEditorTarget : TargetRules
{
    public QLEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;

        bLegacyPublicIncludePaths = false;

        ShadowVariableWarningLevel = WarningLevel.Error;

        ExtraModuleNames.AddRange( new string[] { "QL" } );
    }
}
