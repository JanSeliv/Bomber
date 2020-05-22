// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class RD : ModuleRules
{
	public RD(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		bUseRTTI = true;
		
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
            // Add the import library
            PublicDefinitions.Add("_WIN32");
            PublicDefinitions.Add("_WINSOCK_DEPRECATED_NO_WARNINGS");

            var libFolder = Path.Combine(ModuleDirectory, "libs", "Win", "x64", "Release");
            string[] libs = new string[]
            {
	            "rd_framework_cpp.lib",
	            "clsocket.lib",
	            "rd_core_cpp.lib"
            };
            foreach (string lib in libs)
            {
	            PublicAdditionalLibraries.Add(Path.Combine(libFolder, lib));
            }

            string[] paths = new string[] {
	            "include",
	            "include/rd_core_cpp",
	            "include/rd_framework_cpp",
	            "include/thirdparty",
	            "include/rd_core_cpp/lifetime",
	            "include/rd_core_cpp/logger",
	            "include/rd_core_cpp/reactive",
	            "include/rd_core_cpp/std",
	            "include/rd_core_cpp/types",
	            "include/rd_core_cpp/util",
	            "include/rd_core_cpp/reactive/base",
	            "include/rd_framework_cpp/base",
	            "include/rd_framework_cpp/ext",
	            "include/rd_framework_cpp/impl",
	            "include/rd_framework_cpp/intern",
	            "include/rd_framework_cpp/scheduler",
	            "include/rd_framework_cpp/serialization",
	            "include/rd_framework_cpp/task",
	            "include/rd_framework_cpp/util",
	            "include/rd_framework_cpp/wire",
	            "include/rd_framework_cpp/scheduler/base",
	            "include/thirdparty/clsocket",
	            "include/thirdparty/mpark",
	            "include/thirdparty/nonstd",
	            "include/thirdparty/optional",
	            "include/thirdparty/tsl",
	            "include/thirdparty/clsocket/src",
	            "include/thirdparty/optional/tl"
            };

			foreach(var item in paths)
            {
                PublicIncludePaths.Add(Path.Combine(ModuleDirectory, item));
            }
		}
	}
}
