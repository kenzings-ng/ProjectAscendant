// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Controller/PAIsometricMovementMath.h"
#include "Network/PANetLagCompensation.h"
#include "PABaseCharacter.generated.h"

class UCharacterMovementComponent;
class USpringArmComponent;
class UCameraComponent;
class UAbilitySystemComponent;
class UAscendantAttributeSet;
class UPAStaminaComponent;
class UPaperFlipbookComponent;
class UPaperZDAnimationComponent;
class UPACurrencyComponent;
class UPAPaperdollComponent;

/**
 * APABaseCharacter
 *
 * Lớp nhân vật cơ sở cho Project Ascendant (2.5D HD-2D Dark Fantasy Action RPG).
 * Quản lý di chuyển 8 hướng mượt mà, độc lập hướng quay ngắm đòn (Decoupled Aiming),
 * tích hợp Unreal Engine Gameplay Ability System (GAS) và UAscendantAttributeSet.
 */
UCLASS()
class PROJECTASCENDANT_API APABaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APABaseCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PostInitializeComponents() override;

	// -------------------------------------------------------------------------
	// Locomotion (Story 001)
	// -------------------------------------------------------------------------

	/**
	 * Nhận lệnh di chuyển từ Player Controller theo hệ quy chiếu màn hình 2D
	 * và đưa vào CharacterMovementComponent để thực hiện di chuyển có client prediction.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Locomotion")
	void Input_MoveScreenRelative(const FVector2D& ScreenInputVector);

	/** Trả về vận tốc đi bộ cơ bản tối đa hiện tại (mặc định 550 cm/s). */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Locomotion")
	float GetBaseMoveSpeed() const { return BaseMoveSpeed; }

	/** Cập nhật vận tốc đi bộ tối đa. */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Locomotion")
	void SetBaseMoveSpeed(float NewSpeed);

	/** Trả về trạng thái Kiệt Sức (State.Exhausted) hiện tại của nhân vật. */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|State")
	bool IsExhausted() const { return bIsExhausted; }

	/** Thiết lập cờ trạng thái Kiệt Sức và cập nhật lại tốc độ CharacterMovementComponent. */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|State")
	void SetExhaustedState(bool bInExhausted);

	// -------------------------------------------------------------------------
	// Decoupled Aiming & PaperZD Orientation (Story 002)
	// -------------------------------------------------------------------------

	/**
	 * Cập nhật hướng ngắm bắn trực tiếp từ vector thế giới 2D.
	 * Tách biệt hoàn toàn với vận tốc di chuyển (cho phép vừa lùi vừa chém).
	 *
	 * @param NewAimDirection Vector hướng ngắm chuẩn hóa trên mặt phẳng XY.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Aiming")
	void UpdateAimDirection(const FVector& NewAimDirection);

	/**
	 * Cập nhật hướng ngắm dựa trên tọa độ mục tiêu (điểm giao cắt chuột trên mặt đất).
	 *
	 * @param TargetWorldLocation Tọa độ thế giới mục tiêu.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Aiming")
	void UpdateAimFromTargetLocation(const FVector& TargetWorldLocation);

	/** Trả về vector hướng ngắm bắn hiện tại (chuẩn hóa 2D). */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Aiming")
	FVector GetCurrentAimDirection() const { return CurrentAimDirection; }

	/** Trả về góc ngắm độ hiện tại (-180 đến +180 độ). */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Aiming")
	float GetCurrentAimAngleDegrees() const { return CurrentAimAngleDegrees; }

	/** Trả về hướng 8 chiều phục vụ chọn flipbook PaperZD Animation Blueprint. */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Aiming")
	EPAAimDirection8Way GetCurrent8WayOrientation() const { return Current8WayOrientation; }

	/** Trả về cờ kiểm tra có input ngắm chủ động hay không (chuột hoặc analog stick) */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Aiming")
	bool HasActiveAimInput() const { return bHasActiveAimInput; }

	/** Đặt cờ trạng thái input ngắm chủ động */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Aiming")
	void SetHasActiveAimInput(bool bActive) { bHasActiveAimInput = bActive; }

	// -------------------------------------------------------------------------
	// Camera & SpringArm (Story 003)
	// -------------------------------------------------------------------------

	/** Trả về SpringArm boom của camera */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Camera")
	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Trả về Camera component của nhân vật */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Camera")
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Cập nhật socket offset cho camera boom (dùng cho Look-Ahead đón đầu) */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Camera")
	void SetCameraSocketOffset(const FVector& NewOffset);

	/** Trả về socket offset hiện tại của camera boom */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Camera")
	FVector GetCameraSocketOffset() const;

	// -------------------------------------------------------------------------
	// Paper2D Sprite & PaperZD Animation (Story pzd-002 / ADR-0002)
	// -------------------------------------------------------------------------

	/** Trả về SpriteComponent hiển thị flipbook pixel của nhân vật */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Animation")
	UPaperFlipbookComponent* GetSpriteComponent() const { return SpriteComponent.Get(); }

	/** Trả về PaperZDAnimationComponent của nhân vật */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Animation")
	UPaperZDAnimationComponent* GetPaperZDAnimComponent() const { return PaperZDAnimComponent.Get(); }

	/** Cập nhật xoay góc Sprite (Yaw = 0 cho bên Phải, Yaw = 180 cho bên Trái - AC-2) */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Animation")
	void SetSpriteFacingRight(bool bFacingRight);

	// -------------------------------------------------------------------------
	// Combat & Hitbox Window (Story pzd-002 / AC-3)
	// -------------------------------------------------------------------------

	/** Mở cửa sổ kích hoạt va chạm Hitbox (gọi từ PaperZD AnimNotify trên Server) */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Combat")
	void NotifyHitboxOpened();

	/** Đóng cửa sổ va chạm Hitbox (gọi từ PaperZD AnimNotify trên Server) */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Combat")
	void NotifyHitboxClosed();

	/** Trả về cờ kiểm tra cửa sổ Hitbox đang mở hay không */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Combat")
	bool IsHitboxActive() const { return bIsHitboxActive; }

	// -------------------------------------------------------------------------
	// Gameplay Ability System (GAS - Story 001)
	// -------------------------------------------------------------------------

	/** Trả về AbilitySystemComponent của nhân vật (IAbilitySystemInterface) */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Trả về AttributeSet quản lý chỉ số nhân vật */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|GAS")
	UAscendantAttributeSet* GetAttributeSet() const;

	/** Trả về StaminaComponent quản lý chu trình thể lực và kiệt sức (Story 003) */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Stamina")
	UPAStaminaComponent* GetStaminaComponent() const { return StaminaComponent.Get(); }

	// -------------------------------------------------------------------------
	// Lag Compensation Rewind (Story cmbt-001 / ADR-0001)
	// -------------------------------------------------------------------------

	/** Ghi nhận snapshot vị trí hiện tại của nhân vật vào circular history buffer */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|LagCompensation")
	void RecordPositionSnapshot();

	/** Truy vấn vị trí đã được tua lại trong quá khứ tại Timestamp (nội suy tuyến tính) */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|LagCompensation")
	bool GetRewoundTransform(float TargetTimestamp, FPAPastTransform& OutSnapshot) const;

	/** Lấy buffer lịch sử vị trí phục vụ kiểm thử và debug */
	const FPAPositionHistoryBuffer& GetPositionHistoryBuffer() const { return PositionHistoryBuffer; }
	FPAPositionHistoryBuffer& GetMutablePositionHistoryBuffer() { return PositionHistoryBuffer; }

	/** Callback khi lượng máu của nhân vật cạn kiệt (Health <= 0) */
	virtual void HandleOutOfHealth(AActor* InstigatorActor);

	/** Callback khi thanh thế đứng của nhân vật bị phá vỡ (Posture >= MaxPosture) */
	virtual void HandlePostureBroken(AActor* InstigatorActor);

	/** Trả về CurrencyComponent quản lý ví tiền tệ của nhân vật (Story econ-001) */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Economy")
	UPACurrencyComponent* GetCurrencyComponent() const { return CurrencyComponent.Get(); }

	/** Trả về PaperdollComponent quản lý hiển thị trang bị đa tầng */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Paperdoll")
	UPAPaperdollComponent* GetPaperdollComponent() const { return PaperdollComponent.Get(); }

protected:
	/** Circular ring buffer lưu trữ lịch sử vị trí 200ms phục vụ Lag Compensation */
	UPROPERTY()
	FPAPositionHistoryBuffer PositionHistoryBuffer;

	/** Handle bộ đếm thời gian ghi nhận snapshot định kỳ trên server */
	FTimerHandle PositionHistoryTimerHandle;

	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	/** Vận tốc cơ bản chuẩn hóa theo GDD (550 cm/s) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Movement", meta = (ClampMin = "0.0"))
	float BaseMoveSpeed = 550.0f;

	/** Cờ đánh dấu nhân vật đang ở trạng thái Kiệt Sức */
	UPROPERTY(ReplicatedUsing = OnRep_IsExhausted, BlueprintReadOnly, Category = "ProjectAscendant|State")
	bool bIsExhausted = false;

	/** Cờ kiểm tra cửa sổ va chạm Hitbox đang mở (PaperZD AnimNotify) */
	UPROPERTY(BlueprintReadOnly, Category = "ProjectAscendant|Combat")
	bool bIsHitboxActive = false;

	/** Vector hướng ngắm bắn hiện tại (được đồng bộ qua mạng Iris) */
	UPROPERTY(ReplicatedUsing = OnRep_AimDirection, BlueprintReadOnly, Category = "ProjectAscendant|Aiming")
	FVector CurrentAimDirection = FVector(0.7071f, 0.7071f, 0.0f); // Mặc định hướng Đông Bắc (45 độ)

	/** Góc ngắm bắn theo độ (-180 đến +180) */
	UPROPERTY(BlueprintReadOnly, Category = "ProjectAscendant|Aiming")
	float CurrentAimAngleDegrees = 45.0f;

	/** Phân vùng 8 hướng phục vụ hoạt ảnh PaperZD */
	UPROPERTY(BlueprintReadOnly, Category = "ProjectAscendant|Aiming")
	EPAAimDirection8Way Current8WayOrientation = EPAAimDirection8Way::NorthEast;

	/** Cờ kiểm tra có input ngắm bắn chủ động hay không (chuột hoặc analog stick) */
	UPROPERTY(BlueprintReadOnly, Category = "ProjectAscendant|Aiming")
	bool bHasActiveAimInput = false;

	/** Component hiển thị Sprite Pixel 2D (Paper2D) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPaperFlipbookComponent> SpriteComponent;

	/** Component quản lý Animation PaperZD AnimBP (Story pzd-002 / ADR-0002) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPaperZDAnimationComponent> PaperZDAnimComponent;

	/** Cần giữ camera SpringArm góc nhìn 2.5D Isometric (AC-1) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Camera chính bám theo nhân vật */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	/** Gameplay Ability System Component (GAS) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** Bộ chỉ số và thuộc tính thực thể (Health, Mana, Stamina, Posture) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAscendantAttributeSet> AttributeSet;

	/** Component quản trị thể lực, trễ hồi phục và trạng thái kiệt sức (Story 003) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPAStaminaComponent> StaminaComponent;

	/** Component quản lý ví song tiền tệ Vàng và Tàn Trang (Story econ-001) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Economy", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPACurrencyComponent> CurrencyComponent;

	/** Component quản lý trực quan hóa trang bị đa tầng Modular Paperdoll */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ProjectAscendant|Paperdoll", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPAPaperdollComponent> PaperdollComponent;

	UFUNCTION()
	virtual void OnRep_IsExhausted();

	UFUNCTION()
	virtual void OnRep_AimDirection();

	/** Cập nhật lại MaxWalkSpeed của CharacterMovementComponent */
	void RefreshMovementSpeed();

	/** Cập nhật góc độ và hướng 8 chiều dựa trên CurrentAimDirection */
	void RefreshAimOrientation();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
