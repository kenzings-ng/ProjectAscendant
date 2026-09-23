// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ActiveGameplayEffectHandle.h"
#include "World/PASanctuaryVolume.h"
#include "PAMonsterLeashComponent.generated.h"

class UCharacterMovementComponent;
class UAbilitySystemComponent;
class UPAThreatComponent;
class UGameplayEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnMonsterLeashStateChanged, bool, bIsLeashing);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPAOnMonsterLeashCompleted);

/**
 * UPAMonsterLeashComponent
 *
 * Component quản lý biên giới kéo quái vật / Boss (Leash Boundary Component).
 * - AC-2: Bất kỳ quái vật / boss nào bị kéo vượt quá 2500cm từ vị trí spawn origin sẽ bước vào trạng thái LeashReset.
 * - Xóa sạch aggro (threat table), nhận trạng thái bất tử (State.Invulnerable / State.Immune).
 * - Chạy về vị trí gốc với tốc độ 1.5x walk speed.
 * - Hồi phục 100% Health và Posture qua GameplayEffect có thẩm quyền từ Server.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPAMonsterLeashComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPAMonsterLeashComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Kiểm tra khoảng cách leash theo chu kỳ trên Dedicated Server */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|World|Leash")
	void CheckLeashStatus();

	/** Kích hoạt quá trình LeashReset (AC-2) */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|World|Leash")
	void TriggerLeashReset();

	/** Hoàn tất quá trình LeashReset khi đã về đến vị trí gốc (AC-2) */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|World|Leash")
	void CompleteLeashReset();

	/** Thực thi di chuyển quay về điểm xuất phát */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|World|Leash")
	void ExecuteReturnMovement();

	/** Thiết lập tọa độ điểm Spawn Origin thủ công */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|World|Leash")
	void SetSpawnOrigin(const FVector& InOrigin);

	/** Trả về tọa độ điểm Spawn Origin */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|World|Leash")
	FVector GetSpawnOrigin() const { return SpawnOrigin; }

	/** Trả về cờ kiểm tra quái vật có đang trong trạng thái LeashReset hay không */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|World|Leash")
	bool IsLeashResetting() const { return bIsLeashResetting; }

	/** Trả về khoảng cách giới hạn Leash */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|World|Leash")
	float GetLeashDistance() const { return LeashDistance; }

	/** Thiết lập khoảng cách giới hạn Leash */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|World|Leash")
	void SetLeashDistance(float InDistance) { LeashDistance = InDistance; }

	/** Trả về hệ số tăng tốc khi chạy về */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|World|Leash")
	float GetLeashSpeedMultiplier() const { return LeashSpeedMultiplier; }

	/** Thiết lập hệ số tăng tốc khi chạy về */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|World|Leash")
	void SetLeashSpeedMultiplier(float InMultiplier) { LeashSpeedMultiplier = InMultiplier; }

	/** Tính toán vector hướng quay về vị trí spawn */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|World|Leash")
	FVector GetReturnDirection() const;

	/** Delegate phát khi trạng thái LeashReset thay đổi */
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|World|Leash")
	FPAOnMonsterLeashStateChanged OnLeashStateChanged;

	/** Delegate phát khi quái vật đã hồi vị trí thành công */
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|World|Leash")
	FPAOnMonsterLeashCompleted OnLeashCompleted;

protected:
	/** Vị trí xuất phát ban đầu */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|World|Leash")
	FVector SpawnOrigin = FVector::ZeroVector;

	/** Khoảng cách tối đa trước khi bị kéo về (mặc định 2500cm theo AC-2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|World|Leash", meta = (ClampMin = "500.0"))
	float LeashDistance = FPASanctuaryLeashPipeline::kLeashDefaultDistance;

	/** Cự ly sai số chấp nhận đã về đến gốc (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|World|Leash", meta = (ClampMin = "10.0"))
	float ArrivalTolerance = FPASanctuaryLeashPipeline::kLeashArrivalTolerance;

	/** Hệ số nhân tốc độ chạy về (1.5x theo AC-2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|World|Leash", meta = (ClampMin = "1.0"))
	float LeashSpeedMultiplier = FPASanctuaryLeashPipeline::kLeashSpeedMultiplier;

	/** Chu kỳ kiểm tra cự ly leash (giây) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|World|Leash", meta = (ClampMin = "0.05"))
	float CheckInterval = 0.25f;

	/** Đang trong trạng thái hồi vị trí hay không */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|World|Leash")
	bool bIsLeashResetting = false;

	/** Tốc độ chạy bộ cơ bản trước khi Leash để khôi phục lại */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|World|Leash")
	float CachedOriginalWalkSpeed = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|World|Leash|Tags")
	FGameplayTag TagStateLeashReset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|World|Leash|Tags")
	FGameplayTag TagStateInvulnerable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|World|Leash|Tags")
	FGameplayTag TagStateImmune;

private:
	FTimerHandle LeashCheckTimerHandle;
	FActiveGameplayEffectHandle LeashImmunityEffectHandle;

	UPROPERTY()
	TObjectPtr<UGameplayEffect> CachedImmunityEffect;

	UPROPERTY()
	TObjectPtr<UGameplayEffect> CachedRegenEffect;

	void InitCachedGameplayEffects();
	void ApplyImmunityAndTags();
	void RemoveImmunityAndTags();
	void RegenerateHealthAndPosture();
};
