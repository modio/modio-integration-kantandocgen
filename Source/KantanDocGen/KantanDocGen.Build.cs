/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
// Copyright (C) 2016-2017 Cameron Angus. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class KantanDocGen : ModuleRules
{
	public KantanDocGen(ReadOnlyTargetRules Target): base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
#if UE_5_2_OR_LATER 
		IWYUSupport = IWYUSupport.Full;
#else
		bEnforceIWYU = true;
#endif
		bUseUnity = false;

		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "../../ThirdParty/variant/include"));
		PublicDependencyModuleNames.AddRange(
            new string[] {
	            "AnimGraph",
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "Slate",
				"SlateCore",
                "UnrealEd",
                "PropertyEditor",
                "EditorStyle",
				"BlueprintGraph",
				"GraphEditor",
				"MainFrame",
				"LevelEditor",
				"XmlParser",
				"Json",
				"JsonUtilities",
				"UMG",
				"Projects",
                "ImageWriteQueue",
				"RenderCore",
				"SlateRHIRenderer",
				"Settings",
				"AssetRegistry",
				"UMGEditor"
            }
        );
	}
}
