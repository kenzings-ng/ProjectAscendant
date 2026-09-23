// Copyright Project Ascendant. All Rights Reserved.

#include "Network/PAIrisSpatialFilter.h"

EPAIrisSpatialTier UPAIrisSpatialFilter::EvaluateSpatialTier(
	const FVector& ObserverLocation,
	const FVector& TargetLocation,
	const FPAIrisSpatialConfig& Config)
{
	const float Dist = FVector::Dist(ObserverLocation, TargetLocation);

	if (Dist <= Config.Tier1Radius)
	{
		return EPAIrisSpatialTier::Tier1_HighFrequency;
	}

	if (Dist <= Config.Tier2Radius)
	{
		return EPAIrisSpatialTier::Tier2_MidFrequency;
	}

	return EPAIrisSpatialTier::Tier3_Dormant;
}

float UPAIrisSpatialFilter::GetReplicationFrequencyForDistance(
	float DistanceCm,
	const FPAIrisSpatialConfig& Config)
{
	if (DistanceCm <= Config.Tier1Radius)
	{
		return Config.Tier1Frequency; // 60Hz
	}

	if (DistanceCm <= Config.Tier2Radius)
	{
		return Config.Tier2Frequency; // 30Hz
	}

	return Config.Tier3Frequency; // 0Hz (Dormant)
}

bool UPAIrisSpatialFilter::ShouldCullReplication(
	float DistanceCm,
	const FPAIrisSpatialConfig& Config)
{
	// Vượt quá 3500 cm (35m) thì loại bỏ khỏi luồng đồng bộ cao tần
	return DistanceCm > Config.Tier2Radius;
}

float UPAIrisSpatialFilter::CalculateReplicationInterval(float FrequencyHz)
{
	if (FrequencyHz > 0.0001f)
	{
		return 1.0f / FrequencyHz;
	}

	return 999999.0f; // Vô hạn (không gửi gói định kỳ)
}
