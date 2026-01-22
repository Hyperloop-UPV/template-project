# Template Project

HyperloopUPV's Template project for STM32 development with LwIP.

Designed using CMake with defaults for development with VSCode.

This makes use of the [ST-LIB](https://github.com/HyperloopUPV-H8/ST-LIB) developed by HyperloopUPV's team.

## Container Setup
To use it you must install [Dev Containers extension on VSCode](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) and [Docker](https://www.docker.com). Be careful to use the instructions related to your OS, as docker not works in the same way exactly between them.
Then, when you open this folder in VSCode, you will have the ability to reopen it inside the container. Don't worry, the first time you do it takes a loong time.
## SetUp
Create a virtual environment with
>python3 -m venv virtual

at the root of the project
## Modes
### Simulator
The container is fully ready to develop, compile and debug the code in simulator mode, so you don't have to worry about setting your environment
### MCU
The container is ready to develop and compile the code in MCU mode, but it couldn't be possible to flash and debug through it, so you should do it in your local machine. To do it, you must install the [STM32CubeCLT](https://www.st.com/en/development-tools/stm32cubeclt.html).
To flash and debug, you should be outside the container, and use the [Cortex-Debug VSCode extension](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug)


## Notes
If you are going to develop OUTSIDE the container, you MUST change `template-project.code-workspace` file after project creation if you DON'T have STLIB on ../ST-LIB relative path. This template is intended for you to develop always inside the container, so you should not be concerned about this.
