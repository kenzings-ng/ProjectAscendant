// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/PABaseCharacter.h"
#include "World/PAMonsterLeashComponent.h"
#include "PAStoneGolemBoss.generated.h"

class UPAMonsterLeashComponent;

/**
 * APAStoneGolemBoss
 *
 * Boss Stone Golem trong Project Ascendant (2.5D HD-2D Dark Fantasy Action RPG).
 * Kế thừa APABaseCharacter, tích hợp:
 *  - AC-1: PaperZD State Machine (FB_Golem_Idle, FB_Golem_Walk, FB_Golem_Slam, FB_Golem_Stagger, FB_Golem_Death @ 10 FPS).
 *  - AC-2: AI Aggro & Monster Leash Component (AggroRadius 800cm, LeashRadius 2500cm, tự động truy đuổi hoặc hồi vị trí).
 *  - AC-3: Kỹ năng Ground Slam AoE (Khoảng cách <= 250cm, phạm vi nện búa AoE 300cm gây 35 sát thương và 30 posture damage).
 */
UCLASS()
class PROJECTASCENDANT_API APAStoneGolemBoss : public APABaseCharacter
{
	GENERATED_BODY()

public:
	APAStoneGolemBoss(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// -------------------------------------------------------------------------
	// AI Aggro & Combat Logic
	// -------------------------------------------------------------------------

	/** Cập nhật AI chu kỳ: tìm mục tiêu, truy đuổi và kích hoạt kỹ năng */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Boss|AI")
	void UpdateBossAI(float DeltaSeconds);

	/** Thử kích hoạt đòn đập đất Ground Slam */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Boss|Combat")
	bool TryExecuteGroundSlam();

	/** Thực thi sát thương AoE của cú đập Ground Slam (gọi từ AnimNotify hoặc logic server) */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Boss|Combat")
	void PerformSlamAoEDamage();

	/** Thiết lập mục tiêu truy đuổi */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Boss|AI")
	void SetTargetActor(AActor* NewTarget);

	/** Trả về mục tiêu hiện tại */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Boss|AI")
	AActor* GetTargetActor() const { return CurrentTarget.Get(); }

	/** Trả về Leash Component */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Boss|Leash")
	UPAMonsterLeashComponent* GetLeashComponent() const { return LeashComponent; }

	/** Kiểm tra xem Boss có đang trong trạng thái Slam không */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Boss|Combat")
	bool IsPerformingSlam() const { return bIsPerformingSlam; }

	/** Hoàn tất chiêu Slam (reset cờ đánh) */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Boss|Combat")
	void FinishGroundSlam();

	// -------------------------------------------------------------------------
	// Configuration Properties
	// -------------------------------------------------------------------------

	/** Bán kính phát hiện và kích hoạt Aggro (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Boss|AI")
	float AggroRadius;

	/** Tầm đánh kích hoạt Ground Slam (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Boss|Combat")
	float AttackRange;

	/** Thời gian hồi chiêu Ground Slam (giây) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Boss|Combat")
	float SlamCooldown;

	/** Bán kính sát thương lan AoE của Ground Slam (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Boss|Combat")
	float SlamAoERadius;

	/** Sát thương vật lý cơ bản của Ground Slam */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Boss|Combat")
	float SlamBaseDamage;

	/** Sát thương phá vỡ Posture của Ground Slam */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Boss|Combat")
	float SlamPostureDamage;

	/** Tốc độ di chuyển khi đi bộ tuần tra / truy đuổi (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Boss|Locomotion")
	float BossWalkSpeed;

protected:
	UFUNCTION()
	void HandleLeashStateChanged(bool bIsLeashing);

	UFUNCTION()
	void HandleLeashCompleted();

	/** Hết cooldown Slam */
	UFUNCTION()
	void ResetSlamCooldown();

	/** Component quản lý biên giới quay đầu Leash */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Boss|Components")
	TObjectPtr<UPAMonsterLeashComponent> LeashComponent;

	/** Mục tiêu hiện tại đang nhắm đến */
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CurrentTarget;

	/** Cờ trạng thái đang dập búa Slam */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Boss|State")
	bool bIsPerformingSlam;

	/** Cờ sẵn sàng thi triển Slam */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Boss|State")
	bool bCanSlam;

	/** Timer quản lý hồi chiêu Slam */
	FTimerHandle SlamCooldownTimerHandle;

	/** Timer tự động kết thúc chiêu Slam */
	FTimerHandle SlamDurationTimerHandle;
};
