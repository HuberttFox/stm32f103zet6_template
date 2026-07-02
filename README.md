# STM32F103ZET6 通用模板仓库

本仓库基于正点原子（ALIENTEK）战舰 F103 开发板代码整理而来，作为 STM32F103ZET6 芯片的通用开发模板。

## 目录结构

```
├── Drivers/
│   ├── BSP/                  # 板级支持包
│   │   ├── LED/              # LED 驱动 (PB5/PE5)
│   │   └── KEY/              # 按键驱动 (PE4/PE3/PE2/PA0)
│   ├── CMSIS/                # ARM Cortex-M3 CMSIS 核心层
│   ├── STM32F1xx_HAL_Driver/ # ST 官方 HAL/LL 驱动库
│   └── SYSTEM/               # 系统级支持库
│       ├── sys/              # 系统函数（时钟初始化、NVIC、WFI、待机等）
│       ├── delay/            # SysTick 延时（delay_us/delay_ms）
│       └── usart/            # 串口驱动（USART1, printf 重定向）
├── Middlewares/               # 中间件目录（当前为空）
├── Projects/
│   └── MDK-ARM/              # Keil uVision 项目 + VS Code EIDE 配置
├── User/
│   ├── main.c                # 主程序模板
│   ├── stm32f1xx_hal_conf.h  # HAL 库配置
│   ├── stm32f1xx_it.c        # 中断服务函数
│   └── stm32f1xx_it.h        # 中断声明
├── keilkill.bat              # Keil 编译中间件清理脚本
├── README.md                 # 本文件
└── .gitignore
```

## 与原版的差异

本模板从正点原子原始代码整理而来，主要变更如下：

### 已删除的内容

| 文件/目录 | 说明 |
|-----------|------|
| `Output/` | 编译产物（每次编译自动生成） |
| `Projects/MDK-ARM/build/` | EIDE 编译输出 |
| `Drivers/BSP/TIMER/` | 正点原子实验专用：TIM8 PWM 输入捕获 Demo |
| `Projects/MDK-ARM/DebugConfig/` | 调试配置文件（绑定原 Demo） |
| `README.TXT / Drivers/readme.txt` | 原始实验说明（含推广信息，功能等价由本文件替代） |

> 以上内容可从 git 历史中恢复。

### 已禁用/裁剪的 HAL 模块

在 `User/stm32f1xx_hal_conf.h` 中，以下模块默认被注释（禁用）。若需使用，取消对应 `#define` 的注释即可：

| 模块 | 宏定义 | 说明 |
|------|--------|------|
| CAN | `HAL_CAN_MODULE_ENABLED` | 控制器局域网 |
| CEC | `HAL_CEC_MODULE_ENABLED` | HDMI 消费电子控制 |
| ETH | `HAL_ETH_MODULE_ENABLED` | 以太网（需外部 PHY） |
| HCD | `HAL_HCD_MODULE_ENABLED` | USB 主机 |
| I2S | `HAL_I2S_MODULE_ENABLED` | I2S 音频接口 |
| IRDA | `HAL_IRDA_MODULE_ENABLED` | 红外通信 |
| MMC | `HAL_MMC_MODULE_ENABLED` | 多媒体卡 |
| NAND | `HAL_NAND_MODULE_ENABLED` | NAND Flash（FSMC） |
| NOR | `HAL_NOR_MODULE_ENABLED` | NOR Flash（FSMC） |
| PCCARD | `HAL_PCCARD_MODULE_ENABLED` | PC 卡（FSMC） |
| PCD | `HAL_PCD_MODULE_ENABLED` | USB 设备 |
| RTC | `HAL_RTC_MODULE_ENABLED` | 实时时钟（需外部 32.768kHz 晶振） |
| SD | `HAL_SD_MODULE_ENABLED` | SD 卡（SDIO） |
| SMARTCARD | `HAL_SMARTCARD_MODULE_ENABLED` | 智能卡 |
| SRAM | `HAL_SRAM_MODULE_ENABLED` | 外部 SRAM（FSMC） |

当前 **启用的模块**：`RCC / GPIO / DMA / CORTEX / FLASH / PWR / TIM / UART / USART / SPI / I2C / ADC / DAC / IWDG / WWDG / CRC / EXTI`

### 版权头清理

`Drivers/BSP/LED/`、`Drivers/BSP/KEY/`、`Drivers/SYSTEM/` 下的源码文件已移除正点原子中文版权和推广信息，功能代码保持不变。

## 开发环境

### Keil MDK-ARM (uVision 5)

- 打开 `Projects/MDK-ARM/atk_f103.uvprojx`
- 工具链：ARM Compiler 5 (AC5)
- 目标芯片：STM32F103ZE
- 输出：`Output/atk_f103.hex` / `atk_f103.axf`

### VS Code + EIDE

- 安装 EIDE 插件
- 打开工作区 `Projects/MDK-ARM/atk_f103.code-workspace`
- 或直接打开 `Projects/MDK-ARM/` 目录
- 构建/下载/调试均可通过 VS Code 任务或 EIDE 面板操作

## 模板主程序说明

`User/main.c` 提供了最小启动模板：

```c
HAL_Init();                     // HAL 库初始化
sys_stm32_clock_init(x9);      // 系统时钟 72MHz (8MHz HSE x9)
delay_init(72);                 // 延时初始化
usart_init(115200);             // 串口初始化 (printf)
led_init();                     // LED 初始化
```

在此基础上，可直接添加外设测试代码。

## 快速开始一个新项目

1. 复制本仓库（或基于 git 创建新分支）
2. 在 `User/` 下添加你的模块代码
3. 在 `Drivers/BSP/` 下添加外设驱动
4. 若需使用未启用的 HAL 模块，编辑 `stm32f1xx_hal_conf.h` 取消对应宏注释
5. 在 Keil 或 EIDE 中构建并下载
