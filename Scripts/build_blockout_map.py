import unreal

def build_blockout_map():
    unreal.log("=== Project Ascendant: Building L_Ruins_Blockout.umap ===")
    
    editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    editor_level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    
    map_package_path = "/Game/Maps/L_Ruins_Blockout"
    
    # Create new level
    try:
        unreal.EditorLevelLibrary.new_level(map_package_path)
    except Exception as e:
        unreal.log_warning(f"EditorLevelLibrary.new_level fallback: {e}")
        try:
            editor_level_subsystem.new_level(map_package_path)
        except Exception as e2:
            unreal.log_warning(f"LevelEditorSubsystem fallback: {e2}")
    
    # Destroy ALL existing actors in the level to avoid duplicates
    existing_actors = editor_actor_subsystem.get_all_level_actors()
    for actor in existing_actors:
        unreal.log(f"Destroying existing actor: {actor.get_actor_label()}")
        editor_actor_subsystem.destroy_actor(actor)

    # Load basic cube static mesh and dark grid material for floor and pillars
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

    # Spawn Ground Floor (6000 x 6000 x 100 cm)
    floor_actor = editor_actor_subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(0, 0, -50),
        unreal.Rotator(0, 0, 0)
    )
    if floor_actor:
        floor_actor.set_actor_label("Floor_Graybox_60x60m")
        mesh_comp = floor_actor.static_mesh_component
        if mesh_comp and cube_mesh:
            mesh_comp.set_static_mesh(cube_mesh)
            mesh_comp.set_world_scale3d(unreal.Vector(60.0, 60.0, 1.0))
            mesh_comp.set_collision_profile_name("BlockAll")
            mesh_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            if grid_mat:
                mesh_comp.set_material(0, grid_mat)
        unreal.log("Spawned Ground Floor (6000x6000cm, WorldGridMaterial)")

    # Spawn PlayerStart at (0, 0, 100)
    player_start = editor_actor_subsystem.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(0, 0, 100),
        unreal.Rotator(pitch=0.0, yaw=45.0, roll=0.0)
    )
    if player_start:
        player_start.set_actor_label("PlayerStart_Ruins")
        unreal.log("Spawned PlayerStart at (0, 0, 100)")

    # Spawn Directional Light (Dark Fantasy Moonlight - 25.0 Lux)
    dir_light = editor_actor_subsystem.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 1000),
        unreal.Rotator(pitch=-55.0, yaw=40.0, roll=0.0)
    )
    if dir_light:
        dir_light.set_actor_label("DirectionalLight_Moonlit")
        light_comp = dir_light.get_component_by_class(unreal.DirectionalLightComponent)
        if light_comp:
            light_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            light_comp.set_intensity(25.0)
            light_comp.set_light_color(unreal.LinearColor(0.65, 0.80, 1.0, 1.0))
            light_comp.set_editor_property("forward_shading_priority", 1)
            try:
                light_comp.set_editor_property("atmosphere_sun_light", True)
            except Exception:
                pass
        unreal.log("Spawned DirectionalLight (Pitch -55, Yaw 40, Movable, 25.0 Lux)")

    # Spawn SkyAtmosphere (Required for SkyLight Real-Time Capture)
    sky_atmo = editor_actor_subsystem.spawn_actor_from_class(
        unreal.SkyAtmosphere,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    if sky_atmo:
        sky_atmo.set_actor_label("SkyAtmosphere_Ruins")
        unreal.log("Spawned SkyAtmosphere")

    # Spawn SkyLight (Movable Dynamic Ambient - 0.75 cd/m2)
    sky_light = editor_actor_subsystem.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 1200),
        unreal.Rotator(0, 0, 0)
    )
    if sky_light:
        sky_light.set_actor_label("SkyLight_Ambient")
        sl_comp = sky_light.get_component_by_class(unreal.SkyLightComponent)
        if sl_comp:
            sl_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            sl_comp.set_intensity(0.75)
            try:
                sl_comp.set_editor_property("real_time_capture", True)
            except Exception:
                pass
        unreal.log("Spawned SkyLight (Movable, Intensity 0.75)")

    # Spawn Global PostProcessVolume for Clean Exposure Calibration
    pp_actor = editor_actor_subsystem.spawn_actor_from_class(
        unreal.PostProcessVolume,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    if pp_actor:
        pp_actor.set_actor_label("PostProcessVolume_Global")
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
        unreal.log("Spawned PostProcessVolume_Global (Unbound, Calibrated Histogram Exposure)")

    # Spawn ExponentialHeightFog for Dark Fantasy Atmosphere
    height_fog = editor_actor_subsystem.spawn_actor_from_class(
        unreal.ExponentialHeightFog,
        unreal.Vector(0, 0, 50),
        unreal.Rotator(0, 0, 0)
    )
    if height_fog:
        height_fog.set_actor_label("ExponentialHeightFog_DarkFantasy")
        fog_comp = height_fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
        if fog_comp:
            fog_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            fog_comp.set_fog_density(0.003)
            fog_comp.set_volumetric_fog(True)
        unreal.log("Spawned ExponentialHeightFog with Volumetric Fog enabled")

    # Spawn Occlusion Test Stone Pillars
    pillar_locations = [
        (unreal.Vector(300, 300, 300), "StonePillar_Occlusion_NE"),
        (unreal.Vector(-300, -300, 300), "StonePillar_Occlusion_SW"),
        (unreal.Vector(-600, 500, 300), "StonePillar_Boundary_NW"),
        (unreal.Vector(600, -500, 300), "StonePillar_Boundary_SE")
    ]
    for loc, label in pillar_locations:
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
                p_comp.set_world_scale3d(unreal.Vector(2.0, 2.0, 6.0)) # 200 x 200 x 600 cm
                p_comp.set_collision_profile_name("BlockAll")
                p_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
                if grid_mat:
                    p_comp.set_material(0, grid_mat)
            unreal.log(f"Spawned {label} at {loc}")

    # Spawn NavMeshBoundsVolume
    nav_volume = editor_actor_subsystem.spawn_actor_from_class(
        unreal.NavMeshBoundsVolume,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    if nav_volume:
        nav_volume.set_actor_label("NavMeshBoundsVolume_Ruins")
        nav_volume.set_actor_scale3d(unreal.Vector(60.0, 60.0, 10.0))
        unreal.log("Spawned NavMeshBoundsVolume")

    # Safe Sanctuary Starting Hub: No immediate boss on spawn
    # The starting area is peaceful for learning controls and testing movement.

    # Save Level
    try:
        unreal.EditorLevelLibrary.save_current_level()
    except Exception as e:
        unreal.log_warning(f"save_current_level fallback: {e}")
        editor_level_subsystem.save_current_level()

    unreal.log(f"=== Successfully saved level to {map_package_path} ===")

if __name__ == "__main__":
    build_blockout_map()
