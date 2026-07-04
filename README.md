# STM32F103ZET6 通用开发模板

本模板为 STM32F103ZET6 芯片提供的全面、优化的 Keil MDK-ARM 和 VS Code + EIDE 开发环境支持。

## 主要特性

- **驱动支持**：专为 STM32F103ZET6 设计，支持 LED, KEY 等常用外设
- **最小模板**：`User/main.c` 提供完整的初始化和 LED 闪烁示例
- **HAL 模块优化**：预配置 HAL 库，只启动常用模块，降低启动负担
- **调试支持**：Keil 和 EIDE 中均内置完整的调试配置
- **版本控制**：基于 git 的开发模板，支持分支管理

## 目录结构

```
├── Drivers/
│   ├── BSP/                  # 板级支持包
│   │   ├── LED/              # LED 驱动
│   │   └── KEY/              # 按键驱动
│   ├── CMSIS/                # ARM Cortex-M3 CMSIS 核心
│   ├── STM32F1xx_HAL_Driver/ # ST HAL 驱动库
│   └── SYSTEM/               # 系统级支持库
│       ├── sys/              # 系统初始化、NVIC、WFI 等
│       ├── delay/            # SysTick 延时
│       └── usart/            # 串口驱动和 printf
├── Projects/
│   └── MDK-ARM/              # 编译环境配置
├── User/
│   ├── main.c                # 主程序模板
│   ├── stm32f1xx_hal_conf.h  # HAL 配置文件
│   └── stm32f1xx_it.c        # 中断服务
├── keilkill.bat              # Keil 构建辅助脚本
└── .gitignore
```

## 目录介绍

### Drivers/
- **BSP/**：板级支持，包括 LED 和按键驱动函数
- **CMSIS/**：ARM Cortex-M3 CMSIS 核心代码
- **STM32F1xx_HAL_Driver/**：STM32 HAL 驱动库
- **SYSTEM/**：系统支持层，包括时钟、延时、外设初始化

### User/
- **main.c**：模板主程序，包含 HAL 初始化、时钟配置、延时、串口和 LED 初始化
- **stm32f1xx_hal_conf.h**：HAL 库的编译配置，可以启用/关闭特定外设
- **stm32f1xx_it.c**：中断服务函数

### keilkill.bat
Windows 下辅助脚本，用于清理 Keil 构建产物

## 开发环境

### Keil MDK-ARM (uVision 5)

1. 打开 `Projects/MDK-ARM/atk_f103.uvprojx`
2. 工具链：ARM Compiler 5 (AC5)
3. 芯片设置：STM32F103ZE
4. 输出文件：`Output/atk_f103.axf` (用于调试) / `atk_f103.hex` (用于下载)

### VS Code + EIDE

1. 安装 VS Code EIDE 插件
2. 打开工作区 `Projects/MDK-ARM/atk_f103.code-workspace` 
3. 或者直接打开 `Projects/MDK-ARM/` 目录
4. 通过 VS Code 任务或 EIDE 面板执行构建、下载和调试

## 使用说明

### 初始配置

您可以直接使用模板，项目已包含必要的初始化代码。

`User/main.c` 默认包含以下初始化流程：

```c
HAL_Init();                     // HAL 初始化
sys_stm32_clock_init(9);       // 系统时钟 72MHz (8MHz HSE x9)
delay_init(72);                 // 延时初始化
usart_init(115200);             // 串口初始化 (printf 功能)
led_init();                     // LED 初始化
```

主循环将闪烁 LED，并输出调试信息到串口。

### 扩展自定义外设

1. **添加外设支持**：在 `Drivers/BSP/` 中添加新的驱动模块
2. **编辑 HAL 配置**：在 `User/stm32f1xx_hal_conf.h` 中启用需要的 HAL 模块
3. **创建用户代码**：在 `User/` 中创建新的源文件并调用相应的 HAL API
4. **构建项目**：使用 Keil MDK-ARM 或 VS Code EIDE 构建，下载到开发板

### 示例扩展步骤

```
1. 创建新外设驱动文件夹 Drivers/BSP/NEW/
2. 实现 NEW_init() 等初始化函数
3. 在 User/main.c 中添加调用
4. 重新构建并下载
```

### 添加并启用新的 HAL 模块

如果您需要使用模板中禁用的 HAL 外设，请编辑 `User/stm32f1xx_hal_conf.h`

以下模块默认禁用，但可以快速启用：
- **GPIO / SPI / I2C**：常见外设通信
- **ADC / DAC**：模拟信号采集/输出
- **TIM**：定时控制和 PWM
- **USART/UART**：串口通信

```c
// 示例：在 stm32f1xx_hal_conf.h 中取消注释
#define HAL_GPIO_MODULE_ENABLED    // 默认已启用
#define HAL_SPI_MODULE_ENABLED     // 默认禁用，若需使用请取消注释
```

## 快速入门

1. 克隆此仓库到本地
2. 选择开发工具：Keil MDK-ARM 或 VS Code + EIDE
3. 运行示例项目，观察 LED 闪烁和串口输出
4. 根据需要扩展外设，编写自己的代码
5. 构建、下载和调试您的项目

## 注意事项

- 本模板为 STM32F103ZET6 专属，不适用于其他型号
- 初始化代码假设使用 8MHz 外部晶振，时钟配置为 72MHz
- 根据实际需要调整串口波特率和 LED/按键端口映射
- 若需使用禁用模块，请确保连接了相应的外部电路
