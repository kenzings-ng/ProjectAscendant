// Copyright Project Ascendant. All Rights Reserved.

#include "Character/PACivilianNPCComponent.h"
#include "Character/PABaseCharacter.h"
#include "Character/PAPaperdollComponent.h"
#include "PaperFlipbookComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Character.h"

// -----------------------------------------------------------------------------
// Constants & Static Mapping (Story visual-003)
// -----------------------------------------------------------------------------
const FName FPACivilianConstants::Prop_Blacksmith_Hammer(TEXT("Prop_Blacksmith_Hammer"));
const FName FPACivilianConstants::Prop_Tongs(TEXT("Prop_Tongs"));
const FName FPACivilianConstants::Prop_Merchant_GoldPouch(TEXT("Prop_Merchant_GoldPouch"));
const FName FPACivilianConstants::Prop_Merchant_Scale(TEXT("Prop_Merchant_Scale"));
const FName FPACivilianConstants::Prop_Villager_Basket(TEXT("Prop_Villager_Basket"));
const FName FPACivilianConstants::Prop_Villager_Broom(TEXT("Prop_Villager_Broom"));
const FName FPACivilianConstants::Prop_Guard_CitySpear(TEXT("Prop_Guard_CitySpear"));
const FName FPACivilianConstants::Prop_Guard_CityShield(TEXT("Prop_Guard_CityShield"));
const FName FPACivilianConstants::Prop_Quest_Scroll(TEXT("Prop_Quest_Scroll"));
const FName FPACivilianConstants::Prop_Quest_Lantern(TEXT("Prop_Quest_Lantern"));

const FName FPACivilianConstants::VFX_Quest_Exclamation(TEXT("VFX_Quest_Exclamation"));
const FName FPACivilianConstants::FB_QuestGiver_Wave(TEXT("FB_QuestGiver_Wave_Callout"));
const FName FPACivilianConstants::MasterMaterialName(TEXT("M_PaperZD_Civilian_Base"));

EPAMasterRig FPACivilianConstants::GetMasterRigForCivilianRole(EPACivilianRole Role)
{
	switch (Role)
	{
	case EPACivilianRole::Blacksmith:
	case EPACivilianRole::TownGuard:
		return EPAMasterRig::HeavyTank; // Dáng vạm vỡ, chân đầm chắc

	case EPACivilianRole::Merchant:
	case EPACivilianRole::Villager:
		return EPAMasterRig::Agility;   // Dáng nhanh nhẹn, bình dân

	case EPACivilianRole::QuestGiver:
		return EPAMasterRig::Caster;    // Dáng học giả, đĩnh đạc

	default:
		return EPAMasterRig::Agility;
	}
}

FName FPACivilianConstants::GetDefaultRightPropForRole(EPACivilianRole Role)
{
	switch (Role)
	{
	case EPACivilianRole::Blacksmith: return Prop_Blacksmith_Hammer;
	case EPACivilianRole::Merchant:   return Prop_Merchant_GoldPouch;
	case EPACivilianRole::Villager:   return Prop_Villager_Basket;
	case EPACivilianRole::TownGuard:  return Prop_Guard_CitySpear;
	case EPACivilianRole::QuestGiver: return Prop_Quest_Scroll;
	default:                          return NAME_None;
	}
}

FName FPACivilianConstants::GetDefaultLeftPropForRole(EPACivilianRole Role)
{
	switch (Role)
	{
	case EPACivilianRole::Blacksmith: return Prop_Tongs;
	case EPACivilianRole::Merchant:   return Prop_Merchant_Scale;
	case EPACivilianRole::TownGuard:  return Prop_Guard_CityShield;
	case EPACivilianRole::QuestGiver: return Prop_Quest_Lantern;
	case EPACivilianRole::Villager:
	default:                          return NAME_None;
	}
}

FPACivilianPalettePreset FPACivilianConstants::GetPalettePresetForZone(FName ZoneId)
{
	const FString ZoneStr = ZoneId.ToString().ToLower();

	if (ZoneStr.Contains(TEXT("ashen")))
	{
		// Ashen Keep: Xám tro (#424242), Đỏ gạch nung (#8D2B2B), Thép rỉ sét (#78909C)
		return FPACivilianPalettePreset(
			FName(TEXT("Citadel_AshenKeep")),
			FLinearColor(0.259f, 0.259f, 0.259f, 1.0f),
			FLinearColor(0.553f, 0.169f, 0.169f, 1.0f),
			FLinearColor(0.471f, 0.565f, 0.612f, 1.0f),
			FName(TEXT("Ashen_Keep"))
		);
	}
	else if (ZoneStr.Contains(TEXT("sanctum")))
	{
		// Sanctum Fortress: Trắng ngà (#F5F5F5), Lam ngọc thẫm (#1A237E), Chỉ vàng kim (#FFD54F)
		return FPACivilianPalettePreset(
			FName(TEXT("Citadel_SanctumFortress")),
			FLinearColor(0.961f, 0.961f, 0.961f, 1.0f),
			FLinearColor(0.102f, 0.137f, 0.494f, 1.0f),
			FLinearColor(1.0f, 0.835f, 0.31f, 1.0f),
			FName(TEXT("Sanctum_Fortress"))
		);
	}

	// Mặc định: Verdant Bastion: Nâu da bò (#8D6E63), Xanh rêu (#4E6E58), Vải lanh mộc (#D7CCC8)
	return FPACivilianPalettePreset(
		FName(TEXT("Citadel_VerdantBastion")),
		FLinearColor(0.553f, 0.431f, 0.388f, 1.0f),
		FLinearColor(0.306f, 0.431f, 0.345f, 1.0f),
		FLinearColor(0.843f, 0.8f, 0.784f, 1.0f),
		FName(TEXT("Verdant_Bastion"))
	);
}

// -----------------------------------------------------------------------------
// UPACivilianNPCComponent Implementation
// -----------------------------------------------------------------------------

UPACivilianNPCComponent::UPACivilianNPCComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Model.SetRole(EPACivilianRole::Villager);
	Model.SetZone(FName(TEXT("Citadel_VerdantBastion")));
}

void UPACivilianNPCComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializePropComponents();
	SynchronizeVisuals();
}

void UPACivilianNPCComponent::SetCivilianRole(EPACivilianRole NewRole)
{
	Model.SetRole(NewRole);
	SynchronizeVisuals();
	OnCivilianRoleChanged.Broadcast(NewRole);
}

bool UPACivilianNPCComponent::SetZonePalette(FName ZoneId)
{
	Model.SetZone(ZoneId);
	SynchronizeVisuals();
	return true;
}

bool UPACivilianNPCComponent::SetZonePaletteFromCitadel(const UPACitadelComponent* Citadel)
{
	if (!Citadel)
	{
		return false;
	}
	return SetZonePalette(Citadel->GetCitadelId());
}

void UPACivilianNPCComponent::AttachCivilianProp(FName PropVisualId, bool bLeftHand)
{
	Model.AttachProp(PropVisualId, bLeftHand);
	SynchronizeVisuals();
}

void UPACivilianNPCComponent::PlayCalloutAnimation()
{
	Model.PlayCalloutAnimation();
	SynchronizeVisuals();
}

void UPACivilianNPCComponent::StopCalloutAnimation()
{
	Model.StopCalloutAnimation();
	SynchronizeVisuals();
}

bool UPACivilianNPCComponent::EvaluateTargetThreat(AActor* TargetActor, float DetectionRadius)
{
	if (Model.Role != EPACivilianRole::TownGuard || !TargetActor)
	{
		return false;
	}

	const FVector SelfLoc = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	const FVector TargetLoc = TargetActor->GetActorLocation();

	bool bIsOutlaw = false;
	if (const APABaseCharacter* BaseChar = Cast<APABaseCharacter>(TargetActor))
	{
		bIsOutlaw = BaseChar->IsOutlaw();
	}
	else
	{
		bIsOutlaw = TargetActor->ActorHasTag(FName(TEXT("State.Outlaw"))) ||
		            TargetActor->ActorHasTag(FName(TEXT("State.Wanted")));
	}

	const bool bTriggered = Model.EvaluateTargetThreat(SelfLoc, TargetLoc, bIsOutlaw, DetectionRadius);
	if (bTriggered)
	{
		ExecuteLethalGatekeeperStrike(TargetActor);
		OnTownGuardStateChanged.Broadcast(Model.GuardState, TargetActor);
		return true;
	}

	return false;
}

bool UPACivilianNPCComponent::ExecuteLethalGatekeeperStrike(AActor* TargetActor)
{
	if (Model.Role != EPACivilianRole::TownGuard || !TargetActor)
	{
		return false;
	}

	const FVector SelfLoc = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	const FVector TargetLoc = TargetActor->GetActorLocation();
	FVector KnockbackDir = (TargetLoc - SelfLoc).GetSafeNormal2D();
	if (KnockbackDir.IsNearlyZero())
	{
		KnockbackDir = FVector(1.0f, 0.0f, 0.0f);
	}

	if (ACharacter* TargetChar = Cast<ACharacter>(TargetActor))
	{
		TargetChar->LaunchCharacter(KnockbackDir * FPACivilianConstants::GuardLethalStrikeKnockback, true, true);
	}

	SynchronizeVisuals();
	return true;
}

void UPACivilianNPCComponent::InitializePropComponents()
{
	AActor* OwnerActor = GetOwner();
	USceneComponent* AttachRoot = OwnerActor ? OwnerActor->GetRootComponent() : nullptr;

	if (OwnerActor && AttachRoot)
	{
		if (!RightHandPropComponent)
		{
			RightHandPropComponent = NewObject<UPaperFlipbookComponent>(OwnerActor, TEXT("Civilian_RightProp"));
			if (RightHandPropComponent)
			{
				RightHandPropComponent->AttachToComponent(AttachRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FPAPaperdollConstants::HandSocket_R);
				RightHandPropComponent->SetTranslucentSortPriority(50);
				RightHandPropComponent->RegisterComponent();
			}
		}

		if (!LeftHandPropComponent)
		{
			LeftHandPropComponent = NewObject<UPaperFlipbookComponent>(OwnerActor, TEXT("Civilian_LeftProp"));
			if (LeftHandPropComponent)
			{
				LeftHandPropComponent->AttachToComponent(AttachRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FPAPaperdollConstants::HandSocket_L);
				LeftHandPropComponent->SetTranslucentSortPriority(5);
				LeftHandPropComponent->RegisterComponent();
			}
		}
	}

	SynchronizeVisuals();
}

void UPACivilianNPCComponent::RegisterRightHandPropComponent(UPaperFlipbookComponent* InComp)
{
	if (InComp)
	{
		RightHandPropComponent = InComp;
		RightHandPropComponent->SetTranslucentSortPriority(50);
		SynchronizeVisuals();
	}
}

void UPACivilianNPCComponent::RegisterLeftHandPropComponent(UPaperFlipbookComponent* InComp)
{
	if (InComp)
	{
		LeftHandPropComponent = InComp;
		LeftHandPropComponent->SetTranslucentSortPriority(5);
		SynchronizeVisuals();
	}
}

void UPACivilianNPCComponent::SynchronizeVisuals()
{
	if (RightHandPropComponent)
	{
		const bool bShowRight = !Model.AttachedRightProp.IsNone();
		RightHandPropComponent->SetVisibility(bShowRight);
	}

	if (LeftHandPropComponent)
	{
		const bool bShowLeft = !Model.AttachedLeftProp.IsNone();
		LeftHandPropComponent->SetVisibility(bShowLeft);
	}

	// Đồng bộ Master Rig với UPAPaperdollComponent nếu có trên cùng Actor
	if (AActor* OwnerActor = GetOwner())
	{
		if (UPAPaperdollComponent* Paperdoll = OwnerActor->FindComponentByClass<UPAPaperdollComponent>())
		{
			Paperdoll->SetMasterRig(Model.MasterRig);

			if (Model.Role == EPACivilianRole::TownGuard && Model.GuardState == EPAGuardAIState::CombatActive)
			{
				// Tái dùng đòn đâm của Weapon Family 3 (Polearms)
				Paperdoll->SetUpperBodyWeaponFamily(EPAWeaponFamily::Polearm_2H);
			}
		}
	}
}
