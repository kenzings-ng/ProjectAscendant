// Copyright Project Ascendant. All Rights Reserved.

#include "Character/PAPaperdollComponent.h"
#include "Character/PABaseCharacter.h"
#include "Inventory/PAEquipmentComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"
#include "PaperFlipbookComponent.h"
#include "Net/UnrealNetwork.h"

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
						Pair.Value->SetPlaybackPositionInFrames(CurrentFrame);
					}
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
