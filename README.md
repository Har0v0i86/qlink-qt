# Qlink · Qt 连连看桌面游戏

C++ / Qt Widgets 课程项目，支持单人限时挑战和同屏双人对战，包含连接路径判定、角色移动、消除动画、道具、暂停与存档。本仓库用于展示游戏交互、状态管理和测试实践。

> 这是**不含原图片素材的代码展示版**。可以构建程序及运行交互逻辑测试，但人物、地图与图标无法完整显示；

## 功能与实现

- **连接判定**：使用 BFS 携带位置、方向和转弯次数寻找最多两次转弯的连接路径，并回溯路径供界面绘制。
- **可消除配对检测**：枚举同型箱对，判断当前棋盘是否仍存在合法配对；不等同于证明整个棋盘可全部清空。
- **角色与输入**：角色插值移动、方向动画与待处理方向；双人模式分别维护角色、选择和计分状态。
- **状态与存档**：暂停菜单、动画期间输入控制，以及使用 QDataStream 保存／恢复棋盘、玩家与剩余时间。
- **交互测试**：QTest 覆盖首次选择、重复取消、类型不同、合法连接、路径阻挡、转弯超限、动画期间输入、跨玩家取消等场景。

## 构建与运行

依赖：Qt 6 Widgets / Test、C++17 编译器、CMake 3.16+。编译器必须与 Qt Kit 匹配。

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH="<Qt 6 kit directory>"
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

运行 `build/Qlink`（Windows 为 `build/Qlink.exe`，多配置构建可能在 `build/Debug/`）。也可在 Qt Creator 中打开 `CMakeLists.txt`，选择 Qt 6 Desktop Kit。Qt 动态库和平台插件需可被找到。只构建游戏可添加 `-DBUILD_TESTING=OFF`。

| 操作 | 按键 |
| --- | --- |
| 玩家一移动 | W / A / S / D |
| 玩家二移动 | 方向键 |
| 暂停／返回 | Esc |
| 暂停菜单选择／确认 | W / S，Enter |

存档文件 `savegame.dat` 写入当前工作目录，不纳入版本管理。

### 补充本地素材

`resource.qrc` 保留了 `/images` 资源前缀。若持有可用的授权素材，可在本地添加 `Actor1.png`、`Actor3.png`、`Dungeon_A2.png`、`SF_Outside_B.png`、`CaveBar_B.png`、`IconSet.png`、`fsm_Forest01_E.png`，并在 qrc 的 qresource 节点中逐一添加 `<file>文件名.png</file>` 后重建。图集尺寸与排列需对应 `Character.cpp` 和 `GameBoard.cpp` 中的裁剪规则，不能用任意图片替代。

## 代码导航

| 文件 | 职责 |
| --- | --- |
| `GameLogic.cpp/.h` | 连接路径与配对检测 |
| `GameBoard.cpp/.h` | 棋盘、游戏状态、输入、绘制、存档 |
| `Character.cpp/.h` | 角色移动与动画 |
| `MainMenu` / `MainWindow` | 菜单与窗口组织 |
| `tests/tst_gameboard.cpp` | QTest 交互测试 |
| `resource.qrc` | 图像资源注册 |

## 验证与已知边界

2026-10-14：在 Windows、Qt 6.9.0、MinGW 13.1、Ninja 环境实际编译游戏与测试程序；`ctest --output-on-failure` 结果为 **1/1 测试目标通过**（目标内包含多个交互测试）。未执行完整手动游玩验收。

Windows 建议在纯英文路径下构建；本次中文路径导致 moc 无法创建文件，迁移至英文路径后构建和测试通过。

- 路径算法的 visited 当前按格子记录最少转弯数，尚未按进入方向区分；不宣称其已通过所有边界情况的完备性验证。
- `GameBoard` 集中了较多逻辑，进一步拆分状态、渲染和持久化是可改进方向。
- 原课程项目的图片素材未纳入公开仓库，本次不作素材授权声明。

## 项目说明

作者：[Har0v0i86]。源自 SEP 软件工程实践课程项目。发布版整理了 Qt 6 CMake 构建和测试入口，移除了构建缓存与个人 IDE 配置，保留核心游戏逻辑。未附加统一开源许可证。
