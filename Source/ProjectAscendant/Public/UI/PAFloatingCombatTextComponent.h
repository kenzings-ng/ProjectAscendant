// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/PACombatTextTypes.h"
#include "PAFloatingCombatTextComponent.generated.h"

/**
 * UPAFloatingCombatTextComponent
 *
 * ActorComponent quản lý Floating Combat Text (số sát thương nổi) cho nhân vật.
 * Attach vào PlayerController hoặc HUD Actor, cung cấp API spawn và tick update
 * cho hệ thống UMG Canvas renderer.
 *
 * Tham chiếu GDD: design/gdd/combat-hud.md §Floating Combat Text
 * ADR-0002: Combat event delegates
 *
 * Đặc điểm:
 * - Object pooling (tối đa 50 instance) → zero GC spikes
 * - Quỹ đạo đạn đạo P(t) = P0 + V0*t + 0.5*g*t²
 * - Radial scatter offset tự động (golden angle) tránh chồng chéo
 * - Fade out tuyến tính theo lifetime
 */
UCLASS(ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPAFloatingCombatTextComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPAFloatingCombatTextComponent();

	// ===========================================================
	// Lifecycle
	// ===========================================================

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===========================================================
	// Spawn API
	// ===========================================================

	/**
	 * Spawn một số sát thương nổi tại vị trí World Space.
	 * @param WorldLocation   Vị trí 3D nguồn gốc sát thương
	 * @param Amount          Số lượng sát thương (hiển thị dạng số nguyên)
	 * @param Type            Loại sát thương (Normal, Critical, Posture)
	 */
	UFUNCTION(BlueprintCallable, Category = "CombatText")
	void SpawnCombatText(const FVector& WorldLocation, float Amount, EPACombatTextType Type);

	/**
	 * Spawn chữ "PERFECT!" trên đầu nhân vật.
	 * @param CharacterLocation   Vị trí gốc nhân vật (chữ sẽ nổi lên 60cm phía trên)
	 */
	UFUNCTION(BlueprintCallable, Category = "CombatText")
	void SpawnPerfectDodgeCallout(const FVector& CharacterLocation);

	// ===========================================================
	// Query API (cho UMG Canvas Renderer)
	// ===========================================================

	/** Số instance đang hoạt động */
	UFUNCTION(BlueprintPure, Category = "CombatText")
	int32 GetActiveCount() const { return Pool.GetActiveCount(); }

	/** Tổng slot trong pool */
	UFUNCTION(BlueprintPure, Category = "CombatText")
	int32 GetPoolSize() const { return Pool.GetPoolSize(); }

	/** Truy cập pool trực tiếp (cho render layer) */
	const FPACombatTextPool& GetPool() const { return Pool; }

	// ===========================================================
	// Delegates cho Blueprint Event Binding
	// ===========================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatTextSpawned, int32, InstanceIndex, EPACombatTextType, Type);

	UPROPERTY(BlueprintAssignable, Category = "CombatText|Events")
	FOnCombatTextSpawned OnCombatTextSpawned;

protected:
	/** Object pool quản lý tất cả text instances */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CombatText")
	FPACombatTextPool Pool;
};
