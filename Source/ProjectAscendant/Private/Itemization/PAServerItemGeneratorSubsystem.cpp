// Copyright Project Ascendant. All Rights Reserved.

#include "Itemization/PAServerItemGeneratorSubsystem.h"
#include "Itemization/PAAffixDataLoader.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameplayTagsManager.h"

UPAServerItemGeneratorSubsystem::UPAServerItemGeneratorSubsystem()
{
}

void UPAServerItemGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeAffixDefinitions(nullptr);
	RegisterConsoleCommands();
}

void UPAServerItemGeneratorSubsystem::Deinitialize()
{
	UnregisterConsoleCommands();

	Super::Deinitialize();
}

void UPAServerItemGeneratorSubsystem::InitializeAffixDefinitions(UDataTable* InDataTable)
{
	CachedAffixDefinitions.Empty();

	if (InDataTable)
	{
		AffixDataTable = InDataTable;
	}
	else if (!AffixDataTable)
	{
		AffixDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Itemization/Data/DT_AffixDefinitions"));
		if (!AffixDataTable)
		{
			AffixDataTable = UPAAffixDataLoader::CreateAffixDefinitionsTable(this);
		}
	}

	if (AffixDataTable)
	{
		TArray<FPAAffixDefinitionRow*> Rows;
		AffixDataTable->GetAllRows<FPAAffixDefinitionRow>(TEXT("UPAServerItemGeneratorSubsystem"), Rows);
		for (const FPAAffixDefinitionRow* Row : Rows)
		{
			if (Row)
			{
				CachedAffixDefinitions.Add(*Row);
			}
		}
	}
	else
	{
		CachedAffixDefinitions = UPAAffixDataLoader::GetCanonicalAffixDefinitions();
	}
}

FPASavedItemInstance UPAServerItemGeneratorSubsystem::GenerateItemInstance(
	FName ItemDefId,
	int32 ItemLevel,
	EPAItemRarity Rarity,
	EPAForgeTier ForgeTier)
{
	if (CachedAffixDefinitions.Num() == 0)
	{
		InitializeAffixDefinitions(nullptr);
	}

	const EPAForgeTier EffectiveForgeTier = (ForgeTier != EPAForgeTier::None)
		? ForgeTier
		: GetForgeTierFromItemLevel(ItemLevel);

	const int32 AffixTier = GetAffixTierFromItemLevel(ItemLevel);

	// 1. Phân bổ số lượng Prefix và Suffix theo Rarity (Story item-002):
	// - Common: 0 affix
	// - Uncommon: 1-2 (random 1 hoặc 2)
	// - Rare: 3 (2P + 1S)
	// - Epic: 4 (2P + 2S)
	// - Legendary: 4 (2P + 2S)
	int32 NumPrefixes = 0;
	int32 NumSuffixes = 0;

	switch (Rarity)
	{
	case EPAItemRarity::Common:
		NumPrefixes = 0;
		NumSuffixes = 0;
		break;

	case EPAItemRarity::Uncommon:
	{
		const int32 Total = FMath::RandRange(1, 2);
		if (Total == 1)
		{
			if (FMath::RandBool())
			{
				NumPrefixes = 1;
				NumSuffixes = 0;
			}
			else
			{
				NumPrefixes = 0;
				NumSuffixes = 1;
			}
		}
		else
		{
			NumPrefixes = 1;
			NumSuffixes = 1;
		}
		break;
	}

	case EPAItemRarity::Rare:
		NumPrefixes = 2;
		NumSuffixes = 1;
		break;

	case EPAItemRarity::Epic:
	case EPAItemRarity::Legendary:
		NumPrefixes = 2;
		NumSuffixes = 2;
		break;

	default:
		break;
	}

	// 2. Lọc candidate pool dùng TRỰC TIẾP FPAAffixDefinitionRow::IsUnlockedAtForgeTier()
	TArray<const FPAAffixDefinitionRow*> CandidatePrefixes;
	TArray<const FPAAffixDefinitionRow*> CandidateSuffixes;

	for (const FPAAffixDefinitionRow& Row : CachedAffixDefinitions)
	{
		if (Row.IsUnlockedAtForgeTier(EffectiveForgeTier))
		{
			if (Row.AffixType == EPAAffixType::Prefix)
			{
				CandidatePrefixes.Add(&Row);
			}
			else if (Row.AffixType == EPAAffixType::Suffix)
			{
				CandidateSuffixes.Add(&Row);
			}
		}
	}

	// 3. Roll Prefixes (không bao giờ trùng tên)
	TArray<FPAAffixInstance> RolledAffixes;
	TArray<const FPAAffixDefinitionRow*> AvailablePrefixes = CandidatePrefixes;

	for (int32 i = 0; i < NumPrefixes && AvailablePrefixes.Num() > 0; ++i)
	{
		const int32 PickIdx = FMath::RandRange(0, AvailablePrefixes.Num() - 1);
		const FPAAffixDefinitionRow* Row = AvailablePrefixes[PickIdx];
		AvailablePrefixes.RemoveAtSwap(PickIdx);

		float MinVal = 0.0f;
		float MaxVal = 0.0f;
		if (!Row->GetTierRange(AffixTier, MinVal, MaxVal))
		{
			for (int32 FallbackTier = 1; FallbackTier <= 4; ++FallbackTier)
			{
				if (Row->GetTierRange(FallbackTier, MinVal, MaxVal))
				{
					break;
				}
			}
		}

		float RolledVal = FMath::FRandRange(MinVal, MaxVal);
		if (MaxVal <= 1.0f)
		{
			RolledVal = FMath::RoundToFloat(RolledVal * 100.0f) / 100.0f;
		}
		else
		{
			RolledVal = FMath::RoundToFloat(RolledVal * 10.0f) / 10.0f;
		}

		// Áp dụng trần cứng HardCapPct nếu có (prefix_posture_dmg <= 35.0%, prefix_stagger_duration <= 0.5s)
		if (Row->HardCapPct > 0.0f && RolledVal > Row->HardCapPct)
		{
			RolledVal = Row->HardCapPct;
		}

		RolledAffixes.Add(FPAAffixInstance(Row->AffixName, Row->AffixType, RolledVal, AffixTier));
	}

	// 4. Roll Suffixes (không bao giờ trùng tên)
	TArray<const FPAAffixDefinitionRow*> AvailableSuffixes = CandidateSuffixes;

	for (int32 i = 0; i < NumSuffixes && AvailableSuffixes.Num() > 0; ++i)
	{
		const int32 PickIdx = FMath::RandRange(0, AvailableSuffixes.Num() - 1);
		const FPAAffixDefinitionRow* Row = AvailableSuffixes[PickIdx];
		AvailableSuffixes.RemoveAtSwap(PickIdx);

		float MinVal = 0.0f;
		float MaxVal = 0.0f;
		if (!Row->GetTierRange(AffixTier, MinVal, MaxVal))
		{
			for (int32 FallbackTier = 1; FallbackTier <= 4; ++FallbackTier)
			{
				if (Row->GetTierRange(FallbackTier, MinVal, MaxVal))
				{
					break;
				}
			}
		}

		float RolledVal = FMath::FRandRange(MinVal, MaxVal);
		if (MaxVal <= 1.0f)
		{
			RolledVal = FMath::RoundToFloat(RolledVal * 100.0f) / 100.0f;
		}
		else
		{
			RolledVal = FMath::RoundToFloat(RolledVal * 10.0f) / 10.0f;
		}

		if (Row->HardCapPct > 0.0f && RolledVal > Row->HardCapPct)
		{
			RolledVal = Row->HardCapPct;
		}

		RolledAffixes.Add(FPAAffixInstance(Row->AffixName, Row->AffixType, RolledVal, AffixTier));
	}

	// 5. Khởi tạo FPASavedItemInstance
	FPASavedItemInstance Item;
	Item.ItemInstanceUID = FGuid::NewGuid();
	Item.ItemDefId = ItemDefId.IsNone() ? FName("Item_IronSword") : ItemDefId;
	Item.ItemLevel = ItemLevel;
	Item.CurrentDurability = 100.0f;
	Item.MaxDurability = 100.0f;
	Item.RarityTag = GetRarityTag(Rarity);
	Item.ActiveAffixes = MoveTemp(RolledAffixes);

	// Thiết lập MaxSocketCapacity theo Rarity (Story item-002 & item-003):
	// Các ô ngọc khởi tạo ở trạng thái KHÓA (bIsUnlocked = false).
	// Trách nhiệm đục lỗ mở khóa (chuyển bIsUnlocked = true) thuộc về Thợ Rèn (Story item-003):
	// - Outpost Forge: Khóa 100% việc đục lỗ.
	// - Field Forge: Cho phép đục Regular socket (Rare tối đa 1, Epic tối đa 2).
	// - Forbidden Forge: Cho phép đục Prismatic socket (Legendary tối đa 2 Regular + 1 Prismatic).
	switch (Rarity)
	{
	case EPAItemRarity::Rare:
		Item.SocketSlots.Add(FPASocketSlot(EPASocketType::Regular, false));
		break;
	case EPAItemRarity::Epic:
		Item.SocketSlots.Add(FPASocketSlot(EPASocketType::Regular, false));
		Item.SocketSlots.Add(FPASocketSlot(EPASocketType::Regular, false));
		break;
	case EPAItemRarity::Legendary:
		Item.SocketSlots.Add(FPASocketSlot(EPASocketType::Regular, false));
		Item.SocketSlots.Add(FPASocketSlot(EPASocketType::Regular, false));
		Item.SocketSlots.Add(FPASocketSlot(EPASocketType::Prismatic, false));
		break;
	default:
		break;
	}

	return Item;
}

EPAForgeTier UPAServerItemGeneratorSubsystem::GetForgeTierFromItemLevel(int32 ItemLevel)
{
	if (ItemLevel <= 20)
	{
		return EPAForgeTier::Tier1_Outpost;
	}
	if (ItemLevel <= 35)
	{
		return EPAForgeTier::Tier2_Field;
	}
	return EPAForgeTier::Tier3_Forbidden;
}

int32 UPAServerItemGeneratorSubsystem::GetAffixTierFromItemLevel(int32 ItemLevel)
{
	if (ItemLevel <= 15)
	{
		return 1;
	}
	if (ItemLevel <= 30)
	{
		return 2;
	}
	if (ItemLevel <= 45)
	{
		return 3;
	}
	return 4;
}

FGameplayTag UPAServerItemGeneratorSubsystem::GetRarityTag(EPAItemRarity Rarity)
{
	static bool bTagsRegistered = false;
	if (!bTagsRegistered)
	{
		UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		Manager.AddNativeGameplayTag(FName("Item.Rarity.Common"));
		Manager.AddNativeGameplayTag(FName("Item.Rarity.Uncommon"));
		Manager.AddNativeGameplayTag(FName("Item.Rarity.Rare"));
		Manager.AddNativeGameplayTag(FName("Item.Rarity.Epic"));
		Manager.AddNativeGameplayTag(FName("Item.Rarity.Legendary"));
		bTagsRegistered = true;
	}

	switch (Rarity)
	{
	case EPAItemRarity::Common:
		return FGameplayTag::RequestGameplayTag(FName("Item.Rarity.Common"), false);
	case EPAItemRarity::Uncommon:
		return FGameplayTag::RequestGameplayTag(FName("Item.Rarity.Uncommon"), false);
	case EPAItemRarity::Rare:
		return FGameplayTag::RequestGameplayTag(FName("Item.Rarity.Rare"), false);
	case EPAItemRarity::Epic:
		return FGameplayTag::RequestGameplayTag(FName("Item.Rarity.Epic"), false);
	case EPAItemRarity::Legendary:
		return FGameplayTag::RequestGameplayTag(FName("Item.Rarity.Legendary"), false);
	default:
		return FGameplayTag::EmptyTag;
	}
}

EPAItemRarity UPAServerItemGeneratorSubsystem::ParseRarityFromString(const FString& InStr)
{
	if (InStr.Equals(TEXT("1"), ESearchCase::IgnoreCase) || InStr.Equals(TEXT("Common"), ESearchCase::IgnoreCase))
	{
		return EPAItemRarity::Common;
	}
	if (InStr.Equals(TEXT("2"), ESearchCase::IgnoreCase) || InStr.Equals(TEXT("Uncommon"), ESearchCase::IgnoreCase))
	{
		return EPAItemRarity::Uncommon;
	}
	if (InStr.Equals(TEXT("3"), ESearchCase::IgnoreCase) || InStr.Equals(TEXT("Rare"), ESearchCase::IgnoreCase))
	{
		return EPAItemRarity::Rare;
	}
	if (InStr.Equals(TEXT("4"), ESearchCase::IgnoreCase) || InStr.Equals(TEXT("Epic"), ESearchCase::IgnoreCase))
	{
		return EPAItemRarity::Epic;
	}
	if (InStr.Equals(TEXT("5"), ESearchCase::IgnoreCase) || InStr.Equals(TEXT("Legendary"), ESearchCase::IgnoreCase))
	{
		return EPAItemRarity::Legendary;
	}

	return EPAItemRarity::Common;
}

EPAForgeTier UPAServerItemGeneratorSubsystem::ParseForgeTierFromString(const FString& InStr)
{
	if (InStr.Equals(TEXT("1"), ESearchCase::IgnoreCase) || InStr.Equals(TEXT("Outpost"), ESearchCase::IgnoreCase))
	{
		return EPAForgeTier::Tier1_Outpost;
	}
	if (InStr.Equals(TEXT("2"), ESearchCase::IgnoreCase) || InStr.Equals(TEXT("Field"), ESearchCase::IgnoreCase))
	{
		return EPAForgeTier::Tier2_Field;
	}
	if (InStr.Equals(TEXT("3"), ESearchCase::IgnoreCase) || InStr.Equals(TEXT("Forbidden"), ESearchCase::IgnoreCase))
	{
		return EPAForgeTier::Tier3_Forbidden;
	}

	return EPAForgeTier::None;
}

void UPAServerItemGeneratorSubsystem::RegisterConsoleCommands()
{
	if (CheatGenerateItemCmd)
	{
		return;
	}

	IConsoleManager& ConsoleManager = IConsoleManager::Get();
	IConsoleObject* Existing = ConsoleManager.FindConsoleObject(TEXT("Cheat.GenerateItem"));
	if (!Existing)
	{
		CheatGenerateItemCmd = ConsoleManager.RegisterConsoleCommand(
			TEXT("Cheat.GenerateItem"),
			TEXT("Sinh trang bị ngẫu nhiên (Story item-002): Cheat.GenerateItem <iLvl> <Rarity: 1-5 hoặc Common/Uncommon/Rare/Epic/Legendary> [ForgeTier: 1-3]"),
			FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
			{
				UPAServerItemGeneratorSubsystem* Subsystem = nullptr;
				if (GEngine)
				{
					for (const FWorldContext& Context : GEngine->GetWorldContexts())
					{
						if (UGameInstance* GI = Context.OwningGameInstance)
						{
							Subsystem = GI->GetSubsystem<UPAServerItemGeneratorSubsystem>();
							if (Subsystem)
							{
								break;
							}
						}
					}
				}

				if (!Subsystem)
				{
					UE_LOG(LogTemp, Warning, TEXT("[Cheat.GenerateItem] Không tìm thấy UPAServerItemGeneratorSubsystem trong GameInstance!"));
					return;
				}

				ExecuteCheatGenerateItem(Args, Subsystem);
			}),
			ECVF_Cheat
		);
	}
}

void UPAServerItemGeneratorSubsystem::UnregisterConsoleCommands()
{
	if (CheatGenerateItemCmd)
	{
		IConsoleManager::Get().UnregisterConsoleObject(CheatGenerateItemCmd);
		CheatGenerateItemCmd = nullptr;
	}
}

bool UPAServerItemGeneratorSubsystem::ExecuteCheatGenerateItem(
	const TArray<FString>& Args,
	UPAServerItemGeneratorSubsystem* Subsystem,
	FPASavedItemInstance* OutItem)
{
	if (Args.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat.GenerateItem] Cú pháp: Cheat.GenerateItem <iLvl> <Rarity> [ForgeTier]"));
		return false;
	}

	if (!Subsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Cheat.GenerateItem] Subsystem là nullptr!"));
		return false;
	}

	const int32 iLvl = FMath::Clamp(FCString::Atoi(*Args[0]), 1, 50);
	const EPAItemRarity Rarity = ParseRarityFromString(Args[1]);

	EPAForgeTier ForgeTier = EPAForgeTier::None;
	if (Args.Num() >= 3)
	{
		ForgeTier = ParseForgeTierFromString(Args[2]);
	}

	const FPASavedItemInstance Generated = Subsystem->GenerateItemInstance(NAME_None, iLvl, Rarity, ForgeTier);

	FString AffixSummary;
	for (const FPAAffixInstance& Affix : Generated.ActiveAffixes)
	{
		AffixSummary += FString::Printf(TEXT("[%s: %.2f] "), *Affix.AffixId.ToString(), Affix.RolledValue);
	}

	UE_LOG(LogTemp, Display, TEXT("[Cheat.GenerateItem] Thành công: UID=%s, iLvl=%d, Rarity=%s, Sockets=%d, Affixes (%d): %s"),
		*Generated.ItemInstanceUID.ToString(),
		Generated.ItemLevel,
		*Generated.RarityTag.ToString(),
		Generated.SocketSlots.Num(),
		Generated.ActiveAffixes.Num(),
		*AffixSummary);

	if (OutItem)
	{
		*OutItem = Generated;
	}

	return true;
}
