@echo off
setlocal enabledelayedexpansion
REM This script is supposed to be under ${REPO_PATH}/tools/init.bat

REM Get the script directory and repo directory
set "SCRIPT_DIR=%~dp0"
set "REPO_DIR=%SCRIPT_DIR%.."

echo Script directory: %SCRIPT_DIR%
echo Repository directory: %REPO_DIR%

REM Change to repo directory
cd /d "%REPO_DIR%"

REM Create virtual environment
python -m venv virtual

REM Activate virtual environment
call virtual\Scripts\activate.bat

REM Install requirements
python -m pip install --upgrade pip
pip install -r requirements.txt

REM Update git submodules
git submodule update --init
where bash >nul 2>&1
if %errorlevel%==0 (
    bash -lc "./deps/ST-LIB/tools/init-submodules.sh"
) else (
    echo bash was not found in PATH. Run deps/ST-LIB/tools/init-submodules.sh manually.
)

echo Setup complete!
