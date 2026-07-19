# STM32F103ZET6 通用开发模板

面向 STM32F103ZET6 的 Keil MDK-ARM / VS Code + EIDE 固件模板。工程基于 STM32F1 HAL，默认运行在 8MHz HSE × 9 = 72MHz SYSCLK，适合作为外设实验和板级驱动开发起点。

## 硬件与工具链

- MCU: STM32F103ZE，Cortex-M3，512KB Flash，64KB SRAM
- Keil 工程: `Projects/MDK-ARM/atk_f103.uvprojx`
- Keil target: `LED`
- 编译器: ARM Compiler 5 (`ARMCC` V5.06 update 5)
- 编译选项: C99、microLIB、`USE_HAL_DRIVER`、`STM32F103xE`
- 输出: `Output/atk_f103.axf`，Keil 工程已开启 HEX 生成
- 下载: Keil 可用 ST-LINK / CMSIS-DAP；EIDE 默认 OpenOCD + `cmsis-dap` + `stm32f1x`

## 目录结构

```text
├── Drivers/
│   ├── BSP/
│   │   ├── LED/
│   │   ├── KEY/
│   │   ├── OLED/
│   │   └── TPAD/
│   ├── CMSIS/                # CMSIS
│   ├── STM32F1xx_HAL_Driver/ # HAL 库
│   └── SYSTEM/               # 系统支持
├── Projects/MDK-ARM/         # 工程配置
│   ├── .vscode/
│   ├── .eide/
│   ├── atk_f103.uvprojx
│   └── atk_f103.code-workspace
├── User/                     # 应用入口
├── keilkill.bat              # 清理 Keil 产物
├── CLAUDE.md                 # AI 项目指引
└── README.md
```

## 构建与下载

无 Makefile/CMake。构建依赖 Keil MDK-ARM 或 VS Code EIDE。

- **Keil:** 打开 `Projects/MDK-ARM/atk_f103.uvprojx`，target `LED`，Build。ST-LINK/CMSIS-DAP SWD 下载。
- **VS Code + EIDE:** 打开 `Projects/MDK-ARM/atk_f103.code-workspace`，运行 task `build` → `flash`。下载配置见 `.eide/eide.yml`（OpenOCD, cmsis-dap, stm32f1x）。
- **清理:** `keilkill.bat`（**编辑前检查删除范围**）。

## 架构说明

### `User/`

- `main.c`: 应用入口，负责 HAL、时钟、延时、串口、BSP 初始化。默认流程：`HAL_Init()` → `sys_stm32_clock_init(RCC_PLL_MUL9)` → `delay_init(72)` → `usart_init(115200)` → `led_init()`。主循环 500ms 翻转 LED0，`printf` 通过 USART1 输出。
- `stm32f1xx_hal_conf.h`: HAL 模块开关。当前模板启用了 GPIO/RCC/PWR/CORTEX/TIM/UART/DMA/EXTI 等模块，也保留部分暂未使用模块供扩展。
- `stm32f1xx_it.c`: Cortex 异常入口和 `SysTick_Handler()`，其中 `SysTick_Handler()` 调用 `HAL_IncTick()`。

### `Drivers/SYSTEM/`

- `sys/`: 向量表、全局中断、待机、软复位、系统时钟初始化。
- `delay/`: 直接操作 SysTick 实现 busy-wait `delay_us()` / `delay_ms()`，并重写 `HAL_Delay()`。
- `usart/`: USART1，PA9/PA10，支持 RX 中断和 `printf` 重定向，无半主机依赖。

### `Drivers/BSP/`

BSP 模块采用固定模式：头文件放引脚、时钟使能宏和公开 API；`.c` 文件放 HAL GPIO/TIM 初始化与业务逻辑。

| 模块 | 主要 API | 说明 |
|---|---|---|
| LED | `led_init()`、`LED0()`、`LED1()`、`LEDx_TOGGLE()` | PB5 / PE5，高电平点亮 |
| KEY | `key_init()`、`key_scan(mode)` | PE4/PE3/PE2 低有效，PA0 WK_UP 高有效，带 10ms 消抖 |
| TPAD | `tpad_init(psc)`、`tpad_scan(mode)` | PA1 + TIM5_CH2，RC 充放电输入捕获，初始化时采样基线 |
| OLED | `oled_init()`、`oled_refresh_gram()`、`oled_show_string()`、`oled_show_num()`、`oled_clear()`、`OLED_CMD`/`OLED_DATA` | PC0-PC7 8080 并口，128×64 帧缓冲 |

## 添加新 BSP 外设

1. 在 `Drivers/BSP/<MODULE>/` 新增 `.h` / `.c`
2. 在头文件定义引脚、GPIO/TIM/USART 等时钟使能宏、公开 API
3. 在 `.c` 文件实现 HAL 初始化和外设逻辑
4. 在 `User/stm32f1xx_hal_conf.h` 启用需要的 HAL 模块
5. 在 `User/main.c` include 对应头文件并调用初始化
6. 将新增 `.c` 同步加入：
   - `Projects/MDK-ARM/atk_f103.uvprojx`
   - `Projects/MDK-ARM/.eide/eide.yml`

工程源文件是显式枚举的，只把文件放进目录不会自动参与构建。

## 注意事项

- 默认外部晶振为 8MHz，系统时钟初始化目标为 72MHz。
- `delay_init()` 会接管 SysTick 做忙等待延时；调用后不要假设 HAL timeout / `HAL_GetTick()` 行为等同于 stock HAL。
- TPAD 注释和参数默认假设 TIM5 时钟为 72MHz；常用 `tpad_init(72)` 让定时器 tick 为 1us。
- OLED 8080 并口占用了 PC0-PC7（数据）、PD3（RS）、PD6（CS）、PG13（RD）、PG14（WR）、PG15（RST），添加其他外设时注意引脚不冲突。
- OLED 使用双缓冲 + `oled_refresh_gram()` 模式，所有绘图操作先写内存缓冲区，再一次性刷入 SSD1306 显存。
- Keil / EIDE 工程文件容易产生格式噪声，改 XML/YAML 时尽量只改必要片段。
- `Drivers/CMSIS/` 和 `Drivers/STM32F1xx_HAL_Driver/` 是 vendor 代码，除非升级库版本，否则避免大范围修改。
