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
│   ├── BSP/                  # 板级外设驱动
│   │   ├── LED/              # PB5 / PE5，高电平点亮
│   │   ├── KEY/              # PE2/3/4 低有效，PA0 WK_UP 高有效
│   │   ├── OLED/             # PC0-PC7 8080 并行，SSD1306 0.96" 128×64
│   │   └── TPAD/             # PA1，TIM5_CH2 输入捕获触摸检测
│   ├── CMSIS/                # Cortex-M3 / STM32F1 CMSIS
│   ├── STM32F1xx_HAL_Driver/ # STM32F1 HAL 库
│   └── SYSTEM/               # 系统支持层：时钟、延时、串口
├── Projects/MDK-ARM/         # Keil、EIDE、VS Code 工程配置
├── User/                     # 应用入口、HAL 配置、中断入口
├── keilkill.bat              # Windows 下清理 Keil 构建产物
├── CLAUDE.md                 # Claude Code 项目说明
└── README.md
```

## 构建、下载、清理

本仓库没有 Makefile、CMake、命令行 lint 或单元测试入口。构建和下载依赖 Keil MDK-ARM 或 VS Code EIDE。

### Keil MDK-ARM

1. 打开 `Projects/MDK-ARM/atk_f103.uvprojx`
2. 选择 target `LED`
3. Build / Rebuild
4. 使用 ST-LINK 或 CMSIS-DAP 通过 SWD 下载

### VS Code + EIDE

打开 `Projects/MDK-ARM/atk_f103.code-workspace`，安装 EIDE 插件后可运行以下任务：

| 任务 | 作用 |
|---|---|
| `build` | 调用 `${command:eide.project.build}` 编译 |
| `rebuild` | 调用 `${command:eide.project.rebuild}` 全量重编译 |
| `clean` | 调用 `${command:eide.project.clean}` 清理 |
| `flash` | 调用 `${command:eide.project.uploadToDevice}` 下载 |
| `build and flash` | 编译后下载 |

EIDE 下载配置在 `Projects/MDK-ARM/.eide/eide.yml`：OpenOCD、`cmsis-dap`、target `stm32f1x`、基地址 `0x08000000`。

### 清理 Keil 产物

Windows 下可在仓库根目录运行：

```bat
keilkill.bat
```

脚本会递归删除多类 Keil 中间文件和输出文件，编辑前先检查删除范围。

## 启动流程

`User/main.c` 是固件入口，默认流程：

```c
HAL_Init();
sys_stm32_clock_init(RCC_PLL_MUL9);  // 8MHz HSE × 9 = 72MHz
delay_init(72);
usart_init(115200);
led_init();
```

主循环每 500ms 翻转 LED0，并通过 USART1 支持 `printf` 输出。

## 架构说明

### `User/`

- `main.c`: 应用入口，负责 HAL、时钟、延时、串口、BSP 初始化。
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
| OLED | `oled_init()`、`oled_refresh_gram()`、`oled_show_string()`、`oled_clear()` | PC0-PC7 8080 并行接口，128×64 帧缓冲 |

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
- LED 高电平点亮；KEY0/1/2 低电平按下，WK_UP 高电平按下。
- OLED 8080 并口占用了 PC0-PC7（数据）、PD3（RS）、PD6（CS）、PG13（RD）、PG14（WR）、PG15（RST），添加其他外设时注意引脚不冲突。
- OLED 使用双缓冲 + `oled_refresh_gram()` 模式，所有绘图操作先写内存缓冲区，再一次性刷入 SSD1306 显存。
- Keil / EIDE 工程文件容易产生格式噪声，改 XML/YAML 时尽量只改必要片段。
- `Drivers/CMSIS/` 和 `Drivers/STM32F1xx_HAL_Driver/` 是 vendor 代码，除非升级库版本，否则避免大范围修改。
