import os
import subprocess
import argparse

def run_command(command, cwd=None):
    print(f"\nRunning: {' '.join(command)}")
    result = subprocess.run(command, cwd=cwd)
    if result.returncode != 0:
        print("Command failed!")
        exit(result.returncode)

def main():
    build_dir = "build"

    if not os.path.exists(build_dir):
        os.makedirs(build_dir)
    
    run_command(["cmake", ".."], cwd=build_dir)

    run_command(["cmake", "--build", ".", "--target", "clean"], cwd=build_dir)

    run_command(["cmake", "--build", ".", "--target", "_client"], cwd=build_dir)
    run_command(["cmake", "--build", ".", "--target", "_server"], cwd=build_dir)

    print("\nBuild completed successfully.")

if __name__ == "__main__":
    main()