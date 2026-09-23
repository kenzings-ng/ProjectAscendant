// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/PAPartBreakingTypes.h"
#include "PAPartBreakingComponent.generated.h"

/**
 * UPAPartBreakingComponent
 *
 * ActorComponent quản lý hệ thống Phá hủy bộ phận giải phẫu của Boss.
 * Theo dõi máu Sừng, Đuôi, Giáp ngực; khóa vĩnh viễn chiêu thức trong Behavior Tree;
 * mở khóa tử huyệt ngực (+50% damage) và phát tín hiệu rơi nguyên liệu.
 *
 * Tham chiếu GDD: design/gdd/stagger-system.md §Part Breaking System
 * ADR-0001: Server-authoritative logic
 * ADR-0002: PaperZD & GAS integration
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPAPartBreakingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPAPartBreakingComponent();

	virtual void BeginPlay() override;

	// ===========================================================
	// Public API
	// ===========================================================

	UFUNCTION(BlueprintCallable, Category = "Combat|PartBreaking")
	void InitializeParts(float InBossMaxHP = 10000.0f);

	/**
	 * Gây sát thương vào một bộ phận cụ thể.
	 * Tự động trừ cả Part HP và Boss Total HP, đồng thời áp dụng nhân hệ số yếu điểm nếu có.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|PartBreaking")
	FPAPartBreakResult ApplyPartDamage(EPABossPartType PartType, float RawDamage);

	// ===========================================================
	// Getters
	// ===========================================================

	UFUNCTION(BlueprintPure, Category = "Combat|PartBreaking")
	bool IsPartBroken(EPABossPartType PartType) const { return Model.IsPartBroken(PartType); }

	UFUNCTION(BlueprintPure, Category = "Combat|PartBreaking")
	float GetPartHealth(EPABossPartType PartType) const { return Model.GetPartHealth(PartType); }

	UFUNCTION(BlueprintPure, Category = "Combat|PartBreaking")
	float GetPartHealthPercent(EPABossPartType PartType) const { return Model.GetPartHealthPercent(PartType); }

	UFUNCTION(BlueprintPure, Category = "Combat|PartBreaking")
	bool IsChestWeakpointActive() const { return Model.bChestWeakpointActive; }

	UFUNCTION(BlueprintPure, Category = "Combat|PartBreaking")
	float GetDamageMultiplier(EPABossPartType PartType) const { return Model.GetDamageMultiplier(PartType); }

	UFUNCTION(BlueprintPure, Category = "Combat|PartBreaking")
	const FPAPartBreakingModel& GetModel() const { return Model; }

	// ===========================================================
	// Delegates
	// ===========================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartBroken, EPABossPartType, PartType, FName, DropItemId);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChestWeakpointRevealed);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossStunnedByBreak, float, StunDuration);

	UPROPERTY(BlueprintAssignable, Category = "Combat|PartBreaking|Events")
	FOnPartBroken OnPartBroken;

	UPROPERTY(BlueprintAssignable, Category = "Combat|PartBreaking|Events")
	FOnChestWeakpointRevealed OnChestWeakpointRevealed;

	UPROPERTY(BlueprintAssignable, Category = "Combat|PartBreaking|Events")
	FOnBossStunnedByBreak OnBossStunnedByBreak;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|PartBreaking")
	FPAPartBreakingModel Model;
};
