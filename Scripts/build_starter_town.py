import unreal

def build_starter_town():
    unreal.log("=== Project Ascendant: Building L_VerdantFrontier_Outpost.umap ===")
    
    editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    editor_level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    
    map_package_path = "/Game/Maps/L_VerdantFrontier_Outpost"
    
    # Create or open level
    try:
        unreal.EditorLevelLibrary.new_level(map_package_path)
    except Exception as e:
        unreal.log_warning(f"EditorLevelLibrary.new_level fallback: {e}")
        try:
            editor_level_subsystem.new_level(map_package_path)
        except Exception as e2:
            unreal.log_warning(f"LevelEditorSubsystem fallback: {e2}")
    
    # Destroy existing actors to ensure clean build
    existing_actors = editor_actor_subsystem.get_all_level_actors()
    for actor in existing_actors:
        unreal.log(f"Destroying existing actor: {actor.get_actor_label()}")
        editor_actor_subsystem.destroy_actor(actor)

    # Load basic shapes and materials
    cube_mesh = None
    try:
        cube_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
    except Exception:
        pass
    if not cube_mesh:
        try:
            cube_mesh = unreal.load_object(None, "/Engine/BasicShapes/Cube.Cube")
        except Exception:
            pass

    cylinder_mesh = None
    try:
        cylinder_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cylinder.Cylinder")
    except Exception:
        pass
    if not cylinder_mesh:
        try:
            cylinder_mesh = unreal.load_object(None, "/Engine/BasicShapes/Cylinder.Cylinder")
        except Exception:
            pass

    grid_mat = None
    try:
        grid_mat = unreal.EditorAssetLibrary.load_asset("/Engine/EngineMaterials/WorldGridMaterial.WorldGridMaterial")
    except Exception:
        pass
    if not grid_mat:
        try:
            grid_mat = unreal.load_object(None, "/Engine/EngineMaterials/WorldGridMaterial.WorldGridMaterial")
        except Exception:
            pass

    # 1. Main Town Outpost Plaza Floor (5000 x 5000 x 100 cm)
    plaza_actor = editor_actor_subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(0, 0, -50),
        unreal.Rotator(0, 0, 0)
    )
    if plaza_actor:
        plaza_actor.set_actor_label("Floor_Outpost_Plaza")
        mesh_comp = plaza_actor.static_mesh_component
        if mesh_comp and cube_mesh:
            mesh_comp.set_static_mesh(cube_mesh)
            mesh_comp.set_world_scale3d(unreal.Vector(50.0, 50.0, 1.0))
            mesh_comp.set_collision_profile_name("BlockAll")
            mesh_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            if grid_mat:
                mesh_comp.set_material(0, grid_mat)
        unreal.log("Spawned Floor_Outpost_Plaza (5000x5000cm)")

    # Outer Frontier Ground (12000 x 12000 x 80 cm)
    outer_ground = editor_actor_subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(0, 0, -60),
        unreal.Rotator(0, 0, 0)
    )
    if outer_ground:
        outer_ground.set_actor_label("Floor_Frontier_Outer")
        mesh_comp = outer_ground.static_mesh_component
        if mesh_comp and cube_mesh:
            mesh_comp.set_static_mesh(cube_mesh)
            mesh_comp.set_world_scale3d(unreal.Vector(120.0, 120.0, 0.8))
            mesh_comp.set_collision_profile_name("BlockAll")
            mesh_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            if grid_mat:
                mesh_comp.set_material(0, grid_mat)
        unreal.log("Spawned Floor_Frontier_Outer (12000x12000cm)")

    # 2. PlayerStart at Outpost Sanctuary (0, -200, 100)
    player_start = editor_actor_subsystem.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(0, -200, 100),
        unreal.Rotator(pitch=0.0, yaw=45.0, roll=0.0)
    )
    if player_start:
        player_start.set_actor_label("PlayerStart_SanctuaryOutpost")
        unreal.log("Spawned PlayerStart_SanctuaryOutpost at (0, -200, 100)")

    # 3. Sanctuary Campfire (Tier 1 Checkpoint & Safe Zone Hub)
    # Campfire Stone Base
    cf_base = editor_actor_subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(0, 150, 15),
        unreal.Rotator(0, 0, 0)
    )
    if cf_base:
        cf_base.set_actor_label("Campfire_Sanctuary_Base")
        base_mesh = cylinder_mesh if cylinder_mesh else cube_mesh
        cf_comp = cf_base.static_mesh_component
        if cf_comp and base_mesh:
            cf_comp.set_static_mesh(base_mesh)
            cf_comp.set_world_scale3d(unreal.Vector(2.0, 2.0, 0.3))
            cf_comp.set_collision_profile_name("BlockAll")
            cf_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            if grid_mat:
                cf_comp.set_material(0, grid_mat)

    # Campfire PointLight (Warm Amber Sanctuary Glow)
    cf_light = editor_actor_subsystem.spawn_actor_from_class(
        unreal.PointLight,
        unreal.Vector(0, 150, 120),
        unreal.Rotator(0, 0, 0)
    )
    if cf_light:
        cf_light.set_actor_label("PointLight_Campfire_Sanctuary")
        pl_comp = cf_light.get_component_by_class(unreal.PointLightComponent)
        if pl_comp:
            pl_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            pl_comp.set_intensity(4500.0)
            pl_comp.set_attenuation_radius(1800.0)
            pl_comp.set_light_color(unreal.LinearColor(1.0, 0.62, 0.22, 1.0))
        unreal.log("Spawned PointLight_Campfire_Sanctuary (Warm Amber, 4500 Lux)")

    # 4. Training Grounds & Training Dummies (Sân Tập Kỹ Năng)
    # Training Dummy 1 (Wood)
    dummy_mesh = cylinder_mesh if cylinder_mesh else cube_mesh
    dummy_1 = editor_actor_subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(600, 250, 100),
        unreal.Rotator(0, 0, 0)
    )
    if dummy_1:
        dummy_1.set_actor_label("TrainingDummy_Wood_01")
        d1_comp = dummy_1.static_mesh_component
        if d1_comp and dummy_mesh:
            d1_comp.set_static_mesh(dummy_mesh)
            d1_comp.set_world_scale3d(unreal.Vector(1.0, 1.0, 2.0))
            d1_comp.set_collision_profile_name("BlockAll")
            d1_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            if grid_mat:
                d1_comp.set_material(0, grid_mat)
        unreal.log("Spawned TrainingDummy_Wood_01 at (600, 250, 100)")

    # Training Dummy 2 (Wood)
    dummy_2 = editor_actor_subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(600, -250, 100),
        unreal.Rotator(0, 0, 0)
    )
    if dummy_2:
        dummy_2.set_actor_label("TrainingDummy_Wood_02")
        d2_comp = dummy_2.static_mesh_component
        if d2_comp and dummy_mesh:
            d2_comp.set_static_mesh(dummy_mesh)
            d2_comp.set_world_scale3d(unreal.Vector(1.0, 1.0, 2.0))
            d2_comp.set_collision_profile_name("BlockAll")
            d2_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            if grid_mat:
                d2_comp.set_material(0, grid_mat)
        unreal.log("Spawned TrainingDummy_Wood_02 at (600, -250, 100)")

    # Weapon Rack / Training Barrier
    rack = editor_actor_subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(800, 0, 60),
        unreal.Rotator(0, 90, 0)
    )
    if rack:
        rack.set_actor_label("TrainingRack_Weapons")
        r_comp = rack.static_mesh_component
        if r_comp and cube_mesh:
            r_comp.set_static_mesh(cube_mesh)
            r_comp.set_world_scale3d(unreal.Vector(1.0, 5.0, 1.2))
            r_comp.set_collision_profile_name("BlockAll")
            r_comp.set_mobility(unreal.ComponentMobility.MOVABLE)

    # 5. Blacksmith Forge Station (Tier 1 Outpost Blacksmith)
    # Anvil
    anvil = editor_actor_subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(-600, 350, 40),
        unreal.Rotator(0, 0, 0)
    )
    if anvil:
        anvil.set_actor_label("Blacksmith_Anvil")
        a_comp = anvil.static_mesh_component
        if a_comp and cube_mesh:
            a_comp.set_static_mesh(cube_mesh)
            a_comp.set_world_scale3d(unreal.Vector(1.2, 1.2, 0.8))
            a_comp.set_collision_profile_name("BlockAll")
            a_comp.set_mobility(unreal.ComponentMobility.MOVABLE)

    # Forge Furnace
    furnace = editor_actor_subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(-750, 350, 100),
        unreal.Rotator(0, 0, 0)
    )
    if furnace:
        furnace.set_actor_label("Blacksmith_Furnace")
        f_comp = furnace.static_mesh_component
        if f_comp and cube_mesh:
            f_comp.set_static_mesh(cube_mesh)
            f_comp.set_world_scale3d(unreal.Vector(2.2, 2.2, 2.0))
            f_comp.set_collision_profile_name("BlockAll")
            f_comp.set_mobility(unreal.ComponentMobility.MOVABLE)

    # Forge PointLight (Fiery Orange)
    forge_light = editor_actor_subsystem.spawn_actor_from_class(
        unreal.PointLight,
        unreal.Vector(-680, 350, 130),
        unreal.Rotator(0, 0, 0)
    )
    if forge_light:
        forge_light.set_actor_label("PointLight_Forge")
        fl_comp = forge_light.get_component_by_class(unreal.PointLightComponent)
        if fl_comp:
            fl_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            fl_comp.set_intensity(2500.0)
            fl_comp.set_attenuation_radius(1200.0)
            fl_comp.set_light_color(unreal.LinearColor(1.0, 0.40, 0.08, 1.0))

    # 6. Merchant Supply Depot (Tier 1 Outpost Merchant)
    # Counter
    merchant_counter = editor_actor_subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(-600, -350, 40),
        unreal.Rotator(0, 0, 0)
    )
    if merchant_counter:
        merchant_counter.set_actor_label("Merchant_Counter")
        mc_comp = merchant_counter.static_mesh_component
        if mc_comp and cube_mesh:
            mc_comp.set_static_mesh(cube_mesh)
            mc_comp.set_world_scale3d(unreal.Vector(1.2, 3.0, 0.8))
            mc_comp.set_collision_profile_name("BlockAll")
            mc_comp.set_mobility(unreal.ComponentMobility.MOVABLE)

    # Merchant Lantern (Warm Soft Light)
    merchant_light = editor_actor_subsystem.spawn_actor_from_class(
        unreal.PointLight,
        unreal.Vector(-550, -350, 150),
        unreal.Rotator(0, 0, 0)
    )
    if merchant_light:
        merchant_light.set_actor_label("PointLight_Merchant")
        ml_comp = merchant_light.get_component_by_class(unreal.PointLightComponent)
        if ml_comp:
            ml_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            ml_comp.set_intensity(2000.0)
            ml_comp.set_attenuation_radius(1000.0)
            ml_comp.set_light_color(unreal.LinearColor(1.0, 0.88, 0.55, 1.0))

    # 7. Palisade Perimeter Towers & Boundary Pillars
    palisade_pillars = [
        (unreal.Vector(1800, 1800, 250), "Palisade_Watchtower_NE"),
        (unreal.Vector(1800, -1800, 250), "Palisade_Watchtower_SE"),
        (unreal.Vector(-1800, 1800, 250), "Palisade_Watchtower_NW"),
        (unreal.Vector(-1800, -1800, 250), "Palisade_Watchtower_SW"),
        (unreal.Vector(0, 1800, 150), "Palisade_Wall_North"),
        (unreal.Vector(0, -1800, 150), "Palisade_Wall_South"),
        (unreal.Vector(-1800, 0, 150), "Palisade_Wall_West"),
        (unreal.Vector(1800, 0, 150), "Palisade_Gate_East")
    ]
    for loc, label in palisade_pillars:
        pillar = editor_actor_subsystem.spawn_actor_from_class(
            unreal.StaticMeshActor,
            loc,
            unreal.Rotator(0, 0, 0)
        )
        if pillar:
            pillar.set_actor_label(label)
            p_comp = pillar.static_mesh_component
            if p_comp and cube_mesh:
                p_comp.set_static_mesh(cube_mesh)
                p_comp.set_world_scale3d(unreal.Vector(2.0, 2.0, 5.0))
                p_comp.set_collision_profile_name("BlockAll")
                p_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
                if grid_mat:
                    p_comp.set_material(0, grid_mat)

    # 8. DirectionalLight (Serene Morning Dawn - 30.0 Lux)
    dir_light = editor_actor_subsystem.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 1000),
        unreal.Rotator(pitch=-50.0, yaw=45.0, roll=0.0)
    )
    if dir_light:
        dir_light.set_actor_label("DirectionalLight_MorningSun")
        light_comp = dir_light.get_component_by_class(unreal.DirectionalLightComponent)
        if light_comp:
            light_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            light_comp.set_intensity(30.0)
            light_comp.set_light_color(unreal.LinearColor(1.0, 0.94, 0.85, 1.0))
            light_comp.set_editor_property("forward_shading_priority", 1)
            try:
                light_comp.set_editor_property("atmosphere_sun_light", True)
            except Exception:
                pass
        unreal.log("Spawned DirectionalLight (Pitch -50, Yaw 45, 30.0 Lux)")

    # 9. SkyAtmosphere & SkyLight
    sky_atmo = editor_actor_subsystem.spawn_actor_from_class(
        unreal.SkyAtmosphere,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    if sky_atmo:
        sky_atmo.set_actor_label("SkyAtmosphere_Outpost")

    sky_light = editor_actor_subsystem.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 1200),
        unreal.Rotator(0, 0, 0)
    )
    if sky_light:
        sky_light.set_actor_label("SkyLight_Outpost")
        sl_comp = sky_light.get_component_by_class(unreal.SkyLightComponent)
        if sl_comp:
            sl_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            sl_comp.set_intensity(0.80)
            try:
                sl_comp.set_editor_property("real_time_capture", True)
            except Exception:
                pass

    # 10. PostProcessVolume (Histogram Auto Exposure)
    pp_actor = editor_actor_subsystem.spawn_actor_from_class(
        unreal.PostProcessVolume,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    if pp_actor:
        pp_actor.set_actor_label("PostProcessVolume_Outpost")
        try:
            pp_actor.set_editor_property("unbound", True)
        except Exception:
            pass
        try:
            pp_settings = pp_actor.get_editor_property("settings")
            pp_settings.set_editor_property("override_auto_exposure_method", True)
            pp_settings.set_editor_property("auto_exposure_method", unreal.AutoExposureMethod.AEM_HISTOGRAM)
            pp_settings.set_editor_property("override_auto_exposure_min_brightness", True)
            pp_settings.set_editor_property("auto_exposure_min_brightness", 0.05)
            pp_settings.set_editor_property("override_auto_exposure_max_brightness", True)
            pp_settings.set_editor_property("auto_exposure_max_brightness", 15.0)
            pp_settings.set_editor_property("override_auto_exposure_bias", True)
            pp_settings.set_editor_property("auto_exposure_bias", 0.0)
            pp_actor.set_editor_property("settings", pp_settings)
        except Exception as e:
            unreal.log_warning(f"PostProcessSettings exposure setup fallback: {e}")

    # 11. ExponentialHeightFog (Morning Outpost Mist)
    height_fog = editor_actor_subsystem.spawn_actor_from_class(
        unreal.ExponentialHeightFog,
        unreal.Vector(0, 0, 50),
        unreal.Rotator(0, 0, 0)
    )
    if height_fog:
        height_fog.set_actor_label("ExponentialHeightFog_Outpost")
        fog_comp = height_fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
        if fog_comp:
            fog_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            fog_comp.set_fog_density(0.002)
            fog_comp.set_volumetric_fog(True)

    # 12. NavMeshBoundsVolume (120 x 120 x 10)
    nav_volume = editor_actor_subsystem.spawn_actor_from_class(
        unreal.NavMeshBoundsVolume,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    if nav_volume:
        nav_volume.set_actor_label("NavMeshBoundsVolume_Outpost")
        nav_volume.set_actor_scale3d(unreal.Vector(120.0, 120.0, 10.0))

    # NO BOSS - Sanctuary Safe Outpost Hub
    unreal.log("Sanctuary Safe Outpost verified: 0 bosses, 0 hostile monsters spawned.")

    # Save Level
    try:
        unreal.EditorLevelLibrary.save_current_level()
    except Exception as e:
        unreal.log_warning(f"save_current_level fallback: {e}")
        editor_level_subsystem.save_current_level()

    unreal.log(f"=== Successfully built and saved Starter Town to {map_package_path} ===")

if __name__ == "__main__":
    build_starter_town()
