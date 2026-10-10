// Copyright Project Ascendant. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "UObject/Package.h"
#include "Controller/PABasePlayerController.h"
#include "Crafting/PABlacksmithComponent.h"
#include "Economy/PACurrencyComponent.h"
#include "Economy/PAMerchantComponent.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"
#include "Network/PAServiceRequestComponent.h"
#include "Network/PAServiceRequestTestProbe.h"
#include "UI/PABlacksmithForgeWidget.h"
#include "UI/PAMerchantShopWidget.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * X11b: player-routed service requests and server-confirmed UI.
 *
 * "Client" simulation: in a standalone automation world a Server RPC is absorbed when the calling actor has no
 * authority (AActor::GetFunctionCallspace), while a Client RPC still runs locally. Setting the PlayerController's
 * role to ROLE_AutonomousProxy therefore reproduces a remote client whose request has been sent but whose
 * confirmation has not arrived yet; Client_Confirm* is then delivered explicitly. With ROLE_Authority the full
 * request -> server validation -> confirmation path runs synchronously (same as listen-host / standalone).
 */

namespace PAServiceRoutingTestHelper
{
	static AActor* SpawnLocatedActor(UWorld* World, const FVector& Location, AActor* Owner)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Params.Owner = Owner;
		AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, Params);
		if (Actor && !Actor->GetRootComponent())
		{
			USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("TestRoot"));
			Actor->SetRootComponent(Root);
			Root->RegisterComponent();
		}
		if (Actor)
		{
			Actor->SetActorLocation(Location);
		}
		return Actor;
	}

	struct FRoutedPlayer
	{
		APlayerController* PC = nullptr;
		APawn* Pawn = nullptr;
		UPAInventoryComponent* Inventory = nullptr;
		UPACurrencyComponent* Wallet = nullptr;
		UPAServiceRequestComponent* Router = nullptr;
	};

	static FRoutedPlayer SpawnRoutedPlayer(UWorld* World, const FVector& PawnLocation)
	{
		FRoutedPlayer Player;
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Player.PC = World->SpawnActor<APlayerController>(APlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
		Player.Pawn = Cast<APawn>(World->SpawnActor<APawn>(APawn::StaticClass(), PawnLocation, FRotator::ZeroRotator, Params));
		if (Player.Pawn && !Player.Pawn->GetRootComponent())
		{
			USceneComponent* Root = NewObject<USceneComponent>(Player.Pawn, TEXT("TestRoot"));
			Player.Pawn->SetRootComponent(Root);
			Root->RegisterComponent();
			Player.Pawn->SetActorLocation(PawnLocation);
		}
		if (Player.PC && Player.Pawn)
		{
			Player.Pawn->SetOwner(Player.PC);
			Player.PC->SetPawn(Player.Pawn);
			Player.Inventory = NewObject<UPAInventoryComponent>(Player.Pawn, TEXT("TestInventory"));
			Player.Wallet = NewObject<UPACurrencyComponent>(Player.Pawn, TEXT("TestWallet"));
			Player.Router = NewObject<UPAServiceRequestComponent>(Player.PC, TEXT("TestRouter"));
			Player.Router->RegisterComponent();
		}
		return Player;
	}

	static void DestroyRoutedPlayer(FRoutedPlayer& Player)
	{
		if (Player.PC) { Player.PC->SetRole(ROLE_Authority); Player.PC->SetPawn(nullptr); }
		if (Player.Pawn) { Player.Pawn->Destroy(); }
		if (Player.PC) { Player.PC->Destroy(); }
		Player = FRoutedPlayer();
	}

	static UItemStaticDataAsset* MakeItem(FName ItemId, EPAItemCategory Category, EPAItemRarity Rarity, int32 BaseSellPrice, int32 MaxStack)
	{
		UItemStaticDataAsset* Asset = NewObject<UItemStaticDataAsset>();
		Asset->ItemId = ItemId;
		Asset->Category = Category;
		Asset->RarityTier = Rarity;
		Asset->BaseSellPrice = BaseSellPrice;
		Asset->MaxStackSize = MaxStack;
		return Asset;
	}
}

using namespace PAServiceRoutingTestHelper;

// =============================================================================
// 1. Production wiring: every APABasePlayerController owns a replicated router
// =============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAServiceRoutingPlayerControllerTest,
	"ProjectAscendant.Network.ServiceRouting.PlayerControllerOwnsRouter",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAServiceRoutingPlayerControllerTest::RunTest(const FString& Parameters)
{
	const APABasePlayerController* DefaultPC = GetDefault<APABasePlayerController>();
	const UPAServiceRequestComponent* Router = DefaultPC ? DefaultPC->GetServiceRequestComponent() : nullptr;
	if (!TestNotNull(TEXT("APABasePlayerController has a UPAServiceRequestComponent"), Router))
	{
		return false;
	}
	TestTrue(TEXT("Router is replicated (required for its RPCs)"), Router->GetIsReplicated());
	return true;
}

// =============================================================================
// 2. Shop widget: completion only on server confirmation
// =============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAServiceRoutingShopWidgetTest,
	"ProjectAscendant.Network.ServiceRouting.ShopWidgetWaitsForConfirmation",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAServiceRoutingShopWidgetTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!TestNotNull(TEXT("World available"), World))
	{
		return false;
	}

	FRoutedPlayer Player = SpawnRoutedPlayer(World, FVector(0.0f, 0.0f, 0.0f));
	AActor* Shop = SpawnLocatedActor(World, FVector(100.0f, 0.0f, 0.0f), nullptr); // NPC merchant, no player owner
	UPAMerchantComponent* Merchant = NewObject<UPAMerchantComponent>(Shop, TEXT("TestMerchant"));
	if (!TestNotNull(TEXT("Player"), Player.Router) || !TestNotNull(TEXT("Merchant"), Merchant))
	{
		if (Shop) { Shop->Destroy(); }
		DestroyRoutedPlayer(Player);
		return false;
	}

	UItemStaticDataAsset* Potion = MakeItem(FName("item_test_potion"), EPAItemCategory::Consumable, EPAItemRarity::Common, 10, 99);
	FPAMerchantCatalogEntry CatalogEntry;
	CatalogEntry.ItemData = Potion;
	CatalogEntry.PriceGold = 10;
	CatalogEntry.AvailableStock = -1;
	Merchant->AddCatalogEntry(CatalogEntry);

	EPACurrencyTransactionError CurrErr;
	Player.Wallet->AddCurrency(EPACurrencyType::Gold, 100, CurrErr);

	UPAMerchantShopWidget* Widget = NewObject<UPAMerchantShopWidget>(GetTransientPackage());
	UPAServiceRequestTestProbe* Probe = NewObject<UPAServiceRequestTestProbe>(GetTransientPackage());
	Widget->OnTransactionCompleted.AddDynamic(Probe, &UPAServiceRequestTestProbe::HandleShopCompleted);
	Widget->OnTransactionRejected.AddDynamic(Probe, &UPAServiceRequestTestProbe::HandleShopRejected);
	Widget->InitializeShop(Merchant, Player.Inventory, Player.Wallet, 0, Player.Router);
	Widget->SelectCatalogItem(0);

	// --- Remote client: request sent, no confirmation yet -> nothing fires, model unchanged ---
	Player.PC->SetRole(ROLE_AutonomousProxy);
	const int32 BaseId = Player.Router->AllocateRequestId();
	Widget->ExecuteBuy();
	TestTrue(TEXT("Client: request pending after ExecuteBuy"), Widget->IsRequestPending());
	TestEqual(TEXT("Client: OnTransactionCompleted not fired before confirmation"), Probe->ShopCompletedCount, 0);
	TestEqual(TEXT("Client: OnTransactionRejected not fired before confirmation"), Probe->ShopRejectedCount, 0);
	TestEqual(TEXT("Client: no local gold change"), Player.Wallet->GetGold(), 100LL);
	TestEqual(TEXT("Client: no local item added"), Player.Inventory->GetItemCount(FName("item_test_potion")), 0);

	Widget->ExecuteBuy(); // second click while pending: ignored (no new request id consumed)
	TestEqual(TEXT("Client: second click while pending sends nothing"), Player.Router->AllocateRequestId(), BaseId + 2);

	// Confirmation for some other request: ignored.
	Player.Router->Client_ConfirmMerchantRequest(BaseId + 100, true, EPATransactionError::None);
	TestEqual(TEXT("Client: foreign confirmation ignored"), Probe->ShopCompletedCount, 0);
	TestTrue(TEXT("Client: still pending"), Widget->IsRequestPending());

	// Server rejection for our request: Rejected fires, Completed does not.
	Player.Router->Client_ConfirmMerchantRequest(BaseId + 1, false, EPATransactionError::DistanceExceeded);
	TestEqual(TEXT("Client: rejection delivered"), Probe->ShopRejectedCount, 1);
	TestEqual(TEXT("Client: rejection error forwarded"), Probe->LastShopError, EPATransactionError::DistanceExceeded);
	TestEqual(TEXT("Client: Completed not fired on rejection"), Probe->ShopCompletedCount, 0);
	TestFalse(TEXT("Client: no longer pending"), Widget->IsRequestPending());

	// --- Authority (listen-host / standalone): request -> server -> confirmation in one call ---
	Player.PC->SetRole(ROLE_Authority);
	Widget->ExecuteBuy();
	TestEqual(TEXT("Host: Completed fired once after server confirmation"), Probe->ShopCompletedCount, 1);
	TestFalse(TEXT("Host: not pending"), Widget->IsRequestPending());
	TestEqual(TEXT("Host: server charged 10 Gold"), Player.Wallet->GetGold(), 90LL);
	TestEqual(TEXT("Host: potion added"), Player.Inventory->GetItemCount(FName("item_test_potion")), 1);
	TestEqual(TEXT("Host: widget gold refreshed from wallet"), Widget->GetPlayerGold(), 90);

	// --- Out of range (310 cm): server rejects, widget reports rejection only ---
	Player.Pawn->SetActorLocation(FVector(410.0f, 0.0f, 0.0f));
	Widget->ExecuteBuy();
	TestEqual(TEXT("Host 310 cm: rejection delivered"), Probe->ShopRejectedCount, 2);
	TestEqual(TEXT("Host 310 cm: DistanceExceeded"), Probe->LastShopError, EPATransactionError::DistanceExceeded);
	TestEqual(TEXT("Host 310 cm: Completed not fired"), Probe->ShopCompletedCount, 1);
	TestEqual(TEXT("Host 310 cm: gold untouched"), Player.Wallet->GetGold(), 90LL);

	Shop->Destroy();
	DestroyRoutedPlayer(Player);
	return true;
}

// =============================================================================
// 3. Forge widget: completion only on server confirmation
// =============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAServiceRoutingForgeWidgetTest,
	"ProjectAscendant.Network.ServiceRouting.ForgeWidgetWaitsForConfirmation",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAServiceRoutingForgeWidgetTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!TestNotNull(TEXT("World available"), World))
	{
		return false;
	}

	FRoutedPlayer Player = SpawnRoutedPlayer(World, FVector(0.0f, 0.0f, 0.0f));
	AActor* ForgeActor = SpawnLocatedActor(World, FVector(100.0f, 0.0f, 0.0f), nullptr); // NPC forge, no player owner
	UPABlacksmithComponent* Forge = NewObject<UPABlacksmithComponent>(ForgeActor, TEXT("TestForge"));
	if (!TestNotNull(TEXT("Player"), Player.Router) || !TestNotNull(TEXT("Forge"), Forge))
	{
		if (ForgeActor) { ForgeActor->Destroy(); }
		DestroyRoutedPlayer(Player);
		return false;
	}
	Forge->SetForgeTier(EPABlacksmithTier::Tier1_Outpost);

	// +0 -> +1: 100 Gold + 2 iron_ore, 100% success (crft-001 AC-3).
	Player.Inventory->AddItemToSlot(0, MakeItem(FName("item_test_sword"), EPAItemCategory::Equipment, EPAItemRarity::Rare, 300, 1), 1);
	Player.Inventory->AddItemToSlot(1, MakeItem(FName("iron_ore"), EPAItemCategory::Material, EPAItemRarity::None, 0, 999), 2);
	EPACurrencyTransactionError CurrErr;
	Player.Wallet->AddCurrency(EPACurrencyType::Gold, 100, CurrErr);

	UPABlacksmithForgeWidget* Widget = NewObject<UPABlacksmithForgeWidget>(GetTransientPackage());
	UPAServiceRequestTestProbe* Probe = NewObject<UPAServiceRequestTestProbe>(GetTransientPackage());
	Widget->OnTransactionCompleted.AddDynamic(Probe, &UPAServiceRequestTestProbe::HandleForgeCompleted);
	Widget->InitializeForge(Forge, Player.Inventory, Player.Wallet, Player.Router);
	Widget->SetTargetEquipmentSlot(0);

	auto EnhancementLevel = [&Player]() { return Player.Inventory->GetItemAtSlot(0)->DynamicData.EnhancementLevel; };

	// --- Remote client: hold completes, request sent, no confirmation yet ---
	Player.PC->SetRole(ROLE_AutonomousProxy);
	const int32 BaseId = Player.Router->AllocateRequestId();
	Widget->UpdateHoldInputWithDelta(1.0f, true);
	TestTrue(TEXT("Client: hold completed"), Widget->IsHoldCompleted());
	TestTrue(TEXT("Client: request pending"), Widget->IsRequestPending());
	TestEqual(TEXT("Client: OnTransactionCompleted not fired before confirmation"), Probe->ForgeCompletedCount, 0);
	TestEqual(TEXT("Client: no local enhancement"), EnhancementLevel(), 0);
	TestEqual(TEXT("Client: no local gold change"), Player.Wallet->GetGold(), 100LL);

	Player.Router->Client_ConfirmForgeRequest(BaseId + 100, true, EPACraftingError::None);
	TestEqual(TEXT("Client: foreign confirmation ignored"), Probe->ForgeCompletedCount, 0);

	Player.Router->Client_ConfirmForgeRequest(BaseId + 1, false, EPACraftingError::InCombat);
	TestEqual(TEXT("Client: confirmation fires completion once"), Probe->ForgeCompletedCount, 1);
	TestFalse(TEXT("Client: confirmation carries failure"), Probe->bLastForgeSuccess);
	TestEqual(TEXT("Client: confirmation carries error"), Probe->LastForgeError, EPACraftingError::InCombat);
	TestFalse(TEXT("Client: no longer pending"), Widget->IsRequestPending());

	// --- Authority: full routed path; success confirmed and applied by the server ---
	Player.PC->SetRole(ROLE_Authority);
	Widget->UpdateHoldInputWithDelta(0.0f, false); // release
	Widget->UpdateHoldInputWithDelta(1.0f, true);  // hold again
	TestEqual(TEXT("Host: completion fired after confirmation"), Probe->ForgeCompletedCount, 2);
	TestTrue(TEXT("Host: confirmed success"), Probe->bLastForgeSuccess);
	TestEqual(TEXT("Host: no error"), Probe->LastForgeError, EPACraftingError::None);
	TestEqual(TEXT("Host: server enhanced to +1"), EnhancementLevel(), 1);
	TestEqual(TEXT("Host: server charged 100 Gold"), Player.Wallet->GetGold(), 0LL);
	TestEqual(TEXT("Host: server consumed 2 iron ore"), Player.Inventory->GetItemCount(FName("iron_ore")), 0);

	ForgeActor->Destroy();
	DestroyRoutedPlayer(Player);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
