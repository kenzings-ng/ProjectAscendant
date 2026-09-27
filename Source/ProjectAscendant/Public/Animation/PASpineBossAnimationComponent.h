// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpineSkeletonAnimationComponent.h"
#include "SpineSkeletonRendererComponent.h"
#include "PASpineBossAnimationComponent.generated.h"

class APAStoneGolemBoss;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpineBossEventReceived, const FString&, EventName);

/**
 * EPASpineBossState
 *
 * Enum đại diện cho các trạng thái hoạt ảnh chính của Boss được quản lý bởi Spine 4.3 runtime.
 */
UENUM(BlueprintType)
enum class EPASpineBossState : uint8
{
	Idle,
	Walk,
	Slam,
	Stagger,
	Death
};

/**
 * UPASpineBossAnimationComponent
 *
 * Component thử nghiệm tích hợp Spine 4.3 Runtime cho Stone Golem Boss.
 * Đóng vai trò Adapter chuyển đổi giữa AI / GAS Combat Logic của Boss và hệ thống Skeleton / Bone animation của Spine.
 * Hoạt động song song hoặc thay thế PaperZD Flipbook khi bUseSpineMode = true.
 */
UCLASS(ClassGroup = (ProjectAscendant), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPASpineBossAnimationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPASpineBossAnimationComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Kích hoạt hoặc vô hiệu hóa chế độ hiển thị bằng Spine thay cho PaperZD */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Boss|Spine")
	void SetSpineModeActive(bool bActive);

	/** Kiểm tra chế độ Spine có đang hoạt động hay không */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Boss|Spine")
	bool IsSpineModeActive() const { return bIsSpineActive; }

	/** Chơi hoạt ảnh Slam của Boss qua Spine */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Boss|Spine")
	bool PlaySpineSlamAnimation();

	/** Chơi hoạt ảnh Stagger */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Boss|Spine")
	bool PlaySpineStaggerAnimation();

	/** Chơi hoạt ảnh Death */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Boss|Spine")
	bool PlaySpineDeathAnimation();

	/** Delegate thông báo khi nhận được sự kiện Spine (ví dụ: "slam_impact", "footstep") */
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Boss|Spine|Events")
	FOnSpineBossEventReceived OnSpineBossEvent;

	// -------------------------------------------------------------------------
	// Spine Animation Mapping Configuration
	// -------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Boss|Spine|Config")
	FString IdleAnimName = TEXT("idle");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Boss|Spine|Config")
	FString WalkAnimName = TEXT("walk");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Boss|Spine|Config")
	FString SlamAnimName = TEXT("slam");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Boss|Spine|Config")
	FString StaggerAnimName = TEXT("stagger");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Boss|Spine|Config")
	FString DeathAnimName = TEXT("death");

	/** Tên event trong file Spine phát ra tại thời điểm nện búa dập đất */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Boss|Spine|Config")
	FString SlamImpactEventName = TEXT("slam_impact");

	/** Thời gian crossfade chuyển đổi giữa các animation track (giây) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Boss|Spine|Config")
	float DefaultMixDuration = 0.15f;

protected:
	/** Callback khi Spine nhận được Event từ animation timeline */
	UFUNCTION()
	void HandleSpineAnimationEvent(UTrackEntry* TrackEntry, FSpineEvent Event);

	/** Callback khi Spine hoàn thành một animation không loop (như Slam hoặc Stagger) */
	UFUNCTION()
	void HandleSpineAnimationComplete(UTrackEntry* TrackEntry);

	/** Cập nhật trạng thái Locomotion (Idle vs Walk) dựa trên vận tốc Character Movement */
	void UpdateLocomotionState();

	/** Con trỏ tới Boss sở hữu */
	UPROPERTY(Transient)
	TWeakObjectPtr<APAStoneGolemBoss> OwnerBoss;

	/** Con trỏ tới Spine Animation Component của Actor */
	UPROPERTY(Transient)
	TWeakObjectPtr<USpineSkeletonAnimationComponent> SpineAnimComp;

	/** Con trỏ tới Spine Renderer Component của Actor */
	UPROPERTY(Transient)
	TWeakObjectPtr<USpineSkeletonRendererComponent> SpineRendererComp;

	/** Trạng thái hiện tại của Spine */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Boss|Spine|State")
	EPASpineBossState CurrentState = EPASpineBossState::Idle;

	/** Cờ trạng thái Spine đang active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Boss|Spine|State")
	bool bIsSpineActive = false;

	/** Cờ ngăn chặn ngắt chiêu khi đang trong animation độc quyền (Slam, Stagger, Death) */
	bool bIsLockedInAction = false;
};
