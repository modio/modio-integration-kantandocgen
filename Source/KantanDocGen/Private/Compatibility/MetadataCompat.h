#pragma once

#include "Misc/EngineVersionComparison.h"
#include "UObject/MetaData.h"
#include "UObject/Package.h"
 
/// @brief Namespace containing helper methods to simplify MetaData code
namespace KantanDocGenMetadataEngineCompat
{
	FORCEINLINE static auto GetMetaData(UPackage* Package)
	{
#if UE_VERSION_OLDER_THAN(5, 6, 0)
		return Package->GetMetaData();
#else
		return &Package->GetMetaData();
#endif
	}

	FORCEINLINE static TMap<FName, FString>* GetMapForObject(const UObject* Object)
	{
#if UE_VERSION_OLDER_THAN(5, 6, 0)
		return UMetaData::GetMapForObject(Object);
#else
		return FMetaData::GetMapForObject(Object);
#endif
	}
} // namespace KantanDocGenMetadataEngineCompat
