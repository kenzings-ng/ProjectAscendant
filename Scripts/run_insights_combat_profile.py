import unreal
import time
import os
import glob

def run_combat_profiling():
    unreal.log("================================================================")
    unreal.log(" Project Ascendant: Running Unreal Insights / CSV Combat Profiler")
    unreal.log(" Scenario: 50 Players + Boss + FCT + Alpha Overdraw Hotspot")
    unreal.log("================================================================")

    # Load World
    map_path = "/Game/Maps/L_VerdantFrontier_Outpost"
    loaded = unreal.EditorLoadingAndSavingUtils.load_map(map_path)
    unreal.log(f"Map Loaded: {loaded}")

    world = unreal.EditorLevelLibrary.get_editor_world()
    if not world:
        unreal.log_error("Failed to get editor world!")
        return False

    # Spawn Stone Golem Boss at Center
    boss_loc = unreal.Vector(0.0, 0.0, 50.0)
    boss_rot = unreal.Rotator(0.0, 0.0, 0.0)
    boss_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PAStoneGolemBoss, boss_loc, boss_rot)
    unreal.log(f"Spawned Stone Golem Boss: {boss_actor.get_name() if boss_actor else 'None'}")

    # Spawn 50 Player Actors around Boss (radius 100-500cm)
    spawned_players = []
    classes = [unreal.PABaseCharacter]
    
    # We will spawn 50 Character actors around the boss
    import math
    for i in range(50):
        angle = (2.0 * math.pi / 50.0) * i
        dist = 120.0 + (i % 5) * 75.0  # 120cm to 420cm radius
        p_loc = unreal.Vector(dist * math.cos(angle), dist * math.sin(angle), 50.0)
        p_rot = unreal.Rotator(0.0, math.degrees(angle + math.pi), 0.0) # facing boss
        
        player = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PABaseCharacter, p_loc, p_rot)
        if player:
            spawned_players.append(player)
            # Setup Paperdoll visual
            paperdoll = player.get_component_by_class(unreal.PAPaperdollComponent)
            if paperdoll:
                # Equip Armor and Weapon
                paperdoll.equip_visual(unreal.EPAPaperdollLayer.CHEST_ARMOR, "item_iron_armor", "Visual_IronArmor")
                paperdoll.equip_visual(unreal.EPAPaperdollLayer.MAINHAND_WEAPON, "item_broadsword", "Visual_Broadsword")
                if i % 2 == 0:
                    paperdoll.equip_visual(unreal.EPAPaperdollLayer.OFFHAND_SHIELD, "item_iron_shield", "Visual_IronShield")

    unreal.log(f"Successfully spawned {len(spawned_players)} players with active Paperdoll components.")

    # Setup Floating Combat Text Component on Boss
    fct_comp = boss_actor.get_component_by_class(unreal.PAFloatingCombatTextComponent) if boss_actor else None
    if fct_comp:
        for j in range(50):
            fct_comp.spawn_combat_text(boss_loc + unreal.Vector(0, 0, 150), 350.0 + j * 20.0, unreal.EPACombatTextType.NORMAL_DAMAGE)
        unreal.log("Spawned 50 concurrent Floating Combat Text entries.")

    unreal.log("--- Executing Profiling Commands ---")
    # Start Unreal Insights Trace
    unreal.SystemLibrary.execute_console_command(world, "Trace.Start cpu,gpu,frame,bookmark")
    unreal.SystemLibrary.execute_console_command(world, "CsvProfile Start -name=PeakCombat50")
    unreal.SystemLibrary.execute_console_command(world, "stat RHI")
    unreal.SystemLibrary.execute_console_command(world, "stat GPU")
    unreal.SystemLibrary.execute_console_command(world, "stat SceneRendering")

    # Let the engine process frames
    unreal.log("Collecting frame render statistics...")
    time.sleep(2.0)

    # Stop Profiling
    unreal.SystemLibrary.execute_console_command(world, "CsvProfile Stop")
    unreal.SystemLibrary.execute_console_command(world, "Trace.Stop")
    unreal.log("Profiling capture stopped successfully.")

    # Cleanup spawned actors to keep map clean
    if boss_actor:
        unreal.EditorLevelLibrary.destroy_actor(boss_actor)
    for p in spawned_players:
        unreal.EditorLevelLibrary.destroy_actor(p)
    unreal.log("Cleaned up test actors.")

    return True

if __name__ == "__main__":
    run_combat_profiling()
