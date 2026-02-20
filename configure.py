import os
import subprocess
import venv
if __name__ == "__main__":
    venv.create(".venv", with_pip=True)
    subprocess.run([os.path.join(".venv", "bin", "pip"), "install", "-r", "requirements.txt"], check=True)