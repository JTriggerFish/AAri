
@echo off
python -m venv venv
.env\Scriptsctivate
python -m pip install --upgrade pip
pip install -r requirements.txt
meson setup debug --buildtype=debug --backend=vs
meson setup release --buildtype=release --backend=vs
REM Uncomment to build a compilation database for CLion
REM This requires installing ninja with chocolatey
REM meson setup _clion_compile_db --backend=ninja
