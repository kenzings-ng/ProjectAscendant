// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "World/PAZoneTypes.h"
#include "PACampfireComponent.generated.h"

/**
 * UPACampfireComponent
 *
 * ActorComponent quản lý Đống Lửa (Campfire): vùng Thánh địa an toàn bán kính 1000cm,
 * xóa cừu hận, cấm PvP, hồi phục 100% tài nguyên và điểm mốc Dịch chuyển nhanh (Fast Travel).
 *
 * Tham chiếu GDD: design/gdd/zone-system.md §Campfire & Sanctuary System
 * ADR-0001: Server-authoritative logic
 */
UCLASS(ClassGroup = (World), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPACampfireComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPACampfireComponent();

	virtual void BeginPlay() override;

	// ===========================================================
	// Public API
	// ===========================================================

	UFUNCTION(BlueprintCallable, Category = "Zone|Campfire")
	void InitializeCampfire(FName InId, EPAZoneTier InTier, bool bInitiallyLit = false);

	UFUNCTION(BlueprintCallable, Category = "Zone|Campfire")
	bool LightCampfire();

	/**
	 * Người chơi ngồi nghỉ tại đống lửa.
	 * @return true nếu hồi phục thành công (HP, MP, Stamina 100%, đầy Flask).
	 */
	UFUNCTION(BlueprintCallable, Category = "Zone|Campfire")
	bool RestAtCampfire(float& OutHP, float& OutMP, float& OutStamina, int32& OutFlasks);

	UFUNCTION(BlueprintPure, Category = "Zone|Campfire")
	bool IsLit() const { return CampfireData.bIsLit; }

	UFUNCTION(BlueprintPure, Category = "Zone|Campfire")
	FName GetCampfireId() const { return CampfireData.CampfireId; }

	UFUNCTION(BlueprintPure, Category = "Zone|Campfire")
	EPAZoneTier GetZoneTier() const { return CampfireData.ZoneTier; }

	UFUNCTION(BlueprintPure, Category = "Zone|Campfire")
	bool IsInsideSanctuary(const FVector& Pos) const;

	// ===========================================================
	// Delegates
	// ===========================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCampfireLit, FName, CampfireId);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCampfireRestCompleted);

	UPROPERTY(BlueprintAssignable, Category = "Zone|Campfire|Events")
	FOnCampfireLit OnCampfireLit;

	UPROPERTY(BlueprintAssignable, Category = "Zone|Campfire|Events")
	FOnCampfireRestCompleted OnCampfireRestCompleted;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Campfire")
	FPACampfireNode CampfireData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Campfire")
	float SanctuaryRadius = 1000.0f;
};
