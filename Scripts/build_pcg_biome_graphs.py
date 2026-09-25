#!/usr/bin/env python3
# ==============================================================================
# build_pcg_biome_graphs.py: Procedural Content Generation Configuration Builder
# Project: Project Ascendant (2.5D Isometric Hardcore ARPG MMO on UE 5.8)
# Conforms to: ARCH-PCG-ENV-2026-V1 & knissen/unreal-pcg-examples architecture
# ==============================================================================

import os
import json

OUT_PCG_DIR = "Content/Maps/PCG"
os.makedirs(OUT_PCG_DIR, exist_ok=True)

# 1. Citadel Coordinates from zone-001 specification
CITADEL_LOCATIONS = [
    {"name": "Verdant Bastion", "x": 0.0, "y": 0.0, "z": 100.0, "safe_radius_cm": 5000.0},
    {"name": "Ashen Keep", "x": 35000.0, "y": 25000.0, "z": 450.0, "safe_radius_cm": 5000.0},
    {"name": "Sanctum Fortress", "x": -30000.0, "y": 45000.0, "z": 800.0, "safe_radius_cm": 5000.0}
]

# 2. Road Spline Graph Definition
ROAD_SPLINE_GRAPH = {
    "graph_name": "PCG_CitadelRoadSplineGraph",
    "version": "1.0",
    "engine_target": "UE_5.8",
    "nodes": [
        {
            "id": "GetSplineData_0",
            "type": "PCGGetSplineData",
            "properties": {
                "actor_filter": "AllWorldActors",
                "tag_filter": "CitadelConnectingRoad"
            }
        },
        {
            "id": "SplineSampler_0",
            "type": "PCGSplineSampler",
            "properties": {
                "mode": "Distance",
                "dimension": "OnSpline",
                "step_distance_cm": 100.0,
                "subdivide_curves": True
            }
        },
        {
            "id": "Projection_0",
            "type": "PCGProjection",
            "properties": {
                "projection_axis": "Z",
                "max_slope_deg": 25.0,
                "target": "Landscape"
            }
        },
        {
            "id": "RoadCrossSection_0",
            "type": "PCGTransformPoints",
            "properties": {
                "path_width_cm": 400.0,
                "falloff_shoulder_cm": 100.0
            }
        },
        {
            "id": "CobblestoneSpawner_0",
            "type": "PCGStaticMeshSpawner",
            "properties": {
                "mesh_paths": [
                    "/Game/Content/Maps/Meshes/SM_RoadCobblestone_01",
                    "/Game/Content/Maps/Meshes/SM_RoadCobblestone_02"
                ],
                "hism_enabled": True
            }
        }
    ],
    "connections": [
        {"from": "GetSplineData_0", "to": "SplineSampler_0"},
        {"from": "SplineSampler_0", "to": "Projection_0"},
        {"from": "Projection_0", "to": "RoadCrossSection_0"},
        {"from": "RoadCrossSection_0", "to": "CobblestoneSpawner_0"}
    ]
}

# 3. World Biome Graph Definition
WORLD_BIOME_GRAPH = {
    "graph_name": "PCG_WorldBiomeGraph",
    "version": "1.0",
    "engine_target": "UE_5.8",
    "exclusion_zones": CITADEL_LOCATIONS,
    "nodes": [
        {
            "id": "SurfaceSampler_0",
            "type": "PCGSurfaceSampler",
            "properties": {
                "sampling_method": "PoissonDisk",
                "points_per_squared_meter": 0.05,
                "looseness": 1.5
            }
        },
        {
            "id": "DistanceFilter_Exclusion",
            "type": "PCGDistanceFilter",
            "properties": {
                "mode": "ExcludeInsideRadius",
                "reference_points": [
                    {"x": c["x"], "y": c["y"], "radius": c["safe_radius_cm"]} for c in CITADEL_LOCATIONS
                ]
            }
        },
        {
            "id": "SlopeFilter_0",
            "type": "PCGSlopeFilter",
            "properties": {
                "min_slope_deg": 0.0,
                "max_slope_deg": 40.0
            }
        },
        {
            "id": "BiomePartition_0",
            "type": "PCGBiomeSwitch",
            "properties": {
                "biomes": {
                    "VerdantBastion": {
                        "foliage_density": 0.8,
                        "rock_density": 0.3,
                        "tree_palette": ["SM_Oak_Large", "SM_Pine_Young", "SM_Bush_Fern"],
                        "rock_palette": ["SM_MossyBoulder_01", "SM_MossyBoulder_02"]
                    },
                    "AshenKeep": {
                        "foliage_density": 0.2,
                        "rock_density": 0.8,
                        "tree_palette": ["SM_BurntTrunk_01", "SM_ThornBush"],
                        "rock_palette": ["SM_ObsidianCrag_01", "SM_AshenRock_02"]
                    },
                    "SanctumFortress": {
                        "foliage_density": 0.5,
                        "rock_density": 0.6,
                        "tree_palette": ["SM_SnowyPine_01", "SM_WhiteBirch"],
                        "rock_palette": ["SM_GranitePillar_01", "SM_FrostRock_02"]
                    }
                }
            }
        },
        {
            "id": "FoliageSpawner_0",
            "type": "PCGStaticMeshSpawner",
            "properties": {
                "hism_enabled": True,
                "cull_distance_min": 15000.0,
                "cull_distance_max": 30000.0
            }
        }
    ],
    "connections": [
        {"from": "SurfaceSampler_0", "to": "DistanceFilter_Exclusion"},
        {"from": "DistanceFilter_Exclusion", "to": "SlopeFilter_0"},
        {"from": "SlopeFilter_0", "to": "BiomePartition_0"},
        {"from": "BiomePartition_0", "to": "FoliageSpawner_0"}
    ]
}

def build_pcg_assets():
    road_path = os.path.join(OUT_PCG_DIR, "PCG_CitadelRoadSplineGraph.json")
    with open(road_path, "w") as f:
        json.dump(ROAD_SPLINE_GRAPH, f, indent=2)
    print(f"✅ Generated PCG Road Spline Graph: {road_path}")

    biome_path = os.path.join(OUT_PCG_DIR, "PCG_WorldBiomeGraph.json")
    with open(biome_path, "w") as f:
        json.dump(WORLD_BIOME_GRAPH, f, indent=2)
    print(f"✅ Generated PCG World Biome Graph:  {biome_path}")

    exclusion_path = os.path.join(OUT_PCG_DIR, "Citadel_SafeZone_Exclusion_Volumes.json")
    with open(exclusion_path, "w") as f:
        json.dump(CITADEL_LOCATIONS, f, indent=2)
    print(f"✅ Generated Exclusion Volumes:      {exclusion_path}")

if __name__ == "__main__":
    build_pcg_assets()
