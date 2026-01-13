@echo off
setlocal enabledelayedexpansion
# This script is supposed to be under ${REPO_PATH}/tools/init.bat

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
pip install -r requirements.txt

REM Update git submodules
git submodule update --init

echo Setup complete!
