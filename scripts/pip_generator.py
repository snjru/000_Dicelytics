import math
import random
import csv
from pathlib import Path

def get_project_root() -> Path:
    """
    Returns the absolute path to the project root directory (dicelytics/).
    This ensures output files are always saved in the correct location.
    """
    # Assuming this script is located at: dicelytics/scripts/pip_generator.py
    return Path(__file__).resolve().parent.parent

def generate_dice_pip_dataset(
    num_samples: int = 1000,
    dice_size_mm: float = 8.0,
    circle_radius_mm: float = 20.0,
    output_relative_path: str = "data/synthetic/dice_pips_dataset.csv"
) -> None:
    """
    Generates synthetic (x, y) coordinate data of dice pips for multiple samples
    under specific physical constraints (3 dice of 8mm size inside a 40mm diameter circle).
    
    Exports the dataset into a CSV file inside data/synthetic/ for C++ algorithm testing.
    """
    
    # Define output absolute path
    project_root = get_project_root()
    output_file_path = project_root / output_relative_path

    # Ensure the target directory exists (creates data/synthetic/ if missing)
    output_file_path.parent.mkdir(parents=True, exist_ok=True)

    # Relative normalized positions of pips on a dice face (-1.0 to 1.0 scale)
    offset = 0.5
    pip_patterns = {
        1: [(0.0, 0.0)],
        2: [(-offset, -offset), (offset, offset)],
        3: [(-offset, -offset), (0.0, 0.0), (offset, offset)],
        4: [(-offset, -offset), (-offset, offset), (offset, -offset), (offset, offset)],
        5: [(-offset, -offset), (-offset, offset), (0.0, 0.0), (offset, -offset), (offset, offset)],
        6: [(-offset, -offset), (-offset, 0.0), (-offset, offset), 
            (offset, -offset), (offset, 0.0), (offset, offset)]
    }

    # Open CSV file for writing
    with open(output_file_path, mode='w', newline='', encoding='utf-8') as csv_file:
        fieldnames = ['sample_id', 'dice_id', 'pip_index_in_dice', 'pip_value', 'pip_x_mm', 'pip_y_mm']
        writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
        writer.writeheader()

        for sample_id in range(1, num_samples + 1):
            dice_centers = []
            
            # Step 1: Randomly place 3 dice inside the 40mm circle without overlap
            max_attempts = 1000
            for _ in range(3):
                for _ in range(max_attempts):
                    # Keep dice center inside circle boundary
                    r = math.sqrt(random.uniform(0.0, (circle_radius_mm - dice_size_mm / 2.0) ** 2))
                    theta = random.uniform(0.0, 2.0 * math.pi)
                    cx = r * math.cos(theta)
                    cy = r * math.sin(theta)
                    
                    # Collision check: ensure distance between dice centers >= 8mm
                    overlap = False
                    for ox, oy in dice_centers:
                        if math.hypot(cx - ox, cy - oy) < dice_size_mm:
                            overlap = True
                            break
                            
                    if not overlap:
                        dice_centers.append((cx, cy))
                        break

            # Step 2: Determine face values, orientation, and calculate absolute (x, y) for each pip
            for dice_id, (cx, cy) in enumerate(dice_centers, start=1):
                pip_value = random.randint(1, 6)
                rot_angle = random.uniform(0.0, 2.0 * math.pi)
                cos_a = math.cos(rot_angle)
                sin_a = math.sin(rot_angle)

                for pip_idx, (rx, ry) in enumerate(pip_patterns[pip_value], start=1):
                    # Scale relative coordinates to millimeters
                    rel_x = rx * (dice_size_mm / 2.0 * 0.5)
                    rel_y = ry * (dice_size_mm / 2.0 * 0.5)
                    
                    # Rotate relative position
                    rot_x = rel_x * cos_a - rel_y * sin_a
                    rot_y = rel_x * sin_a + rel_y * cos_a
                    
                    # Translate to global center + add Gaussian noise (std_dev = 0.1mm)
                    noise_x = random.gauss(0.0, 0.1)
                    noise_y = random.gauss(0.0, 0.1)
                    abs_x = round(cx + rot_x + noise_x, 3)
                    abs_y = round(cy + rot_y + noise_y, 3)

                    # Write record to CSV
                    writer.writerow({
                        'sample_id': sample_id,
                        'dice_id': dice_id,
                        'pip_index_in_dice': pip_idx,
                        'pip_value': pip_value,
                        'pip_x_mm': abs_x,
                        'pip_y_mm': abs_y
                    })

    print(f"[SUCCESS] Dataset with {num_samples} samples generated at:\n  -> {output_file_path}")

if __name__ == "__main__":
    generate_dice_pip_dataset(num_samples=1000)