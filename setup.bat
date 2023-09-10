
@echo off
python -m venv venv
.env\Scriptsctivate
python -m pip install --upgrade pip
pip install -r requirements.txt
meson setup debug --buildtype=debug --backend=vs
meson setup release --buildtype=release --backend=vs
