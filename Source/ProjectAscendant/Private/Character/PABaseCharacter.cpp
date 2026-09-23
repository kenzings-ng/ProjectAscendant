// Copyright Project Ascendant. All Rights Reserved.

#include "Character/PABaseCharacter.h"
#include "Controller/PAIsometricMovementMath.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "AbilitySystemComponent.h"
#include "Combat/AscendantAttributeSet.h"
#include "Combat/PAStaminaComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "PaperZDAnimationComponent.h"
#include "PaperZDAnimInstance.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Blueprint.h"
#include "Net/UnrealNetwork.h"
#include "Combat/PAGameplayAbility_MeleeAttack.h"
#include "Combat/PAGameplayAbility_Finisher.h"
#include "Combat/PAGameplayAbility_Dash.h"
#include "GameplayEffect.h"
#include "UI/PACharacterSelectTypes.h"
#include "Account/PAAccountSubsystem.h"
#include "Economy/PACurrencyComponent.h"
#include "Network/PALootDropletActor.h"

APABaseCharacter::APABaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = BaseMoveSpeed;
		MoveComp->MaxAcceleration = 2048.0f;
		MoveComp->BrakingDecelerationWalking = 2048.0f;
		MoveComp->GroundFriction = 8.0f;

		// Tách biệt hướng quay mặt với hướng di chuyển (Decoupled Aiming - ADR-0002)
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->bUseControllerDesiredRotation = false;

		MoveComp->bConstrainToPlane = true;
		MoveComp->SetPlaneConstraintNormal(FVector::UpVector);
	}

	// -------------------------------------------------------------------------
	// SpringArm & Camera Setup (Story 003 / AC-1)
	// -------------------------------------------------------------------------
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	// Góc xoay cố định 2.5D Isometric: Pitch = -45 độ, Yaw = 45 độ, Roll = 0 độ
	CameraBoom->SetRelativeRotation(FRotator(UPAIsometricMovementMath::kDefaultCameraPitch, UPAIsometricMovementMath::kDefaultCameraYaw, 0.0f));
	CameraBoom->TargetArmLength = UPAIsometricMovementMath::kDefaultTargetArmLength;
	// Tắt kiểm tra va chạm để ngăn camera tự thu phóng đâm vào mặt nhân vật (AC-1 & Control Manifest)
	CameraBoom->bDoCollisionTest = false;
	// Khóa hướng xoay không phụ thuộc vào góc quay nhân vật
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;
	// Bật độ trễ camera làm mượt chuyển động lướt né
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = UPAIsometricMovementMath::kDefaultCameraLagSpeed;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// -------------------------------------------------------------------------
	// Gameplay Ability System & AttributeSet Setup (Story 001)
	// -------------------------------------------------------------------------
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UAscendantAttributeSet>(TEXT("AttributeSet"));
	StaminaComponent = CreateDefaultSubobject<UPAStaminaComponent>(TEXT("StaminaComponent"));
	CurrencyComponent = CreateDefaultSubobject<UPACurrencyComponent>(TEXT("CurrencyComponent"));
	CurrencyComponent->SetIsReplicated(true);

	// -------------------------------------------------------------------------
	// Paper2D Sprite & PaperZD Animation Setup (Story pzd-002 / ADR-0002)
	// -------------------------------------------------------------------------
	SpriteComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("CharacterSprite"));
	SpriteComponent->SetupAttachment(RootComponent);
	SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpriteComponent->SetGenerateOverlapEvents(false);

	PaperZDAnimComponent = CreateDefaultSubobject<UPaperZDAnimationComponent>(TEXT("PaperZDAnimComponent"));
	PaperZDAnimComponent->PrimaryComponentTick.bCanEverTick = true;
	PaperZDAnimComponent->PrimaryComponentTick.bStartWithTickEnabled = true;
	PaperZDAnimComponent->PrimaryComponentTick.bAllowTickOnDedicatedServer = true; // ADR-0002: Đảm bảo AnimBP tick trên Dedicated Server
	PaperZDAnimComponent->InitRenderComponent(SpriteComponent);

	bReplicates = true;
	SetReplicatingMovement(true);

	// Khởi tạo hướng ngắm mặc định hướng về Đông Bắc theo góc nhìn camera
	RefreshAimOrientation();
}

void APABaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	RefreshMovementSpeed();
	RefreshAimOrientation();

	if (AttributeSet)
	{
		AttributeSet->OnOutOfHealth.AddUObject(this, &APABaseCharacter::HandleOutOfHealth);
		AttributeSet->OnPostureBroken.AddUObject(this, &APABaseCharacter::HandlePostureBroken);
	}
}

void APABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Đồng bộ chỉ số cơ sở theo Chức nghiệp được chọn trong AccountSubsystem (GDD foundational-classes.md)
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UPAAccountSubsystem* AccSub = GI->GetSubsystem<UPAAccountSubsystem>())
		{
			const FName SelectedClassTag = AccSub->GetSelectedCharacterClass();
			EPACharacterClass SelectedClass = EPACharacterClass::Vanguard;
			if (SelectedClassTag == FName(TEXT("Class.Ranger")))
			{
				SelectedClass = EPACharacterClass::Ranger;
			}
			else if (SelectedClassTag == FName(TEXT("Class.Arcanist")))
			{
				SelectedClass = EPACharacterClass::Arcanist;
			}

			const FPACharacterClassInfo ClassInfo = FPACharacterClassRegistry::GetClassInfo(SelectedClass);
			if (AttributeSet)
			{
				AttributeSet->InitMaxHealth(ClassInfo.BaseHealth);
				AttributeSet->InitHealth(ClassInfo.BaseHealth);
				AttributeSet->InitMaxStamina(ClassInfo.BaseStamina);
				AttributeSet->InitStamina(ClassInfo.BaseStamina);
				AttributeSet->InitMaxMana(ClassInfo.BaseMana);
				AttributeSet->InitMana(ClassInfo.BaseMana);
				AttributeSet->InitMaxPosture(ClassInfo.BasePosture);
				AttributeSet->InitPosture(0.0f);
				AttributeSet->InitMoveSpeed(ClassInfo.MoveSpeed);
			}
			SetBaseMoveSpeed(ClassInfo.MoveSpeed);
		}
	}

	// Fallback nạp flipbook và AnimBP cho nhân vật nếu chưa được gán trong Blueprint
	if (SpriteComponent && !SpriteComponent->GetFlipbook())
	{
		if (UPaperFlipbook* IdleFb = Cast<UPaperFlipbook>(StaticLoadObject(UPaperFlipbook::StaticClass(), nullptr, TEXT("/Game/art/characters/vanguard/flipbooks/FB_Vanguard_Idle.FB_Vanguard_Idle"))))
		{
			SpriteComponent->SetFlipbook(IdleFb);
		}
	}

	if (PaperZDAnimComponent && !PaperZDAnimComponent->GetAnimInstanceClass())
	{
		UClass* LoadedAnimBPClass = StaticLoadClass(UPaperZDAnimInstance::StaticClass(), nullptr, TEXT("/Game/art/characters/vanguard/anim/ABP_Vanguard.ABP_Vanguard_C"));
		if (!LoadedAnimBPClass)
		{
			if (UBlueprint* AnimBP = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, TEXT("/Game/art/characters/vanguard/anim/ABP_Vanguard.ABP_Vanguard"))))
			{
				LoadedAnimBPClass = AnimBP->GeneratedClass;
			}
		}

		if (LoadedAnimBPClass)
		{
			PaperZDAnimComponent->SetAnimInstanceClass(LoadedAnimBPClass);
		}
	}

	if (HasAuthority())
	{
		// Cấp quyền thi triển kỹ năng chiến đấu cơ bản
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UPAGameplayAbility_MeleeAttack::StaticClass(), 1, INDEX_NONE, this));
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UPAGameplayAbility_Finisher::StaticClass(), 1, INDEX_NONE, this));
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UPAGameplayAbility_Dash::StaticClass(), 1, INDEX_NONE, this));
		}

		// Bắt đầu ghi nhận lịch sử vị trí định kỳ 50Hz (0.02s) phục vụ Lag Compensation Rewind
		GetWorldTimerManager().SetTimer(
			PositionHistoryTimerHandle,
			this,
			&APABaseCharacter::RecordPositionSnapshot,
			0.02f,
			true
		);
	}
}

float APABaseCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.0f && HasAuthority() && AbilitySystemComponent)
	{
		static const FGameplayTag TagInvuln = FGameplayTag::RequestGameplayTag(TEXT("State.Invulnerable"), false);
		static const FGameplayTag TagSanctuary = FGameplayTag::RequestGameplayTag(TEXT("State.InSanctuary"), false);
		if ((TagInvuln.IsValid() && AbilitySystemComponent->HasMatchingGameplayTag(TagInvuln)) ||
			(TagSanctuary.IsValid() && AbilitySystemComponent->HasMatchingGameplayTag(TagSanctuary)))
		{
			return 0.0f;
		}

		UGameplayEffect* GenericDamageEffect = NewObject<UGameplayEffect>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UGameplayEffect::StaticClass(), TEXT("GE_GenericDamage")));
		if (GenericDamageEffect)
		{
			GenericDamageEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
			FGameplayModifierInfo ModInfo;
			ModInfo.Attribute = UAscendantAttributeSet::GetHealthAttribute();
			ModInfo.ModifierOp = EGameplayModOp::Additive;
			ModInfo.ModifierMagnitude = FScalableFloat(-ActualDamage);
			GenericDamageEffect->Modifiers.Add(ModInfo);

			FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
			ContextHandle.AddInstigator(DamageCauser, DamageCauser);
			AbilitySystemComponent->ApplyGameplayEffectToSelf(GenericDamageEffect, 1.0f, ContextHandle);
		}
	}
	return ActualDamage;
}

void APABaseCharacter::Input_MoveScreenRelative(const FVector2D& ScreenInputVector)
{
	if (ScreenInputVector.IsNearlyZero())
	{
		return;
	}

	const FVector WorldDirection = UPAIsometricMovementMath::CalculateWorldDirection(ScreenInputVector, 45.0f);

	if (!WorldDirection.IsNearlyZero())
	{
		AddMovementInput(WorldDirection, 1.0f);
	}
}

void APABaseCharacter::UpdateAimDirection(const FVector& NewAimDirection)
{
	if (NewAimDirection.IsNearlyZero())
	{
		bHasActiveAimInput = false;
		return;
	}

	bHasActiveAimInput = true;
	CurrentAimDirection = NewAimDirection.GetSafeNormal2D();
	RefreshAimOrientation();
}

void APABaseCharacter::UpdateAimFromTargetLocation(const FVector& TargetWorldLocation)
{
	const FVector AimDir = UPAIsometricMovementMath::CalculateAimDirection2D(GetActorLocation(), TargetWorldLocation);
	bHasActiveAimInput = !AimDir.IsNearlyZero();
	UpdateAimDirection(AimDir);
}

void APABaseCharacter::RefreshAimOrientation()
{
	CurrentAimAngleDegrees = UPAIsometricMovementMath::CalculateAimAngleDegrees(CurrentAimDirection);
	Current8WayOrientation = UPAIsometricMovementMath::Get8WayDirectionFromAngle(CurrentAimAngleDegrees);
}

void APABaseCharacter::OnRep_AimDirection()
{
	RefreshAimOrientation();
}

void APABaseCharacter::SetBaseMoveSpeed(float NewSpeed)
{
	BaseMoveSpeed = FMath::Max(0.0f, NewSpeed);
	RefreshMovementSpeed();
}

void APABaseCharacter::SetExhaustedState(bool bInExhausted)
{
	if (bIsExhausted != bInExhausted)
	{
		bIsExhausted = bInExhausted;
		RefreshMovementSpeed();
	}
}

void APABaseCharacter::OnRep_IsExhausted()
{
	RefreshMovementSpeed();
}

void APABaseCharacter::RefreshMovementSpeed()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		const float EffectiveSpeed = UPAIsometricMovementMath::CalculateEffectiveMoveSpeed(BaseMoveSpeed, bIsExhausted);
		MoveComp->MaxWalkSpeed = EffectiveSpeed;
	}
}

void APABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APABaseCharacter, bIsExhausted, COND_None);
	DOREPLIFETIME_CONDITION(APABaseCharacter, CurrentAimDirection, COND_SimulatedOnly);
}

void APABaseCharacter::SetCameraSocketOffset(const FVector& NewOffset)
{
	if (CameraBoom)
	{
		CameraBoom->SocketOffset = NewOffset;
	}
}

FVector APABaseCharacter::GetCameraSocketOffset() const
{
	return CameraBoom ? CameraBoom->SocketOffset : FVector::ZeroVector;
}

UAbilitySystemComponent* APABaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

UAscendantAttributeSet* APABaseCharacter::GetAttributeSet() const
{
	return AttributeSet.Get();
}

void APABaseCharacter::HandleOutOfHealth(AActor* InstigatorActor)
{
	// Xử lý khi nhân vật cạn kiệt sinh mệnh (Health <= 0) - AC-1
	if (AbilitySystemComponent)
	{
		static const FGameplayTag TagDead = FGameplayTag::RequestGameplayTag(TEXT("State.Dead"), false);
		if (TagDead.IsValid())
		{
			AbilitySystemComponent->AddLooseGameplayTag(TagDead);
		}
		AbilitySystemComponent->CancelAllAbilities();
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
		MoveComp->StopMovementImmediately();
	}

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}

	// Xử lý hình phạt tử trận PvE trừ 50% Gold rơi vào Vệt Tro Tàn (Story econ-001 / AC-3)
	if (CurrencyComponent && HasAuthority())
	{
		int64 DroppedGold = 0;
		if (CurrencyComponent->HandlePvEDeathPenalty(DroppedGold) && DroppedGold > 0)
		{
			if (UWorld* World = GetWorld())
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				if (APALootDropletActor* Droplet = World->SpawnActor<APALootDropletActor>(APALootDropletActor::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, SpawnParams))
				{
					APlayerController* PC = Cast<APlayerController>(GetController());
					Droplet->InitializeDroplet(
						GetName(),
						PC,
						TEXT("PvE_Death_AshenRemnant"),
						static_cast<int32>(FMath::Min(DroppedGold, static_cast<int64>(MAX_int32))),
						{}
					);
				}
			}
		}
	}
}

void APABaseCharacter::HandlePostureBroken(AActor* InstigatorActor)
{
	// Xử lý khi nhân vật bị vỡ thế đứng (Posture >= MaxPosture) - AC-1 & AC-3
	if (AbilitySystemComponent)
	{
		static const FGameplayTag TagBroken = FGameplayTag::RequestGameplayTag(TEXT("State.Broken"), false);
		if (TagBroken.IsValid())
		{
			AbilitySystemComponent->AddLooseGameplayTag(TagBroken);
		}

		static const FGameplayTag TagStunned = FGameplayTag::RequestGameplayTag(TEXT("State.Stunned"), false);
		if (TagStunned.IsValid())
		{
			AbilitySystemComponent->AddLooseGameplayTag(TagStunned);
		}

		AbilitySystemComponent->CancelAllAbilities();
	}

	// Tự động giải trừ trạng thái Broken/Stunned sau 3.0s nếu còn sống
	if (UWorld* World = GetWorld())
	{
		FTimerHandle StaggerTimerHandle;
		World->GetTimerManager().SetTimer(
			StaggerTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (AbilitySystemComponent)
				{
					static const FGameplayTag TagBroken = FGameplayTag::RequestGameplayTag(TEXT("State.Broken"), false);
					if (TagBroken.IsValid())
					{
						AbilitySystemComponent->RemoveLooseGameplayTag(TagBroken);
					}

					static const FGameplayTag TagStunned = FGameplayTag::RequestGameplayTag(TEXT("State.Stunned"), false);
					if (TagStunned.IsValid())
					{
						AbilitySystemComponent->RemoveLooseGameplayTag(TagStunned);
					}
				}

				if (AttributeSet)
				{
					AttributeSet->SetPosture(0.0f);
				}
			}),
			3.0f,
			false
		);
	}
}

void APABaseCharacter::SetSpriteFacingRight(bool bFacingRight)
{
	if (SpriteComponent)
	{
		// AC-2: Yaw 0 độ cho bên phải, Yaw 180 độ cho bên trái
		const FRotator TargetRotation(0.0f, bFacingRight ? 0.0f : 180.0f, 0.0f);
		SpriteComponent->SetRelativeRotation(TargetRotation);
	}
}

void APABaseCharacter::NotifyHitboxOpened()
{
	if (HasAuthority())
	{
		bIsHitboxActive = true;

		static const FGameplayTag TagHitboxActive = FGameplayTag::RequestGameplayTag(TEXT("Event.Combat.Hitbox.Active"), false);
		if (TagHitboxActive.IsValid() && AbilitySystemComponent)
		{
			FGameplayEventData EventData;
			EventData.EventTag = TagHitboxActive;
			EventData.Instigator = this;
			EventData.Target = this;
			EventData.EventMagnitude = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TagHitboxActive, EventData);
		}
	}
}

void APABaseCharacter::NotifyHitboxClosed()
{
	if (HasAuthority())
	{
		bIsHitboxActive = false;

		static const FGameplayTag TagHitboxEnd = FGameplayTag::RequestGameplayTag(TEXT("Event.Combat.Hitbox.End"), false);
		if (TagHitboxEnd.IsValid() && AbilitySystemComponent)
		{
			FGameplayEventData EventData;
			EventData.EventTag = TagHitboxEnd;
			EventData.Instigator = this;
			EventData.Target = this;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, TagHitboxEnd, EventData);
		}
	}
}

void APABaseCharacter::RecordPositionSnapshot()
{
	if (UWorld* World = GetWorld())
	{
		const float CurrentTime = World->GetTimeSeconds();
		const FVector CurrentLoc = GetActorLocation();
		const FRotator CurrentRot = GetActorRotation();
		UCapsuleComponent* Capsule = GetCapsuleComponent();
		const FVector Extents = Capsule ? FVector(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight()) : FVector(34.0f, 34.0f, 88.0f);
		PositionHistoryBuffer.RecordSnapshot(CurrentTime, CurrentLoc, CurrentRot, Extents);
	}
}

bool APABaseCharacter::GetRewoundTransform(float TargetTimestamp, FPAPastTransform& OutSnapshot) const
{
	return PositionHistoryBuffer.GetInterpolatedSnapshot(TargetTimestamp, OutSnapshot);
}



