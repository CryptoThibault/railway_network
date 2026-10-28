from pathlib import Path
import subprocess
import sys

root = Path(sys.argv[1]).resolve()
compiler = sys.argv[2] if len(sys.argv) > 2 else "g++"
headers = sorted((root / "inc").glob("*.hpp"))
for header in headers:
    result = subprocess.run(
        [compiler, "-std=c++20", "-Wall", "-Wextra", "-Werror", "-fsyntax-only",
         "-Iinc", "-Ilib/include", "-Ilib/template", "-x", "c++", "-"],
        input=f'#include "{header.name}"\n', cwd=root, capture_output=True, text=True,
        timeout=10)
    assert result.returncode == 0, f"{header.name}:\n{result.stderr}"
print(f"Standalone header checks passed: {len(headers)} headers")
