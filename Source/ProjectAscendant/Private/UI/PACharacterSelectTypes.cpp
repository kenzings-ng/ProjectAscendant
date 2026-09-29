// Copyright Project Ascendant. All Rights Reserved.

#include "UI/PACharacterSelectTypes.h"

FPACharacterClassInfo FPACharacterClassRegistry::GetClassInfo(EPACharacterClass InClass)
{
	FPACharacterClassInfo Info;
	Info.ClassType = InClass;

	switch (InClass)
	{
	case EPACharacterClass::Vanguard:
		Info.ClassTag = TEXT("Class.Vanguard");
		Info.DisplayName = NSLOCTEXT("ProjectAscendant", "VanguardName", "Chiến Binh (Vanguard)");
		Info.Tagline = NSLOCTEXT("ProjectAscendant", "VanguardTagline", "Bức Tường Thép Bất Hoại");
		Info.LoreDescription = NSLOCTEXT("ProjectAscendant", "VanguardLore",
			"Bậc thầy cận chiến sở hữu độ kiên cố tuyệt đối. Khả năng giương khiên phòng thủ và phản đòn Perfect Parry làm chủ từng khung hình, mở toang tử huyệt thế đứng (Posture) của những Lãnh Chúa khổng lồ.");
		Info.PrimaryRole = NSLOCTEXT("ProjectAscendant", "VanguardRole", "Cận Chiến / Đỡ Đòn & Phản Kích");
		Info.PrimaryWeapon = NSLOCTEXT("ProjectAscendant", "VanguardWeapon", "Kiếm Sắt & Khiên Thép");
		Info.BaseHealth = 120.0f;
		Info.BaseStamina = 110.0f;
		Info.BaseMana = 80.0f;
		Info.BasePosture = 120.0f;
		Info.MoveSpeed = 520.0f;
		Info.KeyAbilities = {
			TEXT("Khiên Kích (Shield Bash)"),
			TEXT("Kiếm Khí Trảm (Blade Arc)"),
			TEXT("Thế Thủ & Perfect Parry"),
			TEXT("Đòn Trừng Phạt (Posture Finisher)")
		};
		Info.SpritesheetAssetPath = TEXT("/Game/art/characters/T_Vanguard_Spritesheet.T_Vanguard_Spritesheet");
		Info.AnimBlueprintPath = TEXT("/Game/art/characters/vanguard/anim/ABP_Vanguard.ABP_Vanguard_C");
		break;

	case EPACharacterClass::Ranger:
		Info.ClassTag = TEXT("Class.Ranger");
		Info.DisplayName = NSLOCTEXT("ProjectAscendant", "RangerName", "Du Hiệp (Ranger)");
		Info.Tagline = NSLOCTEXT("ProjectAscendant", "RangerTagline", "Bóng Ma Tật Phong & Thợ Săn Tử Thần");
		Info.LoreDescription = NSLOCTEXT("ProjectAscendant", "RangerLore",
			"Thợ săn bóng tối với sự cơ động vượt trội. Lướt né xé gió trong chớp mắt, găm tên xuyên phá mọi hàng ngũ quái và kiểm soát chiến trường bằng bẫy chông chết chóc.");
		Info.PrimaryRole = NSLOCTEXT("ProjectAscendant", "RangerRole", "Xạ Thủ Tầm Xa / Cơ Động & Cấu Rỉa");
		Info.PrimaryWeapon = NSLOCTEXT("ProjectAscendant", "RangerWeapon", "Cung Săn Tật Phong");
		Info.BaseHealth = 90.0f;
		Info.BaseStamina = 120.0f;
		Info.BaseMana = 90.0f;
		Info.BasePosture = 90.0f;
		Info.MoveSpeed = 570.0f;
		Info.KeyAbilities = {
			TEXT("Xuyên Tâm Tiễn (Piercing Shot)"),
			TEXT("Lưới Bẫy Chông (Caltrop Trap)"),
			TEXT("Nhịp Lướt Tật Phong (Dash Cancel)"),
			TEXT("Mưa Tên Tử Thần (Arrow Rain)")
		};
		Info.SpritesheetAssetPath = TEXT("/Game/art/characters/ranger_pixel_spritesheet.ranger_pixel_spritesheet");
		Info.AnimBlueprintPath = TEXT("/Game/art/characters/vanguard/anim/ABP_Vanguard.ABP_Vanguard_C");
		break;

	case EPACharacterClass::Arcanist:
		Info.ClassTag = TEXT("Class.Arcanist");
		Info.DisplayName = NSLOCTEXT("ProjectAscendant", "ArcanistName", "Thuật Sĩ (Arcanist)");
		Info.Tagline = NSLOCTEXT("ProjectAscendant", "ArcanistTagline", "Bậc Thầy Thao Túng Chiến Trường");
		Info.LoreDescription = NSLOCTEXT("ProjectAscendant", "ArcanistLore",
			"Học giả cổ thuật thao túng dòng chảy ma pháp. Gom quái vật vào tâm bão ma thuật và dội những vụ nổ hồ quang sấm sét quét sạch chiến trường trong tích tắc.");
		Info.PrimaryRole = NSLOCTEXT("ProjectAscendant", "ArcanistRole", "Pháp Sư / Khống Chế & Sát Thương AoE");
		Info.PrimaryWeapon = NSLOCTEXT("ProjectAscendant", "ArcanistWeapon", "Trượng Cổ Thuật Khắc Ấn");
		Info.BaseHealth = 85.0f;
		Info.BaseStamina = 90.0f;
		Info.BaseMana = 140.0f;
		Info.BasePosture = 80.0f;
		Info.MoveSpeed = 530.0f;
		Info.KeyAbilities = {
			TEXT("Xoáy Nghịch Ma Pháp (Gravity Singularity)"),
			TEXT("Quá Tải Ma Lực (Arcane Surge)"),
			TEXT("Trượng Kích Năng Lượng"),
			TEXT("Khiên Hộ Thể (Mana Shield)")
		};
		Info.SpritesheetAssetPath = TEXT("/Game/art/characters/arcanist_pixel_spritesheet.arcanist_pixel_spritesheet");
		Info.AnimBlueprintPath = TEXT("/Game/art/characters/vanguard/anim/ABP_Vanguard.ABP_Vanguard_C");
		break;
	}

	return Info;
}

TArray<FPACharacterClassInfo> FPACharacterClassRegistry::GetAllClasses()
{
	return {
		GetClassInfo(EPACharacterClass::Vanguard),
		GetClassInfo(EPACharacterClass::Ranger),
		GetClassInfo(EPACharacterClass::Arcanist)
	};
}
