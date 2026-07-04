# STM32F103ZET6 通用开发模板

本模板为 STM32F103ZET6 芯片提供的 Keil MDK-ARM 和 VS Code + EIDE 开发环境支持。

## 目录结构

```
├── Drivers/
│   ├── BSP/                  # 板级支持包
│   │   ├── LED/              # LED 驱动 (PB5, PE5)
│   │   ├── KEY/              # 按键驱动 (PE2/3/4, PA0)
│   │   └── TPAD/             # 触摸按键驱动 (PA1, TIM5_CH2)
│   ├── CMSIS/                # ARM Cortex-M3 CMSIS
│   ├── STM32F1xx_HAL_Driver/ # ST HAL 驱动库
│   └── SYSTEM/               # 系统支持层
│       ├── sys/              # 时钟初始化、NVIC、待机、软复位
│       ├── delay/            # SysTick 延时 (us/ms)
│       └── usart/            # 串口驱动 (USART1, printf)
├── Projects/
│   └── MDK-ARM/              # Keil 工程 + EIDE + VS Code 配置
├── User/
│   ├── main.c                # 主程序入口
│   ├── stm32f1xx_hal_conf.h  # HAL 模块配置
│   ├── stm32f1xx_it.c        # 中断服务函数
│   └── stm32f1xx_it.h
├── keilkill.bat              # 清理 Keil 构建产物
└── README.md
```

## 使用说明

### 默认初始化流程 (`User/main.c`)

```c
HAL_Init();                     // HAL 库初始化
sys_stm32_clock_init(9);       // 系统时钟 72MHz (8MHz HSE x 9)
delay_init(72);                 // SysTick 延时初始化
usart_init(115200);             // USART1 (printf)
led_init();                     // LED GPIO 初始化
```

主循环每 500ms 切换 LED0，串口输出调试信息。

### BSP 模块

| 模块 | 引脚 | 说明 |
|---|---|---|
| LED | PB5, PE5 | 推挽输出，高电平点亮 |
| KEY | PE4/PE3/PE2 (低有效), PA0 (高有效) | 带消抖扫描，支持单次/连按模式 |
| TPAD | PA1 (TIM5_CH2) | 电容触摸按键，RC 充放电 + 输入捕获 |

### HAL 模块状态

**已启用且使用中：** CORTEX, FLASH, GPIO, PWR, RCC, TIM, UART, DMA, EXTI

**已启用但未在模板代码中使用**（按需保留）: ADC, CRC, DAC, I2C, SPI, USART, IWDG, WWDG

**默认禁用**（用时在 `stm32f1xx_hal_conf.h` 取消注释）: CAN, ETH, RTC, SD, NAND, NOR, SRAM 等

## 开发环境

### Keil MDK-ARM

打开 `Projects/MDK-ARM/atk_f103.uvprojx`，工具链 ARM Compiler 5 (AC5)，芯片 STM32F103ZE，下载算法 `STM32F10x_512.FLM`。调试器 ST-LINK / CMSIS-DAP，SWD 协议。

### VS Code + EIDE

安装 EIDE 插件，打开工作区 `Projects/MDK-ARM/atk_f103.code-workspace` 或直接打开 `Projects/MDK-ARM/` 目录。编译、下载、调试均通过 EIDE 面板执行。

## 添加新外设

1. `Drivers/BSP/` 下创建模块目录，实现 `xxx_init()` 等 API
2. 在 `User/stm32f1xx_hal_conf.h` 启用所需 HAL 模块
3. 在 `User/main.c` 中调用初始化
4. 将源文件加入 Keil 工程 (uvprojx) 和 EIDE 配置 (eide.yml) 的对应分组

## 注意事项

- 本模板为 STM32F103ZET6 (512KB Flash, 64KB SRAM) 专属
- 默认使用 8MHz 外部晶振，PLL 倍频至 72MHz
- `delay_init()` 接管 SysTick 用于忙等待延时，HAL 超时 API (`HAL_GetTick`) 不可用
- LED 高电平点亮，低电平熄灭；按键低电平有效 (WK_UP 除外)
