// Copyright Epic Games, Inc. All Rights Reserved.


#include "MultipleDistanceTest.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Distance.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include <algorithm>


#define ENVQUERYTEST_DISTANCE_NAN_DETECTION 1

namespace
{
	FORCEINLINE float CalcDistance3D(const FVector& PosA, const FVector& PosB)
	{
		return (PosB - PosA).Size();
	}

	FORCEINLINE float CalcDistance2D(const FVector& PosA, const FVector& PosB)
	{
		return (PosB - PosA).Size2D();
	}

	FORCEINLINE float CalcDistanceZ(const FVector& PosA, const FVector& PosB)
	{
		return PosB.Z - PosA.Z;
	}

	FORCEINLINE float CalcDistanceAbsoluteZ(const FVector& PosA, const FVector& PosB)
	{
		return FMath::Abs(PosB.Z - PosA.Z);
	}

	FORCEINLINE void CheckItemLocationForNaN(const FVector& ItemLocation, UObject* QueryOwner, int32 Index, uint8 TestMode)
	{
#if ENVQUERYTEST_DISTANCE_NAN_DETECTION
		ensureMsgf(!ItemLocation.ContainsNaN(), TEXT("EnvQueryTest_Distance NaN in ItemLocation with owner %s. X=%f,Y=%f,Z=%f. Index:%d, TesMode:%d"), *GetPathNameSafe(QueryOwner), ItemLocation.X, ItemLocation.Y, ItemLocation.Z, Index, TestMode);
#endif
	}

	FORCEINLINE void CheckContextLocationForNaN(const FVector& ContextLocation, UObject* QueryOwner, int32 Index, uint8 TestMode)
	{
#if ENVQUERYTEST_DISTANCE_NAN_DETECTION
		ensureMsgf(!ContextLocation.ContainsNaN(), TEXT("EnvQueryTest_Distance NaN in ContextLocations with owner %s. X=%f,Y=%f,Z=%f. Index:%d, TesMode:%d"), *GetPathNameSafe(QueryOwner), ContextLocation.X, ContextLocation.Y, ContextLocation.Z, Index, TestMode);
#endif
	}
}

UMultipleDistanceTest::UMultipleDistanceTest(const FObjectInitializer& ObjectInitializer)
{
	DistanceTo = UEnvQueryContext_Querier::StaticClass();
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
}

void UMultipleDistanceTest::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr)
	{
		return;
	}

	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	float MinThresholdValue = FloatValueMin.GetValue();

	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);
	float MaxThresholdValue = FloatValueMax.GetValue();

	// don't support context Item here, it doesn't make any sense
	TArray<FVector> ContextLocations;
	if (!QueryInstance.PrepareContext(DistanceTo, ContextLocations))
	{
		return;
	}

	TArray<float> Values;

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		Values = TArray<float>();
		const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
		CheckItemLocationForNaN(ItemLocation, QueryOwner, It.GetIndex(), EEnvTestDistance::Distance3D);

		for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
		{
			CheckContextLocationForNaN(ContextLocations[ContextIndex], QueryOwner, ContextIndex, EEnvTestDistance::Distance3D);
			const float Distance = CalcDistance3D(ItemLocation, ContextLocations[ContextIndex]);
			Values.Add(Distance);
		}
		float score = *std::min_element(Values.GetData(), Values.GetData() + Values.Num());
		It.SetScore(TestPurpose, FilterType, score, MinThresholdValue, MaxThresholdValue);
	}
}

FText UMultipleDistanceTest::GetDescriptionTitle() const
{
	FString ModeDesc;
	ModeDesc = TEXT("");


	return FText::FromString(FString::Printf(TEXT("%s%s: to %s"),
		*Super::GetDescriptionTitle().ToString(), *ModeDesc,
		*UEnvQueryTypes::DescribeContext(DistanceTo).ToString()));
}

FText UMultipleDistanceTest::GetDescriptionDetails() const
{
	return DescribeFloatTestParams();
}
