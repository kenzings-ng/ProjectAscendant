// Copyright Project Ascendant. All Rights Reserved.

#include "UI/PACharacterSelectWidget.h"
#include "Account/PAAccountSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Styling/CoreStyle.h"

void UPACharacterSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AvailableClasses = FPACharacterClassRegistry::GetAllClasses();
	SelectClass(EPACharacterClass::Vanguard);
}

void UPACharacterSelectWidget::SelectClass(EPACharacterClass InClass)
{
	CurrentlySelectedClass = InClass;
	const FPACharacterClassInfo SelectedInfo = GetSelectedClassInfo();

	if (UPAAccountSubsystem* AccountSubsystem = GetAccountSubsystem())
	{
		AccountSubsystem->SetSelectedCharacterClass(FName(*SelectedInfo.ClassTag));
	}

	UpdateDetailsView(SelectedInfo);

	OnClassSelectionChanged.Broadcast(SelectedInfo);
	OnClassSelectedBP(SelectedInfo);
}

void UPACharacterSelectWidget::SelectNextClass()
{
	const uint8 CurrentIndex = static_cast<uint8>(CurrentlySelectedClass);
	const uint8 TotalClasses = static_cast<uint8>(3);
	const uint8 NextIndex = (CurrentIndex + 1) % TotalClasses;
	SelectClass(static_cast<EPACharacterClass>(NextIndex));
}

void UPACharacterSelectWidget::SelectPreviousClass()
{
	const uint8 CurrentIndex = static_cast<uint8>(CurrentlySelectedClass);
	const uint8 TotalClasses = static_cast<uint8>(3);
	const uint8 PrevIndex = (CurrentIndex + TotalClasses - 1) % TotalClasses;
	SelectClass(static_cast<EPACharacterClass>(PrevIndex));
}

FPACharacterClassInfo UPACharacterSelectWidget::GetSelectedClassInfo() const
{
	return FPACharacterClassRegistry::GetClassInfo(CurrentlySelectedClass);
}

void UPACharacterSelectWidget::ConfirmSelectionAndEnterWorld()
{
	const FPACharacterClassInfo SelectedInfo = GetSelectedClassInfo();

	if (UPAAccountSubsystem* AccountSubsystem = GetAccountSubsystem())
	{
		AccountSubsystem->SetSelectedCharacterClass(FName(*SelectedInfo.ClassTag));
	}

	OnEnteringWorldBP(TargetGameWorldMapName);

	RemoveFromParent();

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}

	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, TargetGameWorldMapName);
	}
}

UPAAccountSubsystem* UPACharacterSelectWidget::GetAccountSubsystem() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UPAAccountSubsystem>();
	}
	return nullptr;
}

void UPACharacterSelectWidget::UpdateDetailsView(const FPACharacterClassInfo& Info)
{
	if (SelectedClassNameText.IsValid())
	{
		SelectedClassNameText->SetText(Info.DisplayName);
	}
	if (SelectedClassTaglineText.IsValid())
	{
		SelectedClassTaglineText->SetText(Info.Tagline);
	}
	if (SelectedClassLoreText.IsValid())
	{
		SelectedClassLoreText->SetText(Info.LoreDescription);
	}
	if (SelectedClassRoleText.IsValid())
	{
		SelectedClassRoleText->SetText(FText::Format(NSLOCTEXT("ProjectAscendant", "RoleFormat", "Trường Phái: {0}"), Info.PrimaryRole));
	}
	if (SelectedClassWeaponText.IsValid())
	{
		SelectedClassWeaponText->SetText(FText::Format(NSLOCTEXT("ProjectAscendant", "WeaponFormat", "Vũ Khí Khởi Đầu: {0}"), Info.PrimaryWeapon));
	}
	if (SelectedClassStatsText.IsValid())
	{
		const FString StatsString = FString::Printf(
			TEXT("Máu Tối Đa (Max HP): %d    |    Thể Lực (Stamina): %d    |    Nội Lực (Mana): %d\nThế Đứng (Posture): %d     |    Tốc Độ Chạy: %d cm/s"),
			FMath::RoundToInt(Info.BaseHealth),
			FMath::RoundToInt(Info.BaseStamina),
			FMath::RoundToInt(Info.BaseMana),
			FMath::RoundToInt(Info.BasePosture),
			FMath::RoundToInt(Info.MoveSpeed)
		);
		SelectedClassStatsText->SetText(FText::FromString(StatsString));
	}
	if (SelectedClassAbilitiesText.IsValid())
	{
		FString AbilitiesString = TEXT("Bộ Kỹ Năng Khởi Đầu (Starter Kit):\n");
		for (int32 i = 0; i < Info.KeyAbilities.Num(); ++i)
		{
			AbilitiesString += FString::Printf(TEXT("  [Slot %d] %s\n"), i + 1, *Info.KeyAbilities[i]);
		}
		SelectedClassAbilitiesText->SetText(FText::FromString(AbilitiesString));
	}

	const FLinearColor ActiveBorderColor(0.85f, 0.72f, 0.38f, 1.0f);
	const FLinearColor InactiveBorderColor(0.20f, 0.22f, 0.28f, 0.7f);

	if (VanguardCardBorder.IsValid())
	{
		VanguardCardBorder->SetBorderBackgroundColor(CurrentlySelectedClass == EPACharacterClass::Vanguard ? ActiveBorderColor : InactiveBorderColor);
	}
	if (RangerCardBorder.IsValid())
	{
		RangerCardBorder->SetBorderBackgroundColor(CurrentlySelectedClass == EPACharacterClass::Ranger ? ActiveBorderColor : InactiveBorderColor);
	}
	if (ArcanistCardBorder.IsValid())
	{
		ArcanistCardBorder->SetBorderBackgroundColor(CurrentlySelectedClass == EPACharacterClass::Arcanist ? ActiveBorderColor : InactiveBorderColor);
	}
}

FReply UPACharacterSelectWidget::HandleClassCardClicked(EPACharacterClass InClass)
{
	SelectClass(InClass);
	return FReply::Handled();
}

FReply UPACharacterSelectWidget::HandleEnterWorldClicked()
{
	ConfirmSelectionAndEnterWorld();
	return FReply::Handled();
}

TSharedRef<SWidget> UPACharacterSelectWidget::RebuildWidget()
{
	const FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 22);
	const FSlateFontInfo SubtitleFont = FCoreStyle::GetDefaultFontStyle("Italic", 11);
	const FSlateFontInfo CardTitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 14);
	const FSlateFontInfo CardSubFont = FCoreStyle::GetDefaultFontStyle("Regular", 10);
	const FSlateFontInfo HeaderFont = FCoreStyle::GetDefaultFontStyle("Bold", 18);
	const FSlateFontInfo BodyFont = FCoreStyle::GetDefaultFontStyle("Regular", 11);
	const FSlateFontInfo BoldBodyFont = FCoreStyle::GetDefaultFontStyle("Bold", 11);
	const FSlateFontInfo ButtonFont = FCoreStyle::GetDefaultFontStyle("Bold", 13);

	const FLinearColor GoldColor(0.85f, 0.72f, 0.38f, 1.0f);
	const FLinearColor SubtitleColor(0.65f, 0.65f, 0.70f, 1.0f);
	const FLinearColor DarkBgColor(0.04f, 0.05f, 0.07f, 0.95f);
	const FLinearColor CardBgColor(0.08f, 0.09f, 0.12f, 0.90f);
	const FLinearColor InactiveBorderColor(0.20f, 0.22f, 0.28f, 0.7f);

	return SNew(SBorder)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.80f))
		[
			SNew(SBox)
			.WidthOverride(960.0f)
			.HeightOverride(640.0f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(DarkBgColor)
				.Padding(FMargin(24.0f, 18.0f))
				[
					SNew(SVerticalBox)
					// Header
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 2.0f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("ProjectAscendant", "CharSelectTitle", "CHỌN CHỨC NGHIỆP KHỞI ĐẦU"))
						.Font(TitleFont)
						.ColorAndOpacity(FSlateColor(GoldColor))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 16.0f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("ProjectAscendant", "CharSelectSubtitle", "4 Chức Nghiệp Nền Tảng (foundational-classes.md) — Định hình bản sắc chiến đấu và cây kỹ năng"))
						.Font(SubtitleFont)
						.ColorAndOpacity(FSlateColor(SubtitleColor))
					]
					// Main Content Area: Left (Class Cards) & Right (Details Panel)
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(0.0f, 0.0f, 0.0f, 14.0f)
					[
						SNew(SHorizontalBox)
						// Left Column: 3 Class Cards
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.0f, 0.0f, 16.0f, 0.0f)
						[
							SNew(SBox)
							.WidthOverride(300.0f)
							[
								SNew(SVerticalBox)
								// Card 1: Vanguard
								+ SVerticalBox::Slot()
								.FillHeight(1.0f)
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SAssignNew(VanguardCardBorder, SBorder)
									.BorderBackgroundColor(InactiveBorderColor)
									.Padding(FMargin(2.0f))
									[
										SNew(SButton)
										.ButtonColorAndOpacity(CardBgColor)
										.OnClicked(FOnClicked::CreateUObject(this, &UPACharacterSelectWidget::HandleClassCardClicked, EPACharacterClass::Vanguard))
										[
											SNew(SVerticalBox)
											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(8.0f, 6.0f, 8.0f, 2.0f)
											[
												SNew(STextBlock)
												.Text(NSLOCTEXT("ProjectAscendant", "CardVanguardTitle", "⚔  Chiến Binh (Vanguard)"))
												.Font(CardTitleFont)
												.ColorAndOpacity(FSlateColor(GoldColor))
											]
											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(8.0f, 0.0f, 8.0f, 2.0f)
											[
												SNew(STextBlock)
												.Text(NSLOCTEXT("ProjectAscendant", "CardVanguardTag", "Bức Tường Thép Bất Hoại"))
												.Font(CardSubFont)
												.ColorAndOpacity(FSlateColor(SubtitleColor))
											]
											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(8.0f, 0.0f, 8.0f, 4.0f)
											[
												SNew(STextBlock)
												.Text(NSLOCTEXT("ProjectAscendant", "CardVanguardRole", "Cận Chiến / Perfect Parry"))
												.Font(CardSubFont)
												.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.8f, 1.0f)))
											]
										]
									]
								]
								// Card 2: Ranger
								+ SVerticalBox::Slot()
								.FillHeight(1.0f)
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SAssignNew(RangerCardBorder, SBorder)
									.BorderBackgroundColor(InactiveBorderColor)
									.Padding(FMargin(2.0f))
									[
										SNew(SButton)
										.ButtonColorAndOpacity(CardBgColor)
										.OnClicked(FOnClicked::CreateUObject(this, &UPACharacterSelectWidget::HandleClassCardClicked, EPACharacterClass::Ranger))
										[
											SNew(SVerticalBox)
											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(8.0f, 6.0f, 8.0f, 2.0f)
											[
												SNew(STextBlock)
												.Text(NSLOCTEXT("ProjectAscendant", "CardRangerTitle", "🏹  Du Hiệp (Ranger)"))
												.Font(CardTitleFont)
												.ColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.9f, 0.6f)))
											]
											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(8.0f, 0.0f, 8.0f, 2.0f)
											[
												SNew(STextBlock)
												.Text(NSLOCTEXT("ProjectAscendant", "CardRangerTag", "Bóng Ma Tật Phong & Thợ Săn"))
												.Font(CardSubFont)
												.ColorAndOpacity(FSlateColor(SubtitleColor))
											]
											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(8.0f, 0.0f, 8.0f, 4.0f)
											[
												SNew(STextBlock)
												.Text(NSLOCTEXT("ProjectAscendant", "CardRangerRole", "Xạ Thủ Tầm Xa / Cơ Động Cao"))
												.Font(CardSubFont)
												.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.8f, 1.0f)))
											]
										]
									]
								]
								// Card 3: Arcanist
								+ SVerticalBox::Slot()
								.FillHeight(1.0f)
								.Padding(0.0f, 0.0f, 0.0f, 4.0f)
								[
									SAssignNew(ArcanistCardBorder, SBorder)
									.BorderBackgroundColor(InactiveBorderColor)
									.Padding(FMargin(2.0f))
									[
										SNew(SButton)
										.ButtonColorAndOpacity(CardBgColor)
										.OnClicked(FOnClicked::CreateUObject(this, &UPACharacterSelectWidget::HandleClassCardClicked, EPACharacterClass::Arcanist))
										[
											SNew(SVerticalBox)
											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(8.0f, 6.0f, 8.0f, 2.0f)
											[
												SNew(STextBlock)
												.Text(NSLOCTEXT("ProjectAscendant", "CardArcanistTitle", "🔮  Thuật Sĩ (Arcanist)"))
												.Font(CardTitleFont)
												.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.55f, 1.0f)))
											]
											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(8.0f, 0.0f, 8.0f, 2.0f)
											[
												SNew(STextBlock)
												.Text(NSLOCTEXT("ProjectAscendant", "CardArcanistTag", "Bậc Thầy Thao Túng Chiến Trường"))
												.Font(CardSubFont)
												.ColorAndOpacity(FSlateColor(SubtitleColor))
											]
											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(8.0f, 0.0f, 8.0f, 4.0f)
											[
												SNew(STextBlock)
												.Text(NSLOCTEXT("ProjectAscendant", "CardArcanistRole", "Pháp Sư AoE / Khống Chế"))
												.Font(CardSubFont)
												.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.8f, 1.0f)))
											]
										]
									]
								]
							]
						]
						// Right Column: Class Details Panel
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNew(SBorder)
							.BorderBackgroundColor(CardBgColor)
							.Padding(FMargin(18.0f, 14.0f))
							[
								SNew(SVerticalBox)
								// Selected Class Name & Tagline
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 2.0f)
								[
									SAssignNew(SelectedClassNameText, STextBlock)
									.Text(FText::FromString(TEXT("Chiến Binh (Vanguard)")))
									.Font(HeaderFont)
									.ColorAndOpacity(FSlateColor(GoldColor))
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SAssignNew(SelectedClassTaglineText, STextBlock)
									.Text(FText::FromString(TEXT("Bức Tường Thép Bất Hoại")))
									.Font(SubtitleFont)
									.ColorAndOpacity(FSlateColor(SubtitleColor))
								]
								// Lore / Style
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SAssignNew(SelectedClassLoreText, STextBlock)
									.Text(FText::FromString(TEXT("Bậc thầy cận chiến sở hữu độ kiên cố tuyệt đối.")))
									.Font(BodyFont)
									.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.88f)))
									.AutoWrapText(true)
								]
								// Role & Weapon
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 4.0f)
								[
									SAssignNew(SelectedClassRoleText, STextBlock)
									.Text(FText::FromString(TEXT("Trường Phái: Cận Chiến / Đỡ Đòn")))
									.Font(BoldBodyFont)
									.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.8f, 1.0f)))
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SAssignNew(SelectedClassWeaponText, STextBlock)
									.Text(FText::FromString(TEXT("Vũ Khí: Kiếm Sắt & Khiên Thép")))
									.Font(BoldBodyFont)
									.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.8f, 0.6f)))
								]
								// Base Attributes Block
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SNew(SBorder)
									.BorderBackgroundColor(FLinearColor(0.05f, 0.06f, 0.08f, 0.85f))
									.Padding(FMargin(10.0f, 8.0f))
									[
										SAssignNew(SelectedClassStatsText, STextBlock)
										.Text(FText::FromString(TEXT("Máu: 120 | Thể Lực: 110 | Mana: 80 | Posture: 120 | Tốc Độ: 520 cm/s")))
										.Font(BodyFont)
										.ColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.95f, 0.95f)))
									]
								]
								// Starter Kit Abilities Block
								+ SVerticalBox::Slot()
								.FillHeight(1.0f)
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SAssignNew(SelectedClassAbilitiesText, STextBlock)
									.Text(FText::FromString(TEXT("Bộ Kỹ Năng Khởi Đầu (Starter Kit)")))
									.Font(BodyFont)
									.ColorAndOpacity(FSlateColor(FLinearColor(0.80f, 0.85f, 0.90f)))
								]
								// Enter World Button
								+ SVerticalBox::Slot()
								.AutoHeight()
								.HAlign(HAlign_Fill)
								.Padding(0.0f, 4.0f, 0.0f, 0.0f)
								[
									SNew(SButton)
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									.ButtonColorAndOpacity(FLinearColor(0.24f, 0.18f, 0.08f, 1.0f))
									.OnClicked_UObject(this, &UPACharacterSelectWidget::HandleEnterWorldClicked)
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("ProjectAscendant", "EnterWorldBtn", "✦  BƯỚC VÀO THẾ GIỚI GAME (THỊ TRẤN TIỀN TRẠM)  ✦"))
										.Font(ButtonFont)
										.ColorAndOpacity(FSlateColor(GoldColor))
										.Margin(FMargin(16.0f, 10.0f))
									]
								]
							]
						]
					]
				]
			]
		];
}
