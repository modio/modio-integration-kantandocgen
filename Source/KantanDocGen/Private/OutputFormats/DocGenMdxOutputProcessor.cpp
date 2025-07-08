#include "OutputFormats/DocGenMdxOutputProcessor.h"
#include "Algo/Transform.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Json.h"
#include "JsonDomBuilder.h"
#include "KantanDocGenLog.h"
#include "Misc/FileHelper.h"
#include "Misc/Optional.h"
#include "Misc/Paths.h"

FString DocGenMdxOutputProcessor::Quote(const FString& In)
{
	if (In.TrimStartAndEnd().StartsWith("\""))
	{
		return In;
	}
	return "\"" + In.TrimStartAndEnd() + "\"";
}

TOptional<FString> DocGenMdxOutputProcessor::GetObjectStringField(const TSharedPtr<FJsonObject> Obj,
																  const FString& FieldName)
{
	FString FieldValue;
	if (!Obj->TryGetStringField(FieldName, FieldValue))
	{
		return {};
	}
	else
	{
		return FieldValue;
	}
}

TOptional<FString> DocGenMdxOutputProcessor::GetObjectStringField(const TSharedPtr<FJsonValue> Obj,
																  const FString& FieldName)
{
	const TSharedPtr<FJsonObject>* UnderlyingObject = nullptr;
	if (!Obj->TryGetObject(UnderlyingObject))
	{
		return {};
	}
	else
	{
		return GetObjectStringField(*UnderlyingObject, FieldName);
	}
}

TOptional<TArray<FString>> DocGenMdxOutputProcessor::GetNamesFromFileAtLocation(const FString& NameType,
																				const FString& ClassFile)
{
	TSharedPtr<FJsonObject> ParsedClass = LoadFileToJson(ClassFile);
	if (!ParsedClass)
	{
		return {};
	}

	if (ParsedClass->HasTypedField<EJson::Array>(NameType))
	{
		TArray<FString> NodeNames;
		for (const auto& Value : ParsedClass->GetArrayField(NameType))
		{
			TOptional<FString> FuncID = GetObjectStringField(Value, "id");
			if (FuncID.IsSet())
			{
				NodeNames.Add(FuncID.GetValue());
			}
		}
		return NodeNames;
	}
	else if (ParsedClass->HasTypedField<EJson::Object>(NameType))
	{
		TArray<FString> NodeNames;
		for (const auto& Node : ParsedClass->GetObjectField(NameType)->Values)
		{
			TOptional<FString> Name = GetObjectStringField(Node.Value, "id");
			if (Name.IsSet())
			{
				NodeNames.Add(Name.GetValue());
			}
		}
		return NodeNames;
	}
	else if (ParsedClass->HasTypedField<EJson::Null>(NameType))
	{
		return TArray<FString>();
	}
	return {};
}

TSharedPtr<FJsonObject> DocGenMdxOutputProcessor::ParseNodeFile(const FString& NodeFilePath)
{
	TSharedPtr<FJsonObject> ParsedNode = LoadFileToJson(NodeFilePath);
	if (!ParsedNode)
	{
		return {};
	}

	TSharedPtr<FJsonObject> OutNode = MakeShared<FJsonObject>();

	CopyJsonField("inputs", ParsedNode, OutNode);
	CopyJsonField("outputs", ParsedNode, OutNode);
	CopyJsonField("rawsignature", ParsedNode, OutNode);
	CopyJsonField("class_id", ParsedNode, OutNode);
	CopyJsonField("doxygen", ParsedNode, OutNode);
	CopyJsonField("imgpath", ParsedNode, OutNode);
	CopyJsonField("shorttitle", ParsedNode, OutNode);
	CopyJsonField("fulltitle", ParsedNode, OutNode);
	CopyJsonField("static", ParsedNode, OutNode);
	CopyJsonField("autocast", ParsedNode, OutNode);
	CopyJsonField("funcname", ParsedNode, OutNode);
	CopyJsonField("access_specifier", ParsedNode, OutNode);
	CopyJsonField("meta", ParsedNode, OutNode);
	return OutNode;
}

TSharedPtr<FJsonObject> DocGenMdxOutputProcessor::ParseClassFile(const FString& ClassFilePath)
{
	TSharedPtr<FJsonObject> ParsedClass = LoadFileToJson(ClassFilePath);
	if (!ParsedClass)
	{
		return {};
	}

	TSharedPtr<FJsonObject> OutNode = MakeShared<FJsonObject>();
	// Reusing the class template for now so renaming id to class_id to be consistent
	if (TSharedPtr<FJsonValue> Field = ParsedClass->TryGetField(TEXT("id")))
	{
		OutNode->SetField("class_id", Field);
	}

	CopyJsonField(TEXT("doxygen"), ParsedClass, OutNode);
	CopyJsonField(TEXT("display_name"), ParsedClass, OutNode);
	CopyJsonField(TEXT("fields"), ParsedClass, OutNode);
	CopyJsonField(TEXT("parent_class"), ParsedClass, OutNode);
	CopyJsonField(TEXT("meta"), ParsedClass, OutNode);
	CopyJsonField(TEXT("blueprint_generated"), ParsedClass, OutNode);
	CopyJsonField(TEXT("widget_blueprint"), ParsedClass, OutNode);
	CopyJsonField(TEXT("class_path"), ParsedClass, OutNode);
	CopyJsonField(TEXT("context_string"), ParsedClass, OutNode);
	return OutNode;
}

TSharedPtr<FJsonObject> DocGenMdxOutputProcessor::ParseStructFile(const FString& StructFilePath)
{
	TSharedPtr<FJsonObject> ParsedStruct = LoadFileToJson(StructFilePath);
	if (!ParsedStruct)
	{
		return {};
	}

	TSharedPtr<FJsonObject> OutNode = MakeShared<FJsonObject>();
	// Reusing the class template for now so renaming id to class_id to be consistent
	if (TSharedPtr<FJsonValue> Field = ParsedStruct->TryGetField(TEXT("id")))
	{
		OutNode->SetField(TEXT("class_id"), Field);
	}

	CopyJsonField(TEXT("doxygen"), ParsedStruct, OutNode);
	CopyJsonField(TEXT("display_name"), ParsedStruct, OutNode);
	CopyJsonField(TEXT("fields"), ParsedStruct, OutNode);
	CopyJsonField(TEXT("parent_class"), ParsedStruct, OutNode);
	CopyJsonField(TEXT("meta"), ParsedStruct, OutNode);
	CopyJsonField(TEXT("blueprint_generated"), ParsedStruct, OutNode);
	CopyJsonField(TEXT("widget_blueprint"), ParsedStruct, OutNode);
	CopyJsonField(TEXT("class_path"), ParsedStruct, OutNode);
	CopyJsonField(TEXT("context_string"), ParsedStruct, OutNode);
	return OutNode;
}

TSharedPtr<FJsonObject> DocGenMdxOutputProcessor::ParseEnumFile(const FString& EnumFilePath)
{
	TSharedPtr<FJsonObject> ParsedEnum = LoadFileToJson(EnumFilePath);
	if (!ParsedEnum)
	{
		return {};
	}

	TSharedPtr<FJsonObject> OutNode = MakeShared<FJsonObject>();

	CopyJsonField(TEXT("id"), ParsedEnum, OutNode);
	CopyJsonField(TEXT("doxygen"), ParsedEnum, OutNode);
	CopyJsonField(TEXT("display_name"), ParsedEnum, OutNode);
	CopyJsonField(TEXT("values"), ParsedEnum, OutNode);
	CopyJsonField(TEXT("meta"), ParsedEnum, OutNode);

	return OutNode;
}

void DocGenMdxOutputProcessor::CopyJsonField(const FString& FieldName, TSharedPtr<FJsonObject> ParsedNode,
											 TSharedPtr<FJsonObject> OutNode)
{
	if (TSharedPtr<FJsonValue> Field = ParsedNode->TryGetField(FieldName))
	{
		OutNode->SetField(FieldName, Field);
	}
}

TSharedPtr<FJsonObject> DocGenMdxOutputProcessor::InitializeMainOutputFromIndex(TSharedPtr<FJsonObject> ParsedIndex)
{
	TSharedPtr<FJsonObject> Output = MakeShared<FJsonObject>();

	CopyJsonField(TEXT("display_name"), ParsedIndex, Output);
	return Output;
}

EIntermediateProcessingResult DocGenMdxOutputProcessor::ConvertJsonToMdx(FString IntermediateDir)
{
	const FFilePath InJsonPath {IntermediateDir / TEXT("consolidated.json")};
	const FFilePath OutMdxPath {IntermediateDir / TEXT("docs.mdx")};

	const FString Format {TEXT("markdown")};
	void* PipeRead = nullptr;
	void* PipeWrite = nullptr;
	verify(FPlatformProcess::CreatePipe(PipeRead, PipeWrite));

	const FString Args = Quote(TemplatePath.FilePath) + " " + Quote(InJsonPath.FilePath) + " " +
						 Quote(OutMdxPath.FilePath) + " " + Quote(Format);

	FProcHandle Proc = FPlatformProcess::CreateProc(*(BinaryPath.Path / TEXT("convert.exe")), *Args, true, false, false,
													nullptr, 0, nullptr, PipeWrite);
	if (!Proc.IsValid())
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("Failed to create process %s"), *(BinaryPath.Path / TEXT("convert.exe")));
		return EIntermediateProcessingResult::UnknownError;
	}

	FString BufferedText;
	int32 ReturnCode = 0;
	for (bool bProcessFinished = false; !bProcessFinished;)
	{
		bProcessFinished = FPlatformProcess::GetProcReturnCode(Proc, &ReturnCode);
		BufferedText += FPlatformProcess::ReadPipe(PipeRead);
		int32 EndOfLineIdx;
		while (BufferedText.FindChar('\n', EndOfLineIdx))
		{
			FString Line = BufferedText.Left(EndOfLineIdx);
			Line.RemoveFromEnd(TEXT("\r"));

			UE_LOG(LogKantanDocGen, Error, TEXT("[KantanDocGen] %s"), *Line);

			BufferedText = BufferedText.Mid(EndOfLineIdx + 1);
		}

		FPlatformProcess::Sleep(0.1f);
	}
	FPlatformProcess::CloseProc(Proc);
	Proc.Reset();
	if (ReturnCode != 0)
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("KantanDocGen tool failed (code %i), see above output."), ReturnCode);
		return EIntermediateProcessingResult::UnknownError;
	}

	// create docusaurus staging directory
	const FString DocusaurusStagingPath {IntermediateDir / TEXT("docusaurus")};
	if (!FPlatformFileManager::Get().GetPlatformFile().CopyDirectoryTree(*DocusaurusStagingPath, *DocusaurusPath.Path,
																		 true))
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("Failed to copy template docusaurus %s to intermediate directory %s"),
			   *DocusaurusPath.Path, *DocusaurusStagingPath);
		return EIntermediateProcessingResult::UnknownError;
	}

	// merge doc_root into staging directory
	if (!FPlatformFileManager::Get().GetPlatformFile().CopyDirectoryTree(*(DocusaurusStagingPath / "public"),
																		 *(DocRootPath.Path), false))
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("Failed to merge doc_root %s into docusaurus staging directory %s"),
			   *(DocRootPath.Path), *(DocusaurusStagingPath / TEXT("public")));
		return EIntermediateProcessingResult::UnknownError;
	}

	// copy the newly generated mdx and img files into staging directory
	const FFilePath MdxDestinationPath {DocusaurusStagingPath / TEXT("public/en-us/refdocs.mdx")};
	const FDirectoryPath ImgDestinationPath {DocusaurusStagingPath / TEXT("public/en-us/img/refdocs")};
	if (IFileManager::Get().Copy(*MdxDestinationPath.FilePath, *OutMdxPath.FilePath) != 0)
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("Failed to copy generated file %s to %s"), *OutMdxPath.FilePath,
			   *MdxDestinationPath.FilePath);
		return EIntermediateProcessingResult::UnknownError;
	}
	TArray<FString> ImgDirectories;
	IFileManager::Get().FindFilesRecursive(ImgDirectories, *IntermediateDir, TEXT("img"), false, true);
	for (FString ImgDirectory : ImgDirectories)
	{
		if (ImgDirectory.Contains(DocusaurusStagingPath))
		{
			continue;
		}
		TArray<FString> ImageFiles;
		IFileManager::Get().FindFiles(ImageFiles, *ImgDirectory, TEXT("png"));
		for (FString Image : ImageFiles)
		{
			// if this limit is adjusted, all <plugin>/Doc/template/function.mdx.in files must be updated to match
			if (Image.Len() > 140)
			{
				UE_LOG(LogKantanDocGen, Warning,
					   TEXT("Skipping image with filename length %d to avoid windows path length restrictions: %s"),
					   Image.Len(), *Image);
				continue;
			}
			FString SourceImagePath {ImgDirectory / Image};
			FString DestinationImagePath {ImgDestinationPath.Path / Image};
			if (IFileManager::Get().Copy(*DestinationImagePath, *SourceImagePath) != 0)
			{
				UE_LOG(LogKantanDocGen, Error, TEXT("Failed to copy %s to %s"), *SourceImagePath,
					   *DestinationImagePath);
				return EIntermediateProcessingResult::UnknownError;
			}
		}
	}
	return EIntermediateProcessingResult::Success;
}

EIntermediateProcessingResult DocGenMdxOutputProcessor::ConvertMdxToHtml(FString IntermediateDir, FString OutputDir)
{
	const FString DocusaurusStagingPath {IntermediateDir / TEXT("docusaurus")};

	// invoke npm install to install required packages
	if (const EIntermediateProcessingResult ReturnCode = RunNPMCommand(TEXT("install"), DocusaurusStagingPath);
		ReturnCode != EIntermediateProcessingResult::Success)
	{
		return ReturnCode;
	}

	// invoke npm run build to build the html docs
	if (const EIntermediateProcessingResult ReturnCode = RunNPMCommand(TEXT("run build"), DocusaurusStagingPath);
		ReturnCode != EIntermediateProcessingResult::Success)
	{
		return ReturnCode;
	}

	// copy result from intermediate directory to output directory
	if (!FPlatformFileManager::Get().GetPlatformFile().CopyDirectoryTree(
			*OutputDir, *(DocusaurusStagingPath / TEXT("build")), true))
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("Failed to copy build docs %s to output directory %s"),
			   *(DocusaurusStagingPath / TEXT("build")), *OutputDir);
		return EIntermediateProcessingResult::UnknownError;
	}
	return EIntermediateProcessingResult::Success;
}

EIntermediateProcessingResult DocGenMdxOutputProcessor::RunNPMCommand(const FString& Command,
																	  const FString& PackageJsonPath) const
{
	FString NPMDirectory = FPaths::GetPath(NpmExecutablePath.FilePath);
	FString NodeExe = FString::Printf(TEXT("%s\\node.exe"), *NPMDirectory);
	FString EscapedNPMDirectory = NPMDirectory.ReplaceCharWithEscapedChar();
	// We run this JS code to simulate npm.cmd, but allow us to read the output and catch errors
	FString JsCode = FString::Printf(TEXT("try { "
										  "  process.env['PATH'] += path.delimiter + '%s';"
										  "  const o = require('child_process').execSync('%s\\\\npm.cmd %s'); "
										  "  console.log(o.toString()); "
										  "} catch (e) { "
										  "  console.error(e.stdout?.toString() || e.message); "
										  "  process.exit(1); "
										  "}"),
									 *EscapedNPMDirectory, *EscapedNPMDirectory, *Command);
	FString Args = FString::Printf(TEXT("-e \"%s\""), *JsCode);

	void* ReadPipe = nullptr;
	void* WritePipe = nullptr;
	FPlatformProcess::CreatePipe(ReadPipe, WritePipe);

	FProcHandle ProcessHandle = FPlatformProcess::CreateProc(*NodeExe, *Args,
															 true, // bLaunchDetached
															 false, // bLaunchHidden
															 false, // bLaunchReallyHidden
															 nullptr, 0, *PackageJsonPath, WritePipe);

	if (!ProcessHandle.IsValid())
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("npm command [%s] failed for %s"), *Command, *(NpmExecutablePath.FilePath));
		return EIntermediateProcessingResult::UnknownError;
	}

	FString Output;
	while (FPlatformProcess::IsProcRunning(ProcessHandle))
	{
		Output += FPlatformProcess::ReadPipe(ReadPipe);
		FPlatformProcess::Sleep(0.1f);
	}
	// final flush
	Output += FPlatformProcess::ReadPipe(ReadPipe);

	int32 ExitCode;
	FPlatformProcess::GetProcReturnCode(ProcessHandle, &ExitCode);
	FPlatformProcess::ClosePipe(ReadPipe, WritePipe);

	if (ExitCode != 0)
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("npm command [%s] error"), *Command);
		UE_LOG(LogKantanDocGen, Error, TEXT("npm exited with code %d"), ExitCode);
		UE_LOG(LogKantanDocGen, Error, TEXT("npm output:\n%s"), *Output);

		return EIntermediateProcessingResult::UnknownError;
	}

	UE_LOG(LogKantanDocGen, Log, TEXT("npm command [%s] was a success"), *Command);
	return EIntermediateProcessingResult::Success;
}

DocGenMdxOutputProcessor::DocGenMdxOutputProcessor(TOptional<FFilePath> TemplatePathOverride,
												   TOptional<FDirectoryPath> BinaryPathOverride,
												   TOptional<FFilePath> NpmPathOverride,
												   TOptional<FDirectoryPath> DocRootPathOverride,
												   TOptional<FDirectoryPath> DocusaurusPathOverride)
{
	if (BinaryPathOverride.IsSet())
	{
		BinaryPath = BinaryPathOverride.GetValue();
	}
	else
	{
		BinaryPath.Path = FString(TEXT("bin"));
	}
	if (TemplatePathOverride.IsSet())
	{
		TemplatePath = TemplatePathOverride.GetValue();
	}
	else
	{
		TemplatePath.FilePath = BinaryPath.Path / TEXT("template") / TEXT("docs.mdx.in");
	}
	if (NpmPathOverride.IsSet())
	{
		NpmExecutablePath = NpmPathOverride.GetValue();
	}
	else
	{
		NpmExecutablePath.FilePath = TEXT("C:/Program Files/nodejs/npm.cmd");
	}
	if (DocRootPathOverride.IsSet())
	{
		DocRootPath = DocRootPathOverride.GetValue();
	}
	else
	{
		DocRootPath.Path = BinaryPath.Path / TEXT("doc_root");
	}
	if (DocusaurusPathOverride.IsSet())
	{
		DocusaurusPath = DocusaurusPathOverride.GetValue();
	}
	else
	{
		DocusaurusPath.Path = FPaths::ProjectDir() / TEXT("Tools/docusaurus");
	}
}

EIntermediateProcessingResult DocGenMdxOutputProcessor::ProcessIntermediateDocs(FString const& IntermediateDir,
																				FString const& OutputDir,
																				FString const& DocTitle,
																				bool bCleanOutput)
{
	TSharedPtr<FJsonObject> ParsedIndex = LoadFileToJson(IntermediateDir / TEXT("index.json"));

	TSharedPtr<FJsonObject> ConsolidatedOutput = InitializeMainOutputFromIndex(ParsedIndex);

	EIntermediateProcessingResult ClassResult =
		ConsolidateClasses(ParsedIndex, IntermediateDir, OutputDir, ConsolidatedOutput);
	if (ClassResult != EIntermediateProcessingResult::Success)
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("Failed to consolidate classes"));
		return ClassResult;
	}

	EIntermediateProcessingResult StructResult =
		ConsolidateStructs(ParsedIndex, IntermediateDir, OutputDir, ConsolidatedOutput);
	if (StructResult != EIntermediateProcessingResult::Success)
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("Failed to consolidate structs"));
		return StructResult;
	}

	EIntermediateProcessingResult EnumResult =
		ConsolidateEnums(ParsedIndex, IntermediateDir, OutputDir, ConsolidatedOutput);
	if (EnumResult != EIntermediateProcessingResult::Success)
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("Failed to consolidate Enums"));
		return EnumResult;
	}

	FString Result;
	auto JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Result);
	FJsonSerializer::Serialize(ConsolidatedOutput.ToSharedRef(), JsonWriter);

	if (!FFileHelper::SaveStringToFile(Result, *(IntermediateDir / TEXT("consolidated.json")),
									   FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		return EIntermediateProcessingResult::DiskWriteFailure;
	}
	// create mdx and image files, and copy them to doc_root
	if (ConvertJsonToMdx(IntermediateDir) == EIntermediateProcessingResult::Success)
	{
		// invoke npm to convert mdx to html, and copy results to specified output directory
		return ConvertMdxToHtml(IntermediateDir, OutputDir);
	}

	UE_LOG(LogKantanDocGen, Error, TEXT("Failed to consolidate due to unknown error"));
	return EIntermediateProcessingResult::UnknownError;
}

EIntermediateProcessingResult DocGenMdxOutputProcessor::ConsolidateClasses(TSharedPtr<FJsonObject> ParsedIndex,
																		   FString const& IntermediateDir,
																		   FString const& OutputDir,
																		   TSharedPtr<FJsonObject> ConsolidatedOutput)
{
	FJsonDomBuilder::FArray StaticFunctionList;
	FJsonDomBuilder::FObject ClassFunctionList;
	TOptional<TArray<FString>> ClassNames = GetNamesFromIndexFile(TEXT("classes"), TEXT("class"), ParsedIndex);
	if (!ClassNames.IsSet())
	{
		return EIntermediateProcessingResult::UnknownError;
	}

	for (const auto& ClassName : ClassNames.GetValue())
	{
		const FString ClassFilePath = IntermediateDir / ClassName / ClassName + TEXT(".json");
		TOptional<TArray<FString>> NodeNames = GetNamesFromFileAtLocation(TEXT("nodes"), ClassFilePath);
		TSharedPtr<FJsonObject> ParsedClass = ParseStructFile(ClassFilePath);
		if (!NodeNames.IsSet())
		{
			return EIntermediateProcessingResult::UnknownError;
		}
		else
		{
			TOptional<FString> NodeClassID;
			FJsonDomBuilder::FArray Nodes;
			for (const auto& NodeName : NodeNames.GetValue())
			{
				const FString NodeFilePath = IntermediateDir / ClassName / TEXT("nodes") / NodeName + TEXT(".json");

				if (TSharedPtr<FJsonObject> NodeJson = ParseNodeFile(NodeFilePath))
				{
					FString RelImagePath;
					if (NodeJson->TryGetStringField(TEXT("imgpath"), RelImagePath))
					{
						FString SourceImagePath = IntermediateDir / ClassName / TEXT("nodes") / RelImagePath;
						SourceImagePath =
							IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*SourceImagePath);
						IFileManager::Get().Copy(*(OutputDir / TEXT("img") / FPaths::GetCleanFilename(RelImagePath)),
												 *SourceImagePath, true);
					}
					bool FunctionIsStatic = false;
					NodeJson->TryGetBoolField(TEXT("static"), FunctionIsStatic);

					if (FunctionIsStatic)
					{
						StaticFunctionList.Add(MakeShared<FJsonValueObject>(NodeJson));
					}
					else
					{
						Nodes.Add(MakeShared<FJsonValueObject>(NodeJson));
					}
				}
				else
				{
					return EIntermediateProcessingResult::UnknownError;
				}
			}
			// We don't want classes in our classlist if all their nodes are static
			FJsonDomBuilder::FObject ClassObj;
			ClassObj.Set(TEXT("functions"), Nodes);
			ClassObj.Set(TEXT("class_id"), ClassName);
			ClassObj.Set(TEXT("display_name"), ParsedClass->GetStringField(TEXT("display_name")));
			ClassObj.Set(TEXT("meta"), MakeShared<FJsonValueObject>(ParsedClass->GetObjectField(TEXT("meta"))));
			ClassObj.Set(TEXT("parent_class"),
						 MakeShared<FJsonValueObject>(ParsedClass->GetObjectField(TEXT("parent_class"))));
			const TSharedPtr<FJsonObject>* DoxygenBlock;
			bool bHadDoxygenBlock = ParsedClass->TryGetObjectField(TEXT("doxygen"), DoxygenBlock);
			if (bHadDoxygenBlock)
			{
				ClassObj.Set(TEXT("doxygen"), MakeShared<FJsonValueObject>(*DoxygenBlock));
			}
			const TArray<TSharedPtr<FJsonValue>>* FieldArray;
			bool bHadFields = ParsedClass->TryGetArrayField(TEXT("fields"), FieldArray);
			ClassObj.Set(TEXT("fields"), bHadFields ? MakeShared<FJsonValueArray>(*FieldArray)
													: MakeShared<FJsonValueArray>(TArray<TSharedPtr<FJsonValue>> {}));

			ClassFunctionList.Set(ClassName, ClassObj);
		}
	}

	ConsolidatedOutput->SetField(TEXT("functions"), StaticFunctionList.AsJsonValue());
	ConsolidatedOutput->SetField(TEXT("classes"), ClassFunctionList.AsJsonValue());
	return EIntermediateProcessingResult::Success;
}

EIntermediateProcessingResult DocGenMdxOutputProcessor::ConsolidateStructs(TSharedPtr<FJsonObject> ParsedIndex,
																		   FString const& IntermediateDir,
																		   FString const& OutputDir,
																		   TSharedPtr<FJsonObject> ConsolidatedOutput)
{
	FJsonDomBuilder::FArray StructList;

	TOptional<TArray<FString>> StructNames = GetNamesFromIndexFile(TEXT("structs"), TEXT("struct"), ParsedIndex);
	if (!StructNames.IsSet())
	{
		return EIntermediateProcessingResult::UnknownError;
	}

	for (const auto& StructName : StructNames.GetValue())
	{
		const FString StructFilePath = IntermediateDir / StructName / StructName + TEXT(".json");
		TSharedPtr<FJsonObject> StructJson = ParseStructFile(StructFilePath);
		StructList.Add(MakeShared<FJsonValueObject>(StructJson));
	}

	ConsolidatedOutput->SetField(TEXT("structs"), StructList.AsJsonValue());
	return EIntermediateProcessingResult::Success;
}

EIntermediateProcessingResult DocGenMdxOutputProcessor::ConsolidateEnums(TSharedPtr<FJsonObject> ParsedIndex,
																		 FString const& IntermediateDir,
																		 FString const& OutputDir,
																		 TSharedPtr<FJsonObject> ConsolidatedOutput)
{
	FJsonDomBuilder::FArray EnumList;

	TOptional<TArray<FString>> EnumNames = GetNamesFromIndexFile(TEXT("enums"), TEXT("enum"), ParsedIndex);
	if (!EnumNames.IsSet())
	{
		UE_LOG(LogKantanDocGen, Error, TEXT("Failed to get enum names from index file"));
		return EIntermediateProcessingResult::UnknownError;
	}

	for (const auto& EnumName : EnumNames.GetValue())
	{
		UE_LOG(LogKantanDocGen, Log, TEXT("Processing enum: %s"), *EnumName);
		const FString EnumFilePath = IntermediateDir / EnumName / EnumName + TEXT(".json");
		TSharedPtr<FJsonObject> EnumJson = ParseEnumFile(EnumFilePath);
		EnumList.Add(MakeShared<FJsonValueObject>(EnumJson));
	}

	ConsolidatedOutput->SetField(TEXT("enums"), EnumList.AsJsonValue());
	return EIntermediateProcessingResult::Success;
}

TOptional<TArray<FString>> DocGenMdxOutputProcessor::GetNamesFromIndexFile(const FString& NameType,
																		   const FString& ChildNameType,
																		   TSharedPtr<FJsonObject> ParsedIndex)
{
	if (!ParsedIndex)
	{
		return {};
	}
	TArray<FString> ClassJsonFiles;
	TArray<TSharedPtr<FJsonValue>> Entries;

	const TArray<TSharedPtr<FJsonValue>>* ArrayEntries = nullptr;
	if (ParsedIndex->TryGetArrayField(NameType, ArrayEntries))
	{
		Entries = *ArrayEntries;
	}
	// If there was only a single entry, it won't be added as an array and our code generator will fail later on.
	else
	{
		const TSharedPtr<FJsonObject>* ArrayObjectEntry = nullptr;
		if (ParsedIndex->TryGetObjectField(NameType, ArrayObjectEntry))
		{
			const TSharedPtr<FJsonObject>* ObjectEntry = nullptr;
			// This is a special case where we're only storing a single object in the index
			if ((*ArrayObjectEntry)->TryGetObjectField(ChildNameType, ObjectEntry))
			{
				// Wrap the object in a JsonValue for uniform processing
				Entries.Add(MakeShared<FJsonValueObject>(*ObjectEntry));
			}
		}
	}

	for (const auto& Entry : Entries)
	{
		TOptional<FString> EnumID = GetObjectStringField(Entry, TEXT("id"));
		if (EnumID.IsSet())
		{
			UE_LOG(LogKantanDocGen, Log, TEXT("Processing entry: %s"), *EnumID.GetValue());
			ClassJsonFiles.Add(EnumID.GetValue());
		}
		else
		{
			UE_LOG(LogKantanDocGen, Error, TEXT("Entry missing 'id' field: %s"), *Entry->AsString());
		}
	}

	if (ClassJsonFiles.Num())
	{
		return ClassJsonFiles;
	}
	else
	{
		return {};
	}
}

TSharedPtr<FJsonObject> DocGenMdxOutputProcessor::LoadFileToJson(FString const& FilePath)
{
	FString IndexFileString;
	if (!FFileHelper::LoadFileToString(IndexFileString, &FPlatformFileManager::Get().GetPlatformFile(), *FilePath))
	{
		return nullptr;
	}

	TSharedPtr<FJsonStringReader> TopLevelJson = FJsonStringReader::Create(IndexFileString);
	TSharedPtr<FJsonObject> ParsedFile;
	if (!FJsonSerializer::Deserialize<TCHAR>(*TopLevelJson, ParsedFile, FJsonSerializer::EFlags::None))
	{
		return nullptr;
	}
	else
	{
		return ParsedFile;
	}
}
