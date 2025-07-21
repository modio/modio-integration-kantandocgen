#pragma once

#include "UObject/MetaData.h"
#include "UObject/Package.h"

// Compatibility for UPackage::GetMetaData() returning FMetaData& instead of UMetaData* in UE 5.6+
#if UE_VERSION_OLDER_THAN(5, 6, 0)
	// In UE 5.5 and older, GetMetaData() returns a UMetaData*
	#define GET_METADATA_FROM_PACKAGE(Package) (Package->GetMetaData())
#else
	// In UE 5.6+, GetMetaData() returns a FMetaData&. We take its address to get a FMetaData*.
	#define GET_METADATA_FROM_PACKAGE(Package) (&(Package->GetMetaData()))
#endif

// Compatibility for UMetaData::GetMapForObject being renamed to FMetaData::GetMapForObject in UE 5.6+
#if UE_VERSION_OLDER_THAN(5, 6, 0)
	#define GET_METADATA_MAP_FOR_OBJECT(StructOrObject) UMetaData::GetMapForObject(StructOrObject)
#else
	#define GET_METADATA_MAP_FOR_OBJECT(StructOrObject) FMetaData::GetMapForObject(StructOrObject)
#endif