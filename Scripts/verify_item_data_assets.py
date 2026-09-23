import unreal

def verify():
    unreal.log("=== Project Ascendant: Exhaustive Verification of Item DataAssets (Story item-001) ===")
    
    expected_assets = [
        ("DA_Weapon_IronSword", {
            "item_id": "Item_Weapon_IronSword",
            "base_physical_damage_bonus": 15.0,
            "max_stack_size": 1,
            "category": "EQUIPMENT",
            "allowed_equipment_slot": "MAINHAND",
            "rarity_tier": "UNCOMMON",
            "equip_gameplay_effect": "GE_Item_IronSword",
        }),
        ("DA_Armor_IronPlate", {
            "item_id": "Item_Armor_IronPlate",
            "base_armor_bonus": 40.0,
            "max_stack_size": 1,
            "category": "EQUIPMENT",
            "allowed_equipment_slot": "BODY_ARMOR",
            "rarity_tier": "RARE",
            "equip_gameplay_effect": "GE_Item_IronPlate",
        }),
        ("DA_Potion_Health", {
            "item_id": "Item_Potion_Health",
            "preferred_quickbar_slot": "QUICKBAR_1",
            "consumable_duration": 0.8,
            "consumable_health_restore": 100.0,
            "consumable_speed_debuff_ratio": 0.30,
            "max_stack_size": 20,
            "category": "CONSUMABLE",
        }),
        ("DA_SkillBook_Dash", {
            "item_id": "Item_SkillBook_Dash",
            "category": "SKILL_BOOK",
            "max_stack_size": 1,
            "granted_ability_class": "PAGameplayAbility_Dash",
        }),
    ]
    
    all_ok = True
    for asset_name, checks in expected_assets:
        path = f"/Game/Items/DataAssets/{asset_name}"
        asset = unreal.EditorAssetLibrary.load_asset(path)
        if not asset:
            unreal.log_error(f"Asset missing: {path}")
            all_ok = False
            continue
            
        unreal.log(f"\n--- Checking {path} (Class: {asset.get_class().get_name()}) ---")
        for prop, val in checks.items():
            actual = asset.get_editor_property(prop)
            unreal.log(f"  {prop}: expected={val}, actual={actual}")
            if isinstance(val, float):
                if abs(float(actual) - val) > 0.001:
                    unreal.log_error(f"Property mismatch for {prop}: expected {val}, got {actual}")
                    all_ok = False
            elif isinstance(val, int):
                if int(actual) != val:
                    unreal.log_error(f"Property mismatch for {prop}: expected {val}, got {actual}")
                    all_ok = False
            else:
                actual_str = str(actual)
                if val.upper() not in actual_str.upper():
                    unreal.log_error(f"Property mismatch for {prop}: expected {val} to be in {actual_str}")
                    all_ok = False
                    
        # Special check for DA_SkillBook_Dash required_class_tag
        if asset_name == "DA_SkillBook_Dash":
            tag = asset.get_editor_property("required_class_tag")
            tag_export = tag.export_text() if hasattr(tag, 'export_text') else str(tag)
            unreal.log(f"  required_class_tag: {tag_export}")
            if "Class.Vanguard" not in tag_export:
                unreal.log_error(f"DA_SkillBook_Dash required_class_tag does not contain Class.Vanguard: {tag_export}")
                all_ok = False

    if all_ok:
        unreal.log("\n=== ALL ITEM DATAASSETS VERIFICATION PASSED (100% SPEC ACCLIANCE) ===")
    else:
        unreal.log_error("\n=== ITEM DATAASSETS VERIFICATION FAILED ===")
        
    return all_ok

if __name__ == "__main__":
    verify()
