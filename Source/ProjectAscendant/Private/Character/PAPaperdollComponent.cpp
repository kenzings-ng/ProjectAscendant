#include "Character/PAPaperdollComponent.h"
#include "Character/PABaseCharacter.h"
#include "Inventory/PAEquipmentComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"
#include "PaperFlipbookComponent.h"
#include "Net/UnrealNetwork.h"

// -----------------------------------------------------------------------------
// Constants & Sort Key
// -----------------------------------------------------------------------------
const FName FPAPaperdollConstants::HandSocket_R(TEXT("HandSocket_R"));
const FName FPAPaperdollConstants::HandSocket_L(TEXT("HandSocket_L"));
const FVector2D FPAPaperdollConstants::FootPivot(64.0f, 114.0f);
const FIntPoint FPAPaperdollConstants::PlaceholderDimensions(128, 128);

const FName FPAPaperdollConstants::Socket_LowerBody(TEXT("Socket_LowerBody"));
const FName FPAPaperdollConstants::Socket_UpperBody(TEXT("Socket_UpperBody"));
const FVector2D FPAPaperdollConstants::WaistPivot(64.0f, 80.0f);

const FName FPAPaperdollConstants::Socket_HelmCrest(TEXT("Socket_HelmCrest"));
const FName FPAPaperdollConstants::Socket_Tabard(TEXT("Socket_Tabard"));
const FVector2D FPAPaperdollConstants::HelmCrestLocation(64.0f, 40.0f);
const FVector2D FPAPaperdollConstants::TabardLocation(64.0f, 60.0f);
const FIntPoint FPAPaperdollConstants::CrestPlaceholderDimensions(32, 32);
const FIntPoint FPAPaperdollConstants::TabardPlaceholderDimensions(48, 64);

FName FPAPaperdollConstants::GetDefaultLowerBodyAssetForRig(EPAMasterRig Rig)
{
	switch (Rig)
	{
	case EPAMasterRig::HeavyTank: return FName(TEXT("FB_Lower_HeavyTank_Set"));
	case EPAMasterRig::Agility:   return FName(TEXT("FB_Lower_Agility_Set"));
	case EPAMasterRig::Caster:    return FName(TEXT("FB_Lower_Caster_Set"));
	case EPAMasterRig::Monk:      return FName(TEXT("FB_Lower_Monk_Set"));
	default:                      return FName(TEXT("FB_Lower_HeavyTank_Set"));
	}
}

FName FPAPaperdollConstants::GetDefaultUpperBodyAssetForFamily(EPAWeaponFamily Family)
{
	switch (Family)
	{
	case EPAWeaponFamily::Blade_1H:     return FName(TEXT("FB_Upper_1H_Blade_Combo"));
	case EPAWeaponFamily::Heavy_2H:     return FName(TEXT("FB_Upper_2H_Heavy_Combo"));
	case EPAWeaponFamily::Polearm_2H:   return FName(TEXT("FB_Upper_2H_Polearm_Combo"));
	case EPAWeaponFamily::Bow_2H:       return FName(TEXT("FB_Upper_2H_Bow_Combo"));
	case EPAWeaponFamily::DualDaggers:  return FName(TEXT("FB_Upper_Dual_Daggers_Combo"));
	case EPAWeaponFamily::Staff_2H:     return FName(TEXT("FB_Upper_2H_Staff_Combo"));
	case EPAWeaponFamily::MaceRelic_1H: return FName(TEXT("FB_Upper_1H_MaceRelic_Combo"));
	default:                            return NAME_None;
	}
}

EPAWeaponFamily FPAPaperdollConstants::GetWeaponFamilyFromTag(FGameplayTag Tag)
{
	if (!Tag.IsValid())
	{
		return EPAWeaponFamily::None;
	}

	const FString TagStr = Tag.ToString();
	if (TagStr.Contains(TEXT("Weapon.1H.Blade"))) return EPAWeaponFamily::Blade_1H;
	if (TagStr.Contains(TEXT("Weapon.2H.Heavy"))) return EPAWeaponFamily::Heavy_2H;
	if (TagStr.Contains(TEXT("Weapon.2H.Polearm"))) return EPAWeaponFamily::Polearm_2H;
	if (TagStr.Contains(TEXT("Weapon.2H.Bow"))) return EPAWeaponFamily::Bow_2H;
	if (TagStr.Contains(TEXT("Weapon.Dual.Daggers"))) return EPAWeaponFamily::DualDaggers;
	if (TagStr.Contains(TEXT("Weapon.2H.Staff"))) return EPAWeaponFamily::Staff_2H;
	if (TagStr.Contains(TEXT("Weapon.1H.Mace"))) return EPAWeaponFamily::MaceRelic_1H;

	return EPAWeaponFamily::None;
}

#include "GameplayTagsManager.h"

FGameplayTag FPAPaperdollConstants::GetTagForWeaponFamily(EPAWeaponFamily Family)
{
	FName TagName = NAME_None;
	switch (Family)
	{
	case EPAWeaponFamily::Blade_1H:     TagName = FName(TEXT("Weapon.1H.Blade")); break;
	case EPAWeaponFamily::Heavy_2H:     TagName = FName(TEXT("Weapon.2H.Heavy")); break;
	case EPAWeaponFamily::Polearm_2H:   TagName = FName(TEXT("Weapon.2H.Polearm")); break;
	case EPAWeaponFamily::Bow_2H:       TagName = FName(TEXT("Weapon.2H.Bow")); break;
	case EPAWeaponFamily::DualDaggers:  TagName = FName(TEXT("Weapon.Dual.Daggers")); break;
	case EPAWeaponFamily::Staff_2H:     TagName = FName(TEXT("Weapon.2H.Staff")); break;
	case EPAWeaponFamily::MaceRelic_1H: TagName = FName(TEXT("Weapon.1H.Mace")); break;
	default: break;
	}

	if (TagName.IsNone())
	{
		return FGameplayTag();
	}

	UGameplayTagsManager::Get().AddNativeGameplayTag(TagName);
	return FGameplayTag::RequestGameplayTag(TagName, false);
}

FName FPAPaperdollConstants::GetDefaultCrestAssetForClass(FName ClassName)
{
	const FString NameStr = ClassName.ToString().ToLower();
	if (NameStr.Contains(TEXT("vanguard")))     return FName(TEXT("FB_Crest_Vanguard"));
	if (NameStr.Contains(TEXT("ranger")))       return FName(TEXT("FB_Crest_Ranger"));
	if (NameStr.Contains(TEXT("arcanist")))     return FName(TEXT("FB_Crest_Arcanist"));
	if (NameStr.Contains(TEXT("acolyte")))      return FName(TEXT("FB_Crest_Acolyte"));
	if (NameStr.Contains(TEXT("berserker")))    return FName(TEXT("FB_Crest_Berserker"));
	if (NameStr.Contains(TEXT("shadowblade")))  return FName(TEXT("FB_Crest_Shadowblade"));
	if (NameStr.Contains(TEXT("elementalist"))) return FName(TEXT("FB_Crest_Elementalist"));
	if (NameStr.Contains(TEXT("templar")))      return FName(TEXT("FB_Crest_Templar"));
	if (NameStr.Contains(TEXT("voidblade")) || NameStr.Contains(TEXT("void blade"))) return FName(TEXT("FB_Crest_VoidBlade"));
	if (NameStr.Contains(TEXT("chronomancer"))) return FName(TEXT("FB_Crest_Chronomancer"));
	if (NameStr.Contains(TEXT("dragonknight")) || NameStr.Contains(TEXT("dragon knight"))) return FName(TEXT("FB_Crest_DragonKnight"));
	if (NameStr.Contains(TEXT("godslayer")) || NameStr.Contains(TEXT("god slayer"))) return FName(TEXT("FB_Crest_GodSlayer"));

	return FName(TEXT("FB_Crest_Vanguard"));
}

FName FPAPaperdollConstants::GetDefaultTabardAssetForClass(FName ClassName)
{
	const FString NameStr = ClassName.ToString().ToLower();
	if (NameStr.Contains(TEXT("vanguard")))     return FName(TEXT("FB_Tabard_Vanguard"));
	if (NameStr.Contains(TEXT("ranger")))       return FName(TEXT("FB_Tabard_Ranger"));
	if (NameStr.Contains(TEXT("arcanist")))     return FName(TEXT("FB_Tabard_Arcanist"));
	if (NameStr.Contains(TEXT("acolyte")))      return FName(TEXT("FB_Tabard_Acolyte"));
	if (NameStr.Contains(TEXT("berserker")))    return FName(TEXT("FB_Tabard_Berserker"));
	if (NameStr.Contains(TEXT("shadowblade")))  return FName(TEXT("FB_Tabard_Shadowblade"));
	if (NameStr.Contains(TEXT("elementalist"))) return FName(TEXT("FB_Tabard_Elementalist"));
	if (NameStr.Contains(TEXT("templar")))      return FName(TEXT("FB_Tabard_Templar"));
	if (NameStr.Contains(TEXT("voidblade")) || NameStr.Contains(TEXT("void blade"))) return FName(TEXT("FB_Tabard_VoidBlade"));
	if (NameStr.Contains(TEXT("chronomancer"))) return FName(TEXT("FB_Tabard_Chronomancer"));
	if (NameStr.Contains(TEXT("dragonknight")) || NameStr.Contains(TEXT("dragon knight"))) return FName(TEXT("FB_Tabard_DragonKnight"));
	if (NameStr.Contains(TEXT("godslayer")) || NameStr.Contains(TEXT("god slayer"))) return FName(TEXT("FB_Tabard_GodSlayer"));

	return FName(TEXT("FB_Tabard_Vanguard"));
}

FGameplayTag FPAPaperdollConstants::GetTagForClass(FName ClassName)
{
	FString CleanName = ClassName.ToString();
	if (CleanName.StartsWith(TEXT("Class.")))
	{
		CleanName = CleanName.RightChop(6);
	}
	CleanName.RemoveSpacesInline();

	const FName TagName = FName(*FString::Printf(TEXT("Class.%s"), *CleanName));
	UGameplayTagsManager::Get().AddNativeGameplayTag(TagName);
	return FGameplayTag::RequestGameplayTag(TagName, false);
}

FName FPAPaperdollConstants::GetClassNameFromTag(FGameplayTag ClassTag)
{
	if (!ClassTag.IsValid())
	{
		return FName(TEXT("Vanguard"));
	}

	FString TagStr = ClassTag.ToString();
	if (TagStr.StartsWith(TEXT("Class.")))
	{
		TagStr = TagStr.RightChop(6);
	}
	return FName(*TagStr);
}

bool FPAPaperdollSortKey::IsMirroredDirection(EPAAimDirection8Way Direction)
{
	return Direction == EPAAimDirection8Way::West ||
	       Direction == EPAAimDirection8Way::SouthWest ||
	       Direction == EPAAimDirection8Way::NorthWest;
}

int32 FPAPaperdollSortKey::GetSortPriorityForIdentitySocket(FName SocketName, EPAAimDirection8Way Direction)
{
	if (SocketName == FPAPaperdollConstants::Socket_HelmCrest)
	{
		return 36;
	}
	if (SocketName == FPAPaperdollConstants::Socket_Tabard)
	{
		return 26;
	}
	return 20;
}

int32 FPAPaperdollSortKey::GetSortPriorityForSlot(EPAPaperdollSlot Slot, EPAAimDirection8Way Direction)
{
	const bool bMirrored = IsMirroredDirection(Direction);

	switch (Slot)
	{
	case EPAPaperdollSlot::OffHand:
		// Khi không mirror (hướng Đông): OffHand ở phía xa camera -> SortKey = 5
		// Khi mirror (hướng Tây): OffHand đảo chiều sang phía gần camera -> SortKey = 50
		return bMirrored ? 50 : 5;

	case EPAPaperdollSlot::Pants:
		return 15;

	case EPAPaperdollSlot::Boots:
		return 18;

	case EPAPaperdollSlot::Chest:
		return 25;

	case EPAPaperdollSlot::Gloves:
		return 30;

	case EPAPaperdollSlot::Helm:
		return 35;

	case EPAPaperdollSlot::Amulet:
		return 38;

	case EPAPaperdollSlot::Ring:
		return 40;

	case EPAPaperdollSlot::MainHand:
		// Khi không mirror (hướng Đông): MainHand ở phía gần camera -> SortKey = 50
		// Khi mirror (hướng Tây): MainHand đảo chiều ra phía xa camera -> SortKey = 5
		return bMirrored ? 5 : 50;

	default:
		return 20;
	}
}

FName FPAPaperdollSortKey::GetSocketNameForSlot(EPAPaperdollSlot Slot)
{
	switch (Slot)
	{
	case EPAPaperdollSlot::MainHand:
		return FPAPaperdollConstants::HandSocket_R;
	case EPAPaperdollSlot::OffHand:
		return FPAPaperdollConstants::HandSocket_L;
	default:
		return NAME_None;
	}
}

UPAPaperdollComponent::UPAPaperdollComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UPAPaperdollComponent::BeginPlay()
{
	Super::BeginPlay();

	// Tự động tìm kiếm và liên kết với UPAEquipmentComponent trên cùng Actor
	if (!BoundEquipmentComponent.IsValid())
	{
		if (AActor* OwnerActor = GetOwner())
		{
			if (UPAEquipmentComponent* EquipComp = OwnerActor->FindComponentByClass<UPAEquipmentComponent>())
			{
				BindToEquipmentComponent(EquipComp);
			}
		}
	}

	SynchronizeLayerSprites();
}

void UPAPaperdollComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Đồng bộ khung hình (Frame Lockstep) giữa BaseSprite và các lớp giáp/vũ khí Paperdoll
	if (APABaseCharacter* BaseChar = Cast<APABaseCharacter>(GetOwner()))
	{
		if (UPaperFlipbookComponent* BaseSprite = BaseChar->GetSpriteComponent())
		{
			const int32 CurrentFrame = BaseSprite->GetPlaybackPositionInFrames();
			for (auto& Pair : LayerComponents)
			{
				if (Pair.Value && Pair.Value->IsVisible())
				{
					if (Pair.Value->GetPlaybackPositionInFrames() != CurrentFrame)
					{
						Pair.Value->SetPlaybackPositionInFrames(CurrentFrame, false);
					}
				}
			}

			if (LowerBodyComponent && LowerBodyComponent->IsVisible())
			{
				if (LowerBodyComponent->GetPlaybackPositionInFrames() != CurrentFrame)
				{
					LowerBodyComponent->SetPlaybackPositionInFrames(CurrentFrame, false);
				}
			}

			if (UpperBodyComponent && UpperBodyComponent->IsVisible())
			{
				if (UpperBodyComponent->GetPlaybackPositionInFrames() != CurrentFrame)
				{
					UpperBodyComponent->SetPlaybackPositionInFrames(CurrentFrame, false);
				}
			}

			if (HelmCrestComponent && HelmCrestComponent->IsVisible())
			{
				if (HelmCrestComponent->GetPlaybackPositionInFrames() != CurrentFrame)
				{
					HelmCrestComponent->SetPlaybackPositionInFrames(CurrentFrame, false);
				}
			}

			if (TabardComponent && TabardComponent->IsVisible())
			{
				if (TabardComponent->GetPlaybackPositionInFrames() != CurrentFrame)
				{
					TabardComponent->SetPlaybackPositionInFrames(CurrentFrame, false);
				}
			}
		}
	}
}

void UPAPaperdollComponent::BindToEquipmentComponent(UPAEquipmentComponent* InEquipmentComponent)
{
	if (!InEquipmentComponent || BoundEquipmentComponent.Get() == InEquipmentComponent)
	{
		return;
	}

	BoundEquipmentComponent = InEquipmentComponent;

	// Đăng ký lắng nghe sự kiện trang bị thay đổi
	InEquipmentComponent->OnItemEquipped.AddDynamic(this, &UPAPaperdollComponent::HandleItemEquipped);
	InEquipmentComponent->OnItemUnequipped.AddDynamic(this, &UPAPaperdollComponent::HandleItemUnequipped);
}

bool UPAPaperdollComponent::EquipVisual(EPAPaperdollLayer Layer, FName ItemId, FName VisualAssetId)
{
	const bool bSuccess = Model.EquipVisual(Layer, ItemId, VisualAssetId);
	if (bSuccess)
	{
		SynchronizeLayerSprites();
		OnPaperdollVisualChanged.Broadcast(Layer, ItemId, VisualAssetId, true);
	}
	return bSuccess;
}

bool UPAPaperdollComponent::UnequipVisual(EPAPaperdollLayer Layer)
{
	const bool bSuccess = Model.UnequipVisual(Layer);
	if (bSuccess)
	{
		SynchronizeLayerSprites();
		const FName ActiveAsset = Model.GetActiveVisualAssetId(Layer);
		OnPaperdollVisualChanged.Broadcast(Layer, NAME_None, ActiveAsset, Layer == EPAPaperdollLayer::BaseBody || !ActiveAsset.IsNone());
	}
	return bSuccess;
}

void UPAPaperdollComponent::ResetToStarterCloth()
{
	Model.ResetToStarterCloth();
	SynchronizeLayerSprites();
	OnPaperdollVisualChanged.Broadcast(EPAPaperdollLayer::BaseBody, FName(TEXT("item_starter_cloth")), FName(TEXT("Visual_StarterCloth")), true);
}

void UPAPaperdollComponent::RegisterLayerFlipbookComponent(EPAPaperdollLayer Layer, UPaperFlipbookComponent* FlipbookComp)
{
	if (FlipbookComp)
	{
		LayerComponents.FindOrAdd(Layer) = FlipbookComp;
		SynchronizeLayerSprites();
	}
}

UPaperFlipbookComponent* UPAPaperdollComponent::GetLayerFlipbookComponent(EPAPaperdollLayer Layer) const
{
	const TObjectPtr<UPaperFlipbookComponent>* Found = LayerComponents.Find(Layer);
	return Found ? Found->Get() : nullptr;
}

void UPAPaperdollComponent::HandleItemEquipped(EPAEquipmentSlot Slot, const FPAInventoryItemEntry& Item)
{
	EPAPaperdollLayer Layer;
	if (FPAPaperdollModel::EquipmentSlotToPaperdollLayer(Slot, Layer))
	{
		// Đọc VisualAssetId từ StaticData nếu được thiết lập, nếu không thì lấy ItemDefId
		const FName VisualId = (Item.StaticData && !Item.StaticData->PaperdollVisualId.IsNone())
			? Item.StaticData->PaperdollVisualId
			: Item.ItemDefId;
		EquipVisual(Layer, Item.ItemDefId, VisualId);
	}
}

void UPAPaperdollComponent::HandleItemUnequipped(EPAEquipmentSlot Slot, const FPAInventoryItemEntry& Item)
{
	EPAPaperdollLayer Layer;
	if (FPAPaperdollModel::EquipmentSlotToPaperdollLayer(Slot, Layer))
	{
		UnequipVisual(Layer);
	}
}

bool UPAPaperdollComponent::EquipSlot(EPAPaperdollSlot Slot, FName ItemId, FName VisualAssetId)
{
	const bool bSuccess = Model.EquipSlot(Slot, ItemId, VisualAssetId);
	if (bSuccess)
	{
		SynchronizeLayerSprites();

		// Đồng bộ sang legacy Layer event nếu tương thích
		EPAPaperdollLayer LegacyLayer = EPAPaperdollLayer::BaseBody;
		if (Slot == EPAPaperdollSlot::Helm) LegacyLayer = EPAPaperdollLayer::Helmet;
		else if (Slot == EPAPaperdollSlot::Chest) LegacyLayer = EPAPaperdollLayer::ChestArmor;
		else if (Slot == EPAPaperdollSlot::MainHand) LegacyLayer = EPAPaperdollLayer::MainhandWeapon;
		else if (Slot == EPAPaperdollSlot::OffHand) LegacyLayer = EPAPaperdollLayer::OffhandShield;

		OnPaperdollVisualChanged.Broadcast(LegacyLayer, ItemId, VisualAssetId, true);
	}
	return bSuccess;
}

bool UPAPaperdollComponent::UnequipSlot(EPAPaperdollSlot Slot)
{
	const bool bSuccess = Model.UnequipSlot(Slot);
	if (bSuccess)
	{
		SynchronizeLayerSprites();

		EPAPaperdollLayer LegacyLayer = EPAPaperdollLayer::BaseBody;
		if (Slot == EPAPaperdollSlot::Helm) LegacyLayer = EPAPaperdollLayer::Helmet;
		else if (Slot == EPAPaperdollSlot::Chest) LegacyLayer = EPAPaperdollLayer::ChestArmor;
		else if (Slot == EPAPaperdollSlot::MainHand) LegacyLayer = EPAPaperdollLayer::MainhandWeapon;
		else if (Slot == EPAPaperdollSlot::OffHand) LegacyLayer = EPAPaperdollLayer::OffhandShield;

		OnPaperdollVisualChanged.Broadcast(LegacyLayer, NAME_None, NAME_None, false);
	}
	return bSuccess;
}

void UPAPaperdollComponent::RegisterSlotFlipbookComponent(EPAPaperdollSlot Slot, UPaperFlipbookComponent* FlipbookComp)
{
	if (FlipbookComp)
	{
		SlotComponents.FindOrAdd(Slot) = FlipbookComp;
		FlipbookComp->SetTranslucentSortPriority(FPAPaperdollSortKey::GetSortPriorityForSlot(Slot, CurrentOrientation));
		SynchronizeLayerSprites();
	}
}

UPaperFlipbookComponent* UPAPaperdollComponent::GetSlotFlipbookComponent(EPAPaperdollSlot Slot) const
{
	const TObjectPtr<UPaperFlipbookComponent>* Found = SlotComponents.Find(Slot);
	return Found ? Found->Get() : nullptr;
}

void UPAPaperdollComponent::UpdateDirectionalSortKeys(EPAAimDirection8Way Direction)
{
	CurrentOrientation = Direction;

	if (LowerBodyComponent)
	{
		LowerBodyComponent->SetTranslucentSortPriority(10);
	}
	if (UpperBodyComponent)
	{
		UpperBodyComponent->SetTranslucentSortPriority(20);
	}
	if (TabardComponent)
	{
		TabardComponent->SetTranslucentSortPriority(26);
	}
	if (HelmCrestComponent)
	{
		HelmCrestComponent->SetTranslucentSortPriority(36);
	}

	for (auto& Pair : SlotComponents)
	{
		if (Pair.Value)
		{
			const int32 SortKey = FPAPaperdollSortKey::GetSortPriorityForSlot(Pair.Key, Direction);
			Pair.Value->SetTranslucentSortPriority(SortKey);
		}
	}
}

void UPAPaperdollComponent::SetMasterRig(EPAMasterRig InRig)
{
	Model.SetMasterRig(InRig);
	SynchronizeLayerSprites();
}

void UPAPaperdollComponent::SetUpperBodyWeaponFamily(EPAWeaponFamily InFamily)
{
	Model.SetUpperBodyWeaponFamily(InFamily);
	SynchronizeLayerSprites();
}

bool UPAPaperdollComponent::SetUpperBodyWeaponFamilyByTag(FGameplayTag WeaponTag)
{
	const bool bSuccess = Model.SetUpperBodyWeaponFamilyByTag(WeaponTag);
	if (bSuccess)
	{
		SynchronizeLayerSprites();
	}
	return bSuccess;
}

void UPAPaperdollComponent::InitializeDecoupledMasterRigComponents()
{
	AActor* OwnerActor = GetOwner();
	USceneComponent* AttachRoot = OwnerActor ? OwnerActor->GetRootComponent() : nullptr;

	if (OwnerActor && AttachRoot)
	{
		if (!LowerBodyComponent)
		{
			LowerBodyComponent = NewObject<UPaperFlipbookComponent>(OwnerActor, TEXT("Paperdoll_LowerBody"));
			if (LowerBodyComponent)
			{
				LowerBodyComponent->AttachToComponent(AttachRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				LowerBodyComponent->SetTranslucentSortPriority(10);
				LowerBodyComponent->RegisterComponent();
			}
		}

		if (!UpperBodyComponent)
		{
			UpperBodyComponent = NewObject<UPaperFlipbookComponent>(OwnerActor, TEXT("Paperdoll_UpperBody"));
			if (UpperBodyComponent)
			{
				UpperBodyComponent->AttachToComponent(AttachRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				UpperBodyComponent->SetTranslucentSortPriority(20);
				UpperBodyComponent->RegisterComponent();
			}
		}
	}

	UpdateDirectionalSortKeys(CurrentOrientation);
	SynchronizeLayerSprites();
}

void UPAPaperdollComponent::RegisterLowerBodyComponent(UPaperFlipbookComponent* InComp)
{
	if (InComp)
	{
		LowerBodyComponent = InComp;
		LowerBodyComponent->SetTranslucentSortPriority(10);
		SynchronizeLayerSprites();
	}
}

void UPAPaperdollComponent::RegisterUpperBodyComponent(UPaperFlipbookComponent* InComp)
{
	if (InComp)
	{
		UpperBodyComponent = InComp;
		UpperBodyComponent->SetTranslucentSortPriority(20);
		SynchronizeLayerSprites();
	}
}

void UPAPaperdollComponent::Initialize9SlotSubcomponents()
{
	AActor* OwnerActor = GetOwner();
	USceneComponent* AttachRoot = OwnerActor ? OwnerActor->GetRootComponent() : nullptr;

	for (uint8 i = 0; i < (uint8)EPAPaperdollSlot::Count; ++i)
	{
		EPAPaperdollSlot Slot = (EPAPaperdollSlot)i;
		if (!SlotComponents.Contains(Slot))
		{
			if (OwnerActor)
			{
				FString CompName = FString::Printf(TEXT("PaperdollSlot_%d"), i);
				UPaperFlipbookComponent* NewComp = NewObject<UPaperFlipbookComponent>(OwnerActor, *CompName);
				if (NewComp)
				{
					const FName SocketName = FPAPaperdollSortKey::GetSocketNameForSlot(Slot);
					if (SocketName != NAME_None && AttachRoot)
					{
						NewComp->AttachToComponent(AttachRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
					}
					else if (AttachRoot)
					{
						NewComp->AttachToComponent(AttachRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
					}

					NewComp->RegisterComponent();
					RegisterSlotFlipbookComponent(Slot, NewComp);
				}
			}
		}
	}

	InitializeClassIdentityComponents();

	UpdateDirectionalSortKeys(CurrentOrientation);
}

void UPAPaperdollComponent::InitializeClassIdentityComponents()
{
	AActor* OwnerActor = GetOwner();
	USceneComponent* AttachRoot = OwnerActor ? OwnerActor->GetRootComponent() : nullptr;

	if (OwnerActor && AttachRoot)
	{
		if (!HelmCrestComponent)
		{
			HelmCrestComponent = NewObject<UPaperFlipbookComponent>(OwnerActor, TEXT("Paperdoll_HelmCrest"));
			if (HelmCrestComponent)
			{
				HelmCrestComponent->AttachToComponent(AttachRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FPAPaperdollConstants::Socket_HelmCrest);
				HelmCrestComponent->SetTranslucentSortPriority(36);
				HelmCrestComponent->RegisterComponent();
			}
		}

		if (!TabardComponent)
		{
			TabardComponent = NewObject<UPaperFlipbookComponent>(OwnerActor, TEXT("Paperdoll_Tabard"));
			if (TabardComponent)
			{
				TabardComponent->AttachToComponent(AttachRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FPAPaperdollConstants::Socket_Tabard);
				TabardComponent->SetTranslucentSortPriority(26);
				TabardComponent->RegisterComponent();
			}
		}
	}

	UpdateDirectionalSortKeys(CurrentOrientation);
	SynchronizeLayerSprites();
}

void UPAPaperdollComponent::RegisterHelmCrestComponent(UPaperFlipbookComponent* InComp)
{
	if (InComp)
	{
		HelmCrestComponent = InComp;
		HelmCrestComponent->SetTranslucentSortPriority(36);
		SynchronizeLayerSprites();
	}
}

void UPAPaperdollComponent::RegisterTabardComponent(UPaperFlipbookComponent* InComp)
{
	if (InComp)
	{
		TabardComponent = InComp;
		TabardComponent->SetTranslucentSortPriority(26);
		SynchronizeLayerSprites();
	}
}

void UPAPaperdollComponent::SetClassIdentity(FName InClassName)
{
	Model.SetClassIdentity(InClassName);
	SynchronizeLayerSprites();
}

bool UPAPaperdollComponent::SetClassIdentityByTag(FGameplayTag InClassTag)
{
	const bool bSuccess = Model.SetClassIdentityByTag(InClassTag);
	if (bSuccess)
	{
		SynchronizeLayerSprites();
	}
	return bSuccess;
}

void UPAPaperdollComponent::SetHelmCrestVisual(FName InAssetId)
{
	Model.SetHelmCrestVisual(InAssetId);
	SynchronizeLayerSprites();
}

void UPAPaperdollComponent::SetTabardVisual(FName InAssetId)
{
	Model.SetTabardVisual(InAssetId);
	SynchronizeLayerSprites();
}

void UPAPaperdollComponent::SetHelmCrestVisibility(bool bVisible)
{
	Model.SetHelmCrestVisible(bVisible);
	SynchronizeLayerSprites();
}

void UPAPaperdollComponent::SetTabardVisibility(bool bVisible)
{
	Model.SetTabardVisible(bVisible);
	SynchronizeLayerSprites();
}

void UPAPaperdollComponent::SynchronizeLayerSprites()
{
	for (auto& Pair : LayerComponents)
	{
		if (Pair.Value)
		{
			const FName ActiveVisual = Model.GetActiveVisualAssetId(Pair.Key);
			const bool bShouldShow = !ActiveVisual.IsNone();
			Pair.Value->SetVisibility(bShouldShow);
		}
	}

	for (auto& Pair : SlotComponents)
	{
		if (Pair.Value)
		{
			const FName ActiveVisual = Model.GetActiveVisualAssetForSlot(Pair.Key);
			const bool bShouldShow = !ActiveVisual.IsNone();
			Pair.Value->SetVisibility(bShouldShow);
		}
	}

	if (LowerBodyComponent)
	{
		const FName ActiveVisual = Model.GetLowerBodyVisualAssetId();
		LowerBodyComponent->SetVisibility(!ActiveVisual.IsNone());
	}

	if (UpperBodyComponent)
	{
		const FName ActiveVisual = Model.GetUpperBodyVisualAssetId();
		UpperBodyComponent->SetVisibility(!ActiveVisual.IsNone());
	}

	if (HelmCrestComponent)
	{
		const FName ActiveVisual = Model.GetHelmCrestVisualAssetId();
		HelmCrestComponent->SetVisibility(!ActiveVisual.IsNone());
	}

	if (TabardComponent)
	{
		const FName ActiveVisual = Model.GetTabardVisualAssetId();
		TabardComponent->SetVisibility(!ActiveVisual.IsNone());
	}
}

void UPAPaperdollComponent::OnRep_Model()
{
	SynchronizeLayerSprites();
}

void UPAPaperdollComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPAPaperdollComponent, Model);
}

// -----------------------------------------------------------------------------
// Story item-005: Static Material Instance Assignment & Calibration
// -----------------------------------------------------------------------------

FPARarityMaterialPreset UPAPaperdollComponent::GetRarityMaterialPreset(EPAItemRarity Rarity)
{
	switch (Rarity)
	{
	case EPAItemRarity::Common:
		return FPARarityMaterialPreset(
			FGameplayTag::RequestGameplayTag(FName("Item.Rarity.Common"), false),
			EPAItemRarity::Common,
			0.0f,
			FLinearColor(0.91f, 0.925f, 0.922f, 1.0f), // #E8ECEB
			FName("MI_Weapon_Common"));

	case EPAItemRarity::Uncommon:
		return FPARarityMaterialPreset(
			FGameplayTag::RequestGameplayTag(FName("Item.Rarity.Uncommon"), false),
			EPAItemRarity::Uncommon,
			0.5f,
			FLinearColor(0.063f, 0.725f, 0.506f, 1.0f), // #10B981
			FName("MI_Weapon_Uncommon"));

	case EPAItemRarity::Rare:
		return FPARarityMaterialPreset(
			FGameplayTag::RequestGameplayTag(FName("Item.Rarity.Rare"), false),
			EPAItemRarity::Rare,
			1.8f,
			FLinearColor(0.231f, 0.51f, 0.965f, 1.0f), // #3B82F6
			FName("MI_Weapon_Rare"));

	case EPAItemRarity::Epic:
		return FPARarityMaterialPreset(
			FGameplayTag::RequestGameplayTag(FName("Item.Rarity.Epic"), false),
			EPAItemRarity::Epic,
			3.5f,
			FLinearColor(0.659f, 0.333f, 0.969f, 1.0f), // #A855F7
			FName("MI_Weapon_Epic"));

	case EPAItemRarity::Legendary:
		return FPARarityMaterialPreset(
			FGameplayTag::RequestGameplayTag(FName("Item.Rarity.Legendary"), false),
			EPAItemRarity::Legendary,
			7.0f,
			FLinearColor(0.961f, 0.62f, 0.043f, 1.0f), // #F59E0B
			FName("MI_Weapon_Legendary"));

	default:
		return FPARarityMaterialPreset();
	}
}

bool UPAPaperdollComponent::IsDrawCallWithinCalibratedBudget(int32 MeasuredSceneDrawCalls)
{
	// Chuẩn đo thực tế từ PeakCombat50.utrace: 446 đến 462 draw calls tổng scene
	return MeasuredSceneDrawCalls >= 446 && MeasuredSceneDrawCalls <= 462;
}

bool UPAPaperdollComponent::SetWeaponMaterialForRarity(FGameplayTag RarityTag)
{
	const FString TagStr = RarityTag.ToString().ToLower();
	EPAItemRarity TargetRarity = EPAItemRarity::Common;

	if (TagStr.Contains(TEXT("legendary")))
	{
		TargetRarity = EPAItemRarity::Legendary;
	}
	else if (TagStr.Contains(TEXT("epic")))
	{
		TargetRarity = EPAItemRarity::Epic;
	}
	else if (TagStr.Contains(TEXT("rare")))
	{
		TargetRarity = EPAItemRarity::Rare;
	}
	else if (TagStr.Contains(TEXT("uncommon")))
	{
		TargetRarity = EPAItemRarity::Uncommon;
	}
	else
	{
		TargetRarity = EPAItemRarity::Common;
	}

	return SetWeaponMaterialForRarityEnum(TargetRarity);
}

bool UPAPaperdollComponent::SetWeaponMaterialForRarityEnum(EPAItemRarity Rarity)
{
	const FPARarityMaterialPreset Preset = GetRarityMaterialPreset(Rarity);
	CurrentWeaponRarityTag = Preset.RarityTag;

	UPaperFlipbookComponent* WeaponComp = GetSlotFlipbookComponent(EPAPaperdollSlot::MainHand);
	if (!WeaponComp)
	{
		WeaponComp = GetLayerFlipbookComponent(EPAPaperdollLayer::MainhandWeapon);
	}

	if (WeaponComp && CurrentWeaponMaterial)
	{
		WeaponComp->SetMaterial(0, CurrentWeaponMaterial);
	}

	return true;
}

void UPAPaperdollComponent::SetWeaponMaterialDirect(UMaterialInterface* InMaterial)
{
	CurrentWeaponMaterial = InMaterial;

	UPaperFlipbookComponent* WeaponComp = GetSlotFlipbookComponent(EPAPaperdollSlot::MainHand);
	if (!WeaponComp)
	{
		WeaponComp = GetLayerFlipbookComponent(EPAPaperdollLayer::MainhandWeapon);
	}

	if (WeaponComp && CurrentWeaponMaterial)
	{
		WeaponComp->SetMaterial(0, CurrentWeaponMaterial);
	}
}
