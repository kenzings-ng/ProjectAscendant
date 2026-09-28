import re

with open("Tools/Spine/generate_master_spine_complete_v3.py", "r") as f:
    code = f.read()

# Replace slam animation block with the new grounded IK squash animation
new_slam = '''        "slam": {
            "events": [
                {"time": 0.60, "name": "slam_impact", "int": 100}
            ],
            "bones": {
                "pelvis": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.35, "x": -3, "y": 10},   # Wind-up lift
                        {"time": 0.45, "x": -3, "y": 10},   # Apex pause
                        {"time": 0.60, "x": 4, "y": -16},   # Heavy downward smash
                        {"time": 0.75, "x": 3, "y": -12},   # Shockwave squat
                        {"time": 1.20, "x": 0, "y": 0}      # Recover
                    ]
                },
                "torso": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": -14}, # Arch back
                        {"time": 0.45, "value": -14},
                        {"time": 0.60, "value": 18},  # Slam forward
                        {"time": 0.75, "value": 14},  # Stay crushed
                        {"time": 1.20, "value": 0}
                    ]
                },
                "head": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": -8},
                        {"time": 0.60, "value": 12},
                        {"time": 0.75, "value": 8},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "shoulder_r": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": 32},  # Raise high
                        {"time": 0.45, "value": 32},
                        {"time": 0.60, "value": -26}, # Smash down
                        {"time": 0.75, "value": -22},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "arm_upper_r": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": 14},
                        {"time": 0.60, "value": -12},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "arm_lower_r": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": 16},
                        {"time": 0.60, "value": -14},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "shoulder_l": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": 28},
                        {"time": 0.45, "value": 28},
                        {"time": 0.60, "value": -22},
                        {"time": 0.75, "value": -18},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "arm_upper_l": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": 12},
                        {"time": 0.60, "value": -10},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "arm_lower_l": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": 14},
                        {"time": 0.60, "value": -12},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "thigh_r": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.35, "x": 0, "y": -8},
                        {"time": 0.60, "x": 0, "y": 14},  # Knee compression keeps foot on ground
                        {"time": 0.75, "x": 0, "y": 10},
                        {"time": 1.20, "x": 0, "y": 0}
                    ],
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": -5},
                        {"time": 0.60, "value": 12},
                        {"time": 0.75, "value": 8},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "calf_r": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": 5},
                        {"time": 0.60, "value": -12},
                        {"time": 0.75, "value": -8},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "thigh_l": {
                    "translate": [
                        {"time": 0.0, "x": 0, "y": 0},
                        {"time": 0.35, "x": 0, "y": -8},
                        {"time": 0.60, "x": 0, "y": 14},
                        {"time": 0.75, "x": 0, "y": 10},
                        {"time": 1.20, "x": 0, "y": 0}
                    ],
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": 5},
                        {"time": 0.60, "value": -12},
                        {"time": 0.75, "value": -8},
                        {"time": 1.20, "value": 0}
                    ]
                },
                "calf_l": {
                    "rotate": [
                        {"time": 0.0, "value": 0},
                        {"time": 0.35, "value": -5},
                        {"time": 0.60, "value": 12},
                        {"time": 0.75, "value": 8},
                        {"time": 1.20, "value": 0}
                    ]
                }
            }
        }'''

# Replace from '"slam": {' to the matching end of slam block
slam_start = code.find('"slam": {')
slam_end = code.find('"walk": {')
# find last closing brace before "walk": {
last_brace = code.rfind("},", 0, slam_end)
code = code[:slam_start] + new_slam + code[last_brace + 2:]

with open("Tools/Spine/generate_master_spine_complete_v3.py", "w") as f:
    f.write(code)

print("Updated generate_master_spine_complete_v3.py with grounded slam IK!")
