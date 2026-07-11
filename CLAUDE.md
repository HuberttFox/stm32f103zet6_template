# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project type and toolchain

STM32F103ZET6 bare-metal/HAL firmware template for Keil MDK-ARM and VS Code + EIDE.

- MCU: STM32F103ZE, Cortex-M3, 512KB Flash, 64KB SRAM.
- Keil project: `Projects/MDK-ARM/atk_f103.uvprojx`, target `LED`.
- Compiler: ARM Compiler 5 (`ARMCC` V5.06 update 5), C99 enabled, microLIB enabled.
- Preprocessor defines: `USE_HAL_DRIVER`, `STM32F103xE`.
- Output: `Output/atk_f103.axf` and hex file generation enabled.

## Common commands and workflows

No Makefile, CMake, repo-local lint, or unit-test runner exists in this repository. Build/flash is through Keil MDK-ARM or VS Code EIDE.

- Keil build: open `Projects/MDK-ARM/atk_f103.uvprojx` in Keil MDK-ARM and build target `LED`.
- VS Code/EIDE workspace: open `Projects/MDK-ARM/atk_f103.code-workspace`.
- VS Code task labels from `Projects/MDK-ARM/.vscode/tasks.json`:
  - `build` runs `${command:eide.project.build}`
  - `rebuild` runs `${command:eide.project.rebuild}`
  - `clean` runs `${command:eide.project.clean}`
  - `flash` runs `${command:eide.project.uploadToDevice}`
  - `build and flash` runs `${command:eide.project.buildAndFlash}`
- EIDE uploader in `.eide/eide.yml`: OpenOCD, `cmsis-dap`, target `stm32f1x`, base address `0x08000000`.
- Windows cleanup script: run `keilkill.bat` from repository root to delete Keil build artifacts. Review before editing; it recursively deletes many build-output extensions.
- Single-test command: none. There are no automated tests in this firmware template.

## High-level architecture

`User/main.c` is firmware entry. Current boot flow is:

1. `HAL_Init()`
2. `sys_stm32_clock_init(RCC_PLL_MUL9)` for 8MHz HSE x9 = 72MHz SYSCLK
3. `delay_init(72)` for SysTick-based busy-wait delays
4. `usart_init(115200)` for USART1/printf
5. `led_init()`
6. Main loop toggles LED0 every 500ms

Core layers:

- `User/`: application entry, HAL config, interrupt file.
  - `stm32f1xx_hal_conf.h` controls enabled HAL modules. Several modules are enabled for template use even when not currently used by `main.c`.
  - `stm32f1xx_it.c` keeps default Cortex exception handlers and `SysTick_Handler()` calling `HAL_IncTick()`.
- `Drivers/SYSTEM/`: board support utilities shared by BSP and app code.
  - `sys/`: vector table helpers, interrupt enable/disable, standby/reset, 72MHz clock setup through HAL RCC.
  - `delay/`: direct SysTick busy-wait delays and `HAL_Delay()` override. Do not assume HAL timeout behavior works like stock HAL without checking this layer.
  - `usart/`: USART1 on PA9/PA10, RX interrupt support, `printf` retarget via `fputc`, no semihosting.
- `Drivers/BSP/`: board-level peripherals with pin macros in headers and HAL GPIO/TIM code in `.c` files.
  - LED: PB5 and PE5, active high, `LEDx()` and `LEDx_TOGGLE()` macros.
  - KEY: PE4/PE3/PE2 active low, PA0 `WK_UP` active high, debounced scan with single/continuous mode.
  - TPAD: PA1 with TIM5_CH2 input capture; RC charge timing baseline calibrated in `tpad_init()`.
- `Drivers/STM32F1xx_HAL_Driver/` and `Drivers/CMSIS/`: vendor libraries. Avoid broad edits unless updating vendor code intentionally.
- `Projects/MDK-ARM/`: Keil, EIDE, and VS Code project metadata. Source files are explicitly listed here.

## Change patterns that matter

- When adding a new BSP peripheral, add source/header under `Drivers/BSP/<MODULE>/`, enable required HAL module in `User/stm32f1xx_hal_conf.h`, include/call initialization from `User/main.c`, and add new `.c` file to both `Projects/MDK-ARM/atk_f103.uvprojx` and `Projects/MDK-ARM/.eide/eide.yml`.
- Keep include style consistent with existing project paths, e.g. `#include "SYSTEM/sys/sys.h"` and `#include "BSP/LED/led.h"`.
- Keep pin, clock-enable, and public API macros in module headers; keep HAL initialization logic in matching `.c` files.
- Clock assumptions are baked into timing code: default board clock is 72MHz from 8MHz HSE, and TPAD comments assume TIM5 runs at 72MHz on APB1 timer clock.
- Project uses CRLF-prone Windows/Keil tooling. Avoid unnecessary formatting churn in XML/YAML project files.
